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
	: AbstractFileFilter(FileType::VECTOR_BLF)
	, d(new VectorBLFFilterPrivate(this)) {
}

VectorBLFFilter::~VectorBLFFilter() = default;

/*!
  reads the content of the file \c fileName to the data source \c dataSource.
*/
void VectorBLFFilter::readDataFromFile(const QString& fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode mode) {
	d->readDataFromFile(fileName, dataSource, mode);
}

/*!
writes the content of the data source \c dataSource to the file \c fileName.
*/
void VectorBLFFilter::write(const QString& fileName, AbstractDataSource* dataSource) {
	d->write(fileName, dataSource);
}

///////////////////////////////////////////////////////////////////////
/*!
  loads the predefined filter settings for \c filterName
*/
void VectorBLFFilter::loadFilterSettings(const QString& /*filterName*/) {
}

/*!
  saves the current settings as a new filter with the name \c filterName
*/
void VectorBLFFilter::saveFilterSettings(const QString& /*filterName*/) const {
}

///////////////////////////////////////////////////////////////////////

QStringList VectorBLFFilter::vectorNames() const {
	return d->vectorNames;
}

void VectorBLFFilter::setConvertTimeToSeconds(bool convert) {
	if (convert == d->convertTimeToSeconds)
		return;
	d->clearParseState();
	d->convertTimeToSeconds = convert;
}

void VectorBLFFilter::setTimeHandlingMode(TimeHandling mode) {
	if (mode == d->timeHandlingMode)
		return;
	d->clearParseState();
	d->timeHandlingMode = mode;
}

QString VectorBLFFilter::fileInfoString(const QString& fileName) {
	DEBUG(Q_FUNC_INFO);
	QString info;

	Q_UNUSED(fileName);
	return info;
}

bool VectorBLFFilter::isValid(const QString& filename) {
	return VectorBLFFilterPrivate::isValid(filename);
}

bool VectorBLFFilter::setDBCFile(const QString& file) {
	return d->setDBCFile(file);
}

QVector<QStringList> VectorBLFFilter::preview(const QString& filename, int lines) {
	return d->preview(filename, lines);
}

const QVector<AbstractColumn::ColumnMode> VectorBLFFilter::columnModes() const {
	return d->columnModes();
}

//#####################################################################
//################### Private implementation ##########################
//#####################################################################
void VectorBLFFilterPrivate::DataContainer::clear() {
	for (uint i = 0; i < m_dataContainer.size(); i++) {
		switch (m_columnModes.at(i)) {
		case AbstractColumn::ColumnMode::BigInt:
			delete static_cast<QVector<qint64>*>(m_dataContainer[i]);
			break;
		case AbstractColumn::ColumnMode::Integer:
			delete static_cast<QVector<qint32>*>(m_dataContainer[i]);
			break;
		case AbstractColumn::ColumnMode::Double:
			delete static_cast<QVector<double>*>(m_dataContainer[i]);
			break;
			// TODO: implement missing cases
		}
	}
	m_columnModes.clear();
	m_dataContainer.clear();
}

int VectorBLFFilterPrivate::DataContainer::size() const {
	return m_dataContainer.size();
}

const QVector<AbstractColumn::ColumnMode> VectorBLFFilterPrivate::DataContainer::columnModes() const {
	return m_columnModes;
}

/*!
 * \brief dataContainer
 * Do not modify outside as long as DataContainer exists!
 * \return
 */
std::vector<void*> VectorBLFFilterPrivate::DataContainer::dataContainer() const {
	return m_dataContainer;
}

AbstractColumn::ColumnMode VectorBLFFilterPrivate::DataContainer::columnMode(int index) const {
	return m_columnModes.at(index);
}

const void* VectorBLFFilterPrivate::DataContainer::datas(int index) const {
	if (index < size())
		return m_dataContainer.at(index);
	return nullptr;
}

bool VectorBLFFilterPrivate::DataContainer::squeeze() const {
	for (uint32_t i = 0; i < m_dataContainer.size(); i++) {
		switch (m_columnModes.at(i)) {
		case AbstractColumn::ColumnMode::BigInt:
			static_cast<QVector<qint64>*>(m_dataContainer[i])->squeeze();
			break;
		case AbstractColumn::ColumnMode::Integer:
			static_cast<QVector<qint32>*>(m_dataContainer[i])->squeeze();
			break;
		case AbstractColumn::ColumnMode::Double: {
			static_cast<QVector<double>*>(m_dataContainer[i])->squeeze();
			break;
		}
			// TODO: implement missing cases
		}
	}

	if (m_dataContainer.size() == 0)
		return true;

	// Check that all vectors have same length
	int size = -1;
	switch (m_columnModes.at(0)) {
	case AbstractColumn::ColumnMode::BigInt:
		size = static_cast<QVector<qint64>*>(m_dataContainer[0])->size();
		break;
	case AbstractColumn::ColumnMode::Integer:
		size = static_cast<QVector<qint32>*>(m_dataContainer[0])->size();
		break;
	case AbstractColumn::ColumnMode::Double:
		size = static_cast<QVector<double>*>(m_dataContainer[0])->size();
		break;
		// TODO: implement missing cases
	}

	if (size == -1)
		return false;

	for (uint32_t i = 0; i < m_dataContainer.size(); i++) {
		int s = -1;
		switch (m_columnModes.at(i)) {
		case AbstractColumn::ColumnMode::BigInt:
			s = static_cast<QVector<qint64>*>(m_dataContainer[i])->size();
			break;
		case AbstractColumn::ColumnMode::Integer:
			s = static_cast<QVector<qint32>*>(m_dataContainer[i])->size();
			break;
		case AbstractColumn::ColumnMode::Double:
			s = static_cast<QVector<double>*>(m_dataContainer[i])->size();
			break;
			// TODO: implement missing cases
		}
		if (s != size)
			return false;
	}
	return true;
}

VectorBLFFilterPrivate::VectorBLFFilterPrivate(VectorBLFFilter* owner)
	: q(owner) {
}

/*!
	parses the content of the file \c fileName and fill the tree using rootItem.
	returns -1 on error
*/
QVector<QStringList> VectorBLFFilterPrivate::preview(const QString& fileName, int lines) {
	if (!isValid(fileName))
		return QVector<QStringList>();

	const int readMessages = readDataFromFile(fileName, lines);
	if (readMessages == 0)
		return QVector<QStringList>();

	QVector<QStringList> strings;

	for (int i = 0; i < readMessages; i++) {
		QStringList l;
		for (int c = 0; c < m_DataContainer.size(); c++) {
			const auto* data_ptr = m_DataContainer.datas(c);
			if (!data_ptr)
				continue;

			switch (m_DataContainer.columnMode(c)) {
			case AbstractColumn::ColumnMode::BigInt: {
				const auto v = *static_cast<const QVector<qint64>*>(data_ptr);
				l.append(QString::number(v.at(i)));
				break;
			}
			case AbstractColumn::ColumnMode::Integer: {
				const auto v = *static_cast<const QVector<qint32>*>(data_ptr);
				l.append(QString::number(v.at(i)));
				break;
			}
			case AbstractColumn::ColumnMode::Double: {
				const auto v = *static_cast<const QVector<double>*>(data_ptr);
				l.append(QString::number(v.at(i)));
				break;
			}
			}
		}
		strings.append(l);
	}
	return strings;
}

bool VectorBLFFilterPrivate::isValid(const QString& filename) {
	try {
		Vector::BLF::File f;
		f.open(filename.toLocal8Bit().data());
		f.close();
		return true;
	} catch (Vector::BLF::Exception e) {
		return false;
	}
	return false;
}

void VectorBLFFilterPrivate::clearParseState() {
	m_parseState = ParseState();
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

	if (timeHandlingMode == VectorBLFFilter::TimeHandling::ConcatNAN) {
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

	if (!m_DataContainer.squeeze())
		return 0;

	for (const auto& message : v) {
		delete message;
	}

	m_parseState = ParseState(message_counter);
	return message_counter;
}

int VectorBLFFilterPrivate::readDataFromFileSeparateTime(const QString& fileName, int lines) {
	return 0; // Not implemented yet
}

int VectorBLFFilterPrivate::readDataFromFile(const QString& fileName, int lines) {
	switch (timeHandlingMode) {
	case VectorBLFFilter::TimeHandling::ConcatNAN:
		// fall through
	case VectorBLFFilter::TimeHandling::ConcatPrevious:
		return readDataFromFileCommonTime(fileName, lines);
	case VectorBLFFilter::TimeHandling::Separate:
		return readDataFromFileSeparateTime(fileName, lines);
	}
	return 0;
}

/*!
	reads the content of the file \c fileName to the data source \c dataSource.
	Uses the settings defined in the data source.
*/
void VectorBLFFilterPrivate::readDataFromFile(const QString& fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode mode, int lines) {
	if (!isValid(fileName))
		return;

	int rows = readDataFromFile(fileName, lines);

	auto dc = m_DataContainer.dataContainer();
	const int columnOffset = dataSource->prepareImport(dc, mode, rows, vectorNames.length(), vectorNames, columnModes(), false);
	dataSource->finalizeImport(columnOffset);
}

/*!
	writes the content of \c dataSource to the file \c fileName.
*/
void VectorBLFFilterPrivate::write(const QString& /*fileName*/, AbstractDataSource* /*dataSource*/) {
	// TODO: writing Vector BLF not implemented yet
}

bool VectorBLFFilterPrivate::setDBCFile(const QString& filename) {
	return m_dbcParser.parseFile(filename);
}

const QVector<AbstractColumn::ColumnMode> VectorBLFFilterPrivate::columnModes() {
	return m_DataContainer.columnModes();
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################

/*!
  Saves as XML.
 */
void VectorBLFFilter::save(QXmlStreamWriter* writer) const {
	writer->writeStartElement(QStringLiteral("hdfFilter"));
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
