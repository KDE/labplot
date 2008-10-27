/***************************************************************************
    File                 : FunctionWidget.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Stefan Gerlach, Alexander Semke
    Email (use @ for *)  : stefan.gerlach*uni-konstanz.de, alexander.semke*web.de
    Description          : widget for creating plot function
                           
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

#include "ui_functionplotwidget.h"
#include "plots/Plot.h"

class Set;
class LabelWidget;
class PlotStyleWidgetInterface;
class ValuesWidget;
class QMdiSubWindow;

/**
 * @brief Represents the widget where a function can be created/edited
 * This widget is embedded in \c FunctionPlotDialog.
 */
class FunctionPlotWidget : public QWidget{
    Q_OBJECT

public:
    FunctionPlotWidget(QWidget*);
    ~FunctionPlotWidget();

	void setSet(Set*);
	void saveSet(Set*);
	void setPlotType(const Plot::PlotType&);

private:
	Ui::FunctionPlotWidget ui;
	LabelWidget* labelWidget;
	ValuesWidget* valuesWidget;
	PlotStyleWidgetInterface* plotStyleWidget;
	Plot::PlotType plotType;

	int createSetData(Set* set);

private slots:
	void insert(const QString s);
};

#endif
