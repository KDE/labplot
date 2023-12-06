/*
	File                 : ImportSqlDatabaseTest.h
	Project              : LabPlot
	Description          : Tests for the import from SQL databases
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ImportSqlDatabaseTest.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "kdefrontend/datasources/ImportSQLDatabaseWidget.h"

#include <KConfig>
#include <KConfigGroup>

void ImportSqlDatabaseTest::initTestCase() {
	// prepare the database connection
	QString m_configPath(QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).constFirst() + QStringLiteral("sql_connections"));
	KConfig config(m_configPath, KConfig::SimpleConfig);
	KConfigGroup group = config.group(QStringLiteral("chinook"));
	group.writeEntry("Driver", QStringLiteral("QSQLITE"));
	group.writeEntry("DatabaseName", QFINDTESTDATA(QLatin1String("data/chinook.db")));

	// needed in order to have the signals triggered by SignallingUndoCommand, see LabPlot.cpp
	// TODO: redesign/remove this
	qRegisterMetaType<const AbstractAspect*>("const AbstractAspect*");
	qRegisterMetaType<const AbstractColumn*>("const AbstractColumn*");
}

// ##############################################################################
// ########################  import from a table ################################
// ##############################################################################

/*!
 * read the full table import in the replace mode
 */
void ImportSqlDatabaseTest::testFullTableReplace() {
	// prepare the target spreadsheet
	Spreadsheet spreadsheet(QStringLiteral("test"), false);

	// import the first table "artists"
	ImportSQLDatabaseWidget w;
	w.loadSettings();
	w.read(&spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// check the spreadsheet size and columns names and modes
	QCOMPARE(spreadsheet.rowCount(), 347);
	QCOMPARE(spreadsheet.columnCount(), 3);

	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("AlbumId"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("Title"));
	QCOMPARE(spreadsheet.column(2)->name(), QLatin1String("ArtistId"));

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Text);
	QCOMPARE(spreadsheet.column(2)->columnMode(), AbstractColumn::ColumnMode::Integer);
}

/*!
 * read the full table import in the append mode
 */
void ImportSqlDatabaseTest::testFullTableAppend() {
	// prepare the target spreadsheet
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	spreadsheet.setColumnCount(1);
	spreadsheet.column(0)->setName(QStringLiteral("test"));
	spreadsheet.column(0)->setColumnMode(AbstractColumn::ColumnMode::Double);

	// import the first table "artists"
	ImportSQLDatabaseWidget w;
	w.loadSettings();
	w.read(&spreadsheet, AbstractFileFilter::ImportMode::Append);

	// check the spreadsheet size and columns names and modes
	QCOMPARE(spreadsheet.rowCount(), 347);
	QCOMPARE(spreadsheet.columnCount(), 4);

	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("test"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("AlbumId"));
	QCOMPARE(spreadsheet.column(2)->name(), QLatin1String("Title"));
	QCOMPARE(spreadsheet.column(3)->name(), QLatin1String("ArtistId"));

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(2)->columnMode(), AbstractColumn::ColumnMode::Text);
	QCOMPARE(spreadsheet.column(3)->columnMode(), AbstractColumn::ColumnMode::Integer);
}

/*!
 * read the full table import in the prepand mode
 */
void ImportSqlDatabaseTest::testFullTablePrepend() {
	// prepare the target spreadsheet
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	spreadsheet.setColumnCount(1);
	spreadsheet.column(0)->setName(QStringLiteral("test"));
	spreadsheet.column(0)->setColumnMode(AbstractColumn::ColumnMode::Double);

	// import the first table "artists"
	ImportSQLDatabaseWidget w;
	w.loadSettings();
	w.read(&spreadsheet, AbstractFileFilter::ImportMode::Prepend);

	// check the spreadsheet size and columns names and modes
	QCOMPARE(spreadsheet.rowCount(), 347);
	QCOMPARE(spreadsheet.columnCount(), 4);

	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("AlbumId"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("Title"));
	QCOMPARE(spreadsheet.column(2)->name(), QLatin1String("ArtistId"));
	QCOMPARE(spreadsheet.column(3)->name(), QLatin1String("test"));

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Text);
	QCOMPARE(spreadsheet.column(2)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(3)->columnMode(), AbstractColumn::ColumnMode::Double);
}

void ImportSqlDatabaseTest::testFullTableCustomRowRange() {

}

void ImportSqlDatabaseTest::testFullTableCustomColumnRange() {

}

void ImportSqlDatabaseTest::testFullTableCustomRowColumnRange() {

}

// ##############################################################################
// ##################  import the result of a custom query ######################
// ##############################################################################
void ImportSqlDatabaseTest::testQuery() {

}

QTEST_MAIN(ImportSqlDatabaseTest)
