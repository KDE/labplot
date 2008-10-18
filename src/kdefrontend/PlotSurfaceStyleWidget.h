/***************************************************************************
    File                 : PlotSurfaceStyleWidget.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Alexander Semke
    Email (use @ for *)  : alexander.semke*web.de
    Description          : widget for surface plot style

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
#ifndef PLOTSURFACESTYLEWIDGET_H
#define PLOTSURFACESTYLEWIDGET_H

#include "../ui_plotsurfacestylewidget.h"
#include "PlotStyleWidget.h"

/*!
 * @brief Represents the widget where all the style settings of a surface plot can be modified
*
 * This widget is embedded in \c FunctionPlotWidget.
 */
class PlotSurfaceStyleWidget : public QWidget, public PlotStyleWidgetInterface{
    Q_OBJECT

public:
    PlotSurfaceStyleWidget(QWidget*);
    ~PlotSurfaceStyleWidget();

	void setStyle(const Style* );
	void saveStyle(Style*) const;

private:
	Ui::PlotSurfaceStyleWidget ui;

private slots:
	void selectColorMap();
};

#endif
