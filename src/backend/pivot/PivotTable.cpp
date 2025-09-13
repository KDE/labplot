/*
	File                 : PivotTable.cpp
	Project              : LabPlot
	Description          : Aspect providing pivot table functionality
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2019-2025 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "PivotTable.h"
#include "PivotTablePrivate.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/macros.h"
#include "backend/lib/trace.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "frontend/pivot/HierarchicalHeaderView.h"
#include "frontend/pivot/PivotTableView.h"

#include <QIcon>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QStandardItemModel>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KMessageBox>
#include <KSharedConfig>

/*!
  \class PivotTable
  \brief Aspect providing a pivot table.

  \ingroup backend
*/
PivotTable::PivotTable(const QString& name, bool loading)
	: AbstractPart(name, AspectType::PivotTable)
	, d_ptr(new PivotTablePrivate(this)) {
	Q_UNUSED(loading)
}

// ##############################################################################
// ##########################  getter methods  ##################################
// ##############################################################################
BASIC_D_READER_IMPL(PivotTable, PivotTable::DataSourceType, dataSourceType, dataSourceType)
BASIC_SHARED_D_READER_IMPL(PivotTable, const Spreadsheet*, dataSourceSpreadsheet, dataSourceSpreadsheet)

QString PivotTable::dataSourceSpreadsheetPath() const {
	Q_D(const PivotTable);
	return d->dataSourceSpreadsheetPath;
}

BASIC_SHARED_D_READER_IMPL(PivotTable, QString, dataSourceConnection, dataSourceConnection)
BASIC_SHARED_D_READER_IMPL(PivotTable, QString, dataSourceTable, dataSourceTable)

QAbstractItemModel* PivotTable::dataModel() const {
	Q_D(const PivotTable);
	return d->dataModel;
}
HierarchicalHeaderModel* PivotTable::horizontalHeaderModel() const {
	Q_D(const PivotTable);
	return d->horizontalHeaderModel;
}
HierarchicalHeaderModel* PivotTable::verticalHeaderModel() const {
	Q_D(const PivotTable);
	return d->verticalHeaderModel;
}

// ##############################################################################
// #################  setter methods and undo commands ##########################
// ##############################################################################
STD_SETTER_CMD_IMPL_F_S(PivotTable, SetDataSourceType, PivotTable::DataSourceType, dataSourceType, recalculate)
void PivotTable::setDataSourceType(DataSourceType type) {
	Q_D(PivotTable);
	if (type != d->dataSourceType)
		exec(new PivotTableSetDataSourceTypeCmd(d, type, ki18n("%1: data source type changed")));
}

STD_SETTER_CMD_IMPL_F_S(PivotTable, SetDataSourceSpreadsheet, const Spreadsheet*, dataSourceSpreadsheet, recalculate)
void PivotTable::setDataSourceSpreadsheet(const Spreadsheet* spreadsheet) {
	Q_D(PivotTable);
	if (spreadsheet != d->dataSourceSpreadsheet)
		exec(new PivotTableSetDataSourceSpreadsheetCmd(d, spreadsheet, ki18n("%1: data source spreadsheet changed")));
}

const QStringList& PivotTable::dimensions() const {
	Q_D(const PivotTable);
	return d->dimensions;
}

const QStringList& PivotTable::measures() const {
	Q_D(const PivotTable);
	return d->measures;
}

const QStringList& PivotTable::rows() const {
	Q_D(const PivotTable);
	return d->rows;
}

void PivotTable::addToRows(const QString& field) {
	Q_D(PivotTable);
	d->addToRows(field);
}

void PivotTable::removeFromRows(const QString& field) {
	Q_D(PivotTable);
	d->removeFromRows(field);
}

const QStringList& PivotTable::columns() const {
	Q_D(const PivotTable);
	return d->columns;
}

void PivotTable::addToColumns(const QString& dimension) {
	Q_D(PivotTable);
	d->addToColumns(dimension);
}

void PivotTable::removeFromColumns(const QString& field) {
	Q_D(PivotTable);
	d->removeFromColumns(field);
}

BASIC_SHARED_D_READER_IMPL(PivotTable, QVector<PivotTable::Value>, values, values)

STD_SETTER_CMD_IMPL_F_S(PivotTable, SetValues, QVector<PivotTable::Value>, values, recalculate)
void PivotTable::setValues(const QVector<Value> values) {
	Q_D(PivotTable);
	if (values != d->values)
		exec(new PivotTableSetValuesCmd(d, values, ki18n("%1: set values")));
}
bool PivotTable::exportView() const {
	return true;
}

bool PivotTable::printView() {
	return true;
}

bool PivotTable::printPreview() const {
	return true;
}

/*! Constructs a primary view on me.
  This method may be called multiple times during the life time of an Aspect, or it might not get
  called at all. Aspects must not depend on the existence of a view for their operation.
*/
QWidget* PivotTable::view() const {
	if (!m_partView) {
		m_view = new PivotTableView(const_cast<PivotTable*>(this));
		m_partView = m_view;
	}
	return m_partView;
}

/*!
  Returns a new context menu. The caller takes ownership of the menu.
*/
QMenu* PivotTable::createContextMenu() {
	QMenu* menu = AbstractPart::createContextMenu();
	Q_ASSERT(menu);
	Q_EMIT requestProjectContextMenu(menu);
	return menu;
}

/*!
  Returns an icon to be used for decorating my views.
  */
QIcon PivotTable::icon() const {
	return QIcon::fromTheme(QLatin1String("labplot-spreadsheet"));
}

// ##############################################################################
// ######################  Private implementation ###############################
// ##############################################################################
PivotTablePrivate::PivotTablePrivate(PivotTable* owner)
	: q(owner)
	, dataModel(new QStandardItemModel)
	, horizontalHeaderModel(new HierarchicalHeaderModel)
	, verticalHeaderModel(new HierarchicalHeaderModel) {
}

QString PivotTablePrivate::name() const {
	return q->name();
}

void PivotTablePrivate::addToRows(const QString& field) {
	rows << field;
	recalculate();
}

void PivotTablePrivate::removeFromRows(const QString& field) {
	rows.removeOne(field);
	recalculate();
}

void PivotTablePrivate::addToColumns(const QString& dimension) {
	columns << dimension;
	recalculate();
}

void PivotTablePrivate::removeFromColumns(const QString& field) {
	columns.removeOne(field);
	recalculate();
}

void PivotTablePrivate::recalculate() {
	PERFTRACE(QLatin1String(Q_FUNC_INFO));
	WAIT_CURSOR;

	if (!dataModel || !horizontalHeaderModel || !verticalHeaderModel) {
		RESET_CURSOR;
		return;
	}

	// clear the previous result
	dataModel->clear();
	horizontalHeaderModel->clear();
	verticalHeaderModel->clear();

	// nothing to do if no spreadsheet is set yet
	if (dataSourceType == PivotTable::DataSourceType::Spreadsheet && !dataSourceSpreadsheet) {
		Q_EMIT q->changed(); // notify about the new result
		RESET_CURSOR;
		return;
	}

	if (rows.isEmpty() && columns.isEmpty() && !showTotals) {
		Q_EMIT q->changed(); // notify about the new result
		RESET_CURSOR;
		return;
	}

	if (dataSourceType == PivotTable::DataSourceType::Spreadsheet && m_dbTableName != dataSourceSpreadsheet->uuid().toString(QUuid::Id128)) {
		createDb();
		if (m_dbTableName.isEmpty()) {
			Q_EMIT q->changed(); // notify about the new result
			RESET_CURSOR;
			return;
		}
	}

	// construct and execute the SQL query
	QSqlQuery sqlQuery;
	QString query = createSQLQuery();
	QDEBUG(query);
	if (!sqlQuery.exec(query)) {
		RESET_CURSOR;
		KMessageBox::error(nullptr, i18n("Failed to process the query.") + QLatin1Char('\n') + sqlQuery.lastError().databaseText());
		Q_EMIT q->changed();
		return;
	}

	// copy the result into the data models
	populateDataModels(sqlQuery);

	// notify about the new result
	Q_EMIT q->changed();
	RESET_CURSOR;
}

QString PivotTablePrivate::createSQLQuery() const {
	PERFTRACE(QLatin1String(Q_FUNC_INFO));

	// SELECT part
	// add values to the selection
	QString valuesString;
	if ((!values.isEmpty())) {
		QStringList valuesList;
		for (const auto& value : values) {
			switch (value.aggregation) {
			case PivotTable::Aggregation::Count:
				valuesList << QLatin1String("COUNT(") + value.name + QLatin1Char(')');
				break;
			case PivotTable::Aggregation::Sum:
				valuesList << QLatin1String("SUM(") + value.name + QLatin1Char(')');
				break;
			case PivotTable::Aggregation::Min:
				valuesList << QLatin1String("MIN(") + value.name + QLatin1Char(')');
				break;
			case PivotTable::Aggregation::Max:
				valuesList << QLatin1String("MAX(") + value.name + QLatin1Char(')');
				break;
			case PivotTable::Aggregation::Avg:
				valuesList << QLatin1String("AVG(") + value.name + QLatin1Char(')');
				break;
			}
		}

		valuesString = valuesList.join(QLatin1Char(','));
	} else
		valuesString = QLatin1String("COUNT(*)");

	// add dimensions to the selection
	QString dimensionsString;
	if (!rows.isEmpty())
		dimensionsString = rows.join(QLatin1Char(','));

	if (!columns.isEmpty()) {
		if (!dimensionsString.isEmpty())
			dimensionsString += QLatin1Char(',');
		dimensionsString += columns.join(QLatin1Char(','));
	}

	// SELECT part
	QString query = QLatin1String("SELECT ");
	if (!dimensionsString.isEmpty())
		query += dimensionsString + QLatin1Char(',');

	query += valuesString;

	// FROM part
	query += QLatin1String(" FROM ") + m_dbTableName;

	// WHERE part
	// TODO: implement filtering

	// GROUP BY part
	if (!dimensionsString.isEmpty())
		query += QLatin1String(" GROUP BY ") + dimensionsString;

	// ORDER BY part
	if (!sortDimension.isEmpty()) {
		switch (sortType) {
		case PivotTable::Sort::NoSort:
			query += QLatin1String(" ORDER BY ") + sortDimension;
			break;
		case PivotTable::Sort::Ascending:
			query += QLatin1String(" ORDER BY ") + sortDimension + QLatin1String(" ASC");
			break;
		case PivotTable::Sort::Descending:
			query += QLatin1String(" ORDER BY ") + sortDimension + QLatin1String(" DESC");
			break;
		}
	}

	QDEBUG(query);
	return query;
}

QString PivotTablePrivate::headerText(PivotTable::Value value) const {
	switch (value.aggregation) {
	case PivotTable::Aggregation::Count: {
		if (value.name.isEmpty())
			return i18n("Count");
		else
			return i18n("Count of %1", value.name);
	}
	case PivotTable::Aggregation::Sum:
		return i18n("Sum of %1", value.name);
	case PivotTable::Aggregation::Min:
		return i18n("Minimum of %1", value.name);
	case PivotTable::Aggregation::Max:
		return i18n("Maximum of %1", value.name);
	case PivotTable::Aggregation::Avg:
		return i18n("Average of %1", value.name);
	}
	return QString();
}

// copy the result into the models
void PivotTablePrivate::populateDataModels(QSqlQuery sqlQuery) {
	PERFTRACE(QLatin1String(Q_FUNC_INFO));

	// navigate to the last record to get the total number of records in the resultset
	sqlQuery.last();
	const int recordsCount = sqlQuery.at() + 1;
	sqlQuery.first();
	sqlQuery.previous(); // navigate in front of the first record so we also read it below in the whie loop

	const int columnsCount = sqlQuery.record().count(); // total number of columns in the result set
	const int firstValueIndex = rows.size() + columns.size(); // index of the first value column (the first column with a value, not a field name
	const int valuesCount = columnsCount - firstValueIndex; // number of value columns (the columns with values, not field names)
	Q_ASSERT(valuesCount == values.size()); // the number of value columns must be equal to the number of values

	DEBUG("number of columns " << columnsCount);
	DEBUG("number rows: " << recordsCount);
	DEBUG("number values: " << valuesCount);
	DEBUG("index of the first value column: " << firstValueIndex);

	// resize the data models and set the data
	if (columns.isEmpty() && rows.isEmpty()) { // no fields provided, show the values only
		// resize the models
		verticalHeaderModel->setColumnCount(0);
		verticalHeaderModel->setRowCount(0);

		horizontalHeaderModel->setColumnCount(valuesCount);
		horizontalHeaderModel->setRowCount(1);

		dataModel->setColumnCount(valuesCount);
		dataModel->setRowCount(1);

		// set the values and their descriptions
		sqlQuery.next();
		for (int i = 0; i < valuesCount; ++i) {
			horizontalHeaderModel->setData(horizontalHeaderModel->index(0, i), headerText(values.at(i)), Qt::DisplayRole);
			dataModel->setItem(0, i, new QStandardItem(sqlQuery.value(i).toString()));
		}
	} else if (columns.isEmpty()) { // all selected field columns were put on rows
		// we have:
		// * all field columns on rows
		//   -> one column for the vertical header with the number of rows equal to the number of record sets in the result query
		// * only values on columns
		//   -> one row for the horizontal header with the number of columns equal to the number of values

		// resize the models
		verticalHeaderModel->setColumnCount(rows.count());
		verticalHeaderModel->setRowCount(recordsCount);

		horizontalHeaderModel->setColumnCount(valuesCount);
		horizontalHeaderModel->setRowCount(1);

		dataModel->setColumnCount(valuesCount);
		if (recordsCount != -1)
			dataModel->setRowCount(recordsCount);

		// set the description of the values
		for (int i = 0; i < valuesCount; ++i)
			horizontalHeaderModel->setData(horizontalHeaderModel->index(0, i), headerText(values.at(i)), Qt::DisplayRole);

		QVector<int> start_span(firstValueIndex, 0);
		QVector<int> end_span(firstValueIndex, 0);
		QVector<QString> last_value(firstValueIndex, QString());

		int row = 0;
		while (sqlQuery.next()) {
			bool parent_header_changed = false;
			for (int i = 0; i < firstValueIndex; ++i) {
				const auto& value = sqlQuery.value(i).toString();
				if (row == 0 || value != last_value[i] || parent_header_changed) {
					// set the span for the previous group if needed
					if (row > 0 && end_span[i] > start_span[i]) {
						int span = end_span[i] - start_span[i];
						if (span > 1)
							verticalHeaderModel->setSpan(start_span[i], i, span, 1);
					}
					start_span[i] = row;
					parent_header_changed = true;
				}
				verticalHeaderModel->setData(verticalHeaderModel->index(row, i), value, Qt::DisplayRole);
				last_value[i] = value;
				end_span[i] = row + 1;
			}

			// values
			for (int i = firstValueIndex; i < columnsCount; ++i) {
				const auto& value = sqlQuery.value(i).toString();
				dataModel->setItem(row, i - firstValueIndex, new QStandardItem(value));
			}
			++row;
		}

		// finalize spans for the last group
		for (int i = 0; i < firstValueIndex; ++i) {
			int span = end_span[i] - start_span[i];
			if (span > 1)
				verticalHeaderModel->setSpan(start_span[i], i, span, 1);
		}

		verticalHeaderModel->setSpan(1, 0, 0, rows.count());
	} else if (rows.isEmpty()) { // all selected field columns were put on columns
		// we have:
		// * all field columns on columns
		//   -> one column for the vertical header with the number of rows equal to the number of record sets in the result query
		// * only values on columns
		//   -> one row for the horizontal header with the number of columns equal to the number of values

		// resize the models
		verticalHeaderModel->setColumnCount(1);
		verticalHeaderModel->setRowCount(valuesCount);

		horizontalHeaderModel->setColumnCount(recordsCount);
		horizontalHeaderModel->setRowCount(columns.count());

		dataModel->setColumnCount(recordsCount);
		dataModel->setRowCount(valuesCount);

		// set the description of the values
		for (int i = 0; i < valuesCount; ++i)
			verticalHeaderModel->setData(verticalHeaderModel->index(i, 0), headerText(values.at(i)), Qt::DisplayRole);

		// set the values
		QVector<int> start_span(firstValueIndex, 0);
		QVector<int> end_span(firstValueIndex, 0);
		QVector<QString> last_value(firstValueIndex, QString());

		int col = 0;
		while (sqlQuery.next()) {
			bool parent_header_changed = false;
			for (int i = 0; i < firstValueIndex; ++i) {
				const auto& value = sqlQuery.value(i).toString();
				if (col == 0 || value != last_value[i] || parent_header_changed) {
					// Set span for previous group if needed
					if (col > 0 && end_span[i] > start_span[i]) {
						int span = end_span[i] - start_span[i];
						if (span > 1)
							horizontalHeaderModel->setSpan(i, start_span[i], 1, span);
					}
					start_span[i] = col;
					parent_header_changed = true;
				}
				horizontalHeaderModel->setData(horizontalHeaderModel->index(i, col), value, Qt::DisplayRole);
				last_value[i] = value;
				end_span[i] = col + 1;
			}

			// values
			for (int i = firstValueIndex; i < columnsCount; ++i) {
				const auto& value = sqlQuery.value(i).toString();
				dataModel->setItem(i - firstValueIndex, col, new QStandardItem(value));
			}
			++col;
		}
		// finalize spans for the last group
		for (int i = 0; i < firstValueIndex; ++i) {
			int span = end_span[i] - start_span[i];
			if (span > 1)
				horizontalHeaderModel->setSpan(i, start_span[i], 1, span);
		}

		horizontalHeaderModel->setSpan(0, 1, columns.count(), 0);
	} else {
	}
}

void PivotTablePrivate::createDb() {
	PERFTRACE(QLatin1String(Q_FUNC_INFO));
	measures.clear();
	dimensions.clear();
	for (const auto* col : dataSourceSpreadsheet->children<Column>()) {
		if (col->isNumeric())
			measures << col->name();
		else
			dimensions << col->name();
	}

	m_dbTableName = dataSourceSpreadsheet->uuid().toString(QUuid::Id128);
	if (!QSqlDatabase::database().tables().contains(m_dbTableName)) {
		const bool rc = dataSourceSpreadsheet->exportToSQLite(QString(), m_dbTableName);
		if (!rc)
			m_dbTableName = QString(); // reset the table name since the table creation has failed
	}
}

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################
/*!
  Saves as XML.
 */
void PivotTable::save(QXmlStreamWriter* writer) const {
	Q_D(const PivotTable);
	writer->writeStartElement(QStringLiteral("pivotTable"));
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	writer->writeStartElement(QStringLiteral("general"));
	writer->writeAttribute(QStringLiteral("dataSourceType"), QString::number(static_cast<int>(d->dataSourceType)));
	writer->writeAttribute(QStringLiteral("dataSourceSpreadsheet"), d->dataSourceSpreadsheet->path());
	WRITE_STRING_LIST("rows", d->rows);
	WRITE_STRING_LIST("columns", d->columns);
	// WRITE_STRING_LIST("values", d->values);
	writer->writeEndElement();
}

/*!
  Loads from XML.
*/
bool PivotTable::load(XmlStreamReader* reader, bool preview) {
	Q_D(PivotTable);
	Q_UNUSED(preview);
	if (!readBasicAttributes(reader))
		return false;

	QString str;
	QXmlStreamAttributes attribs;

	while (!reader->atEnd()) {
		reader->readNext();

		if (reader->isEndElement() && reader->name() == QStringLiteral("pivotTable"))
			break;

		if (reader->isStartElement()) {
			if (reader->name() == QStringLiteral("comment")) {
				if (!readCommentElement(reader))
					return false;
			} else if (reader->name() == QStringLiteral("general")) {
				attribs = reader->attributes();
				READ_INT_VALUE("dataSourceType", dataSourceType, PivotTable::DataSourceType);
				str = attribs.value(QStringLiteral("dataSourceSpreadsheet")).toString();
				d->dataSourceSpreadsheetPath = str;
				READ_STRING_LIST("rows", d->rows);
				READ_STRING_LIST("columns", d->columns);
				// READ_STRING_LIST("values", d->values);
			} else { // unknown element
				reader->raiseUnknownElementWarning();
				if (!reader->skipToEndElement())
					return false;
			}
		}
	}

	return !reader->hasError();
}
