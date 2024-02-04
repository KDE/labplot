/*
	File                 : SparklineRunnable.h
	Project              : LabPlot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Kuntal Bar <barkuntal6@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef SPARKLINERUNNABLE_H
#define SPARKLINERUNNABLE_H

#include <QObject>
#include <QRunnable>

#include <backend/core/column/Column.h>

class SparkLineRunnable : public QObject, public QRunnable {
	Q_OBJECT
public:
	SparkLineRunnable(Column* col)
		: QObject(nullptr)
		, col(col) {
	}

	void run() override;
	QPixmap getResultPixmap();

Q_SIGNALS:
	void taskFinished(const QPixmap& resultPixmap);

private:
	Column* col;
	QPixmap resultPixmap{QPixmap()};
};

#endif // SPARKLINERUNNABLE_H