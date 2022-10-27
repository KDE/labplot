/*
	File                 : AxisDock.cpp
	Project              : LabPlot
	Description          : axes widget class
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2011-2022 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2012-2021 Stefan Gerlach <stefan.gerlach@uni-konstanz.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "AxisDock.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/core/Project.h"
#include "backend/core/column/Column.h"
#include "backend/lib/macros.h"
#include "backend/worksheet/Worksheet.h"
#include "commonfrontend/widgets/DateTimeSpinBox.h"
#include "commonfrontend/widgets/NumberSpinBox.h"
#include "commonfrontend/widgets/TreeViewComboBox.h"
#include "kdefrontend/GuiTools.h"
#include "kdefrontend/TemplateHandler.h"
#include "kdefrontend/widgets/LabelWidget.h"
#include "kdefrontend/widgets/LineWidget.h"

#include <KLineEdit>
#include <KLocalizedString>
#include <KMessageBox>

#include <QDir>
#include <QPainter>
#include <QTimer>

extern "C" {
#include "backend/nsl/nsl_math.h"
#include <gsl/gsl_math.h>
}

namespace {
enum PositionAlignmentComboBoxIndex {
	Top_Left = 0,
	Bottom_Right = 1,
	Center = 2,
	Logical = 3,
};
}

using Dimension = CartesianCoordinateSystem::Dimension;

/*!
 \class AxisDock
 \brief Provides a widget for editing the properties of the axes currently selected in the project explorer.

 \ingroup kdefrontend
*/

AxisDock::AxisDock(QWidget* parent)
	: BaseDock(parent) {
	ui.setupUi(this);
	m_leName = ui.leName;
	m_teComment = ui.teComment;
	m_teComment->setFixedHeight(1.5 * m_leName->height());

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
	gridLayout->addWidget(dtsbMajorTicksIncrement, 5, 3);

	cbMajorTicksColumn = new TreeViewComboBox(ui.tabTicks);
	gridLayout->addWidget(cbMajorTicksColumn, 9, 3);

	cbLabelsTextColumn = new TreeViewComboBox(ui.tabTicks);
	gridLayout->addWidget(cbLabelsTextColumn, 11, 3);

	// minor ticks
	dtsbMinorTicksIncrement = new DateTimeSpinBox(ui.tabTicks);
	gridLayout->addWidget(dtsbMinorTicksIncrement, 24, 3);

	cbMinorTicksColumn = new TreeViewComboBox(ui.tabTicks);
	gridLayout->addWidget(cbMinorTicksColumn, 25, 3);

	// "Grid"-tab
	gridLayout = qobject_cast<QGridLayout*>(ui.tabGrid->layout());
	majorGridLineWidget = new LineWidget(ui.tabLine);
	majorGridLineWidget->setPrefix(QStringLiteral("MajorGrid"));
	gridLayout->addWidget(majorGridLineWidget, 1, 0, 1, 3);

	minorGridLineWidget = new LineWidget(ui.tabLine);
	minorGridLineWidget->setPrefix(QStringLiteral("MinorGrid"));
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

	//**********************************  Slots **********************************************

	//"General"-tab
	connect(ui.leName, &QLineEdit::textChanged, this, &AxisDock::nameChanged);
	connect(ui.teComment, &QTextEdit::textChanged, this, &AxisDock::commentChanged);
	connect(ui.chkVisible, &QCheckBox::clicked, this, &AxisDock::visibilityChanged);

	connect(ui.cbOrientation, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AxisDock::orientationChanged);
	connect(ui.cbPosition, QOverload<int>::of(&QComboBox::currentIndexChanged), this, QOverload<int>::of(&AxisDock::positionChanged));
	connect(ui.sbPosition, QOverload<double>::of(&NumberSpinBox::valueChanged), this, QOverload<double>::of(&AxisDock::positionChanged));
	connect(ui.sbPositionLogical, QOverload<double>::of(&NumberSpinBox::valueChanged), this, QOverload<double>::of(&AxisDock::logicalPositionChanged));
	connect(ui.cbScale, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AxisDock::scaleChanged);

	connect(ui.cbRangeType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AxisDock::rangeTypeChanged);
	connect(ui.leStart, &QLineEdit::textChanged, this, &AxisDock::startChanged);
	connect(ui.leEnd, &QLineEdit::textChanged, this, &AxisDock::endChanged);
	connect(ui.dateTimeEditStart, &QDateTimeEdit::dateTimeChanged, this, &AxisDock::startDateTimeChanged);
	connect(ui.dateTimeEditEnd, &QDateTimeEdit::dateTimeChanged, this, &AxisDock::endDateTimeChanged);
	connect(ui.sbZeroOffset, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &AxisDock::zeroOffsetChanged);
	connect(ui.tbOffsetLeft, &QToolButton::clicked, this, &AxisDock::setLeftOffset);
	connect(ui.tbOffsetCenter, &QToolButton::clicked, this, &AxisDock::setCenterOffset);
	connect(ui.tbOffsetRight, &QToolButton::clicked, this, &AxisDock::setRightOffset);

	connect(ui.sbScalingFactor, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &AxisDock::scalingFactorChanged);
	connect(ui.tbUnityScale, &QToolButton::clicked, this, &AxisDock::setUnityScale);
	connect(ui.tbUnityRange, &QToolButton::clicked, this, &AxisDock::setUnityRange);

	connect(ui.chkShowScaleOffset, &QCheckBox::toggled, this, &AxisDock::showScaleOffsetChanged);

	connect(ui.cbPlotRanges, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AxisDock::plotRangeChanged);

	//"Line"-tab

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
	connect(ui.leMajorTickStartOffset, &KLineEdit::textChanged, this, &AxisDock::majorTicksStartOffsetChanged);
	connect(ui.leMajorTickStartValue, &KLineEdit::textChanged, this, &AxisDock::majorTicksStartValueChanged);
	connect(ui.tbFirstTickData, &QToolButton::clicked, this, &AxisDock::setTickOffsetData);
	connect(ui.tbFirstTickAuto, &QToolButton::clicked, this, &AxisDock::setTickOffsetAuto);
	connect(cbMajorTicksColumn, &TreeViewComboBox::currentModelIndexChanged, this, &AxisDock::majorTicksColumnChanged);
	connect(ui.cbMajorTicksLineStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AxisDock::majorTicksLineStyleChanged);
	connect(ui.kcbMajorTicksColor, &KColorButton::changed, this, &AxisDock::majorTicksColorChanged);
	connect(ui.sbMajorTicksWidth, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &AxisDock::majorTicksWidthChanged);
	connect(ui.sbMajorTicksLength, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &AxisDock::majorTicksLengthChanged);
	connect(ui.sbMajorTicksOpacity, QOverload<int>::of(&QSpinBox::valueChanged), this, &AxisDock::majorTicksOpacityChanged);

	//"Minor ticks"-tab
	connect(ui.cbMinorTicksDirection, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AxisDock::minorTicksDirectionChanged);
	connect(ui.cbMinorTicksType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AxisDock::minorTicksTypeChanged);
	connect(ui.cbMinorTicksAutoNumber, &QCheckBox::stateChanged, this, &AxisDock::minorTicksAutoNumberChanged);
	connect(ui.sbMinorTicksNumber, QOverload<int>::of(&QSpinBox::valueChanged), this, &AxisDock::minorTicksNumberChanged);
	connect(ui.sbMinorTicksSpacingNumeric, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &AxisDock::minorTicksSpacingChanged);
	connect(dtsbMinorTicksIncrement, &DateTimeSpinBox::valueChanged, this, &AxisDock::minorTicksSpacingChanged);
	connect(cbMinorTicksColumn, &TreeViewComboBox::currentModelIndexChanged, this, &AxisDock::minorTicksColumnChanged);
	connect(ui.cbMinorTicksLineStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AxisDock::minorTicksLineStyleChanged);
	connect(ui.kcbMinorTicksColor, &KColorButton::changed, this, &AxisDock::minorTicksColorChanged);
	connect(ui.sbMinorTicksWidth, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &AxisDock::minorTicksWidthChanged);
	connect(ui.sbMinorTicksLength, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &AxisDock::minorTicksLengthChanged);
	connect(ui.sbMinorTicksOpacity, QOverload<int>::of(&QSpinBox::valueChanged), this, &AxisDock::minorTicksOpacityChanged);

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

	// template handler
	auto* frame = new QFrame(this);
	auto* hlayout = new QHBoxLayout(frame);
	hlayout->setContentsMargins(0, 11, 0, 11);

	auto* templateHandler = new TemplateHandler(this, TemplateHandler::ClassName::Axis);
	hlayout->addWidget(templateHandler);
	connect(templateHandler, &TemplateHandler::loadConfigRequested, this, &AxisDock::loadConfigFromTemplate);
	connect(templateHandler, &TemplateHandler::saveConfigRequested, this, &AxisDock::saveConfigAsTemplate);
	connect(templateHandler, &TemplateHandler::info, this, &AxisDock::info);

	ui.verticalLayout->addWidget(frame);

	init();
}

AxisDock::~AxisDock() {
	if (m_aspectTreeModel)
		delete m_aspectTreeModel;
}

void AxisDock::init() {
	Lock lock(m_initializing);

	// Validators
	ui.leStart->setValidator(new QDoubleValidator(ui.leStart));
	ui.leEnd->setValidator(new QDoubleValidator(ui.leEnd));
	ui.leMajorTickStartOffset->setValidator(new QDoubleValidator(ui.leMajorTickStartOffset));
	ui.leMajorTickStartValue->setValidator(new QDoubleValidator(ui.leMajorTickStartValue));

	// TODO move this stuff to retranslateUI()
	ui.cbPosition->addItem(i18n("Top")); // Left
	ui.cbPosition->addItem(i18n("Bottom")); // Right
	ui.cbPosition->addItem(i18n("Centered"));
	ui.cbPosition->addItem(i18n("Logical"));

	// range types
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

	// scales
	for (const auto& name : RangeT::scaleNames())
		ui.cbScale->addItem(name);

	ui.cbOrientation->addItem(i18n("Horizontal"));
	ui.cbOrientation->addItem(i18n("Vertical"));

	// Arrows
	ui.cbArrowType->addItem(i18n("No arrow"));
	ui.cbArrowType->addItem(i18n("Simple, Small"));
	ui.cbArrowType->addItem(i18n("Simple, Big"));
	ui.cbArrowType->addItem(i18n("Filled, Small"));
	ui.cbArrowType->addItem(i18n("Filled, Big"));
	ui.cbArrowType->addItem(i18n("Semi-filled, Small"));
	ui.cbArrowType->addItem(i18n("Semi-filled, Big"));

	QPainter pa;
	pa.setPen(QPen(Qt::SolidPattern, 0));
	QPixmap pm(20, 20);
	ui.cbArrowType->setIconSize(QSize(20, 20));

	// no arrow
	pm.fill(Qt::transparent);
	pa.begin(&pm);
	pa.setRenderHint(QPainter::Antialiasing);
	pa.setBrush(Qt::SolidPattern);
	pa.drawLine(3, 10, 17, 10);
	pa.end();
	ui.cbArrowType->setItemIcon(0, pm);

	// simple, small
	double cos_phi = cos(M_PI / 6.);
	pm.fill(Qt::transparent);
	pa.begin(&pm);
	pa.setRenderHint(QPainter::Antialiasing);
	pa.drawLine(3, 10, 17, 10);
	pa.drawLine(17, 10, 10, 10 - 5 * cos_phi);
	pa.drawLine(17, 10, 10, 10 + 5 * cos_phi);
	pa.end();
	ui.cbArrowType->setItemIcon(1, pm);

	// simple, big
	pm.fill(Qt::transparent);
	pa.begin(&pm);
	pa.setRenderHint(QPainter::Antialiasing);
	pa.drawLine(3, 10, 17, 10);
	pa.drawLine(17, 10, 10, 10 - 10 * cos_phi);
	pa.drawLine(17, 10, 10, 10 + 10 * cos_phi);
	pa.end();
	ui.cbArrowType->setItemIcon(2, pm);

	// filled, small
	pm.fill(Qt::transparent);
	pa.begin(&pm);
	pa.setRenderHint(QPainter::Antialiasing);
	pa.setBrush(Qt::SolidPattern);
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
	pa.setBrush(Qt::SolidPattern);
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
	pa.setBrush(Qt::SolidPattern);
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
	pa.setBrush(Qt::SolidPattern);
	pa.drawLine(3, 10, 17, 10);
	QPolygonF points6;
	points6 << QPointF(17, 10) << QPointF(10, 10 - 10 * cos_phi) << QPointF(13, 10) << QPointF(10, 10 + 10 * cos_phi);
	pa.drawPolygon(points6);
	pa.end();
	ui.cbArrowType->setItemIcon(6, pm);

	ui.cbArrowPosition->addItem(i18n("Left"));
	ui.cbArrowPosition->addItem(i18n("Right"));
	ui.cbArrowPosition->addItem(i18n("Both"));

	ui.cbMajorTicksDirection->addItem(i18n("None"));
	ui.cbMajorTicksDirection->addItem(i18n("In"));
	ui.cbMajorTicksDirection->addItem(i18n("Out"));
	ui.cbMajorTicksDirection->addItem(i18n("In and Out"));

	ui.cbMajorTicksType->addItem(i18n("Number"));
	ui.cbMajorTicksType->addItem(i18n("Spacing"));
	ui.cbMajorTicksType->addItem(i18n("Custom column"));

	ui.cbMajorTicksStartType->addItem(i18n("Absolute Value"));
	ui.cbMajorTicksStartType->addItem(i18n("Offset"));

	ui.cbMinorTicksDirection->addItem(i18n("None"));
	ui.cbMinorTicksDirection->addItem(i18n("In"));
	ui.cbMinorTicksDirection->addItem(i18n("Out"));
	ui.cbMinorTicksDirection->addItem(i18n("In and Out"));

	ui.cbMinorTicksType->addItem(i18n("Number"));
	ui.cbMinorTicksType->addItem(i18n("Spacing"));
	ui.cbMinorTicksType->addItem(i18n("Custom column"));

	GuiTools::updatePenStyles(ui.cbMajorTicksLineStyle, QColor(Qt::black));
	GuiTools::updatePenStyles(ui.cbMinorTicksLineStyle, QColor(Qt::black));

	// labels
	ui.cbLabelsPosition->addItem(i18n("No labels"));
	ui.cbLabelsPosition->addItem(i18n("Top"));
	ui.cbLabelsPosition->addItem(i18n("Bottom"));
	ui.cbLabelsTextType->addItem(i18n("Position values"));
	ui.cbLabelsTextType->addItem(i18n("Custom column"));

	// see Axis::labelsFormatToIndex() and Axis::indexToLabelsFormat()
	ui.cbLabelsFormat->addItem(i18n("Decimal notation"));
	ui.cbLabelsFormat->addItem(i18n("Scientific notation"));
	ui.cbLabelsFormat->addItem(i18n("Scientific E notation"));
	ui.cbLabelsFormat->addItem(i18n("Powers of 10"));
	ui.cbLabelsFormat->addItem(i18n("Powers of 2"));
	ui.cbLabelsFormat->addItem(i18n("Powers of e"));
	ui.cbLabelsFormat->addItem(i18n("Multiples of π"));

	ui.cbLabelsDateTimeFormat->addItems(AbstractColumn::dateTimeFormats());

	ui.cbLabelsBackgroundType->addItem(i18n("Transparent"));
	ui.cbLabelsBackgroundType->addItem(i18n("Color"));
}

void AxisDock::setModel() {
	QList<AspectType> list{AspectType::Folder, AspectType::Workbook, AspectType::Spreadsheet, AspectType::CantorWorksheet, AspectType::Column};
	cbMajorTicksColumn->setTopLevelClasses(list);
	cbMinorTicksColumn->setTopLevelClasses(list);
	cbLabelsTextColumn->setTopLevelClasses(list);

	list = {AspectType::Column};
	m_aspectTreeModel->setSelectableAspects(list);

	cbMajorTicksColumn->setModel(m_aspectTreeModel);
	cbMinorTicksColumn->setModel(m_aspectTreeModel);
	cbLabelsTextColumn->setModel(m_aspectTreeModel);
}

/*!
  sets the axes. The properties of the axes in the list \c list can be edited in this widget.
*/
void AxisDock::setAxes(QList<Axis*> list) {
	QDEBUG(Q_FUNC_INFO << ", Axis LIST =" << list)
	Lock lock(m_initializing);
	m_axesList = list;
	m_axis = list.first();
	setAspects(list);
	Q_ASSERT(m_axis != nullptr);
	m_aspectTreeModel = new AspectTreeModel(m_axis->project());
	this->setModel();

	labelWidget->setAxes(list);

	// if there are more then one axis in the list, disable the tab "general"
	if (list.size() == 1) {
		ui.lName->setEnabled(true);
		ui.leName->setEnabled(true);
		ui.lComment->setEnabled(true);
		ui.teComment->setEnabled(true);
		ui.leName->setText(m_axis->name());
		ui.teComment->setText(m_axis->comment());
		this->setModelIndexFromColumn(cbMajorTicksColumn, m_axis->majorTicksColumn());
		this->setModelIndexFromColumn(cbMinorTicksColumn, m_axis->minorTicksColumn());
		this->setModelIndexFromColumn(cbLabelsTextColumn, m_axis->labelsTextColumn());
	} else {
		ui.lName->setEnabled(false);
		ui.leName->setEnabled(false);
		ui.lComment->setEnabled(false);
		ui.teComment->setEnabled(false);
		ui.leName->setText(QString());
		ui.teComment->setText(QString());
		cbMajorTicksColumn->setCurrentModelIndex(QModelIndex());
		cbMinorTicksColumn->setCurrentModelIndex(QModelIndex());
		cbLabelsTextColumn->setCurrentModelIndex(QModelIndex());
	}
	ui.leName->setStyleSheet(QStringLiteral(""));
	ui.leName->setToolTip(QStringLiteral(""));

	// show the properties of the first axis
	this->load();

	QList<Line*> lines;
	QList<Line*> majorGridLines;
	QList<Line*> minorGridLines;
	for (auto* axis : m_axesList) {
		lines << axis->line();
		majorGridLines << axis->majorGridLine();
		minorGridLines << axis->minorGridLine();
	}

	lineWidget->setLines(lines);
	majorGridLineWidget->setLines(majorGridLines);
	minorGridLineWidget->setLines(minorGridLines);

	updatePlotRanges();
	initConnections();
}

void AxisDock::initConnections() {
	while (!m_connections.isEmpty())
		disconnect(m_connections.takeFirst());

	// general
	m_connections << connect(m_axis, &Axis::aspectDescriptionChanged, this, &AxisDock::aspectDescriptionChanged);
	m_connections << connect(m_axis, &Axis::orientationChanged, this, QOverload<Axis::Orientation>::of(&AxisDock::axisOrientationChanged));
	m_connections << connect(m_axis,
							 QOverload<Axis::Position>::of(&Axis::positionChanged),
							 this,
							 QOverload<Axis::Position>::of(&AxisDock::axisPositionChanged));
	m_connections << connect(m_axis, QOverload<double>::of(&Axis::positionChanged), this, QOverload<double>::of(&AxisDock::axisPositionChanged));
	m_connections << connect(m_axis, &Axis::logicalPositionChanged, this, &AxisDock::axisLogicalPositionChanged);
	m_connections << connect(m_axis, &Axis::scaleChanged, this, &AxisDock::axisScaleChanged);
	m_connections << connect(m_axis, &Axis::rangeTypeChanged, this, &AxisDock::axisRangeTypeChanged);
	m_connections << connect(m_axis, &Axis::startChanged, this, &AxisDock::axisStartChanged);
	m_connections << connect(m_axis, &Axis::endChanged, this, &AxisDock::axisEndChanged);
	m_connections << connect(m_axis, &Axis::zeroOffsetChanged, this, &AxisDock::axisZeroOffsetChanged);
	m_connections << connect(m_axis, &Axis::scalingFactorChanged, this, &AxisDock::axisScalingFactorChanged);
	m_connections << connect(m_axis, &Axis::showScaleOffsetChanged, this, &AxisDock::axisShowScaleOffsetChanged);
	m_connections << connect(m_axis, &WorksheetElement::plotRangeListChanged, this, &AxisDock::updatePlotRanges);

	// line
	m_connections << connect(m_axis, &Axis::arrowTypeChanged, this, &AxisDock::axisArrowTypeChanged);
	m_connections << connect(m_axis, &Axis::arrowPositionChanged, this, &AxisDock::axisArrowPositionChanged);
	m_connections << connect(m_axis, &Axis::arrowSizeChanged, this, &AxisDock::axisArrowSizeChanged);

	// ticks
	m_connections << connect(m_axis, &Axis::majorTicksDirectionChanged, this, &AxisDock::axisMajorTicksDirectionChanged);
	m_connections << connect(m_axis, &Axis::majorTicksTypeChanged, this, &AxisDock::axisMajorTicksTypeChanged);
	m_connections << connect(m_axis, &Axis::majorTicksAutoNumberChanged, this, &AxisDock::axisMajorTicksAutoNumberChanged);
	m_connections << connect(m_axis, &Axis::majorTicksNumberChanged, this, &AxisDock::axisMajorTicksNumberChanged);
	m_connections << connect(m_axis, &Axis::majorTicksSpacingChanged, this, &AxisDock::axisMajorTicksSpacingChanged);
	m_connections << connect(m_axis, &Axis::majorTicksStartTypeChanged, this, &AxisDock::axisMajorTicksStartTypeChanged);
	m_connections << connect(m_axis, &Axis::majorTickStartOffsetChanged, this, &AxisDock::axisMajorTicksStartOffsetChanged);
	m_connections << connect(m_axis, &Axis::majorTickStartValueChanged, this, &AxisDock::axisMajorTicksStartValueChanged);
	m_connections << connect(m_axis, &Axis::majorTicksColumnChanged, this, &AxisDock::axisMajorTicksColumnChanged);
	m_connections << connect(m_axis, &Axis::majorTicksPenChanged, this, &AxisDock::axisMajorTicksPenChanged);
	m_connections << connect(m_axis, &Axis::majorTicksLengthChanged, this, &AxisDock::axisMajorTicksLengthChanged);
	m_connections << connect(m_axis, &Axis::majorTicksOpacityChanged, this, &AxisDock::axisMajorTicksOpacityChanged);
	m_connections << connect(m_axis, &Axis::minorTicksDirectionChanged, this, &AxisDock::axisMinorTicksDirectionChanged);
	m_connections << connect(m_axis, &Axis::minorTicksTypeChanged, this, &AxisDock::axisMinorTicksTypeChanged);
	m_connections << connect(m_axis, &Axis::minorTicksAutoNumberChanged, this, &AxisDock::axisMinorTicksAutoNumberChanged);
	m_connections << connect(m_axis, &Axis::minorTicksNumberChanged, this, &AxisDock::axisMinorTicksNumberChanged);
	m_connections << connect(m_axis, &Axis::minorTicksIncrementChanged, this, &AxisDock::axisMinorTicksSpacingChanged);
	m_connections << connect(m_axis, &Axis::minorTicksColumnChanged, this, &AxisDock::axisMinorTicksColumnChanged);
	m_connections << connect(m_axis, &Axis::minorTicksPenChanged, this, &AxisDock::axisMinorTicksPenChanged);
	m_connections << connect(m_axis, &Axis::minorTicksLengthChanged, this, &AxisDock::axisMinorTicksLengthChanged);
	m_connections << connect(m_axis, &Axis::minorTicksOpacityChanged, this, &AxisDock::axisMinorTicksOpacityChanged);

	// labels
	m_connections << connect(m_axis, &Axis::labelsFormatChanged, this, &AxisDock::axisLabelsFormatChanged);
	m_connections << connect(m_axis, &Axis::labelsFormatAutoChanged, this, &AxisDock::axisLabelsFormatAutoChanged);
	m_connections << connect(m_axis, &Axis::labelsAutoPrecisionChanged, this, &AxisDock::axisLabelsAutoPrecisionChanged);
	m_connections << connect(m_axis, &Axis::labelsPrecisionChanged, this, &AxisDock::axisLabelsPrecisionChanged);
	m_connections << connect(m_axis, &Axis::labelsDateTimeFormatChanged, this, &AxisDock::axisLabelsDateTimeFormatChanged);
	m_connections << connect(m_axis, &Axis::labelsPositionChanged, this, &AxisDock::axisLabelsPositionChanged);
	m_connections << connect(m_axis, &Axis::labelsOffsetChanged, this, &AxisDock::axisLabelsOffsetChanged);
	m_connections << connect(m_axis, &Axis::labelsRotationAngleChanged, this, &AxisDock::axisLabelsRotationAngleChanged);
	m_connections << connect(m_axis, &Axis::labelsTextTypeChanged, this, &AxisDock::axisLabelsTextTypeChanged);
	m_connections << connect(m_axis, &Axis::labelsTextColumnChanged, this, &AxisDock::axisLabelsTextColumnChanged);
	m_connections << connect(m_axis, &Axis::labelsFontChanged, this, &AxisDock::axisLabelsFontChanged);
	m_connections << connect(m_axis, &Axis::labelsColorChanged, this, &AxisDock::axisLabelsFontColorChanged);
	m_connections << connect(m_axis, &Axis::labelsBackgroundTypeChanged, this, &AxisDock::axisLabelsBackgroundTypeChanged);
	m_connections << connect(m_axis, &Axis::labelsBackgroundColorChanged, this, &AxisDock::axisLabelsBackgroundColorChanged);
	m_connections << connect(m_axis, &Axis::labelsPrefixChanged, this, &AxisDock::axisLabelsPrefixChanged);
	m_connections << connect(m_axis, &Axis::labelsSuffixChanged, this, &AxisDock::axisLabelsSuffixChanged);
	m_connections << connect(m_axis, &Axis::labelsOpacityChanged, this, &AxisDock::axisLabelsOpacityChanged);

	m_connections << connect(m_axis, &Axis::visibleChanged, this, &AxisDock::axisVisibilityChanged);
}

/*
 * updates the locale in the widgets. called when the application settins are changed.
 */
void AxisDock::updateLocale() {
	SET_NUMBER_LOCALE
	ui.sbMajorTicksSpacingNumeric->setLocale(numberLocale);
	ui.sbMajorTicksWidth->setLocale(numberLocale);
	ui.sbMajorTicksLength->setLocale(numberLocale);
	ui.sbMinorTicksSpacingNumeric->setLocale(numberLocale);
	ui.sbMinorTicksWidth->setLocale(numberLocale);
	ui.sbMinorTicksLength->setLocale(numberLocale);
	ui.sbLabelsOffset->setLocale(numberLocale);

	// update the QLineEdits, avoid the change events
	Lock lock(m_initializing);
	ui.sbPosition->setLocale(numberLocale);
	ui.leStart->setText(numberLocale.toString(m_axis->range().start()));
	ui.leEnd->setText(numberLocale.toString(m_axis->range().end()));

	// scales
	ui.cbScale->clear();
	for (const auto& name : RangeT::scaleNames())
		ui.cbScale->addItem(name);

	labelWidget->updateLocale();
	lineWidget->updateLocale();
	majorGridLineWidget->updateLocale();
	minorGridLineWidget->updateLocale();
}

void AxisDock::activateTitleTab() {
	ui.tabWidget->setCurrentWidget(ui.tabTitle);
}

void AxisDock::setModelIndexFromColumn(TreeViewComboBox* cb, const AbstractColumn* column) {
	if (column)
		cb->setCurrentModelIndex(m_aspectTreeModel->modelIndexOfAspect(column));
	else
		cb->setCurrentModelIndex(QModelIndex());
}

void AxisDock::updatePlotRanges() {
	updatePlotRangeList(ui.cbPlotRanges);

	if (m_axis->coordinateSystemCount() == 0)
		return;

	Axis::Orientation orientation = m_axis->orientation();
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

//*************************************************************
//********** SLOTs for changes triggered in AxisDock **********
//*************************************************************
//"General"-tab
void AxisDock::visibilityChanged(bool state) {
	if (m_initializing)
		return;

	for (auto* axis : m_axesList)
		axis->setVisible(state);
}

/*!
	called if the orientation (horizontal or vertical) of the current axis is changed.
*/
void AxisDock::orientationChanged(int item) {
	auto orientation{Axis::Orientation(item)};
	if (orientation == Axis::Orientation::Horizontal) {
		ui.cbPosition->setItemText(Top_Left, i18n("Top"));
		ui.cbPosition->setItemText(Bottom_Right, i18n("Bottom"));
		// ui.cbPosition->setItemText(Center, i18n("Center") ); // must not updated
		ui.cbLabelsPosition->setItemText(1, i18n("Top"));
		ui.cbLabelsPosition->setItemText(2, i18n("Bottom"));
	} else { // vertical
		ui.cbPosition->setItemText(Top_Left, i18n("Left"));
		ui.cbPosition->setItemText(Bottom_Right, i18n("Right"));
		// ui.cbPosition->setItemText(Center, i18n("Center") ); // must not updated
		ui.cbLabelsPosition->setItemText(1, i18n("Right"));
		ui.cbLabelsPosition->setItemText(2, i18n("Left"));
	}

	if (m_initializing)
		return;

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

	if (m_initializing)
		return;

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
	if (m_initializing)
		return;

	double offset = Worksheet::convertToSceneUnits(value, m_worksheetUnit);
	for (auto* axis : m_axesList)
		axis->setOffset(offset);
}

void AxisDock::logicalPositionChanged(double value) {
	if (m_initializing)
		return;

	for (auto* axis : m_axesList)
		axis->setLogicalPosition(value);
}

void AxisDock::scaleChanged(int index) {
	if (m_initializing)
		return;

	auto scale = static_cast<RangeT::Scale>(index);
	for (auto* axis : m_axesList)
		axis->setScale(scale);
}

void AxisDock::rangeTypeChanged(int index) {
	auto rangeType = static_cast<Axis::RangeType>(index);
	bool autoScale = (rangeType != Axis::RangeType::Custom);
	ui.leStart->setEnabled(!autoScale);
	ui.leEnd->setEnabled(!autoScale);
	ui.dateTimeEditStart->setEnabled(!autoScale);
	ui.dateTimeEditEnd->setEnabled(!autoScale);

	if (m_initializing)
		return;

	for (auto* axis : m_axesList)
		axis->setRangeType(rangeType);

	updateLocale(); // update values
}

void AxisDock::startChanged() {
	if (m_initializing)
		return;

	bool ok;
	SET_NUMBER_LOCALE
	double value{numberLocale.toDouble(ui.leStart->text(), &ok)};
	if (!ok)
		return;

	// check first, whether the value for the lower limit is valid for the log- and square root scaling. If not, set the default values.
	const auto scale = RangeT::Scale(ui.cbScale->currentIndex());
	if (scale == RangeT::Scale::Log10 || scale == RangeT::Scale::Log2 || scale == RangeT::Scale::Ln) {
		if (value <= 0) {
			KMessageBox::sorry(this,
							   i18n("The axes lower limit has a non-positive value. Default minimal value will be used."),
							   i18n("Wrong lower limit value"));
			// TODO: why is this a good value?
			value = 0.01;
			ui.leStart->setText(numberLocale.toString(value));
		}
	} else if (scale == RangeT::Scale::Sqrt) {
		if (value < 0) {
			KMessageBox::sorry(this, i18n("The axes lower limit has a negative value. Default minimal value will be used."), i18n("Wrong lower limit value"));
			value = 0;
			ui.leStart->setText(numberLocale.toString(value));
		}
	}

	const Lock lock(m_initializing);
	for (auto* axis : m_axesList)
		axis->setStart(value);
}

void AxisDock::endChanged() {
	if (m_initializing)
		return;

	bool ok;
	SET_NUMBER_LOCALE
	const double value{numberLocale.toDouble(ui.leEnd->text(), &ok)};
	if (!ok)
		return;

	// TODO: also check for negative values and log-scales?

	const Lock lock(m_initializing);
	for (auto* axis : m_axesList)
		axis->setEnd(value);
}

void AxisDock::startDateTimeChanged(const QDateTime& dateTime) {
	if (m_initializing)
		return;

	quint64 value = dateTime.toMSecsSinceEpoch();
	for (auto* axis : m_axesList)
		axis->setStart(value);
}

void AxisDock::endDateTimeChanged(const QDateTime& dateTime) {
	if (m_initializing)
		return;

	quint64 value = dateTime.toMSecsSinceEpoch();
	for (auto* axis : m_axesList)
		axis->setEnd(value);
}

void AxisDock::zeroOffsetChanged(double offset) {
	DEBUG(Q_FUNC_INFO)
	if (m_initializing)
		return;

	// TODO: check for negative values and log scales? (Move this check into the axis it self!)

	for (auto* axis : m_axesList)
		axis->setZeroOffset(offset);
}

void AxisDock::setOffset(double offset) {
	SET_NUMBER_LOCALE
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
	if (m_initializing)
		return;

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
	if (m_initializing)
		return;

	const Lock lock(m_initializing);
	for (auto* axis : m_axesList)
		axis->setShowScaleOffset(state);
}

// "Line"-tab
void AxisDock::arrowTypeChanged(int index) {
	auto type = (Axis::ArrowType)index;
	if (type == Axis::ArrowType::NoArrow) {
		ui.cbArrowPosition->setEnabled(false);
		ui.sbArrowSize->setEnabled(false);
	} else {
		ui.cbArrowPosition->setEnabled(true);
		ui.sbArrowSize->setEnabled(true);
	}

	if (m_initializing)
		return;

	for (auto* axis : m_axesList)
		axis->setArrowType(type);
}

void AxisDock::arrowPositionChanged(int index) {
	if (m_initializing)
		return;

	auto position = (Axis::ArrowPosition)index;
	for (auto* axis : m_axesList)
		axis->setArrowPosition(position);
}

void AxisDock::arrowSizeChanged(int value) {
	if (m_initializing)
		return;

	double v = Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point);
	for (auto* axis : m_axesList)
		axis->setArrowSize(v);
}

//"Major ticks" tab
void AxisDock::majorTicksDirectionChanged(int index) {
	Axis::TicksDirection direction = Axis::TicksDirection(index);

	bool b = (direction != Axis::noTicks);
	ui.lMajorTicksType->setEnabled(b);
	ui.cbMajorTicksType->setEnabled(b);
	ui.lMajorTicksType->setEnabled(b);
	ui.cbMajorTicksType->setEnabled(b);
	ui.lMajorTicksNumber->setEnabled(b);
	ui.sbMajorTicksNumber->setEnabled(b);
	ui.lMajorTicksSpacingNumeric->setEnabled(b);
	ui.sbMajorTicksSpacingNumeric->setEnabled(b);
	ui.lMajorTicksIncrementDateTime->setEnabled(b);
	dtsbMajorTicksIncrement->setEnabled(b);
	ui.lMajorTicksLineStyle->setEnabled(b);
	ui.cbMajorTicksLineStyle->setEnabled(b);
	dtsbMinorTicksIncrement->setEnabled(b);
	if (b) {
		if (ui.cbMajorTicksLineStyle->currentIndex() != -1) {
			auto penStyle = Qt::PenStyle(ui.cbMajorTicksLineStyle->currentIndex());
			b = (penStyle != Qt::NoPen);
		} else
			b = false;
	}
	ui.lMajorTicksColor->setEnabled(b);
	ui.kcbMajorTicksColor->setEnabled(b);
	ui.lMajorTicksWidth->setEnabled(b);
	ui.sbMajorTicksWidth->setEnabled(b);
	ui.lMajorTicksLength->setEnabled(b);
	ui.sbMajorTicksLength->setEnabled(b);
	ui.lMajorTicksOpacity->setEnabled(b);
	ui.sbMajorTicksOpacity->setEnabled(b);

	if (m_initializing)
		return;

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

	auto type = Axis::TicksType(index);
	if (type == Axis::TicksType::TotalNumber) {
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
	} else if (type == Axis::TicksType::Spacing) {
		ui.lMajorTicksNumber->hide();
		ui.sbMajorTicksNumber->hide();
		ui.cbMajorTicksAutoNumber->hide();
		ui.lMajorTicksSpacingNumeric->show();
		ui.lMajorTicksStartType->show();
		ui.cbMajorTicksStartType->show();

		if (m_axis->isNumeric()) {
			ui.lMajorTicksIncrementDateTime->hide();
			dtsbMajorTicksIncrement->hide();
			ui.lMajorTicksSpacingNumeric->show();
			ui.sbMajorTicksSpacingNumeric->show();
		} else {
			ui.lMajorTicksIncrementDateTime->show();
			dtsbMajorTicksIncrement->show();
			ui.lMajorTicksSpacingNumeric->hide();
			ui.sbMajorTicksSpacingNumeric->hide();
		}

		ui.lMajorTicksColumn->hide();
		cbMajorTicksColumn->hide();
		ui.sbZeroOffset->show();
		ui.tbFirstTickAuto->show();
		ui.tbFirstTickData->show();
		updateMajorTicksStartType(true);

		// Check if spacing is not too small
		majorTicksSpacingChanged();
	} else { // custom column
		ui.lMajorTicksNumber->hide();
		ui.sbMajorTicksNumber->hide();
		ui.cbMajorTicksAutoNumber->hide();
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
	}

	if (m_initializing)
		return;

	for (auto* axis : m_axesList)
		axis->setMajorTicksType(type);
}

void AxisDock::majorTicksAutoNumberChanged(int value) {
	if (m_initializing)
		return;

	for (auto* axis : m_axesList)
		axis->setMajorTicksAutoNumber(value);
}

void AxisDock::majorTicksNumberChanged(int value) {
	DEBUG(Q_FUNC_INFO << ", number = " << value)
	if (m_initializing)
		return;

	for (auto* axis : m_axesList)
		axis->setMajorTicksNumber(value);
}

void AxisDock::majorTicksSpacingChanged() {
	if (m_initializing)
		return;

	bool numeric = m_axis->isNumeric();
	double spacing = numeric ? ui.sbMajorTicksSpacingNumeric->value() : dtsbMajorTicksIncrement->value();
	double range = m_axis->range().length();
	DEBUG(Q_FUNC_INFO << ", major spacing = " << spacing << ", range = " << range)

	// fix spacing if incorrect (not set or > 100 ticks)
	if (spacing == 0. || range / spacing > 100.) {
		if (spacing == 0.)
			spacing = range / (ui.sbMajorTicksNumber->value() - 1);

		if (range / spacing > 100.)
			spacing = range / 100.;

		DEBUG("new spacing = " << spacing)
		// determine stepsize and number of decimals
		m_initializing = true;
		if (numeric) {
			int decimals = nsl_math_rounded_decimals(spacing) + 1;
			DEBUG("decimals = " << decimals << ", step = " << gsl_pow_int(10., -decimals))
			ui.sbMajorTicksSpacingNumeric->setDecimals(decimals);
			ui.sbMajorTicksSpacingNumeric->setSingleStep(gsl_pow_int(10., -decimals));
			ui.sbMajorTicksSpacingNumeric->setMaximum(range);
			ui.sbMajorTicksSpacingNumeric->setValue(spacing);
		} else // TODO: check reversed axis
			dtsbMajorTicksIncrement->setValue(spacing);
		m_initializing = false;
	}

	for (auto* axis : m_axesList)
		axis->setMajorTicksSpacing(spacing);
}

void AxisDock::majorTicksStartTypeChanged(int state) {
	if (m_initializing)
		return;

	updateMajorTicksStartType(true);

	auto type = static_cast<Axis::TicksStartType>(state);
	for (auto* axis : m_axesList)
		axis->setMajorTicksStartType(type);
}

void AxisDock::majorTicksStartOffsetChanged() {
	if (m_initializing)
		return;

	if (ui.leMajorTickStartOffset->text().isEmpty()) // default value
		ui.leMajorTickStartOffset->setText(QStringLiteral("0"));

	bool ok;
	SET_NUMBER_LOCALE
	const double offset = numberLocale.toDouble(ui.leMajorTickStartOffset->text(), &ok);
	if (!ok)
		return;

	ui.leMajorTickStartOffset->setClearButtonEnabled(offset != 0);

	for (auto* axis : m_axesList)
		axis->setMajorTickStartOffset(offset);
}

void AxisDock::majorTicksStartValueChanged() {
	if (m_initializing)
		return;

	if (ui.leMajorTickStartValue->text().isEmpty()) // default value
		ui.leMajorTickStartValue->setText(QStringLiteral("0"));

	bool ok;
	SET_NUMBER_LOCALE
	const double value = numberLocale.toDouble(ui.leMajorTickStartValue->text(), &ok);
	if (!ok)
		return;

	ui.leMajorTickStartValue->setClearButtonEnabled(value != 0);

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

	SET_NUMBER_LOCALE
	ui.leMajorTickStartOffset->setText(numberLocale.toString(offset));
}

void AxisDock::majorTicksLineStyleChanged(int index) {
	if (index == -1)
		return;

	auto penStyle = Qt::PenStyle(index);

	bool b = (penStyle != Qt::NoPen);
	ui.lMajorTicksColor->setEnabled(b);
	ui.kcbMajorTicksColor->setEnabled(b);
	ui.lMajorTicksWidth->setEnabled(b);
	ui.sbMajorTicksWidth->setEnabled(b);
	ui.lMajorTicksLength->setEnabled(b);
	ui.sbMajorTicksLength->setEnabled(b);
	ui.lMajorTicksOpacity->setEnabled(b);
	ui.sbMajorTicksOpacity->setEnabled(b);

	if (m_initializing)
		return;

	QPen pen;
	for (auto* axis : m_axesList) {
		pen = axis->majorTicksPen();
		pen.setStyle(penStyle);
		axis->setMajorTicksPen(pen);
	}
}

void AxisDock::majorTicksColumnChanged(const QModelIndex& index) {
	if (m_initializing)
		return;

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	AbstractColumn* column = nullptr;
	if (aspect) {
		column = dynamic_cast<AbstractColumn*>(aspect);
		Q_ASSERT(column != nullptr);
	}

	for (auto* axis : m_axesList)
		axis->setMajorTicksColumn(column);
}

void AxisDock::majorTicksColorChanged(const QColor& color) {
	if (m_initializing)
		return;

	QPen pen;
	for (auto* axis : m_axesList) {
		pen = axis->majorTicksPen();
		pen.setColor(color);
		axis->setMajorTicksPen(pen);
	}

	m_initializing = true;
	GuiTools::updatePenStyles(ui.cbMajorTicksLineStyle, color);
	m_initializing = false;
}

void AxisDock::majorTicksWidthChanged(double value) {
	if (m_initializing)
		return;

	QPen pen;
	for (auto* axis : m_axesList) {
		pen = axis->majorTicksPen();
		pen.setWidthF(Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point));
		axis->setMajorTicksPen(pen);
	}
}

void AxisDock::majorTicksLengthChanged(double value) {
	if (m_initializing)
		return;

	for (auto* axis : m_axesList)
		axis->setMajorTicksLength(Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point));
}

void AxisDock::majorTicksOpacityChanged(int value) {
	if (m_initializing)
		return;

	qreal opacity{value / 100.};
	for (auto* axis : m_axesList)
		axis->setMajorTicksOpacity(opacity);
}

//"Minor ticks" tab
void AxisDock::minorTicksDirectionChanged(int index) {
	Axis::TicksDirection direction = Axis::TicksDirection(index);
	bool b = (direction != Axis::noTicks);
	ui.lMinorTicksType->setEnabled(b);
	ui.cbMinorTicksType->setEnabled(b);
	ui.lMinorTicksType->setEnabled(b);
	ui.cbMinorTicksType->setEnabled(b);
	ui.lMinorTicksNumber->setEnabled(b);
	ui.sbMinorTicksNumber->setEnabled(b);
	ui.lMinorTicksSpacingNumeric->setEnabled(b);
	ui.sbMinorTicksSpacingNumeric->setEnabled(b);
	ui.lMinorTicksIncrementDateTime->setEnabled(b);
	dtsbMinorTicksIncrement->setEnabled(b);
	ui.lMinorTicksLineStyle->setEnabled(b);
	ui.cbMinorTicksLineStyle->setEnabled(b);
	if (b) {
		if (ui.cbMinorTicksLineStyle->currentIndex() != -1) {
			auto penStyle = Qt::PenStyle(ui.cbMinorTicksLineStyle->currentIndex());
			b = (penStyle != Qt::NoPen);
		} else
			b = false;
	}
	ui.lMinorTicksColor->setEnabled(b);
	ui.kcbMinorTicksColor->setEnabled(b);
	ui.lMinorTicksWidth->setEnabled(b);
	ui.sbMinorTicksWidth->setEnabled(b);
	ui.lMinorTicksLength->setEnabled(b);
	ui.sbMinorTicksLength->setEnabled(b);
	ui.lMinorTicksOpacity->setEnabled(b);
	ui.sbMinorTicksOpacity->setEnabled(b);

	if (m_initializing)
		return;

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

	if (m_initializing)
		return;

	for (auto* axis : m_axesList)
		axis->setMinorTicksType(type);
}

void AxisDock::minorTicksAutoNumberChanged(int value) {
	if (m_initializing)
		return;

	for (auto* axis : m_axesList)
		axis->setMinorTicksAutoNumber(value);
}

void AxisDock::minorTicksNumberChanged(int value) {
	if (m_initializing)
		return;

	for (auto* axis : m_axesList)
		axis->setMinorTicksNumber(value);
}

void AxisDock::minorTicksSpacingChanged() {
	if (m_initializing)
		return;

	bool numeric = m_axis->isNumeric();
	double spacing = numeric ? ui.sbMinorTicksSpacingNumeric->value() : dtsbMinorTicksIncrement->value();
	double range = m_axis->range().length();
	// DEBUG("minor spacing = " << spacing << ", range = " << range)
	int numberTicks = 0;

	int majorTicks = m_axis->majorTicksNumber();
	if (spacing > 0.)
		numberTicks = range / (majorTicks - 1) / spacing - 1; // recalc
	// DEBUG("	nticks = " << numberTicks)

	// set if unset or > 100.
	if (spacing == 0. || numberTicks > 100) {
		if (spacing == 0.)
			spacing = range / (majorTicks - 1) / (ui.sbMinorTicksNumber->value() + 1);

		numberTicks = range / (majorTicks - 1) / spacing - 1; // recalculate number of ticks

		if (numberTicks > 100) // maximum 100 minor ticks
			spacing = range / (majorTicks - 1) / (100 + 1);

		DEBUG("new spacing = " << spacing)
		DEBUG("new nticks = " << numberTicks)
		// determine stepsize and number of decimals
		m_initializing = true;
		if (numeric) {
			int decimals = nsl_math_rounded_decimals(spacing) + 1;
			DEBUG("decimals = " << decimals << ", step = " << gsl_pow_int(10., -decimals))
			ui.sbMinorTicksSpacingNumeric->setDecimals(decimals);
			ui.sbMinorTicksSpacingNumeric->setSingleStep(gsl_pow_int(10., -decimals));
			ui.sbMinorTicksSpacingNumeric->setMaximum(range);
			ui.sbMinorTicksSpacingNumeric->setValue(spacing);
		} else
			dtsbMinorTicksIncrement->setValue(spacing);
		m_initializing = false;
	}

	for (auto* axis : m_axesList)
		axis->setMinorTicksSpacing(spacing);
}

void AxisDock::minorTicksColumnChanged(const QModelIndex& index) {
	if (m_initializing)
		return;

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	auto* column = dynamic_cast<AbstractColumn*>(aspect);
	Q_ASSERT(column != nullptr);

	for (auto* axis : m_axesList)
		axis->setMinorTicksColumn(column);
}

void AxisDock::minorTicksLineStyleChanged(int index) {
	if (index == -1)
		return;

	auto penStyle = Qt::PenStyle(index);

	bool b = (penStyle != Qt::NoPen);
	ui.lMinorTicksColor->setEnabled(b);
	ui.kcbMinorTicksColor->setEnabled(b);
	ui.lMinorTicksWidth->setEnabled(b);
	ui.sbMinorTicksWidth->setEnabled(b);
	ui.lMinorTicksLength->setEnabled(b);
	ui.sbMinorTicksLength->setEnabled(b);
	ui.lMinorTicksOpacity->setEnabled(b);
	ui.sbMinorTicksOpacity->setEnabled(b);

	if (m_initializing)
		return;

	QPen pen;
	for (auto* axis : m_axesList) {
		pen = axis->minorTicksPen();
		pen.setStyle(penStyle);
		axis->setMinorTicksPen(pen);
	}
}

void AxisDock::minorTicksColorChanged(const QColor& color) {
	if (m_initializing)
		return;

	QPen pen;
	for (auto* axis : m_axesList) {
		pen = axis->minorTicksPen();
		pen.setColor(color);
		axis->setMinorTicksPen(pen);
	}

	m_initializing = true;
	GuiTools::updatePenStyles(ui.cbMinorTicksLineStyle, color);
	m_initializing = false;
}

void AxisDock::minorTicksWidthChanged(double value) {
	if (m_initializing)
		return;

	QPen pen;
	for (auto* axis : m_axesList) {
		pen = axis->minorTicksPen();
		pen.setWidthF(Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point));
		axis->setMinorTicksPen(pen);
	}
}

void AxisDock::minorTicksLengthChanged(double value) {
	if (m_initializing)
		return;

	for (auto* axis : m_axesList)
		axis->setMinorTicksLength(Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point));
}

void AxisDock::minorTicksOpacityChanged(int value) {
	if (m_initializing)
		return;

	qreal opacity{value / 100.};
	for (auto* axis : m_axesList)
		axis->setMinorTicksOpacity(opacity);
}

//"Tick labels"-tab
void AxisDock::labelsFormatChanged(int index) {
	if (m_initializing)
		return;

	for (auto* axis : m_axesList)
		axis->setLabelsFormat(Axis::indexToLabelsFormat(index));
}

void AxisDock::labelsFormatAutoChanged(bool automatic) {
	if (m_initializing)
		return;

	for (auto* axis : m_axesList)
		axis->setLabelsFormatAuto(automatic);
}

void AxisDock::labelsPrecisionChanged(int value) {
	if (m_initializing)
		return;

	for (auto* axis : m_axesList)
		axis->setLabelsPrecision(value);
}

void AxisDock::labelsAutoPrecisionChanged(bool state) {
	ui.sbLabelsPrecision->setEnabled(!state);

	if (m_initializing)
		return;

	for (auto* axis : m_axesList)
		axis->setLabelsAutoPrecision(state);
}

void AxisDock::labelsDateTimeFormatChanged() {
	if (m_initializing)
		return;

	for (auto* axis : m_axesList)
		axis->setLabelsDateTimeFormat(ui.cbLabelsDateTimeFormat->currentText());
}

void AxisDock::labelsPositionChanged(int index) {
	auto position = Axis::LabelsPosition(index);

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

	if (m_initializing)
		return;

	for (auto* axis : m_axesList)
		axis->setLabelsPosition(position);
}

void AxisDock::labelsOffsetChanged(double value) {
	if (m_initializing)
		return;

	for (auto* axis : m_axesList)
		axis->setLabelsOffset(Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point));
}

void AxisDock::labelsRotationChanged(int value) {
	if (m_initializing)
		return;

	for (auto* axis : m_axesList)
		axis->setLabelsRotationAngle(value);
}

void AxisDock::labelsTextTypeChanged(int index) {
	if (!m_axis)
		return; // don't do anything when we're addItem()'ing strings and the axis is not available yet

	auto type = Axis::LabelsTextType(index);
	if (type == Axis::LabelsTextType::PositionValues) {
		ui.lLabelsTextColumn->hide();
		cbLabelsTextColumn->hide();

		bool numeric = m_axis->isNumeric();
		ui.lLabelsFormat->setVisible(numeric);
		ui.cbLabelsFormat->setVisible(numeric);
		ui.chkLabelsAutoPrecision->setVisible(numeric);
		ui.lLabelsPrecision->setVisible(numeric);
		ui.sbLabelsPrecision->setVisible(numeric);
		ui.lLabelsDateTimeFormat->setVisible(!numeric);
		ui.cbLabelsDateTimeFormat->setVisible(!numeric);
	} else {
		ui.lLabelsTextColumn->show();
		cbLabelsTextColumn->show();
		labelsTextColumnChanged(cbLabelsTextColumn->currentModelIndex());
	}

	if (m_initializing)
		return;

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
			ui.cbLabelsFormat->show();
			ui.lLabelsPrecision->show();
			ui.framePrecision->show();
			ui.lLabelsDateTimeFormat->hide();
			ui.cbLabelsDateTimeFormat->hide();
			break;
		case AbstractColumn::ColumnMode::Text:
			ui.lLabelsFormat->hide();
			ui.cbLabelsFormat->hide();
			ui.lLabelsPrecision->hide();
			ui.framePrecision->hide();
			ui.lLabelsDateTimeFormat->hide();
			ui.cbLabelsDateTimeFormat->hide();
			break;
		case AbstractColumn::ColumnMode::DateTime:
		case AbstractColumn::ColumnMode::Month:
		case AbstractColumn::ColumnMode::Day:
			ui.lLabelsFormat->hide();
			ui.cbLabelsFormat->hide();
			ui.lLabelsPrecision->hide();
			ui.framePrecision->hide();
			ui.lLabelsDateTimeFormat->show();
			ui.cbLabelsDateTimeFormat->show();
			break;
		}
	} else {
		auto type = Axis::LabelsTextType(ui.cbLabelsTextType->currentIndex());
		if (type == Axis::LabelsTextType::CustomValues) {
			ui.lLabelsFormat->hide();
			ui.cbLabelsFormat->hide();
			ui.lLabelsPrecision->hide();
			ui.framePrecision->hide();
			ui.lLabelsDateTimeFormat->hide();
			ui.cbLabelsDateTimeFormat->hide();
		}
	}

	if (m_initializing)
		return;

	for (auto* axis : m_axesList)
		axis->setLabelsTextColumn(column);
}

void AxisDock::labelsPrefixChanged() {
	if (m_initializing)
		return;

	const QString& prefix = ui.leLabelsPrefix->text();
	for (auto* axis : m_axesList)
		axis->setLabelsPrefix(prefix);
}

void AxisDock::labelsSuffixChanged() {
	if (m_initializing)
		return;

	const QString& suffix = ui.leLabelsSuffix->text();
	for (auto* axis : m_axesList)
		axis->setLabelsSuffix(suffix);
}

void AxisDock::labelsFontChanged(const QFont& font) {
	if (m_initializing)
		return;

	QFont labelsFont = font;
	labelsFont.setPixelSize(Worksheet::convertToSceneUnits(font.pointSizeF(), Worksheet::Unit::Point));
	for (auto* axis : m_axesList)
		axis->setLabelsFont(labelsFont);
}

void AxisDock::labelsFontColorChanged(const QColor& color) {
	if (m_initializing)
		return;

	for (auto* axis : m_axesList)
		axis->setLabelsColor(color);
}

void AxisDock::labelsBackgroundTypeChanged(int index) {
	auto type = Axis::LabelsBackgroundType(index);

	bool transparent = (type == Axis::LabelsBackgroundType::Transparent);
	ui.lLabelsBackgroundColor->setVisible(!transparent);
	ui.kcbLabelsBackgroundColor->setVisible(!transparent);

	if (m_initializing)
		return;

	for (auto* axis : m_axesList)
		axis->setLabelsBackgroundType(type);
}

void AxisDock::labelsBackgroundColorChanged(const QColor& color) {
	if (m_initializing)
		return;

	for (auto* axis : m_axesList)
		axis->setLabelsBackgroundColor(color);
}

void AxisDock::labelsOpacityChanged(int value) {
	if (m_initializing)
		return;

	qreal opacity{value / 100.};
	for (auto* axis : m_axesList)
		axis->setLabelsOpacity(opacity);
}

//*************************************************************
//************ SLOTs for changes triggered in Axis ************
//*************************************************************
void AxisDock::axisOrientationChanged(Axis::Orientation orientation) {
	m_initializing = true;
	ui.cbOrientation->setCurrentIndex(static_cast<int>(orientation));
	m_initializing = false;
}

void AxisDock::axisPositionChanged(Axis::Position position) {
	m_initializing = true;

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

	m_initializing = false;
}

void AxisDock::axisPositionChanged(double value) {
	const Lock lock(m_initializing);
	ui.sbPosition->setValue(Worksheet::convertFromSceneUnits(value, m_worksheetUnit));
}

void AxisDock::axisLogicalPositionChanged(double value) {
	const Lock lock(m_initializing);
	ui.sbPositionLogical->setValue(value);
}

void AxisDock::axisScaleChanged(RangeT::Scale scale) {
	const Lock lock(m_initializing);
	ui.cbScale->setCurrentIndex(static_cast<int>(scale));
}

void AxisDock::axisRangeTypeChanged(Axis::RangeType type) {
	const Lock lock(m_initializing);
	ui.cbRangeType->setCurrentIndex(static_cast<int>(type));
}

void AxisDock::axisStartChanged(double value) {
	if (m_initializing)
		return;
	const Lock lock(m_initializing);

	SET_NUMBER_LOCALE
	ui.leStart->setText(numberLocale.toString(value));
	ui.dateTimeEditStart->setDateTime(QDateTime::fromMSecsSinceEpoch(value));

	// determine stepsize and number of decimals
	const double range{m_axis->range().length()};
	const int decimals{nsl_math_rounded_decimals(range) + 1};
	DEBUG("range = " << range << ", decimals = " << decimals)
	ui.sbMajorTicksSpacingNumeric->setDecimals(decimals);
	ui.sbMajorTicksSpacingNumeric->setSingleStep(gsl_pow_int(10., -decimals));
	ui.sbMajorTicksSpacingNumeric->setMaximum(range);
}

void AxisDock::axisEndChanged(double value) {
	if (m_initializing)
		return;
	const Lock lock(m_initializing);

	SET_NUMBER_LOCALE
	ui.leEnd->setText(numberLocale.toString(value));
	ui.dateTimeEditEnd->setDateTime(QDateTime::fromMSecsSinceEpoch(value));

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
	const Lock lock(m_initializing);
	ui.sbZeroOffset->setValue(value);
}
void AxisDock::axisScalingFactorChanged(qreal value) {
	const Lock lock(m_initializing);
	ui.sbScalingFactor->setValue(value);
}
void AxisDock::axisShowScaleOffsetChanged(bool b) {
	if (m_initializing)
		return;
	const Lock lock(m_initializing);
	ui.chkShowScaleOffset->setChecked(b);
}

// line
void AxisDock::axisArrowTypeChanged(Axis::ArrowType type) {
	m_initializing = true;
	ui.cbArrowType->setCurrentIndex(static_cast<int>(type));
	m_initializing = false;
}

void AxisDock::axisArrowPositionChanged(Axis::ArrowPosition position) {
	m_initializing = true;
	ui.cbArrowPosition->setCurrentIndex(static_cast<int>(position));
	m_initializing = false;
}

void AxisDock::axisArrowSizeChanged(qreal size) {
	m_initializing = true;
	ui.sbArrowSize->setValue((int)Worksheet::convertFromSceneUnits(size, Worksheet::Unit::Point));
	m_initializing = false;
}

// major ticks
void AxisDock::axisMajorTicksDirectionChanged(Axis::TicksDirection direction) {
	m_initializing = true;
	ui.cbMajorTicksDirection->setCurrentIndex(direction);
	m_initializing = false;
}
void AxisDock::axisMajorTicksTypeChanged(Axis::TicksType type) {
	m_initializing = true;
	ui.cbMajorTicksType->setCurrentIndex(static_cast<int>(type));
	m_initializing = false;
}
void AxisDock::axisMajorTicksAutoNumberChanged(bool automatic) {
	m_initializing = true;
	ui.cbMajorTicksAutoNumber->setChecked(automatic);
	ui.sbMajorTicksNumber->setEnabled(!automatic);
	m_initializing = false;
}
void AxisDock::axisMajorTicksNumberChanged(int number) {
	m_initializing = true;
	ui.sbMajorTicksNumber->setValue(number);
	m_initializing = false;
}
void AxisDock::axisMajorTicksSpacingChanged(qreal increment) {
	Lock lock(m_initializing);
	if (m_axis->isNumeric())
		ui.sbMajorTicksSpacingNumeric->setValue(increment);
	else
		dtsbMajorTicksIncrement->setValue(increment);
}
void AxisDock::axisMajorTicksStartTypeChanged(Axis::TicksStartType type) {
	const Lock lock(m_initializing);
	ui.cbMajorTicksStartType->setCurrentIndex(static_cast<int>(type));
}
void AxisDock::axisMajorTicksStartOffsetChanged(qreal value) {
	const Lock lock(m_initializing);
	SET_NUMBER_LOCALE
	ui.leMajorTickStartOffset->setText(numberLocale.toString(value));
}
void AxisDock::axisMajorTicksStartValueChanged(qreal value) {
	if (m_initializing)
		return;
	const Lock lock(m_initializing);
	SET_NUMBER_LOCALE
	ui.leMajorTickStartValue->setText(numberLocale.toString(value));
}
void AxisDock::axisMajorTicksColumnChanged(const AbstractColumn* column) {
	Lock lock(m_initializing);
	cbMajorTicksColumn->setColumn(column, m_axis->majorTicksColumnPath());
}
void AxisDock::axisMajorTicksPenChanged(const QPen& pen) {
	m_initializing = true;
	ui.cbMajorTicksLineStyle->setCurrentIndex(pen.style());
	ui.kcbMajorTicksColor->setColor(pen.color());
	ui.sbMajorTicksWidth->setValue(Worksheet::convertFromSceneUnits(pen.widthF(), Worksheet::Unit::Point));
	m_initializing = false;
}
void AxisDock::axisMajorTicksLengthChanged(qreal length) {
	m_initializing = true;
	ui.sbMajorTicksLength->setValue(Worksheet::convertFromSceneUnits(length, Worksheet::Unit::Point));
	m_initializing = false;
}
void AxisDock::axisMajorTicksOpacityChanged(qreal opacity) {
	m_initializing = true;
	ui.sbMajorTicksOpacity->setValue(round(opacity * 100.0));
	m_initializing = false;
}

// minor ticks
void AxisDock::axisMinorTicksDirectionChanged(Axis::TicksDirection direction) {
	m_initializing = true;
	ui.cbMinorTicksDirection->setCurrentIndex(direction);
	m_initializing = false;
}
void AxisDock::axisMinorTicksTypeChanged(Axis::TicksType type) {
	m_initializing = true;
	ui.cbMinorTicksType->setCurrentIndex(static_cast<int>(type));
	m_initializing = false;
}
void AxisDock::axisMinorTicksAutoNumberChanged(bool automatic) {
	m_initializing = true;
	ui.cbMinorTicksAutoNumber->setChecked(automatic);
	ui.sbMinorTicksNumber->setEnabled(!automatic);
	m_initializing = false;
}
void AxisDock::axisMinorTicksNumberChanged(int number) {
	m_initializing = true;
	ui.sbMinorTicksNumber->setValue(number);
	m_initializing = false;
}
void AxisDock::axisMinorTicksSpacingChanged(qreal increment) {
	Lock lock(m_initializing);
	if (m_axis->isNumeric())
		ui.sbMinorTicksSpacingNumeric->setValue(increment);
	else
		dtsbMinorTicksIncrement->setValue(increment);
}
void AxisDock::axisMinorTicksColumnChanged(const AbstractColumn* column) {
	Lock lock(m_initializing);
	cbMinorTicksColumn->setColumn(column, m_axis->minorTicksColumnPath());
}
void AxisDock::axisMinorTicksPenChanged(const QPen& pen) {
	m_initializing = true;
	ui.cbMinorTicksLineStyle->setCurrentIndex(pen.style());
	ui.kcbMinorTicksColor->setColor(pen.color());
	ui.sbMinorTicksWidth->setValue(Worksheet::convertFromSceneUnits(pen.widthF(), Worksheet::Unit::Point));
	m_initializing = false;
}
void AxisDock::axisMinorTicksLengthChanged(qreal length) {
	m_initializing = true;
	ui.sbMinorTicksLength->setValue(Worksheet::convertFromSceneUnits(length, Worksheet::Unit::Point));
	m_initializing = false;
}
void AxisDock::axisMinorTicksOpacityChanged(qreal opacity) {
	m_initializing = true;
	ui.sbMinorTicksOpacity->setValue(round(opacity * 100.0));
	m_initializing = false;
}

// labels
void AxisDock::axisLabelsFormatChanged(Axis::LabelsFormat format) {
	m_initializing = true;
	ui.cbLabelsFormat->setCurrentIndex(Axis::labelsFormatToIndex(format));
	m_initializing = false;
}
void AxisDock::axisLabelsFormatAutoChanged(bool automatic) {
	m_initializing = true;
	ui.chkLabelsFormatAuto->setChecked(automatic);
	ui.cbLabelsFormat->setEnabled(!automatic);
	m_initializing = false;
}
void AxisDock::axisLabelsAutoPrecisionChanged(bool on) {
	m_initializing = true;
	ui.chkLabelsAutoPrecision->setChecked(on);
	m_initializing = false;
}
void AxisDock::axisLabelsPrecisionChanged(int precision) {
	m_initializing = true;
	ui.sbLabelsPrecision->setValue(precision);
	m_initializing = false;
}
void AxisDock::axisLabelsDateTimeFormatChanged(const QString& format) {
	m_initializing = true;
	ui.cbLabelsDateTimeFormat->setCurrentText(format);
	m_initializing = false;
}
void AxisDock::axisLabelsPositionChanged(Axis::LabelsPosition position) {
	m_initializing = true;
	ui.cbLabelsPosition->setCurrentIndex(static_cast<int>(position));
	m_initializing = false;
}
void AxisDock::axisLabelsOffsetChanged(double offset) {
	m_initializing = true;
	ui.sbLabelsOffset->setValue(Worksheet::convertFromSceneUnits(offset, Worksheet::Unit::Point));
	m_initializing = false;
}
void AxisDock::axisLabelsRotationAngleChanged(qreal rotation) {
	m_initializing = true;
	ui.sbLabelsRotation->setValue(rotation);
	m_initializing = false;
}
void AxisDock::axisLabelsTextTypeChanged(Axis::LabelsTextType type) {
	m_initializing = true;
	ui.cbLabelsTextType->setCurrentIndex(static_cast<int>(type));
	m_initializing = false;
}
void AxisDock::axisLabelsTextColumnChanged(const AbstractColumn* column) {
	m_initializing = true;
	cbLabelsTextColumn->setColumn(column, m_axis->labelsTextColumnPath());
	m_initializing = false;
}
void AxisDock::axisLabelsFontChanged(const QFont& font) {
	m_initializing = true;
	// we need to set the font size in points for KFontRequester
	QFont newFont(font);
	newFont.setPointSizeF(round(Worksheet::convertFromSceneUnits(font.pixelSize(), Worksheet::Unit::Point)));
	ui.kfrLabelsFont->setFont(newFont);
	m_initializing = false;
}
void AxisDock::axisLabelsFontColorChanged(const QColor& color) {
	m_initializing = true;
	ui.kcbLabelsFontColor->setColor(color);
	m_initializing = false;
}
void AxisDock::axisLabelsBackgroundTypeChanged(Axis::LabelsBackgroundType type) {
	m_initializing = true;
	ui.cbLabelsBackgroundType->setCurrentIndex(static_cast<int>(type));
	m_initializing = false;
}
void AxisDock::axisLabelsBackgroundColorChanged(const QColor& color) {
	m_initializing = true;
	ui.kcbLabelsBackgroundColor->setColor(color);
	m_initializing = false;
}
void AxisDock::axisLabelsPrefixChanged(const QString& prefix) {
	m_initializing = true;
	ui.leLabelsPrefix->setText(prefix);
	m_initializing = false;
}
void AxisDock::axisLabelsSuffixChanged(const QString& suffix) {
	m_initializing = true;
	ui.leLabelsSuffix->setText(suffix);
	m_initializing = false;
}
void AxisDock::axisLabelsOpacityChanged(qreal opacity) {
	m_initializing = true;
	ui.sbLabelsOpacity->setValue(round(opacity * 100.0));
	m_initializing = false;
}

void AxisDock::updateMajorTicksStartType(bool visible) {
	const bool absoluteValue = (ui.cbMajorTicksStartType->currentIndex() == 0);

	ui.lMajorTickStartOffset->setVisible(visible && !absoluteValue);
	ui.leMajorTickStartOffset->setVisible(visible && !absoluteValue);
	ui.tbFirstTickData->setVisible(visible && !absoluteValue);
	ui.tbFirstTickAuto->setVisible(visible && !absoluteValue);
	ui.leMajorTickStartValue->setVisible(visible && absoluteValue);
	ui.lMajorTickStartValue->setVisible(visible && absoluteValue);
}

void AxisDock::axisVisibilityChanged(bool on) {
	m_initializing = true;
	ui.chkVisible->setChecked(on);
	m_initializing = false;
}

//*************************************************************
//************************* Settings **************************
//*************************************************************
void AxisDock::load() {
	// General
	ui.chkVisible->setChecked(m_axis->isVisible());

	Axis::Orientation orientation = m_axis->orientation();
	ui.cbOrientation->setCurrentIndex(static_cast<int>(orientation));

	const auto* plot = static_cast<const CartesianPlot*>(m_axis->parentAspect());
	const auto* cSystem = plot->coordinateSystem(m_axis->coordinateSystemIndex());
	const int xIndex{cSystem->index(Dimension::X)}, yIndex{cSystem->index(Dimension::Y)};

	Range<double> logicalRange(0, 0);
	if (orientation == Axis::Orientation::Horizontal) {
		logicalRange = plot->range(Dimension::Y, yIndex);
		ui.cbPosition->setItemText(Top_Left, i18n("Top"));
		ui.cbPosition->setItemText(Bottom_Right, i18n("Bottom"));
		ui.cbPosition->setItemText(Center, i18n("Centered"));
	} else {
		logicalRange = plot->range(Dimension::X, xIndex);
		ui.cbPosition->setItemText(Top_Left, i18n("Left"));
		ui.cbPosition->setItemText(Bottom_Right, i18n("Right"));
		ui.cbPosition->setItemText(Center, i18n("Centered"));
	}

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

	SET_NUMBER_LOCALE
	ui.sbPosition->setValue(Worksheet::convertFromSceneUnits(m_axis->offset(), m_worksheetUnit));

	spinBoxCalculateMinMax(ui.sbPositionLogical, logicalRange, m_axis->logicalPosition());
	ui.sbPositionLogical->setValue(m_axis->logicalPosition());

	ui.cbScale->setCurrentIndex(static_cast<int>(m_axis->scale()));
	ui.cbRangeType->setCurrentIndex(static_cast<int>(m_axis->rangeType()));
	ui.leStart->setText(numberLocale.toString(m_axis->range().start()));
	ui.leEnd->setText(numberLocale.toString(m_axis->range().end()));

	// depending on the range format of the axis (numeric vs. datetime), show/hide the corresponding widgets
	bool numeric = m_axis->isNumeric();

	// ranges
	ui.lStart->setVisible(numeric);
	ui.lEnd->setVisible(numeric);
	ui.leStart->setVisible(numeric);
	ui.leEnd->setVisible(numeric);
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
		ui.dateTimeEditStart->setDateTime(QDateTime::fromMSecsSinceEpoch(m_axis->range().start()));
		ui.dateTimeEditEnd->setDateTime(QDateTime::fromMSecsSinceEpoch(m_axis->range().end()));
	}

	ui.sbZeroOffset->setValue(m_axis->zeroOffset());
	ui.sbScalingFactor->setValue(m_axis->scalingFactor());
	ui.chkShowScaleOffset->setChecked(m_axis->showScaleOffset());

	// Line
	ui.cbArrowType->setCurrentIndex((int)m_axis->arrowType());
	ui.cbArrowPosition->setCurrentIndex((int)m_axis->arrowPosition());
	ui.sbArrowSize->setValue((int)Worksheet::convertFromSceneUnits(m_axis->arrowSize(), Worksheet::Unit::Point));

	// Major ticks
	ui.cbMajorTicksDirection->setCurrentIndex((int)m_axis->majorTicksDirection());
	ui.cbMajorTicksType->setCurrentIndex((int)m_axis->majorTicksType());
	ui.cbMajorTicksAutoNumber->setChecked(m_axis->majorTicksAutoNumber());
	ui.sbMajorTicksNumber->setEnabled(!m_axis->majorTicksAutoNumber());

	ui.sbMajorTicksNumber->setValue(m_axis->majorTicksNumber());
	auto value{m_axis->majorTicksSpacing()};
	if (numeric) {
		ui.sbMajorTicksSpacingNumeric->setDecimals(nsl_math_decimal_places(value) + 1);
		ui.sbMajorTicksSpacingNumeric->setValue(value);
		ui.sbMajorTicksSpacingNumeric->setSingleStep(value / 10.);
	} else
		dtsbMajorTicksIncrement->setValue(value);
	ui.cbMajorTicksStartType->setCurrentIndex(static_cast<int>(m_axis->majorTicksStartType()));
	ui.leMajorTickStartOffset->setText(numberLocale.toString(m_axis->majorTickStartOffset()));
	ui.leMajorTickStartValue->setText(numberLocale.toString(m_axis->majorTickStartValue()));
	ui.cbMajorTicksLineStyle->setCurrentIndex((int)m_axis->majorTicksPen().style());
	ui.kcbMajorTicksColor->setColor(m_axis->majorTicksPen().color());
	ui.sbMajorTicksWidth->setValue(Worksheet::convertFromSceneUnits(m_axis->majorTicksPen().widthF(), Worksheet::Unit::Point));
	ui.sbMajorTicksLength->setValue(Worksheet::convertFromSceneUnits(m_axis->majorTicksLength(), Worksheet::Unit::Point));
	ui.sbMajorTicksOpacity->setValue(round(m_axis->majorTicksOpacity() * 100.0));

	// Minor ticks
	ui.cbMinorTicksDirection->setCurrentIndex((int)m_axis->minorTicksDirection());
	ui.cbMinorTicksType->setCurrentIndex((int)m_axis->minorTicksType());
	ui.cbMinorTicksAutoNumber->setChecked(m_axis->majorTicksAutoNumber());
	ui.sbMinorTicksNumber->setEnabled(!m_axis->majorTicksAutoNumber());
	ui.sbMinorTicksNumber->setValue(m_axis->minorTicksNumber());
	ui.cbMinorTicksLineStyle->setCurrentIndex((int)m_axis->minorTicksPen().style());
	ui.kcbMinorTicksColor->setColor(m_axis->minorTicksPen().color());
	ui.sbMinorTicksWidth->setValue(Worksheet::convertFromSceneUnits(m_axis->minorTicksPen().widthF(), Worksheet::Unit::Point));
	ui.sbMinorTicksLength->setValue(Worksheet::convertFromSceneUnits(m_axis->minorTicksLength(), Worksheet::Unit::Point));
	ui.sbMinorTicksOpacity->setValue(round(m_axis->minorTicksOpacity() * 100.0));

	// Extra ticks
	// TODO

	// Tick label
	ui.cbLabelsPosition->setCurrentIndex((int)m_axis->labelsPosition());
	ui.sbLabelsOffset->setValue(Worksheet::convertFromSceneUnits(m_axis->labelsOffset(), Worksheet::Unit::Point));
	ui.sbLabelsRotation->setValue(m_axis->labelsRotationAngle());
	ui.cbLabelsTextType->setCurrentIndex((int)m_axis->labelsTextType());
	ui.cbLabelsFormat->setCurrentIndex(Axis::labelsFormatToIndex(m_axis->labelsFormat()));
	ui.cbLabelsFormat->setEnabled(!m_axis->labelsFormatAuto());
	ui.chkLabelsFormatAuto->setChecked(m_axis->labelsFormatAuto());
	ui.chkLabelsAutoPrecision->setChecked((int)m_axis->labelsAutoPrecision());
	ui.sbLabelsPrecision->setValue((int)m_axis->labelsPrecision());
	ui.cbLabelsDateTimeFormat->setCurrentText(m_axis->labelsDateTimeFormat());

	// we need to set the font size in points for KFontRequester
	QFont font = m_axis->labelsFont();
	font.setPointSizeF(round(Worksheet::convertFromSceneUnits(font.pixelSize(), Worksheet::Unit::Point)));
	ui.kfrLabelsFont->setFont(font);
	ui.kcbLabelsFontColor->setColor(m_axis->labelsColor());
	ui.cbLabelsBackgroundType->setCurrentIndex((int)m_axis->labelsBackgroundType());
	ui.kcbLabelsBackgroundColor->setColor(m_axis->labelsBackgroundColor());
	ui.leLabelsPrefix->setText(m_axis->labelsPrefix());
	ui.leLabelsSuffix->setText(m_axis->labelsSuffix());
	ui.sbLabelsOpacity->setValue(round(m_axis->labelsOpacity() * 100.0));

	majorTicksTypeChanged(ui.cbMajorTicksType->currentIndex());
	GuiTools::updatePenStyles(ui.cbMajorTicksLineStyle, ui.kcbMajorTicksColor->color());
	minorTicksTypeChanged(ui.cbMinorTicksType->currentIndex());
	GuiTools::updatePenStyles(ui.cbMinorTicksLineStyle, ui.kcbMinorTicksColor->color());
	labelsTextTypeChanged(ui.cbLabelsTextType->currentIndex());
	labelsTextColumnChanged(cbLabelsTextColumn->currentModelIndex());
}

void AxisDock::loadConfigFromTemplate(KConfig& config) {
	// extract the name of the template from the file name
	QString name;
	int index = config.name().lastIndexOf(QLatin1Char('/'));
	if (index != -1)
		name = config.name().right(config.name().size() - index - 1);
	else
		name = config.name();

	int size = m_axesList.size();
	if (size > 1)
		m_axis->beginMacro(i18n("%1 axes: template \"%2\" loaded", size, name));
	else
		m_axis->beginMacro(i18n("%1: template \"%2\" loaded", m_axis->name(), name));

	this->loadConfig(config);

	m_axis->endMacro();
}

void AxisDock::loadConfig(KConfig& config) {
	KConfigGroup group = config.group("Axis");

	// General
	ui.cbOrientation->setCurrentIndex(group.readEntry("Orientation", (int)m_axis->orientation()));

	int index = group.readEntry("Position", (int)m_axis->position());
	if (index > 1)
		ui.cbPosition->setCurrentIndex(index - 2);
	else
		ui.cbPosition->setCurrentIndex(index);

	SET_NUMBER_LOCALE
	ui.sbPositionLogical->setValue(group.readEntry("LogicalPosition", m_axis->logicalPosition()));
	ui.sbPosition->setValue(Worksheet::convertFromSceneUnits(group.readEntry("PositionOffset", m_axis->offset()), m_worksheetUnit));
	ui.cbScale->setCurrentIndex(group.readEntry("Scale", static_cast<int>(m_axis->scale())));
	ui.cbRangeType->setCurrentIndex(group.readEntry("RangeType", static_cast<int>(m_axis->rangeType())));
	ui.leStart->setText(numberLocale.toString(group.readEntry("Start", m_axis->range().start())));
	ui.leEnd->setText(numberLocale.toString(group.readEntry("End", m_axis->range().end())));
	ui.sbZeroOffset->setValue(group.readEntry("ZeroOffset", m_axis->zeroOffset()));
	ui.sbScalingFactor->setValue(group.readEntry("ScalingFactor", m_axis->scalingFactor()));
	ui.chkShowScaleOffset->setChecked(group.readEntry("ShowScaleOffset", static_cast<int>(m_axis->showScaleOffset())));

	// Title
	KConfigGroup axisLabelGroup = config.group("AxisLabel");
	labelWidget->loadConfig(axisLabelGroup);

	// Line
	lineWidget->loadConfig(group);
	ui.cbArrowType->setCurrentIndex(group.readEntry("ArrowType", (int)m_axis->arrowType()));
	ui.cbArrowPosition->setCurrentIndex(group.readEntry("ArrowPosition", (int)m_axis->arrowPosition()));
	ui.sbArrowSize->setValue(Worksheet::convertFromSceneUnits(group.readEntry("ArrowSize", m_axis->arrowSize()), Worksheet::Unit::Point));

	// Major ticks
	ui.cbMajorTicksDirection->setCurrentIndex(group.readEntry("MajorTicksDirection", (int)m_axis->majorTicksDirection()));
	ui.cbMajorTicksType->setCurrentIndex(group.readEntry("MajorTicksType", (int)m_axis->majorTicksType()));
	ui.sbMajorTicksNumber->setValue(group.readEntry("MajorTicksNumber", m_axis->majorTicksNumber()));
	auto value{group.readEntry("MajorTicksIncrement", m_axis->majorTicksSpacing())};
	bool numeric = m_axis->isNumeric();
	if (numeric) {
		ui.sbMajorTicksSpacingNumeric->setDecimals(nsl_math_decimal_places(value) + 1);
		ui.sbMajorTicksSpacingNumeric->setValue(value);
		ui.sbMajorTicksSpacingNumeric->setSingleStep(value / 10.);
	} else
		dtsbMajorTicksIncrement->setValue(value);
	ui.cbMajorTicksStartType->setCurrentIndex(group.readEntry("MajorTicksStartType", (int)m_axis->majorTicksStartType()));
	ui.leMajorTickStartOffset->setText(numberLocale.toString(group.readEntry("MajorTickStartOffset", m_axis->majorTickStartOffset())));
	ui.leMajorTickStartValue->setText(numberLocale.toString(group.readEntry("MajorTickStartValue", m_axis->majorTickStartValue())));
	ui.cbMajorTicksLineStyle->setCurrentIndex(group.readEntry("MajorTicksLineStyle", (int)m_axis->majorTicksPen().style()));
	ui.kcbMajorTicksColor->setColor(group.readEntry("MajorTicksColor", m_axis->majorTicksPen().color()));
	ui.sbMajorTicksWidth->setValue(
		Worksheet::convertFromSceneUnits(group.readEntry("MajorTicksWidth", m_axis->majorTicksPen().widthF()), Worksheet::Unit::Point));
	ui.sbMajorTicksLength->setValue(Worksheet::convertFromSceneUnits(group.readEntry("MajorTicksLength", m_axis->majorTicksLength()), Worksheet::Unit::Point));
	ui.sbMajorTicksOpacity->setValue(round(group.readEntry("MajorTicksOpacity", m_axis->majorTicksOpacity()) * 100.0));

	// Minor ticks
	ui.cbMinorTicksDirection->setCurrentIndex(group.readEntry("MinorTicksDirection", (int)m_axis->minorTicksDirection()));
	ui.cbMinorTicksType->setCurrentIndex(group.readEntry("MinorTicksType", (int)m_axis->minorTicksType()));
	ui.sbMinorTicksNumber->setValue(group.readEntry("MinorTicksNumber", m_axis->minorTicksNumber()));
	value = group.readEntry("MinorTicksIncrement", m_axis->minorTicksSpacing());
	if (numeric)
		ui.sbMinorTicksSpacingNumeric->setValue(value);
	else
		dtsbMinorTicksIncrement->setValue(value);
	ui.cbMinorTicksLineStyle->setCurrentIndex(group.readEntry("MinorTicksLineStyle", (int)m_axis->minorTicksPen().style()));
	ui.kcbMinorTicksColor->setColor(group.readEntry("MinorTicksColor", m_axis->minorTicksPen().color()));
	ui.sbMinorTicksWidth->setValue(
		Worksheet::convertFromSceneUnits(group.readEntry("MinorTicksWidth", m_axis->minorTicksPen().widthF()), Worksheet::Unit::Point));
	ui.sbMinorTicksLength->setValue(Worksheet::convertFromSceneUnits(group.readEntry("MinorTicksLength", m_axis->minorTicksLength()), Worksheet::Unit::Point));
	ui.sbMinorTicksOpacity->setValue(round(group.readEntry("MinorTicksOpacity", m_axis->minorTicksOpacity()) * 100.0));

	// Extra ticks
	// TODO

	// Tick label
	ui.cbLabelsFormat->setCurrentIndex(
		Axis::labelsFormatToIndex((Axis::LabelsFormat)(group.readEntry("LabelsFormat", Axis::labelsFormatToIndex(m_axis->labelsFormat())))));
	ui.chkLabelsAutoPrecision->setChecked(group.readEntry("LabelsAutoPrecision", (int)m_axis->labelsAutoPrecision()));
	ui.sbLabelsPrecision->setValue(group.readEntry("LabelsPrecision", (int)m_axis->labelsPrecision()));
	ui.cbLabelsDateTimeFormat->setCurrentText(group.readEntry("LabelsDateTimeFormat", "yyyy-MM-dd hh:mm:ss"));
	ui.cbLabelsPosition->setCurrentIndex(group.readEntry("LabelsPosition", (int)m_axis->labelsPosition()));
	ui.sbLabelsOffset->setValue(Worksheet::convertFromSceneUnits(group.readEntry("LabelsOffset", m_axis->labelsOffset()), Worksheet::Unit::Point));
	ui.sbLabelsRotation->setValue(group.readEntry("LabelsRotation", m_axis->labelsRotationAngle()));
	ui.cbLabelsTextType->setCurrentIndex(group.readEntry("LabelsTextType", (int)m_axis->labelsTextType()));

	// we need to set the font size in points for KFontRequester
	QFont font = m_axis->labelsFont();
	font.setPointSizeF(round(Worksheet::convertFromSceneUnits(font.pixelSize(), Worksheet::Unit::Point)));
	ui.kfrLabelsFont->setFont(group.readEntry("LabelsFont", font));

	ui.kcbLabelsFontColor->setColor(group.readEntry("LabelsFontColor", m_axis->labelsColor()));
	ui.cbLabelsBackgroundType->setCurrentIndex(group.readEntry("LabelsBackgroundType", (int)m_axis->labelsBackgroundType()));
	ui.kcbLabelsBackgroundColor->setColor(group.readEntry("LabelsBackgroundColor", m_axis->labelsBackgroundColor()));
	ui.leLabelsPrefix->setText(group.readEntry("LabelsPrefix", m_axis->labelsPrefix()));
	ui.leLabelsSuffix->setText(group.readEntry("LabelsSuffix", m_axis->labelsSuffix()));
	ui.sbLabelsOpacity->setValue(round(group.readEntry("LabelsOpacity", m_axis->labelsOpacity()) * 100.0));

	// Grid
	majorGridLineWidget->loadConfig(group);
	minorGridLineWidget->loadConfig(group);

	m_initializing = true;
	this->majorTicksTypeChanged(ui.cbMajorTicksType->currentIndex());
	GuiTools::updatePenStyles(ui.cbMajorTicksLineStyle, ui.kcbMajorTicksColor->color());
	this->minorTicksTypeChanged(ui.cbMinorTicksType->currentIndex());
	GuiTools::updatePenStyles(ui.cbMinorTicksLineStyle, ui.kcbMinorTicksColor->color());
	m_initializing = false;
}

void AxisDock::saveConfigAsTemplate(KConfig& config) {
	KConfigGroup group = config.group("Axis");

	// General
	group.writeEntry("Orientation", ui.cbOrientation->currentIndex());

	if (ui.cbPosition->currentIndex() == 2) {
		group.writeEntry("Position", static_cast<int>(Axis::Position::Centered));
	} else if (ui.cbPosition->currentIndex() == 3) {
		group.writeEntry("Position", static_cast<int>(Axis::Position::Centered));
	} else {
		if (ui.cbOrientation->currentIndex() == static_cast<int>(Axis::Orientation::Horizontal))
			group.writeEntry("Position", ui.cbPosition->currentIndex());
		else
			group.writeEntry("Position", ui.cbPosition->currentIndex() + 2);
	}

	SET_NUMBER_LOCALE
	group.writeEntry("LogicalPosition", ui.sbPositionLogical->value());
	group.writeEntry("PositionOffset", Worksheet::convertToSceneUnits(ui.sbPosition->value(), m_worksheetUnit));
	group.writeEntry("Scale", ui.cbScale->currentIndex());
	group.writeEntry("RangeType", ui.cbRangeType->currentIndex());
	group.writeEntry("Start", numberLocale.toDouble(ui.leStart->text()));
	group.writeEntry("End", numberLocale.toDouble(ui.leEnd->text()));
	group.writeEntry("ZeroOffset", ui.sbZeroOffset->value());
	group.writeEntry("ScalingFactor", ui.sbScalingFactor->value());
	group.writeEntry("ShowScaleOffset", ui.chkShowScaleOffset->isChecked());

	// Title
	KConfigGroup axisLabelGroup = config.group("AxisLabel");
	labelWidget->saveConfig(axisLabelGroup);

	// Line
	lineWidget->saveConfig(group);

	// Major ticks
	group.writeEntry("MajorTicksDirection", ui.cbMajorTicksDirection->currentIndex());
	group.writeEntry("MajorTicksType", ui.cbMajorTicksType->currentIndex());
	group.writeEntry("MajorTicksNumber", ui.sbMajorTicksNumber->value());
	bool numeric = m_axis->isNumeric();
	if (numeric)
		group.writeEntry("MajorTicksIncrement", QString::number(ui.sbMajorTicksSpacingNumeric->value()));
	else
		group.writeEntry("MajorTicksIncrement", QString::number(dtsbMajorTicksIncrement->value()));
	group.writeEntry("MajorTicksStartType", ui.cbMajorTicksStartType->currentIndex());
	group.writeEntry("MajorTickStartOffset", numberLocale.toDouble(ui.leMajorTickStartOffset->text()));
	group.writeEntry("MajorTickStartValue", numberLocale.toDouble(ui.leMajorTickStartValue->text()));
	group.writeEntry("MajorTicksLineStyle", ui.cbMajorTicksLineStyle->currentIndex());
	group.writeEntry("MajorTicksColor", ui.kcbMajorTicksColor->color());
	group.writeEntry("MajorTicksWidth", Worksheet::convertToSceneUnits(ui.sbMajorTicksWidth->value(), Worksheet::Unit::Point));
	group.writeEntry("MajorTicksLength", Worksheet::convertToSceneUnits(ui.sbMajorTicksLength->value(), Worksheet::Unit::Point));
	group.writeEntry("MajorTicksOpacity", ui.sbMajorTicksOpacity->value() / 100.);

	// Minor ticks
	group.writeEntry("MinorTicksDirection", ui.cbMinorTicksDirection->currentIndex());
	group.writeEntry("MinorTicksType", ui.cbMinorTicksType->currentIndex());
	group.writeEntry("MinorTicksNumber", ui.sbMinorTicksNumber->value());
	if (numeric)
		group.writeEntry("MinorTicksIncrement", QString::number(ui.sbMinorTicksSpacingNumeric->value()));
	else
		group.writeEntry("MinorTicksIncrement", QString::number(dtsbMinorTicksIncrement->value()));
	group.writeEntry("MinorTicksLineStyle", ui.cbMinorTicksLineStyle->currentIndex());
	group.writeEntry("MinorTicksColor", ui.kcbMinorTicksColor->color());
	group.writeEntry("MinorTicksWidth", Worksheet::convertFromSceneUnits(ui.sbMinorTicksWidth->value(), Worksheet::Unit::Point));
	group.writeEntry("MinorTicksLength", Worksheet::convertFromSceneUnits(ui.sbMinorTicksLength->value(), Worksheet::Unit::Point));
	group.writeEntry("MinorTicksOpacity", ui.sbMinorTicksOpacity->value() / 100.);

	// Extra ticks
	//  TODO

	// Tick label
	group.writeEntry("LabelsFormat", static_cast<int>(Axis::indexToLabelsFormat(ui.cbLabelsFormat->currentIndex())));
	group.writeEntry("LabelsAutoPrecision", ui.chkLabelsAutoPrecision->isChecked());
	group.writeEntry("LabelsPrecision", ui.sbLabelsPrecision->value());
	group.writeEntry("LabelsPosition", ui.cbLabelsPosition->currentIndex());
	group.writeEntry("LabelsOffset", Worksheet::convertToSceneUnits(ui.sbLabelsOffset->value(), Worksheet::Unit::Point));
	group.writeEntry("LabelsRotation", ui.sbLabelsRotation->value());
	group.writeEntry("LabelsFont", ui.kfrLabelsFont->font());
	group.writeEntry("LabelsFontColor", ui.kcbLabelsFontColor->color());
	group.writeEntry("LabelsBackgroundType", ui.cbLabelsBackgroundType->currentIndex());
	group.writeEntry("LabelsBackgroundColor", ui.kcbLabelsBackgroundColor->color());
	group.writeEntry("LabelsPrefix", ui.leLabelsPrefix->text());
	group.writeEntry("LabelsSuffix", ui.leLabelsSuffix->text());
	group.writeEntry("LabelsOpacity", ui.sbLabelsOpacity->value() / 100.);

	// Grid
	majorGridLineWidget->saveConfig(group);
	minorGridLineWidget->saveConfig(group);

	config.sync();
}
