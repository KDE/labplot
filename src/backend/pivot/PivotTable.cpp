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

void PivotTable::setHorizontalHeaderModel(QAbstractItemModel* model) {
	Q_D(PivotTable);
	d->horizontalHeaderModel = dynamic_cast<HierarchicalHeaderModel*>(model);
}

void PivotTable::setVerticalHeaderModel(QAbstractItemModel* model) {
	Q_D(PivotTable);
	d->verticalHeaderModel = dynamic_cast<HierarchicalHeaderModel*>(model);
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
	, dataModel(new QStandardItemModel) {
// 	horizontalHeaderModel(new HierarchicalHeaderModel),
// 	verticalHeaderModel(new HierarchicalHeaderModel)
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

/*
QStringList PivotTablePrivate::members(const QString& dimension, PivotTable::SortType sort) {
	//check whether we have the members already
	if (m_members.contains(dimension))
		return m_members.value(dimension);

	QStringList members;

	//construct the query
	QString query;
	swich case (sort) {
	case PivotTable::NoSort:
		query = "SELECT DISTINCT " + dimension " FROM pivot;";
		break;
	case PivotTable::SortAscending:
		query = "SELECT DISTINCT " + dimension " FROM pivot ASC;";
		break;
	case PivotTable::SortDescending:
		query = "SELECT DISTINCT " + dimension " FROM pivot DESC;";
		break;
	}

	//execute the query
	QSqlQuery q;
	if (!q.exec(query)) {
		KMessageBox::error(nullptr, i18n("Failed to process the query.") + "\n" + q.lastError().databaseText());
		return members;
	}

	//copy the results to the string list
	while (query.next())
		members << query.value(0).toString();

	//cache the members of the new dimension
	m_members[dimension] = members;

	return members;
}
*/

void PivotTablePrivate::recalculate() {
	PERFTRACE(QLatin1String(Q_FUNC_INFO));
	WAIT_CURSOR;

	if (dataSourceType == PivotTable::DataSourceSpreadsheet && !m_dbCreated)
		createDb();

	if (!dataModel || !horizontalHeaderModel || !verticalHeaderModel) {
		RESET_CURSOR;
		return;
	}

	// clear the previous result
	dataModel->clear();
	horizontalHeaderModel->clear();
	verticalHeaderModel->clear();

	// nothing to do if no spreadsheet is set yet
	if (dataSourceType == PivotTable::DataSourceSpreadsheet && !dataSourceSpreadsheet) {
		Q_EMIT q->changed(); // notify about the new result
		RESET_CURSOR;
		return;
	}

	if (rows.isEmpty() && columns.isEmpty() && !showTotals) {
		Q_EMIT q->changed(); // notify about the new result
		RESET_CURSOR;
		return;
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
	QString query{QLatin1String("SELECT ")};
	QString groupByString;

	if (!showNulls) {
		// if we don't need to show combinations with empty intersections, put everything into GROUP BY
		if (!rows.isEmpty())
			groupByString = rows.join(QLatin1Char(','));

		if (!columns.isEmpty()) {
			if (!groupByString.isEmpty())
				groupByString += QLatin1Char(',');
			groupByString += columns.join(QLatin1Char(','));
		}

		if (!groupByString.isEmpty()) {
			query += groupByString;
			// if (showTotals)
			query += QLatin1String(", COUNT(*) FROM pivot");

			query += QLatin1String(" GROUP BY ") + groupByString;

			if (!sortDimension.isEmpty()) {
				switch (sortType) {
				case PivotTable::NoSort:
					query += QLatin1String(" ORDER BY ") + sortDimension;
					break;
				case PivotTable::SortAscending:
					query += QLatin1String(" ORDER BY ") + sortDimension + QLatin1String(" ASC");
					break;
				case PivotTable::SortDescending:
					query += QLatin1String(" ORDER BY ") + sortDimension + QLatin1String(" DESC");
					break;
				}
			}
		} else {
			// no dimensions selected, show totals only
			query += QLatin1String("COUNT(*) FROM pivot");
		}
	} else {

	}

	return query;
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

	DEBUG("number of columns " << columnsCount);
	DEBUG("number rows: " << recordsCount);
	DEBUG("number values: " << valuesCount);
	DEBUG("index of the first value column: " << firstValueIndex);

	// resize the data models and set the data
	if (columns.isEmpty() && rows.isEmpty()) { // no fields provided, show the total count only
		// resize the models
		verticalHeaderModel->setColumnCount(0);
		verticalHeaderModel->setRowCount(0);

		horizontalHeaderModel->setColumnCount(1);
		horizontalHeaderModel->setRowCount(1);

		dataModel->setColumnCount(1);
		dataModel->setRowCount(1);

		// set the value
		horizontalHeaderModel->setData(horizontalHeaderModel->index(0, 0), i18n("Totals"), Qt::DisplayRole);
		sqlQuery.next();
		dataModel->setItem(0, 0, new QStandardItem(sqlQuery.value(0).toString()));
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

		// set the values
		// TODO: only "Totals" value at the moment, needs to be extended later when we allow to add other values
		horizontalHeaderModel->setData(horizontalHeaderModel->index(0, 0), i18n("Totals"), Qt::DisplayRole);

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
	} else if (rows.isEmpty()) { // all selected field columns were put on colums
		// we have:
		// * all field colums on columns
		//   -> one column for the vertical header with the number of rows equal to the number of record sets in the result query
		// * only values on columns
		//   -> one row for the horizontal header with the number of columns equal to the number of values

		// resize the models
		verticalHeaderModel->setColumnCount(recordsCount);
		verticalHeaderModel->setRowCount(1);

		horizontalHeaderModel->setColumnCount(recordsCount);
		horizontalHeaderModel->setRowCount(columns.size());

		dataModel->setRowCount(valuesCount);
		dataModel->setColumnCount(recordsCount);

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

		horizontalHeaderModel->setSpan(0,1,columns.count(),0);
	} else {
	}
}

void PivotTablePrivate::createDb() {
	PERFTRACE(QLatin1String(Q_FUNC_INFO));
	for (auto* col : dataSourceSpreadsheet->children<Column>()) {
		if (col->isNumeric())
			measures << col->name();
		else
			dimensions << col->name();
	}

	m_dbCreated = dataSourceSpreadsheet->exportToSQLite(QString(), QLatin1String("pivot"));
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
	writer->writeAttribute(QStringLiteral("dataSourceType"), QString::number(d->dataSourceType));
	writer->writeAttribute(QStringLiteral("dataSourceSpreadsheet"), d->dataSourceSpreadsheet->path());
	WRITE_STRING_LIST("rows", d->rows);
	WRITE_STRING_LIST("columns", d->columns);
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
			} else { // unknown element
				reader->raiseUnknownElementWarning();
				if (!reader->skipToEndElement())
					return false;
			}
		}
	}

	return !reader->hasError();
}
