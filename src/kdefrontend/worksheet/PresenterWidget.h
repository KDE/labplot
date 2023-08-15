/*
	File                 : PresenterWidget.h
	Project              : LabPlot
	Description          : Widget for dynamic presenting of worksheets
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2016 Fabian Kristof <fkristofszabolcs@gmail.com>
	SPDX-FileCopyrightText: 2018-2023 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef PRESENTERWIDGET_H
#define PRESENTERWIDGET_H

#include <QWidget>

class QLabel;
class QTimeLine;
class QPushButton;
class SlidingPanel;
class WorksheetView;
class Worksheet;

class PresenterWidget : public QWidget {
	Q_OBJECT

public:
	explicit PresenterWidget(Worksheet* worksheet, bool interactive = false, QWidget* parent = nullptr);
	~PresenterWidget() override;

private:
	Worksheet* m_worksheet;
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

private Q_SLOTS:
	void slideDown();
	void slideUp();
};

#endif // PRESENTERWIDGET_H
