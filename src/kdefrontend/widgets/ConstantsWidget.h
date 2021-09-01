/*
    File                 : ConstansWidget.h
    Project              : LabPlot
    Description          : widget for selecting constants
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2014 Alexander Semke (alexander.semke@web.de)

*/

/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/
#ifndef CONSTANTSWIDGET_H
#define CONSTANTSWIDGET_H

#include <QWidget>

#include "ui_constantswidget.h"

class QStringList;
class ExpressionParser;

class ConstantsWidget: public QWidget {
	Q_OBJECT

public:
	explicit ConstantsWidget(QWidget*);

private:
	Ui::ConstantsWidget ui;
	ExpressionParser* m_expressionParser;

signals:
	void constantSelected(const QString&);
	void canceled();

private slots:
	void groupChanged(int);
	void filterChanged(const QString&);
	void constantChanged(const QString&);
	void insertClicked();
};

#endif //CONSTANTSWIDGET_H
