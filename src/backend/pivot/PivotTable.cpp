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
#include "frontend/pivot/PivotTableView.h"
#include "frontend/pivot/HierarchicalHeaderView.h"
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
PivotTable::PivotTable(const QString& name, bool loading) : AbstractPart(name, AspectType::PivotTable),
	d(new PivotTablePrivate(this)) {
	Q_UNUSED(loading)
}

//##############################################################################
//##########################  getter methods  ##################################
//##############################################################################
BASIC_D_READER_IMPL(PivotTable, PivotTable::DataSourceType, dataSourceType, dataSourceType)
BASIC_D_READER_IMPL(PivotTable, Spreadsheet*, dataSourceSpreadsheet, dataSourceSpreadsheet)
BASIC_SHARED_D_READER_IMPL(PivotTable, QString, dataSourceConnection, dataSourceConnection)
BASIC_SHARED_D_READER_IMPL(PivotTable, QString, dataSourceTable, dataSourceTable)

QAbstractItemModel* PivotTable::dataModel() const {
	return d->dataModel;
}

void PivotTable::setHorizontalHeaderModel(QAbstractItemModel* model) const {
	d->horizontalHeaderModel = dynamic_cast<HierarchicalHeaderModel*>(model);
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
	Q_EMIT requestProjectContextMenu(menu);
	return menu;
}

/*!
  Returns an icon to be used for decorating my views.
  */
QIcon PivotTable::icon() const {
	return QIcon::fromTheme(QLatin1String("labplot-spreadsheet"));
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
		Q_EMIT q->changed();
		return;
	}

	WAIT_CURSOR;

	if (dataSourceType == PivotTable::DataSourceSpreadsheet && !m_dbCreated)
		createDb();

	//construct the SQL statement string
	QString query{QLatin1String("SELECT ")};
	QString groupByString;

	if (!showNulls) {
		//if we don't need to show combinations with empty intersections, put everything into GROUP BY
		if (!rows.isEmpty())
			groupByString = rows.join(QLatin1Char(','));

		if (!columns.isEmpty()) {
			if (!groupByString.isEmpty())
				groupByString += QLatin1Char(',');
			groupByString += columns.join(QLatin1Char(','));
		}

		if (!groupByString.isEmpty()) {
			query += groupByString;
// 			if (showTotals)
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
			//no dimensions selected, show totals only
			query += QLatin1String("COUNT(*) FROM pivot");
		}
	} else {

	}

	QDEBUG(query);

	//execute the query
	QSqlQuery sqlQuery;
	if (!sqlQuery.exec(query)) {
		RESET_CURSOR;
		KMessageBox::error(nullptr, i18n("Failed to process the query.") + QLatin1Char('\n') + sqlQuery.lastError().databaseText());
		Q_EMIT q->changed();
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

	// qDebug()<<"model in recalculate " << horizontalHeaderModel;

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
		horizontalHeaderModel->setData(horizontalHeaderModel->index(0, 0), i18n("Totals"), Qt::DisplayRole);
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
		horizontalHeaderModel->setData(horizontalHeaderModel->index(0, 0), i18n("Totals"), Qt::DisplayRole);
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

		/*
		while (sqlQuery.next()) {
			qDebug()<<"row: " << row;
			horizontalHeaderModel->setRowCount(row+1);
			for (int i = 0; i < firstValueIndex; ++i) {
				// qDebug()<<"adding to the horizontal header " << sqlQuery.value(i);
				horizontalHeaderModel->setData(horizontalHeaderModel->index(row, i), sqlQuery.value(i), Qt::DisplayRole);
			}

			//values
			for (int i = firstValueIndex; i < columnsCount; ++i) {
				QString value = sqlQuery.value(i).toString();
				// qDebug()<<"adding value " << value;
				if (rowsCount == -1)
					dataModel->setRowCount(row + 1);
				dataModel->setItem(row, i - firstValueIndex, new QStandardItem(value));
			}

			++row;
		}
		*/

		int* start_span = new int[firstValueIndex];
		int* end_span = new int[firstValueIndex];
		QString* last_value= new QString[firstValueIndex];

		verticalHeaderModel->setRowCount(row+1);
		for (int i = 0; i < firstValueIndex; ++i) {
			start_span[i] = 1;
			end_span[i] = 1;
			last_value[i] = QLatin1String("");
			verticalHeaderModel->setData(verticalHeaderModel->index(row, i), rows.at(i), Qt::DisplayRole);
		}
		row++;

		while (sqlQuery.next()) {
			qDebug()<<"row: " << row;
			verticalHeaderModel->setRowCount(row+1);

//            if(sqlQuery.value(0).toString() != last_value)
//            {
//                if(end_span > start_span)
//                    verticalHeaderModel->setSpan(start_span,0,end_span-start_span,0);
//                start_span = end_span;
//                last_value = sqlQuery.value(0).toString();
//            }
//            end_span = end_span + 1;
			bool parent_header_changed = false;
			for (int i = 0; i < firstValueIndex; ++i) {
				QString queryVal = sqlQuery.value(i).toString();
				qDebug()<<"adding to the horizontal header " << query;

				if(queryVal != last_value[i] || parent_header_changed)
				{
					verticalHeaderModel->setData(verticalHeaderModel->index(row, i), queryVal, Qt::DisplayRole);

					if(end_span[i] > start_span[i]+1)
						verticalHeaderModel->setSpan(start_span[i],i,end_span[i]-start_span[i],0);
					start_span[i] = end_span[i];
					parent_header_changed = true;
					}
					last_value[i] = queryVal;
					end_span[i] = end_span[i] + 1;
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

		for(int i = 0; i < firstValueIndex; ++i){
			if(end_span[i] > start_span[i]){
				verticalHeaderModel->setSpan(start_span[i],i,end_span[i]-start_span[i],0);
			}
		}
		verticalHeaderModel->setSpan(1,0,0,rows.count());

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
	Q_EMIT q->changed();
	RESET_CURSOR;
}

void PivotTablePrivate::createDb() {
	for (auto* col : dataSourceSpreadsheet->children<Column>()) {
		if (col->isNumeric())
			measures << col->name();
		else
			dimensions << col->name();
	}

	m_dbCreated = dataSourceSpreadsheet->exportToSQLite(QString(), QLatin1String("pivot"));
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
/*!
  Saves as XML.
 */
void PivotTable::save(QXmlStreamWriter* writer) const {
	writer->writeStartElement(QLatin1String("pivotTable"));
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
