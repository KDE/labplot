/*
    File                 : JsonFilterTest.cpp
    Project              : LabPlot
    Description          : Tests for the JSON I/O-filter.
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2018 Andrey Cygankov <craftplace.ms@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "JsonFilterTest.h"
#include "backend/datasources/filters/JsonFilter.h"
#include "backend/spreadsheet/Spreadsheet.h"

#include <KLocalizedString>

void JsonFilterTest::initTestCase() {
	// needed in order to have the signals triggered by SignallingUndoCommand, see LabPlot.cpp
	//TODO: redesign/remove this
	qRegisterMetaType<const AbstractAspect*>("const AbstractAspect*");
	qRegisterMetaType<const AbstractColumn*>("const AbstractColumn*");
}

void JsonFilterTest::testArrayImport() {
	Spreadsheet spreadsheet("test", false);
	JsonFilter filter;

	const QString& fileName = QFINDTESTDATA(QLatin1String("data/array.json"));
	AbstractFileFilter::ImportMode mode = AbstractFileFilter::ImportMode::Replace;
	filter.setCreateIndexEnabled(true);
	filter.setDataRowType(QJsonValue::Array);
	filter.setDateTimeFormat(QLatin1String("yyyy-MM-dd"));
	filter.readDataFromFile(fileName, &spreadsheet, mode);

	QCOMPARE(spreadsheet.columnCount(), 3);
	QCOMPARE(spreadsheet.rowCount(), 3);
	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::DateTime);
	QCOMPARE(spreadsheet.column(2)->columnMode(), AbstractColumn::ColumnMode::Double);

	QCOMPARE(spreadsheet.column(0)->plotDesignation(), AbstractColumn::PlotDesignation::X);
	QCOMPARE(spreadsheet.column(1)->plotDesignation(), AbstractColumn::PlotDesignation::Y);
	QCOMPARE(spreadsheet.column(2)->plotDesignation(), AbstractColumn::PlotDesignation::Y);

	QCOMPARE(spreadsheet.column(0)->name(), i18n("index"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("Column 1")); //TODO is translatable in JsonFilter
	QCOMPARE(spreadsheet.column(2)->name(), QLatin1String("Column 2"));

	QCOMPARE(spreadsheet.column(0)->integerAt(0), 1);
	QCOMPARE(spreadsheet.column(0)->integerAt(1), 2);
	QCOMPARE(spreadsheet.column(0)->integerAt(2), 3);

	QDateTime value = QDateTime::fromString(QLatin1String("2018-06-01"), QLatin1String("yyyy-MM-dd"));
	QCOMPARE(spreadsheet.column(1)->dateTimeAt(0), value);
	value = QDateTime::fromString(QLatin1String("2018-06-02"), QLatin1String("yyyy-MM-dd"));
	QCOMPARE(spreadsheet.column(1)->dateTimeAt(1), value);
	value = QDateTime::fromString(QLatin1String("2018-06-03"), QLatin1String("yyyy-MM-dd"));
	QCOMPARE(spreadsheet.column(1)->dateTimeAt(2), value);

	QCOMPARE(spreadsheet.column(2)->valueAt(0), 0.01);
	QCOMPARE(spreadsheet.column(2)->valueAt(1), 0.02);
	QCOMPARE(spreadsheet.column(2)->valueAt(2), 0.03);
}

/*!
 * import objects with an additional column for the index
 */
void JsonFilterTest::testObjectImport01() {
	Spreadsheet spreadsheet("test", false);
	JsonFilter filter;

	const QString& fileName = QFINDTESTDATA(QLatin1String("data/object.json"));
	AbstractFileFilter::ImportMode mode = AbstractFileFilter::ImportMode::Replace;
	filter.setCreateIndexEnabled(true);
	filter.setDataRowType(QJsonValue::Object);
	filter.readDataFromFile(fileName, &spreadsheet, mode);

	QCOMPARE(spreadsheet.columnCount(), 5);
	QCOMPARE(spreadsheet.rowCount(), 3);
	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(2)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(3)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(4)->columnMode(), AbstractColumn::ColumnMode::Text);

	QCOMPARE(spreadsheet.column(0)->plotDesignation(), AbstractColumn::PlotDesignation::X);
	QCOMPARE(spreadsheet.column(1)->plotDesignation(), AbstractColumn::PlotDesignation::Y);
	QCOMPARE(spreadsheet.column(2)->plotDesignation(), AbstractColumn::PlotDesignation::Y);
	QCOMPARE(spreadsheet.column(3)->plotDesignation(), AbstractColumn::PlotDesignation::Y);
	QCOMPARE(spreadsheet.column(4)->plotDesignation(), AbstractColumn::PlotDesignation::Y);

	QCOMPARE(spreadsheet.column(0)->name(), i18n("index"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("1"));
	QCOMPARE(spreadsheet.column(2)->name(), QLatin1String("2"));
	QCOMPARE(spreadsheet.column(3)->name(), QLatin1String("3"));
	QCOMPARE(spreadsheet.column(4)->name(), QLatin1String("4"));

	QCOMPARE(spreadsheet.column(0)->integerAt(0), 1);
	QCOMPARE(spreadsheet.column(0)->integerAt(1), 2);
	QCOMPARE(spreadsheet.column(0)->integerAt(2), 3);

	QCOMPARE(spreadsheet.column(1)->valueAt(0), 1.234);
	QCOMPARE(spreadsheet.column(1)->valueAt(1), 111.);
	QCOMPARE(spreadsheet.column(1)->valueAt(2), 0.001);

	QCOMPARE(spreadsheet.column(2)->valueAt(0), 2.345);
	QCOMPARE(spreadsheet.column(2)->valueAt(1), 222.);
	QCOMPARE(spreadsheet.column(2)->valueAt(2), 0.002);

	QCOMPARE(spreadsheet.column(3)->valueAt(0), 3.456);
	QCOMPARE(spreadsheet.column(3)->valueAt(1), 333.);
	QCOMPARE(spreadsheet.column(3)->valueAt(2), 0.003);

	QCOMPARE(spreadsheet.column(4)->textAt(0), QString("field1"));
	QCOMPARE(spreadsheet.column(4)->textAt(1), QString("field2"));
	QCOMPARE(spreadsheet.column(4)->textAt(2), QString("field3"));
}

/*!
 * import objects with an additional column for the object names
 */
void JsonFilterTest::testObjectImport02() {
	Spreadsheet spreadsheet("test", false);
	JsonFilter filter;

	const QString& fileName = QFINDTESTDATA(QLatin1String("data/object.json"));
	AbstractFileFilter::ImportMode mode = AbstractFileFilter::ImportMode::Replace;
	filter.setDataRowType(QJsonValue::Object);
	filter.setImportObjectNames(true);
	filter.readDataFromFile(fileName, &spreadsheet, mode);

	QCOMPARE(spreadsheet.columnCount(), 5);
	QCOMPARE(spreadsheet.rowCount(), 3);
	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Text);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(2)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(3)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(4)->columnMode(), AbstractColumn::ColumnMode::Text);

	QCOMPARE(spreadsheet.column(0)->plotDesignation(), AbstractColumn::PlotDesignation::X);
	QCOMPARE(spreadsheet.column(1)->plotDesignation(), AbstractColumn::PlotDesignation::Y);
	QCOMPARE(spreadsheet.column(2)->plotDesignation(), AbstractColumn::PlotDesignation::Y);
	QCOMPARE(spreadsheet.column(3)->plotDesignation(), AbstractColumn::PlotDesignation::Y);
	QCOMPARE(spreadsheet.column(4)->plotDesignation(), AbstractColumn::PlotDesignation::Y);

	QCOMPARE(spreadsheet.column(0)->name(), i18n("name"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("1"));
	QCOMPARE(spreadsheet.column(2)->name(), QLatin1String("2"));
	QCOMPARE(spreadsheet.column(3)->name(), QLatin1String("3"));
	QCOMPARE(spreadsheet.column(4)->name(), QLatin1String("4"));

	QCOMPARE(spreadsheet.column(0)->textAt(0), QString("field1"));
	QCOMPARE(spreadsheet.column(0)->textAt(1), QString("field2"));
	QCOMPARE(spreadsheet.column(0)->textAt(2), QString("field3"));

	QCOMPARE(spreadsheet.column(1)->valueAt(0), 1.234);
	QCOMPARE(spreadsheet.column(1)->valueAt(1), 111.);
	QCOMPARE(spreadsheet.column(1)->valueAt(2), 0.001);

	QCOMPARE(spreadsheet.column(2)->valueAt(0), 2.345);
	QCOMPARE(spreadsheet.column(2)->valueAt(1), 222.);
	QCOMPARE(spreadsheet.column(2)->valueAt(2), 0.002);

	QCOMPARE(spreadsheet.column(3)->valueAt(0), 3.456);
	QCOMPARE(spreadsheet.column(3)->valueAt(1), 333.);
	QCOMPARE(spreadsheet.column(3)->valueAt(2), 0.003);

	QCOMPARE(spreadsheet.column(4)->textAt(0), QString("field1"));
	QCOMPARE(spreadsheet.column(4)->textAt(1), QString("field2"));
	QCOMPARE(spreadsheet.column(4)->textAt(2), QString("field3"));
}

/*!
 * import objects with an additional column for the object names
 * with custom start and end columns
 */
void JsonFilterTest::testObjectImport03() {
	Spreadsheet spreadsheet("test", false);
	JsonFilter filter;

	const QString& fileName = QFINDTESTDATA(QLatin1String("data/object.json"));
	AbstractFileFilter::ImportMode mode = AbstractFileFilter::ImportMode::Replace;
	filter.setDataRowType(QJsonValue::Object);
	filter.setImportObjectNames(true);
	filter.setStartColumn(2);
	filter.setEndColumn(3);
	filter.readDataFromFile(fileName, &spreadsheet, mode);

	QCOMPARE(spreadsheet.columnCount(), 3);
	QCOMPARE(spreadsheet.rowCount(), 3);
	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Text);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(2)->columnMode(), AbstractColumn::ColumnMode::Double);

	QCOMPARE(spreadsheet.column(0)->plotDesignation(), AbstractColumn::PlotDesignation::X);
	QCOMPARE(spreadsheet.column(1)->plotDesignation(), AbstractColumn::PlotDesignation::Y);
	QCOMPARE(spreadsheet.column(2)->plotDesignation(), AbstractColumn::PlotDesignation::Y);

	QCOMPARE(spreadsheet.column(0)->name(), i18n("name"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("2"));
	QCOMPARE(spreadsheet.column(2)->name(), QLatin1String("3"));

	QCOMPARE(spreadsheet.column(0)->textAt(0), QString("field1"));
	QCOMPARE(spreadsheet.column(0)->textAt(1), QString("field2"));
	QCOMPARE(spreadsheet.column(0)->textAt(2), QString("field3"));

	QCOMPARE(spreadsheet.column(1)->valueAt(0), 2.345);
	QCOMPARE(spreadsheet.column(1)->valueAt(1), 222.);
	QCOMPARE(spreadsheet.column(1)->valueAt(2), 0.002);

	QCOMPARE(spreadsheet.column(2)->valueAt(0), 3.456);
	QCOMPARE(spreadsheet.column(2)->valueAt(1), 333.);
	QCOMPARE(spreadsheet.column(2)->valueAt(2), 0.003);
}

/*!
 * import objects with an additional datetime column for the object names
 */
void JsonFilterTest::testObjectImport04() {
	Spreadsheet spreadsheet("test", false);
	JsonFilter filter;

	const QString& fileName = QFINDTESTDATA(QLatin1String("data/intraday.json"));
	AbstractFileFilter::ImportMode mode = AbstractFileFilter::ImportMode::Replace;
	QVector<int> rows{0, 1};
	filter.setModelRows(rows);
	filter.setDataRowType(QJsonValue::Object);
	filter.setImportObjectNames(true);

	QString dateTimeFormat("yyyy-MM-dd hh:mm:ss");
	filter.setDateTimeFormat(dateTimeFormat);
	filter.setStartRow(1);
	filter.setEndRow(2);
	filter.readDataFromFile(fileName, &spreadsheet, mode);

	QCOMPARE(spreadsheet.columnCount(), 6);
	QCOMPARE(spreadsheet.rowCount(), 2);
	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::DateTime);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(2)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(3)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(4)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(5)->columnMode(), AbstractColumn::ColumnMode::Integer);

	QCOMPARE(spreadsheet.column(0)->plotDesignation(), AbstractColumn::PlotDesignation::X);
	QCOMPARE(spreadsheet.column(1)->plotDesignation(), AbstractColumn::PlotDesignation::Y);
	QCOMPARE(spreadsheet.column(2)->plotDesignation(), AbstractColumn::PlotDesignation::Y);
	QCOMPARE(spreadsheet.column(3)->plotDesignation(), AbstractColumn::PlotDesignation::Y);
	QCOMPARE(spreadsheet.column(4)->plotDesignation(), AbstractColumn::PlotDesignation::Y);
	QCOMPARE(spreadsheet.column(5)->plotDesignation(), AbstractColumn::PlotDesignation::Y);

	QCOMPARE(spreadsheet.column(0)->name(), i18n("timestamp"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("1. open"));
	QCOMPARE(spreadsheet.column(2)->name(), QLatin1String("2. high"));
	QCOMPARE(spreadsheet.column(3)->name(), QLatin1String("3. low"));
	QCOMPARE(spreadsheet.column(4)->name(), QLatin1String("4. close"));
	QCOMPARE(spreadsheet.column(5)->name(), QLatin1String("5. volume"));

	//TODO: the values are sorted with respect to the names of the objects, i.e. to the timestamp. Why?
	QCOMPARE(spreadsheet.column(0)->dateTimeAt(0).toString(dateTimeFormat), QString("2018-06-14 15:56:00"));
	QCOMPARE(spreadsheet.column(0)->dateTimeAt(1).toString(dateTimeFormat), QString("2018-06-14 15:57:00"));

	QCOMPARE(spreadsheet.column(1)->valueAt(0), 101.2700);
	QCOMPARE(spreadsheet.column(1)->valueAt(1), 101.2700);

	QCOMPARE(spreadsheet.column(2)->valueAt(0), 101.2800);
	QCOMPARE(spreadsheet.column(2)->valueAt(1), 101.3350);

	QCOMPARE(spreadsheet.column(3)->valueAt(0), 101.2600);
	QCOMPARE(spreadsheet.column(3)->valueAt(1), 101.2550);

	QCOMPARE(spreadsheet.column(4)->valueAt(0), 101.2600);
	QCOMPARE(spreadsheet.column(4)->valueAt(1), 101.3300);

	QCOMPARE(spreadsheet.column(5)->integerAt(0), 27960);
	QCOMPARE(spreadsheet.column(5)->integerAt(1), 127830);
}

QTEST_MAIN(JsonFilterTest)
