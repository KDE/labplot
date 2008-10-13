/***************************************************************************
    File                 : AxesWidget.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Alexander Semke
    Email (use @ for *)  : alexander.semke*web.de
    Description          : axes widget class
                           
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
#ifndef AXESWIDGET_H
#define AXESWIDGET_H

#include <QtGui>

#include "../ui_axeswidget.h"
#include "../plots/Plot.h"
#include "../elements/Axis.h"

class LabelWidget;

/**
 * @brief Represents the widget where all the axes setting can be modified
 * This widget is embedded in \c AxisDialog.
 */
class AxesWidget : public QWidget{
    Q_OBJECT

public:
    AxesWidget(QWidget*);
    ~AxesWidget();

	void setAxes( QList<Axis>*, const int axisNumber=0);
	void saveAxes(QList<Axis>*);
	void setPlotType(const Plot::PlotType& );

private:
	Ui::AxesWidget ui;
	LabelWidget* labelWidget	;
	QList<Axis> listAxes;
	Plot::PlotType plotType;
	int currentAxis;
	bool m_dataChanged;
	bool initializing;
	void showAxis(const short);
	void saveAxis(const short);
	void resizeEvent(QResizeEvent *);

signals:
	void dataChanged(bool);

public slots:
	void restoreDefaults();

private slots:
	void currentAxisChanged(int);

	//"General"-tab
	void axisStateChanged(int);
	void scaleTypeChanged(int);
	void positionChanged(int);

	//"Ticks"-tab
	void ticksTypeChanged(int);
	void majorTicksNumberTypeChanged(int);
	void minorTicksNumberTypeChanged(int);

	//"Tick labels"-tab
	void labelFormatChanged(const QString&);

	//"Grid"-tab
	void createMajorGridStyles();
	void createMinorGridStyles();

	void slotDataChanged();
};

#endif
