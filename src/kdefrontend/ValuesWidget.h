/***************************************************************************
    File                 : ValuesWidget.cc
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Alexander Semke
    Email (use @ for *)  : alexander.semke*web.de
    Description          : values widget class

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
#ifndef VALUESWIDGET_H
#define VALUESWIDGET_H

#include "../ui_valueswidget.h"

/**
 * \brief Widget for changing the properties of the values annotation.
*
* This widget is embedded in \c FunctionPlotWidget.
 */
class ValuesWidget: public QWidget{
	Q_OBJECT

public:
	ValuesWidget(QWidget *parent);
	~ValuesWidget();

signals:
	void dataChanged(bool);

public slots:


private:
	Ui::ValuesWidget ui;

private slots:
	void labelFormatChanged(const QString&);
	void slotDataChanged();
};

#endif //VALUESWIDGET_H
