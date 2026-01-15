/*
	File                 : JsonFilter.cpp
	Project              : LabPlot
	Description          : JSON I/O-filter.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2018 Andrey Cygankov <craftplace.ms@gmail.com>
	SPDX-FileCopyrightText: 2018-2025 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2018-2020 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "backend/datasources/filters/JsonFilter.h"
#include "backend/core/column/Column.h"
#include "backend/datasources/filters/JsonFilterPrivate.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/trace.h"
#include "backend/spreadsheet/Spreadsheet.h"

#include <KCompressionDevice>
#include <KLocalizedString>

#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

/*!
	\class JsonFilter
	\brief Manages the import/export of data from/to a file formatted using JSON.

	\ingroup datasources
*/
JsonFilter::JsonFilter()
	: AbstractFileFilter(FileType::JSON)
	, d(new JsonFilterPrivate(this)) {
}

JsonFilter::~JsonFilter() = default;

/*!
 * reads the content of the device \c device.
 */
void JsonFilter::readDataFromDevice(QIODevice& device, AbstractDataSource* dataSource, ImportMode importMode, int lines) {
	d->readDataFromDevice(device, dataSource, importMode, lines);
}

/*!
 * reads the content of the file \c fileName.
 */
void JsonFilter::readDataFromFile(const QString& fileName, AbstractDataSource* dataSource, ImportMode importMode) {
	d->readDataFromFile(fileName, dataSource, importMode);
}

QVector<QStringList> JsonFilter::preview(const QString& fileName, int lines) {
	return d->preview(fileName, lines);
}

QVector<QStringList> JsonFilter::preview(QIODevice& device, int lines) {
	return d->preview(device, lines);
}

/*!
 * writes the content of the data source \c dataSource to the file \c fileName.
 */
void JsonFilter::write(const QString& fileName, AbstractDataSource* dataSource) {
	d->write(fileName, dataSource);
}

/*!
 * returns the list of all predefined data types.
 */
QStringList JsonFilter::dataTypes() {
	const QMetaObject& mo = AbstractColumn::staticMetaObject;
	const QMetaEnum& me = mo.enumerator(mo.indexOfEnumerator("ColumnMode"));
	QStringList list;
	for (int i = 0; i <= 100; ++i) // me.keyCount() does not work because we have holes in enum
		if (me.valueToKey(i))
			list << QLatin1String(me.valueToKey(i));
	return list;
}

/*!
 * returns the list of all predefined data row types.
 */
QStringList JsonFilter::dataRowTypes() {
	return (QStringList() << QStringLiteral("Array") << QStringLiteral("Object"));
}

void JsonFilter::setDataRowType(QJsonValue::Type type) {
	d->rowType = type;
}
QJsonValue::Type JsonFilter::dataRowType() const {
	return d->rowType;
}

void JsonFilter::setModel(QJsonModel* model) {
	d->model = model;
}

void JsonFilter::setModelRows(const QVector<int>& rows) {
	d->modelRows = rows;
}

QVector<int> JsonFilter::modelRows() const {
	return d->modelRows;
}

void JsonFilter::setDateTimeFormat(const QString& f) {
	d->dateTimeFormat = f;
}
QString JsonFilter::dateTimeFormat() const {
	return d->dateTimeFormat;
}

void JsonFilter::setNumberFormat(QLocale::Language lang) {
	d->numberFormat = lang;
}
QLocale::Language JsonFilter::numberFormat() const {
	return d->numberFormat;
}

void JsonFilter::setNaNValueToZero(bool b) {
	d->nanValue = (b ? 0 : NAN);
}
bool JsonFilter::NaNValueToZeroEnabled() const {
	if (d->nanValue == 0)
		return true;
	return false;
}

void JsonFilter::setCreateIndexEnabled(bool b) {
	d->createIndexEnabled = b;
}

void JsonFilter::setImportObjectNames(bool b) {
	d->importObjectNames = b;
}

QStringList JsonFilter::vectorNames() const {
	return d->vectorNames;
}

QVector<AbstractColumn::ColumnMode> JsonFilter::columnModes() {
	return d->columnModes;
}

void JsonFilter::setStartRow(const int r) {
	d->startRow = r;
}
int JsonFilter::startRow() const {
	return d->startRow;
}

void JsonFilter::setEndRow(const int r) {
	d->endRow = r;
}
int JsonFilter::endRow() const {
	return d->endRow;
}

void JsonFilter::setStartColumn(const int c) {
	d->startColumn = c;
}
int JsonFilter::startColumn() const {
	return d->startColumn;
}

void JsonFilter::setEndColumn(const int c) {
	d->endColumn = c;
}
int JsonFilter::endColumn() const {
	return d->endColumn;
}

QString JsonFilter::fileInfoString(const QString& fileName) {
	DEBUG(Q_FUNC_INFO);

	KCompressionDevice device(fileName);

	if (!device.open(QIODevice::ReadOnly))
		return i18n("Open device failed");

	if (device.atEnd() && !device.isSequential())
		return i18n("Empty file");

	QJsonParseError err;
	QJsonDocument doc = QJsonDocument::fromJson(device.readAll(), &err);

	if (err.error != QJsonParseError::NoError || doc.isEmpty())
		return i18n("Parse error: %1 at offset %2", err.errorString(), err.offset);

	QString info;
	info += i18n("Valid JSON document");

	// TODO: get number of object, etc.
	// if (prepareDocumentToRead(doc) != 0)
	//	return info;

	// reset to start of file
	if (!device.isSequential())
		device.seek(0);

	return info;
}

// #####################################################################
// ################### Private implementation ##########################
// #####################################################################
JsonFilterPrivate::JsonFilterPrivate(JsonFilter* owner)
	: q(owner) {
}

/*!
 * returns -1 if a parse error has occurred, 1 if the current row type not supported and 0 otherwise.
 * returns \c true if successful, \c false otherwise.
 */
bool JsonFilterPrivate::parseColumnModes(const QJsonValue& row, const QString& rowName) {
	columnModes.clear();
	vectorNames.clear();

	// add index column if required
	if (createIndexEnabled) {
		columnModes << AbstractColumn::ColumnMode::Integer;
		vectorNames << i18n("index");
	}

	// add column for object names if required
	if (importObjectNames) {
		const auto mode = AbstractFileFilter::columnMode(rowName, dateTimeFormat, numberFormat);
		columnModes << mode;
		if (mode == AbstractColumn::ColumnMode::DateTime)
			vectorNames << i18n("timestamp");
		else if (mode == AbstractColumn::ColumnMode::Month)
			vectorNames << i18n("month");
		else if (mode == AbstractColumn::ColumnMode::Day)
			vectorNames << i18n("day");
		else
			vectorNames << i18n("name");
	}

	// determine the column modes and names
	for (int i = startColumn - 1; i < endColumn; ++i) {
		QJsonValue columnValue;
		if (row.isArray()) {
			columnValue = *(row.toArray().begin() + i);
			vectorNames << i18n("Column %1", QString::number(i + 1));
		} else if (row.isObject()) {
			const auto& keys = row.toObject().keys();
			if (i >= keys.count())
				return false;
			QString key = keys.at(i);
			vectorNames << key;
			columnValue = row.toObject().value(key);
		} else {
			columnValue = row;
			vectorNames << rowName;
		}

		switch (columnValue.type()) {
		case QJsonValue::Double:
			columnModes << AbstractColumn::ColumnMode::Double;
			break;
		case QJsonValue::String:
			columnModes << AbstractFileFilter::columnMode(columnValue.toString(), dateTimeFormat, numberFormat);
			break;
		case QJsonValue::Array:
		case QJsonValue::Object:
		case QJsonValue::Bool:
		case QJsonValue::Null:
		case QJsonValue::Undefined:
			return false;
		}
	}

	return true;
}

void JsonFilterPrivate::setEmptyValue(int column, int row) {
	switch (columnModes.at(column)) {
	case AbstractColumn::ColumnMode::Double:
		static_cast<QVector<double>*>(m_dataContainer[column])->operator[](row) = nanValue;
		break;
	case AbstractColumn::ColumnMode::Integer:
		static_cast<QVector<int>*>(m_dataContainer[column])->operator[](row) = 0;
		break;
	case AbstractColumn::ColumnMode::BigInt:
		static_cast<QVector<qint64>*>(m_dataContainer[column])->operator[](row) = 0;
		break;
	case AbstractColumn::ColumnMode::DateTime:
		static_cast<QVector<QDateTime>*>(m_dataContainer[column])->operator[](row) = QDateTime();
		break;
	case AbstractColumn::ColumnMode::Text:
		static_cast<QVector<QString>*>(m_dataContainer[column])->operator[](row) = QString();
		break;
	case AbstractColumn::ColumnMode::Month:
	case AbstractColumn::ColumnMode::Day:
		break;
	}
}

void JsonFilterPrivate::setValueFromString(int column, int row, const QString& valueString) {
	QLocale locale(numberFormat);
	switch (columnModes.at(column)) {
	case AbstractColumn::ColumnMode::Double: {
		bool isNumber;
		const double value = locale.toDouble(valueString, &isNumber);
		static_cast<QVector<double>*>(m_dataContainer[column])->operator[](row) = isNumber ? value : nanValue;
		break;
	}
	case AbstractColumn::ColumnMode::Integer: {
		bool isNumber;
		const int value = locale.toInt(valueString, &isNumber);
		static_cast<QVector<int>*>(m_dataContainer[column])->operator[](row) = isNumber ? value : 0;
		break;
	}
	case AbstractColumn::ColumnMode::BigInt: {
		bool isNumber;
		const qint64 value = locale.toLongLong(valueString, &isNumber);
		static_cast<QVector<qint64>*>(m_dataContainer[column])->operator[](row) = isNumber ? value : 0;
		break;
	}
	case AbstractColumn::ColumnMode::DateTime: {
		const QDateTime valueDateTime = QDateTime::fromString(valueString, dateTimeFormat);
		static_cast<QVector<QDateTime>*>(m_dataContainer[column])->operator[](row) = valueDateTime.isValid() ? valueDateTime : QDateTime();
		break;
	}
	case AbstractColumn::ColumnMode::Text:
		static_cast<QVector<QString>*>(m_dataContainer[column])->operator[](row) = valueString;
		break;
	case AbstractColumn::ColumnMode::Month:
	case AbstractColumn::ColumnMode::Day:
		break;
	}
}

/*!
 * prepare device/file for reading
 */
int JsonFilterPrivate::prepareDeviceToRead(QIODevice& device) {
	DEBUG(Q_FUNC_INFO << ", device is sequential = " << device.isSequential());

	if (!device.open(QIODevice::ReadOnly)) {
		q->setLastError(i18n("Failed to open the device/file."));
		return -1;
	}

	if (device.atEnd() && !device.isSequential()) { // empty file
		q->setLastError(i18n("Device/file is empty."));
		return 1;
	}

	QJsonParseError err;
	m_doc = QJsonDocument::fromJson(device.readAll(), &err);

	if (err.error != QJsonParseError::NoError || m_doc.isEmpty()) {
		q->setLastError(i18n("JSON format error or document empty."));
		return 1;
	}

	// reset to start of file
	if (!device.isSequential())
		device.seek(0);

	return 0;
}

/*!
 * determines the relevant part of the full JSON document to be read and its structure.
 * returns \c true if successful, \c false otherwise.
 */
bool JsonFilterPrivate::prepareDocumentToRead() {
	PERFTRACE(QStringLiteral("Prepare the JSON document to read"));

	QString firstRowName; // the name of the first selected row in the JSON model
	if (modelRows.isEmpty())
		m_preparedDoc = m_doc;
	else {
		if (modelRows.size() == 1)
			m_preparedDoc = m_doc; // root element selected, use the full document
		else {
			// when running tests there is no ImportFileWidget and JsonOptionsWidget available
			// where the model is created and also passed to JsonFilter. So, we need to create
			// a model here for in this case.
			if (!model) {
				model = new QJsonModel();
				model->loadJson(m_doc);
			}

			QModelIndex index;
			for (auto& it : modelRows)
				index = model->index(it, 0, index);

			if (index.isValid()) {
				// the key/name of the structure in the JSON to be parsed
				const auto* item = static_cast<QJsonTreeItem*>(index.internalPointer());
				firstRowName = item->key();

				// the part of the JSON document to be parsed
				m_preparedDoc = model->genJsonByIndex(index);
			}
		}
	}

	if (!m_preparedDoc.isEmpty()) {
		if (m_preparedDoc.isArray())
			containerType = JsonFilter::DataContainerType::Array;
		else if (m_preparedDoc.isObject())
			containerType = JsonFilter::DataContainerType::Object;
		else
			return false;
	} else {
		DEBUG("Empty document.");
		return false;
	}

	int countRows = 0;
	int countCols = -1;
	QJsonValue firstRow; // first row/element within the selected index in the JSON model
	importObjectNames = (importObjectNames && (rowType == QJsonValue::Object));

	switch (containerType) {
	case JsonFilter::DataContainerType::Array: {
		QJsonArray arr = m_preparedDoc.array();
		int count = arr.count();

		if (count < startRow)
			return false;

		const int endRowOffset = (endRow == -1 || endRow > count) ? count : endRow;
		firstRow = *(arr.begin() + (startRow - 1));

		// determine the number of columns to be imported
		if (!firstRow.isArray()) // array of plain data types
			countCols = 1;
		else { // array of arrays
			// check the number of elements in the first element of the array
			count = firstRow.toArray().count();
			countCols = (countCols == -1 || countCols > count) ? count : countCols;
		}

		countRows = endRowOffset - startRow + 1;
		break;
	}
	case JsonFilter::DataContainerType::Object: {
		QJsonObject obj = m_preparedDoc.object();

		if (obj.count() < startRow)
			return false;

		const int startRowOffset = startRow - 1;
		const int endRowOffset = (endRow == -1 || endRow > obj.count()) ? obj.count() : endRow;
		firstRow = *(obj.begin() + startRowOffset);
		firstRowName = (obj.begin() + startRowOffset).key();

		// determine the number of columns to be imported
		QJsonObject row = firstRow.toObject();
		if (row.isEmpty())
			break;
		countCols = (countCols == -1 || countCols > row.count()) ? row.count() : countCols;

		countRows = endRowOffset - startRow + 1;
		break;
	}
	}

	if (endColumn == -1 || endColumn > countCols)
		endColumn = countCols;

	m_actualRows = countRows;
	m_actualCols = endColumn - startColumn + 1 + createIndexEnabled + importObjectNames;

	DEBUG("start/end column: = " << startColumn << ' ' << endColumn);
	DEBUG("start/end rows = " << startRow << ' ' << endRow);
	DEBUG("actual cols/rows = " << m_actualCols << ' ' << m_actualRows);

	if (!parseColumnModes(firstRow, firstRowName)) {
		DEBUG("failed to parse the column modes");
		return false;
	}

	return true;
}

/*!
 * reads the content of the file \c fileName to the data source \c dataSource.
 */
void JsonFilterPrivate::readDataFromFile(const QString& fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode) {
	KCompressionDevice device(fileName);
	readDataFromDevice(device, dataSource, importMode);
}

/*!
 * reads the content of device \c device to the data source \c dataSource.
 */
void JsonFilterPrivate::readDataFromDevice(QIODevice& device, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode, int lines) {
	if (!m_prepared) {
		const int deviceError = prepareDeviceToRead(device);
		if (deviceError != 0) {
			q->setLastError(i18n("Empty file or invalid JSON document."));
			return;
		}
		m_prepared = true;
	}

	if (prepareDocumentToRead())
		importData(dataSource, importMode, lines);
}

/*!
 * import the content of document \c m_preparedDoc to the data source \c dataSource.
 */
void JsonFilterPrivate::importData(AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode, int lines) {
	bool ok = false;
	m_columnOffset = dataSource->prepareImport(m_dataContainer, importMode, m_actualRows, m_actualCols, vectorNames, columnModes, ok);
	if (!ok) {
		q->setLastError(i18n("Not enough memory."));
		return;
	}

	int rowOffset = startRow - 1;
	int colOffset = (int)createIndexEnabled + (int)importObjectNames;
	DEBUG("reading " << m_actualRows << " lines");
	DEBUG("reading " << m_actualCols << " columns");

	int progressIndex = 0;
	const float progressInterval = 0.01 * lines; // update on every 1% only

	const auto& array = m_preparedDoc.array();
	const auto& arrayIterator = array.begin();
	const auto& object = m_preparedDoc.object();
	const auto& objectIterator = object.begin();

	for (int i = 0; i < m_actualRows; ++i) {
		if (createIndexEnabled)
			static_cast<QVector<int>*>(m_dataContainer[0])->operator[](i) = i + 1;

		QJsonValue row;
		switch (containerType) {
		case JsonFilter::DataContainerType::Array:
			row = *(arrayIterator + rowOffset + i);
			break;
		case JsonFilter::DataContainerType::Object:
			if (importObjectNames) {
				const QString& rowName = (objectIterator + rowOffset + i).key();
				setValueFromString((int)createIndexEnabled, i, rowName);
			}
			row = *(objectIterator + rowOffset + i);
			break;
		}

		for (int n = 0; n < m_actualCols - colOffset; ++n) {
			QJsonValue value;
			if (row.isArray())
				value = *(row.toArray().begin() + n + startColumn - 1);
			else if (row.isObject())
				value = *(row.toObject().begin() + n + startColumn - 1);
			else
				value = row;

			switch (value.type()) {
			case QJsonValue::Double:
				if (columnModes[colOffset + n] == AbstractColumn::ColumnMode::Double)
					static_cast<QVector<double>*>(m_dataContainer[colOffset + n])->operator[](i) = value.toDouble();
				else
					setEmptyValue(colOffset + n, i + startRow - 1);
				break;
			case QJsonValue::String:
				setValueFromString(colOffset + n, i, value.toString());
				break;
			case QJsonValue::Array:
			case QJsonValue::Object:
			case QJsonValue::Bool:
			case QJsonValue::Null:
			case QJsonValue::Undefined:
				setEmptyValue(colOffset + n, i + startRow - 1);
				break;
			}
		}

		// ask to update the progress bar only if we have more than 1000 lines
		// only in 1% steps
		progressIndex++;
		if (m_actualRows > 1000 && progressIndex > progressInterval) {
			double value = 100. * i / lines;
			Q_EMIT q->completed(static_cast<int>(value));
			progressIndex = 0;
			QApplication::processEvents(QEventLoop::AllEvents, 0);
		}
	}

	// set the plot designation to 'X' for index and name columns, if available
	auto* spreadsheet = dynamic_cast<Spreadsheet*>(dataSource);
	if (spreadsheet) {
		if (createIndexEnabled)
			spreadsheet->column(m_columnOffset)->setPlotDesignation(AbstractColumn::PlotDesignation::X);
		if (importObjectNames)
			spreadsheet->column(m_columnOffset + (int)createIndexEnabled)->setPlotDesignation(AbstractColumn::PlotDesignation::X);
	}

	dataSource->finalizeImport(m_columnOffset, startColumn, startColumn + m_actualCols - 1, dateTimeFormat, importMode);
}

/*!
 * generates the preview for the file \c fileName.
 */
QVector<QStringList> JsonFilterPrivate::preview(const QString& fileName, int lines) {
	if (!m_prepared) {
		KCompressionDevice device(fileName);
		return preview(device, lines);
	} else
		return preview(lines);
}

/*!
 * generates the preview for device \c device.
 */
QVector<QStringList> JsonFilterPrivate::preview(QIODevice& device, int lines) {
	if (!m_prepared) {
		const int deviceError = prepareDeviceToRead(device);
		if (deviceError != 0) {
			DEBUG("Device error = " << deviceError);
			return {};
		}
	}

	if (prepareDocumentToRead())
		return preview(lines);

	return {};
}

/*!
 * generates the preview for document \c m_preparedDoc.
 */
QVector<QStringList> JsonFilterPrivate::preview(int lines) {
	QVector<QStringList> dataStrings;
	const int rowOffset = startRow - 1;
	DEBUG("	Generating preview for " << std::min(lines, m_actualRows) << " lines");

	const auto& array = m_preparedDoc.array();
	const auto& arrayIterator = array.begin();
	const auto& object = m_preparedDoc.object();
	const auto& objectIterator = object.begin();

	for (int i = 0; i < std::min(lines, m_actualRows); ++i) {
		QString rowName;
		QJsonValue row;
		switch (containerType) {
		case JsonFilter::DataContainerType::Object:
			rowName = (objectIterator + rowOffset + i).key();
			row = *(objectIterator + rowOffset + i);
			break;
		case JsonFilter::DataContainerType::Array:
			row = *(arrayIterator + rowOffset + i);
			break;
		}

		QStringList lineString;
		if (createIndexEnabled)
			lineString += QString::number(i + 1);
		if (importObjectNames)
			lineString += rowName;

		for (int n = startColumn - 1; n < endColumn; ++n) {
			QJsonValue value;
			if (row.isObject())
				value = *(row.toObject().begin() + n);
			else if (row.isArray())
				value = *(row.toArray().begin() + n);
			else
				value = row;

			switch (value.type()) {
			case QJsonValue::Double:
				lineString += QString::number(value.toDouble(), 'g', 16);
				break;
			case QJsonValue::String:
				lineString += value.toString();
				break;
			case QJsonValue::Array:
			case QJsonValue::Object:
			case QJsonValue::Bool:
			case QJsonValue::Null:
			case QJsonValue::Undefined:
				lineString += QString();
				break;
			}
		}
		dataStrings << lineString;
	}
	return dataStrings;
}

/*!
 * writes the content of \c dataSource to the file \c fileName.
 */
void JsonFilterPrivate::write(const QString& /*fileName*/, AbstractDataSource* /*dataSource*/) {
	// TODO: saving data to json file not supported yet
}

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################
/*!
 * Saves as XML.
 */
void JsonFilter::save(QXmlStreamWriter* writer) const {
	writer->writeStartElement(QStringLiteral("jsonFilter"));
	writer->writeAttribute(QStringLiteral("rowType"), QString::number(d->rowType));
	writer->writeAttribute(QStringLiteral("dateTimeFormat"), d->dateTimeFormat);
	writer->writeAttribute(QStringLiteral("numberFormat"), QString::number(d->numberFormat));
	writer->writeAttribute(QStringLiteral("createIndex"), QString::number(d->createIndexEnabled));
	writer->writeAttribute(QStringLiteral("importObjectNames"), QString::number(d->importObjectNames));
	writer->writeAttribute(QStringLiteral("nanValue"), QString::number(d->nanValue));
	writer->writeAttribute(QStringLiteral("startRow"), QString::number(d->startRow));
	writer->writeAttribute(QStringLiteral("endRow"), QString::number(d->endRow));
	writer->writeAttribute(QStringLiteral("startColumn"), QString::number(d->startColumn));
	writer->writeAttribute(QStringLiteral("endColumn"), QString::number(d->endColumn));

	QStringList list;
	for (const auto& it : modelRows())
		list.append(QString::number(it));

	writer->writeAttribute(QStringLiteral("modelRows"), list.join(QLatin1Char(';')));

	writer->writeEndElement();
}

/*!
 * Loads from XML.
 */
bool JsonFilter::load(XmlStreamReader* reader) {
	const auto& attribs = reader->attributes();
	QString str;

	READ_INT_VALUE("rowType", rowType, QJsonValue::Type);
	READ_STRING_VALUE("dateTimeFormat", dateTimeFormat);
	READ_INT_VALUE("numberFormat", numberFormat, QLocale::Language);
	READ_INT_VALUE("createIndex", createIndexEnabled, bool);
	READ_INT_VALUE("importObjectNames", importObjectNames, bool);
	READ_DOUBLE_VALUE("nanValue", nanValue);
	READ_INT_VALUE("startRow", startRow, int);
	READ_INT_VALUE("endRow", endRow, int);
	READ_INT_VALUE("startColumn", startColumn, int);
	READ_INT_VALUE("endColumn", endColumn, int);

	const auto& list = attribs.value(QStringLiteral("modelRows")).toString().split(QLatin1Char(';'));
	if (list.isEmpty())
		reader->raiseMissingAttributeWarning(QStringLiteral("'modelRows'"));
	else {
		d->modelRows = QVector<int>();
		for (auto& it : list)
			d->modelRows.append(it.toInt());
	}

	return true;
}
