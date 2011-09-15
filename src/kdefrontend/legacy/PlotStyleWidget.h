/***************************************************************************
    File                 : PlotStyleWidget.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Alexander Semke
    Email (use @ for *)  : alexander.semke*web.de
    Description          : plot style widget

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
#ifndef PLOTSTYLEWIDGET_H
#define PLOTSTYLEWIDGET_H

#include "ui_plotstylewidget.h"

class Style;

class PlotStyleWidgetInterface{
public:
	virtual void setStyle(const Style* )=0;
	virtual void saveStyle(Style*) const=0;
	virtual ~PlotStyleWidgetInterface(){}
};


/**
 * \brief Represents the widget where all the style setting of a plot can be modified.
*
 * This widget is embedded in \c FunctionWidget.
 */
class PlotStyleWidget : public QWidget, public PlotStyleWidgetInterface{
    Q_OBJECT

public:
    PlotStyleWidget(QWidget*);
    ~PlotStyleWidget();
	void setStyle(const Style* );
	void saveStyle(Style*) const;

private:
	Ui::PlotStyleWidget ui;
	void resizeEvent(QResizeEvent *);

private slots:
	void symbolTypeChanged(int);
	void symbolFillingColorChanged();

	void fillLineStyleBox();
	void fillAreaFillingPatternBox();
	void fillSymbolTypeBox();
	void fillSymbolFillingBox();
	void fillSymbolFillingPatternBox();

	void boxWidthStateChanged(int);
};

#endif
