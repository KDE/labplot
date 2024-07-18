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

class ImportSqlDatabaseTest : public CommonTest {
	Q_OBJECT

private Q_SLOTS:
	void initTestCase();

	// import full table
	void testFullTableReplace();
	void testFullTableAppend();
	void testFullTablePrepend();
	void testFullTableCustomRowRange();
	void testFullTableCustomColumnRange01();
	void testFullTableCustomColumnRange02();
	void testFullTableCustomColumnRange03();
	void testFullTableCustomRowColumnRange();

	// import the result of a custom query
	void testQuery();
};
#endif
