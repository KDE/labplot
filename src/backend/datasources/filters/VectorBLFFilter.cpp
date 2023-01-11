/*
	File                 : VectorBLFFilter.cpp
	Project              : LabPlot
	Description          : Vector BLF I/O-filter
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-License-Identifier: GPL-3.0-or-later
*/

/* TODO:
 * Feature: implement missing data types and ranks
 * Performance: only fill dataPointer or dataStrings (not both)
 */

#include "backend/datasources/filters/VectorBLFFilter.h"
#include "backend/core/column/Column.h"
#include "backend/datasources/LiveDataSource.h"
#include "backend/datasources/filters/VectorBLFFilterPrivate.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/macros.h"
#include "backend/lib/trace.h"

#include <Vector/BLF/Exceptions.h>
#include <Vector/BLF/File.h>
#include <libdbc/dbc.hpp>

#include <KLocalizedString>
#include <QFile>
#include <QProcess>
#include <QTreeWidgetItem>

//////////////////////////////////////////////////////////////////////

/*!
	\class VectorBLFFilter
	\brief Manages the import/export of data from/to a Vector BLF file.

	\ingroup datasources
*/
VectorBLFFilter::VectorBLFFilter()
	: CANFilter(FileType::VECTOR_BLF, new VectorBLFFilterPrivate(this)) {
}

VectorBLFFilter::~VectorBLFFilter() = default;

QString VectorBLFFilter::fileInfoString(const QString& fileName) {
	DEBUG(Q_FUNC_INFO);
	QString info;

	Q_UNUSED(fileName);
	return info;
}

bool VectorBLFFilter::isValid(const QString& filename) {
	try {
		Vector::BLF::File f;
		f.open(filename.toLocal8Bit().data());
        if (!f.is_open())
            return false; // No file
        f.close();
        return true;
	} catch (Vector::BLF::Exception e) {
        return false; // Signature was invalid or something else
	}
	return false;
}

//#####################################################################
//################### Private implementation ##########################
//#####################################################################

VectorBLFFilterPrivate::VectorBLFFilterPrivate(VectorBLFFilter* owner)
	: CANFilterPrivate(owner)
	, q(owner) {
}

bool VectorBLFFilterPrivate::isValid(const QString& filename) const {
	return VectorBLFFilter::isValid(filename);
}

int VectorBLFFilterPrivate::readDataFromFileCommonTime(const QString& fileName, int lines) {
	if (!isValid(fileName) || !m_dbcParser.isValid())
		return 0;

	if (m_parseState.ready && m_parseState.lines == lines)
		return m_parseState.lines;

	m_DataContainer.clear();

	Vector::BLF::File file;
	file.open(fileName.toLocal8Bit().data());

	QVector<const Vector::BLF::CanMessage2*> v;
	Vector::BLF::ObjectHeaderBase* ohb = nullptr;
	QVector<uint32_t> ids;
	uint64_t message_counter = 0;
	QVector<qint64>* timestamps = new QVector<qint64>();
	QVector<double>* timestamps_seconds = new QVector<double>();
	{
		PERFTRACE(QLatin1String(Q_FUNC_INFO));
		while (file.good() && ((lines >= 0 && message_counter < lines) || lines < 0)) {
			try {
				ohb = file.read();
			} catch (std::runtime_error& e) {
				DEBUG("Exception: " << e.what() << std::endl);
			}
			if (ohb == nullptr)
				break;

			if (ohb->objectType != Vector::BLF::ObjectType::CAN_MESSAGE2)
				continue;

			double timestamp_seconds;
			uint64_t timestamp;
			// TODO: why there exist 2 object headers?
			/* ObjectHeader */
			auto* oh = dynamic_cast<Vector::BLF::ObjectHeader*>(ohb);
			if (oh != nullptr) {
				timestamp = oh->objectTimeStamp;
				switch (oh->objectFlags) {
				case Vector::BLF::ObjectHeader::ObjectFlags::TimeTenMics:
					timestamp_seconds = (double)timestamp / pow(10, 5);
					break;
				case Vector::BLF::ObjectHeader::ObjectFlags::TimeOneNans:
					timestamp_seconds = (double)timestamp / pow(10, 9);
					break;
				}
			}

			/* ObjectHeader2 */
			auto* oh2 = dynamic_cast<Vector::BLF::ObjectHeader2*>(ohb);
			if (oh2 != nullptr) {
				timestamp = oh2->objectTimeStamp;
				switch (oh2->objectFlags) {
				case Vector::BLF::ObjectHeader2::ObjectFlags::TimeTenMics:
					timestamp_seconds = (double)timestamp / pow(10, 5);
					break;
				case Vector::BLF::ObjectHeader2::ObjectFlags::TimeOneNans:
					timestamp_seconds = (double)timestamp / pow(10, 9);
					break;
				}
			}

			if (convertTimeToSeconds)
				timestamps_seconds->append(timestamp_seconds);
			else
				timestamps->append(timestamp);

			const auto message = reinterpret_cast<Vector::BLF::CanMessage2*>(ohb);
			v.append(message);
			const int id = message->id;
			if (!ids.contains(id))
				ids.append(id);
			message_counter++;
		}
	}

	QHash<uint32_t, int> idIndexTable;
	vectorNames = m_dbcParser.signals(ids, idIndexTable);

	// Assign timestamps and allocate memory
	if (convertTimeToSeconds)
		m_DataContainer.appendVector<double>(timestamps_seconds, AbstractColumn::ColumnMode::Double);
	else
		m_DataContainer.appendVector<qint64>(timestamps, AbstractColumn::ColumnMode::BigInt); // BigInt is qint64 and not quint64!
	for (int i = 0; i < vectorNames.length(); i++) {
		auto* vector = new QVector<double>();
		vector->resize(message_counter);
		m_DataContainer.appendVector(vector, AbstractColumn::ColumnMode::Double);
	}

	vectorNames.prepend(QStringLiteral("Time")); // Must be done after allocating memory

	if (timeHandlingMode == CANFilter::TimeHandling::ConcatNAN) {
		int message_index = 0;
		for (const auto& message : v) {
			const auto values = m_dbcParser.parseMessage(message->id, message->data);
			if (values.length() == 0) {
				// id is not available in the dbc file, so it is not possible to decode
				DEBUG("Unable to decode message: " << message->id);
				continue;
			}
			const auto startIndex = idIndexTable.value(message->id) + 1; // +1 because of time
			for (int i = 1; i < startIndex; i++) {
				m_DataContainer.setData<double>(i, message_index, NAN);
			}
			for (int i = startIndex; i < startIndex + values.length(); i++) {
				m_DataContainer.setData<double>(i, message_index, values.at(i - startIndex));
			}
			for (int i = startIndex + values.length(); i < m_DataContainer.size(); i++) {
				m_DataContainer.setData<double>(i, message_index, NAN);
			}
			message_index++;
		}
	} else {
		// Use previous value
		auto it = v.constBegin();
		bool valid = false;
		do {
			const auto message = *it;
			if (!message)
				break;
			const auto values = m_dbcParser.parseMessage(message->id, message->data);
			it++;
			if (values.length() == 0) {
				// id is not available in the dbc file, so it is not possible to decode
				DEBUG("Unable to decode message: " << message->id);
				continue;
			}
			valid = true;
			const auto startIndex = idIndexTable.value(message->id) + 1; // +1 because of time
			for (int i = 1; i < startIndex; i++) {
				m_DataContainer.setData<double>(i, 0, 0);
			}
			for (int i = startIndex; i < startIndex + values.length(); i++) {
				m_DataContainer.setData<double>(i, 0, values.at(i - startIndex));
			}
			for (int i = startIndex + values.length(); i < m_DataContainer.size(); i++) {
				m_DataContainer.setData<double>(i, 0, 0);
			}
		} while (!valid);

		int message_index = 1;
		for (; it != v.end(); it++) {
			const auto message = *it;
            if (!message)
                break;
			const auto values = m_dbcParser.parseMessage(message->id, message->data);
			if (values.length() == 0) {
				// id is not available in the dbc file, so it is not possible to decode
				DEBUG("Unable to decode message: " << message->id);
				continue;
			}
			const auto startIndex = idIndexTable.value(message->id) + 1; // +1 because of time
			for (int i = 1; i < startIndex; i++) {
				const auto prevValue = m_DataContainer.data<double>(i, message_index - 1);
				m_DataContainer.setData<double>(i, message_index, prevValue);
			}
			for (int i = startIndex; i < startIndex + values.length(); i++) {
				m_DataContainer.setData<double>(i, message_index, values.at(i - startIndex));
			}
			for (int i = startIndex + values.length(); i < m_DataContainer.size(); i++) {
				const auto prevValue = m_DataContainer.data<double>(i, message_index - 1);
				m_DataContainer.setData<double>(i, message_index, prevValue);
			}
			message_index++;
		}
	}

	// The other one is assigned to the datacontainer
	if (convertTimeToSeconds)
		delete timestamps;
	else
		delete timestamps_seconds;

    for (const auto& message : v)
        delete message;

	if (!m_DataContainer.squeeze())
		return 0;

	m_parseState = ParseState(message_counter);
	return message_counter;
}

int VectorBLFFilterPrivate::readDataFromFileSeparateTime(const QString& fileName, int lines) {
	return 0; // Not implemented yet
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################

/*!
  Saves as XML.
 */
void VectorBLFFilter::save(QXmlStreamWriter* writer) const {
	writer->writeStartElement(QStringLiteral("VectorBLFFilter"));
	writer->writeEndElement();
}

/*!
  Loads from XML.
*/
bool VectorBLFFilter::load(XmlStreamReader*) {
	// 	KLocalizedString attributeWarning = ki18n("Attribute '%1' missing or empty, default value is used");
	// 	QXmlStreamAttributes attribs = reader->attributes();
	return true;
}
