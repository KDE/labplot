/*
File                 : PresenterWidget.h
Project              : LabPlot
Description          : Widget for static presenting of worksheets
--------------------------------------------------------------------
SPDX-FileCopyrightText: 2016 Fabian Kristof (fkristofszabolcs@gmail.com)
SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef PRESENTERWIDGET_H
#define PRESENTERWIDGET_H

#include <QWidget>

class QLabel;
class QTimeLine;
class QPushButton;
class SlidingPanel;

class PresenterWidget : public QWidget {
	Q_OBJECT

public:
	explicit PresenterWidget(const QPixmap& pixmap, const QString& worksheetName, QWidget *parent = nullptr);
	~PresenterWidget() override;

private:
	QLabel* m_imageLabel;
	QTimeLine* m_timeLine;
	SlidingPanel* m_panel;
	void startTimeline();

protected:
	void keyPressEvent(QKeyEvent*) override;
	void focusOutEvent(QFocusEvent*) override;
	bool eventFilter(QObject*, QEvent*) override;
#ifdef Q_OS_MAC
	void closeEvent(QCloseEvent*) override;
#endif

private slots:
	void slideDown();
	void slideUp();
};

#endif // PRESENTERWIDGET_H
