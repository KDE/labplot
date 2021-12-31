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
	QString input = "[1.0, 2.0]";
	VariableParser parser("maxima", input);

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
	QString input = "[\"a\", \"b\"]";
	VariableParser parser("Maxima", input);

	QCOMPARE(parser.isParsed(), true);
	QCOMPARE(parser.dataType(), AbstractColumn::ColumnMode::Text);

	const auto values = parser.text();
	QCOMPARE(values.size(), 2);
// 	QCOMPARE(values.at(0), "a");
// 	QCOMPARE(values.at(1), "b");
}

//**********************************************************
//************************* Python *************************
//**********************************************************
/*!
	read a list of doubles
*/
void NotebookTest::testParserPython01() {
	QString input = "[1.0, 2.0]";
	VariableParser parser("python", input);

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
	QString input = "(1.0, 2.0)";
	VariableParser parser("python", input);

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
	QString input = "{1.0, 2.0}";
	VariableParser parser("python", input);

	QCOMPARE(parser.isParsed(), true);
	QCOMPARE(parser.dataType(), AbstractColumn::ColumnMode::Double);

	const auto values = parser.doublePrecision();
	QCOMPARE(values.size(), 2);
	QCOMPARE(values.at(0), 1.0);
	QCOMPARE(values.at(1), 2.0);
}

//numpy data types

//np.ones(2, dtype=np.int16)
void NotebookTest::testParserPython04() {
	QString input = "array([1, 1], dtype=int16)";
	VariableParser parser("python", input);

	QCOMPARE(parser.isParsed(), true);
	QCOMPARE(parser.dataType(), AbstractColumn::ColumnMode::Integer);

	const auto values = parser.integers();
	QCOMPARE(values.size(), 2);
	QCOMPARE(values.at(0), 1);
	QCOMPARE(values.at(1), 1);
}

//np.ones((2,2), dtype=np.int16)
void NotebookTest::testParserPython05() {
	QString input = "array([[1, 1], [1, 1]], dtype=int16)";
	VariableParser parser("python", input);

	QCOMPARE(parser.isParsed(), false);
}

QTEST_MAIN(NotebookTest)
