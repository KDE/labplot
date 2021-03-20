/***************************************************************************
    File                 : AxisDock.h
    Project              : LabPlot
    Description          : axes widget class
    --------------------------------------------------------------------
    Copyright            : (C) 2011-2018 by Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2013 by Stefan Gerlach (stefan.gerlach@uni-konstanz.de)

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
#ifndef AXISDOCK_H
#define AXISDOCK_H

#include "ui_axisdock.h"
#include "backend/worksheet/plots/cartesian/Axis.h"
#include "kdefrontend/dockwidgets/BaseDock.h"
#include <KConfig>

class AbstractAspect;
class LabelWidget;
class TreeViewComboBox;
class AspectTreeModel;
class AbstractColumn;
class DateTimeSpinBox;

class AxisDock : public BaseDock {
	Q_OBJECT

public:
	explicit AxisDock(QWidget*);
	~AxisDock() override;

	void setAxes(QList<Axis*>);
	void activateTitleTab();
	void updateLocale() override;
	void updateAutoScale();

private:
	Ui::AxisDock ui;
	QList<Axis*> m_axesList;
	Axis* m_axis{nullptr};
	AspectTreeModel* m_aspectTreeModel{nullptr};
	LabelWidget* labelWidget;
	TreeViewComboBox* cbMajorTicksColumn;
	TreeViewComboBox* cbMinorTicksColumn;
	TreeViewComboBox* cbLabelsTextColumn;
	bool m_dataChanged{false};

	void setModel();
	void setModelIndexFromColumn(TreeViewComboBox*, const AbstractColumn*);
	void updatePlotRanges() const override;

	void load();
	void loadConfig(KConfig&);

	// own created widgets
	DateTimeSpinBox* dtsbMajorTicksIncrement {nullptr};
	DateTimeSpinBox* dtsbMinorTicksIncrement {nullptr};

private slots:
	void init();

	//SLOTs for changes triggered in AxisDock
	//"General"-tab
	void visibilityChanged(bool);
	void orientationChanged(int);
	void positionChanged(int);
	void positionChanged();
	void scaleChanged(int);
	void autoScaleChanged(int);
	void startChanged();
	void endChanged();
	void startDateTimeChanged(const QDateTime&);
	void endDateTimeChanged(const QDateTime&);
	void zeroOffsetChanged();
	void scalingFactorChanged();

	//Line-Tab
  	void lineStyleChanged(int);
	void lineColorChanged(const QColor&);
	void lineWidthChanged(double);
	void lineOpacityChanged(int);
	void arrowPositionChanged(int);
	void arrowTypeChanged(int);
	void arrowSizeChanged(int);

	//"Major ticks"-tab
	void majorTicksDirectionChanged(int);
	void majorTicksTypeChanged(int);
 	void majorTicksNumberChanged(int);
	void majorTicksSpacingChanged();
	void majorTicksColumnChanged(const QModelIndex&);
	void majorTicksLineStyleChanged(int);
	void majorTicksColorChanged(const QColor&);
	void majorTicksWidthChanged(double);
	void majorTicksLengthChanged(double);
	void majorTicksOpacityChanged(int);

	//"Minor ticks"-tab
	void minorTicksDirectionChanged(int);
	void minorTicksTypeChanged(int);
 	void minorTicksNumberChanged(int);
	void minorTicksSpacingChanged();
	void minorTicksColumnChanged(const QModelIndex&);
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
	void labelsDateTimeFormatChanged();
	void labelsPositionChanged(int);
	void labelsOffsetChanged(double);
	void labelsRotationChanged(int);
	void labelsTextTypeChanged(int);
	void labelsTextColumnChanged(const QModelIndex&);
	void labelsFontChanged(const QFont&);
	void labelsFontColorChanged(const QColor&);
	void labelsBackgroundTypeChanged(int);
	void labelsBackgroundColorChanged(const QColor&);
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
	//General-Tab
	void axisDescriptionChanged(const AbstractAspect*);
	void axisOrientationChanged(Axis::Orientation);
	void axisPositionChanged(Axis::Position);
	void axisPositionChanged(double);
	void axisScaleChanged(RangeT::Scale);
	void axisAutoScaleChanged(bool);
	void axisStartChanged(double);
	void axisEndChanged(double);
	void axisZeroOffsetChanged(qreal);
	void axisScalingFactorChanged(qreal);

	//line
	void axisLinePenChanged(const QPen&);
	void axisLineOpacityChanged(qreal);
	void axisArrowTypeChanged(Axis::ArrowType);
	void axisArrowPositionChanged(Axis::ArrowPosition);
	void axisArrowSizeChanged(qreal);

	//ticks
	void axisMajorTicksDirectionChanged(Axis::TicksDirection);
	void axisMajorTicksTypeChanged(Axis::TicksType);
	void axisMajorTicksNumberChanged(int);
	void axisMajorTicksSpacingChanged(qreal);
	void axisMajorTicksColumnChanged(const AbstractColumn*);
	void axisMajorTicksPenChanged(const QPen&);
	void axisMajorTicksLengthChanged(qreal);
	void axisMajorTicksOpacityChanged(qreal);
	void axisMinorTicksDirectionChanged(Axis::TicksDirection);
	void axisMinorTicksTypeChanged(Axis::TicksType);
	void axisMinorTicksNumberChanged(int);
	void axisMinorTicksSpacingChanged(qreal);
	void axisMinorTicksColumnChanged(const AbstractColumn*);
	void axisMinorTicksPenChanged(const QPen&);
	void axisMinorTicksLengthChanged(qreal);
	void axisMinorTicksOpacityChanged(qreal);

	//labels
	void axisLabelsFormatChanged(Axis::LabelsFormat);
	void axisLabelsAutoPrecisionChanged(bool);
	void axisLabelsPrecisionChanged(int);
	void axisLabelsDateTimeFormatChanged(const QString&);
	void axisLabelsPositionChanged(Axis::LabelsPosition);
	void axisLabelsOffsetChanged(double);
	void axisLabelsRotationAngleChanged(qreal);
	void axisLabelsTextTypeChanged(Axis::LabelsTextType);
	void axisLabelsTextColumnChanged(const AbstractColumn*);
	void axisLabelsFontChanged(const QFont&);
	void axisLabelsFontColorChanged(const QColor&);
	void axisLabelsBackgroundTypeChanged(Axis::LabelsBackgroundType);
	void axisLabelsBackgroundColorChanged(const QColor&);
	void axisLabelsPrefixChanged(const QString&);
	void axisLabelsSuffixChanged(const QString&);
	void axisLabelsOpacityChanged(qreal);

	//grids
	void axisMajorGridPenChanged(const QPen&);
	void axisMajorGridOpacityChanged(qreal);
	void axisMinorGridPenChanged(const QPen&);
	void axisMinorGridOpacityChanged(qreal);

	void axisVisibilityChanged(bool);

	//save/load template
	void loadConfigFromTemplate(KConfig&);
	void saveConfigAsTemplate(KConfig&);

signals:
	void info(const QString&);
};

#endif
