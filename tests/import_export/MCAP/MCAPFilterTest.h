/*
	File                 : MCAPFilterTest.h
	Project              : LabPlot
	Description          : Tests for the JSON I/O-filter.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2018 Andrey Cygankov <craftplace.ms@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MCAPFILTERTEST_H
#define MCAPFILTERTEST_H

#include <QtTest>

class MCAPFilterTest : public QObject {
	Q_OBJECT

private Q_SLOTS:
	void initTestCase();

	void testArrayImport();
	void testExport();
	void testImportWithoutValidTopics();
	void testImportWrongFile();

};

#endif
