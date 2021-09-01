/*
File                 : PresenterWidget_mac.mm
Project              : LabPlot
Description          : Reimplementation of QWidget::closeEvent() to workaround QTBUG-46701
--------------------------------------------------------------------
SPDX-FileCopyrightText: 2019 Alexander Semke <alexander.semke@web.de>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "PresenterWidget.h"
#include <AppKit/AppKit.h>

//After closing a widget/window where showFullScreen() was called before, we are left on macOS
//with a black screen (https://bugreports.qt.io/browse/QTBUG-46701).
//Explicitly close the native window to workaround this problem.

void PresenterWidget::closeEvent(QCloseEvent* event) {
	QWidget::closeEvent(event);

	NSView* view = reinterpret_cast<NSView*>(winId());
	if (view == nil)
		return;

	NSWindow* window = view.window;
	if (window == nil)
		return;

	[window close];
}
