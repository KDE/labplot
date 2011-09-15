/***************************************************************************
    File                 : FunctionWidget.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2009 by Stefan Gerlach, Alexander Semke
    Email (use @ for *)  : stefan.gerlach*uni-konstanz.de, alexander.semke*web.de
    Description          : widget for creating a data set from a function

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
#ifndef FUNCTIONWIDGET_H
#define FUNCTIONWIDGET_H

#include "ui_functionwidget.h"
#include "plots/Plot.h"

class Set;


class FunctionWidget : public QWidget{
    Q_OBJECT

	public:
		FunctionWidget(QWidget*, const Plot::PlotType& type=Plot::PLOT2D);
		~FunctionWidget();

		void init();
		void setSet(Set*);
		void saveSet(Set*);

	private:
		Ui::FunctionWidget ui;
		Plot::PlotType plotType;
		int createSetData(Set* set);

	private slots:
		void insertSlot(const QString&);
		void functionChangedSlot(const QString&);
		void syncStatusChangedSlot(bool);

	 signals:
		void functionChanged(const QString&);
};

#endif
