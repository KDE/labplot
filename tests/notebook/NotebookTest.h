/*
    File                 : NotebookTest.h
    Project              : LabPlot
    Description          : Tests for the Notebook
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2021 Alexander Semke <alexander.semke@web.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef NOTEBOOKTEST_H
#define NOTEBOOKTEST_H

#include <QtTest>

class NotebookTest : public QObject {
	Q_OBJECT

private Q_SLOTS:
	void initTestCase();

	//Maxima
	void testParserMaxima01();
	void testParserMaxima02();

    //Python
	void testParserPython01();
	void testParserPython02();
	void testParserPython03();
};

#endif
