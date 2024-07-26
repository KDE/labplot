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
	CommonMetaTest::initTestCase();

	// prepare the database connection
	QString m_configPath(QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).constFirst() + QStringLiteral("sql_connections"));
	KConfig config(m_configPath, KConfig::SimpleConfig);
	KConfigGroup group = config.group(QStringLiteral("chinook"));
	group.writeEntry("Driver", QStringLiteral("QSQLITE"));
	group.writeEntry("DatabaseName", QFINDTESTDATA(QLatin1String("data/chinook.db")));
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
	w.setCustomQuery(false);
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
	w.setCustomQuery(false);
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
	w.setCustomQuery(false);
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
	// prepare the target spreadsheet
	Spreadsheet spreadsheet(QStringLiteral("test"), false);

	// import the records from 10 to 20 from the first table "artists"
	ImportSQLDatabaseWidget w;
	w.loadSettings();
	w.setCustomQuery(false);
	w.setStartRow(10);
	w.setEndRow(20);
	w.read(&spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// check the spreadsheet size and columns names and modes
	QCOMPARE(spreadsheet.rowCount(), 11);
	QCOMPARE(spreadsheet.columnCount(), 3);

	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("AlbumId"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("Title"));
	QCOMPARE(spreadsheet.column(2)->name(), QLatin1String("ArtistId"));

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Text);
	QCOMPARE(spreadsheet.column(2)->columnMode(), AbstractColumn::ColumnMode::Integer);

	// first row in the spreadsheet
	QCOMPARE(spreadsheet.column(0)->integerAt(0), 10);
	QCOMPARE(spreadsheet.column(1)->textAt(0), QLatin1String("Audioslave"));
	QCOMPARE(spreadsheet.column(2)->integerAt(0), 8);

	// last row in the spreadsheet
	QCOMPARE(spreadsheet.column(0)->integerAt(10), 20);
	QCOMPARE(spreadsheet.column(1)->textAt(10), QLatin1String("The Best Of Buddy Guy - The Millenium Collection"));
	QCOMPARE(spreadsheet.column(2)->integerAt(10), 15);
}

/*!
 * import the first two columns only
 */
void ImportSqlDatabaseTest::testFullTableCustomColumnRange01() {
	// prepare the target spreadsheet
	Spreadsheet spreadsheet(QStringLiteral("test"), false);

	// import the records from 10 to 20 from the first table "artists"
	ImportSQLDatabaseWidget w;
	w.loadSettings();
	w.setCustomQuery(false);
	w.setStartColumn(1);
	w.setEndColumn(2);
	w.read(&spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// check the spreadsheet size and columns names and modes
	QCOMPARE(spreadsheet.rowCount(), 347);
	QCOMPARE(spreadsheet.columnCount(), 2);

	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("AlbumId"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("Title"));

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Text);

	// first row in the spreadsheet
	QCOMPARE(spreadsheet.column(0)->integerAt(0), 1);
	QCOMPARE(spreadsheet.column(1)->textAt(0), QLatin1String("For Those About To Rock We Salute You"));

	// last row in the spreadsheet
	QCOMPARE(spreadsheet.column(0)->integerAt(346), 347);
	QCOMPARE(spreadsheet.column(1)->textAt(346), QLatin1String("Koyaanisqatsi (Soundtrack from the Motion Picture)"));
}

/*!
 * import the last two columns only
 */
void ImportSqlDatabaseTest::testFullTableCustomColumnRange02() {
	// prepare the target spreadsheet
	Spreadsheet spreadsheet(QStringLiteral("test"), false);

	// import the records from 10 to 20 from the first table "artists"
	ImportSQLDatabaseWidget w;
	w.loadSettings();
	w.setCustomQuery(false);
	w.setStartColumn(2);
	w.setEndColumn(3);
	w.read(&spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// check the spreadsheet size and columns names and modes
	QCOMPARE(spreadsheet.rowCount(), 347);
	QCOMPARE(spreadsheet.columnCount(), 2);

	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("Title"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("ArtistId"));

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Text);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);

	// first row in the spreadsheet
	QCOMPARE(spreadsheet.column(0)->textAt(0), QLatin1String("For Those About To Rock We Salute You"));
	QCOMPARE(spreadsheet.column(1)->integerAt(0), 1);

	// last row in the spreadsheet
	QCOMPARE(spreadsheet.column(0)->textAt(346), QLatin1String("Koyaanisqatsi (Soundtrack from the Motion Picture)"));
	QCOMPARE(spreadsheet.column(1)->integerAt(346), 275);
}

/*!
 * import the second column only
 */
void ImportSqlDatabaseTest::testFullTableCustomColumnRange03() {
	// prepare the target spreadsheet
	Spreadsheet spreadsheet(QStringLiteral("test"), false);

	// import the records from 10 to 20 from the first table "artists"
	ImportSQLDatabaseWidget w;
	w.loadSettings();
	w.setCustomQuery(false);
	w.setStartColumn(2);
	w.setEndColumn(2);
	w.read(&spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// check the spreadsheet size and columns names and modes
	QCOMPARE(spreadsheet.rowCount(), 347);
	QCOMPARE(spreadsheet.columnCount(), 1);

	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("Title"));
	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Text);

	// first row in the spreadsheet
	QCOMPARE(spreadsheet.column(0)->textAt(0), QLatin1String("For Those About To Rock We Salute You"));

	// last row in the spreadsheet
	QCOMPARE(spreadsheet.column(0)->textAt(346), QLatin1String("Koyaanisqatsi (Soundtrack from the Motion Picture)"));
}

/*!
 * import the second column only, records from 10 to 20
 */
void ImportSqlDatabaseTest::testFullTableCustomRowColumnRange() {
	// prepare the target spreadsheet
	Spreadsheet spreadsheet(QStringLiteral("test"), false);

	// import the records from 10 to 20 from the first table "artists"
	ImportSQLDatabaseWidget w;
	w.loadSettings();
	w.setCustomQuery(false);
	w.setStartRow(10);
	w.setEndRow(20);
	w.setStartColumn(2);
	w.setEndColumn(2);
	w.read(&spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// check the spreadsheet size and columns names and modes
	QCOMPARE(spreadsheet.rowCount(), 11);
	QCOMPARE(spreadsheet.columnCount(), 1);

	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("Title"));
	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Text);

	// first row in the spreadsheet
	QCOMPARE(spreadsheet.column(0)->textAt(0), QLatin1String("Audioslave"));

	// last row in the spreadsheet
	QCOMPARE(spreadsheet.column(0)->textAt(10), QLatin1String("The Best Of Buddy Guy - The Millenium Collection"));
}

// ##############################################################################
// ##################  import the result of a custom query ######################
// ##############################################################################
void ImportSqlDatabaseTest::testQuery() {
	// prepare the target spreadsheet
	Spreadsheet spreadsheet(QStringLiteral("test"), false);

	// import the resultset of a custom query
	ImportSQLDatabaseWidget w;
	w.loadSettings();
	w.setCustomQuery(true);
	w.setQuery(QLatin1String("select title from albums where title like '%best%';"));
	w.refreshPreview();
	w.read(&spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// check the spreadsheet size and columns names and modes
	QCOMPARE(spreadsheet.rowCount(), 15);
	QCOMPARE(spreadsheet.columnCount(), 1);

	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("Title"));
	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Text);

	// first row in the spreadsheet
	QCOMPARE(spreadsheet.column(0)->textAt(0), QLatin1String("The Best Of Billy Cobham"));

	// last row in the spreadsheet
	QCOMPARE(spreadsheet.column(0)->textAt(14), QLatin1String("The Best of Beethoven"));
}

QTEST_MAIN(ImportSqlDatabaseTest)
