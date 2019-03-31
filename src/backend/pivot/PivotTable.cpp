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
#include "backend/lib/commandtemplates.h"
#include "backend/lib/trace.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/spreadsheet/Spreadsheet.h"

#include <QAbstractItemModel>
#include <QIcon>
#include <QSqlQuery>
#include <QSqlError>

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

// BASIC_SHARED_D_READER_IMPL(PivotTable, const QAbstractItemModel*, dataModel, dataModel)
// BASIC_SHARED_D_READER_IMPL(PivotTable, const QAbstractItemModel*, horizontalHeaderModel, horizontalHeaderModel)
// BASIC_SHARED_D_READER_IMPL(PivotTable, const QAbstractItemModel*, verticalHeaderModel, verticalHeaderModel)
QAbstractItemModel* PivotTable::dataModel() const {
	return d->dataModel;
}

QAbstractItemModel* PivotTable::horizontalHeaderModel() const {
	return d->horizontalHeaderModel;
}

QAbstractItemModel* PivotTable::verticalHeaderModel() const {
	return d->verticalHeaderModel;
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

void PivotTable::addToRowLabels(const QString& dimension) {
	d->addToRowLabels(dimension);
}

void PivotTable::addToColumnLabels(const QString& dimension) {
	d->addToColumnLabels(dimension);
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
	/*TODO: dataModel(new QAbstractItemModel),
	horizontalHeaderModel(new QAbstractItemModel),
	verticalHeaderModel(new QAbstractItemModel) */{

}

PivotTablePrivate::~PivotTablePrivate() {

}

QString PivotTablePrivate::name() const {
	return q->name();
}

void PivotTablePrivate::addToRowLabels(const QString& dimension) {
	m_rowLabels << dimension;
	recalculate();
}

void PivotTablePrivate::addToColumnLabels(const QString& dimension) {
	m_columnLabels << dimension;
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
// 	horizontalHeaderModel->clear();
// 	verticalHeaderModel->clear();
// 	dataModel->clear();

	if (dataSourceType == PivotTable::DataSourceSpreadsheet && !dataSourceSpreadsheet)
		return;

	if (m_rowLabels.isEmpty() && m_columnLabels.isEmpty()) {
		//notify about the new result
// 		emit q->changed();
		return;
	}

	WAIT_CURSOR;

	if (dataSourceType == PivotTable::DataSourceSpreadsheet && dataSourceSpreadsheet && !m_dbCreated)
		createDb();

	//construct the SQL statement string
	QString query{"SELECT "};
	QString dimensionString;

	if (!showNulls) {
		//if we don't need to show combinations with empty intersections, put everything into GROUP BY
		if (!m_rowLabels.isEmpty())
			dimensionString = m_rowLabels.join(',');

		if (!m_columnLabels.isEmpty()) {
			if (!dimensionString.isEmpty())
				dimensionString += ',';
			dimensionString = m_columnLabels.join(',');
		}

		query += dimensionString;
		query += dimensionString + ", COUNT(*) FROM pivot GROUP BY " + dimensionString;

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

	}

	//execute the query
	QSqlQuery sqlQuery;
	if (!sqlQuery.exec(query)) {
		RESET_CURSOR;
		KMessageBox::error(nullptr, i18n("Failed to process the query.") + "\n" + sqlQuery.lastError().databaseText());
// 		emit q->changed();
		return;
	}

	//copy the result into the models
    while (sqlQuery.next()) {
		//TODO:
    }

	//notify about the new result
// 	emit q->changed();
	RESET_CURSOR;
}


void PivotTablePrivate::createDb() {
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

	//create table
	const int cols = dataSourceSpreadsheet->columnCount();
	QString query = QLatin1String("create table ") + dataSourceSpreadsheet->name() + QLatin1String(" (");
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
	query = "INSERT INTO '" + dataSourceSpreadsheet->name() + "' (";
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

			query += QLatin1Char('\'') + col->asStringColumn()->textAt(i) + QLatin1Char('\'');
		}
		query += QLatin1String(")");
	}
	query += QLatin1Char(';');
	}

	//insert values
	if (!q.exec(query)) {
		RESET_CURSOR;
		KMessageBox::error(nullptr, i18n("Failed to insert values into the table."));
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
	if (!readBasicAttributes(reader))
		return false;

	//TODO:

	return !reader->hasError();
}
