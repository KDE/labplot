/***************************************************************************
    File                 : AxisDock.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2011-2012 by Alexander Semke
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
#ifndef AXESDOCK_H
#define AXESDOCK_H

#include "ui_axisdock.h"

class Axis;
class LabelWidget;

class AxisDock : public QWidget{
    Q_OBJECT

public:
    AxisDock(QWidget*);
    ~AxisDock();

	void setAxes(QList<Axis*>);
	void activateTitleTab();

private:
	Ui::AxisDock ui;
	QList<Axis*> m_axesList;
	Axis* m_axis;
	LabelWidget* labelWidget;
	bool m_dataChanged;
	bool m_initializing;

private slots:
	void init();

	//SLOTs for changes triggered in AxisDock
	//"General"-tab
	void nameChanged();
	void commentChanged();
	void visibilityChanged(int);
	void orientationChanged(int);
	void positionChanged(int);
	void positionOffsetChanged();
	void scaleChanged(int);
	void startChanged();
	void endChanged();
	void zeroOffsetChanged();
	void scalingFactorChanged();

	//"Title"-tab
	void titleChanged();

	//Line-Tab
  	void lineStyleChanged(int);
	void lineColorChanged(const QColor&);
	void lineWidthChanged(double);
	void lineOpacityChanged(int);
	
	//"Major ticks"-tab
	void majorTicksDirectionChanged(int);
	void majorTicksTypeChanged(int);
 	void majorTicksNumberChanged(int);
	void majorTicksIncrementChanged();
	void majorTicksLineStyleChanged(int);
	void majorTicksColorChanged(const QColor&);
	void majorTicksWidthChanged(double);
	void majorTicksLengthChanged(double);
	void majorTicksOpacityChanged(int);
	
	//"Minor ticks"-tab
	void minorTicksDirectionChanged(int);
	void minorTicksTypeChanged(int);
 	void minorTicksNumberChanged(int);
	void minorTicksIncrementChanged();
	void minorTicksLineStyleChanged(int);
	void minorTicksColorChanged(const QColor&);
	void minorTicksWidthChanged(double);
	void minorTicksLengthChanged(double);
	void minorTicksOpacityChanged(int);
	
	//"Extra ticks"-tab
	
	//"Tick labels"-tab
	void labelsPositionChanged(int);
	void labelsOffsetChanged(double);
	void labelsRotationChanged(int);
	void labelsFontChanged(const QFont&);
	void labelsFontColorChanged(const QColor&);	
	void labelsPrefixChanged();
	void labelsSuffixChanged();
	void labelsOpacityChanged(int);
	
	//"Grid"-tab
	//TODO
	
	//SLOTs for changes triggered in Axis
	void axisOrientationChanged();

	//save/load
	void loadConfig(KConfig&);
	void saveConfig(KConfig&);
};

#endif
