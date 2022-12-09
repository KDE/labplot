/*
	File                 : AxisDock.h
	Project              : LabPlot
	Description          : axes widget class
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2011-2022 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2013-2021 Stefan Gerlach <stefan.gerlach@uni-konstanz.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef AXISDOCK_H
#define AXISDOCK_H

#include "commonfrontend/widgets/NumberSpinBox.h"
#include "kdefrontend/dockwidgets/BaseDock.h"
#include "ui_axisdock.h"

class AbstractAspect;
class LineWidget;
class LabelWidget;
class TreeViewComboBox;
class AspectTreeModel;
class AbstractColumn;
class DateTimeSpinBox;
class NumberSpinBox;
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
	LineWidget* lineWidget{nullptr};
	LineWidget* majorTicksLineWidget{nullptr};
	LineWidget* minorTicksLineWidget{nullptr};
	LineWidget* majorGridLineWidget{nullptr};
	LineWidget* minorGridLineWidget{nullptr};
	bool m_dataChanged{false};

	void setModel();
	void setModelIndexFromColumn(TreeViewComboBox*, const AbstractColumn*);
	void updatePlotRanges() override;
	void updateMajorTicksStartType(bool visible);
	void initConnections();

	void load();
	void loadConfig(KConfig&);
	void updatePositionText(Axis::Orientation);
	void updateLabelsPosition(Axis::LabelsPosition);

	void setOffset(double);

	// own created widgets
	DateTimeSpinBox* dtsbMajorTicksIncrement{nullptr};
	DateTimeSpinBox* dtsbMinorTicksIncrement{nullptr};

	QVector<QMetaObject::Connection> m_connections;

	friend class AxisTest;

private Q_SLOTS:
	void init();

	// SLOTs for changes triggered in AxisDock
	//"General"-tab
	void visibilityChanged(bool);
	void orientationChanged(int);
	void positionChanged(int);
	void positionChanged(double value);
	void logicalPositionChanged(double value);
	void scaleChanged(int);
	void rangeTypeChanged(int);
	void startChanged(double);
	void endChanged(double);
	void startDateTimeChanged(const QDateTime&);
	void endDateTimeChanged(const QDateTime&);
	void zeroOffsetChanged(double);
	void scalingFactorChanged(double);
	void showScaleOffsetChanged(bool);
	void setLeftOffset();
	void setCenterOffset();
	void setRightOffset();
	void setUnityScale();
	void setUnityRange();

	// Line-Tab
	void arrowPositionChanged(int);
	void arrowTypeChanged(int);
	void arrowSizeChanged(int);

	//"Major ticks"-tab
	void majorTicksDirectionChanged(int);
	void majorTicksTypeChanged(int);
	void majorTicksAutoNumberChanged(int);
	void majorTicksNumberChanged(int);
	void majorTicksSpacingChanged();
	void majorTicksColumnChanged(const QModelIndex&);
	void majorTicksStartTypeChanged(int state);
	void majorTicksStartOffsetChanged(double);
	void majorTicksStartValueChanged(double);
	void setTickOffsetData(bool nice = false); // set first tick on first data point (if nice: nice value)
	void setTickOffsetAuto() {
		setTickOffsetData(true);
	}
	void majorTicksLengthChanged(double);

	//"Minor ticks"-tab
	void minorTicksDirectionChanged(int);
	void minorTicksTypeChanged(int);
	void minorTicksAutoNumberChanged(int);
	void minorTicksNumberChanged(int);
	void minorTicksSpacingChanged();
	void minorTicksColumnChanged(const QModelIndex&);
	void minorTicksLengthChanged(double);

	//"Extra ticks"-tab

	//"Tick labels"-tab
	void labelsFormatChanged(int);
	void labelsFormatAutoChanged(bool);
	void labelsPrecisionChanged(int);
	void labelsAutoPrecisionChanged(bool);
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

	// SLOTs for changes triggered in Axis
	// General-Tab
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

	// line
	void axisArrowTypeChanged(Axis::ArrowType);
	void axisArrowPositionChanged(Axis::ArrowPosition);
	void axisArrowSizeChanged(qreal);

	// ticks
	void axisMajorTicksDirectionChanged(Axis::TicksDirection);
	void axisMajorTicksTypeChanged(Axis::TicksType);
	void axisMajorTicksAutoNumberChanged(bool);
	void axisMajorTicksNumberChanged(int);
	void axisMajorTicksSpacingChanged(qreal);
	void axisMajorTicksStartTypeChanged(Axis::TicksStartType);
	void axisMajorTicksStartOffsetChanged(qreal);
	void axisMajorTicksStartValueChanged(qreal);
	void axisMajorTicksColumnChanged(const AbstractColumn*);
	void axisMajorTicksLengthChanged(qreal);

	void axisMinorTicksDirectionChanged(Axis::TicksDirection);
	void axisMinorTicksTypeChanged(Axis::TicksType);
	void axisMinorTicksAutoNumberChanged(bool);
	void axisMinorTicksNumberChanged(int);
	void axisMinorTicksSpacingChanged(qreal);
	void axisMinorTicksColumnChanged(const AbstractColumn*);
	void axisMinorTicksLengthChanged(qreal);

	// labels
	void axisLabelsFormatChanged(Axis::LabelsFormat);
	void axisLabelsFormatAutoChanged(bool);
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

	void axisVisibilityChanged(bool);

	// save/load template
	void loadConfigFromTemplate(KConfig&);
	void saveConfigAsTemplate(KConfig&);

Q_SIGNALS:
	void info(const QString&);
};

#endif
