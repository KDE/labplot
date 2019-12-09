/***************************************************************************
File                 : PresenterWidget_mac.mm
Project              : LabPlot
Description          : Reimplementation of QWidget::closeEvent() to workaround QTBUG-46701
--------------------------------------------------------------------
Copyright            : (C) 2019 by Alexander Semke (alexander.semke@web.de)
***************************************************************************/

/***************************************************************************
*                                                                         *
*  This program is free software; you can redistribute it and/or modify   *
*  it under the terms of the GNU General Public License as published by   *
*  the Free Software Foundation; either version 2 of the License, or      *
*  (at your option) any later version.                                    *
*                                                                         *
*  This program is distributed in the hope that it will be useful,        *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
*  GNU General Public License for more details.                           *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the Free Software           *
*   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
*   Boston, MA  02110-1301  USA                                           *
*                                                                         *
***************************************************************************/
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
