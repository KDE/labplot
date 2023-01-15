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

#ifdef HAVE_VECTOR_BLF
#include <Vector/BLF/Exceptions.h>
#include <Vector/BLF/File.h>
#endif

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
#ifdef HAVE_VECTOR_BLF
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
#endif
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

#ifdef HAVE_VECTOR_BLF
bool getTime(const Vector::BLF::ObjectHeaderBase* ohb, uint64_t& timestamp) {
	/* ObjectHeader */
	auto* oh = dynamic_cast<const Vector::BLF::ObjectHeader*>(ohb);
	if (oh != nullptr) {
		timestamp = oh->objectTimeStamp;
		switch (oh->objectFlags) {
		case Vector::BLF::ObjectHeader::ObjectFlags::TimeTenMics:
			return false;
		case Vector::BLF::ObjectHeader::ObjectFlags::TimeOneNans:
			return true;
		}
	}

	/* ObjectHeader2 */
	auto* oh2 = dynamic_cast<const Vector::BLF::ObjectHeader2*>(ohb);
	if (oh2 != nullptr) {
		timestamp = oh2->objectTimeStamp;
		switch (oh2->objectFlags) {
		case Vector::BLF::ObjectHeader2::ObjectFlags::TimeTenMics:
			return false;
		case Vector::BLF::ObjectHeader2::ObjectFlags::TimeOneNans:
			return true;
		}
	}
	return true;
}
#endif

int VectorBLFFilterPrivate::readDataFromFileCommonTime(const QString& fileName, int lines) {
	PERFTRACE(QLatin1String(Q_FUNC_INFO));
	if (!isValid(fileName) || !m_dbcParser.isValid())
		return 0;

	if (m_parseState.ready && m_parseState.lines == lines)
		return m_parseState.lines;

	m_DataContainer.clear();

#ifdef HAVE_VECTOR_BLF

	Vector::BLF::File file;
	file.open(fileName.toLocal8Bit().data());

	// 1. Reading in messages
	QVector<const Vector::BLF::ObjectHeaderBase*> v;
	Vector::BLF::ObjectHeaderBase* ohb = nullptr;
	QVector<uint32_t> ids;
	uint64_t message_counter = 0;
	{
		PERFTRACE(QLatin1String(Q_FUNC_INFO) + QLatin1String("Parsing BLF file"));
		while (file.good() && ((lines >= 0 && message_counter < lines) || lines < 0)) {
			try {
				ohb = file.read();
			} catch (std::runtime_error& e) {
				DEBUG("Exception: " << e.what() << std::endl);
			}
			if (ohb == nullptr)
				break;

			if (ohb->objectType != Vector::BLF::ObjectType::CAN_MESSAGE2 && ohb->objectType != Vector::BLF::ObjectType::CAN_MESSAGE)
				continue;

			int id;
			if (ohb->objectType == Vector::BLF::ObjectType::CAN_MESSAGE) {
				const auto message = reinterpret_cast<Vector::BLF::CanMessage*>(ohb);
				id = message->id;
			} else if (ohb->objectType == Vector::BLF::ObjectType::CAN_MESSAGE2) {
				const auto message = reinterpret_cast<Vector::BLF::CanMessage2*>(ohb);
				id = message->id;
			} else
				return 0;

			v.append(ohb);
			if (!ids.contains(id))
				ids.append(id);
			message_counter++;
		}
	}

	// 2. Create vector names
	QHash<uint32_t, int> idIndexTable;
	vectorNames = m_dbcParser.signals(ids, idIndexTable);

	// 3. allocate memory
	if (convertTimeToSeconds) {
		auto* vector = new QVector<double>();
		vector->resize(message_counter);
		m_DataContainer.appendVector<double>(vector, AbstractColumn::ColumnMode::Double);
	} else {
		auto* vector = new QVector<qint64>();
		vector->resize(message_counter);
		m_DataContainer.appendVector<qint64>(vector, AbstractColumn::ColumnMode::BigInt); // BigInt is qint64 and not quint64!
	}
	for (int i = 0; i < vectorNames.length(); i++) {
		auto* vector = new QVector<double>();
		vector->resize(message_counter);
		m_DataContainer.appendVector(vector, AbstractColumn::ColumnMode::Double);
	}

	// 4. fill datacontainer
	int message_index = 0;
	bool timeInNS = true;
	if (timeHandlingMode == CANFilter::TimeHandling::ConcatNAN) {
		for (const auto ohb : v) {
			int id;
			std::vector<double> values;
			if (ohb->objectType == Vector::BLF::ObjectType::CAN_MESSAGE) {
				const auto message = reinterpret_cast<const Vector::BLF::CanMessage*>(ohb);
				id = message->id;
				m_dbcParser.parseMessage(message->id, message->data, values);
			} else if (ohb->objectType == Vector::BLF::ObjectType::CAN_MESSAGE2) {
				const auto message = reinterpret_cast<const Vector::BLF::CanMessage2*>(ohb);
				id = message->id;
				m_dbcParser.parseMessage(message->id, message->data, values);
			} else
				return 0;

			if (values.size() == 0) {
				// id is not available in the dbc file, so it is not possible to decode
				DEBUG("Unable to decode message: " << id);
				continue;
			}

			uint64_t timestamp;
			timeInNS = getTime(ohb, timestamp);
			if (convertTimeToSeconds) {
				double timestamp_seconds;
				if (timeInNS)
					timestamp_seconds = (double)timestamp / pow(10, 9); // TimeOneNans
				else
					timestamp_seconds = (double)timestamp / pow(10, 5); // TimeTenMics

				m_DataContainer.setData<double>(0, message_index, timestamp_seconds);
			} else
				m_DataContainer.setData<qint64>(0, message_index, timestamp);

			const auto startIndex = idIndexTable.value(id) + 1; // +1 because of time
			for (int i = 1; i < startIndex; i++) {
				m_DataContainer.setData<double>(i, message_index, NAN);
			}
			for (int i = startIndex; i < startIndex + values.size(); i++) {
				m_DataContainer.setData<double>(i, message_index, values.at(i - startIndex));
			}
			for (int i = startIndex + values.size(); i < m_DataContainer.size(); i++) {
				m_DataContainer.setData<double>(i, message_index, NAN);
			}
			message_index++;
		}
	} else {
		bool firstMessageValid = false;
		for (const auto ohb : v) {
			int id;
			std::vector<double> values;
			if (ohb->objectType == Vector::BLF::ObjectType::CAN_MESSAGE) {
				const auto message = reinterpret_cast<const Vector::BLF::CanMessage*>(ohb);
				id = message->id;
				m_dbcParser.parseMessage(message->id, message->data, values);
			} else if (ohb->objectType == Vector::BLF::ObjectType::CAN_MESSAGE2) {
				const auto message = reinterpret_cast<const Vector::BLF::CanMessage2*>(ohb);
				id = message->id;
				m_dbcParser.parseMessage(message->id, message->data, values);
			} else
				return 0;

			if (values.size() == 0) {
				// id is not available in the dbc file, so it is not possible to decode
				DEBUG("Unable to decode message: " << id);
				continue;
			}

			uint64_t timestamp;
			timeInNS = getTime(ohb, timestamp);
			if (convertTimeToSeconds) {
				double timestamp_seconds;
				if (timeInNS)
					timestamp_seconds = (double)timestamp / pow(10, 9); // TimeOneNans
				else
					timestamp_seconds = (double)timestamp / pow(10, 5); // TimeTenMics

				m_DataContainer.setData<double>(0, message_index, timestamp_seconds);
			} else
				m_DataContainer.setData<qint64>(0, message_index, timestamp);

			if (firstMessageValid) {
				const auto startIndex = idIndexTable.value(id) + 1; // +1 because of time
				for (std::vector<double>::size_type i = 1; i < startIndex; i++) {
					const auto prevValue = m_DataContainer.data<double>(i, message_index - 1);
					m_DataContainer.setData<double>(i, message_index, prevValue);
				}
				for (std::vector<double>::size_type i = startIndex; i < startIndex + values.size(); i++) {
					m_DataContainer.setData<double>(i, message_index, values.at(i - startIndex));
				}
				for (std::vector<double>::size_type i = startIndex + values.size(); i < m_DataContainer.size(); i++) {
					const auto prevValue = m_DataContainer.data<double>(i, message_index - 1);
					m_DataContainer.setData<double>(i, message_index, prevValue);
				}
			} else {
				const auto startIndex = idIndexTable.value(id) + 1; // +1 because of time
				for (std::vector<double>::size_type i = 1; i < startIndex; i++) {
					m_DataContainer.setData<double>(i, message_index, 0);
				}
				for (std::vector<double>::size_type i = startIndex; i < startIndex + values.size(); i++) {
					m_DataContainer.setData<double>(i, message_index, values.at(i - startIndex));
				}
				for (std::vector<double>::size_type i = startIndex + values.size(); i < m_DataContainer.size(); i++) {
					m_DataContainer.setData<double>(i, message_index, 0);
				}
				firstMessageValid = true;
			}
			message_index++;
		}
	}

	// 5. add Time column to vector Names
	if (convertTimeToSeconds)
		vectorNames.prepend(QObject::tr("Time_s")); // Must be done after allocating memory
	else if (timeInNS)
		vectorNames.prepend(QObject::tr("Time_ns")); // Must be done after allocating memory
	else
		vectorNames.prepend(QObject::tr("Time_10µs")); // Must be done after allocating memory

	for (const auto& message : v)
		delete message;

	if (!m_DataContainer.resize(message_index))
		return 0;

	// Use message_counter here, because it will be used as reference for caching
	m_parseState = ParseState(message_counter);
	return message_index;
#else
	return 0;
#endif // HAVE_VECTOR_BLF
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
