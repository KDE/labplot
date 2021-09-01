/*
    File                 : MemoryWidget.h
    Project              : LabPlot
    Description          : widget showing the amount of memory consumed by the process
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2018 Alexander Semke <alexander.semke@web.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MEMORYWIDGET_H
#define MEMORYWIDGET_H

#include <QTimer>
#include <QLabel>

class MemoryWidget : public QLabel {
	Q_OBJECT

public:
	explicit MemoryWidget(QWidget*);

public slots:
	void refreshMemoryInfo();

private:
	QTimer m_timer;
};

#endif

