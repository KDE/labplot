/*
    File                 : ProjectImportTest.h
    Project              : LabPlot
    Description          : Tests for project imports
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2018 Alexander Semke <alexander.semke@web.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef PROJECTIMPORTTEST_H
#define PROJECTIMPORTTEST_H

#include <QtTest>

class ProjectImportTest : public QObject {
	Q_OBJECT

private Q_SLOTS:
	void initTestCase();

	//import of LabPlot projects

#ifdef HAVE_LIBORIGIN
	//import of Origin projects
	void testOrigin01();
	void testOrigin02();
	void testOrigin03();
	void testOrigin04();
	void testOriginTextNumericColumns();
	void testOrigin_2folder_with_graphs();

	// test tags
	void testParseOriginTags_data();
	void testParseOriginTags();
#endif
};
#endif
