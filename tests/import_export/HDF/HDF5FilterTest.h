/*
File                 : HDF5FilterTest.h
Project              : LabPlot
Description          : Tests for the HDF5 I/O-filter.
--------------------------------------------------------------------
--------------------------------------------------------------------
SPDX-FileCopyrightText: 2021 Stefan Gerlach (stefan.gerlach@uni.kn)

*/

/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#ifndef HDF5FILTERTEST_H
#define HDF5FILTERTEST_H

#include "../../CommonTest.h"
#include <QtTest>

class HDF5FilterTest : public CommonTest {
	Q_OBJECT

private slots:
	void testImportDouble();
	void testImportDoublePortion();
	void testImportInt();
	void testImportIntPortion();
};


#endif
