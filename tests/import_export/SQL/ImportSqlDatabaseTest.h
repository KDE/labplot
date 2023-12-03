/*
	File                 : ImportSqlDatabaseTest.h
	Project              : LabPlot
	Description          : Tests for the import from SQL databases
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef IMPORTSQLDATABASETEST_H
#define IMPORTSQLDATABASETEST_H

#include "../../CommonTest.h"
#include <QtTest>

class ImportSqlDatabaseTest : public CommonTest {
	Q_OBJECT

private Q_SLOTS:
	void initTestCase();

	// import full table
	void testFullTable();
	void testFullTableCustomRowRange();
	void testFullTableCustomColumnRange();
	void testFullTableCustomRowColumnRange();

	// import the result of a custom query
	void testQuery();
};
#endif
