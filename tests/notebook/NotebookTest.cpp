/*
	File                 : NotebookTest.cpp
	Project              : LabPlot
	Description          : Tests for the Notebook
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2021 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "NotebookTest.h"
#include "backend/cantorWorksheet/VariableParser.h"
#include "backend/core/AbstractColumn.h"

extern "C" {
#include <gsl/gsl_math.h>
}

void NotebookTest::initTestCase() {
	qRegisterMetaType<const AbstractAspect*>("const AbstractAspect*");
	qRegisterMetaType<const AbstractColumn*>("const AbstractColumn*");
	QLocale::setDefault(QLocale(QLocale::C));
}

//**********************************************************
//************************* Maxima *************************
//**********************************************************

/*!
	read an array of doubles
*/
void NotebookTest::testParserMaxima01() {
	QString input = QStringLiteral("[1.0, 2.0]");
	VariableParser parser(QStringLiteral("maxima"), input);

	QCOMPARE(parser.isParsed(), true);
	QCOMPARE(parser.dataType(), AbstractColumn::ColumnMode::Double);

	const auto values = parser.doublePrecision();
	QCOMPARE(values.size(), 2);
	QCOMPARE(values.at(0), 1.0);
	QCOMPARE(values.at(1), 2.0);
}

/*!
	read an array of strings
*/
void NotebookTest::testParserMaxima02() {
	QString input = QStringLiteral("[\"a\", \"b\"]");
	VariableParser parser(QStringLiteral("Maxima"), input);

	QCOMPARE(parser.isParsed(), true);
	QCOMPARE(parser.dataType(), AbstractColumn::ColumnMode::Text);

	const auto values = parser.text();
	QCOMPARE(values.size(), 2);
	// 	QCOMPARE(values.at(0), QStringLiteral("a"));
	// 	QCOMPARE(values.at(1), QStringLiteral("b"));
}

//**********************************************************
//************************* Python *************************
//**********************************************************
/*!
	read a list of doubles
*/
void NotebookTest::testParserPython01() {
	QString input = QStringLiteral("[1.0, 2.0]");
	VariableParser parser(QStringLiteral("python"), input);

	QCOMPARE(parser.isParsed(), true);
	QCOMPARE(parser.dataType(), AbstractColumn::ColumnMode::Double);

	const auto values = parser.doublePrecision();
	QCOMPARE(values.size(), 2);
	QCOMPARE(values.at(0), 1.0);
	QCOMPARE(values.at(1), 2.0);
}

/*!
	read a tuple of doubles
*/
void NotebookTest::testParserPython02() {
	QString input = QStringLiteral("(1.0, 2.0)");
	VariableParser parser(QStringLiteral("python"), input);

	QCOMPARE(parser.isParsed(), true);
	QCOMPARE(parser.dataType(), AbstractColumn::ColumnMode::Double);

	const auto values = parser.doublePrecision();
	QCOMPARE(values.size(), 2);
	QCOMPARE(values.at(0), 1.0);
	QCOMPARE(values.at(1), 2.0);
}

/*!
	read a set of doubles
*/
void NotebookTest::testParserPython03() {
	QString input = QStringLiteral("{1.0, 2.0}");
	VariableParser parser(QStringLiteral("python"), input);

	QCOMPARE(parser.isParsed(), true);
	QCOMPARE(parser.dataType(), AbstractColumn::ColumnMode::Double);

	const auto values = parser.doublePrecision();
	QCOMPARE(values.size(), 2);
	QCOMPARE(values.at(0), 1.0);
	QCOMPARE(values.at(1), 2.0);
}

// numpy data types

// np.ones(2, dtype=np.int16)
void NotebookTest::testParserPython04() {
	QString input = QStringLiteral("array([1, 1], dtype=int16)");
	VariableParser parser(QStringLiteral("python"), input);

	QCOMPARE(parser.isParsed(), true);
	QCOMPARE(parser.dataType(), AbstractColumn::ColumnMode::Integer);

	const auto values = parser.integers();
	QCOMPARE(values.size(), 2);
	QCOMPARE(values.at(0), 1);
	QCOMPARE(values.at(1), 1);
}

// np.ones((2,2), dtype=np.int16)
void NotebookTest::testParserPython05() {
	QString input = QStringLiteral("array([[1, 1], [1, 1]], dtype=int16)");
	VariableParser parser(QStringLiteral("python"), input);

	QCOMPARE(parser.isParsed(), false);
}

void NotebookTest::testParserPython06() {
	// Testing datetime ms
	QString input = QStringLiteral("array(['2016-03-26T02:14:34.000', '2017-03-26T02:14:34.000', '2018-03-26T02:14:34.000'], dtype=datetime64[ms])");
	VariableParser parser(QStringLiteral("python"), input);

	QTEST_ASSERT(parser.dataType() == AbstractColumn::ColumnMode::DateTime);
	QCOMPARE(parser.isParsed(), true);
	auto res = parser.dateTime();

	QCOMPARE(res.length(), 3);
	QCOMPARE(res.at(0).isValid(), true);
	QCOMPARE(res.at(1).isValid(), true);
	QCOMPARE(res.at(2).isValid(), true);
	QTEST_ASSERT(res.at(0).toString(QStringLiteral("yyyy-MM-ddThh:mm:ss.zzz")) == QStringLiteral("2016-03-26T02:14:34.000"));
	QTEST_ASSERT(res.at(1).toString(QStringLiteral("yyyy-MM-ddThh:mm:ss.zzz")) == QStringLiteral("2017-03-26T02:14:34.000"));
	QTEST_ASSERT(res.at(2).toString(QStringLiteral("yyyy-MM-ddThh:mm:ss.zzz")) == QStringLiteral("2018-03-26T02:14:34.000"));
}

void NotebookTest::testParserPython07() {
	// Testing datetime s
	QString input = QStringLiteral("array(['2016-03-26T02:14:34', '2017-03-26T02:14:34', '2018-03-26T02:14:34'], dtype=datetime64[s])");
	VariableParser parser(QStringLiteral("python"), input);

	QTEST_ASSERT(parser.dataType() == AbstractColumn::ColumnMode::DateTime);
	QCOMPARE(parser.isParsed(), true);
	auto res = parser.dateTime();

	QCOMPARE(res.length(), 3);
	QCOMPARE(res.at(0).isValid(), true);
	QCOMPARE(res.at(1).isValid(), true);
	QCOMPARE(res.at(2).isValid(), true);
	QTEST_ASSERT(res.at(0).toString(QStringLiteral("yyyy-MM-ddThh:mm:ss.zzz")) == QStringLiteral("2016-03-26T02:14:34.000"));
	QTEST_ASSERT(res.at(1).toString(QStringLiteral("yyyy-MM-ddThh:mm:ss.zzz")) == QStringLiteral("2017-03-26T02:14:34.000"));
	QTEST_ASSERT(res.at(2).toString(QStringLiteral("yyyy-MM-ddThh:mm:ss.zzz")) == QStringLiteral("2018-03-26T02:14:34.000"));
}

void NotebookTest::testParserPython08() {
	// Testing minute datetime
	QString input = QStringLiteral("array(['2016-03-26T02:14', '2017-03-26T02:14', '2018-03-26T02:14'], dtype=datetime64[m])");
	VariableParser parser(QStringLiteral("python"), input);

	QTEST_ASSERT(parser.dataType() == AbstractColumn::ColumnMode::DateTime);
	QCOMPARE(parser.isParsed(), true);
	auto res = parser.dateTime();

	QCOMPARE(res.length(), 3);
	QCOMPARE(res.at(0).isValid(), true);
	QCOMPARE(res.at(1).isValid(), true);
	QCOMPARE(res.at(2).isValid(), true);
	QVERIFY2(res.at(0).toString(QStringLiteral("yyyy-MM-ddThh:mm:ss.zzz")) == QStringLiteral("2016-03-26T02:14:00.000"),
			 qPrintable(res.at(0).toString(QStringLiteral("yyyy-MM-ddThh:mm:ss.zzz"))));
	QVERIFY2(res.at(1).toString(QStringLiteral("yyyy-MM-ddThh:mm:ss.zzz")) == QStringLiteral("2017-03-26T02:14:00.000"),
			 qPrintable(res.at(1).toString(QStringLiteral("yyyy-MM-ddThh:mm:ss.zzz"))));
	QVERIFY2(res.at(2).toString(QStringLiteral("yyyy-MM-ddThh:mm:ss.zzz")) == QStringLiteral("2018-03-26T02:14:00.000"),
			 qPrintable(res.at(2).toString(QStringLiteral("yyyy-MM-ddThh:mm:ss.zzz"))));
}

void NotebookTest::testParserPython09() {
	// Testing hour datetime
	QString input = QStringLiteral("array(['2016-03-26T02', '2017-03-26T02', '2018-03-26T02'], dtype=datetime64[h])");
	VariableParser parser(QStringLiteral("python"), input);

	QTEST_ASSERT(parser.dataType() == AbstractColumn::ColumnMode::DateTime);
	QCOMPARE(parser.isParsed(), true);
	auto res = parser.dateTime();

	QCOMPARE(res.length(), 3);
	QCOMPARE(res.at(0).isValid(), true);
	QCOMPARE(res.at(1).isValid(), true);
	QCOMPARE(res.at(2).isValid(), true);
	QVERIFY2(res.at(0).toString(QStringLiteral("yyyy-MM-ddThh:mm:ss.zzz")) == QStringLiteral("2016-03-26T02:00:00.000"),
			 qPrintable(res.at(0).toString(QStringLiteral("yyyy-MM-ddThh:mm:ss.zzz"))));
	QVERIFY2(res.at(1).toString(QStringLiteral("yyyy-MM-ddThh:mm:ss.zzz")) == QStringLiteral("2017-03-26T02:00:00.000"),
			 qPrintable(res.at(1).toString(QStringLiteral("yyyy-MM-ddThh:mm:ss.zzz"))));
	QVERIFY2(res.at(2).toString(QStringLiteral("yyyy-MM-ddThh:mm:ss.zzz")) == QStringLiteral("2018-03-26T02:00:00.000"),
			 qPrintable(res.at(2).toString(QStringLiteral("yyyy-MM-ddThh:mm:ss.zzz"))));
}
void NotebookTest::testParserPython10() {
	// Testing datetime day
	QString input = QStringLiteral("array(['2016-03-26', '2017-03-26', '2018-03-26'], dtype=datetime64[D])");
	VariableParser parser(QStringLiteral("python"), input);

	QTEST_ASSERT(parser.dataType() == AbstractColumn::ColumnMode::DateTime);
	QCOMPARE(parser.isParsed(), true);
	auto res = parser.dateTime();

	QCOMPARE(res.length(), 3);
	QCOMPARE(res.at(0).isValid(), true);
	QCOMPARE(res.at(1).isValid(), true);
	QCOMPARE(res.at(2).isValid(), true);
	QTEST_ASSERT(res.at(0).toString(QStringLiteral("yyyy-MM-ddThh:mm:ss.zzz")) == QStringLiteral("2016-03-26T00:00:00.000"));
	QTEST_ASSERT(res.at(1).toString(QStringLiteral("yyyy-MM-ddThh:mm:ss.zzz")) == QStringLiteral("2017-03-26T00:00:00.000"));
	QTEST_ASSERT(res.at(2).toString(QStringLiteral("yyyy-MM-ddThh:mm:ss.zzz")) == QStringLiteral("2018-03-26T00:00:00.000"));
}

QTEST_MAIN(NotebookTest)
