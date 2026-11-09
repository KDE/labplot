/*
	File                 : ScriptTest.h
	Project              : LabPlot
	Description          : Tests for Script (Python debugging)
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2026 Martin Marmsoler <martin.marmsoler@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SCRIPTTEST_H
#define SCRIPTTEST_H

#include "../../CommonMetaTest.h"

class ScriptTest : public CommonMetaTest {
	Q_OBJECT

private Q_SLOTS:
	void debugContinueBreakpoints();
	void debugStepOver();
	void debugStepIntoAndOut();
	void debugStop();
	void debugVariables();
	void debugAddBreakpointWhilePaused();
	void debugErrorLine();
	void toggleBreakpoint();
	void toggleBreakpointToolbar();
	void floatingToolBar();
	void textLabelMultilineText();
	void breakpointsNotSaved();
	void variableViewerChildren();
	void variableViewerWhilePaused();
	void debugShortcutsWithConflictingWindowAction();
};

#endif
