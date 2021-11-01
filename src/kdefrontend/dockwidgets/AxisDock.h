/*
    File                 : AxisDock.h
    Project              : LabPlot
    Description          : axes widget class
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2011-2021 Alexander Semke <alexander.semke@web.de>
    SPDX-FileCopyrightText: 2013-2021 Stefan Gerlach <stefan.gerlach@uni-konstanz.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef AXISDOCK_H
#define AXISDOCK_H

#include "ui_axisdock.h"
#include "kdefrontend/dockwidgets/BaseDock.h"

class AbstractAspect;
class LabelWidget;
class TreeViewComboBox;
class AspectTreeModel;
class AbstractColumn;
class DateTimeSpinBox;
class KConfig;

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

	void setOffset(double);

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
	void positionChanged(double value);
	void logicalPositionChanged(double value);
	void scaleChanged(int);
	void rangeTypeChanged(int);
	void startChanged();
	void endChanged();
	void startDateTimeChanged(const QDateTime&);
	void endDateTimeChanged(const QDateTime&);
	void zeroOffsetChanged();
	void scalingFactorChanged();
	void showScaleOffsetChanged();
	void setLeftOffset();
	void setCenterOffset();
	void setRightOffset();
	void setUnityScale();
	void setUnityRange();

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
	void majorTickStartOffsetChanged();
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
	void axisOrientationChanged(Axis::Orientation);
	void axisPositionChanged(Axis::Position);
	void axisPositionChanged(double);
	void axisLogicalPositionChanged(double);
	void axisScaleChanged(RangeT::Scale);
	void axisRangeTypeChanged(Axis::RangeType);
	void axisStartChanged(double);
	void axisEndChanged(double);
	void axisScalingFactorChanged(qreal);
	void axisZeroOffsetChanged(qreal);
	void axisShowScaleOffsetChanged(bool);

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
	void axisMajorTickStartOffsetChanged(qreal);
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
