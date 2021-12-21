/*
    File                 : FunctionsWidget.h
    Project              : LabPlot
    Description          : widget for selecting functions
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2014 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FUNCTIONSWIDGET_H
#define FUNCTIONSWIDGET_H

#include <QWidget>

#include "ui_functionswidget.h"

class ExpressionParser;

class FunctionsWidget: public QWidget {
	Q_OBJECT

public:
	explicit FunctionsWidget(QWidget*);

private:
	Ui::FunctionsWidget ui;
	ExpressionParser* m_expressionParser;

Q_SIGNALS:
	void functionSelected(const QString&);
	void canceled();

private Q_SLOTS:
	void groupChanged(int);
	void filterChanged(const QString&);
	void insertClicked();
};

#endif //FUNCTIONSWIDGET_H
