/*
	File                 : JSONFilterTest.h
	Project              : LabPlot
	Description          : Tests for the JSON I/O-filter.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2018 Andrey Cygankov <craftplace.ms@gmail.com>
	SPDX-FileCopyrightText: 2024 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef JSONFILTERTEST_H
#define JSONFILTERTEST_H

#include "../../CommonMetaTest.h"

class JSONFilterTest : public CommonMetaTest {
	Q_OBJECT

private Q_SLOTS:
	void testArrayImport();
	void testObjectImport01();
	void testObjectImport02();
	void testObjectImport03();
	void testObjectImport04();
};

#endif
