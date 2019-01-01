/***************************************************************************
    File                 : ConstansWidget.h
    Project              : LabPlot
    Description          : widget for selecting constants
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
