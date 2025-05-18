/*
	File                 : AxisDock.cpp
	Project              : LabPlot
	Description          : axes widget class
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2011-2025 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2012-2024 Stefan Gerlach <stefan.gerlach@uni-konstanz.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "AxisDock.h"
#include "backend/core/column/Column.h"
#include "backend/lib/macros.h"
#include "backend/worksheet/Worksheet.h"
#include "frontend/GuiTools.h"
#include "frontend/TemplateHandler.h"
#include "frontend/widgets/DateTimeSpinBox.h"
#include "frontend/widgets/LabelWidget.h"
#include "frontend/widgets/LineWidget.h"
#include "frontend/widgets/NumberSpinBox.h"
#include "frontend/widgets/TreeViewComboBox.h"

#include <KConfig>

#include <QPainter>

#include "backend/nsl/nsl_math.h"
#include <gsl/gsl_math.h>

namespace {
enum PositionAlignmentComboBoxIndex {
	Top_Left = 0,
	Bottom_Right = 1,
	Center = 2,
	Logical = 3,
};
}

/*!
 \class AxisDock
 \brief Provides a widget for editing the properties of the axes currently selected in the project explorer.

 \ingroup frontend
*/

AxisDock::AxisDock(QWidget* parent)
	: BaseDock(parent) {
	ui.setupUi(this);
	setPlotRangeCombobox(ui.cbPlotRanges);
	setBaseWidgets(ui.leName, ui.teComment);
	setVisibilityWidgets(ui.chkVisible);

	//"Title"-tab
	auto* hboxLayout = new QHBoxLayout(ui.tabTitle);
	labelWidget = new LabelWidget(ui.tabTitle);
	labelWidget->setFixedLabelMode(true);
	hboxLayout->addWidget(labelWidget);
	hboxLayout->setContentsMargins(0, 0, 0, 0);
	hboxLayout->setSpacing(0);

	// "Line"-tab
	auto* gridLayout = qobject_cast<QGridLayout*>(ui.tabLine->layout());
	lineWidget = new LineWidget(ui.tabLine);
	gridLayout->addWidget(lineWidget, 1, 0, 1, 3);

	//"Ticks"-tab
	// major ticks
	gridLayout = static_cast<QGridLayout*>(ui.tabTicks->layout());
	dtsbMajorTicksIncrement = new DateTimeSpinBox(ui.tabTicks);
	gridLayout->addWidget(dtsbMajorTicksIncrement, 5, 2);

	auto layout = ui.tabTicks->findChild<QHBoxLayout*>(QStringLiteral("layoutMajorTickStartOffset"));
	dtsbMajorTicksDateTimeStartOffset = new DateTimeSpinBox(ui.tabTicks);
	layout->insertWidget(0, dtsbMajorTicksDateTimeStartOffset);

	cbMajorTicksColumn = new TreeViewComboBox(ui.tabTicks);
	gridLayout->addWidget(cbMajorTicksColumn, 10, 2);

	cbLabelsTextColumn = new TreeViewComboBox(ui.tabTicks);
	gridLayout->addWidget(cbLabelsTextColumn, 12, 2);

	majorTicksLineWidget = new LineWidget(ui.tabTicks);
	gridLayout->addWidget(majorTicksLineWidget, 15, 0, 1, 3);

	// minor ticks
	dtsbMinorTicksIncrement = new DateTimeSpinBox(ui.tabTicks);
	gridLayout->addWidget(dtsbMinorTicksIncrement, 22, 2);

	cbMinorTicksColumn = new TreeViewComboBox(ui.tabTicks);
	gridLayout->addWidget(cbMinorTicksColumn, 23, 2);

	minorTicksLineWidget = new LineWidget(ui.tabTicks);
	gridLayout->addWidget(minorTicksLineWidget, 26, 0, 1, 3);

	// "Grid"-tab
	gridLayout = qobject_cast<QGridLayout*>(ui.tabGrid->layout());
	majorGridLineWidget = new LineWidget(ui.tabLine);
	gridLayout->addWidget(majorGridLineWidget, 1, 0, 1, 3);

	minorGridLineWidget = new LineWidget(ui.tabLine);
	gridLayout->addWidget(minorGridLineWidget, 4, 0, 1, 3);

	// adjust layouts in the tabs
	for (int i = 0; i < ui.tabWidget->count(); ++i) {
		auto* layout = dynamic_cast<QGridLayout*>(ui.tabWidget->widget(i)->layout());
		if (!layout)
			continue;

		layout->setContentsMargins(2, 2, 2, 2);
		layout->setHorizontalSpacing(2);
		layout->setVerticalSpacing(2);
	}

	ui.cbLabelsDateTimeFormat->addItems(AbstractColumn::dateTimeFormats());
	ui.cbArrowType->setIconSize(QSize(20, 20));

	updateLocale();
	retranslateUi();

	//**********************************  Slots **********************************************

	//"General"-tab
	connect(ui.kcbAxisColor, &KColorButton::changed, this, &AxisDock::colorChanged);
	connect(ui.cbOrientation, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AxisDock::orientationChanged);
	connect(ui.cbPosition, QOverload<int>::of(&QComboBox::currentIndexChanged), this, QOverload<int>::of(&AxisDock::positionChanged));
	connect(ui.sbPosition, QOverload<double>::of(&NumberSpinBox::valueChanged), this, QOverload<double>::of(&AxisDock::positionChanged));
	connect(ui.sbPositionLogical, QOverload<double>::of(&NumberSpinBox::valueChanged), this, QOverload<double>::of(&AxisDock::logicalPositionChanged));
	connect(ui.cbScale, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AxisDock::scaleChanged);
	connect(ui.cbRangeScale, &QCheckBox::toggled, this, &AxisDock::rangeScaleChanged);

	connect(ui.cbRangeType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AxisDock::rangeTypeChanged);
	connect(ui.sbStart, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &AxisDock::startChanged);
	connect(ui.sbEnd, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &AxisDock::endChanged);
	connect(ui.dateTimeEditStart, &UTCDateTimeEdit::mSecsSinceEpochUTCChanged, this, &AxisDock::startDateTimeChanged);
	connect(ui.dateTimeEditEnd, &UTCDateTimeEdit::mSecsSinceEpochUTCChanged, this, &AxisDock::endDateTimeChanged);
	connect(ui.sbZeroOffset, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &AxisDock::zeroOffsetChanged);
	connect(ui.tbOffsetLeft, &QToolButton::clicked, this, &AxisDock::setLeftOffset);
	connect(ui.tbOffsetCenter, &QToolButton::clicked, this, &AxisDock::setCenterOffset);
	connect(ui.tbOffsetRight, &QToolButton::clicked, this, &AxisDock::setRightOffset);

	connect(ui.sbScalingFactor, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &AxisDock::scalingFactorChanged);
	connect(ui.tbUnityScale, &QToolButton::clicked, this, &AxisDock::setUnityScale);
	connect(ui.tbUnityRange, &QToolButton::clicked, this, &AxisDock::setUnityRange);

	connect(ui.chkShowScaleOffset, &QCheckBox::toggled, this, &AxisDock::showScaleOffsetChanged);

	//"Line"-tab
	connect(lineWidget, &LineWidget::colorChanged, this, &AxisDock::updateArrowLineColor);
	connect(ui.cbArrowPosition, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AxisDock::arrowPositionChanged);
	connect(ui.cbArrowType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AxisDock::arrowTypeChanged);
	connect(ui.sbArrowSize, QOverload<int>::of(&QSpinBox::valueChanged), this, &AxisDock::arrowSizeChanged);

	//"Major ticks"-tab
	connect(ui.cbMajorTicksDirection, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AxisDock::majorTicksDirectionChanged);
	connect(ui.cbMajorTicksType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AxisDock::majorTicksTypeChanged);
	connect(ui.cbMajorTicksAutoNumber, &QCheckBox::stateChanged, this, &AxisDock::majorTicksAutoNumberChanged);
	connect(ui.sbMajorTicksNumber, QOverload<int>::of(&QSpinBox::valueChanged), this, &AxisDock::majorTicksNumberChanged);
	connect(ui.sbMajorTicksSpacingNumeric, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &AxisDock::majorTicksSpacingChanged);
	connect(dtsbMajorTicksIncrement, &DateTimeSpinBox::valueChanged, this, &AxisDock::majorTicksSpacingChanged);
	connect(ui.cbMajorTicksStartType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AxisDock::majorTicksStartTypeChanged);
	connect(ui.sbMajorTickStartOffset, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &AxisDock::majorTicksStartOffsetChanged);
	connect(dtsbMajorTicksDateTimeStartOffset, &DateTimeSpinBox::valueChanged, this, &AxisDock::majorTicksDateTimeStartOffsetChanged);
	connect(ui.sbMajorTickStartValue, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &AxisDock::majorTicksStartValueChanged);
	connect(ui.sbMajorTickStartDateTime, &UTCDateTimeEdit::mSecsSinceEpochUTCChanged, this, &AxisDock::majorTicksStartDateTimeChanged);
	connect(ui.tbFirstTickData, &QToolButton::clicked, this, &AxisDock::setTickOffsetData);
	connect(ui.tbFirstTickAuto, &QToolButton::clicked, this, &AxisDock::setTickOffsetAuto);
	connect(cbMajorTicksColumn, &TreeViewComboBox::currentModelIndexChanged, this, &AxisDock::majorTicksColumnChanged);
	connect(ui.sbMajorTicksLength, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &AxisDock::majorTicksLengthChanged);

	//"Minor ticks"-tab
	connect(ui.cbMinorTicksDirection, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AxisDock::minorTicksDirectionChanged);
	connect(ui.cbMinorTicksType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AxisDock::minorTicksTypeChanged);
	connect(ui.cbMinorTicksAutoNumber, &QCheckBox::stateChanged, this, &AxisDock::minorTicksAutoNumberChanged);
	connect(ui.sbMinorTicksNumber, QOverload<int>::of(&QSpinBox::valueChanged), this, &AxisDock::minorTicksNumberChanged);
	connect(ui.sbMinorTicksSpacingNumeric, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &AxisDock::minorTicksSpacingChanged);
	connect(dtsbMinorTicksIncrement, &DateTimeSpinBox::valueChanged, this, &AxisDock::minorTicksSpacingChanged);
	connect(cbMinorTicksColumn, &TreeViewComboBox::currentModelIndexChanged, this, &AxisDock::minorTicksColumnChanged);
	connect(ui.sbMinorTicksLength, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &AxisDock::minorTicksLengthChanged);

	//"Extra ticks"-tab

	//"Tick labels"-tab
	connect(ui.cbLabelsFormat, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AxisDock::labelsFormatChanged);
	connect(ui.chkLabelsFormatAuto, &QCheckBox::toggled, this, &AxisDock::labelsFormatAutoChanged);
	connect(ui.sbLabelsPrecision, QOverload<int>::of(&QSpinBox::valueChanged), this, &AxisDock::labelsPrecisionChanged);
	connect(ui.chkLabelsAutoPrecision, &QCheckBox::toggled, this, &AxisDock::labelsAutoPrecisionChanged);
	connect(ui.cbLabelsDateTimeFormat, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AxisDock::labelsDateTimeFormatChanged);
	connect(ui.cbLabelsDateTimeFormat, &QComboBox::currentTextChanged, this, &AxisDock::labelsDateTimeFormatChanged);
	connect(ui.cbLabelsPosition, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AxisDock::labelsPositionChanged);
	connect(ui.sbLabelsOffset, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &AxisDock::labelsOffsetChanged);
	connect(ui.sbLabelsRotation, QOverload<int>::of(&QSpinBox::valueChanged), this, &AxisDock::labelsRotationChanged);
	connect(ui.cbLabelsTextType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AxisDock::labelsTextTypeChanged);
	connect(cbLabelsTextColumn, &TreeViewComboBox::currentModelIndexChanged, this, &AxisDock::labelsTextColumnChanged);
	connect(ui.kfrLabelsFont, &KFontRequester::fontSelected, this, &AxisDock::labelsFontChanged);
	connect(ui.kcbLabelsFontColor, &KColorButton::changed, this, &AxisDock::labelsFontColorChanged);
	connect(ui.cbLabelsBackgroundType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AxisDock::labelsBackgroundTypeChanged);
	connect(ui.kcbLabelsBackgroundColor, &KColorButton::changed, this, &AxisDock::labelsBackgroundColorChanged);
	connect(ui.leLabelsPrefix, &QLineEdit::textChanged, this, &AxisDock::labelsPrefixChanged);
	connect(ui.leLabelsSuffix, &QLineEdit::textChanged, this, &AxisDock::labelsSuffixChanged);
	connect(ui.sbLabelsOpacity, QOverload<int>::of(&QSpinBox::valueChanged), this, &AxisDock::labelsOpacityChanged);

	// Updating the axis color widget if one of the other color widgets color changes
	connect(lineWidget, &LineWidget::colorChanged, this, &AxisDock::setAxisColor);
	connect(majorTicksLineWidget, &LineWidget::colorChanged, this, &AxisDock::setAxisColor);
	connect(minorTicksLineWidget, &LineWidget::colorChanged, this, &AxisDock::setAxisColor);
	connect(labelWidget, &LabelWidget::labelFontColorChangedSignal, this, &AxisDock::setAxisColor);

	// template handler
	auto* frame = new QFrame(this);
	auto* hlayout = new QHBoxLayout(frame);
	hlayout->setContentsMargins(0, 11, 0, 11);

	auto* templateHandler = new TemplateHandler(this, QLatin1String("Axis"));
	hlayout->addWidget(templateHandler);
	connect(templateHandler, &TemplateHandler::loadConfigRequested, this, &AxisDock::loadConfigFromTemplate);
	connect(templateHandler, &TemplateHandler::saveConfigRequested, this, &AxisDock::saveConfigAsTemplate);
	connect(templateHandler, &TemplateHandler::info, this, &AxisDock::info);

	ui.verticalLayout->addWidget(frame);
}

AxisDock::~AxisDock() = default;

void AxisDock::retranslateUi() {
	CONDITIONAL_LOCK_RETURN;

	ui.cbPosition->clear();
	ui.cbPosition->addItem(i18n("Top")); // Left
	ui.cbPosition->addItem(i18n("Bottom")); // Right
	ui.cbPosition->addItem(i18n("Centered"));
	ui.cbPosition->addItem(i18n("Logical"));

	// range types
	ui.cbRangeType->clear();
	ui.cbRangeType->addItem(i18n("Auto"));
	ui.cbRangeType->addItem(i18n("Auto Data"));
	ui.cbRangeType->addItem(i18n("Custom"));

	QString msg = i18n(
		"Axis range:"
		"<ul>"
		"<li>Auto - automatically set the start and end points of the axis to the current plot ranges</li>"
		"<li>Auto Data - automatically set the start and end points of the axis to the minimal and maximal plotted data points, respectively</li>"
		"<li>Custom - manually specify the start and end points of the axis</li>"
		"</ul>");
	ui.lRangeType->setToolTip(msg);
	ui.cbRangeType->setToolTip(msg);

	ui.cbMajorTicksAutoNumber->setToolTip(i18n("If enabled: Use the major tick number as base to determine the new number of ticks for nice looking values. The number of ticks might be different than the number previously set."));

	// scales
	ui.cbScale->clear();
	for (const auto& name : RangeT::scaleNames)
		ui.cbScale->addItem(name.toString());

	ui.cbOrientation->clear();
	ui.cbOrientation->addItem(i18n("Horizontal"));
	ui.cbOrientation->addItem(i18n("Vertical"));

	// Arrows
	ui.cbArrowType->clear();
	ui.cbArrowType->addItem(i18n("No arrow"));
	ui.cbArrowType->addItem(i18n("Simple, Small"));
	ui.cbArrowType->addItem(i18n("Simple, Big"));
	ui.cbArrowType->addItem(i18n("Filled, Small"));
	ui.cbArrowType->addItem(i18n("Filled, Big"));
	ui.cbArrowType->addItem(i18n("Semi-filled, Small"));
	ui.cbArrowType->addItem(i18n("Semi-filled, Big"));
	if (m_axis)
		updateArrowLineColor(m_axis->line()->color()); // call this to re-create the icons after the retranslate

	ui.cbArrowPosition->clear();
	ui.cbArrowPosition->addItem(i18n("Left"));
	ui.cbArrowPosition->addItem(i18n("Right"));
	ui.cbArrowPosition->addItem(i18n("Both"));

	ui.cbMajorTicksDirection->clear();
	ui.cbMajorTicksDirection->addItem(i18n("None"));
	ui.cbMajorTicksDirection->addItem(i18n("In"));
	ui.cbMajorTicksDirection->addItem(i18n("Out"));
	ui.cbMajorTicksDirection->addItem(i18n("In and Out"));

	ui.cbMajorTicksType->clear();
	ui.cbMajorTicksType->addItem(i18n("Number"), (int)Axis::TicksType::TotalNumber);
	ui.cbMajorTicksType->addItem(i18n("Spacing"), (int)Axis::TicksType::Spacing);
	ui.cbMajorTicksType->addItem(i18n("Custom column"), (int)Axis::TicksType::CustomColumn);
	ui.cbMajorTicksType->addItem(i18n("Column labels"), (int)Axis::TicksType::ColumnLabels);

	ui.cbMajorTicksStartType->clear();
	ui.cbMajorTicksStartType->addItem(i18n("Absolute Value"));
	ui.cbMajorTicksStartType->addItem(i18n("Offset"));

	ui.cbMinorTicksDirection->clear();
	ui.cbMinorTicksDirection->addItem(i18n("None"));
	ui.cbMinorTicksDirection->addItem(i18n("In"));
	ui.cbMinorTicksDirection->addItem(i18n("Out"));
	ui.cbMinorTicksDirection->addItem(i18n("In and Out"));

	ui.cbMinorTicksType->clear();
	ui.cbMinorTicksType->addItem(i18n("Number"), (int)Axis::TicksType::TotalNumber);
	ui.cbMinorTicksType->addItem(i18n("Spacing"), (int)Axis::TicksType::Spacing);
	ui.cbMinorTicksType->addItem(i18n("Custom column"), (int)Axis::TicksType::CustomColumn);
	// ui.cbMinorTicksType->addItem(i18n("Column labels"), (int)Axis::TicksType::ColumnLabels);

	// labels
	ui.cbLabelsPosition->clear();
	ui.cbLabelsPosition->addItem(i18n("No labels"));
	ui.cbLabelsPosition->addItem(i18n("Top"));
	ui.cbLabelsPosition->addItem(i18n("Bottom"));

	ui.cbLabelsTextType->clear();
	ui.cbLabelsTextType->addItem(i18n("Position values"), (int)Axis::LabelsTextType::PositionValues);
	ui.cbLabelsTextType->addItem(i18n("Custom column"), (int)Axis::LabelsTextType::CustomValues);

	// see Axis::labelsFormatToIndex() and Axis::indexToLabelsFormat()
	ui.cbLabelsFormat->clear();
	ui.cbLabelsFormat->addItem(i18n("Decimal notation"));
	ui.cbLabelsFormat->addItem(i18n("Scientific notation"));
	ui.cbLabelsFormat->addItem(i18n("Scientific E notation"));
	ui.cbLabelsFormat->addItem(i18n("Powers of 10"));
	ui.cbLabelsFormat->addItem(i18n("Powers of 2"));
	ui.cbLabelsFormat->addItem(i18n("Powers of e"));
	ui.cbLabelsFormat->addItem(i18n("Multiples of π"));

	ui.cbLabelsBackgroundType->clear();
	ui.cbLabelsBackgroundType->addItem(i18n("Transparent"));
	ui.cbLabelsBackgroundType->addItem(i18n("Color"));

	labelWidget->retranslateUi();
	// TODO: lineWidget->retranslateUi();
}

void AxisDock::setModel() {
	QList<AspectType> list{AspectType::Folder, AspectType::Workbook, AspectType::Spreadsheet, AspectType::Notebook, AspectType::Column};
	cbMajorTicksColumn->setTopLevelClasses(list);
	cbMinorTicksColumn->setTopLevelClasses(list);
	cbLabelsTextColumn->setTopLevelClasses(list);

	list = {AspectType::Column};
	auto* model = aspectModel();
	model->setSelectableAspects(list);

	cbMajorTicksColumn->setModel(model);
	cbMinorTicksColumn->setModel(model);
	cbLabelsTextColumn->setModel(model);
}

/*!
  sets the axes. The properties of the axes in the list \c list can be edited in this widget.
*/
void AxisDock::setAxes(QList<Axis*> list) {
	QDEBUG(Q_FUNC_INFO << ", Axis LIST =" << list)
	CONDITIONAL_LOCK_RETURN;
	m_axesList = list;
	m_axis = list.first();
	setAspects(list);
	Q_ASSERT(m_axis != nullptr);
	this->setModel();

	labelWidget->setAxes(list);

	// if there are more than one axis in the list, disable the tab "general"
	if (list.size() == 1) {
		this->setModelIndexFromColumn(cbMajorTicksColumn, m_axis->majorTicksColumn());
		this->setModelIndexFromColumn(cbMinorTicksColumn, m_axis->minorTicksColumn());
		this->setModelIndexFromColumn(cbLabelsTextColumn, m_axis->labelsTextColumn());
	} else {
		cbMajorTicksColumn->setCurrentModelIndex(QModelIndex());
		cbMinorTicksColumn->setCurrentModelIndex(QModelIndex());
		cbLabelsTextColumn->setCurrentModelIndex(QModelIndex());
	}

	// show the properties of the first axis
	this->load();

	QList<Line*> lines;
	QList<Line*> majorTicksLines;
	QList<Line*> minorTicksLines;
	QList<Line*> majorGridLines;
	QList<Line*> minorGridLines;
	for (auto* axis : m_axesList) {
		lines << axis->line();
		majorTicksLines << axis->majorTicksLine();
		minorTicksLines << axis->minorTicksLine();
		majorGridLines << axis->majorGridLine();
		minorGridLines << axis->minorGridLine();
	}

	lineWidget->setLines(lines);
	majorTicksLineWidget->setLines(majorTicksLines);
	minorTicksLineWidget->setLines(minorTicksLines);
	majorGridLineWidget->setLines(majorGridLines);
	minorGridLineWidget->setLines(minorGridLines);

	updatePlotRangeList();
	initConnections();
}

void AxisDock::initConnections() {
	// general
	connect(m_axis, &Axis::orientationChanged, this, QOverload<Axis::Orientation>::of(&AxisDock::axisOrientationChanged));
	connect(m_axis, QOverload<Axis::Position>::of(&Axis::positionChanged), this, QOverload<Axis::Position>::of(&AxisDock::axisPositionChanged));
	connect(m_axis, QOverload<double>::of(&Axis::positionChanged), this, QOverload<double>::of(&AxisDock::axisPositionChanged));
	connect(m_axis, &Axis::logicalPositionChanged, this, &AxisDock::axisLogicalPositionChanged);
	connect(m_axis, &Axis::scaleChanged, this, &AxisDock::axisScaleChanged);
	connect(m_axis, &Axis::rangeScaleChanged, this, &AxisDock::axisRangeScaleChanged);
	connect(m_axis, &Axis::rangeTypeChanged, this, &AxisDock::axisRangeTypeChanged);
	connect(m_axis, &Axis::startChanged, this, &AxisDock::axisStartChanged);
	connect(m_axis, &Axis::endChanged, this, &AxisDock::axisEndChanged);
	connect(m_axis, &Axis::zeroOffsetChanged, this, &AxisDock::axisZeroOffsetChanged);
	connect(m_axis, &Axis::scalingFactorChanged, this, &AxisDock::axisScalingFactorChanged);
	connect(m_axis, &Axis::showScaleOffsetChanged, this, &AxisDock::axisShowScaleOffsetChanged);

	// line
	connect(m_axis, &Axis::arrowTypeChanged, this, &AxisDock::axisArrowTypeChanged);
	connect(m_axis, &Axis::arrowPositionChanged, this, &AxisDock::axisArrowPositionChanged);
	connect(m_axis, &Axis::arrowSizeChanged, this, &AxisDock::axisArrowSizeChanged);

	// ticks
	connect(m_axis, &Axis::majorTicksDirectionChanged, this, &AxisDock::axisMajorTicksDirectionChanged);
	connect(m_axis, &Axis::majorTicksTypeChanged, this, &AxisDock::axisMajorTicksTypeChanged);
	connect(m_axis, &Axis::majorTicksAutoNumberChanged, this, &AxisDock::axisMajorTicksAutoNumberChanged);
	connect(m_axis, &Axis::majorTicksNumberChanged, this, &AxisDock::axisMajorTicksNumberChanged);
	connect(m_axis, &Axis::majorTicksSpacingChanged, this, &AxisDock::axisMajorTicksSpacingChanged);
	connect(m_axis, &Axis::majorTicksStartTypeChanged, this, &AxisDock::axisMajorTicksStartTypeChanged);
	connect(m_axis, &Axis::majorTickStartOffsetChanged, this, &AxisDock::axisMajorTicksStartOffsetChanged);
	connect(m_axis, &Axis::majorTickStartValueChanged, this, &AxisDock::axisMajorTicksStartValueChanged);
	connect(m_axis, &Axis::majorTicksColumnChanged, this, &AxisDock::axisMajorTicksColumnChanged);
	connect(m_axis, &Axis::majorTicksLengthChanged, this, &AxisDock::axisMajorTicksLengthChanged);
	connect(m_axis, &Axis::minorTicksDirectionChanged, this, &AxisDock::axisMinorTicksDirectionChanged);
	connect(m_axis, &Axis::minorTicksTypeChanged, this, &AxisDock::axisMinorTicksTypeChanged);
	connect(m_axis, &Axis::minorTicksAutoNumberChanged, this, &AxisDock::axisMinorTicksAutoNumberChanged);
	connect(m_axis, &Axis::minorTicksNumberChanged, this, &AxisDock::axisMinorTicksNumberChanged);
	connect(m_axis, &Axis::minorTicksIncrementChanged, this, &AxisDock::axisMinorTicksSpacingChanged);
	connect(m_axis, &Axis::minorTicksColumnChanged, this, &AxisDock::axisMinorTicksColumnChanged);
	connect(m_axis, &Axis::minorTicksLengthChanged, this, &AxisDock::axisMinorTicksLengthChanged);

	// labels
	connect(m_axis, &Axis::labelsFormatChanged, this, &AxisDock::axisLabelsFormatChanged);
	connect(m_axis, &Axis::labelsFormatAutoChanged, this, &AxisDock::axisLabelsFormatAutoChanged);
	connect(m_axis, &Axis::labelsAutoPrecisionChanged, this, &AxisDock::axisLabelsAutoPrecisionChanged);
	connect(m_axis, &Axis::labelsPrecisionChanged, this, &AxisDock::axisLabelsPrecisionChanged);
	connect(m_axis, &Axis::labelsDateTimeFormatChanged, this, &AxisDock::axisLabelsDateTimeFormatChanged);
	connect(m_axis, &Axis::labelsPositionChanged, this, &AxisDock::axisLabelsPositionChanged);
	connect(m_axis, &Axis::labelsOffsetChanged, this, &AxisDock::axisLabelsOffsetChanged);
	connect(m_axis, &Axis::labelsRotationAngleChanged, this, &AxisDock::axisLabelsRotationAngleChanged);
	connect(m_axis, &Axis::labelsTextTypeChanged, this, &AxisDock::axisLabelsTextTypeChanged);
	connect(m_axis, &Axis::labelsTextColumnChanged, this, &AxisDock::axisLabelsTextColumnChanged);
	connect(m_axis, &Axis::labelsFontChanged, this, &AxisDock::axisLabelsFontChanged);
	connect(m_axis, &Axis::labelsColorChanged, this, &AxisDock::axisLabelsFontColorChanged);
	connect(m_axis, &Axis::labelsBackgroundTypeChanged, this, &AxisDock::axisLabelsBackgroundTypeChanged);
	connect(m_axis, &Axis::labelsBackgroundColorChanged, this, &AxisDock::axisLabelsBackgroundColorChanged);
	connect(m_axis, &Axis::labelsPrefixChanged, this, &AxisDock::axisLabelsPrefixChanged);
	connect(m_axis, &Axis::labelsSuffixChanged, this, &AxisDock::axisLabelsSuffixChanged);
	connect(m_axis, &Axis::labelsOpacityChanged, this, &AxisDock::axisLabelsOpacityChanged);
}

/*
 * updates the locale in the widgets. called when the application settins are changed.
 */
void AxisDock::updateLocale() {
	const auto numberLocale = QLocale();
	ui.sbMajorTicksSpacingNumeric->setLocale(numberLocale);
	ui.sbMajorTicksLength->setLocale(numberLocale);
	ui.sbMinorTicksSpacingNumeric->setLocale(numberLocale);
	ui.sbMinorTicksLength->setLocale(numberLocale);
	ui.sbLabelsOffset->setLocale(numberLocale);

	// update the QLineEdits, avoid the change events
	CONDITIONAL_LOCK_RETURN;
	ui.sbPosition->setLocale(numberLocale);
	ui.sbStart->setLocale(numberLocale);
	ui.sbEnd->setLocale(numberLocale);

	// scales
	ui.cbScale->clear();
	for (const auto& name : RangeT::scaleNames)
		ui.cbScale->addItem(name.toString());

	labelWidget->updateLocale();
	lineWidget->updateLocale();
	majorTicksLineWidget->updateLocale();
	minorTicksLineWidget->updateLocale();
	majorGridLineWidget->updateLocale();
	minorGridLineWidget->updateLocale();
}

void AxisDock::updatePositionText(Axis::Orientation orientation) {
	switch (orientation) {
	case Axis::Orientation::Horizontal: {
		ui.cbPosition->setItemText(Top_Left, i18n("Top"));
		ui.cbPosition->setItemText(Bottom_Right, i18n("Bottom"));
		// ui.cbPosition->setItemText(Center, i18n("Center") ); // must not updated
		ui.cbLabelsPosition->setItemText(1, i18n("Top"));
		ui.cbLabelsPosition->setItemText(2, i18n("Bottom"));
		break;
	}
	case Axis::Orientation::Vertical: {
		ui.cbPosition->setItemText(Top_Left, i18n("Left"));
		ui.cbPosition->setItemText(Bottom_Right, i18n("Right"));
		// ui.cbPosition->setItemText(Center, i18n("Center") ); // must not updated
		ui.cbLabelsPosition->setItemText(1, i18n("Right"));
		ui.cbLabelsPosition->setItemText(2, i18n("Left"));
		break;
	}
	case Axis::Orientation::Both:
		break;
	}
}

void AxisDock::activateTitleTab() {
	ui.tabWidget->setCurrentWidget(ui.tabTitle);
}

void AxisDock::setModelIndexFromColumn(TreeViewComboBox* cb, const AbstractColumn* column) {
	if (column)
		cb->setCurrentModelIndex(aspectModel()->modelIndexOfAspect(column));
	else
		cb->setCurrentModelIndex(QModelIndex());
}

void AxisDock::updatePlotRangeList() {
	BaseDock::updatePlotRangeList();

	if (m_axis->coordinateSystemCount() == 0)
		return;

	auto orientation = m_axis->orientation();
	Range<double> logicalRange;
	if (orientation == Axis::Orientation::Horizontal)
		logicalRange = m_axis->plot()->range(Dimension::Y, m_axis->plot()->coordinateSystem(m_axis->coordinateSystemIndex())->index(Dimension::Y));
	else
		logicalRange = m_axis->plot()->range(Dimension::X, m_axis->plot()->coordinateSystem(m_axis->coordinateSystemIndex())->index(Dimension::X));
	spinBoxCalculateMinMax(ui.sbPositionLogical, logicalRange, ui.sbPositionLogical->value());
}

void AxisDock::updateAutoScale() {
	m_axis->setRangeType(static_cast<Axis::RangeType>(ui.cbRangeType->currentIndex()));
}

void AxisDock::updateLabelsPosition(Axis::LabelsPosition position) {
	bool b = (position != Axis::LabelsPosition::NoLabels);
	ui.lLabelsOffset->setEnabled(b);
	ui.sbLabelsOffset->setEnabled(b);
	ui.lLabelsRotation->setEnabled(b);
	ui.sbLabelsRotation->setEnabled(b);
	ui.lLabelsFont->setEnabled(b);
	ui.kfrLabelsFont->setEnabled(b);
	ui.lLabelsColor->setEnabled(b);
	ui.kcbLabelsFontColor->setEnabled(b);
	ui.lLabelsPrefix->setEnabled(b);
	ui.leLabelsPrefix->setEnabled(b);
	ui.lLabelsSuffix->setEnabled(b);
	ui.leLabelsSuffix->setEnabled(b);
	ui.lLabelsOpacity->setEnabled(b);
	ui.sbLabelsOpacity->setEnabled(b);
}

//*************************************************************
//********** SLOTs for changes triggered in AxisDock **********
//*************************************************************
//"General"-tab
/*!
 * \brief AxisDock::colorChanged
 * The general color of the axis changes (Title, line, ticks, labels, ...)
 */
void AxisDock::colorChanged(const QColor& color) {
	CONDITIONAL_LOCK_RETURN;

	// Set colors of all other ui elements
	// - Line widget color
	// - Title color
	// - Major tick color
	// - Minor tick color
	// - Tick label color

	const int axesCount = m_axesList.count();
	if (axesCount == 1)
		m_axis->beginMacro(i18n("%1: set axis color", m_axis->name()));
	else
		m_axis->beginMacro(i18n("%1 axes: set color", axesCount));

	lineWidget->setColor(color);
	ui.kcbLabelsFontColor->setColor(color);
	majorTicksLineWidget->setColor(color);
	minorTicksLineWidget->setColor(color);

	for (auto* axis : m_axesList) {
		labelWidget->fontColorChanged(color);
		labelWidget->labelFontColorChanged(color);
		// must be done, because kcbLabelsFontColor is in this class and
		// because of the CONDITIONAL_LOCK_RETURN this function will not be called
		axis->setLabelsColor(color);
	}
	m_axis->endMacro();
}

/*!
	called if the orientation (horizontal or vertical) of the current axis is changed.
*/
void AxisDock::orientationChanged(int item) {
	CONDITIONAL_LOCK_RETURN;

	auto orientation{Axis::Orientation(item)};
	updatePositionText(orientation);

	// depending on the current orientation we need to update axis position and labels position

	// axis position, map from the current index in the combobox to the enum value in Axis::Position
	Axis::Position axisPosition;
	int posIndex = ui.cbPosition->currentIndex();
	if (orientation == Axis::Orientation::Horizontal) {
		if (posIndex > 1)
			posIndex += 2;
		axisPosition = Axis::Position(posIndex);
	} else
		axisPosition = Axis::Position(posIndex + 2);

	// labels position
	posIndex = ui.cbLabelsPosition->currentIndex();
	auto labelsPosition = Axis::LabelsPosition(posIndex);

	for (auto* axis : m_axesList) {
		axis->beginMacro(i18n("%1: set axis orientation", axis->name()));
		axis->setOrientation(orientation);
		axis->setPosition(axisPosition);
		axis->setLabelsPosition(labelsPosition);
		axis->endMacro();
	}
}

/*!
	called if one of the predefined axis positions
	(top, bottom, left, right, center or custom) was changed.
*/
void AxisDock::positionChanged(int index) {
	if (index == -1)
		return; // we occasionally get -1 here, nothing to do in this case

	CONDITIONAL_LOCK_RETURN;

	// map from the current index in the combo box to the enum value in Axis::Position,
	// depends on the current orientation
	bool logical = false;
	Axis::Position position;
	if (index == Logical) {
		position = Axis::Position::Logical;
		logical = true;
	} else if (index == Center)
		position = Axis::Position::Centered;
	else if (ui.cbOrientation->currentIndex() == 0) {
		// horizontal
		switch (index) {
		case Bottom_Right:
			position = Axis::Position::Bottom;
			break;
		case Top_Left:
		default:
			position = Axis::Position::Top;
			break;
		}
	} else {
		// vertical
		switch (index) {
		case Bottom_Right:
			position = Axis::Position::Right;
			break;
		case Top_Left:
		default:
			position = Axis::Position::Left;
			break;
		}
	}

	ui.sbPosition->setVisible(!logical);
	ui.sbPositionLogical->setVisible(logical);

	for (auto* axis : m_axesList)
		axis->setPosition(position);
}

/*!
	called when the custom position of the axis in the corresponding LineEdit is changed.
*/
void AxisDock::positionChanged(double value) {
	CONDITIONAL_RETURN_NO_LOCK;

	double offset = Worksheet::convertToSceneUnits(value, m_worksheetUnit);
	for (auto* axis : m_axesList)
		axis->setOffset(offset);
}

void AxisDock::logicalPositionChanged(double value) {
	CONDITIONAL_RETURN_NO_LOCK;

	for (auto* axis : m_axesList)
		axis->setLogicalPosition(value);
}

void AxisDock::scaleChanged(int index) {
	CONDITIONAL_LOCK_RETURN;

	auto scale = static_cast<RangeT::Scale>(index);
	for (auto* axis : m_axesList) {
		axis->setScale(scale);

		if (axis->majorTicksAutoNumber())
			ui.sbMajorTicksNumber->setValue(axis->majorTicksNumber());
	}
}

void AxisDock::rangeScaleChanged(bool set) {
	CONDITIONAL_LOCK_RETURN;
	for (auto* axis : m_axesList)
		axis->setRangeScale(set);
}

void AxisDock::rangeTypeChanged(int index) {
	auto rangeType = static_cast<Axis::RangeType>(index);
	bool autoScale = (rangeType != Axis::RangeType::Custom);
	ui.sbStart->setEnabled(!autoScale);
	ui.sbEnd->setEnabled(!autoScale);
	ui.dateTimeEditStart->setEnabled(!autoScale);
	ui.dateTimeEditEnd->setEnabled(!autoScale);

	CONDITIONAL_LOCK_RETURN;

	for (auto* axis : m_axesList)
		axis->setRangeType(rangeType);
}

void AxisDock::startChanged(double value) {
	CONDITIONAL_RETURN_NO_LOCK;

	for (auto* axis : m_axesList)
		axis->setStart(value);
}

void AxisDock::endChanged(double value) {
	CONDITIONAL_RETURN_NO_LOCK;

	for (auto* axis : m_axesList)
		axis->setEnd(value);
}

void AxisDock::startDateTimeChanged(qint64 value) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* axis : m_axesList)
		axis->setStart(value);
}

void AxisDock::endDateTimeChanged(qint64 value) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* axis : m_axesList)
		axis->setEnd(value);
}

void AxisDock::zeroOffsetChanged(double offset) {
	DEBUG(Q_FUNC_INFO)
	CONDITIONAL_RETURN_NO_LOCK;

	for (auto* axis : m_axesList)
		axis->setZeroOffset(offset);
}

void AxisDock::setOffset(double offset) {
	ui.sbZeroOffset->setValue(-offset);
}
void AxisDock::setLeftOffset() {
	setOffset(m_axis->range().start());
}
void AxisDock::setCenterOffset() {
	setOffset(m_axis->range().center());
}
void AxisDock::setRightOffset() {
	setOffset(m_axis->range().end());
}

void AxisDock::scalingFactorChanged(double value) {
	CONDITIONAL_RETURN_NO_LOCK;

	for (auto* axis : m_axesList)
		axis->setScalingFactor(value);
}
void AxisDock::setUnityScale() {
	ui.sbScalingFactor->setValue(1. / m_axis->range().size());
}
// set scale and offset to get a range of 0 .. 1
void AxisDock::setUnityRange() {
	ui.sbScalingFactor->setValue(1. / m_axis->range().size());
	ui.sbZeroOffset->setValue(-m_axis->range().start() / m_axis->range().size());
}

void AxisDock::showScaleOffsetChanged(bool state) {
	DEBUG(Q_FUNC_INFO)
	CONDITIONAL_LOCK_RETURN;
	for (auto* axis : m_axesList)
		axis->setShowScaleOffset(state);
}

// "Line"-tab
void AxisDock::updateArrowLineColor(const QColor& color) {
	QPainter pa;
	QPixmap pm(20, 20);

	// no arrow
	pm.fill(Qt::transparent);
	pa.begin(&pm);
	pa.setRenderHint(QPainter::Antialiasing);
	pa.setPen(QPen(color));
	pa.drawLine(3, 10, 17, 10);
	pa.end();
	ui.cbArrowType->setItemIcon(0, pm);

	// simple, small
	const double cos_phi = cos(M_PI / 6.);
	pm.fill(Qt::transparent);
	pa.begin(&pm);
	pa.setRenderHint(QPainter::Antialiasing);
	pa.setPen(QPen(color));
	pa.drawLine(3, 10, 17, 10);
	pa.drawLine(17, 10, 10, 10 - 5 * cos_phi);
	pa.drawLine(17, 10, 10, 10 + 5 * cos_phi);
	pa.end();
	ui.cbArrowType->setItemIcon(1, pm);

	// simple, big
	pm.fill(Qt::transparent);
	pa.begin(&pm);
	pa.setRenderHint(QPainter::Antialiasing);
	pa.setPen(QPen(color));
	pa.drawLine(3, 10, 17, 10);
	pa.drawLine(17, 10, 10, 10 - 10 * cos_phi);
	pa.drawLine(17, 10, 10, 10 + 10 * cos_phi);
	pa.end();
	ui.cbArrowType->setItemIcon(2, pm);

	// filled, small
	pm.fill(Qt::transparent);
	pa.begin(&pm);
	pa.setRenderHint(QPainter::Antialiasing);
	pa.setPen(QPen(color));
	pa.setBrush(QBrush(color, Qt::SolidPattern));
	pa.drawLine(3, 10, 17, 10);
	QPolygonF points3;
	points3 << QPointF(17, 10) << QPointF(10, 10 - 4 * cos_phi) << QPointF(10, 10 + 4 * cos_phi);
	pa.drawPolygon(points3);
	pa.end();
	ui.cbArrowType->setItemIcon(3, pm);

	// filled, big
	pm.fill(Qt::transparent);
	pa.begin(&pm);
	pa.setRenderHint(QPainter::Antialiasing);
	pa.setPen(QPen(color));
	pa.setBrush(QBrush(color, Qt::SolidPattern));
	pa.drawLine(3, 10, 17, 10);
	QPolygonF points4;
	points4 << QPointF(17, 10) << QPointF(10, 10 - 10 * cos_phi) << QPointF(10, 10 + 10 * cos_phi);
	pa.drawPolygon(points4);
	pa.end();
	ui.cbArrowType->setItemIcon(4, pm);

	// semi-filled, small
	pm.fill(Qt::transparent);
	pa.begin(&pm);
	pa.setRenderHint(QPainter::Antialiasing);
	pa.setPen(QPen(color));
	pa.setBrush(QBrush(color, Qt::SolidPattern));
	pa.drawLine(3, 10, 17, 10);
	QPolygonF points5;
	points5 << QPointF(17, 10) << QPointF(10, 10 - 4 * cos_phi) << QPointF(13, 10) << QPointF(10, 10 + 4 * cos_phi);
	pa.drawPolygon(points5);
	pa.end();
	ui.cbArrowType->setItemIcon(5, pm);

	// semi-filled, big
	pm.fill(Qt::transparent);
	pa.begin(&pm);
	pa.setRenderHint(QPainter::Antialiasing);
	pa.setPen(QPen(color));
	pa.setBrush(QBrush(color, Qt::SolidPattern));
	pa.drawLine(3, 10, 17, 10);
	QPolygonF points6;
	points6 << QPointF(17, 10) << QPointF(10, 10 - 10 * cos_phi) << QPointF(13, 10) << QPointF(10, 10 + 10 * cos_phi);
	pa.drawPolygon(points6);
	pa.end();
	ui.cbArrowType->setItemIcon(6, pm);
}

void AxisDock::arrowTypeChanged(int index) {
	CONDITIONAL_LOCK_RETURN;

	auto type = (Axis::ArrowType)index;
	if (type == Axis::ArrowType::NoArrow) {
		ui.cbArrowPosition->setEnabled(false);
		ui.sbArrowSize->setEnabled(false);
	} else {
		ui.cbArrowPosition->setEnabled(true);
		ui.sbArrowSize->setEnabled(true);
	}

	for (auto* axis : m_axesList)
		axis->setArrowType(type);
}

void AxisDock::arrowPositionChanged(int index) {
	CONDITIONAL_LOCK_RETURN;

	auto position = (Axis::ArrowPosition)index;
	for (auto* axis : m_axesList)
		axis->setArrowPosition(position);
}

void AxisDock::arrowSizeChanged(int value) {
	CONDITIONAL_LOCK_RETURN;

	double v = Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point);
	for (auto* axis : m_axesList)
		axis->setArrowSize(v);
}

//"Major ticks" tab
void AxisDock::majorTicksDirectionChanged(int index) {
	const auto direction = Axis::TicksDirection(index);
	const bool b = (direction != Axis::noTicks);
	const bool numeric = m_axis->isNumeric();
	ui.cbMajorTicksType->setEnabled(b);
	ui.cbMajorTicksType->setEnabled(b);
	ui.cbMajorTicksAutoNumber->setEnabled(b);

	if (b)
		ui.sbMajorTicksNumber->setEnabled(true);
	else
		ui.sbMajorTicksNumber->setEnabled(false);

	ui.sbMajorTicksSpacingNumeric->setEnabled(b);
	dtsbMajorTicksIncrement->setEnabled(b);
	ui.cbMajorTicksStartType->setEnabled(b);
	ui.sbMajorTickStartValue->setEnabled(b && numeric);
	ui.sbMajorTickStartDateTime->setEnabled(b && !numeric);
	ui.sbMajorTickStartOffset->setEnabled(b && numeric);
	dtsbMajorTicksDateTimeStartOffset->setEnabled(b && !numeric);
	ui.tbFirstTickData->setEnabled(b);
	ui.tbFirstTickAuto->setEnabled(b);
	cbMajorTicksColumn->setEnabled(b);
	ui.cbLabelsTextType->setEnabled(b);
	cbLabelsTextColumn->setEnabled(b);
	ui.sbMajorTicksLength->setEnabled(b);
	majorTicksLineWidget->setEnabled(b);

	CONDITIONAL_LOCK_RETURN;

	for (auto* axis : m_axesList)
		axis->setMajorTicksDirection(direction);
}

/*!
	called if the current type of the ticks is changed.
	Shows/hides the corresponding widgets.
*/
void AxisDock::majorTicksTypeChanged(int index) {
	if (!m_axis) // If elements are added to the combobox 'cbMajorTicksType' (at init of this class), then this function is called, which is a problem if no
				 // axis are available
		return;

	auto type = (Axis::TicksType)ui.cbMajorTicksType->itemData(index).toInt(); // WRONG!

	ui.lLabelsTextType->setVisible(type != Axis::TicksType::ColumnLabels);
	ui.cbLabelsTextType->setVisible(type != Axis::TicksType::ColumnLabels);
	const auto customValues = m_axis->labelsTextType() == Axis::LabelsTextType::CustomValues;
	ui.lLabelsTextColumn->setVisible(type != Axis::TicksType::ColumnLabels && customValues);
	cbLabelsTextColumn->setVisible(type != Axis::TicksType::ColumnLabels && customValues);

	switch (type) {
	case Axis::TicksType::TotalNumber: {
		ui.lMajorTicksNumber->show();
		ui.sbMajorTicksNumber->show();
		ui.lMajorTicksSpacingNumeric->hide();
		ui.sbMajorTicksSpacingNumeric->hide();
		ui.cbMajorTicksAutoNumber->show();
		ui.lMajorTicksStartType->show();
		ui.cbMajorTicksStartType->show();
		ui.lMajorTicksIncrementDateTime->hide();
		dtsbMajorTicksIncrement->hide();
		ui.lMajorTicksColumn->hide();
		cbMajorTicksColumn->hide();
		ui.sbZeroOffset->show();
		ui.tbFirstTickAuto->show();
		ui.tbFirstTickData->show();
		updateMajorTicksStartType(true);
		break;
	}
	case Axis::TicksType::Spacing: {
		ui.lMajorTicksNumber->hide();
		ui.sbMajorTicksNumber->hide();
		ui.cbMajorTicksAutoNumber->hide();
		ui.lMajorTicksSpacingNumeric->show();
		ui.lMajorTicksStartType->show();
		ui.cbMajorTicksStartType->show();

		const bool numeric = m_axis->isNumeric();

		ui.lMajorTicksSpacingNumeric->setVisible(numeric);
		ui.sbMajorTicksSpacingNumeric->setVisible(numeric);

		ui.lMajorTicksIncrementDateTime->setVisible(!numeric);
		dtsbMajorTicksIncrement->setVisible(!numeric);
		ui.lMajorTicksIncrementDateTime->setVisible(!numeric);
		dtsbMajorTicksDateTimeStartOffset->setVisible(!numeric);

		ui.lMajorTicksColumn->hide();
		cbMajorTicksColumn->hide();
		ui.sbZeroOffset->show();
		ui.tbFirstTickAuto->show();
		ui.tbFirstTickData->show();
		updateMajorTicksStartType(true);

		// Check if spacing is not too small
		majorTicksSpacingChanged();
		break;
	}
	case Axis::TicksType::ColumnLabels:
		// Fall through
	case Axis::TicksType::CustomColumn: {
		ui.lMajorTicksNumber->show();
		ui.sbMajorTicksNumber->show();
		ui.cbMajorTicksAutoNumber->show();
		ui.lMajorTicksSpacingNumeric->hide();
		ui.sbMajorTicksSpacingNumeric->hide();
		ui.lMajorTicksIncrementDateTime->hide();
		dtsbMajorTicksIncrement->hide();
		ui.lMajorTicksStartType->hide();
		ui.cbMajorTicksStartType->hide();
		ui.sbZeroOffset->hide();
		ui.tbFirstTickAuto->hide();
		ui.tbFirstTickData->hide();

		ui.lMajorTicksColumn->show();
		cbMajorTicksColumn->show();

		updateMajorTicksStartType(false);
		break;
	}
	case Axis::TicksType::CustomValues:
		break;
	}

	CONDITIONAL_LOCK_RETURN;

	for (auto* axis : m_axesList)
		axis->setMajorTicksType(type);
}

void AxisDock::majorTicksAutoNumberChanged(int state) {
	bool automatic = (state == Qt::CheckState::Checked ? true : false);

	CONDITIONAL_LOCK_RETURN;

	for (auto* axis : m_axesList)
		axis->setMajorTicksAutoNumber(automatic);

	if (automatic)
		ui.sbMajorTicksNumber->setValue(m_axis->majorTicksNumber()); // set new value
}

void AxisDock::majorTicksNumberChanged(int value) {
	DEBUG(Q_FUNC_INFO << ", number = " << value)
	CONDITIONAL_LOCK_RETURN;

	for (auto* axis : m_axesList)
		axis->setMajorTicksNumber(value);
}

void AxisDock::majorTicksSpacingChanged() {
	bool numeric = m_axis->isNumeric();
	double spacing = numeric ? ui.sbMajorTicksSpacingNumeric->value() : dtsbMajorTicksIncrement->value();
	if (numeric) {
		CONDITIONAL_RETURN_NO_LOCK;

		for (auto* axis : m_axesList)
			axis->setMajorTicksSpacing(spacing);
	} else {
		CONDITIONAL_LOCK_RETURN;

		for (auto* axis : m_axesList)
			axis->setMajorTicksSpacing(spacing);
	}
}

void AxisDock::majorTicksStartTypeChanged(int state) {
	updateMajorTicksStartType(true);

	CONDITIONAL_LOCK_RETURN;

	auto type = static_cast<Axis::TicksStartType>(state);
	for (auto* axis : m_axesList)
		axis->setMajorTicksStartType(type);
}

void AxisDock::majorTicksDateTimeStartOffsetChanged() {
	if (m_axis->isNumeric())
		return;
	const qint64 value = dtsbMajorTicksDateTimeStartOffset->value();
	ui.sbMajorTickStartOffset->setClearButtonEnabled(value != 0);

	CONDITIONAL_LOCK_RETURN;
	for (auto* axis : m_axesList)
		axis->setMajorTickStartOffset(value);
}

void AxisDock::majorTicksStartOffsetChanged(double value) {
	ui.sbMajorTickStartOffset->setClearButtonEnabled(value != 0);

	CONDITIONAL_RETURN_NO_LOCK;

	for (auto* axis : m_axesList)
		axis->setMajorTickStartOffset(value);
}

void AxisDock::majorTicksStartDateTimeChanged(qint64 value) {
	ui.sbMajorTickStartValue->setClearButtonEnabled(value != 0);

	CONDITIONAL_RETURN_NO_LOCK;

	for (auto* axis : m_axesList)
		axis->setMajorTickStartValue(value);
}

void AxisDock::majorTicksStartValueChanged(double value) {
	ui.sbMajorTickStartValue->setClearButtonEnabled(value != 0);

	CONDITIONAL_RETURN_NO_LOCK;

	for (auto* axis : m_axesList)
		axis->setMajorTickStartValue(value);
}

void AxisDock::setTickOffsetData(bool nice) {
	Range<double> dataRange;
	if (m_axis->orientation() == Axis::Orientation::Horizontal)
		dataRange = m_axis->plot()->dataRange(Dimension::X);
	else
		dataRange = m_axis->plot()->dataRange(Dimension::Y);

	if (nice)
		dataRange.niceExtend();

	DEBUG(Q_FUNC_INFO << ", data range = " << dataRange.toStdString())
	const double offset = dataRange.start() - m_axis->range().start();

	ui.sbMajorTickStartOffset->setValue(offset);
	dtsbMajorTicksDateTimeStartOffset->setValue(offset);
}

void AxisDock::majorTicksColumnChanged(const QModelIndex& index) {
	CONDITIONAL_LOCK_RETURN;

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	AbstractColumn* column = nullptr;
	if (aspect) {
		column = dynamic_cast<AbstractColumn*>(aspect);
		Q_ASSERT(column != nullptr);
	}

	for (auto* axis : m_axesList)
		axis->setMajorTicksColumn(column);
}

void AxisDock::majorTicksLengthChanged(double value) {
	CONDITIONAL_RETURN_NO_LOCK;

	for (auto* axis : m_axesList)
		axis->setMajorTicksLength(Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point));
}

//"Minor ticks" tab
void AxisDock::minorTicksDirectionChanged(int index) {
	const auto direction = Axis::TicksDirection(index);
	const bool b = (direction != Axis::noTicks);
	ui.cbMinorTicksType->setEnabled(b);
	ui.cbMinorTicksType->setEnabled(b);

	if (b) {
		if (ui.cbMinorTicksAutoNumber->isChecked())
			ui.sbMinorTicksNumber->setEnabled(false);
		else
			ui.sbMinorTicksNumber->setEnabled(true);
	} else
		ui.sbMinorTicksNumber->setEnabled(false);

	ui.sbMinorTicksSpacingNumeric->setEnabled(b);
	dtsbMinorTicksIncrement->setEnabled(b);
	ui.sbMinorTicksLength->setEnabled(b);
	minorTicksLineWidget->setEnabled(b);

	CONDITIONAL_LOCK_RETURN;

	for (auto* axis : m_axesList)
		axis->setMinorTicksDirection(direction);
}

void AxisDock::minorTicksTypeChanged(int index) {
	if (!m_axis) // If elements are added to the combobox 'cbMajorTicksType' (at init of this class), then this function is called, which is a problem if no
				 // axis are available
		return;

	auto type = Axis::TicksType(index);
	if (type == Axis::TicksType::TotalNumber) {
		ui.lMinorTicksNumber->show();
		ui.sbMinorTicksNumber->show();
		ui.cbMinorTicksAutoNumber->show();
		ui.lMinorTicksSpacingNumeric->hide();
		ui.sbMinorTicksSpacingNumeric->hide();
		ui.lMinorTicksColumn->hide();
		cbMinorTicksColumn->hide();
		ui.lMinorTicksIncrementDateTime->hide();
		dtsbMinorTicksIncrement->hide();
	} else if (type == Axis::TicksType::Spacing) {
		ui.lMinorTicksNumber->hide();
		ui.sbMinorTicksNumber->hide();
		ui.cbMinorTicksAutoNumber->hide();

		if (m_axis->isNumeric()) {
			ui.lMinorTicksSpacingNumeric->show();
			ui.sbMinorTicksSpacingNumeric->show();
			ui.lMinorTicksIncrementDateTime->hide();
			dtsbMinorTicksIncrement->hide();
		} else {
			ui.lMinorTicksSpacingNumeric->hide();
			ui.sbMinorTicksSpacingNumeric->hide();
			ui.lMinorTicksIncrementDateTime->show();
			dtsbMinorTicksIncrement->show();
		}

		ui.lMinorTicksColumn->hide();
		cbMinorTicksColumn->hide();

		// Check if spacing is not to small
		minorTicksSpacingChanged();
	} else {
		ui.lMinorTicksNumber->hide();
		ui.sbMinorTicksNumber->hide();
		ui.cbMinorTicksAutoNumber->hide();
		ui.lMinorTicksSpacingNumeric->hide();
		ui.sbMinorTicksSpacingNumeric->hide();
		ui.lMinorTicksIncrementDateTime->hide();
		dtsbMinorTicksIncrement->hide();
		ui.lMinorTicksColumn->show();
		cbMinorTicksColumn->show();
	}

	CONDITIONAL_LOCK_RETURN;

	for (auto* axis : m_axesList)
		axis->setMinorTicksType(type);
}

void AxisDock::minorTicksAutoNumberChanged(int state) {
	bool automatic = (state == Qt::CheckState::Checked ? true : false);
	ui.sbMinorTicksNumber->setEnabled(!automatic);

	CONDITIONAL_LOCK_RETURN;

	for (auto* axis : m_axesList)
		axis->setMinorTicksAutoNumber(automatic);

	if (automatic)
		ui.sbMinorTicksNumber->setValue(m_axis->minorTicksNumber()); // set new value
}

void AxisDock::minorTicksNumberChanged(int value) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* axis : m_axesList)
		axis->setMinorTicksNumber(value);
}

void AxisDock::minorTicksSpacingChanged() {
	bool numeric = m_axis->isNumeric();
	double spacing = numeric ? ui.sbMinorTicksSpacingNumeric->value() : dtsbMinorTicksIncrement->value();
	if (numeric) {
		CONDITIONAL_RETURN_NO_LOCK;

		for (auto* axis : m_axesList)
			axis->setMinorTicksSpacing(spacing);
	} else {
		CONDITIONAL_LOCK_RETURN;
		for (auto* axis : m_axesList)
			axis->setMinorTicksSpacing(spacing);
	}
}

void AxisDock::minorTicksColumnChanged(const QModelIndex& index) {
	CONDITIONAL_LOCK_RETURN;

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	auto* column = dynamic_cast<AbstractColumn*>(aspect);
	Q_ASSERT(column != nullptr);

	for (auto* axis : m_axesList)
		axis->setMinorTicksColumn(column);
}

void AxisDock::minorTicksLengthChanged(double value) {
	CONDITIONAL_RETURN_NO_LOCK;

	for (auto* axis : m_axesList)
		axis->setMinorTicksLength(Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point));
}

//"Tick labels"-tab
void AxisDock::labelsFormatChanged(int index) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* axis : m_axesList)
		axis->setLabelsFormat(Axis::indexToLabelsFormat(index));
}

void AxisDock::labelsFormatAutoChanged(bool automatic) {
	// Must be above the lock, because if the axis changes the value without interacting with the
	// dock, this should also change
	ui.cbLabelsFormat->setEnabled(!automatic);

	CONDITIONAL_LOCK_RETURN;

	for (auto* axis : m_axesList)
		axis->setLabelsFormatAuto(automatic);
}

void AxisDock::labelsPrecisionChanged(int value) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* axis : m_axesList)
		axis->setLabelsPrecision(value);
}

void AxisDock::labelsAutoPrecisionChanged(bool state) {
	ui.sbLabelsPrecision->setEnabled(!state);

	CONDITIONAL_LOCK_RETURN;

	for (auto* axis : m_axesList)
		axis->setLabelsAutoPrecision(state);
}

void AxisDock::labelsDateTimeFormatChanged() {
	CONDITIONAL_LOCK_RETURN;

	for (auto* axis : m_axesList)
		axis->setLabelsDateTimeFormat(ui.cbLabelsDateTimeFormat->currentText());
}

void AxisDock::labelsPositionChanged(int index) {
	auto position = Axis::LabelsPosition(index);
	updateLabelsPosition(position);

	CONDITIONAL_LOCK_RETURN;

	for (auto* axis : m_axesList)
		axis->setLabelsPosition(position);
}

void AxisDock::labelsOffsetChanged(double value) {
	CONDITIONAL_RETURN_NO_LOCK;

	for (auto* axis : m_axesList)
		axis->setLabelsOffset(Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point));
}

void AxisDock::labelsRotationChanged(int value) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* axis : m_axesList)
		axis->setLabelsRotationAngle(value);
}

void AxisDock::labelsTextTypeChanged(int index) {
	if (!m_axis)
		return; // don't do anything when we're addItem()'ing strings and the axis is not available yet

	const auto type = static_cast<Axis::LabelsTextType>(ui.cbLabelsTextType->itemData(index).toInt());
	switch (type) {
	case Axis::LabelsTextType::PositionValues: {
		ui.lLabelsTextColumn->hide();
		cbLabelsTextColumn->hide();

		bool numeric = m_axis->isNumeric();
		ui.lLabelsFormat->setVisible(numeric);
		ui.frameLabelsFormat->setVisible(numeric);
		ui.lLabelsPrecision->setVisible(numeric);
		ui.frameLabelsPrecision->setVisible(numeric);
		ui.lLabelsDateTimeFormat->setVisible(!numeric);
		ui.cbLabelsDateTimeFormat->setVisible(!numeric);
		break;
	}
	case Axis::LabelsTextType::CustomValues: {
		ui.lLabelsTextColumn->show();
		cbLabelsTextColumn->show();
		labelsTextColumnChanged(cbLabelsTextColumn->currentModelIndex());
		break;
	}
	}

	CONDITIONAL_LOCK_RETURN;

	for (auto* axis : m_axesList)
		axis->setLabelsTextType(type);
}

void AxisDock::labelsTextColumnChanged(const QModelIndex& index) {
	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	auto* column = dynamic_cast<AbstractColumn*>(aspect);

	if (column) {
		// depending on data format of the column (numeric vs. datetime vs. text),
		// show/hide the corresponding widgets for the tick labels format
		switch (column->columnMode()) {
		case AbstractColumn::ColumnMode::Double:
		case AbstractColumn::ColumnMode::Integer:
		case AbstractColumn::ColumnMode::BigInt:
			ui.lLabelsFormat->show();
			ui.frameLabelsFormat->show();
			ui.lLabelsPrecision->show();
			ui.frameLabelsPrecision->show();
			ui.lLabelsDateTimeFormat->hide();
			ui.cbLabelsDateTimeFormat->hide();
			break;
		case AbstractColumn::ColumnMode::Text:
			ui.lLabelsFormat->hide();
			ui.frameLabelsFormat->hide();
			ui.lLabelsPrecision->hide();
			ui.frameLabelsPrecision->hide();
			ui.lLabelsDateTimeFormat->hide();
			ui.cbLabelsDateTimeFormat->hide();
			break;
		case AbstractColumn::ColumnMode::DateTime:
		case AbstractColumn::ColumnMode::Month:
		case AbstractColumn::ColumnMode::Day:
			ui.lLabelsFormat->hide();
			ui.frameLabelsFormat->hide();
			ui.lLabelsPrecision->hide();
			ui.frameLabelsPrecision->hide();
			ui.lLabelsDateTimeFormat->show();
			ui.cbLabelsDateTimeFormat->show();
			break;
		}
	} else {
		auto type = Axis::LabelsTextType(ui.cbLabelsTextType->currentData().toInt());
		switch (type) {
		case Axis::LabelsTextType::CustomValues:
			ui.lLabelsFormat->hide();
			ui.frameLabelsFormat->hide();
			ui.lLabelsPrecision->hide();
			ui.frameLabelsPrecision->hide();
			ui.lLabelsDateTimeFormat->hide();
			ui.cbLabelsDateTimeFormat->hide();
			break;
		case Axis::LabelsTextType::PositionValues:
			break;
		}
	}

	CONDITIONAL_LOCK_RETURN;

	for (auto* axis : m_axesList)
		axis->setLabelsTextColumn(column);
}

void AxisDock::labelsPrefixChanged() {
	CONDITIONAL_LOCK_RETURN;

	const QString& prefix = ui.leLabelsPrefix->text();
	for (auto* axis : m_axesList)
		axis->setLabelsPrefix(prefix);
}

void AxisDock::labelsSuffixChanged() {
	CONDITIONAL_LOCK_RETURN;

	const QString& suffix = ui.leLabelsSuffix->text();
	for (auto* axis : m_axesList)
		axis->setLabelsSuffix(suffix);
}

void AxisDock::labelsFontChanged(const QFont& font) {
	CONDITIONAL_LOCK_RETURN;

	QFont labelsFont = font;
	labelsFont.setPointSizeF(Worksheet::convertToSceneUnits(font.pointSizeF(), Worksheet::Unit::Point));
	for (auto* axis : m_axesList)
		axis->setLabelsFont(labelsFont);
}

void AxisDock::labelsFontColorChanged(const QColor& color) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* axis : m_axesList)
		axis->setLabelsColor(color);

	updateAxisColor();
}

void AxisDock::labelsBackgroundTypeChanged(int index) {
	auto type = Axis::LabelsBackgroundType(index);

	bool transparent = (type == Axis::LabelsBackgroundType::Transparent);
	ui.lLabelsBackgroundColor->setVisible(!transparent);
	ui.kcbLabelsBackgroundColor->setVisible(!transparent);

	CONDITIONAL_LOCK_RETURN;

	for (auto* axis : m_axesList)
		axis->setLabelsBackgroundType(type);
}

void AxisDock::labelsBackgroundColorChanged(const QColor& color) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* axis : m_axesList)
		axis->setLabelsBackgroundColor(color);
}

void AxisDock::labelsOpacityChanged(int value) {
	CONDITIONAL_LOCK_RETURN;

	qreal opacity{value / 100.};
	for (auto* axis : m_axesList)
		axis->setLabelsOpacity(opacity);
}

//*************************************************************
//************ SLOTs for changes triggered in Axis ************
//*************************************************************
void AxisDock::axisOrientationChanged(Axis::Orientation orientation) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbOrientation->setCurrentIndex(static_cast<int>(orientation));
}

void AxisDock::axisPositionChanged(Axis::Position position) {
	CONDITIONAL_LOCK_RETURN;

	// map from the enum Qt::Orientation to the index in the combo box
	int index{static_cast<int>(position)};
	switch (index) {
	case static_cast<int>(Axis::Position::Top):
	case static_cast<int>(Axis::Position::Left):
		ui.cbPosition->setCurrentIndex(Top_Left);
		break;
	case static_cast<int>(Axis::Position::Bottom):
	case static_cast<int>(Axis::Position::Right):
		ui.cbPosition->setCurrentIndex(Bottom_Right);
		break;
	case static_cast<int>(Axis::Position::Centered):
		ui.cbPosition->setCurrentIndex(Center);
		break;
	case static_cast<int>(Axis::Position::Logical):
		ui.cbPosition->setCurrentIndex(Logical);
	}
}

void AxisDock::axisPositionChanged(double value) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbPosition->setValue(Worksheet::convertFromSceneUnits(value, m_worksheetUnit));
}

void AxisDock::axisLogicalPositionChanged(double value) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbPositionLogical->setValue(value);
}

void AxisDock::axisScaleChanged(RangeT::Scale scale) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbScale->setCurrentIndex(static_cast<int>(scale));
}

void AxisDock::updateScale() {
	if (m_axis->rangeScale()) {
		ui.cbScale->setEnabled(false);
		ui.cbScale->setToolTip(i18n("Scale is in sync with the plot scale"));
	} else {
		ui.cbScale->setEnabled(true);
		ui.cbScale->setToolTip(i18n("Scale is async with the plot"));
	}
}

void AxisDock::axisRangeScaleChanged(bool rangeScale) {
	updateScale();

	CONDITIONAL_LOCK_RETURN;
	ui.cbRangeScale->setChecked(rangeScale);
}

void AxisDock::axisRangeTypeChanged(Axis::RangeType type) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbRangeType->setCurrentIndex(static_cast<int>(type));
}

void AxisDock::axisStartChanged(double value) {
	CONDITIONAL_LOCK_RETURN;

	ui.sbStart->setValue(value);
	ui.dateTimeEditStart->setMSecsSinceEpochUTC(value);

	// determine stepsize and number of decimals
	const double range{m_axis->range().length()};
	const int decimals{nsl_math_rounded_decimals(range) + 1};
	DEBUG("range = " << range << ", decimals = " << decimals)
	ui.sbMajorTicksSpacingNumeric->setDecimals(decimals);
	ui.sbMajorTicksSpacingNumeric->setSingleStep(gsl_pow_int(10., -decimals));
	ui.sbMajorTicksSpacingNumeric->setMaximum(range);
}

void AxisDock::axisEndChanged(double value) {
	CONDITIONAL_LOCK_RETURN;

	ui.sbEnd->setValue(value);
	ui.dateTimeEditEnd->setMSecsSinceEpochUTC(value);

	// determine stepsize and number of decimals
	const double range{m_axis->range().length()};
	const int decimals{nsl_math_rounded_decimals(range) + 1};
	DEBUG("range = " << range << ", decimals = " << decimals)
	ui.sbMajorTicksSpacingNumeric->setDecimals(decimals);
	ui.sbMajorTicksSpacingNumeric->setSingleStep(gsl_pow_int(10., -decimals));
	ui.sbMajorTicksSpacingNumeric->setMaximum(range);
}

void AxisDock::axisZeroOffsetChanged(qreal value) {
	DEBUG(Q_FUNC_INFO)
	CONDITIONAL_LOCK_RETURN;
	ui.sbZeroOffset->setValue(value);
}
void AxisDock::axisScalingFactorChanged(qreal value) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbScalingFactor->setValue(value);
}
void AxisDock::axisShowScaleOffsetChanged(bool b) {
	CONDITIONAL_LOCK_RETURN;
	ui.chkShowScaleOffset->setChecked(b);
}

// line
void AxisDock::axisArrowTypeChanged(Axis::ArrowType type) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbArrowType->setCurrentIndex(static_cast<int>(type));
}

void AxisDock::axisArrowPositionChanged(Axis::ArrowPosition position) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbArrowPosition->setCurrentIndex(static_cast<int>(position));
}

void AxisDock::axisArrowSizeChanged(qreal size) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbArrowSize->setValue((int)Worksheet::convertFromSceneUnits(size, Worksheet::Unit::Point));
}

// major ticks
void AxisDock::axisMajorTicksDirectionChanged(Axis::TicksDirection direction) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbMajorTicksDirection->setCurrentIndex(direction);
}
void AxisDock::axisMajorTicksTypeChanged(Axis::TicksType type) {
	CONDITIONAL_LOCK_RETURN;
	const int index = ui.cbMajorTicksType->findData((int)type);
	ui.cbMajorTicksType->setCurrentIndex(index);
}
void AxisDock::axisMajorTicksAutoNumberChanged(bool automatic) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbMajorTicksAutoNumber->setChecked(automatic);
}
void AxisDock::axisMajorTicksNumberChanged(int number) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbMajorTicksNumber->setValue(number);
}
void AxisDock::axisMajorTicksSpacingChanged(qreal increment) {
	CONDITIONAL_LOCK_RETURN;
	if (m_axis->isNumeric())
		ui.sbMajorTicksSpacingNumeric->setValue(increment);
	else
		dtsbMajorTicksIncrement->setValue(increment);
}
void AxisDock::axisMajorTicksStartTypeChanged(Axis::TicksStartType type) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbMajorTicksStartType->setCurrentIndex(static_cast<int>(type));
}
void AxisDock::axisMajorTicksStartOffsetChanged(qreal value) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbMajorTickStartOffset->setValue(value);
	dtsbMajorTicksDateTimeStartOffset->setValue(value);
}
void AxisDock::axisMajorTicksStartValueChanged(qreal value) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbMajorTickStartValue->setValue(value);
	ui.sbMajorTickStartDateTime->setMSecsSinceEpochUTC(value);
}
void AxisDock::axisMajorTicksColumnChanged(const AbstractColumn* column) {
	CONDITIONAL_LOCK_RETURN;
	cbMajorTicksColumn->setAspect(column, m_axis->majorTicksColumnPath());
}
void AxisDock::axisMajorTicksLengthChanged(qreal length) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbMajorTicksLength->setValue(std::round(Worksheet::convertFromSceneUnits(length, Worksheet::Unit::Point)));
}

// minor ticks
void AxisDock::axisMinorTicksDirectionChanged(Axis::TicksDirection direction) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbMinorTicksDirection->setCurrentIndex(direction);
}
void AxisDock::axisMinorTicksTypeChanged(Axis::TicksType type) {
	CONDITIONAL_LOCK_RETURN;
	const int index = ui.cbMinorTicksType->findData((int)type);
	ui.cbMinorTicksType->setCurrentIndex(index);
}
void AxisDock::axisMinorTicksAutoNumberChanged(bool automatic) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbMinorTicksAutoNumber->setChecked(automatic);
}
void AxisDock::axisMinorTicksNumberChanged(int number) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbMinorTicksNumber->setValue(number);
}
void AxisDock::axisMinorTicksSpacingChanged(qreal increment) {
	CONDITIONAL_LOCK_RETURN;
	if (m_axis->isNumeric())
		ui.sbMinorTicksSpacingNumeric->setValue(increment);
	else
		dtsbMinorTicksIncrement->setValue(increment);
}
void AxisDock::axisMinorTicksColumnChanged(const AbstractColumn* column) {
	CONDITIONAL_LOCK_RETURN;
	cbMinorTicksColumn->setAspect(column, m_axis->minorTicksColumnPath());
}
void AxisDock::axisMinorTicksLengthChanged(qreal length) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbMinorTicksLength->setValue(std::round(Worksheet::convertFromSceneUnits(length, Worksheet::Unit::Point)));
}

// labels
void AxisDock::axisLabelsFormatChanged(Axis::LabelsFormat format) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbLabelsFormat->setCurrentIndex(Axis::labelsFormatToIndex(format));
}
void AxisDock::axisLabelsFormatAutoChanged(bool automatic) {
	CONDITIONAL_LOCK_RETURN;
	ui.chkLabelsFormatAuto->setChecked(automatic);
}
void AxisDock::axisLabelsAutoPrecisionChanged(bool on) {
	CONDITIONAL_LOCK_RETURN;
	ui.chkLabelsAutoPrecision->setChecked(on);
}
void AxisDock::axisLabelsPrecisionChanged(int precision) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbLabelsPrecision->setValue(precision);
}
void AxisDock::axisLabelsDateTimeFormatChanged(const QString& format) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbLabelsDateTimeFormat->setCurrentText(format);
}
void AxisDock::axisLabelsPositionChanged(Axis::LabelsPosition position) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbLabelsPosition->setCurrentIndex(static_cast<int>(position));
}
void AxisDock::axisLabelsOffsetChanged(double offset) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbLabelsOffset->setValue(Worksheet::convertFromSceneUnits(offset, Worksheet::Unit::Point));
}
void AxisDock::axisLabelsRotationAngleChanged(qreal rotation) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbLabelsRotation->setValue(rotation);
}
void AxisDock::axisLabelsTextTypeChanged(Axis::LabelsTextType type) {
	CONDITIONAL_LOCK_RETURN;
	const int index = ui.cbLabelsTextType->findData((int)type);
	ui.cbLabelsTextType->setCurrentIndex(index);
}
void AxisDock::axisLabelsTextColumnChanged(const AbstractColumn* column) {
	CONDITIONAL_LOCK_RETURN;
	cbLabelsTextColumn->setAspect(column, m_axis->labelsTextColumnPath());
}
void AxisDock::axisLabelsFontChanged(const QFont& font) {
	CONDITIONAL_LOCK_RETURN;
	// we need to set the font size in points for KFontRequester
	QFont newFont(font);
	newFont.setPointSizeF(std::round(Worksheet::convertFromSceneUnits(font.pointSizeF(), Worksheet::Unit::Point)));
	ui.kfrLabelsFont->setFont(newFont);
}
void AxisDock::axisLabelsFontColorChanged(const QColor& color) {
	CONDITIONAL_LOCK_RETURN;
	updateAxisColor();
	ui.kcbLabelsFontColor->setColor(color);
}
void AxisDock::axisLabelsBackgroundTypeChanged(Axis::LabelsBackgroundType type) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbLabelsBackgroundType->setCurrentIndex(static_cast<int>(type));
}
void AxisDock::axisLabelsBackgroundColorChanged(const QColor& color) {
	CONDITIONAL_LOCK_RETURN;
	ui.kcbLabelsBackgroundColor->setColor(color);
}
void AxisDock::axisLabelsPrefixChanged(const QString& prefix) {
	CONDITIONAL_LOCK_RETURN;
	ui.leLabelsPrefix->setText(prefix);
}
void AxisDock::axisLabelsSuffixChanged(const QString& suffix) {
	CONDITIONAL_LOCK_RETURN;
	ui.leLabelsSuffix->setText(suffix);
}
void AxisDock::axisLabelsOpacityChanged(qreal opacity) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbLabelsOpacity->setValue(std::round(opacity * 100.));
}

void AxisDock::updateMajorTicksStartType(bool visible) {
	const bool absoluteValue = (ui.cbMajorTicksStartType->currentIndex() == 0);
	const bool numeric = m_axis->isNumeric();

	ui.lMajorTickStartOffset->setVisible(visible && !absoluteValue); // for datetime and numeric
	ui.sbMajorTickStartOffset->setVisible(visible && !absoluteValue && numeric);
	dtsbMajorTicksDateTimeStartOffset->setVisible(visible && !absoluteValue && !numeric);
	ui.tbFirstTickData->setVisible(visible && !absoluteValue);
	ui.tbFirstTickAuto->setVisible(visible && !absoluteValue);

	ui.sbMajorTickStartValue->setVisible(visible && absoluteValue && numeric);
	ui.lMajorTickStartValue->setVisible(visible && absoluteValue && numeric);
	ui.lMajorTickStartDateTime->setVisible(visible && absoluteValue && !numeric);
	ui.sbMajorTickStartDateTime->setVisible(visible && absoluteValue && !numeric);
}

//*************************************************************
//************************* Settings **************************
//*************************************************************
void AxisDock::load() {
	// General
	ui.chkVisible->setChecked(m_axis->isVisible());

	updateAxisColor();

	Axis::Orientation orientation = m_axis->orientation();
	ui.cbOrientation->setCurrentIndex(static_cast<int>(orientation));

	const auto* plot = static_cast<const CartesianPlot*>(m_axis->parentAspect());
	const auto* cSystem = plot->coordinateSystem(m_axis->coordinateSystemIndex());
	const int xIndex{cSystem->index(Dimension::X)}, yIndex{cSystem->index(Dimension::Y)};

	Range<double> logicalRange(0, 0);
	if (orientation == Axis::Orientation::Horizontal)
		logicalRange = plot->range(Dimension::Y, yIndex);
	else
		logicalRange = plot->range(Dimension::X, xIndex);
	updatePositionText(orientation);

	int index{static_cast<int>(m_axis->position())};
	bool logical = false;
	switch (index) {
	case static_cast<int>(Axis::Position::Top):
	case static_cast<int>(Axis::Position::Left):
		ui.cbPosition->setCurrentIndex(Top_Left);
		break;
	case static_cast<int>(Axis::Position::Bottom):
	case static_cast<int>(Axis::Position::Right):
		ui.cbPosition->setCurrentIndex(Bottom_Right);
		break;
	case static_cast<int>(Axis::Position::Centered):
		ui.cbPosition->setCurrentIndex(Center);
		break;
	case static_cast<int>(Axis::Position::Logical):
		ui.cbPosition->setCurrentIndex(Logical);
		logical = true;
	}

	ui.sbPositionLogical->setVisible(logical);
	ui.sbPosition->setVisible(!logical);

	ui.sbPosition->setValue(Worksheet::convertFromSceneUnits(m_axis->offset(), m_worksheetUnit));

	spinBoxCalculateMinMax(ui.sbPositionLogical, logicalRange, m_axis->logicalPosition());
	ui.sbPositionLogical->setValue(m_axis->logicalPosition());

	updateScale();
	const bool rangeScale = m_axis->rangeScale();
	ui.cbRangeScale->setChecked(rangeScale);
	ui.cbScale->setCurrentIndex(static_cast<int>(m_axis->scale()));
	// Changing the scale in the axis for the ticks is deprecated
	// So show the options only if rangeScale is not turned on.
	// So the user is once able to enable it and then the dialog
	// disappears
	ui.cbRangeScale->setVisible(!rangeScale);
	ui.cbScale->setVisible(!rangeScale);
	ui.lScale->setVisible(!rangeScale);

	index = static_cast<int>(m_axis->rangeType());
	ui.cbRangeType->setCurrentIndex(index);
	rangeTypeChanged(index);
	ui.sbStart->setValue(m_axis->range().start());
	ui.sbEnd->setValue(m_axis->range().end());

	// depending on the range format of the axis (numeric vs. datetime), show/hide the corresponding widgets
	const bool numeric = m_axis->isNumeric();

	updateLabelsPosition(m_axis->labelsPosition());

	// ranges
	ui.lStart->setVisible(numeric);
	ui.lEnd->setVisible(numeric);
	ui.sbStart->setVisible(numeric);
	ui.sbEnd->setVisible(numeric);
	ui.lStartDateTime->setVisible(!numeric);
	ui.dateTimeEditStart->setVisible(!numeric);
	ui.lEndDateTime->setVisible(!numeric);
	ui.dateTimeEditEnd->setVisible(!numeric);

	// tick labels format
	ui.lLabelsFormat->setVisible(numeric);
	ui.chkLabelsFormatAuto->setVisible(numeric);
	ui.cbLabelsFormat->setVisible(numeric);
	ui.chkLabelsAutoPrecision->setVisible(numeric);
	ui.lLabelsPrecision->setVisible(numeric);
	ui.sbLabelsPrecision->setVisible(numeric);
	ui.lLabelsDateTimeFormat->setVisible(!numeric);
	ui.cbLabelsDateTimeFormat->setVisible(!numeric);

	if (!numeric) {
		if (m_axis->orientation() == Axis::Orientation::Horizontal) {
			ui.dateTimeEditStart->setDisplayFormat(plot->rangeDateTimeFormat(Dimension::X, xIndex));
			ui.dateTimeEditEnd->setDisplayFormat(plot->rangeDateTimeFormat(Dimension::X, xIndex));
		} else {
			// TODO
			ui.dateTimeEditStart->setDisplayFormat(plot->rangeDateTimeFormat(Dimension::Y));
			ui.dateTimeEditEnd->setDisplayFormat(plot->rangeDateTimeFormat(Dimension::Y));
		}
		ui.dateTimeEditStart->setMSecsSinceEpochUTC(m_axis->range().start());
		ui.dateTimeEditEnd->setMSecsSinceEpochUTC(m_axis->range().end());
	}

	ui.sbZeroOffset->setValue(m_axis->zeroOffset());
	ui.sbScalingFactor->setValue(m_axis->scalingFactor());
	ui.chkShowScaleOffset->setChecked(m_axis->showScaleOffset());

	// Line
	ui.cbArrowType->setCurrentIndex((int)m_axis->arrowType());
	ui.cbArrowPosition->setCurrentIndex((int)m_axis->arrowPosition());
	ui.sbArrowSize->setValue((int)Worksheet::convertFromSceneUnits(m_axis->arrowSize(), Worksheet::Unit::Point));
	updateArrowLineColor(m_axis->line()->color());

	// Major ticks
	ui.cbMajorTicksDirection->setCurrentIndex((int)m_axis->majorTicksDirection());
	ui.cbMajorTicksType->setCurrentIndex(ui.cbMajorTicksType->findData((int)m_axis->majorTicksType()));
	ui.cbMajorTicksAutoNumber->setChecked(m_axis->majorTicksAutoNumber());

	ui.sbMajorTicksNumber->setValue(m_axis->majorTicksNumber());
	auto value{m_axis->majorTicksSpacing()};
	if (numeric) {
		ui.sbMajorTicksSpacingNumeric->setDecimals(nsl_math_decimal_places(value) + 1);
		ui.sbMajorTicksSpacingNumeric->setValue(value);
		ui.sbMajorTicksSpacingNumeric->setSingleStep(value / 10.);
	} else
		dtsbMajorTicksIncrement->setValue(value);
	ui.cbMajorTicksStartType->setCurrentIndex(static_cast<int>(m_axis->majorTicksStartType()));
	ui.sbMajorTickStartOffset->setValue(m_axis->majorTickStartOffset());
	dtsbMajorTicksDateTimeStartOffset->setValue(m_axis->majorTickStartOffset());
	ui.sbMajorTickStartValue->setValue(m_axis->majorTickStartValue());
	ui.sbMajorTickStartDateTime->setMSecsSinceEpochUTC(m_axis->majorTickStartValue());
	ui.sbMajorTicksLength->setValue(std::round(Worksheet::convertFromSceneUnits(m_axis->majorTicksLength(), Worksheet::Unit::Point)));

	// Minor ticks
	ui.cbMinorTicksDirection->setCurrentIndex((int)m_axis->minorTicksDirection());
	ui.cbMinorTicksType->setCurrentIndex((int)m_axis->minorTicksType());
	ui.cbMinorTicksAutoNumber->setChecked(m_axis->minorTicksAutoNumber());
	ui.sbMinorTicksNumber->setEnabled(!m_axis->minorTicksAutoNumber());
	ui.sbMinorTicksNumber->setValue(m_axis->minorTicksNumber());
	ui.sbMinorTicksLength->setValue(std::round(Worksheet::convertFromSceneUnits(m_axis->minorTicksLength(), Worksheet::Unit::Point)));

	// Extra ticks
	// TODO

	// Tick label
	ui.cbLabelsPosition->setCurrentIndex((int)m_axis->labelsPosition());
	ui.sbLabelsOffset->setValue(Worksheet::convertFromSceneUnits(m_axis->labelsOffset(), Worksheet::Unit::Point));
	ui.sbLabelsRotation->setValue(m_axis->labelsRotationAngle());
	const int idx = ui.cbLabelsTextType->findData((int)m_axis->labelsTextType());
	ui.cbLabelsTextType->setCurrentIndex(idx);
	ui.cbLabelsFormat->setCurrentIndex(Axis::labelsFormatToIndex(m_axis->labelsFormat()));
	ui.cbLabelsFormat->setEnabled(!m_axis->labelsFormatAuto());
	ui.chkLabelsFormatAuto->setChecked(m_axis->labelsFormatAuto());
	ui.chkLabelsAutoPrecision->setChecked((int)m_axis->labelsAutoPrecision());
	ui.sbLabelsPrecision->setValue((int)m_axis->labelsPrecision());
	ui.cbLabelsDateTimeFormat->setCurrentText(m_axis->labelsDateTimeFormat());

	// we need to set the font size in points for KFontRequester
	QFont font = m_axis->labelsFont();
	font.setPointSizeF(std::round(Worksheet::convertFromSceneUnits(font.pointSizeF(), Worksheet::Unit::Point)));
	ui.kfrLabelsFont->setFont(font);
	ui.kcbLabelsFontColor->setColor(m_axis->labelsColor());
	ui.cbLabelsBackgroundType->setCurrentIndex((int)m_axis->labelsBackgroundType());
	ui.kcbLabelsBackgroundColor->setColor(m_axis->labelsBackgroundColor());
	ui.leLabelsPrefix->setText(m_axis->labelsPrefix());
	ui.leLabelsSuffix->setText(m_axis->labelsSuffix());
	ui.sbLabelsOpacity->setValue(std::round(m_axis->labelsOpacity() * 100.));

	majorTicksDirectionChanged(ui.cbMajorTicksDirection->currentIndex());
	majorTicksTypeChanged(ui.cbMajorTicksType->currentIndex());
	minorTicksTypeChanged(ui.cbMinorTicksType->currentIndex());
	labelsTextTypeChanged(ui.cbLabelsTextType->currentIndex());
	labelsTextColumnChanged(cbLabelsTextColumn->currentModelIndex());
}

void AxisDock::setAxisColor() {
	CONDITIONAL_LOCK_RETURN;
	updateAxisColor();
}

void AxisDock::updateAxisColor() {
	// Set color of the global
	// - Line widget color
	// - Title color
	// - Major tick color
	// - Minor tick color
	// - Tick label color
	QColor color = m_axis->line()->color();
	if (m_axis->title()->fontColor() == color && m_axis->majorTicksLine()->color() == color && m_axis->minorTicksLine()->color() == color
		&& m_axis->labelsColor() == color) {
		// All have same color
		ui.kcbAxisColor->setColor(color);
	} else
		ui.kcbAxisColor->setColor(QColor(0, 0, 0, 0));
}

void AxisDock::loadConfigFromTemplate(KConfig& config) {
	auto name = TemplateHandler::templateName(config);
	int size = m_axesList.size();
	if (size > 1)
		m_axis->beginMacro(i18n("%1 axes: template \"%2\" loaded", size, name));
	else
		m_axis->beginMacro(i18n("%1: template \"%2\" loaded", m_axis->name(), name));

	this->loadConfig(config);

	m_axis->endMacro();
}

void AxisDock::loadConfig(KConfig& config) {
	auto group = config.group(QStringLiteral("Axis"));

	// General
	ui.cbOrientation->setCurrentIndex(group.readEntry(QStringLiteral("Orientation"), (int)m_axis->orientation()));

	int index = group.readEntry(QStringLiteral("Position"), (int)m_axis->position());
	if (index > 1)
		ui.cbPosition->setCurrentIndex(index - 2);
	else
		ui.cbPosition->setCurrentIndex(index);

	ui.sbPositionLogical->setValue(group.readEntry(QStringLiteral("LogicalPosition"), m_axis->logicalPosition()));
	ui.sbPosition->setValue(Worksheet::convertFromSceneUnits(group.readEntry(QStringLiteral("PositionOffset"), m_axis->offset()), m_worksheetUnit));
	ui.cbScale->setCurrentIndex(group.readEntry(QStringLiteral("Scale"), static_cast<int>(m_axis->scale())));
	index = static_cast<int>(m_axis->rangeType());
	ui.cbRangeType->setCurrentIndex(group.readEntry(QStringLiteral("RangeType"), index));
	rangeTypeChanged(index);
	ui.sbStart->setValue(group.readEntry(QStringLiteral("Start"), m_axis->range().start()));
	ui.sbEnd->setValue(group.readEntry(QStringLiteral("End"), m_axis->range().end()));
	ui.sbZeroOffset->setValue(group.readEntry(QStringLiteral("ZeroOffset"), m_axis->zeroOffset()));
	ui.sbScalingFactor->setValue(group.readEntry(QStringLiteral("ScalingFactor"), m_axis->scalingFactor()));
	ui.chkShowScaleOffset->setChecked(group.readEntry(QStringLiteral("ShowScaleOffset"), static_cast<int>(m_axis->showScaleOffset())));

	// Title
	auto axisTitleGroup = config.group(QStringLiteral("AxisTitle"));
	labelWidget->loadConfig(axisTitleGroup);

	// Line
	lineWidget->loadConfig(group);
	ui.cbArrowType->setCurrentIndex(group.readEntry(QStringLiteral("ArrowType"), (int)m_axis->arrowType()));
	ui.cbArrowPosition->setCurrentIndex(group.readEntry(QStringLiteral("ArrowPosition"), (int)m_axis->arrowPosition()));
	ui.sbArrowSize->setValue(Worksheet::convertFromSceneUnits(group.readEntry(QStringLiteral("ArrowSize"), m_axis->arrowSize()), Worksheet::Unit::Point));
	updateArrowLineColor(m_axis->line()->color());

	// Major ticks
	ui.cbMajorTicksDirection->setCurrentIndex(group.readEntry(QStringLiteral("MajorTicksDirection"), (int)m_axis->majorTicksDirection()));
	ui.cbMajorTicksType->setCurrentIndex(ui.cbMajorTicksType->findData(group.readEntry(QStringLiteral("MajorTicksType"), (int)m_axis->majorTicksType())));
	ui.sbMajorTicksNumber->setValue(group.readEntry(QStringLiteral("MajorTicksNumber"), m_axis->majorTicksNumber()));
	auto value{group.readEntry(QStringLiteral("MajorTicksIncrement"), m_axis->majorTicksSpacing())};
	bool numeric = m_axis->isNumeric();
	if (numeric) {
		ui.sbMajorTicksSpacingNumeric->setDecimals(nsl_math_decimal_places(value) + 1);
		ui.sbMajorTicksSpacingNumeric->setValue(value);
		ui.sbMajorTicksSpacingNumeric->setSingleStep(value / 10.);
	} else
		dtsbMajorTicksIncrement->setValue(value);
	ui.cbMajorTicksStartType->setCurrentIndex(group.readEntry(QStringLiteral("MajorTicksStartType"), (int)m_axis->majorTicksStartType()));
	const auto majorTickStartOffset = group.readEntry(QStringLiteral("MajorTickStartOffset"), m_axis->majorTickStartOffset());
	ui.sbMajorTickStartOffset->setValue(majorTickStartOffset);
	dtsbMajorTicksDateTimeStartOffset->setValue(majorTickStartOffset);
	const auto majorTickStartValue = group.readEntry(QStringLiteral("MajorTickStartValue"), m_axis->majorTickStartValue());
	ui.sbMajorTickStartValue->setValue(majorTickStartValue);
	ui.sbMajorTickStartDateTime->setMSecsSinceEpochUTC(majorTickStartValue);
	ui.sbMajorTicksLength->setValue(std::round(
		Worksheet::convertFromSceneUnits(group.readEntry(QStringLiteral("MajorTicksLength"), m_axis->majorTicksLength()), Worksheet::Unit::Point)));
	majorTicksLineWidget->loadConfig(group);

	// Minor ticks
	ui.cbMinorTicksDirection->setCurrentIndex(group.readEntry(QStringLiteral("MinorTicksDirection"), (int)m_axis->minorTicksDirection()));
	ui.cbMinorTicksType->setCurrentIndex(ui.cbMinorTicksType->findData(group.readEntry(QStringLiteral("MajorTicksType"), (int)m_axis->minorTicksType())));
	ui.sbMinorTicksNumber->setValue(group.readEntry(QStringLiteral("MinorTicksNumber"), m_axis->minorTicksNumber()));
	value = group.readEntry(QStringLiteral("MinorTicksIncrement"), m_axis->minorTicksSpacing());
	if (numeric)
		ui.sbMinorTicksSpacingNumeric->setValue(value);
	else
		dtsbMinorTicksIncrement->setValue(value);
	ui.sbMinorTicksLength->setValue(std::round(Worksheet::convertFromSceneUnits(group.readEntry("MinorTicksLength", m_axis->minorTicksLength()), Worksheet::Unit::Point)));
	minorTicksLineWidget->loadConfig(group);

	// Extra ticks
	// TODO

	// Tick label
	ui.cbLabelsFormat->setCurrentIndex(
		Axis::labelsFormatToIndex((Axis::LabelsFormat)(group.readEntry(QStringLiteral("LabelsFormat"), Axis::labelsFormatToIndex(m_axis->labelsFormat())))));
	ui.chkLabelsAutoPrecision->setChecked(group.readEntry(QStringLiteral("LabelsAutoPrecision"), (int)m_axis->labelsAutoPrecision()));
	ui.sbLabelsPrecision->setValue(group.readEntry(QStringLiteral("LabelsPrecision"), (int)m_axis->labelsPrecision()));
	ui.cbLabelsDateTimeFormat->setCurrentText(group.readEntry(QStringLiteral("LabelsDateTimeFormat"), QStringLiteral("yyyy-MM-dd hh:mm:ss")));
	ui.cbLabelsPosition->setCurrentIndex(group.readEntry(QStringLiteral("LabelsPosition"), (int)m_axis->labelsPosition()));
	ui.sbLabelsOffset->setValue(
		Worksheet::convertFromSceneUnits(group.readEntry(QStringLiteral("LabelsOffset"), m_axis->labelsOffset()), Worksheet::Unit::Point));
	ui.sbLabelsRotation->setValue(group.readEntry(QStringLiteral("LabelsRotation"), m_axis->labelsRotationAngle()));
	ui.cbLabelsTextType->setCurrentIndex(group.readEntry(QStringLiteral("LabelsTextType"), (int)m_axis->labelsTextType()));

	// we need to set the font size in points for KFontRequester
	QFont font = m_axis->labelsFont();
	font.setPointSizeF(std::round(Worksheet::convertFromSceneUnits(font.pointSizeF(), Worksheet::Unit::Point)));
	ui.kfrLabelsFont->setFont(group.readEntry(QStringLiteral("LabelsFont"), font));

	ui.kcbLabelsFontColor->setColor(group.readEntry(QStringLiteral("LabelsFontColor"), m_axis->labelsColor()));
	ui.cbLabelsBackgroundType->setCurrentIndex(group.readEntry(QStringLiteral("LabelsBackgroundType"), (int)m_axis->labelsBackgroundType()));
	ui.kcbLabelsBackgroundColor->setColor(group.readEntry(QStringLiteral("LabelsBackgroundColor"), m_axis->labelsBackgroundColor()));
	ui.leLabelsPrefix->setText(group.readEntry(QStringLiteral("LabelsPrefix"), m_axis->labelsPrefix()));
	ui.leLabelsSuffix->setText(group.readEntry(QStringLiteral("LabelsSuffix"), m_axis->labelsSuffix()));
	ui.sbLabelsOpacity->setValue(std::round(group.readEntry(QStringLiteral("LabelsOpacity"), m_axis->labelsOpacity()) * 100.));

	// Grid
	majorGridLineWidget->loadConfig(group);
	minorGridLineWidget->loadConfig(group);

	CONDITIONAL_LOCK_RETURN;
	this->majorTicksTypeChanged(ui.cbMajorTicksType->currentIndex());
	this->minorTicksTypeChanged(ui.cbMinorTicksType->currentIndex());
}

void AxisDock::saveConfigAsTemplate(KConfig& config) {
	auto group = config.group(QStringLiteral("Axis"));

	// General
	auto orientation = (WorksheetElement::Orientation)ui.cbOrientation->currentIndex();
	group.writeEntry(QStringLiteral("Orientation"), (int)orientation);

	if (ui.cbPosition->currentIndex() == 2) {
		group.writeEntry(QStringLiteral("Position"), static_cast<int>(Axis::Position::Centered));
	} else if (ui.cbPosition->currentIndex() == 3) {
		group.writeEntry(QStringLiteral("Position"), static_cast<int>(Axis::Position::Centered));
	} else {
		if (ui.cbOrientation->currentIndex() == static_cast<int>(Axis::Orientation::Horizontal))
			group.writeEntry(QStringLiteral("Position"), ui.cbPosition->currentIndex());
		else
			group.writeEntry(QStringLiteral("Position"), ui.cbPosition->currentIndex() + 2);
	}

	group.writeEntry(QStringLiteral("LogicalPosition"), ui.sbPositionLogical->value());
	group.writeEntry(QStringLiteral("PositionOffset"), Worksheet::convertToSceneUnits(ui.sbPosition->value(), m_worksheetUnit));
	group.writeEntry(QStringLiteral("Scale"), ui.cbScale->currentIndex());
	group.writeEntry(QStringLiteral("RangeType"), ui.cbRangeType->currentIndex());
	group.writeEntry(QStringLiteral("Start"), ui.sbStart->value());
	group.writeEntry(QStringLiteral("End"), ui.sbEnd->value());
	group.writeEntry(QStringLiteral("ZeroOffset"), ui.sbZeroOffset->value());
	group.writeEntry(QStringLiteral("ScalingFactor"), ui.sbScalingFactor->value());
	group.writeEntry(QStringLiteral("ShowScaleOffset"), ui.chkShowScaleOffset->isChecked());

	// BOOKMARK(axis title): Title
	auto axisTitleGroup = config.group(QStringLiteral("AxisTitle"));
	labelWidget->saveConfig(axisTitleGroup);
	// addtionally save rotation for x and y seperately to allow independent values
	if (orientation == Axis::Orientation::Horizontal)
		axisTitleGroup.writeEntry(QStringLiteral("RotationX"), labelWidget->ui.sbRotation->value());
	else {
		axisTitleGroup.writeEntry(QStringLiteral("RotationY"), labelWidget->ui.sbRotation->value());
	}
	// reset rotation used by the other axis (only used when other axis not saved)
	// see Axis.cpp:BOOKMARK(axis title)
	axisTitleGroup.writeEntry(QStringLiteral("Rotation"), 0);

	// Line
	lineWidget->saveConfig(group);

	// Major ticks
	group.writeEntry(QStringLiteral("MajorTicksDirection"), ui.cbMajorTicksDirection->currentIndex());
	group.writeEntry(QStringLiteral("MajorTicksType"), ui.cbMajorTicksType->itemData(ui.cbMajorTicksType->currentIndex()));
	group.writeEntry(QStringLiteral("MajorTicksNumber"), ui.sbMajorTicksNumber->value());
	const bool numeric = m_axis->isNumeric();
	if (numeric)
		group.writeEntry(QStringLiteral("MajorTicksIncrement"), QString::number(ui.sbMajorTicksSpacingNumeric->value()));
	else
		group.writeEntry(QStringLiteral("MajorTicksIncrement"), QString::number(dtsbMajorTicksIncrement->value()));
	group.writeEntry(QStringLiteral("MajorTicksStartType"), ui.cbMajorTicksStartType->currentIndex());
	if (numeric) {
		group.writeEntry(QStringLiteral("MajorTickStartOffset"), ui.sbMajorTickStartOffset->value());
		group.writeEntry(QStringLiteral("MajorTickStartValue"), ui.sbMajorTickStartValue->value());
	} else {
		group.writeEntry(QStringLiteral("MajorTickStartOffset"), dtsbMajorTicksDateTimeStartOffset->value());
		group.writeEntry(QStringLiteral("MajorTickStartValue"), ui.sbMajorTickStartDateTime->dateTime().toMSecsSinceEpoch());
	}
	group.writeEntry(QStringLiteral("MajorTicksLength"), Worksheet::convertToSceneUnits(ui.sbMajorTicksLength->value(), Worksheet::Unit::Point));
	majorTicksLineWidget->saveConfig(group);

	// Minor ticks
	group.writeEntry(QStringLiteral("MinorTicksDirection"), ui.cbMinorTicksDirection->currentIndex());
	group.writeEntry(QStringLiteral("MinorTicksType"), ui.cbMinorTicksType->currentIndex());
	group.writeEntry(QStringLiteral("MinorTicksNumber"), ui.sbMinorTicksNumber->value());
	if (numeric)
		group.writeEntry(QStringLiteral("MinorTicksIncrement"), QString::number(ui.sbMinorTicksSpacingNumeric->value()));
	else
		group.writeEntry(QStringLiteral("MinorTicksIncrement"), QString::number(dtsbMinorTicksIncrement->value()));
	group.writeEntry(QStringLiteral("MinorTicksLength"), Worksheet::convertToSceneUnits(ui.sbMinorTicksLength->value(), Worksheet::Unit::Point));
	minorTicksLineWidget->saveConfig(group);

	// Extra ticks
	//  TODO

	// Tick label
	group.writeEntry(QStringLiteral("LabelsFormat"), static_cast<int>(Axis::indexToLabelsFormat(ui.cbLabelsFormat->currentIndex())));
	group.writeEntry(QStringLiteral("LabelsAutoPrecision"), ui.chkLabelsAutoPrecision->isChecked());
	group.writeEntry(QStringLiteral("LabelsPrecision"), ui.sbLabelsPrecision->value());
	group.writeEntry(QStringLiteral("LabelsPosition"), ui.cbLabelsPosition->currentIndex());
	group.writeEntry(QStringLiteral("LabelsOffset"), Worksheet::convertToSceneUnits(ui.sbLabelsOffset->value(), Worksheet::Unit::Point));
	group.writeEntry(QStringLiteral("LabelsRotation"), ui.sbLabelsRotation->value());
	group.writeEntry(QStringLiteral("LabelsFont"), ui.kfrLabelsFont->font());
	group.writeEntry(QStringLiteral("LabelsFontColor"), ui.kcbLabelsFontColor->color());
	group.writeEntry(QStringLiteral("LabelsBackgroundType"), ui.cbLabelsBackgroundType->currentIndex());
	group.writeEntry(QStringLiteral("LabelsBackgroundColor"), ui.kcbLabelsBackgroundColor->color());
	group.writeEntry(QStringLiteral("LabelsPrefix"), ui.leLabelsPrefix->text());
	group.writeEntry(QStringLiteral("LabelsSuffix"), ui.leLabelsSuffix->text());
	group.writeEntry(QStringLiteral("LabelsOpacity"), ui.sbLabelsOpacity->value() / 100.);

	// Grid
	majorGridLineWidget->saveConfig(group);
	minorGridLineWidget->saveConfig(group);

	config.sync();
}
