/***************************************************************************
    File                 : FunctionsWidget.h
    Project              : LabPlot
    Description          : widget for selecting functions
    --------------------------------------------------------------------
    Copyright            : (C) 2014 by Alexander Semke (alexander.semke@web.de)

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

signals:
	void functionSelected(const QString&);
	void canceled();

private slots:
	void groupChanged(int);
	void filterChanged(const QString&);
	void insertClicked();
};

#endif //FUNCTIONSWIDGET_H
