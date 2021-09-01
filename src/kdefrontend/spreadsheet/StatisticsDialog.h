/*
    File                 : StatisticsDialog.h
    Project              : LabPlot
    Description          : Dialog showing statistics for column values
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016-2017 Fabian Kristof <fkristofszabolcs@gmail.com>
	SPDX-FileCopyrightText: 2016-2021 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef STATISTICSDIALOG_H
#define STATISTICSDIALOG_H

#include <QDialog>

class Column;
class QTabWidget;

class StatisticsDialog : public QDialog {
	Q_OBJECT

public:
	explicit StatisticsDialog(const QString&, const QVector<Column*>&, QWidget *parent = nullptr);
	~StatisticsDialog() override;
	void showStatistics();

private:
	QTabWidget* m_twStatistics;
	QVector<Column*> m_columns;

private slots:
	void currentTabChanged(int);
};

#endif
