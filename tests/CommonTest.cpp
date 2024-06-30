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

#include <QUndoStack>

void CommonTest::initTestCase() {
	KLocalizedString::setApplicationDomain("labplot2");

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
