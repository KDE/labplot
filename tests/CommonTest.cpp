/*
	File                 : CommonTest.cpp
	Project              : LabPlot
	Description          : General test class
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2019 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "CommonTest.h"
#ifdef _WIN32
#include <windows.h>
#endif

void CommonTest::initTestCase() {
#ifdef _WIN32
//	if (AttachConsole(ATTACH_PARENT_PROCESS)) {
//		freopen("CONOUT$", "w", stdout);
//		freopen("CONOUT$", "w", stderr);
//	}
#endif
}
