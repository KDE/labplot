/*
	File                 : CommonTest.cpp
	Project              : LabPlot
	Description          : General test class
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2019-2024 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "CommonTest.h"
#include "src/backend/core/AbstractColumn.h"

#include <QUndoStack>

#ifdef _WIN32
#include <windows.h>
#endif

void CommonTest::initTestCase() {
	// always enable debugging
	enableDebugTrace(true);
	KLocalizedString::setApplicationDomain("labplot");

#ifdef _WIN32
//	if (AttachConsole(ATTACH_PARENT_PROCESS)) {
//		freopen("CONOUT$", "w", stdout);
//		freopen("CONOUT$", "w", stderr);
//	}
#endif
}

void CommonTest::listStack(QUndoStack* stack) {
	qDebug() << "--------------------------";
	qDebug() << "Begin list Undostack History";
	if (stack) {
		for (int i = 0; i < stack->count(); i++) {
			qDebug() << stack->text(i);
		}
	}
	qDebug() << "End list Undostack History";
	qDebug() << "--------------------------";
}
