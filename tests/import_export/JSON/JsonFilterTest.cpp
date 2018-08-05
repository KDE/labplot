#include "JsonFilterTest.h"
#include "backend/datasources/filters/JsonFilter.h"
#include "backend/spreadsheet/Spreadsheet.h"

void JsonFilterTest::initTestCase() {
	const QString currentDir = __FILE__;
	m_dataDir = currentDir.left(currentDir.lastIndexOf(QDir::separator())) + QDir::separator() + QLatin1String("data") + QDir::separator();

	// needed in order to have the signals triggered by SignallingUndoCommand, see LabPlot.cpp
	//TODO: redesign/remove this
	qRegisterMetaType<const AbstractAspect*>("const AbstractAspect*");
	qRegisterMetaType<const AbstractColumn*>("const AbstractColumn*");
}

void JsonFilterTest::testArrayImport() {
	Spreadsheet spreadsheet("test", false);
	JsonFilter filter;

	const QString fileName = m_dataDir + "array.json";
	AbstractFileFilter::ImportMode mode = AbstractFileFilter::Replace;
	filter.setModelRows(QVector<int>());
	filter.setCreateIndexEnabled(true);
	filter.setDataRowType(QJsonValue::Array);
	filter.readDataFromFile(fileName, &spreadsheet, mode);

	QCOMPARE(spreadsheet.columnCount(), 3);
	QCOMPARE(spreadsheet.rowCount(), 3);
	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::Integer);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::Text);
	QCOMPARE(spreadsheet.column(2)->columnMode(), AbstractColumn::Numeric);

	QCOMPARE(spreadsheet.column(0)->integerAt(0), 1);
	QCOMPARE(spreadsheet.column(0)->integerAt(1), 2);
	QCOMPARE(spreadsheet.column(0)->integerAt(2), 3);

	QCOMPARE(spreadsheet.column(1)->textAt(0), QString("2018-06-01"));
	QCOMPARE(spreadsheet.column(1)->textAt(1), QString("2018-06-02"));
	QCOMPARE(spreadsheet.column(1)->textAt(2), QString("2018-06-03"));

	QCOMPARE(spreadsheet.column(2)->valueAt(0), 0.01);
	QCOMPARE(spreadsheet.column(2)->valueAt(1), 0.02);
	QCOMPARE(spreadsheet.column(2)->valueAt(2), 0.03);

}

void JsonFilterTest::testObjectImport() {
	Spreadsheet spreadsheet("test", false);
	JsonFilter filter;

	const QString fileName = m_dataDir + "object.json";
	AbstractFileFilter::ImportMode mode = AbstractFileFilter::Replace;
	filter.setModelRows(QVector<int>());
	filter.setCreateIndexEnabled(true);
	filter.setDataRowType(QJsonValue::Object);
	filter.readDataFromFile(fileName, &spreadsheet, mode);

	QCOMPARE(spreadsheet.columnCount(), 5);
	QCOMPARE(spreadsheet.rowCount(), 3);
	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::Integer);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::Numeric);
	QCOMPARE(spreadsheet.column(2)->columnMode(), AbstractColumn::Numeric);
	QCOMPARE(spreadsheet.column(3)->columnMode(), AbstractColumn::Numeric);
	QCOMPARE(spreadsheet.column(4)->columnMode(), AbstractColumn::Text);

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

QTEST_MAIN(JsonFilterTest)
