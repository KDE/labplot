/*
	File                 : NotebookTest.h
	Project              : LabPlot
	Description          : Tests for the Notebook
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2021-2022 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef NOTEBOOKTEST_H
#define NOTEBOOKTEST_H

#include "../CommonTest.h"

class NotebookTest : public CommonTest {
	Q_OBJECT

private Q_SLOTS:
	void initTestCase() override;

	// Maxima
	void testParserMaxima01();
	void testParserMaxima02();

	// Python
	void testParserPython01();
	void testParserPython02();
	void testParserPython03();
	void testParserPython04();
	void testParserPython05();
	void testParserPython06();
	void testParserPython07();
	void testParserPython08();
	void testParserPython09();
	void testParserPython10();
	void testParserDateTime64ns();

	// Octave
	void testParserOctaveColumnVector();
	void testParserOctaveRowVector();
};

#endif
