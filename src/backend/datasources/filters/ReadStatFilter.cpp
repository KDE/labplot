/*
	File                 : ReadStatFilter.cpp
	Project              : LabPlot
	Description          : ReadStat I/O-filter
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2021 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "ReadStatFilter.h"
#include "ReadStatFilterPrivate.h"
#include "backend/core/column/Column.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/macros.h"
#include "backend/spreadsheet/Spreadsheet.h"

#include <KLocalizedString>
#include <QDateTime>
#include <QFile>

///////////// macros ///////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////

/*!
	\class ReadStatFilter
	\brief Manages the import/export of data from/to a ReadStat file.

	\ingroup datasources
*/
ReadStatFilter::ReadStatFilter()
	: AbstractFileFilter(FileType::READSTAT)
	, d(new ReadStatFilterPrivate(this)) {
}

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
void ReadStatFilter::write(const QString& fileName, AbstractDataSource* dataSource) {
	d->write(fileName, dataSource);
	// TODO: not implemented yet
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

void ReadStatFilter::setStartColumn(const int c) {
	d->startColumn = c;
}
int ReadStatFilter::startColumn() const {
	return d->startColumn;
}

void ReadStatFilter::setEndColumn(const int c) {
	d->endColumn = c;
}
int ReadStatFilter::endColumn() const {
	return d->endColumn;
}

QStringList ReadStatFilter::vectorNames() const {
	return d->varNames;
}

QVector<AbstractColumn::ColumnMode> ReadStatFilter::columnModes() const {
	return d->columnModes;
}

///////////////////////////////////////////////////////////////////////
#ifdef HAVE_READSTAT
int ReadStatFilter::getMetaData(readstat_metadata_t* metadata, void* md) {
	*(readstat_metadata_t*)md = *metadata;

	return READSTAT_HANDLER_OK;
}
#endif

QString ReadStatFilter::fileInfoString(const QString& fileName) {
	DEBUG(Q_FUNC_INFO << ", file name = " << qPrintable(fileName))

	QString info;
#ifdef HAVE_READSTAT
	readstat_parser_t* parser = readstat_parser_init();
	readstat_set_metadata_handler(parser, &getMetaData);

	readstat_error_t error = READSTAT_OK;
	readstat_metadata_t metadata;
	if (fileName.endsWith(QLatin1String(".dta")))
		error = readstat_parse_dta(parser, qPrintable(fileName), &metadata);
	else if (fileName.endsWith(QLatin1String(".sav")) || fileName.endsWith(QLatin1String(".zsav")))
		error = readstat_parse_sav(parser, qPrintable(fileName), &metadata);
	else if (fileName.endsWith(QLatin1String(".por")))
		error = readstat_parse_por(parser, qPrintable(fileName), &metadata);
	else if (fileName.endsWith(QLatin1String(".sas7bdat")))
		error = readstat_parse_sas7bdat(parser, qPrintable(fileName), &metadata);
	else if (fileName.endsWith(QLatin1String(".sas7bcat")))
		error = readstat_parse_sas7bcat(parser, qPrintable(fileName), &metadata);
	else if (fileName.endsWith(QLatin1String(".xpt")) || fileName.endsWith(QLatin1String(".xpt5")) || fileName.endsWith(QLatin1String(".xpt8")))
		error = readstat_parse_xport(parser, qPrintable(fileName), &metadata);
	else
		return i18n("Unknown file extension");

	readstat_parser_free(parser);

	if (error == READSTAT_OK) {
		info += i18n("Number of records: %1", QString::number((int64_t)metadata.row_count));
		info += QLatin1String("<br>");
		info += i18n("Number of variables: %1", QString::number((int64_t)metadata.var_count));
		info += QLatin1String("<br>");
		info += i18n("Creation time: %1", QDateTime::fromSecsSinceEpoch(metadata.creation_time).toString());
		info += QLatin1String("<br>");
		info += i18n("Modification time: %1", QDateTime::fromSecsSinceEpoch(metadata.modified_time).toString());
		info += QLatin1String("<br>");
		info += i18n("Format version: %1", QString::number((int64_t)metadata.file_format_version));
		info += QLatin1String("<br>");
		QString compress;
		switch (metadata.compression) {
		case READSTAT_COMPRESS_NONE:
			compress = QStringLiteral("none");
			break;
		case READSTAT_COMPRESS_ROWS:
			compress = QStringLiteral("rows");
			break;
		case READSTAT_COMPRESS_BINARY:
			compress = QStringLiteral("binary");
			break;
		}
		info += i18n("Compression: %1", compress);
		info += QStringLiteral("<br>");
		QString endian;
		switch (metadata.endianness) {
		case READSTAT_ENDIAN_NONE:
			endian = QStringLiteral("none");
			break;
		case READSTAT_ENDIAN_LITTLE:
			endian = QStringLiteral("little");
			break;
		case READSTAT_ENDIAN_BIG:
			endian = QStringLiteral("big");
			break;
		}
		info += i18n("Endianess: %1", endian);
		info += QLatin1String("<br>");
		info += i18n("Table name: %1", QLatin1String(metadata.table_name));
		info += QLatin1String("<br>");
		info += i18n("File label: %1", QLatin1String(metadata.file_label));
		info += QLatin1String("<br>");
		info += i18n("File encoding: %1", QLatin1String(metadata.file_encoding));
		info += QLatin1String("<br>");
		info += i18n("64bit: %1", QString::number((unsigned int)metadata.is64bit));
		info += QLatin1String("<br>");
	} else {
		info += i18n("Error getting file info");
	}
#endif

	return info;
}

// #####################################################################
// ################### Private implementation ##########################
// #####################################################################

// static members (needed by C callbacks)
int ReadStatFilterPrivate::m_varCount = 0;
int ReadStatFilterPrivate::m_rowCount = 0;
QStringList ReadStatFilterPrivate::varNames;
QVector<AbstractColumn::ColumnMode> ReadStatFilterPrivate::columnModes;
QStringList ReadStatFilterPrivate::m_lineString;
QVector<QStringList> ReadStatFilterPrivate::dataStrings;
std::vector<void*> ReadStatFilterPrivate::m_dataContainer;
QStringList ReadStatFilterPrivate::m_notes;
QVector<QString> ReadStatFilterPrivate::m_valueLabels;
QMap<QString, LabelSet> ReadStatFilterPrivate::m_labelSets;
int ReadStatFilterPrivate::startRow{1}, ReadStatFilterPrivate::endRow{-1};
int ReadStatFilterPrivate::startColumn{1}, ReadStatFilterPrivate::endColumn{-1};

#ifdef HAVE_READSTAT
// callbacks
int ReadStatFilterPrivate::getMetaData(readstat_metadata_t* metadata, void*) {
	DEBUG(Q_FUNC_INFO)
	m_varCount = readstat_get_var_count(metadata);
	m_rowCount = readstat_get_row_count(metadata);
	m_valueLabels.resize(m_varCount);

	return READSTAT_HANDLER_OK;
}
int ReadStatFilterPrivate::getVarName(int /*index*/, readstat_variable_t* variable, const char* val_labels, void*) {
	// DEBUG(Q_FUNC_INFO)

	// only on column from startColumn to endColumn
	const int col = readstat_variable_get_index(variable);
	if (col < startColumn - 1 || (endColumn != -1 && col > endColumn - 1)) {
		// DEBUG(Q_FUNC_INFO << ", out of range row/col "<< index << " / " << col)
		return READSTAT_HANDLER_OK;
	}

	if (val_labels) {
		DEBUG(Q_FUNC_INFO << ", val_labels of col " << col << " : " << val_labels)
		m_valueLabels[col] = QLatin1String(val_labels);
		varNames << QLatin1String(readstat_variable_get_name(variable)) + QStringLiteral(" : ") + QLatin1String(val_labels);
	} else
		varNames << QLatin1String(readstat_variable_get_name(variable));

	return READSTAT_HANDLER_OK;
}
int ReadStatFilterPrivate::getColumnModes(int row, readstat_variable_t* variable, readstat_value_t value, void*) {
	if (row >= m_rowCount) // more rows found than meta data said it has (like -1)
		m_rowCount = row + 1;

	const int col = readstat_variable_get_index(variable);
	if (row >= startRow || col < startColumn - 1 || (endColumn != -1 && col > endColumn - 1)) // run only on first row and selected cols
		return READSTAT_HANDLER_OK;

	// column modes
	switch (value.type) {
	case READSTAT_TYPE_INT8:
	case READSTAT_TYPE_INT16:
	case READSTAT_TYPE_INT32:
		columnModes << AbstractColumn::ColumnMode::Integer;
		break;
	case READSTAT_TYPE_FLOAT:
	case READSTAT_TYPE_DOUBLE:
		columnModes << AbstractColumn::ColumnMode::Double;
		break;
	case READSTAT_TYPE_STRING:
	case READSTAT_TYPE_STRING_REF:
		columnModes << AbstractColumn::ColumnMode::Text;
	}

	return READSTAT_HANDLER_OK;
}
int ReadStatFilterPrivate::getValuesPreview(int row, readstat_variable_t* variable, readstat_value_t value, void* ptr) {
	// DEBUG(Q_FUNC_INFO << ", start/end row =" << startRow << "/" << endRow)

	// read only from start to end row/column
	const int col = readstat_variable_get_index(variable);
	if (row < startRow - 1 || (endRow != -1 && row > endRow - 1) || col < startColumn - 1 || (endColumn != -1 && col > endColumn - 1)) {
		// DEBUG(Q_FUNC_INFO << ", out of range row/col "<< row << " / " << col)
		return READSTAT_HANDLER_OK;
	}

	if (row == startRow - 1)
		getColumnModes(row, variable, value, ptr);

	// read values into m_lineString and finally into dataStrings
	if (col == startColumn - 1)
		m_lineString.clear();

	if (value.is_system_missing) {
		m_lineString << QString();
	} else {
		switch (value.type) {
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
		case READSTAT_TYPE_STRING_REF:
			m_lineString << QLatin1String(readstat_string_value(value));
		}
	}

	if (col == m_varCount - 1 || (endColumn != -1 && col == endColumn - 1)) {
		// QDEBUG(Q_FUNC_INFO << ", data line = " << m_lineString)
		dataStrings << m_lineString;
	}

	return READSTAT_HANDLER_OK;
}
int ReadStatFilterPrivate::getValues(int row, readstat_variable_t* variable, readstat_value_t value, void*) {
	// only read from start to end row/col
	const int col = readstat_variable_get_index(variable);
	if (row < startRow - 1 || (endRow != -1 && row > endRow - 1) || col < startColumn - 1 || (endColumn != -1 && col > endColumn - 1)) {
		// DEBUG(Q_FUNC_INFO << ", out of range row/col "<< row << " / " << col)
		return READSTAT_HANDLER_OK;
	}
	const int rowIndex = row - startRow + 1;
	const int colIndex = col - startColumn + 1;

	// DEBUG(Q_FUNC_INFO << ", row/col = " << row << " / " << col << ", row/col index = " << rowIndex << " / " << colIndex)

	// import data
	if (value.is_system_missing) { // empty
		if (value.type == READSTAT_TYPE_FLOAT || value.type == READSTAT_TYPE_DOUBLE) {
			QVector<double>& container = *static_cast<QVector<double>*>(m_dataContainer[colIndex]);
			container[rowIndex] = NAN;
		}
	} else {
		switch (value.type) {
		case READSTAT_TYPE_INT8: {
			QVector<int>& container = *static_cast<QVector<int>*>(m_dataContainer[colIndex]);
			container[rowIndex] = readstat_int8_value(value);
			break;
		}
		case READSTAT_TYPE_INT16: {
			QVector<int>& container = *static_cast<QVector<int>*>(m_dataContainer[colIndex]);
			container[rowIndex] = readstat_int16_value(value);
			break;
		}
		case READSTAT_TYPE_INT32: {
			QVector<int>& container = *static_cast<QVector<int>*>(m_dataContainer[colIndex]);
			container[rowIndex] = readstat_int32_value(value);
			break;
		}
		case READSTAT_TYPE_FLOAT: {
			QVector<double>& container = *static_cast<QVector<double>*>(m_dataContainer[colIndex]);
			container[rowIndex] = readstat_float_value(value);
			break;
		}
		case READSTAT_TYPE_DOUBLE: {
			QVector<double>& container = *static_cast<QVector<double>*>(m_dataContainer[colIndex]);
			container[rowIndex] = readstat_double_value(value);
			break;
		}
		case READSTAT_TYPE_STRING:
		case READSTAT_TYPE_STRING_REF: {
			QVector<QString>& container = *static_cast<QVector<QString>*>(m_dataContainer[colIndex]);
			container[rowIndex] = QLatin1String(readstat_string_value(value));
		}
		}
	}

	return READSTAT_HANDLER_OK;
}
int ReadStatFilterPrivate::getNotes(int index, const char* note, void*) {
	Q_UNUSED(index)
	DEBUG(Q_FUNC_INFO << " note " << index << ": " << note)
	m_notes << QLatin1String(note);

	return READSTAT_HANDLER_OK;
}
int ReadStatFilterPrivate::getFWeights(readstat_variable_t* /*var*/, void*) {
	// TODO: not used yet
	// const int col = readstat_variable_get_index(var);
	// DEBUG(Q_FUNC_INFO << ", fweight of col " << col)

	return READSTAT_HANDLER_OK;
}
// value labels are read in getVarName() and assigned here
int ReadStatFilterPrivate::getValueLabels(const char* val_label, readstat_value_t value, const char* label, void*) {
	// see https://github.com/tidyverse/haven/blob/master/src/DfReader.cpp
	DEBUG(Q_FUNC_INFO << ", value label = " << val_label << " label = " << label << ", type = " << value.type)

	LabelSet& labelSet = m_labelSets[QLatin1String(val_label)];
	switch (value.type) {
	case READSTAT_TYPE_STRING:
	case READSTAT_TYPE_STRING_REF:
		// DEBUG(Q_FUNC_INFO << ", string value label")
		labelSet.add(QLatin1String(readstat_string_value(value)), QLatin1String(label));
		break;
	case READSTAT_TYPE_INT8:
		// DEBUG(Q_FUNC_INFO << ", int8 value label")
		labelSet.add(readstat_int8_value(value), QLatin1String(label));
		break;
	case READSTAT_TYPE_INT16:
		// DEBUG(Q_FUNC_INFO << ", int16 value label")
		labelSet.add(readstat_int16_value(value), QLatin1String(label));
		break;
	case READSTAT_TYPE_INT32:
		// DEBUG(Q_FUNC_INFO << ", int32 value label")
		labelSet.add(readstat_int32_value(value), QLatin1String(label));
		break;
	case READSTAT_TYPE_FLOAT:
		// DEBUG(Q_FUNC_INFO << ", float value label")
		labelSet.add(readstat_float_value(value), QLatin1String(label));
		break;
	case READSTAT_TYPE_DOUBLE:
		// DEBUG(Q_FUNC_INFO << ", double value label")
		labelSet.add(readstat_double_value(value), QLatin1String(label));
		break;
	}

	return READSTAT_HANDLER_OK;
}
#endif

ReadStatFilterPrivate::ReadStatFilterPrivate(ReadStatFilter*) {
}

#ifdef HAVE_READSTAT
/*!
 * parse the file with name fileName
 */
readstat_error_t ReadStatFilterPrivate::parse(const QString& fileName, bool preview, bool prepare) {
	DEBUG(Q_FUNC_INFO << ", file " << STDSTRING(fileName) << ", start/end row: " << startRow << "/" << endRow)
	m_labelSets.clear();

	readstat_parser_t* parser = readstat_parser_init();
	readstat_set_metadata_handler(parser, &getMetaData); // metadata
	readstat_set_variable_handler(parser, &getVarName); // header
	if (preview) // get data and save into dataStrings
		readstat_set_value_handler(parser, &getValuesPreview);
	else if (prepare) // only read column modes
		readstat_set_value_handler(parser, &getColumnModes);
	else { // get and save data into data container
		readstat_set_value_handler(parser, &getValues);
		readstat_set_note_handler(parser, &getNotes);
	}
	readstat_set_fweight_handler(parser, &getFWeights);
	readstat_set_value_label_handler(parser, &getValueLabels);

	readstat_error_t error = READSTAT_OK;
	if (fileName.endsWith(QLatin1String(".dta")))
		error = readstat_parse_dta(parser, qPrintable(fileName), nullptr);
	else if (fileName.endsWith(QLatin1String(".sav")) || fileName.endsWith(QLatin1String(".zsav")))
		error = readstat_parse_sav(parser, qPrintable(fileName), nullptr);
	else if (fileName.endsWith(QLatin1String(".por")))
		error = readstat_parse_por(parser, qPrintable(fileName), nullptr);
	else if (fileName.endsWith(QLatin1String(".sas7bdat")))
		error = readstat_parse_sas7bdat(parser, qPrintable(fileName), nullptr);
	else if (fileName.endsWith(QLatin1String(".sas7bcat")))
		error = readstat_parse_sas7bcat(parser, qPrintable(fileName), nullptr);
	else if (fileName.endsWith(QLatin1String(".xpt")) || fileName.endsWith(QLatin1String(".xpt5")) || fileName.endsWith(QLatin1String(".xpt8")))
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
	// set max. number of lines to preview by setting endRow
	if (endRow == -1 || endRow > startRow + lines - 1)
		endRow = startRow + lines - 1;

	varNames.clear();
	columnModes.clear();
	dataStrings.clear();

#ifdef HAVE_READSTAT
	readstat_error_t error = parse(fileName, true); // lines?

	if (error == READSTAT_OK) {
		DEBUG(Q_FUNC_INFO << ", var count = " << m_varCount)
		QDEBUG(Q_FUNC_INFO << ", var names = " << varNames)
		for (int i = 0; i < columnModes.size(); i++)
			DEBUG(Q_FUNC_INFO << ", column mode " << i << " = " << ENUM_TO_STRING(AbstractColumn, ColumnMode, columnModes.at(i)))
		DEBUG(Q_FUNC_INFO << ", read " << dataStrings.size() << " lines")
	} else {
		DEBUG(Q_FUNC_INFO << ", ERROR: processing " << qPrintable(fileName))
	}
#else
	Q_UNUSED(fileName)
#endif

	return dataStrings;
}

/*!
	reads the content of file \c fileName to the data source \c dataSource.
	Uses the settings defined in the data source.
*/
void ReadStatFilterPrivate::readDataFromFile(const QString& fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode mode) {
	DEBUG(Q_FUNC_INFO << ", fileName = \'" << STDSTRING(fileName) << "\', dataSource = " << dataSource
					  << ", mode = " << ENUM_TO_STRING(AbstractFileFilter, ImportMode, mode));

	varNames.clear();
	columnModes.clear();
	dataStrings.clear();
	m_valueLabels.clear();
	m_notes.clear();

#ifdef HAVE_READSTAT
	DEBUG(Q_FUNC_INFO << ", Parsing meta data ...")
	// parse meta data and column modes only
	readstat_error_t error = parse(fileName, false, true);
	if (error != READSTAT_OK) {
		DEBUG(Q_FUNC_INFO << ", ERROR preparsing file " << STDSTRING(fileName))
		return;
	}

	DEBUG(Q_FUNC_INFO << ", found " << m_varCount << " cols, " << m_rowCount << " rows")

	// prepare data container
	const int actualEndRow = (endRow == -1 || endRow > m_rowCount) ? m_rowCount : endRow;
	const int actualRows = actualEndRow - startRow + 1;
	const int actualEndColumn = (endColumn == -1 || endColumn > m_varCount) ? m_varCount : endColumn;
	const int actualCols = actualEndColumn - startColumn + 1;
	DEBUG(Q_FUNC_INFO << ", actual cols/rows = " << actualCols << " / " << actualRows)
	const int columnOffset = dataSource->prepareImport(m_dataContainer, mode, actualRows, actualCols, varNames, columnModes);

	error = parse(fileName);
	if (error != READSTAT_OK) {
		DEBUG(Q_FUNC_INFO << ", ERROR parsing file " << STDSTRING(fileName))
		return;
	}

	DEBUG(Q_FUNC_INFO << ", column offset = " << columnOffset << " start/end column = " << startColumn << " / " << actualEndColumn)
	dataSource->finalizeImport(columnOffset, startColumn, actualEndColumn, QString(), mode);

	// value labels
	for (const auto& label : m_valueLabels)
		if (label.size() > 0)
			QDEBUG(Q_FUNC_INFO << ", label " << label << ", label values = " << m_labelSets[label].labels())

	QVector<Column*> columnList = dataSource->children<Column>();
	for (int i = 0; i < columnList.size(); i++) {
		auto* column = columnList.at(i);
		const QString label = m_valueLabels.at(i);
		if (column && label.size() > 0) {
			const auto columnMode = columnModes.at(i);
			const auto valueLabels = m_labelSets[label].labels();
			switch (columnMode) {
			case AbstractColumn::ColumnMode::Text:
				for (int j = 0; j < valueLabels.size(); j++) {
					DEBUG(Q_FUNC_INFO << ", column " << i << ": add string value label: " << STDSTRING(m_labelSets[label].valueString(j)) << " = "
									  << STDSTRING(valueLabels.at(j)))
					column->addValueLabel(m_labelSets[label].valueString(j), valueLabels.at(j));
				}
				break;
			case AbstractColumn::ColumnMode::Double:
				for (int j = 0; j < valueLabels.size(); j++) {
					DEBUG(Q_FUNC_INFO << ", column " << i << ": add double value label: " << m_labelSets[label].valueDouble(j) << " = "
									  << STDSTRING(valueLabels.at(j)))
					column->addValueLabel(m_labelSets[label].valueDouble(j), valueLabels.at(j));
				}
				break;
			case AbstractColumn::ColumnMode::Integer:
			case AbstractColumn::ColumnMode::BigInt:
				for (int j = 0; j < valueLabels.size(); j++) {
					DEBUG(Q_FUNC_INFO << ", column " << i << ": add integer value label: " << m_labelSets[label].valueInt(j) << " = "
									  << STDSTRING(valueLabels.at(j)))
					column->addValueLabel(m_labelSets[label].valueInt(j), valueLabels.at(j));
				}
				break;
			case AbstractColumn::ColumnMode::Month:
			case AbstractColumn::ColumnMode::Day:
			case AbstractColumn::ColumnMode::DateTime:
				// not support by readstat
				break;
			}
		}
	}

	dataSource->setComment(m_notes.join(QLatin1Char('\n')));
#endif
}

/*!
	writes the content of \c dataSource to the file \c fileName.
*/
void ReadStatFilterPrivate::write(const QString& /*fileName*/, AbstractDataSource* /*dataSource*/) {
	// TODO: writing ReadStat files not implemented yet
}

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################

/*!
  Saves as XML.
 */
void ReadStatFilter::save(QXmlStreamWriter* writer) const {
	writer->writeStartElement(QStringLiteral("readstatFilter"));
	writer->writeEndElement();
}

/*!
  Loads from XML.
*/
bool ReadStatFilter::load(XmlStreamReader*) {
	return true;
}
