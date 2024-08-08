/*
	File                 : McapFilter.cpp
	Project              : LabPlot
	Description          : JSON I/O-filter.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2018 Andrey Cygankov <craftplace.ms@gmail.com>
	SPDX-FileCopyrightText: 2018-2020 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2018-2020 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "backend/datasources/filters/McapFilter.h"
#include "backend/core/column/Column.h"
#include "backend/datasources/AbstractDataSource.h"
#include "backend/datasources/filters/McapFilterPrivate.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/trace.h"
#include "backend/spreadsheet/Spreadsheet.h"

#include <KCompressionDevice>
#include <KLocalizedString>
#include <QDataStream>
#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <chrono>
#include <thread>

#include <cmath>

#ifndef HAVE_LZ4
#define MCAP_COMPRESSION_NO_LZ4
#endif

#ifndef HAVE_ZSTD
#define MCAP_COMPRESSION_NO_ZSTD
#endif

#define MCAP_IMPLEMENTATION
#include "mcap/mcap.hpp"
#include "mcap/writer.hpp"

/*!
\class McapFilter
\brief ManagesreadDataFromFile the import/export of data from/to a file formatted using JSON.

\ingroup datasources
*/
McapFilter::McapFilter()
	: AbstractFileFilter(FileType::MCAP)
	, d(new McapFilterPrivate(this)) {
}

McapFilter::~McapFilter() = default;

/*!
reads the content of the file \c fileName.
*/
void McapFilter::readDataFromFile(const QString& fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode) {
	d->readDataFromFile(fileName, dataSource, importMode);
}

QVector<QStringList> McapFilter::preview(const QString& fileName, int lines) {
	return d->preview(fileName, lines);
}

/*!
writes the content of the data source \c dataSource to the file \c fileName.
*/
void McapFilter::write(const QString& fileName, AbstractDataSource* dataSource) {
	d->write(fileName, dataSource);
}

void McapFilter::writeWithOptions(const QString& fileName, AbstractDataSource* dataSource, int compressionMode, int compressionLevel) {
	d->writeWithOptions(fileName, dataSource, compressionMode, compressionLevel);
}

/*!
returns the list of all predefined data types.
*/
QStringList McapFilter::dataTypes() {
	const QMetaObject& mo = AbstractColumn::staticMetaObject;
	const QMetaEnum& me = mo.enumerator(mo.indexOfEnumerator("ColumnMode"));
	QStringList list;
	for (int i = 0; i <= 100; ++i) // me.keyCount() does not work because we have holes in enum
		if (me.valueToKey(i))
			list << QLatin1String(me.valueToKey(i));
	return list;
}

/*!
returns the list of all predefined data row types.
*/
QStringList McapFilter::dataRowTypes() {
	return (QStringList() << QStringLiteral("Array") << QStringLiteral("Object"));
}

void McapFilter::setDataRowType(QJsonValue::Type type) {
	d->rowType = type;
}
QJsonValue::Type McapFilter::dataRowType() const {
	return d->rowType;
}

void McapFilter::setModel(QJsonModel* model) {
	d->model = model;
}

void McapFilter::setModelRows(const QVector<int>& rows) {
	d->modelRows = rows;
}

QVector<int> McapFilter::modelRows() const {
	return d->modelRows;
}

void McapFilter::setDateTimeFormat(const QString& f) {
	d->dateTimeFormat = f;
}
QString McapFilter::dateTimeFormat() const {
	return d->dateTimeFormat;
}

void McapFilter::setNumberFormat(QLocale::Language lang) {
	d->numberFormat = lang;
}
QLocale::Language McapFilter::numberFormat() const {
	return d->numberFormat;
}

void McapFilter::setNaNValueToZero(bool b) {
	d->nanValue = (b ? 0 : NAN);
}
bool McapFilter::NaNValueToZeroEnabled() const {
	if (d->nanValue == 0)
		return true;
	return false;
}

void McapFilter::setCreateIndexEnabled(bool b) {
	d->createIndexEnabled = b;
}

QStringList McapFilter::vectorNames() const {
	return d->vectorNames;
}

QVector<AbstractColumn::ColumnMode> McapFilter::columnModes() {
	return d->columnModes;
}

void McapFilter::setStartRow(const int r) {
	d->startRow = r;
}
int McapFilter::startRow() const {
	return d->startRow;
}

void McapFilter::setEndRow(const int r) {
	d->endRow = r;
}
int McapFilter::endRow() const {
	return d->endRow;
}

void McapFilter::setStartColumn(const int c) {
	d->startColumn = c;
}
int McapFilter::startColumn() const {
	return d->startColumn;
}

void McapFilter::setEndColumn(const int c) {
	d->endColumn = c;
}
int McapFilter::endColumn() const {
	return d->endColumn;
}

QString McapFilter::fileInfoString(const QString& fileName) {
	DEBUG(Q_FUNC_INFO);

	mcap::McapReader reader;
	{
		const auto res = reader.open(STDSTRING(fileName));
		if (!res.ok()) {
			return i18n("Failed to open the device/file or it's empty.");
		}
	}

	auto status = reader.readSummary(mcap::ReadSummaryMethod::NoFallbackScan);
	// TODO: check status

	QString info;
	info += i18n("Valid MCAP file");
	info += QLatin1String("<br>");

	auto stats = reader.statistics();
	if (stats.has_value()) {
		info += i18n("Message count: ") + QString::number(stats.value().messageCount);
		info += QLatin1String("<br>");
		info += i18n("Schema count: ") + QString::number(stats.value().schemaCount);
		info += QLatin1String("<br>");
		info += i18n("Channel count: ") + QString::number(stats.value().channelCount);
		info += QLatin1String("<br>");
		info += i18n("Attachment count: ") + QString::number(stats.value().attachmentCount);
		info += QLatin1String("<br>");
		info += i18n("Metadata count: ") + QString::number(stats.value().metadataCount);
		info += QLatin1String("<br>");
		info += i18n("Message Start Time: ") + QDateTime::fromMSecsSinceEpoch(static_cast<long>(stats.value().messageStartTime) / 1000000).toString();
		info += QLatin1String("<br>");
		info += i18n("Message End Time: ") + QDateTime::fromMSecsSinceEpoch(static_cast<long>(stats.value().messageEndTime / 1000000)).toString();
	} else {
		info += i18n("No Statistics found.");
	}

	info += QLatin1String("<br>");
	info += QLatin1String("JSON Encoded Topics:");
	info += QLatin1String("<br>");

	int maxNoOfTopics = 5;
	int topicCount = 0;
	std::unordered_map<mcap::ChannelId, mcap::ChannelPtr> channel_map = reader.channels();
	std::for_each(channel_map.begin(), channel_map.end(), [&](std::pair<mcap::ChannelId, mcap::ChannelPtr> entry) {
		if (entry.second->messageEncoding == "json") {
			info += QString::fromStdString((entry.second->topic));
			info += QLatin1String("<br>");
			topicCount += 1;
			if (topicCount == maxNoOfTopics) {
				info += QLatin1String("...");
				info += QLatin1String("<br>");
			}
		}
	});

	return info;
}

// #####################################################################
// ################### Private implementation ##########################
// #####################################################################
McapFilterPrivate::McapFilterPrivate(McapFilter* owner)
	: q(owner) {
}

/*!
returns 1 if row is invalid and 0 otherwise.
*/
int McapFilterPrivate::checkRow(QJsonValueRef value, int& countCols) {
	switch (rowType) {
	// TODO: implement other value types
	case QJsonValue::Array: {
		QJsonArray row = value.toArray();
		if (row.isEmpty())
			return 1;
		countCols = (countCols == -1 || countCols > row.count()) ? row.count() : countCols;
		break;
	}
	case QJsonValue::Object: {
		QJsonObject row = value.toObject();
		if (row.isEmpty())
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
int McapFilterPrivate::parseColumnModes(const QJsonValue& row, const QString& /*rowName*/) {
	columnModes.clear();
	vectorNames.clear();

	// add index column if required
	if (createIndexEnabled) {
		columnModes << AbstractColumn::ColumnMode::Integer;
		vectorNames << i18n("index");
	}

	// determine the column modes and names
	for (int i = startColumn - 1; i < endColumn; ++i) {
		QJsonValue columnValue;

		QString key = row.toObject().keys().at(i);
		vectorNames << key;
		columnValue = row.toObject().value(key);
		if (key == QLatin1String("logTime") || key == QLatin1String("publishTime")) {
			columnModes << AbstractColumn::ColumnMode::DateTime;
			continue;
		}
		if (key == QLatin1String("sequence")) {
			columnModes << AbstractColumn::ColumnMode::Integer;
			continue;
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
			return -1;
		}
	}

	return 0;
}

void McapFilterPrivate::setEmptyValue(int column, int row) {
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

void McapFilterPrivate::setValueFromString(int column, int row, const QString& valueString) {
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
		if (vectorNames[column] == QLatin1String("publishTime") || vectorNames[column] == QLatin1String("logTime")) {
			static_cast<QVector<QDateTime>*>(m_dataContainer[column])->operator[](row) = QDateTime::fromMSecsSinceEpoch(valueString.toLong());
			break;
		}
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
	determines the relevant part of the full JSON document to be read and its structure.
	returns \c true if successful, \c false otherwise.
*/
bool McapFilterPrivate::prepareDocumentToRead() {
	DEBUG(Q_FUNC_INFO);
	PERFTRACE(QStringLiteral("Prepare the JSON document to read"));

	if (!m_preparedDoc.isEmpty()) {
		if (m_preparedDoc.isArray())
			containerType = McapFilter::DataContainerType::Array;
		else if (m_preparedDoc.isObject())
			containerType = McapFilter::DataContainerType::Object;
		else
			return false;
	} else
		return false;

	int countRows = 0;
	int countCols = -1;
	QJsonValue firstRow;
	QString firstRowName;

	switch (containerType) {
	case McapFilter::DataContainerType::Array: {
		QJsonArray arr = m_preparedDoc.array();
		int count = arr.count();
		if (count < startRow)
			return false;

		int endRowOffset = (endRow == -1 || endRow > count) ? count : endRow;
		firstRow = *(arr.begin() + (startRow - 1));
		for (QJsonArray::iterator it = arr.begin() + (startRow - 1); it != arr.begin() + endRowOffset; ++it) {
			if (checkRow(*it, countCols) != 0)
				return false;
			countRows++;
		}
		break;
	}
	case McapFilter::DataContainerType::Object: {
		QJsonObject obj = m_preparedDoc.object();
		if (obj.count() < startRow)
			return false;

		int startRowOffset = startRow - 1;
		int endRowOffset = (endRow == -1 || endRow > obj.count()) ? obj.count() : endRow;
		firstRow = *(obj.begin() + startRowOffset);
		firstRowName = (obj.begin() + startRowOffset).key();
		for (QJsonObject::iterator it = obj.begin() + startRowOffset; it != obj.begin() + endRowOffset; ++it) {
			if (checkRow(*it, countCols) != 0)
				return false;
			countRows++;
		}

		break;
	}
	}

	if (endColumn == -1 || endColumn > countCols)
		endColumn = countCols;

	m_actualRows = countRows;
	m_actualCols = endColumn - startColumn + 1 + createIndexEnabled;

	if (parseColumnModes(firstRow, firstRowName) != 0)
		return false;

	DEBUG("start/end column: = " << startColumn << ' ' << endColumn);
	DEBUG("start/end rows = " << startRow << ' ' << endRow);
	DEBUG("actual cols/rows = " << m_actualCols << ' ' << m_actualRows);

	return true;
}

int McapFilterPrivate::mcapToJson(const QString& fileName, int lines) {
	DEBUG(Q_FUNC_INFO);
	DEBUG("MCAP Filter mcapToJson:" << STDSTRING(current_topic));

	mcap::McapReader reader;
	{
		const auto res = reader.open(STDSTRING(fileName));
		if (!res.ok()) {
			std::cerr << "Failed to open " << STDSTRING(fileName) << " for reading: " << res.message << std::endl;
			const_cast<McapFilter*>(q)->setLastError(i18n("Failed to read the file. Reason: %1", QString::fromStdString(res.message)));
			return 0;
		}
	}

	if (current_topic == QLatin1String("")) {
		QVector<QString> topics = getValidTopics(fileName); // Todo: make this more efficient. Only open file once.
		if (topics.size() == 0) {
			const_cast<McapFilter*>(q)->setLastError(i18n("No JSON encoded topics found."));
			return 0;
		}
		current_topic = topics[0];
	}

	mcap::ReadMessageOptions opt;

	std::function<bool(std::string_view)> lambda2 = [this](std::string_view v) {
		std::string s{v};
		return QString::fromStdString(s) == current_topic;
	};
	opt.topicFilter = lambda2;
	auto messageView = reader.readMessages(nullptr, opt);

	// Get number of messages in view
	int msg_count = 0;

	QJsonArray jsonArray;
	try {
		for (auto it = messageView.begin(); it != messageView.end(); it++) {
			// skip any non-json-encoded messages.
			// qDebug() << "Processing topic " << QString::fromStdString(it->channel->topic) << "with encoding"
			// 		 << QString::fromStdString(it->channel->messageEncoding);

			if (it->channel->messageEncoding != "json") { // only support json encoding for now
				continue;
			}
			if (current_topic != QLatin1String("")) {
				if (QString::fromStdString(it->channel->topic) != current_topic) { // only support json encoding for now
					continue;
				}
			}

			// Without going to string_view first I get this error:
			// 32: QWARN  : MCAPFilterTest::testArrayImport() Error parsing JSON string: "{\"value\": 0}\u0005" "garbage at the end of the document"
			std::string_view a(reinterpret_cast<const char*>(it->message.data), it->message.dataSize);

			QString asString = QString::fromStdString(std::string(a));
			QJsonParseError error;
			QJsonDocument jsonDocument = QJsonDocument::fromJson(asString.toUtf8(), &error);

			QJsonObject obj = flattenJson(jsonDocument.object());

			// TODO: handle seqeunce, and times as longs instead of strings
			obj.insert(QLatin1String("sequence"), QJsonValue::fromVariant(QString::number(it->message.sequence)));
			obj.insert(QLatin1String("logTime"), QJsonValue::fromVariant(QString::number(it->message.logTime / 1000000))); // nano to milli otherwise value is 0
			obj.insert(QLatin1String("publishTime"),
					   QJsonValue::fromVariant(QString::number(it->message.publishTime / 1000000))); // nano to milli otherwise value is 0

			if (error.error != QJsonParseError::NoError) {
				qWarning() << "Error parsing JSON string:" << asString << error.errorString();
				continue;
			}

			// Check if the parsed JSON document is an object
			if (!jsonDocument.isObject()) {
				qWarning() << "JSON string does not contain an object:" << asString;
				continue;
			}

			// Convert the JSON document to a JSON object and add it to the JSON array
			// foreach (const QString& key, jsonDocument.object().keys()) {
			// 	QJsonValue value = jsonDocument.object().value(key);
			// 	qDebug() << "Key = " << key << ", Value = " << value;
			// }
			jsonArray.append(obj);
			msg_count++;
			if (msg_count == lines) {
				qDebug() << "Stop reading MCAP file. Requested number of " << lines << "lines reached.";
				break;
			}
		}
	} catch (const std::exception& e) {
		std::cerr << "Error parsing MCAP file: " << e.what() << std::endl;
	}

	QJsonDocument finalJsonDocument(jsonArray);

	// qDebug() << finalJsonDocument.toJson(QJsonDocument::Compact);

	m_preparedDoc = finalJsonDocument;
	return msg_count;
}

/*!
reads the content of the file \c fileName to the data source \c dataSource. Uses the settings defined in the data source.
*/
void McapFilterPrivate::readDataFromFile(const QString& fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode) {
	DEBUG("MCAP Filter: Trying to open file:" << STDSTRING(fileName));

	int msg_count = mcapToJson(fileName);
	bool success = prepareDocumentToRead();
	if (success)
		importData(dataSource, importMode, msg_count);
}

/*!
import the content of document \c m_preparedDoc to the data source \c dataSource. Uses the settings defined in the data source.
*/
void McapFilterPrivate::importData(AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode, int lines) {
	bool ok = false;
	m_columnOffset = dataSource->prepareImport(m_dataContainer, importMode, m_actualRows, m_actualCols, vectorNames, columnModes, ok);
	int rowOffset = startRow - 1;
	int colOffset = (int)createIndexEnabled;
	DEBUG("reading " << m_actualRows << " lines");
	DEBUG("reading " << m_actualCols << " columns");

	int progressIndex = 0;
	const float progressInterval = 0.01 * lines; // update on every 1% only

	const auto& array = m_preparedDoc.array();
	const auto& arrayIterator = array.begin();

	for (int i = 0; i < m_actualRows; ++i) {
		if (createIndexEnabled)
			static_cast<QVector<int>*>(m_dataContainer[0])->operator[](i) = i + 1;

		QJsonValue row;
		row = *(arrayIterator + rowOffset + i);

		for (int n = 0; n < m_actualCols - colOffset; ++n) {
			QJsonValue value;
			value = *(row.toObject().begin() + n + startColumn - 1);

			switch (value.type()) {
			case QJsonValue::Double:
				if (columnModes[colOffset + n] == AbstractColumn::ColumnMode::Double)
					static_cast<QVector<double>*>(m_dataContainer[colOffset + n])->operator[](i) = value.toDouble();
				else
					setEmptyValue(colOffset + n, i + startRow - 1);
				break;
			case QJsonValue::String: {
				setValueFromString(colOffset + n, i, value.toString());
				break;
			}
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

		// set the plot designation to 'X' for index and name columns, if available
		auto* spreadsheet = dynamic_cast<Spreadsheet*>(dataSource);
		if (spreadsheet) {
			if (createIndexEnabled)
				spreadsheet->column(m_columnOffset)->setPlotDesignation(AbstractColumn::PlotDesignation::X);
		}
	}
	dataSource->finalizeImport(m_columnOffset, startColumn, startColumn + m_actualCols - 1, dateTimeFormat, importMode);
}

/*!
generates the preview for the file \c fileName.a
*/
QVector<QStringList> McapFilterPrivate::preview(const QString& fileName, int lines) {
	DEBUG(Q_FUNC_INFO << "lines=" << std::to_string(lines));

	mcapToJson(fileName, lines);
	bool success = prepareDocumentToRead();
	// TODO: check success
	Q_UNUSED(success)
	return preview(lines);
}

/*!
generates the preview for document \c m_preparedDoc.
*/
QVector<QStringList> McapFilterPrivate::preview(int lines) {
	DEBUG(Q_FUNC_INFO);

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
		case McapFilter::DataContainerType::Object:
			rowName = (objectIterator + rowOffset + i).key();
			row = *(objectIterator + rowOffset + i);
			break;
		case McapFilter::DataContainerType::Array:
			row = *(arrayIterator + rowOffset + i);

			break;
		}

		QStringList lineString;
		if (createIndexEnabled)
			lineString += QString::number(i + 1);

		for (int n = startColumn - 1; n < endColumn; ++n) {
			QJsonValue value;
			switch (rowType) {
			case QJsonValue::Object:
				value = *(row.toObject().begin() + n);
				break;
			case QJsonValue::Array:
				value = *(row.toArray().begin() + n);
				break;
			// TODO: implement other value types
			case QJsonValue::Double:
			case QJsonValue::String:
			case QJsonValue::Bool:
			case QJsonValue::Null:
			case QJsonValue::Undefined:
				break;
			}

			switch (value.type()) {
			case QJsonValue::Double:
				lineString += QString::number(value.toDouble(), 'g', 16);
				break;
			case QJsonValue::String:
				if (columnModes.at(n + createIndexEnabled) == AbstractColumn::ColumnMode::DateTime) {
					lineString += QDateTime::fromMSecsSinceEpoch(value.toString().toLong()).toString(dateTimeFormat);
				} else {
					lineString += value.toString();
				}
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
writes the content of \c dataSource to the file \c fileName.
*/
void McapFilterPrivate::write(const QString& fileName, AbstractDataSource* dataSource) {
	DEBUG(Q_FUNC_INFO);
	int compression_type = 0; // None
	int compression_level = 0; // Fastest
	return writeWithOptions(fileName, dataSource, compression_type, compression_level);
}

/*!
writes the content of \c dataSource to the file \c fileName.
*/
void McapFilterPrivate::writeWithOptions(const QString& fileName, AbstractDataSource* dataSource, int compression_mode, int compression_level) {
	DEBUG(Q_FUNC_INFO);

	auto* spreadsheet = dynamic_cast<Spreadsheet*>(dataSource);

	auto opts = mcap::McapWriterOptions("json");
	opts.compressionLevel = static_cast<mcap::CompressionLevel>(compression_level);
	opts.compression = static_cast<mcap::Compression>(compression_mode);

	DEBUG(Q_FUNC_INFO << fileName.toStdString());
	std::string outputFilename = fileName.toStdString();

	static const char* SCHEMA_NAME = "labplot.Spreadsheet";
	static const char* SCHEMA_TEXT = R"({
	"title": "Spreadsheet", 
	"description": "An object exported from Labplot", 
	"type": "object"
	})"; // Todo: Create proper schema.

	mcap::McapWriter writer;
	{
		const auto res = writer.open(outputFilename, opts);
		if (!res.ok()) {
			std::cerr << "Failed to open " << outputFilename << " for writing: " << res.message << std::endl;
			return;
		}
	}
	// Create a channel and schema for our messages.
	// A message's channel informs the reader which topic those messages were published on.
	// A channel's schema informs the reader of how to interpret the messages' content.
	// A schema can be used by multiple channels, and a channel can be used by multiple messages.
	mcap::ChannelId channelId;
	{
		mcap::Schema schema(SCHEMA_NAME, "jsonschema", SCHEMA_TEXT);
		writer.addSchema(schema);

		mcap::Channel channel("/spreadsheet", "json", schema.id);
		writer.addChannel(channel);
		channelId = channel.id;
	}
	mcap::Timestamp startTime = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	// TODO: use startTime
	Q_UNUSED(startTime)
	QJsonArray jsonArray = m_preparedDoc.array();

	int numRows = spreadsheet->rowCount();
	int numCols = spreadsheet->columnCount();

	for (int row = 0; row < numRows; ++row) {
		QJsonObject obj;
		mcap::Message msg;
		msg.channelId = channelId;
		for (int col = 0; col < numCols; ++col) {
			QString columnName = spreadsheet->column(col)->name();
			QVariant value = spreadsheet->column(col)->valueAt(row);
			QJsonValue jsonValue = QJsonValue::fromVariant(value);
			if (columnName == QLatin1String("logTime")) { // Special field in MCAP
				msg.logTime = mcap::Timestamp(value.toLongLong() * 1000000);
				continue;
			}
			if (columnName == QLatin1String("publishTime")) { // Special field in MCAP
				msg.publishTime = mcap::Timestamp(value.toLongLong() * 1000000);
				continue;
			}
			if (columnName == QLatin1String("sequence")) { // Special field in MCAP
				msg.sequence = mcap::Timestamp(value.toLongLong());
				continue;
			}
			obj.insert(columnName, jsonValue);
		}

		QJsonDocument doc(obj); // TODO: Unflatten json?
		QByteArray data = doc.toJson(QJsonDocument::Compact);

		msg.data = reinterpret_cast<const std::byte*>(data.constData());
		msg.dataSize = data.size();
		auto res = writer.write(msg);
		if (!res.ok()) {
			std::cerr << "failed to write message: " << res.message << std::endl;
			writer.close();
			return;
		}
	}
	writer.close();
	qDebug() << "Exported spreadsheet with " << numRows << " rows and " << numCols << "columns to" << fileName;
}

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################
/*!
Saves as XML.
*/
void McapFilter::save(QXmlStreamWriter* writer) const {
	writer->writeStartElement(QStringLiteral("jsonFilter"));
	writer->writeAttribute(QStringLiteral("rowType"), QString::number(d->rowType));
	writer->writeAttribute(QStringLiteral("dateTimeFormat"), d->dateTimeFormat);
	writer->writeAttribute(QStringLiteral("numberFormat"), QString::number(d->numberFormat));
	writer->writeAttribute(QStringLiteral("createIndex"), QString::number(d->createIndexEnabled));
	writer->writeAttribute(QStringLiteral("nanValue"), QString::number(d->nanValue));
	writer->writeAttribute(QStringLiteral("startRow"), QString::number(d->startRow));
	writer->writeAttribute(QStringLiteral("endRow"), QString::number(d->endRow));
	writer->writeAttribute(QStringLiteral("startColumn"), QString::number(d->startColumn));
	writer->writeAttribute(QStringLiteral("endColumn"), QString::number(d->endColumn));

	QStringList list;
	for (auto& it : modelRows())
		list.append(QString::number(it));

	writer->writeAttribute(QStringLiteral("modelRows"), list.join(QLatin1Char(';')));

	writer->writeEndElement();
}

/*!
Loads from XML.
*/
bool McapFilter::load(XmlStreamReader* reader) {
	QXmlStreamAttributes attribs = reader->attributes();
	QString str;

	READ_INT_VALUE("rowType", rowType, QJsonValue::Type);
	READ_STRING_VALUE("dateTimeFormat", dateTimeFormat);
	READ_INT_VALUE("numberFormat", numberFormat, QLocale::Language);
	READ_INT_VALUE("createIndex", createIndexEnabled, bool);
	READ_DOUBLE_VALUE("nanValue", nanValue);
	READ_INT_VALUE("startRow", startRow, int);
	READ_INT_VALUE("endRow", endRow, int);
	READ_INT_VALUE("startColumn", startColumn, int);
	READ_INT_VALUE("endColumn", endColumn, int);

	QStringList list = attribs.value(QStringLiteral("modelRows")).toString().split(QLatin1Char(';'));
	if (list.isEmpty())
		reader->raiseMissingAttributeWarning(QStringLiteral("'modelRows'"));
	else {
		d->modelRows = QVector<int>();
		for (auto& it : list)
			d->modelRows.append(it.toInt());
	}

	return true;
}

QJsonDocument McapFilter::getJsonDocument(const QString& fileName) {
	DEBUG(Q_FUNC_INFO);
	return d->getJsonDocument(fileName);
}

QJsonDocument McapFilterPrivate::getJsonDocument(const QString& fileName) {
	DEBUG(Q_FUNC_INFO);
	mcapToJson(fileName);
	bool success = prepareDocumentToRead();
	// TODO: check success
	Q_UNUSED(success)
	return m_preparedDoc;
}

// https://stackoverflow.com/a/76036333
QJsonObject McapFilterPrivate::flattenJson(QJsonValue jsonVal, QString aggregatedKey) {
	QJsonObject flattenedJson;

	auto dryFx = [&](auto& key, auto& value) {
		const QString delimiter = QLatin1String(".");
		QString nKey = aggregatedKey + delimiter + key;
		if (nKey.at(0) == delimiter)
			nKey.remove(0, 1);

		if (value.isObject() || value.isArray()) {
			QJsonObject nestedJson = flattenJson(value, nKey);
			flattenedJson = mergeJsonObjects(flattenedJson, nestedJson);
		} else {
			flattenedJson.insert(nKey, value);
		}
	};

	if (jsonVal.isArray()) {
		const auto json_arr = jsonVal.toArray();
		for (auto it = json_arr.constBegin(); it != json_arr.constEnd(); ++it) {
			const auto key = QString::number((it - json_arr.constBegin()));
			const auto value = *it;
			dryFx(key, value);
		}
	}

	if (jsonVal.isObject()) {
		const auto json_obj = jsonVal.toObject();
		for (auto it = json_obj.constBegin(); it != json_obj.constEnd(); ++it) {
			const auto key = it.key();
			const auto value = it.value();
			dryFx(key, value);
		}
	}

	return flattenedJson;
}

// TODO: implement unflatten method
QJsonObject McapFilterPrivate::unflattenJson(const QJsonObject& obj, QString /*separator*/) {
	return obj;
}

QJsonObject McapFilterPrivate::mergeJsonObjects(const QJsonObject& obj1, const QJsonObject& obj2) {
	QJsonObject mergedObj = obj1;
	for (auto it = obj2.constBegin(); it != obj2.constEnd(); ++it) {
		mergedObj.insert(it.key(), it.value());
	}
	return mergedObj;
}

QVector<QString> McapFilterPrivate::getValidTopics(const QString& fileName) {
	DEBUG(Q_FUNC_INFO);

	QVector<QString> valid_topics;
	mcap::McapReader reader;
	{
		const auto res = reader.open(STDSTRING(fileName));
		if (!res.ok()) {
			std::cerr << "Failed to open " << STDSTRING(fileName) << " for reading: " << res.message << std::endl;
			const_cast<McapFilter*>(q)->setLastError(i18n("Failed to read the file. Reason: %1", QString::fromStdString(res.message)));
			return valid_topics;
		}
	}

	auto status = reader.readSummary(mcap::ReadSummaryMethod::NoFallbackScan);
	// TODO: check status
	Q_UNUSED(status)
	std::unordered_map<mcap::ChannelId, mcap::ChannelPtr> channel_map = reader.channels();
	std::for_each(channel_map.begin(), channel_map.end(), [&](std::pair<mcap::ChannelId, mcap::ChannelPtr> entry) {
		if (entry.second->messageEncoding == "json") {
			DEBUG("Found valid topic:" << entry.second->topic);
			valid_topics.append(QString::fromStdString((entry.second->topic)));
			current_topic = QString::fromStdString(entry.second->topic); // Last topic by default?
		}
	});
	return valid_topics;
}

QVector<QString> McapFilter::getValidTopics(const QString& fileName) {
	return d->getValidTopics(fileName);
}

void McapFilterPrivate::setCurrentTopic(QString topic) {
	DEBUG(Q_FUNC_INFO);
	if (current_topic != topic) {
		m_prepared = false;
	}
	current_topic = topic;
}

void McapFilter::setCurrentTopic(QString topic) {
	DEBUG(Q_FUNC_INFO);
	d->setCurrentTopic(topic);
}

QString McapFilterPrivate::getCurrentTopic() {
	DEBUG(Q_FUNC_INFO);
	return current_topic;
}

QString McapFilter::getCurrentTopic() {
	return d->getCurrentTopic();
}
