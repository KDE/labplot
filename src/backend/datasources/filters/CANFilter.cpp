/*
	File                 : CANFilter.cpp
	Project              : LabPlot
	Description          : I/O-filter for parsing CAN files which need a dbc file to parse data
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "backend/datasources/filters/CANFilter.h"
#include "backend/core/column/Column.h"
#include "backend/datasources/AbstractDataSource.h"
#include "backend/datasources/filters/CANFilterPrivate.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/macros.h"

#include <KLocalizedString>

//////////////////////////////////////////////////////////////////////
CANFilter::CANFilter(FileType type, CANFilterPrivate* p)
	: AbstractFileFilter(type)
	, d(p) {
}

CANFilter::~CANFilter() = default;

/*!
  reads the content of the file \c fileName to the data source \c dataSource.
*/
void CANFilter::readDataFromFile(const QString& fileName, AbstractDataSource* dataSource, ImportMode mode) {
	d->readDataFromFile(fileName, dataSource, mode);
}

/*!
writes the content of the data source \c dataSource to the file \c fileName.
*/
void CANFilter::write(const QString& fileName, AbstractDataSource* dataSource) {
	d->write(fileName, dataSource);
}

///////////////////////////////////////////////////////////////////////
QStringList CANFilter::vectorNames() const {
	return d->m_signals.signal_names;
}

void CANFilter::setConvertTimeToSeconds(bool convert) {
	if (convert == d->convertTimeToSeconds)
		return;
	d->clearParseState();
	d->convertTimeToSeconds = convert;
}

void CANFilter::setTimeHandlingMode(TimeHandling mode) {
	if (mode == d->timeHandlingMode)
		return;
	d->clearParseState();
	d->timeHandlingMode = mode;
}

QString CANFilter::fileInfoString(const QString& fileName) {
	DEBUG(Q_FUNC_INFO);
	QString info;

	Q_UNUSED(fileName);
	return info;
}

bool CANFilter::setDBCFile(const QString& file) {
	return d->setDBCFile(file);
}

QVector<QStringList> CANFilter::preview(const QString& filename, int lines) {
	return d->preview(filename, lines);
}

const QVector<AbstractColumn::ColumnMode> CANFilter::columnModes() const {
	return d->columnModes();
}

std::vector<void*> CANFilter::dataContainer() const {
	return d->m_DataContainer.dataContainer();
}

// #####################################################################
// ################### Private implementation ##########################
// #####################################################################
void CANFilterPrivate::DataContainer::clear() {
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
		case AbstractColumn::ColumnMode::Text:
		case AbstractColumn::ColumnMode::Month:
		case AbstractColumn::ColumnMode::Day:
		case AbstractColumn::ColumnMode::DateTime:
			break;
		}
	}
	m_columnModes.clear();
	m_dataContainer.clear();
}

size_t CANFilterPrivate::DataContainer::size() const {
	return m_dataContainer.size();
}

const QVector<AbstractColumn::ColumnMode> CANFilterPrivate::DataContainer::columnModes() const {
	return m_columnModes;
}

/*!
 * \brief dataContainer
 * Do not modify outside as long as DataContainer exists!
 * \return
 */
std::vector<void*> CANFilterPrivate::DataContainer::dataContainer() const {
	return m_dataContainer;
}

AbstractColumn::ColumnMode CANFilterPrivate::DataContainer::columnMode(int index) const {
	return m_columnModes.at(index);
}

const void* CANFilterPrivate::DataContainer::datas(size_t index) const {
	if (index < size())
		return m_dataContainer.at(index);
	return nullptr;
}

bool CANFilterPrivate::DataContainer::resize(uint32_t s) const {
	for (uint32_t i = 0; i < m_dataContainer.size(); i++) {
		switch (m_columnModes.at(i)) {
		case AbstractColumn::ColumnMode::BigInt:
			static_cast<QVector<qint64>*>(m_dataContainer[i])->resize(s);
			break;
		case AbstractColumn::ColumnMode::Integer:
			static_cast<QVector<qint32>*>(m_dataContainer[i])->resize(s);
			break;
		case AbstractColumn::ColumnMode::Double:
			static_cast<QVector<double>*>(m_dataContainer[i])->resize(s);
			break;
		// TODO: implement missing cases
		case AbstractColumn::ColumnMode::Text:
		case AbstractColumn::ColumnMode::Month:
		case AbstractColumn::ColumnMode::Day:
		case AbstractColumn::ColumnMode::DateTime:
			break;
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
	case AbstractColumn::ColumnMode::Text:
	case AbstractColumn::ColumnMode::Month:
	case AbstractColumn::ColumnMode::Day:
	case AbstractColumn::ColumnMode::DateTime:
		break;
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
		case AbstractColumn::ColumnMode::Text:
		case AbstractColumn::ColumnMode::Month:
		case AbstractColumn::ColumnMode::Day:
		case AbstractColumn::ColumnMode::DateTime:
			break;
		}
		if (s != size)
			return false;
	}
	return true;
}

CANFilterPrivate::CANFilterPrivate(CANFilter* owner)
	: q(owner) {
}

/*!
	parses the content of the file \c fileName and fill the tree using rootItem.
	returns -1 on error
*/
QVector<QStringList> CANFilterPrivate::preview(const QString& fileName, int lines) {
	if (!isValid(fileName)) {
		q->setLastError(i18n("Invalid file."));
		return {};
	}

	const int readMessages = readDataFromFile(fileName, lines);
	if (readMessages == 0) {
		q->setLastError(i18n("No messages read."));
		return {};
	}

	QVector<QStringList> strings;
	for (int i = 0; i < readMessages; i++) {
		QStringList l;
		for (size_t c = 0; c < m_DataContainer.size(); c++) {
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
			// TODO: other cases
			case AbstractColumn::ColumnMode::Text:
			case AbstractColumn::ColumnMode::Month:
			case AbstractColumn::ColumnMode::Day:
			case AbstractColumn::ColumnMode::DateTime:
				break;
			}
		}
		strings.append(l);
	}
	return strings;
}

void CANFilterPrivate::clearParseState() {
	m_parseState = ParseState();
}

int CANFilterPrivate::readDataFromFile(const QString& fileName, int lines) {
	switch (timeHandlingMode) {
	case CANFilter::TimeHandling::ConcatNAN:
		// fall through
	case CANFilter::TimeHandling::ConcatPrevious:
		return readDataFromFileCommonTime(fileName, lines);
	case CANFilter::TimeHandling::Separate:
		return readDataFromFileSeparateTime(fileName, lines);
	}
	return 0;
}

/*!
	reads the content of the file \c fileName to the data source \c dataSource.
	Uses the settings defined in the data source.
*/
int CANFilterPrivate::readDataFromFile(const QString& fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode mode, int lines) {
	if (!isValid(fileName))
		return 0;

	int rows = readDataFromFile(fileName, lines);
	if (rows == 0)
		return 0;

	auto dc = m_DataContainer.dataContainer();
	bool ok = false;

	// apply datacontainer data to the dataSource
	const int columnOffset = dataSource->prepareImport(dc, mode, rows, m_signals.signal_names.length(), m_signals.signal_names, columnModes(), ok, false);
	if (!ok) {
		q->setLastError(i18n("Not enough memory."));
		return 0;
	}

	dataSource->finalizeImport(columnOffset, 0, m_signals.signal_names.length() - 1);

	// Assign value labels to the column
	auto columns = dataSource->children<Column>();
	if ((size_t)columns.size() == m_signals.value_descriptions.size()) {
		int counter = 0;
		auto signal_descriptions = m_signals.value_descriptions.begin();
		while (signal_descriptions != m_signals.value_descriptions.end()) {
			if (signal_descriptions->size() > 0) {
				auto it = signal_descriptions->begin();
				while (it != signal_descriptions->end()) {
					columns[counter]->addValueLabel((qint64)it->value, it->description);
					it++;
				}
			}
			counter++;
			signal_descriptions++;
		}
	}
	return rows;
}

/*!
	writes the content of \c dataSource to the file \c fileName.
*/
void CANFilterPrivate::write(const QString& /*fileName*/, AbstractDataSource* /*dataSource*/) {
	// TODO: Not yet implemented
}

bool CANFilterPrivate::setDBCFile(const QString& filename) {
	return m_dbcParser.parseFile(filename) == DbcParser::ParseStatus::Success;
}

const QVector<AbstractColumn::ColumnMode> CANFilterPrivate::columnModes() {
	return m_DataContainer.columnModes();
}

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################

/*!
  Saves as XML.
 */
void CANFilter::save(QXmlStreamWriter* writer) const {
	writer->writeStartElement(QStringLiteral("hdfFilter"));
	writer->writeEndElement();
}

/*!
  Loads from XML.
*/
bool CANFilter::load(XmlStreamReader*) {
	return true;
}
