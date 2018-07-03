#include "backend/datasources/filters/JsonFilter.h"
#include "backend/datasources/filters/JsonFilterPrivate.h"
#include "backend/datasources/AbstractDataSource.h"
#include "backend/core/column/Column.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDataStream>
#include <QDateTime>
#include <KLocalizedString>
#include <KFilterDev>

/*!
\class JsonFilter
\brief Manages the import/export of data from/to a file formatted using JSON.

\ingroup datasources
*/
JsonFilter::JsonFilter() : AbstractFileFilter(), d(new JsonFilterPrivate(this)) {}

JsonFilter::~JsonFilter() {}

/*!
reads the content of the device \c device.
*/
void JsonFilter::readDataFromDevice(QIODevice& device, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode, int lines) {
	d->readDataFromDevice(device, dataSource, importMode, lines);
}

/*!
reads the content of the file \c fileName.
*/
QVector<QStringList> JsonFilter::readDataFromFile(const QString& fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode, int lines) {
	d->readDataFromFile(fileName, dataSource, importMode, lines);
	return QVector<QStringList>();  //TODO: remove this later once all read*-functions in the filter classes don't return any preview strings anymore
}

QVector<QStringList> JsonFilter::preview(const QString& fileName) {
	return d->preview(fileName);
}

QVector<QStringList> JsonFilter::preview(QIODevice& device) {
	return d->preview(device);
}

QVector<QStringList> JsonFilter::preview(QJsonDocument& doc) {
	return d->preview(doc);
}

/*!
writes the content of the data source \c dataSource to the file \c fileName.
*/
void JsonFilter::write(const QString& fileName, AbstractDataSource* dataSource) {
	d->write(fileName, dataSource);
}

///////////////////////////////////////////////////////////////////////
/*!
loads the predefined filter settings for \c filterName
*/
void JsonFilter::loadFilterSettings(const QString& filterName) {
	Q_UNUSED(filterName);
}

/*!
saves the current settings as a new filter with the name \c filterName
*/
void JsonFilter::saveFilterSettings(const QString& filterName) const {
	Q_UNUSED(filterName);
}

/*!
returns the list of all predefined data types.
*/
QStringList JsonFilter::dataTypes() {
	const QMetaObject& mo = AbstractColumn::staticMetaObject;
	const QMetaEnum& me = mo.enumerator(mo.indexOfEnumerator("ColumnMode"));
	QStringList list;
	for (int i = 0; i <= 100; ++i)	// me.keyCount() does not work because we have holes in enum
		if (me.valueToKey(i))
			list << me.valueToKey(i);
		return list;
}

/*!
returns the list of all predefined data row types.
*/
QStringList JsonFilter::dataRowTypes() {
	return (QStringList() << "Array" << "Object");
}

void JsonFilter::setDataRowType(QJsonValue::Type type) {
	d->rowType = type;
}
QJsonValue::Type JsonFilter::dataRowType() const {
	return d->rowType;
}

void JsonFilter::setModelRows(QVector<int> rows){
	d->modelRows = rows;
}

QVector<int> JsonFilter::modelRows() const {
	return d->modelRows;
}

void JsonFilter::setDateTimeFormat(const QString &f) {
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
	if (b)
		d->nanValue = 0;
	else
		d->nanValue = NAN;
}
bool JsonFilter::NaNValueToZeroEnabled() const {
	if (d->nanValue == 0)
		return true;
	return false;
}

void JsonFilter::setCreateIndexEnabled(bool b){
	d->createIndexEnabled = b;
}

void JsonFilter::setParseRowsName(bool b) {
	d->parseRowsName = b;
}

void JsonFilter::setVectorNames(const QString& s) {
	d->vectorNames.clear();
	if (!s.simplified().isEmpty())
		d->vectorNames = s.simplified().split(' ');
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

//#####################################################################
//################### Private implementation ##########################
//#####################################################################
JsonFilterPrivate::JsonFilterPrivate(JsonFilter* owner) : q(owner),
	model(new QJsonModel()),
	containerType(JsonFilter::Object),
	rowType(QJsonValue::Object),
	numberFormat(QLocale::C),
	createIndexEnabled(false),
	parseRowsName(false),
	vectorNames(),
	startRow(1),
	endRow(-1),
	startColumn(1),
	endColumn(-1),
	m_actualRows(0),
	m_actualCols(0),
	m_prepared(false),
	m_columnOffset(0) {
}
//TODO: delete model from memory

/*!
returns 1 if row is invalid and 0 otherwise.
*/
int JsonFilterPrivate::checkRow(QJsonValueRef value, int& countCols) {
	switch(rowType){
		//TODO: implement other value types
		case QJsonValue::Array: {
			QJsonArray row = value.toArray();
			if(row.isEmpty())
				return 1;
			countCols = (countCols == -1 || countCols > row.count()) ? row.count() : countCols;
			break;
		}
		case QJsonValue::Object: {
			QJsonObject row = value.toObject();
			if(row.isEmpty())
				return 1;
			countCols = (countCols == -1 || countCols > row.count()) ? row.count() : countCols;
			break;
		}
		case QJsonValue::Double:
		case QJsonValue::String:
		case QJsonValue::Bool:
		case QJsonValue::Null:
		case QJsonValue::Undefined:
			return 1;
	}
	return 0;
}

/*!
returns -1 if a parse error has occurred, 1 if the current row type not supported and 0 otherwise.
*/
int JsonFilterPrivate::parseColumnModes(QJsonValue row, QString rowName) {
	columnModes.resize(m_actualCols);

	int colIndexInContainer = startColumn - 1;
	for(int i = 0; i < m_actualCols; ++i){
		if((createIndexEnabled || parseRowsName) && i == 0){
			if(createIndexEnabled)
				columnModes[i] = AbstractColumn::Integer;
			if(parseRowsName)
				columnModes[i + createIndexEnabled] = AbstractFileFilter::columnMode(rowName, dateTimeFormat, numberFormat);
			i = i + createIndexEnabled + parseRowsName - 1;
			continue;
		}

		QJsonValue columnValue;
		switch (rowType) {
			//TODO: implement other value types
			case QJsonValue::Array: {
				QJsonArray arr = row.toArray();
				if(arr.count() < colIndexInContainer + 1)
					return -1;
				columnValue = *(row.toArray().begin() + colIndexInContainer);
				break;
			}
			case QJsonValue::Object: {
				QJsonObject obj = row.toObject();
				if(obj.count() < colIndexInContainer + 1)
					return -1;
				if(vectorNames.count() == 0)
					vectorNames = row.toObject().keys();
				columnValue = *(row.toObject().begin() + colIndexInContainer);
				break;
			}
			case QJsonValue::Double:
			case QJsonValue::String:
			case QJsonValue::Bool:
			case QJsonValue::Null:
			case QJsonValue::Undefined:
				return 1;
		}

		switch (columnValue.type()) {
			case QJsonValue::Double:
				columnModes[i] = AbstractColumn::Numeric;
				break;
			case QJsonValue::String:
				columnModes[i] = AbstractFileFilter::columnMode(columnValue.toString(), dateTimeFormat, numberFormat);
				break;
			case QJsonValue::Array:
			case QJsonValue::Object:
			case QJsonValue::Bool:
			case QJsonValue::Null:
			case QJsonValue::Undefined:
				return -1;
		}
		colIndexInContainer++;
	}

	if(parseRowsName)
		vectorNames.prepend("row name");
	if(createIndexEnabled)
		vectorNames.prepend("index");

	return 0;
}

void JsonFilterPrivate::setEmptyValue(int column, int row){
	switch (columnModes[column]) {
		case AbstractColumn::Numeric:
			static_cast<QVector<double>*>(m_dataContainer[column])->operator[](row) = nanValue;
			break;
		case AbstractColumn::Integer:
			static_cast<QVector<int>*>(m_dataContainer[column])->operator[](row) = 0;
			break;
		case AbstractColumn::DateTime:
			static_cast<QVector<QDateTime>*>(m_dataContainer[column])->operator[](row) = QDateTime();
			break;
		case AbstractColumn::Text:
			static_cast<QVector<QString>*>(m_dataContainer[column])->operator[](row) = "";
			break;
		case AbstractColumn::Month:
		case AbstractColumn::Day:
			break;
	}
}

void JsonFilterPrivate::setValueFromString(int column, int row, QString valueString){
	QLocale locale(numberFormat);
	switch (columnModes[column]) {
		case AbstractColumn::Numeric: {
			bool isNumber;
			const double value = locale.toDouble(valueString, &isNumber);
			static_cast<QVector<double>*>(m_dataContainer[column])->operator[](row) = isNumber ? value : nanValue;
			break;
		}
		case AbstractColumn::Integer: {
			bool isNumber;
			const int value = locale.toInt(valueString, &isNumber);
			static_cast<QVector<int>*>(m_dataContainer[column])->operator[](row) = isNumber ? value : 0;
			break;
		}
		case AbstractColumn::DateTime: {
			const QDateTime valueDateTime = QDateTime::fromString(valueString, dateTimeFormat);
			static_cast<QVector<QDateTime>*>(m_dataContainer[column])->operator[](row) =
					valueDateTime.isValid() ? valueDateTime : QDateTime();
			break;
		}
		case AbstractColumn::Text:
			static_cast<QVector<QString>*>(m_dataContainer[column])->operator[](row) = valueString;
			break;
		case AbstractColumn::Month:
		case AbstractColumn::Day:
			break;
	}
}

/*!
returns -1 if the device couldn't be opened, 1 if the current read position in the device is at the end
*/
int JsonFilterPrivate::prepareDeviceToRead(QIODevice& device) {
	DEBUG("device is sequential = " << device.isSequential());

	if (!device.open(QIODevice::ReadOnly))
		return -1;

	if (device.atEnd() && !device.isSequential()) // empty file
		return 1;

	QJsonParseError err;
	QJsonDocument doc = QJsonDocument::fromJson(device.readAll(), &err);

	if(err.error != QJsonParseError::NoError || doc.isEmpty())
		return 1;

	if(prepareDocumentToRead(doc) != 0)
		return 2;
	// reset to start of file
	if (!device.isSequential())
		device.seek(0);

	return 0;
}

/*!
returns 2 if a parse error has occurred and 0 otherwise.
*/
int JsonFilterPrivate::prepareDocumentToRead(const QJsonDocument& doc) {
	model->loadJson(doc);

	if(modelRows.isEmpty())
		m_preparedDoc = doc;
	else {
		QModelIndex index;
		for(auto it = modelRows.begin(); it != modelRows.end(); ++it){
			index = model->index(*it, 0, index);
		}
		m_preparedDoc = model->genJsonByIndex(index);
	}

	if(!m_preparedDoc.isEmpty()){
		if(m_preparedDoc.isArray())
			containerType = JsonFilter::Array;
		else if(m_preparedDoc.isObject())
			containerType = JsonFilter::Object;
		else
			return 2;
	}
	else
		return 2;

	int countRows = 0;
	int countCols = -1;
	QJsonValue firstRow;
	QString firstRowName = "";
	parseRowsName = parseRowsName && rowType == QJsonValue::Object;

	switch(containerType) {
		case JsonFilter::Array: {
			QJsonArray arr = m_preparedDoc.array();

			if(arr.count() < startRow)
				return 2;

			int endRowOffset = (endRow == -1 || endRow > arr.count()) ? arr.count() : endRow;
			firstRow = *(arr.begin() + (startRow - 1));
			for(QJsonArray::iterator it = arr.begin() + (startRow - 1); it != arr.begin() + endRowOffset; ++it) {
				if(checkRow(*it, countCols) != 0)
					return 2;
				countRows++;
			}
			break;
		}
		case JsonFilter::Object: {
			QJsonObject obj = m_preparedDoc.object();

			if(obj.count() < startRow)
				return 2;

			int startRowOffset = startRow - 1;
			int endRowOffset = (endRow == -1 || endRow > obj.count()) ? obj.count() : endRow;
			firstRow = *(obj.begin() + startRowOffset);
			firstRowName = (obj.begin() + startRowOffset).key();
			for(QJsonObject::iterator it = obj.begin() + startRowOffset; it != obj.begin() + endRowOffset; ++it) {
				if(checkRow(*it, countCols) != 0)
					return 2;
				countRows++;
			}
			break;
		}
	}

	if(endColumn == -1 || endColumn > countCols)
		endColumn = countCols;

	m_actualRows = countRows;
	m_actualCols = endColumn - startColumn + 1 + createIndexEnabled + parseRowsName;

	if(parseColumnModes(firstRow, firstRowName) != 0)
		return 2;

	DEBUG("start/end column: = " << startColumn << ' ' << endColumn);
	DEBUG("start/end rows = " << startRow << ' ' << endRow);
	DEBUG("actual cols/rows = " << m_actualCols << ' ' << m_actualRows);

	return 0;
}

/*!
reads the content of the file \c fileName to the data source \c dataSource. Uses the settings defined in the data source.
*/
void JsonFilterPrivate::readDataFromFile(const QString& fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode, int lines) {
	 KFilterDev device(fileName);
	 readDataFromDevice(device, dataSource, importMode, lines);
}

/*!
reads the content of device \c device to the data source \c dataSource. Uses the settings defined in the data source.
*/
void JsonFilterPrivate::readDataFromDevice(QIODevice& device, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode, int lines) {
	if(!m_prepared) {
		const int deviceError = prepareDeviceToRead(device);
		if(deviceError != 0){
			DEBUG("Device error = " << deviceError);
			return;
		}
		//TODO: support other modes and vector names
		m_prepared = true;
	}
	importData(dataSource, importMode, lines);
}

/*!
reads the content of document \c doc to the data source \c dataSource. Uses the settings defined in the data source.
*/
void JsonFilterPrivate::readDataFromDocument(const QJsonDocument& doc, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode, int lines) {
	if(!m_prepared) {
		const int docError = prepareDocumentToRead(doc);
		if(docError != 0){
			DEBUG("Document parse error = " << docError);
			return;
		}
		//TODO: support other modes and vector names
		m_prepared = true;
	}
	importData(dataSource, importMode, lines);
}

/*!
import the content of document \c m_preparedDoc to the data source \c dataSource. Uses the settings defined in the data source.
*/
void JsonFilterPrivate::importData(AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode, int lines) {
	Q_UNUSED(lines)

	m_columnOffset = dataSource->prepareImport(m_dataContainer, importMode, m_actualRows, m_actualCols, vectorNames, columnModes);
	int rowOffset = startRow - 1;
	DEBUG("reading " << m_actualRows << " lines");
	for(int i = 0; i < m_actualRows; ++i) {
		QString rowName;
		QJsonValue row;
		switch (containerType) {
			case JsonFilter::Array:
				row = *(m_preparedDoc.array().begin() + rowOffset + i);
				break;
			case JsonFilter::Object:
				rowName = (m_preparedDoc.object().begin() + rowOffset + i).key();
				row = *(m_preparedDoc.object().begin() + rowOffset + i);
				break;
		}

		int colIndex = 0;
		for(int n = 0; n < m_actualCols; ++n) {
			if((createIndexEnabled || parseRowsName) && n == 0) {
				if(createIndexEnabled)
					static_cast<QVector<int>*>(m_dataContainer[n])->operator[](i) = i + 1;
				if(parseRowsName)
					setValueFromString(n + createIndexEnabled, i, rowName);
				n = n + createIndexEnabled + parseRowsName - 1;
				continue;
			}
			QJsonValue value;
			switch(rowType){
				//TODO: implement other value types
				case QJsonValue::Array: {
					value = *(row.toArray().begin() + colIndex);
					break;
				}
				case QJsonValue::Object: {
					value = *(row.toObject().begin() + colIndex);
					break;
				}
				case QJsonValue::Double:
				case QJsonValue::String:
				case QJsonValue::Bool:
				case QJsonValue::Null:
				case QJsonValue::Undefined:
					break;
			}

			switch(value.type()) {
				case QJsonValue::Double:
					if(columnModes[n] == AbstractColumn::Numeric)
						static_cast<QVector<double>*>(m_dataContainer[n])->operator[](i) = value.toDouble();
					else
						setEmptyValue(n, i + startRow - 1);
					break;
				case QJsonValue::String:
					setValueFromString(n, i, value.toString());
					break;
				case QJsonValue::Array:
				case QJsonValue::Object:
				case QJsonValue::Bool:
				case QJsonValue::Null:
				case QJsonValue::Undefined:
					setEmptyValue(n, i + startRow - 1);
					break;
			}
			colIndex++;
		}
		emit q->completed(100 * i/m_actualRows);
	}
	//TODO: fix (startColumn + m_actualCols - 1)
	dataSource->finalizeImport(m_columnOffset, startColumn, startColumn + m_actualCols - 1, dateTimeFormat, importMode);
}

/*!
generates the preview for the file \c fileName.
*/
QVector<QStringList> JsonFilterPrivate::preview(const QString& fileName) {
	KFilterDev device(fileName);
	return preview(device);
}

/*!
generates the preview for device \c device.
*/
QVector<QStringList> JsonFilterPrivate::preview(QIODevice &device) {
	const int deviceError = prepareDeviceToRead(device);
	if (deviceError != 0) {
		DEBUG("Device error = " << deviceError);
		return QVector<QStringList>();
	}

	return preview();
}

/*!
generates the preview for document \c doc.
*/
QVector<QStringList> JsonFilterPrivate::preview(QJsonDocument &doc) {
	if(prepareDocumentToRead(doc) != 0)
		return QVector<QStringList>();
	return preview();
}

/*!
generates the preview for document \c m_preparedDoc.
*/
QVector<QStringList> JsonFilterPrivate::preview() {
	QVector<QStringList> dataStrings;
	int rowOffset = startRow - 1;
	DEBUG("reading " << m_actualRows << " lines");
	for(int i = 0; i < m_actualRows; ++i) {
		QString rowName;
		QJsonValue row;
		switch (containerType) {
			case JsonFilter::Object:
				rowName = (m_preparedDoc.object().begin() + rowOffset + i).key();
				row = *(m_preparedDoc.object().begin() + rowOffset + i);
				break;
			case JsonFilter::Array:
				row = *(m_preparedDoc.array().begin() + rowOffset + i);
				break;
		}

		QStringList lineString;
		int colIndex = 0;
		for(int n = 0; n < m_actualCols; ++n) {
			if((createIndexEnabled || parseRowsName) && n == 0) {
				if(createIndexEnabled)
					lineString += QString::number(i + 1);
				if(parseRowsName)
					lineString += rowName;
				n = n + createIndexEnabled + parseRowsName - 1;
				continue;
			}

			QJsonValue value;
			switch(rowType){
				case QJsonValue::Object: {
					value = *(row.toObject().begin() + colIndex);
					break;
				}
				case QJsonValue::Array: {
					value = *(row.toArray().begin() + colIndex);
					break;
				}
					//TODO: implement other value types
				case QJsonValue::Double:
				case QJsonValue::String:
				case QJsonValue::Bool:
				case QJsonValue::Null:
				case QJsonValue::Undefined:
					break;
			}
			switch(value.type()) {
				case QJsonValue::Double:
					if(columnModes[n] == AbstractColumn::Numeric)
						lineString += QString::number(value.toDouble(), 'g', 16);
					else
						lineString += lineString += QLatin1String("");
					break;
				case QJsonValue::String: {
					//TODO: add parsing string before appending
					lineString += value.toString();
					break;
				}
				case QJsonValue::Array:
				case QJsonValue::Object:
				case QJsonValue::Bool:
				case QJsonValue::Null:
				case QJsonValue::Undefined:
					lineString += QLatin1String("");
					break;
			}
			colIndex++;
		}
		dataStrings << lineString;
		emit q->completed(100 * i/m_actualRows);
	}
	return dataStrings;
}

/*!
writes the content of \c dataSource to the file \c fileName.
*/
void JsonFilterPrivate::write(const QString& fileName, AbstractDataSource* dataSource) {
	Q_UNUSED(fileName);
	Q_UNUSED(dataSource);

	//TODO: save data to json file
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
/*!
Saves as XML.
*/
void JsonFilter::save(QXmlStreamWriter* writer) const {
	writer->writeStartElement("jsonFilter");
	writer->writeAttribute("rowType", QString::number(d->rowType));
	writer->writeAttribute("dateTimeFormat", d->dateTimeFormat);
	writer->writeAttribute("numberFormat", QString::number(d->numberFormat));
	writer->writeAttribute("createIndex", QString::number(d->createIndexEnabled));
	writer->writeAttribute("parseRowsName", QString::number(d->parseRowsName));
	writer->writeAttribute("nanValue", QString::number(d->nanValue));
	writer->writeAttribute("startRow", QString::number(d->startRow));
	writer->writeAttribute("endRow", QString::number(d->endRow));
	writer->writeAttribute("startColumn", QString::number(d->startColumn));
	writer->writeAttribute("endColumn", QString::number(d->endColumn));

	QStringList list;
	for(auto it = modelRows().begin(); it != modelRows().end(); ++it){
		list.append(QString::number(*it));
	}
	writer->writeAttribute("modelRows", list.join(';'));

	writer->writeEndElement();
	DEBUG("JsonFilter save params");
}

/*!
Loads from XML.
*/
bool JsonFilter::load(XmlStreamReader* reader) {
	if (!reader->isStartElement() || reader->name() != "jsonFilter") {
		reader->raiseError(i18n("no json filter element found"));
		return false;
	}
	QString attributeWarning = i18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs = reader->attributes();

	QString str = attribs.value("rowType").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.arg("'rowType'"));
	else
		d->rowType = static_cast<QJsonValue::Type>(str.toInt());

	str = attribs.value("dateTimeFormat").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.arg("'dateTimeFormat'"));
	else
		d->dateTimeFormat = str;

	str = attribs.value("numberFormat").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.arg("'numberFormat'"));
	else
		d->numberFormat = static_cast<QLocale::Language >(str.toInt());

	str = attribs.value("createIndex").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.arg("'createIndex'"));
	else
		d->createIndexEnabled = str.toInt();

	str = attribs.value("parseRowsName").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.arg("'parseRowsName'"));
	else
		d->parseRowsName = str.toInt();

	str = attribs.value("nanValue").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.arg("'nanValue'"));
	else
		d->nanValue = str.toDouble();

	str = attribs.value("startRow").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.arg("'startRow'"));
	else
		d->startRow = str.toInt();

	str = attribs.value("endRow").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.arg("'endRow'"));
	else
		d->endRow = str.toInt();

	str = attribs.value("startColumn").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.arg("'startColumn'"));
	else
		d->startColumn = str.toInt();

	str = attribs.value("endColumn").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.arg("'endColumn'"));
	else
		d->endColumn = str.toInt();

	QStringList list = attribs.value("modelRows").toString().split(';');
	if (list.isEmpty())
		reader->raiseWarning(attributeWarning.arg("'modelRows'"));
	else{
		d->modelRows = QVector<int>();
		for(auto it = list.begin(); it !=list.end(); ++it)
			d->modelRows.append((*it).toInt());
	}

	DEBUG("JsonFilter load params");
	return true;
}