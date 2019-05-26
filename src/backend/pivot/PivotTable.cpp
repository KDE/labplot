/***************************************************************************
    File                 : PivotTable.cpp
    Project              : LabPlot
    Description          : Aspect providing pivot table functionality
    --------------------------------------------------------------------
    Copyright            : (C) 2019 Alexander Semke (alexander.semke@web.de)
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
#include "PivotTable.h"
#include "PivotTablePrivate.h"
#include "kdefrontend/pivot/PivotTableView.h"
#include "kdefrontend/pivot/HierarchicalHeaderView.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/trace.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/spreadsheet/Spreadsheet.h"

#include <QAbstractItemModel>
#include <QIcon>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QStandardItemModel>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>
#include <KMessageBox>

/*!
  \class PivotTable
  \brief Aspect providing a pivot table.

  \ingroup backend
*/
PivotTable::PivotTable(const QString& name, bool loading) : AbstractPart(name),
	d(new PivotTablePrivate(this)) {
	Q_UNUSED(loading)
}

//##############################################################################
//##########################  getter methods  ##################################
//##############################################################################
BASIC_D_READER_IMPL(PivotTable, PivotTable::DataSourceType, dataSourceType, dataSourceType)
BASIC_D_READER_IMPL(PivotTable, Spreadsheet*, dataSourceSpreadsheet, dataSourceSpreadsheet)
CLASS_D_READER_IMPL(PivotTable, QString, dataSourceConnection, dataSourceConnection)
CLASS_D_READER_IMPL(PivotTable, QString, dataSourceTable, dataSourceTable)

QAbstractItemModel* PivotTable::dataModel() const {
	return d->dataModel;
}

void PivotTable::setHorizontalHeaderModel(QAbstractItemModel* model) const {
	d->horizontalHeaderModel = dynamic_cast<HierarchicalHeaderModel*>(model);
	qDebug()<<"model in set " << d->horizontalHeaderModel << "  " << model;
}

void PivotTable::setVerticalHeaderModel(QAbstractItemModel* model) const {
	d->verticalHeaderModel = dynamic_cast<HierarchicalHeaderModel*>(model);
}

//##############################################################################
//#################  setter methods and undo commands ##########################
//##############################################################################
STD_SETTER_CMD_IMPL_F_S(PivotTable, SetDataSourceType, PivotTable::DataSourceType, dataSourceType, recalculate)
void PivotTable::setDataSourceType(DataSourceType type) {
	if (type != d->dataSourceType)
		exec(new PivotTableSetDataSourceTypeCmd(d, type, ki18n("%1: data source type changed")));
}

STD_SETTER_CMD_IMPL_F_S(PivotTable, SetDataSourceSpreadsheet, Spreadsheet*, dataSourceSpreadsheet, recalculate)
void PivotTable::setDataSourceSpreadsheet(Spreadsheet* spreadsheet) {
	if (spreadsheet != d->dataSourceSpreadsheet)
		exec(new PivotTableSetDataSourceSpreadsheetCmd(d, spreadsheet, ki18n("%1: data source spreadsheet changed")));
}

const QStringList& PivotTable::dimensions() const {
	return d->dimensions;
}

const QStringList& PivotTable::measures() const {
	return d->measures;
}

const QStringList& PivotTable::rows() const {
	return d->rows;
}

void PivotTable::addToRows(const QString& field) {
	d->addToRows(field);
}

void PivotTable::removeFromRows(const QString& field) {
	d->removeFromRows(field);
}

const QStringList& PivotTable::columns() const {
	return d->columns;
}

void PivotTable::addToColumns(const QString& dimension) {
	d->addToColumns(dimension);
}

void PivotTable::removeFromColumns(const QString& field) {
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
	emit requestProjectContextMenu(menu);
	return menu;
}

/*!
  Returns an icon to be used for decorating my views.
  */
QIcon PivotTable::icon() const {
	return QIcon::fromTheme("labplot-spreadsheet");
}

//##############################################################################
//######################  Private implementation ###############################
//##############################################################################
PivotTablePrivate::PivotTablePrivate(PivotTable* owner) : q(owner)
,
	dataModel(new QStandardItemModel)
// 	horizontalHeaderModel(new HierarchicalHeaderModel),
// 	verticalHeaderModel(new HierarchicalHeaderModel)
	{

}

PivotTablePrivate::~PivotTablePrivate() {

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
	//clear the previos result
	dataModel->clear();
	horizontalHeaderModel->clear();
	verticalHeaderModel->clear();

	//nothing to do if no spreadsheet is set yet
	if (dataSourceType == PivotTable::DataSourceSpreadsheet && !dataSourceSpreadsheet)
		return;

	if (rows.isEmpty() && columns.isEmpty() && !showTotals) {
		//notify about the new result
		emit q->changed();
		return;
	}

	WAIT_CURSOR;

	if (dataSourceType == PivotTable::DataSourceSpreadsheet && !m_dbCreated)
		createDb();

	//construct the SQL statement string
	QString query{"SELECT "};
	QString groupByString;

	if (!showNulls) {
		//if we don't need to show combinations with empty intersections, put everything into GROUP BY
		if (!rows.isEmpty())
			groupByString = rows.join(',');

		if (!columns.isEmpty()) {
			if (!groupByString.isEmpty())
				groupByString += ',';
			groupByString += columns.join(',');
		}

		if (!groupByString.isEmpty()) {
			query += groupByString;
// 			if (showTotals)
			query += ", COUNT(*) FROM pivot";

			query += " GROUP BY " + groupByString;

			if (!sortDimension.isEmpty()) {
				switch (sortType) {
				case PivotTable::NoSort:
					query += " ORDER BY " + sortDimension;
					break;
				case PivotTable::SortAscending:
					query += " ORDER BY " + sortDimension + " ASC";
					break;
				case PivotTable::SortDescending:
					query += " ORDER BY " + sortDimension + " DESC";
					break;
				}
			}
		} else {
			//no dimensions selected, show totals only
			query += "COUNT(*) FROM pivot";
		}
	} else {

	}

	QDEBUG(query);

	//execute the query
	QSqlQuery sqlQuery;
	if (!sqlQuery.exec(query)) {
		RESET_CURSOR;
		KMessageBox::error(nullptr, i18n("Failed to process the query.") + "\n" + sqlQuery.lastError().databaseText());
		emit q->changed();
		return;
	}

	//copy the result into the models
	int rowsCount = sqlQuery.size();
	int columnsCount = sqlQuery.record().count();
	int firstValueIndex = rows.size() + columns.size();
	int valuesCount = columnsCount - firstValueIndex;

	DEBUG("nubmer of columns " << columnsCount);
	DEBUG("number rows: " << rowsCount);
	DEBUG("number values: " << valuesCount);
	DEBUG("index of the first value column: " << firstValueIndex);

	qDebug()<<"model in recalculate " << horizontalHeaderModel;
	if (!horizontalHeaderModel) {
		RESET_CURSOR;
		return;
	}

	//resize the hierarhical header models
	if (columns.isEmpty() && rows.isEmpty()) {
		//no labels provided, show the total count only

		//vertical header
		verticalHeaderModel->setColumnCount(0);
		verticalHeaderModel->setRowCount(0);

		//horizontal header
		horizontalHeaderModel->setColumnCount(1);
		horizontalHeaderModel->setRowCount(1);
		horizontalHeaderModel->setData(horizontalHeaderModel->index(0, 0), "Totals", Qt::DisplayRole);
	} else if (columns.isEmpty()) {
		//no column labels provided, we have:
		//* all labels on rows
		//   -> one column for the vertical header with the number of rows equal to the number of record sets in the result query
		//* only values on columns
		//   -> one row for the horizontal header with the number of columns equal to the number of values

		//vertical header
		verticalHeaderModel->setColumnCount(rows.count());

		//horizontal header
		horizontalHeaderModel->setColumnCount(valuesCount);
		horizontalHeaderModel->setRowCount(1);

		//TODO: only "Totals" value at the moment, needs to be extended later when we allow to add other values
		horizontalHeaderModel->setData(horizontalHeaderModel->index(0, 0), "Totals", Qt::DisplayRole);
	} else if (rows.isEmpty()) {
		//no row labels provided, we have:
		//* all labels on rows
		//   -> one column for the vertical header with the number of rows equal to the number of record sets in the result query
		//* only values on columns
		//   -> one row for the horizontal header with the number of columns equal to the number of values

		//vertical header

		//horizontal header

// 		for (int i = 0; i < columns.size(); ++i)
// 			horizontalHeaderModel->setData(horizontalHeaderModel->index(0, i), columns.at(i), Qt::DisplayRole);

	} else {
		//TODO:
	}

	//handle the data model
	dataModel->setColumnCount(valuesCount);

	if (rowsCount != -1)
		dataModel->setRowCount(rowsCount);

	int row = 0;

	//add values to the data models
	if (columns.isEmpty() && rows.isEmpty()) {

	} else if (columns.isEmpty()) {
		qDebug()<<"everything on rows";
		while (sqlQuery.next()) {
			qDebug()<<"row: " << row;
			horizontalHeaderModel->setRowCount(row+1);
			for (int i = 0; i < firstValueIndex; ++i) {
				qDebug()<<"adding to the horizontal header " << sqlQuery.value(i);
				horizontalHeaderModel->setData(horizontalHeaderModel->index(row, i), sqlQuery.value(i), Qt::DisplayRole);
			}

			//values
			for (int i = firstValueIndex; i < columnsCount; ++i) {
				QString value = sqlQuery.value(i).toString();
				qDebug()<<"adding value " << value;
				if (rowsCount == -1)
					dataModel->setRowCount(row + 1);
				dataModel->setItem(row, i - firstValueIndex, new QStandardItem(value));
			}

			++row;
		}
	} else if (rows.isEmpty()) {
		qDebug()<<"everything on columns";
// 		for (int i = firstValueIndex; i < columnsCount; ++i) {
// 			QString value = sqlQuery.value(i).toString();
// 			if (rowsCount == -1)
// 				dataModel->setRowCount(row + 1);
// 			dataModel->setItem(0, i - firstValueIndex + row, new QStandardItem(value));
// 		}

	} else {
		//TODO
	}


	//notify about the new result
	emit q->changed();
	RESET_CURSOR;
}

void PivotTablePrivate::createDb() {
	for (auto* col : dataSourceSpreadsheet->children<Column>()) {
		if (col->isNumeric())
			measures << col->name();
		else
			dimensions << col->name();
	}

	PERFTRACE("export spreadsheet to SQLite database");
	QApplication::processEvents(QEventLoop::AllEvents, 0);

	//create database
	const QStringList& drivers = QSqlDatabase::drivers();
	QString driver;
	if (drivers.contains(QLatin1String("QSQLITE3")))
		driver = QLatin1String("QSQLITE3");
	else
		driver = QLatin1String("QSQLITE");

	QSqlDatabase db = QSqlDatabase::addDatabase(driver);
	db.open();

	//create table
	const int cols = dataSourceSpreadsheet->columnCount();
	QString query = QLatin1String("create table ") + QLatin1String("pivot") + QLatin1String(" (");
	for (int i = 0; i < cols; ++i) {
		Column* col = dataSourceSpreadsheet->column(i);
		if (i != 0)
			query += QLatin1String(", ");

		query += QLatin1String("\"") + col->name() + QLatin1String("\" ");
		switch (col->columnMode()) {
		case AbstractColumn::Numeric:
			query += QLatin1String("REAL");
			break;
		case AbstractColumn::Integer:
			query += QLatin1String("INTEGER");
			break;
		case AbstractColumn::Text:
		case AbstractColumn::Month:
		case AbstractColumn::Day:
		case AbstractColumn::DateTime:
			query += QLatin1String("TEXT");
			break;
		}
	}
	query += QLatin1Char(')');
	QSqlQuery q;
	if (!q.exec(query)) {
		RESET_CURSOR;
		KMessageBox::error(nullptr, i18n("Failed to create the SQLite database.") + "\n" + q.lastError().databaseText());
		db.close();
		return;
	}

	//create bulk insert statement
	{
	PERFTRACE("Create the bulk insert statement");
	q.exec(QLatin1String("BEGIN TRANSACTION;"));
	query = "INSERT INTO '" + QLatin1String("pivot") + "' (";
	for (int i = 0; i < cols; ++i) {
		if (i != 0)
			query += QLatin1String(", ");
		query += QLatin1Char('\'') + dataSourceSpreadsheet->column(i)->name() + QLatin1Char('\'');
	}
	query += QLatin1String(") VALUES ");

	for (int i = 0; i < dataSourceSpreadsheet->rowCount(); ++i) {
		if (i != 0)
			query += QLatin1String(",");

		query += QLatin1Char('(');
		for (int j = 0; j < cols; ++j) {
			Column* col = dataSourceSpreadsheet->column(j);
			if (j != 0)
				query += QLatin1String(", ");

			QString text = col->asStringColumn()->textAt(i);
			text = text.replace("'", "''");
			query += QLatin1Char('\'') + text + QLatin1Char('\'');
		}
		query += QLatin1String(")");
	}
	query += QLatin1Char(';');
	}

	//insert values
	if (!q.exec(query)) {
		RESET_CURSOR;
		KMessageBox::error(nullptr, i18n("Failed to insert values into the table."));
		QDEBUG(query);
		QDEBUG("bulk insert error " << q.lastError().databaseText());
	} else
		q.exec(QLatin1String("COMMIT TRANSACTION;"));

	m_dbCreated = true;
}


//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
/*!
  Saves as XML.
 */
void PivotTable::save(QXmlStreamWriter* writer) const {
	writer->writeStartElement("pivotTable");
	writeBasicAttributes(writer);
	writeCommentElement(writer);
	//TODO:

	writer->writeEndElement();
}

/*!
  Loads from XML.
*/
bool PivotTable::load(XmlStreamReader* reader, bool preview) {
	Q_UNUSED(preview);
	if (!readBasicAttributes(reader))
		return false;

	//TODO:

	return !reader->hasError();
}
