/***************************************************************************
File                 : DynamicPresenterWidget.h
Project              : LabPlot
Description          : Widget for dynamic presenting of worksheets
--------------------------------------------------------------------
Copyright            : (C) 2016 by Fabian Kristof (fkristofszabolcs@gmail.com)
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
#ifndef DYNAMICPRESENTERWIDGET_H
#define DYNAMICPRESENTERWIDGET_H

#include <QWidget>

class QLabel;
class QTimeLine;
class QPushButton;
class SlidingPanel;
class WorksheetView;
class Worksheet;

class DynamicPresenterWidget : public QWidget {
	Q_OBJECT

public:
	explicit DynamicPresenterWidget(Worksheet* worksheet, QWidget *parent = nullptr);
	~DynamicPresenterWidget() override;

private:
	WorksheetView* m_view;
	QTimeLine* m_timeLine;
	SlidingPanel* m_panel;
	void startTimeline();

protected:
	void keyPressEvent(QKeyEvent*) override;
	bool eventFilter(QObject*, QEvent*) override;
	void focusOutEvent(QFocusEvent*) override;

private slots:
	void slideDown();
	void slideUp();
};

#endif // DYNAMICPRESENTERWIDGET_H
