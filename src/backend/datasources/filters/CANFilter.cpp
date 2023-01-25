/*
	File                 : CANFilter.cpp
	Project              : LabPlot
	Description          : I/O-filter for parsing CAN files which need a dbc file to parse data
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "backend/datasources/filters/CANFilter.h"
#include "backend/datasources/AbstractDataSource.h"
#include "backend/datasources/filters/CANFilterPrivate.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/macros.h"

#include <KLocalizedString>
#include <QFile>
#include <QProcess>
#include <QTreeWidgetItem>

//////////////////////////////////////////////////////////////////////
CANFilter::CANFilter(FileType type, CANFilterPrivate* p)
	: AbstractFileFilter(type)
	, d(p) {
}

CANFilter::~CANFilter() = default;

/*!
  reads the content of the file \c fileName to the data source \c dataSource.
*/
void CANFilter::readDataFromFile(const QString& fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode mode) {
	d->readDataFromFile(fileName, dataSource, mode);
}

/*!
writes the content of the data source \c dataSource to the file \c fileName.
*/
void CANFilter::write(const QString& fileName, AbstractDataSource* dataSource) {
	d->write(fileName, dataSource);
}

///////////////////////////////////////////////////////////////////////
/*!
  loads the predefined filter settings for \c filterName
*/
void CANFilter::loadFilterSettings(const QString& /*filterName*/) {
}

/*!
  saves the current settings as a new filter with the name \c filterName
*/
void CANFilter::saveFilterSettings(const QString& /*filterName*/) const {
}

///////////////////////////////////////////////////////////////////////

QStringList CANFilter::vectorNames() const {
	return d->vectorNames;
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

//#####################################################################
//################### Private implementation ##########################
//#####################################################################
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
		}
	}
	m_columnModes.clear();
	m_dataContainer.clear();
}

int CANFilterPrivate::DataContainer::size() const {
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

const void* CANFilterPrivate::DataContainer::datas(int index) const {
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
		case AbstractColumn::ColumnMode::Double: {
			static_cast<QVector<double>*>(m_dataContainer[i])->resize(s);
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

CANFilterPrivate::CANFilterPrivate(CANFilter* owner)
	: q(owner) {
}

/*!
	parses the content of the file \c fileName and fill the tree using rootItem.
	returns -1 on error
*/
QVector<QStringList> CANFilterPrivate::preview(const QString& fileName, int lines) {
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
	const int columnOffset = dataSource->prepareImport(dc, mode, rows, vectorNames.length(), vectorNames, columnModes(), false);
	dataSource->finalizeImport(columnOffset);
    return rows;
}

/*!
	writes the content of \c dataSource to the file \c fileName.
*/
void CANFilterPrivate::write(const QString& /*fileName*/, AbstractDataSource* /*dataSource*/) {
	// TODO: Not yet implemented
}

bool CANFilterPrivate::setDBCFile(const QString& filename) {
	return m_dbcParser.parseFile(filename);
}

const QVector<AbstractColumn::ColumnMode> CANFilterPrivate::columnModes() {
	return m_DataContainer.columnModes();
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################

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
	// 	KLocalizedString attributeWarning = ki18n("Attribute '%1' missing or empty, default value is used");
	// 	QXmlStreamAttributes attribs = reader->attributes();
	return true;
}
