/*
    File                 : DynamicPresenterWidget.h
    Project              : LabPlot
    Description          : Widget for dynamic presenting of worksheets
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016 Fabian Kristof <fkristofszabolcs@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/
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
#ifdef Q_OS_MAC
	void closeEvent(QCloseEvent*) override;
#endif

private slots:
	void slideDown();
	void slideUp();
};

#endif // DYNAMICPRESENTERWIDGET_H
