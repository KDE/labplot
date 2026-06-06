/*
	File                 : ParquetFilter.cpp
	Project              : LabPlot
	Description          : Parquet/Arrow IPC/ORC I/O-filter
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2026 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "ParquetFilter.h"
#include "ParquetFilterPrivate.h"
#include "backend/core/column/Column.h"
#include "backend/datasources/AbstractDataSource.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/macros.h"

#ifdef HAVE_PARQUET
#include <arrow/ipc/reader.h>
#include <parquet/arrow/reader.h>
#endif

#ifdef HAVE_ORC
#include <arrow/adapters/orc/adapter.h>
#endif

#include <KLocalizedString>

/*!
	\class ParquetFilter
	\brief Manages the import/export of data from/to files in Apache Parquet,
	Arrow IPC (Feather), and Apache ORC formats using Apache Arrow.

	\ingroup datasources
*/
ParquetFilter::ParquetFilter(FileType type)
	: AbstractFileFilter(type)
	, d(new ParquetFilterPrivate(this)) {
	d->fileType = type;
}

ParquetFilter::~ParquetFilter() = default;

QString ParquetFilter::fileInfoString(const QString& fileName) {
#ifdef HAVE_PARQUET
	QString info;

	auto infile_result = arrow::io::ReadableFile::Open(fileName.toStdString());
	if (!infile_result.ok())
		return i18n("Could not open file.");

	auto infile = *infile_result;

	if (fileName.endsWith(QLatin1String(".parquet"), Qt::CaseInsensitive) || fileName.endsWith(QLatin1String(".parq"), Qt::CaseInsensitive)) {
		auto reader_result = parquet::arrow::OpenFile(infile, arrow::default_memory_pool());
		if (reader_result.ok()) {
			auto reader = std::move(*reader_result);
			auto metadata = reader->parquet_reader()->metadata();
			info += i18n("Format: Apache Parquet");
			info += QLatin1Char('\n');
			info += i18n("Version: %1", metadata->version());
			info += QLatin1Char('\n');
			info += i18n("Created by: %1", QString::fromStdString(metadata->created_by()));
			info += QLatin1Char('\n');
			info += i18n("Number of columns: %1", metadata->num_columns());
			info += QLatin1Char('\n');
			info += i18n("Number of rows: %1", (qlonglong)metadata->num_rows());
			info += QLatin1Char('\n');
			info += i18n("Number of row groups: %1", metadata->num_row_groups());
		}
	} else if (fileName.endsWith(QLatin1String(".feather"), Qt::CaseInsensitive) || fileName.endsWith(QLatin1String(".arrow"), Qt::CaseInsensitive)
			   || fileName.endsWith(QLatin1String(".ipc"), Qt::CaseInsensitive)) {
		auto reader_result = arrow::ipc::RecordBatchFileReader::Open(infile);
		if (reader_result.ok()) {
			auto reader = *reader_result;
			info += i18n("Format: Arrow IPC (Feather)");
			info += QLatin1Char('\n');
			info += i18n("Number of columns: %1", reader->schema()->num_fields());
			info += QLatin1Char('\n');
			info += i18n("Number of record batches: %1", reader->num_record_batches());
		}
	}
#ifdef HAVE_ORC
	else if (fileName.endsWith(QLatin1String(".orc"), Qt::CaseInsensitive)) {
		auto reader_result = arrow::adapters::orc::ORCFileReader::Open(infile, arrow::default_memory_pool());
		if (reader_result.ok()) {
			auto reader = std::move(*reader_result);
			info += i18n("Format: Apache ORC");
			info += QLatin1Char('\n');
			info += i18n("Number of rows: %1", (qlonglong)reader->NumberOfRows());
			info += QLatin1Char('\n');
			info += i18n("Number of stripes: %1", (qlonglong)reader->NumberOfStripes());
		}
	}
#endif

	return info;
#else
	Q_UNUSED(fileName)
	return {};
#endif
}

void ParquetFilter::readDataFromFile(const QString& fileName, AbstractDataSource* dataSource, ImportMode mode) {
	d->readDataFromFile(fileName, dataSource, mode);
}

QVector<QStringList> ParquetFilter::preview(const QString& fileName, int lines) {
	return d->preview(fileName, lines);
}

void ParquetFilter::write(const QString& fileName, AbstractDataSource* dataSource) {
	d->write(fileName, dataSource);
}

QStringList ParquetFilter::columnNames() const {
	return d->columnNames;
}

int ParquetFilter::columnCount() const {
	return d->numColumns;
}

int ParquetFilter::rowCount() const {
	return d->numRows;
}

void ParquetFilter::setStartRow(int r) {
	d->startRow = r;
}

int ParquetFilter::startRow() const {
	return d->startRow;
}

void ParquetFilter::setEndRow(int r) {
	d->endRow = r;
}

int ParquetFilter::endRow() const {
	return d->endRow;
}

void ParquetFilter::setStartColumn(int c) {
	d->startColumn = c;
}

int ParquetFilter::startColumn() const {
	return d->startColumn;
}

void ParquetFilter::setEndColumn(int c) {
	d->endColumn = c;
}

int ParquetFilter::endColumn() const {
	return d->endColumn;
}

void ParquetFilter::setSelectedColumnNames(const QStringList& names) {
	d->selectedColumnNames = names;
}

QStringList ParquetFilter::selectedColumnNames() const {
	return d->selectedColumnNames;
}

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################
void ParquetFilter::save(QXmlStreamWriter* writer) const {
	writer->writeStartElement(QStringLiteral("parquetFilter"));
	writer->writeAttribute(QStringLiteral("startRow"), QString::number(d->startRow));
	writer->writeAttribute(QStringLiteral("endRow"), QString::number(d->endRow));
	writer->writeAttribute(QStringLiteral("startColumn"), QString::number(d->startColumn));
	writer->writeAttribute(QStringLiteral("endColumn"), QString::number(d->endColumn));
	writer->writeEndElement();
}

bool ParquetFilter::load(XmlStreamReader* reader) {
	QString str;
	const auto& attribs = reader->attributes();
	READ_INT_VALUE("startRow", startRow, int);
	READ_INT_VALUE("endRow", endRow, int);
	READ_INT_VALUE("startColumn", startColumn, int);
	READ_INT_VALUE("endColumn", endColumn, int);
	return true;
}

// ##############################################################################
// ######################### Private implementation #############################
// ##############################################################################
ParquetFilterPrivate::ParquetFilterPrivate(ParquetFilter* owner)
	: q(owner) {
}

#ifdef HAVE_PARQUET
std::shared_ptr<arrow::Table> ParquetFilterPrivate::readArrowTable(const QString& fileName) {
	auto infile_result = arrow::io::ReadableFile::Open(fileName.toStdString());
	if (!infile_result.ok()) {
		q->setLastError(i18n("Could not open file: %1", QString::fromStdString(infile_result.status().ToString())));
		return nullptr;
	}
	auto infile = *infile_result;

	std::shared_ptr<arrow::Table> table;

	if (fileType == AbstractFileFilter::FileType::Parquet || fileName.endsWith(QLatin1String(".parquet"), Qt::CaseInsensitive)
		|| fileName.endsWith(QLatin1String(".parq"), Qt::CaseInsensitive)) {
		// read Parquet
		auto reader_result = parquet::arrow::OpenFile(infile, arrow::default_memory_pool());
		if (!reader_result.ok()) {
			q->setLastError(i18n("Failed to open Parquet file: %1", QString::fromStdString(reader_result.status().ToString())));
			return nullptr;
		}
		auto reader = std::move(*reader_result);
		auto result = reader->ReadTable();
		if (!result.ok()) {
			q->setLastError(i18n("Failed to read Parquet table: %1", QString::fromStdString(result.status().ToString())));
			return nullptr;
		}
		table = *result;
	} else if (fileType == AbstractFileFilter::FileType::ArrowIPC || fileName.endsWith(QLatin1String(".feather"), Qt::CaseInsensitive)
			   || fileName.endsWith(QLatin1String(".arrow"), Qt::CaseInsensitive) || fileName.endsWith(QLatin1String(".ipc"), Qt::CaseInsensitive)) {
		// read Arrow IPC / Feather
		auto reader_result = arrow::ipc::RecordBatchFileReader::Open(infile);
		if (!reader_result.ok()) {
			q->setLastError(i18n("Failed to open Arrow IPC file: %1", QString::fromStdString(reader_result.status().ToString())));
			return nullptr;
		}
		auto reader = *reader_result;

		// read all record batches into a table
		std::vector<std::shared_ptr<arrow::RecordBatch>> batches;
		batches.reserve(reader->num_record_batches());
		for (int i = 0; i < reader->num_record_batches(); ++i) {
			auto batch_result = reader->ReadRecordBatch(i);
			if (!batch_result.ok()) {
				q->setLastError(i18n("Failed to read record batch %1: %2", i, QString::fromStdString(batch_result.status().ToString())));
				return nullptr;
			}
			batches.push_back(*batch_result);
		}
		auto table_result = arrow::Table::FromRecordBatches(reader->schema(), batches);
		if (!table_result.ok()) {
			q->setLastError(i18n("Failed to assemble table: %1", QString::fromStdString(table_result.status().ToString())));
			return nullptr;
		}
		table = *table_result;
	}
#ifdef HAVE_ORC
	else if (fileType == AbstractFileFilter::FileType::ORC || fileName.endsWith(QLatin1String(".orc"), Qt::CaseInsensitive)) {
		// Read ORC
		auto reader_result = arrow::adapters::orc::ORCFileReader::Open(infile, arrow::default_memory_pool());
		if (!reader_result.ok()) {
			q->setLastError(i18n("Failed to open ORC file: %1", QString::fromStdString(reader_result.status().ToString())));
			return nullptr;
		}
		auto reader = std::move(*reader_result);
		auto result = reader->Read();
		if (!result.ok()) {
			q->setLastError(i18n("Failed to read ORC table: %1", QString::fromStdString(result.status().ToString())));
			return nullptr;
		}
		table = *result;
	}
#endif

	if (!table) {
		q->setLastError(i18n("Unsupported file format."));
		return nullptr;
	}

	numColumns = table->num_columns();
	numRows = (int)table->num_rows();
	columnNames.clear();
	for (int i = 0; i < table->num_columns(); ++i)
		columnNames << QString::fromStdString(table->schema()->field(i)->name());

	return table;
}

static AbstractColumn::ColumnMode arrowTypeToColumnMode(const std::shared_ptr<arrow::DataType>& type) {
	switch (type->id()) {
	case arrow::Type::BOOL:
	case arrow::Type::INT8:
	case arrow::Type::INT16:
	case arrow::Type::INT32:
	case arrow::Type::UINT8:
	case arrow::Type::UINT16:
	case arrow::Type::UINT32:
		return AbstractColumn::ColumnMode::Integer;
	case arrow::Type::INT64:
	case arrow::Type::UINT64:
		return AbstractColumn::ColumnMode::BigInt;
	case arrow::Type::HALF_FLOAT:
	case arrow::Type::FLOAT:
	case arrow::Type::DOUBLE:
	case arrow::Type::DECIMAL128:
	case arrow::Type::DECIMAL256:
		return AbstractColumn::ColumnMode::Double;
	case arrow::Type::DATE32:
	case arrow::Type::DATE64:
	case arrow::Type::TIMESTAMP:
	case arrow::Type::TIME32:
	case arrow::Type::TIME64:
		return AbstractColumn::ColumnMode::DateTime;
	case arrow::Type::STRING:
	case arrow::Type::LARGE_STRING:
	case arrow::Type::BINARY:
	case arrow::Type::LARGE_BINARY:
	case arrow::Type::FIXED_SIZE_BINARY:
	default:
		return AbstractColumn::ColumnMode::Text;
	}
}

static QString arrowValueToString(const std::shared_ptr<arrow::Array>& array, int64_t row) {
	if (array->IsNull(row))
		return {};
	return QString::fromStdString(array->GetScalar(row).ValueOrDie()->ToString());
}

void ParquetFilterPrivate::importFromTable(const std::shared_ptr<arrow::Table>& table,
										   AbstractDataSource* dataSource,
										   AbstractFileFilter::ImportMode importMode) {
	if (!table || !dataSource)
		return;

	const int cols = table->num_columns();
	const int64_t rows = table->num_rows();

	// Determine which columns to import
	QVector<int> columnIndices;
	if (!selectedColumnNames.isEmpty()) {
		// use user-selected columns
		for (int col = 0; col < cols; ++col) {
			const auto name = QString::fromStdString(table->schema()->field(col)->name());
			if (selectedColumnNames.contains(name))
				columnIndices << col;
		}
	} else {
		// use start/end column range
		const int actualStartCol = startColumn;
		const int actualEndCol = (endColumn == -1 || endColumn > cols) ? cols : endColumn;
		for (int col = actualStartCol - 1; col < actualEndCol; ++col)
			columnIndices << col;
	}

	// Determine actual row range
	const int actualStartRow = startRow;
	const int actualEndRow = (endRow == -1 || endRow > (int)rows) ? (int)rows : endRow;

	const int actualRows = actualEndRow - actualStartRow + 1;
	const int actualCols = columnIndices.size();

	if (actualRows <= 0 || actualCols <= 0) {
		if (rows == 0)
			q->setLastError(i18n("The file has no data to import."));
		return;
	}

	// Prepare column names and modes
	QStringList vectorNames;
	QVector<AbstractColumn::ColumnMode> columnModes;
	for (int col : columnIndices) {
		vectorNames << QString::fromStdString(table->schema()->field(col)->name());
		columnModes << arrowTypeToColumnMode(table->schema()->field(col)->type());
	}

	// Prepare the data container
	std::vector<void*> dataContainer;
	bool ok = false;
	const int columnOffset = dataSource->prepareImport(dataContainer, importMode, actualRows, actualCols, vectorNames, columnModes, ok);
	if (!ok) {
		q->setLastError(i18n("Not enough memory."));
		return;
	}

	// Fill data
	for (int i = 0; i < actualCols; ++i) {
		const int col = columnIndices[i];
		auto chunkedArray = table->column(col);
		auto mode = columnModes[i];

		int64_t globalRow = 0;
		int destRow = 0;
		for (int chunkIdx = 0; chunkIdx < chunkedArray->num_chunks(); ++chunkIdx) {
			auto array = chunkedArray->chunk(chunkIdx);
			for (int64_t localRow = 0; localRow < array->length(); ++localRow, ++globalRow) {
				if (globalRow < actualStartRow - 1)
					continue;
				if (globalRow >= actualEndRow)
					break;

				if (array->IsNull(localRow)) {
					// Handle null values
					switch (mode) {
					case AbstractColumn::ColumnMode::Double:
						static_cast<QVector<double>*>(dataContainer[i])->operator[](destRow) = qQNaN();
						break;
					case AbstractColumn::ColumnMode::Integer:
						static_cast<QVector<int>*>(dataContainer[i])->operator[](destRow) = 0;
						break;
					case AbstractColumn::ColumnMode::BigInt:
						static_cast<QVector<qint64>*>(dataContainer[i])->operator[](destRow) = 0;
						break;
					case AbstractColumn::ColumnMode::Text:
						static_cast<QVector<QString>*>(dataContainer[i])->operator[](destRow) = QString();
						break;
					case AbstractColumn::ColumnMode::DateTime:
						static_cast<QVector<QDateTime>*>(dataContainer[i])->operator[](destRow) = QDateTime();
						break;
					case AbstractColumn::ColumnMode::Day:
					case AbstractColumn::ColumnMode::Month:
						break;
					}
				} else {
					switch (mode) {
					case AbstractColumn::ColumnMode::Double: {
						auto scalar = array->GetScalar(localRow).ValueOrDie();
						auto numericScalar = std::dynamic_pointer_cast<arrow::DoubleScalar>(scalar->CastTo(arrow::float64()).ValueOrDie());
						double val = numericScalar ? numericScalar->value : qQNaN();
						static_cast<QVector<double>*>(dataContainer[i])->operator[](destRow) = val;
						break;
					}
					case AbstractColumn::ColumnMode::Integer: {
						auto scalar = array->GetScalar(localRow).ValueOrDie();
						auto intScalar = std::dynamic_pointer_cast<arrow::Int32Scalar>(scalar->CastTo(arrow::int32()).ValueOrDie());
						int val = intScalar ? intScalar->value : 0;
						static_cast<QVector<int>*>(dataContainer[i])->operator[](destRow) = val;
						break;
					}
					case AbstractColumn::ColumnMode::BigInt: {
						auto scalar = array->GetScalar(localRow).ValueOrDie();
						auto bigIntScalar = std::dynamic_pointer_cast<arrow::Int64Scalar>(scalar->CastTo(arrow::int64()).ValueOrDie());
						qint64 val = bigIntScalar ? bigIntScalar->value : 0;
						static_cast<QVector<qint64>*>(dataContainer[i])->operator[](destRow) = val;
						break;
					}
					case AbstractColumn::ColumnMode::Text: {
						auto scalar = array->GetScalar(localRow).ValueOrDie();
						static_cast<QVector<QString>*>(dataContainer[i])->operator[](destRow) = QString::fromStdString(scalar->ToString());
						break;
					}
					case AbstractColumn::ColumnMode::DateTime: {
						auto scalar = array->GetScalar(localRow).ValueOrDie();
						// Arrow timestamps are stored as int64 (microseconds since epoch by default)
						auto ts = std::dynamic_pointer_cast<arrow::TimestampScalar>(scalar);
						if (ts) {
							// Convert to milliseconds
							int64_t ms = 0;
							switch (std::dynamic_pointer_cast<arrow::TimestampType>(ts->type)->unit()) {
							case arrow::TimeUnit::SECOND:
								ms = ts->value * 1000;
								break;
							case arrow::TimeUnit::MILLI:
								ms = ts->value;
								break;
							case arrow::TimeUnit::MICRO:
								ms = ts->value / 1000;
								break;
							case arrow::TimeUnit::NANO:
								ms = ts->value / 1000000;
								break;
							}
							static_cast<QVector<QDateTime>*>(dataContainer[i])->operator[](destRow) = QDateTime::fromMSecsSinceEpoch(ms, Qt::UTC);
						} else {
							// Date types
							auto dateScalar = std::dynamic_pointer_cast<arrow::Date32Scalar>(scalar);
							if (dateScalar)
								static_cast<QVector<QDateTime>*>(dataContainer[i])->operator[](destRow) =
									QDateTime(QDate::fromJulianDay(2440588 + dateScalar->value), QTime(0, 0), Qt::UTC); // days since 1970-01-01
							else
								static_cast<QVector<QDateTime>*>(dataContainer[i])->operator[](destRow) = QDateTime();
						}
						break;
					}
					case AbstractColumn::ColumnMode::Day:
					case AbstractColumn::ColumnMode::Month:
						break;
					}
				}
				++destRow;
			}
			if (globalRow >= actualEndRow)
				break;
		}

		Q_EMIT q->completed(100 * (i + 1) / actualCols);
	}

	dataSource->finalizeImport(columnOffset, 1, actualCols, QStringLiteral("yyyy-MM-dd hh:mm:ss.zzz"), importMode);
}

QVector<QStringList> ParquetFilterPrivate::previewFromTable(const std::shared_ptr<arrow::Table>& table, int lines) {
	QVector<QStringList> dataStrings;
	if (!table)
		return dataStrings;

	const int cols = table->num_columns();
	const int64_t rows = table->num_rows();

	// Determine which columns to show
	QVector<int> columnIndices;
	if (!selectedColumnNames.isEmpty()) {
		for (int col = 0; col < cols; ++col) {
			const auto name = QString::fromStdString(table->schema()->field(col)->name());
			if (selectedColumnNames.contains(name))
				columnIndices << col;
		}
	} else {
		const int actualStartCol = startColumn;
		const int actualEndCol = (endColumn == -1 || endColumn > cols) ? cols : endColumn;
		for (int col = actualStartCol - 1; col < actualEndCol; ++col)
			columnIndices << col;
	}

	if (columnIndices.isEmpty())
		return dataStrings;

	const int actualStartRow = startRow;
	const int actualEndRow = (endRow == -1 || endRow > (int)rows) ? (int)rows : endRow;
	const int previewRows = std::min(lines, actualEndRow - actualStartRow + 1);

	// Header row
	QStringList header;
	for (int col : columnIndices)
		header << QString::fromStdString(table->schema()->field(col)->name());
	dataStrings << header;

	// Data rows
	for (int row = 0; row < previewRows; ++row) {
		QStringList rowData;
		const int64_t tableRow = actualStartRow - 1 + row;
		for (int col : columnIndices) {
			auto chunkedArray = table->column(col);
			// Find the right chunk and local index
			int64_t remaining = tableRow;
			bool found = false;
			for (int chunkIdx = 0; chunkIdx < chunkedArray->num_chunks(); ++chunkIdx) {
				auto array = chunkedArray->chunk(chunkIdx);
				if (remaining < array->length()) {
					rowData << arrowValueToString(array, remaining);
					found = true;
					break;
				}
				remaining -= array->length();
			}
			if (!found)
				rowData << QString();
		}
		dataStrings << rowData;
	}

	return dataStrings;
}
#endif // HAVE_PARQUET

void ParquetFilterPrivate::readDataFromFile(const QString& fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode) {
#ifdef HAVE_PARQUET
	auto table = readArrowTable(fileName);
	if (!table)
		return;
	importFromTable(table, dataSource, importMode);
#else
	Q_UNUSED(fileName)
	Q_UNUSED(dataSource)
	Q_UNUSED(importMode)
#endif
}

QVector<QStringList> ParquetFilterPrivate::preview(const QString& fileName, int lines) {
#ifdef HAVE_PARQUET
	auto table = readArrowTable(fileName);
	if (!table)
		return {};
	return previewFromTable(table, lines);
#else
	Q_UNUSED(fileName)
	Q_UNUSED(lines)
	return {};
#endif
}

void ParquetFilterPrivate::write(const QString& /*fileName*/, AbstractDataSource* /*dataSource*/) {
	// TODO: export not yet implemented
}
