/*
	File                 : CommonMetaTest.h
	Project              : LabPlot
	Description          : General test class including MetaTypes
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef COMMONMETATEST_H
#define COMMONMETATEST_H

#include "CommonTest.h"

///////////////////////////////////////////////////////

class CommonMetaTest : public CommonTest {
	Q_OBJECT

protected Q_SLOTS:
	void initTestCase();
};
#endif
