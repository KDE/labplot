/*
	File                 : CommonMetaTest.cpp
	Project              : LabPlot
	Description          : General test class with MetaTypes
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "CommonMetaTest.h"
#include "backend/core/AbstractColumn.h"

#ifdef _WIN32
#include <windows.h>
#endif

void CommonMetaTest::initTestCase() {
	CommonTest::initTestCase();

	qRegisterMetaType<const AbstractAspect*>("const AbstractAspect*");
	qRegisterMetaType<const AbstractColumn*>("const AbstractColumn*");
}
