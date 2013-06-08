/***************************************************************************
    File                 : AxisDock.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2011-2013 by Alexander Semke (alexander.semke*web.de)
    Copyright            : (C) 2013 by Stefan Gerlach (stefan.gerlach*uni-konstanz.de)
							(use @ for *)
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
#include "backend/worksheet/plots/cartesian/Axis.h"

class AbstractAspect;
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
	void positionChanged();
	void scaleChanged(int);
	void autoScaleChanged(int);
	void startChanged();
	void endChanged();
	void zeroOffsetChanged();
	void scalingFactorChanged();

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
	void labelsFormatChanged(int);
	void labelsPrecisionChanged(int);
	void labelsAutoPrecisionChanged(int);
	void labelsPositionChanged(int);
	void labelsOffsetChanged(double);
	void labelsRotationChanged(int);
	void labelsFontChanged(const QFont&);
	void labelsFontColorChanged(const QColor&);	
	void labelsPrefixChanged();
	void labelsSuffixChanged();
	void labelsOpacityChanged(int);

	//"Grid"-tab
  	void majorGridStyleChanged(int);
	void majorGridColorChanged(const QColor&);
	void majorGridWidthChanged(double);
	void majorGridOpacityChanged(int);

  	void minorGridStyleChanged(int);
	void minorGridColorChanged(const QColor&);
	void minorGridWidthChanged(double);
	void minorGridOpacityChanged(int);

	//SLOTs for changes triggered in Axis
	//general
	void axisDescriptionChanged(const AbstractAspect*);
	void axisOrientationChanged(Axis::AxisOrientation);
	void axisPositionChanged(Axis::AxisPosition);
	void axisPositionChanged(float);
	void axisScaleChanged(Axis::AxisScale);
	void axisAutoScaleChanged(bool);
	void axisStartChanged(float);
	void axisEndChanged(float);
	void axisZeroOffsetChanged(qreal);
	void axisScalingFactorChanged(qreal);

	//line
	void axisLinePenChanged(const QPen&);
	void axisLineOpacityChanged(qreal);

	//ticks
	void axisMajorTicksDirectionChanged(Axis::TicksDirection);
	void axisMajorTicksTypeChanged(Axis::TicksType);
	void axisMajorTicksNumberChanged(int);
	void axisMajorTicksIncrementChanged(qreal);
	void axisMajorTicksPenChanged(QPen);
	void axisMajorTicksLengthChanged(qreal);
	void axisMajorTicksOpacityChanged(qreal);
	void axisMinorTicksDirectionChanged(Axis::TicksDirection);
	void axisMinorTicksTypeChanged(Axis::TicksType);
	void axisMinorTicksNumberChanged(int);
	void axisMinorTicksIncrementChanged(qreal);
	void axisMinorTicksPenChanged(QPen);
	void axisMinorTicksLengthChanged(qreal);
	void axisMinorTicksOpacityChanged(qreal);
	
	//labels
	void axisLabelsFormatChanged(Axis::LabelsFormat);
	void axisLabelsAutoPrecisionChanged(bool on); 
	void axisLabelsPrecisionChanged(int precision);
	void axisLabelsPositionChanged(Axis::LabelsPosition position);
	void axisLabelsOffsetChanged(float offset);
	void axisLabelsRotationAngleChanged(qreal rotation);
	void axisLabelsFontChanged(QFont font);
	void axisLabelsFontColorChanged(QColor color);
	void axisLabelsPrefixChanged(QString prefix);
	void axisLabelsSuffixChanged(QString suffix);
	void axisLabelsOpacityChanged(qreal opacity);

	//grids
	void axisMajorGridPenChanged(QPen);
	void axisMajorGridOpacityChanged(qreal);
	void axisMinorGridPenChanged(QPen);
	void axisMinorGridOpacityChanged(qreal);

	void axisVisibleChanged(bool);
	
	//save/load
	void loadConfig(KConfig&);
	void saveConfig(KConfig&);
};

#endif
