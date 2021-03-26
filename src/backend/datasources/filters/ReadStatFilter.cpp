/***************************************************************************
File                 : ReadStatFilter.cpp
Project              : LabPlot
Description          : ReadStat I/O-filter
--------------------------------------------------------------------
Copyright            : (C) 2021 by Stefan Gerlach (stefan.gerlach@uni.kn)
***************************************************************************/

/***************************************************************************
*                                                                         *
*  This program is free software; you can redistribute it and/or modify   *
*  it under the terms of the GNU General Public License as published by   *
*  the Free Software Foundation; either version 2 of the License, or      *
*  (at your option) any later version.                                    *
*                                                                         *
*  This program is distributed in the hope that it will be useful,        *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
*  GNU General Public License for more details.                           *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the Free Software           *
*   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
*   Boston, MA  02110-1301  USA                                           *
*                                                                         *
***************************************************************************/
#include "ReadStatFilter.h"
#include "ReadStatFilterPrivate.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/core/column/Column.h"
#include "backend/lib/macros.h"

#include <KLocalizedString>
//#include <QProcess>
#include <QFile>

///////////// macros ///////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////

/*!
	\class ReadStatFilter
	\brief Manages the import/export of data from/to a ReadStat file.

	\ingroup datasources
*/
ReadStatFilter::ReadStatFilter():AbstractFileFilter(FileType::READSTAT), d(new ReadStatFilterPrivate(this)) {}

ReadStatFilter::~ReadStatFilter() = default;

QVector<QStringList> ReadStatFilter::preview(const QString& fileName, int lines) {
	return d->preview(fileName, lines);
}

/*!
  reads the content of the file \c fileName to the data source \c dataSource.
*/
void ReadStatFilter::readDataFromFile(const QString& fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode mode) {
	d->readDataFromFile(fileName, dataSource, mode);
}

/*!
writes the content of the data source \c dataSource to the file \c fileName.
*/
void ReadStatFilter::write(const QString & fileName, AbstractDataSource* dataSource) {
	d->write(fileName, dataSource);
// 	emit()
}

///////////////////////////////////////////////////////////////////////
/*!
  loads the predefined filter settings for \c filterName
*/
void ReadStatFilter::loadFilterSettings(const QString& filterName) {
	Q_UNUSED(filterName);
}

/*!
  saves the current settings as a new filter with the name \c filterName
*/
void ReadStatFilter::saveFilterSettings(const QString& filterName) const {
	Q_UNUSED(filterName);
}

void ReadStatFilter::setStartRow(const int r) {
	d->startRow = r;
}
int ReadStatFilter::startRow() const {
	return d->startRow;
}

void ReadStatFilter::setEndRow(const int r) {
	d->endRow = r;
}
int ReadStatFilter::endRow() const {
	return d->endRow;
}

QStringList ReadStatFilter::vectorNames() const {
	return d->m_varNames;
}

QVector<AbstractColumn::ColumnMode> ReadStatFilter::columnModes() const {
	return d->m_columnModes;
}

///////////////////////////////////////////////////////////////////////
#ifdef HAVE_READSTAT
int ReadStatFilter::getMetaData(readstat_metadata_t *metadata, void *md) {
	*(readstat_metadata_t *)md = *metadata;

	return READSTAT_HANDLER_OK;
}
#endif

QString ReadStatFilter::fileInfoString(const QString& fileName) {
	DEBUG(Q_FUNC_INFO << ", file name = " << qPrintable(fileName))

	QString info;
#ifdef HAVE_READSTAT
	readstat_parser_t *parser = readstat_parser_init();
	readstat_set_metadata_handler(parser, &getMetaData);

	readstat_error_t error = READSTAT_OK;
	readstat_metadata_t metadata;
	if ( fileName.endsWith(QLatin1String(".dta")) )
		error = readstat_parse_dta(parser, qPrintable(fileName), &metadata);
	else if ( fileName.endsWith(QLatin1String(".sav")) || fileName.endsWith(QLatin1String(".zsav")) )
		error = readstat_parse_sav(parser, qPrintable(fileName), &metadata);
	else if (fileName.endsWith(QLatin1String(".por")) )
		error = readstat_parse_por(parser, qPrintable(fileName), &metadata);
	else if (fileName.endsWith(QLatin1String(".sas7bdat")) )
		error = readstat_parse_sas7bdat(parser, qPrintable(fileName), &metadata);
	else if (fileName.endsWith(QLatin1String(".sas7bcat")) )
		error = readstat_parse_sas7bcat(parser, qPrintable(fileName), &metadata);
	else if (fileName.endsWith(QLatin1String(".xpt")) || fileName.endsWith(QLatin1String(".xpt5"))  || fileName.endsWith(QLatin1String(".xpt8")) )
		error = readstat_parse_xport(parser, qPrintable(fileName), &metadata);
	else
		return i18n("Unknown file extension");

	readstat_parser_free(parser);

	if (error == READSTAT_OK) {
		info += i18n("Number of records: %1", QString::number((int64_t)metadata.row_count));
		info += QLatin1String("<br>");
		info += i18n("Number of variables: %1", QString::number((int64_t)metadata.var_count));
		info += QLatin1String("<br>");
		info += i18n("Creation time: %1", QDateTime::fromTime_t(metadata.creation_time).toString());
		info += QLatin1String("<br>");
		info += i18n("Modification time: %1", QDateTime::fromTime_t(metadata.modified_time).toString());
		info += QLatin1String("<br>");
		info += i18n("Format version: %1", QString::number((int64_t)metadata.file_format_version));
		info += QLatin1String("<br>");
		QString compress;
		switch (metadata.compression) {
		case READSTAT_COMPRESS_NONE:
			compress = "none";
			break;
		case READSTAT_COMPRESS_ROWS:
			compress = "rows";
			break;
		case READSTAT_COMPRESS_BINARY:
			compress = "binary";
			break;
		}
		info += i18n("Compression: %1", compress);
		info += QLatin1String("<br>");
		QString endian;
		switch (metadata.endianness) {
		case READSTAT_ENDIAN_NONE:
			endian = "none";
			break;
		case READSTAT_ENDIAN_LITTLE:
			endian = "little";
			break;
		case READSTAT_ENDIAN_BIG:
			endian = "big";
			break;
		}
		info += i18n("Endianess: %1", endian);
		info += QLatin1String("<br>");
		info += i18n("Table name: %1", QString(metadata.table_name));
		info += QLatin1String("<br>");
		info += i18n("File label: %1", QString(metadata.file_label));
		info += QLatin1String("<br>");
		info += i18n("File encoding: %1", QString(metadata.file_encoding));
		info += QLatin1String("<br>");
		info += i18n("64bit: %1", QString::number((unsigned int)metadata.is64bit));
		info += QLatin1String("<br>");
	} else {
		info += i18n("Error getting file info");
	}
#endif

	return info;
}

//#####################################################################
//################### Private implementation ##########################
//#####################################################################

// static members (needed by C callbacks)
int ReadStatFilterPrivate::m_varCount = 0;
int ReadStatFilterPrivate::m_rowCount = 0;
QStringList ReadStatFilterPrivate::m_varNames;
QVector<AbstractColumn::ColumnMode> ReadStatFilterPrivate::m_columnModes;
QStringList ReadStatFilterPrivate::m_lineString;
QVector<QStringList> ReadStatFilterPrivate::m_dataStrings;
std::vector<void*> ReadStatFilterPrivate::m_dataContainer;

#ifdef HAVE_READSTAT
// callbacks
int ReadStatFilterPrivate::getMetaData(readstat_metadata_t *metadata, void *ptr) {
	DEBUG(Q_FUNC_INFO)
	Q_UNUSED(ptr)
	m_varCount = readstat_get_var_count(metadata);
	m_rowCount = readstat_get_row_count(metadata);

	return READSTAT_HANDLER_OK;
}
int ReadStatFilterPrivate::getVarName(int index, readstat_variable_t *variable, const char *val_labels, void *ptr) {
	DEBUG(Q_FUNC_INFO)
	Q_UNUSED(index)
	Q_UNUSED(val_labels)
	Q_UNUSED(ptr)

	m_varNames << readstat_variable_get_name(variable);

	return READSTAT_HANDLER_OK;
}
int ReadStatFilterPrivate::getColumnModes(int obs_index, readstat_variable_t *variable, readstat_value_t value, void *ptr) {
	Q_UNUSED(variable)
	Q_UNUSED(ptr)

	if (obs_index >= m_rowCount)	// more rows found than meta data said (maybe -1)
		m_rowCount = obs_index + 1;

	if (obs_index > 0)
		return READSTAT_HANDLER_OK;
	DEBUG(Q_FUNC_INFO)

	// column modes
	switch (readstat_value_type(value)) {
	case READSTAT_TYPE_INT8:
	case READSTAT_TYPE_INT16:
	case READSTAT_TYPE_INT32:
		m_columnModes << AbstractColumn::ColumnMode::Integer;
		break;
	case READSTAT_TYPE_FLOAT:
	case READSTAT_TYPE_DOUBLE:
		m_columnModes << AbstractColumn::ColumnMode::Numeric;
		break;
	case READSTAT_TYPE_STRING:
		m_columnModes << AbstractColumn::ColumnMode::Text;
	case READSTAT_TYPE_STRING_REF:
		//TODO
		break;
	}

	return READSTAT_HANDLER_OK;
}
int ReadStatFilterPrivate::getValuesPreview(int obs_index, readstat_variable_t *variable, readstat_value_t value, void *ptr) {
	Q_UNUSED(ptr)	// use for lines?

	int var_index = readstat_variable_get_index(variable);

	if (obs_index == 0)
		getColumnModes(obs_index, variable, value, ptr);

	// read values into m_lineString and finally into m_dataStrings
	if (var_index == 0)
		m_lineString.clear();

	if (readstat_value_is_system_missing(value)) {
		m_lineString << QString();
	} else {
		switch (readstat_value_type(value)) {
		case READSTAT_TYPE_INT8:
			m_lineString << QString::number(readstat_int8_value(value));
			break;
		case READSTAT_TYPE_INT16:
			m_lineString << QString::number(readstat_int16_value(value));
			break;
		case READSTAT_TYPE_INT32:
			m_lineString << QString::number(readstat_int32_value(value));
			break;
		case READSTAT_TYPE_FLOAT:
			m_lineString << QString::number(readstat_float_value(value));
			break;
		case READSTAT_TYPE_DOUBLE:
			m_lineString << QString::number(readstat_double_value(value));
			break;
		case READSTAT_TYPE_STRING:
			m_lineString << readstat_string_value(value);
			break;
		case READSTAT_TYPE_STRING_REF:
			//TODO
			break;
		}
	}

	if (var_index == m_varCount - 1) {
		//QDEBUG(Q_FUNC_INFO << ", data line = " << m_lineString)
		m_dataStrings << m_lineString;
	}

	return READSTAT_HANDLER_OK;
}
int ReadStatFilterPrivate::getValues(int row, readstat_variable_t *variable, readstat_value_t value, void *ptr) {
	Q_UNUSED(ptr)	// use for lines?

	const int col = readstat_variable_get_index(variable);
	//DEBUG(Q_FUNC_INFO << ", obs_index = " << obs_index << " var_index = " << var_index)

	// import data
	if (readstat_value_is_system_missing(value)) {	// empty
		if (readstat_value_type(value) == READSTAT_TYPE_FLOAT || readstat_value_type(value) == READSTAT_TYPE_DOUBLE) {
			QVector<double>& container = *static_cast<QVector<double>*>(m_dataContainer[col]);
			container[row] = qQNaN();
		}
	} else {
		switch (readstat_value_type(value)) {
		case READSTAT_TYPE_INT8: {
			QVector<int>& container = *static_cast<QVector<int>*>(m_dataContainer[col]);
			container[row] = readstat_int8_value(value);
			break;
		}
		case READSTAT_TYPE_INT16: {
			QVector<int>& container = *static_cast<QVector<int>*>(m_dataContainer[col]);
			container[row] = readstat_int16_value(value);
			break;
		}
		case READSTAT_TYPE_INT32: {
			QVector<int>& container = *static_cast<QVector<int>*>(m_dataContainer[col]);
			container[row] = readstat_int32_value(value);
			break;
		}
		case READSTAT_TYPE_FLOAT: {
			QVector<double>& container = *static_cast<QVector<double>*>(m_dataContainer[col]);
			container[row] = readstat_float_value(value);
			break;
		}
		case READSTAT_TYPE_DOUBLE: {
			QVector<double>& container = *static_cast<QVector<double>*>(m_dataContainer[col]);
			container[row] = readstat_double_value(value);
			break;
		}
		case READSTAT_TYPE_STRING: {
			QVector<QString>& container = *static_cast<QVector<QString>*>(m_dataContainer[col]);
			container[row] = readstat_string_value(value);
			break;
		}
		case READSTAT_TYPE_STRING_REF:
			//TODO
			break;
		}
	}

	return READSTAT_HANDLER_OK;
}
#endif

ReadStatFilterPrivate::ReadStatFilterPrivate(ReadStatFilter* owner) : q(owner) {
#ifdef HAVE_READSTAT
	m_status = 0;
#endif
}

#ifdef HAVE_READSTAT
/*!
 * parse the file with name fileName
 */
readstat_error_t ReadStatFilterPrivate::parse(const QString& fileName, bool preview, bool prepare) {
	DEBUG(Q_FUNC_INFO << ", file " << fileName.toStdString())

	readstat_parser_t *parser = readstat_parser_init();
	readstat_set_metadata_handler(parser, &getMetaData);	// metadata
	readstat_set_variable_handler(parser, &getVarName);	// header
	if (preview)	// get data and save into m_dataStrings
		readstat_set_value_handler(parser, &getValuesPreview);
	else if (prepare)	// only read column modes
		readstat_set_value_handler(parser, &getColumnModes);
	else	// get and save data into data container
		readstat_set_value_handler(parser, &getValues);
	//TODO: note_handler, fweight_handler, value_label_handler

	readstat_error_t error = READSTAT_OK;
	if ( fileName.endsWith(QLatin1String(".dta")) )
		error = readstat_parse_dta(parser, qPrintable(fileName), nullptr);
	else if ( fileName.endsWith(QLatin1String(".sav")) || fileName.endsWith(QLatin1String(".zsav")) )
		error = readstat_parse_sav(parser, qPrintable(fileName), nullptr);
	else if (fileName.endsWith(QLatin1String(".por")) )
		error = readstat_parse_por(parser, qPrintable(fileName), nullptr);
	else if (fileName.endsWith(QLatin1String(".sas7bdat")) )
		error = readstat_parse_sas7bdat(parser, qPrintable(fileName), nullptr);
	else if (fileName.endsWith(QLatin1String(".sas7bcat")) )
		error = readstat_parse_sas7bcat(parser, qPrintable(fileName), nullptr);
	else if (fileName.endsWith(QLatin1String(".xpt")) || fileName.endsWith(QLatin1String(".xpt5"))  || fileName.endsWith(QLatin1String(".xpt8")) )
		error = readstat_parse_xport(parser, qPrintable(fileName), nullptr);
	else {
		DEBUG(Q_FUNC_INFO << ", ERROR: Unknown file extension")
	}
	readstat_parser_free(parser);

	return error;
}
#endif

/*!
 * generates the preview for the file \c fileName reading the provided number of \c lines.
 */
QVector<QStringList> ReadStatFilterPrivate::preview(const QString& fileName, int lines) {
	Q_UNUSED(lines)	//TODO

	m_varNames.clear();
	m_columnModes.clear();
	m_dataStrings.clear();

#ifdef HAVE_READSTAT
	readstat_error_t error = parse(fileName, true);	// lines?

	if (error == READSTAT_OK) {
		DEBUG(Q_FUNC_INFO << ", var count = " << m_varCount)
		QDEBUG(Q_FUNC_INFO << ", var names = " << m_varNames)
		for (int i = 0; i < m_varCount ; i++)
			DEBUG(Q_FUNC_INFO << ", column mode " << i << " = " << ENUM_TO_STRING(AbstractColumn, ColumnMode, m_columnModes[i]))
		DEBUG(Q_FUNC_INFO << ", read " << m_dataStrings.size() << " lines")
	} else {
		DEBUG(Q_FUNC_INFO << ", ERROR: processing " << qPrintable(fileName))
	}
#else
	Q_UNUSED(fileName)
#endif

	return m_dataStrings;
}

/*!
    reads the content of file \c fileName to the data source \c dataSource.
    Uses the settings defined in the data source.
*/
void ReadStatFilterPrivate::readDataFromFile(const QString& fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode mode) {
	DEBUG(Q_FUNC_INFO << ", fileName = \'" << STDSTRING(fileName) << "\', dataSource = "
	      << dataSource << ", mode = " << ENUM_TO_STRING(AbstractFileFilter, ImportMode, mode));

	m_varNames.clear();
	m_columnModes.clear();
	m_dataStrings.clear();

#ifdef HAVE_READSTAT
	DEBUG(Q_FUNC_INFO << ", Parsing meta data ...")
	// parse meta data and column modes only
	readstat_error_t error = parse(fileName, false, true);		//TODO option "lines" ?
	if (error != READSTAT_OK) {
		DEBUG(Q_FUNC_INFO << ", ERROR preparsing file " << fileName.toStdString())
		return;
	}

	DEBUG(Q_FUNC_INFO << ", found " << m_varCount <<" cols, " << m_rowCount << " rows")

	//prepare data container
	const int startRow = 1, endRow = -1;
	const int actualEndRow = (endRow == -1 || endRow > m_rowCount) ? m_rowCount : endRow;
	const int actualRows = actualEndRow - startRow + 1;
	const int actualCols = m_varCount;
	const int columnOffset = dataSource->prepareImport(m_dataContainer, mode, actualRows, actualCols, m_varNames, m_columnModes);

	error = parse(fileName);		//TODO option "lines" ?
	if (error != READSTAT_OK) {
		DEBUG(Q_FUNC_INFO << ", ERROR parsing file " << fileName.toStdString())
		return;
	}

	dataSource->finalizeImport(columnOffset, 1, actualCols, QString(), mode);
#endif
}

/*!
    writes the content of \c dataSource to the file \c fileName.
*/
void ReadStatFilterPrivate::write(const QString & fileName, AbstractDataSource* dataSource) {
	Q_UNUSED(fileName);
	Q_UNUSED(dataSource);
	//TODO: writing ReadStat files not implemented yet
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################

/*!
  Saves as XML.
 */
void ReadStatFilter::save(QXmlStreamWriter* writer) const {
	writer->writeStartElement("readstatFilter");
	writer->writeEndElement();
}

/*!
  Loads from XML.
*/
bool ReadStatFilter::load(XmlStreamReader* reader) {
	Q_UNUSED(reader);
// 	KLocalizedString attributeWarning = ki18n("Attribute '%1' missing or empty, default value is used");
// 	QXmlStreamAttributes attribs = reader->attributes();
	return true;
}
