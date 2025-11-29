/*
	File                 : ImportSqlDatabaseTest.h
	Project              : LabPlot
	Description          : Tests for the import from SQL databases
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023-2025 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ImportSqlDatabaseTest.h"
#include "backend/core/column/Column.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "frontend/datasources/ImportSQLDatabaseWidget.h"

#include <KConfig>
#include <KConfigGroup>

#include <QSqlQuery>
#include <QSqlRecord>

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
 * read the full table import in the prepend mode
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

// ##############################################################################
// #################################  export ####################################
// ##############################################################################

void ImportSqlDatabaseTest::testExportSpreadsheetToSqlite() {
	// prepare the source spreadsheet
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	spreadsheet.setColumnCount(3);
	spreadsheet.setRowCount(3);

	auto* col1 = spreadsheet.column(0);
	col1->setColumnMode(AbstractColumn::ColumnMode::Text);
	col1->setName(QLatin1String("text_column"));
	col1->setTextAt(0, QLatin1String("A'a"));
	col1->setTextAt(1, QLatin1String("B'b"));
	col1->setTextAt(2, QLatin1String("C'c"));

	auto* col2 = spreadsheet.column(1);
	col2->setColumnMode(AbstractColumn::ColumnMode::Integer);
	col2->setName(QLatin1String("integer_column"));
	col2->setIntegerAt(0, 1);
	col2->setIntegerAt(1, 2);
	col2->setIntegerAt(2, 3);

	auto* col3 = spreadsheet.column(2);
	col3->setColumnMode(AbstractColumn::ColumnMode::Double);
	col3->setName(QLatin1String("double_column"));
	col3->setValueAt(0, 1.1);
	col3->setValueAt(1, 2.2);
	col3->setValueAt(2, 3.3);

	// export to a temp file
	QTemporaryFile file;
	QVERIFY(file.open());
	spreadsheet.exportToSQLite(file.fileName());

	// check the content of the exported db
	const QStringList& drivers = QSqlDatabase::drivers();
	QString driver;
	if (drivers.contains(QLatin1String("QSQLITE3")))
		driver = QLatin1String("QSQLITE3");
	else
		driver = QLatin1String("QSQLITE");

	QSqlDatabase db = QSqlDatabase::addDatabase(driver);
	db.setDatabaseName(file.fileName());
	QVERIFY(db.open());

	QSqlQuery q(QStringLiteral("SELECT * FROM test;"));
	QVERIFY(q.exec());

	QVERIFY(q.next());
	auto record = q.record();

	// check the column names
	QCOMPARE(record.count(), 3);
	QCOMPARE(record.fieldName(0), QLatin1String("text_column"));
	QCOMPARE(record.fieldName(1), QLatin1String("integer_column"));
	QCOMPARE(record.fieldName(2), QLatin1String("double_column"));

	// check the record values
	QCOMPARE(q.value(0).toString(), QLatin1String("A'a"));
	QCOMPARE(q.value(1).toInt(), 1);
	QCOMPARE(q.value(2).toDouble(), 1.1);

	QVERIFY(q.next());
	QCOMPARE(q.value(0).toString(), QLatin1String("B'b"));
	QCOMPARE(q.value(1).toInt(), 2);
	QCOMPARE(q.value(2).toDouble(), 2.2);

	QVERIFY(q.next());
	QCOMPARE(q.value(0).toString(), QLatin1String("C'c"));
	QCOMPARE(q.value(1).toInt(), 3);
	QCOMPARE(q.value(2).toDouble(), 3.3);

	QVERIFY(!q.next()); // last record reached
}

QTEST_MAIN(ImportSqlDatabaseTest)
