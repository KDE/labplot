/***************************************************************************
File                 : AxisDock.cpp
Project              : LabPlot
Description          : axes widget class
--------------------------------------------------------------------
Copyright            : (C) 2011-2020 Alexander Semke (alexander.semke@web.de)
Copyright            : (C) 2012-2021 Stefan Gerlach (stefan.gerlach@uni-konstanz.de)

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

#include "AxisDock.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/core/column/Column.h"
#include "backend/core/Project.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "commonfrontend/widgets/TreeViewComboBox.h"
#include "kdefrontend/GuiTools.h"
#include "kdefrontend/TemplateHandler.h"
#include "kdefrontend/widgets/LabelWidget.h"
#include "commonfrontend/widgets/DateTimeSpinBox.h"

#include <KLocalizedString>
#include <KMessageBox>
#include <KLineEdit>

#include <QTimer>
#include <QDir>
#include <QPainter>

extern "C" {
#include <gsl/gsl_math.h>
#include "backend/nsl/nsl_math.h"
}

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

 \ingroup kdefrontend
*/

AxisDock::AxisDock(QWidget* parent) : BaseDock(parent) {
	ui.setupUi(this);
	m_leName = ui.leName;
	m_leComment = ui.leComment;

	//"Title"-tab
	auto* hboxLayout = new QHBoxLayout(ui.tabTitle);
	labelWidget = new LabelWidget(ui.tabTitle);
	labelWidget->setFixedLabelMode(true);
	hboxLayout->addWidget(labelWidget);
	hboxLayout->setContentsMargins(2,2,2,2);
	hboxLayout->setSpacing(2);

	//"Ticks"-tab
	auto* layout = static_cast<QGridLayout*>(ui.tabTicks->layout());
	cbMajorTicksColumn = new TreeViewComboBox(ui.tabTicks);
	layout->addWidget(cbMajorTicksColumn, 7, 2);
	cbLabelsTextColumn = new TreeViewComboBox(ui.tabTicks);
	layout->addWidget(cbLabelsTextColumn, 9, 2);
	cbMinorTicksColumn = new TreeViewComboBox(ui.tabTicks);
	layout->addWidget(cbMinorTicksColumn, 21, 2);
	dtsbMajorTicksIncrement = new DateTimeSpinBox(ui.tabTicks);
	layout->addWidget(dtsbMajorTicksIncrement, 6, 2);
	dtsbMinorTicksIncrement = new DateTimeSpinBox(ui.tabTicks);
	layout->addWidget(dtsbMinorTicksIncrement, 20, 2);

	//"Labels"-tab
	layout = static_cast<QGridLayout*>(ui.tabLabels->layout());

	//adjust layouts in the tabs
	for (int i = 0; i < ui.tabWidget->count(); ++i) {
		layout = dynamic_cast<QGridLayout*>(ui.tabWidget->widget(i)->layout());
		if (!layout)
			continue;

		layout->setContentsMargins(2,2,2,2);
		layout->setHorizontalSpacing(2);
		layout->setVerticalSpacing(2);
	}

	//set the current locale
	SET_NUMBER_LOCALE
	ui.sbLineWidth->setLocale(numberLocale);
	ui.sbMajorTicksSpacingNumeric->setLocale(numberLocale);
	ui.sbMajorTicksWidth->setLocale(numberLocale);
	ui.sbMajorTicksLength->setLocale(numberLocale);
	ui.sbMinorTicksSpacingNumeric->setLocale(numberLocale);
	ui.sbMinorTicksWidth->setLocale(numberLocale);
	ui.sbMinorTicksLength->setLocale(numberLocale);
	ui.sbLabelsOffset->setLocale(numberLocale);
	ui.sbMajorGridWidth->setLocale(numberLocale);
	ui.sbMinorGridWidth->setLocale(numberLocale);
	labelWidget->updateLocale();

	//**********************************  Slots **********************************************

	//"General"-tab
	connect(ui.leName, &QLineEdit::textChanged, this, &AxisDock::nameChanged);
	connect(ui.leComment, &QLineEdit::textChanged, this, &AxisDock::commentChanged);
	connect(ui.chkVisible, &QCheckBox::clicked, this, &AxisDock::visibilityChanged);

	connect(ui.cbOrientation, QOverload<int>::of(&QComboBox::currentIndexChanged),
	        this, &AxisDock::orientationChanged);
	connect(ui.cbPosition, QOverload<int>::of(&QComboBox::currentIndexChanged),
	        this, QOverload<int>::of(&AxisDock::positionChanged));
	connect(ui.sbPosition, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, QOverload<double>::of(&AxisDock::positionChanged));
	connect(ui.sbPositionLogical, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, QOverload<double>::of(&AxisDock::logicalPositionChanged));
	connect(ui.cbScale, QOverload<int>::of(&QComboBox::currentIndexChanged),
	        this, &AxisDock::scaleChanged);

	connect(ui.chkAutoScale, &QCheckBox::stateChanged, this, &AxisDock::autoScaleChanged);
	connect(ui.leStart, &QLineEdit::textChanged, this, &AxisDock::startChanged);
	connect(ui.leEnd, &QLineEdit::textChanged, this, &AxisDock::endChanged);
	connect(ui.dateTimeEditStart, &QDateTimeEdit::dateTimeChanged, this, &AxisDock::startDateTimeChanged);
	connect(ui.dateTimeEditEnd, &QDateTimeEdit::dateTimeChanged, this, &AxisDock::endDateTimeChanged);
	connect(ui.leZeroOffset, &KLineEdit::textChanged, this, &AxisDock::zeroOffsetChanged);
	connect(ui.leScalingFactor, &KLineEdit::textChanged, this, &AxisDock::scalingFactorChanged);
	connect(ui.cbPlotRanges, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AxisDock::plotRangeChanged);

	//"Line"-tab
	connect(ui.cbLineStyle, QOverload<int>::of(&QComboBox::currentIndexChanged),
	        this, &AxisDock::lineStyleChanged);
	connect(ui.kcbLineColor, &KColorButton::changed, this, &AxisDock::lineColorChanged);
	connect(ui.sbLineWidth, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
	        this, &AxisDock::lineWidthChanged);
	connect(ui.sbLineOpacity, QOverload<int>::of(&QSpinBox::valueChanged),
	        this, &AxisDock::lineOpacityChanged);
	connect(ui.cbArrowPosition, QOverload<int>::of(&QComboBox::currentIndexChanged),
	        this, &AxisDock::arrowPositionChanged);
	connect(ui.cbArrowType, QOverload<int>::of(&QComboBox::currentIndexChanged),
	        this, &AxisDock::arrowTypeChanged);
	connect(ui.sbArrowSize, QOverload<int>::of(&QSpinBox::valueChanged),
	        this, &AxisDock::arrowSizeChanged);

	//"Major ticks"-tab
	connect(ui.cbMajorTicksDirection, QOverload<int>::of(&QComboBox::currentIndexChanged),
	        this, &AxisDock::majorTicksDirectionChanged);
	connect(ui.cbMajorTicksType, QOverload<int>::of(&QComboBox::currentIndexChanged),
	        this, &AxisDock::majorTicksTypeChanged);
	connect(ui.sbMajorTicksNumber, QOverload<int>::of(&QSpinBox::valueChanged),
	        this, &AxisDock::majorTicksNumberChanged);
	connect(ui.sbMajorTicksSpacingNumeric, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
	        this, &AxisDock::majorTicksSpacingChanged);
	connect(dtsbMajorTicksIncrement, &DateTimeSpinBox::valueChanged,
	        this, &AxisDock::majorTicksSpacingChanged);
	connect(cbMajorTicksColumn, &TreeViewComboBox::currentModelIndexChanged,
	        this, &AxisDock::majorTicksColumnChanged);
	connect(ui.cbMajorTicksLineStyle, QOverload<int>::of(&QComboBox::currentIndexChanged),
	        this, &AxisDock::majorTicksLineStyleChanged);
	connect(ui.kcbMajorTicksColor, &KColorButton::changed, this, &AxisDock::majorTicksColorChanged);
	connect(ui.sbMajorTicksWidth, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
	        this, &AxisDock::majorTicksWidthChanged);
	connect(ui.sbMajorTicksLength, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
	        this, &AxisDock::majorTicksLengthChanged);
	connect(ui.sbMajorTicksOpacity, QOverload<int>::of(&QSpinBox::valueChanged),
	        this, &AxisDock::majorTicksOpacityChanged);

	//"Minor ticks"-tab
	connect(ui.cbMinorTicksDirection, QOverload<int>::of(&QComboBox::currentIndexChanged),
	        this, &AxisDock::minorTicksDirectionChanged);
	connect(ui.cbMinorTicksType, QOverload<int>::of(&QComboBox::currentIndexChanged),
	        this, &AxisDock::minorTicksTypeChanged);
	connect(ui.sbMinorTicksNumber, QOverload<int>::of(&QSpinBox::valueChanged),
	        this, &AxisDock::minorTicksNumberChanged);
	connect(ui.sbMinorTicksSpacingNumeric, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
	        this, &AxisDock::minorTicksSpacingChanged);
	connect(dtsbMinorTicksIncrement, &DateTimeSpinBox::valueChanged,
	        this, &AxisDock::minorTicksSpacingChanged);
	connect(cbMinorTicksColumn, &TreeViewComboBox::currentModelIndexChanged,
	        this, &AxisDock::minorTicksColumnChanged);
	connect(ui.cbMinorTicksLineStyle, QOverload<int>::of(&QComboBox::currentIndexChanged),
	        this, &AxisDock::minorTicksLineStyleChanged);
	connect(ui.kcbMinorTicksColor, &KColorButton::changed, this, &AxisDock::minorTicksColorChanged);
	connect(ui.sbMinorTicksWidth, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
	        this, &AxisDock::minorTicksWidthChanged);
	connect(ui.sbMinorTicksLength, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
	        this, &AxisDock::minorTicksLengthChanged);
	connect(ui.sbMinorTicksOpacity, QOverload<int>::of(&QSpinBox::valueChanged),
	        this, &AxisDock::minorTicksOpacityChanged);

	//"Extra ticks"-tab

	//"Tick labels"-tab
	connect(ui.cbLabelsFormat, QOverload<int>::of(&QComboBox::currentIndexChanged),
	        this, &AxisDock::labelsFormatChanged);
	connect(ui.sbLabelsPrecision, QOverload<int>::of(&QSpinBox::valueChanged),
	        this, &AxisDock::labelsPrecisionChanged);
	connect(ui.chkLabelsAutoPrecision, &QCheckBox::stateChanged, this, &AxisDock::labelsAutoPrecisionChanged);
	connect(ui.cbLabelsDateTimeFormat, QOverload<int>::of(&QComboBox::currentIndexChanged),
	        this, &AxisDock::labelsDateTimeFormatChanged);
	connect(ui.cbLabelsDateTimeFormat, &QComboBox::currentTextChanged,
	        this, &AxisDock::labelsDateTimeFormatChanged);
	connect(ui.cbLabelsPosition, QOverload<int>::of(&QComboBox::currentIndexChanged),
	        this, &AxisDock::labelsPositionChanged);
	connect(ui.sbLabelsOffset, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
	        this, &AxisDock::labelsOffsetChanged);
	connect(ui.sbLabelsRotation, QOverload<int>::of(&QSpinBox::valueChanged),
	        this, &AxisDock::labelsRotationChanged);
	connect(ui.cbLabelsTextType, QOverload<int>::of(&QComboBox::currentIndexChanged),
	        this, &AxisDock::labelsTextTypeChanged);
	connect(cbLabelsTextColumn, &TreeViewComboBox::currentModelIndexChanged,
	        this, &AxisDock::labelsTextColumnChanged);
	connect(ui.kfrLabelsFont, &KFontRequester::fontSelected, this, &AxisDock::labelsFontChanged);
	connect(ui.kcbLabelsFontColor, &KColorButton::changed, this, &AxisDock::labelsFontColorChanged);
	connect(ui.cbLabelsBackgroundType, QOverload<int>::of(&QComboBox::currentIndexChanged),
	        this, &AxisDock::labelsBackgroundTypeChanged);
	connect(ui.kcbLabelsBackgroundColor, &KColorButton::changed, this, &AxisDock::labelsBackgroundColorChanged);
	connect(ui.leLabelsPrefix, &QLineEdit::textChanged, this, &AxisDock::labelsPrefixChanged);
	connect(ui.leLabelsSuffix, &QLineEdit::textChanged, this, &AxisDock::labelsSuffixChanged);
	connect(ui.sbLabelsOpacity, QOverload<int>::of(&QSpinBox::valueChanged),
	        this, &AxisDock::labelsOpacityChanged);

	//"Grid"-tab
	connect(ui.cbMajorGridStyle, QOverload<int>::of(&QComboBox::currentIndexChanged),
	        this, &AxisDock::majorGridStyleChanged);
	connect(ui.kcbMajorGridColor, &KColorButton::changed, this, &AxisDock::majorGridColorChanged);
	connect(ui.sbMajorGridWidth, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
	        this, &AxisDock::majorGridWidthChanged);
	connect(ui.sbMajorGridOpacity, QOverload<int>::of(&QSpinBox::valueChanged),
	        this, &AxisDock::majorGridOpacityChanged);

	connect(ui.cbMinorGridStyle, QOverload<int>::of(&QComboBox::currentIndexChanged),
	        this, &AxisDock::minorGridStyleChanged);
	connect(ui.kcbMinorGridColor, &KColorButton::changed, this, &AxisDock::minorGridColorChanged);
	connect(ui.sbMinorGridWidth, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
	        this, &AxisDock::minorGridWidthChanged);
	connect(ui.sbMinorGridOpacity, QOverload<int>::of(&QSpinBox::valueChanged),
	        this, &AxisDock::minorGridOpacityChanged);

	//template handler
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

	//Validators
	ui.leStart->setValidator( new QDoubleValidator(ui.leStart) );
	ui.leEnd->setValidator( new QDoubleValidator(ui.leEnd) );
	ui.leZeroOffset->setValidator( new QDoubleValidator(ui.leZeroOffset) );
	ui.leScalingFactor->setValidator( new QDoubleValidator(ui.leScalingFactor) );

	//TODO move this stuff to retranslateUI()
	ui.cbPosition->addItem(i18n("Top")); // Left
	ui.cbPosition->addItem(i18n("Bottom")); // Right
	ui.cbPosition->addItem(i18n("Centered"));
	ui.cbPosition->addItem(i18n("Logical"));

	// scales
	for (const auto& name: RangeT::scaleNames)
		ui.cbScale->addItem(name);

	ui.cbOrientation->addItem( i18n("Horizontal") );
	ui.cbOrientation->addItem( i18n("Vertical") );

	//Arrows
	ui.cbArrowType->addItem( i18n("No arrow") );
	ui.cbArrowType->addItem( i18n("Simple, Small") );
	ui.cbArrowType->addItem( i18n("Simple, Big") );
	ui.cbArrowType->addItem( i18n("Filled, Small") );
	ui.cbArrowType->addItem( i18n("Filled, Big") );
	ui.cbArrowType->addItem( i18n("Semi-filled, Small") );
	ui.cbArrowType->addItem( i18n("Semi-filled, Big") );

	QPainter pa;
	pa.setPen( QPen(Qt::SolidPattern, 0) );
	QPixmap pm(20, 20);
	ui.cbArrowType->setIconSize( QSize(20,20) );

	//no arrow
	pm.fill(Qt::transparent);
	pa.begin( &pm );
	pa.setRenderHint(QPainter::Antialiasing);
	pa.setBrush(Qt::SolidPattern);
	pa.drawLine(3, 10, 17, 10);
	pa.end();
	ui.cbArrowType->setItemIcon(0, pm);

	//simple, small
	double cos_phi = cos(M_PI/6.);
	pm.fill(Qt::transparent);
	pa.begin( &pm );
	pa.setRenderHint(QPainter::Antialiasing);
	pa.drawLine(3, 10, 17, 10);
	pa.drawLine(17,10, 10, 10-5*cos_phi);
	pa.drawLine(17,10, 10, 10+5*cos_phi);
	pa.end();
	ui.cbArrowType->setItemIcon(1, pm);

	//simple, big
	pm.fill(Qt::transparent);
	pa.begin( &pm );
	pa.setRenderHint(QPainter::Antialiasing);
	pa.drawLine(3, 10, 17, 10);
	pa.drawLine(17,10, 10, 10-10*cos_phi);
	pa.drawLine(17,10, 10, 10+10*cos_phi);
	pa.end();
	ui.cbArrowType->setItemIcon(2, pm);

	//filled, small
	pm.fill(Qt::transparent);
	pa.begin( &pm );
	pa.setRenderHint(QPainter::Antialiasing);
	pa.setBrush(Qt::SolidPattern);
	pa.drawLine(3, 10, 17, 10);
	//TODO: use QPolygon?
	QPointF points3[3] = { QPointF(17, 10), QPointF(10, 10-4*cos_phi), QPointF(10, 10+4*cos_phi) };
	pa.drawPolygon(points3, 3);
	pa.end();
	ui.cbArrowType->setItemIcon(3, pm);

	//filled, big
	pm.fill(Qt::transparent);
	pa.begin( &pm );
	pa.setRenderHint(QPainter::Antialiasing);
	pa.setBrush(Qt::SolidPattern);
	pa.drawLine(3, 10, 17, 10);
	QPointF points4[3] = { QPointF(17, 10), QPointF(10, 10-10*cos_phi), QPointF(10, 10+10*cos_phi) };
	pa.drawPolygon(points4, 3);
	pa.end();
	ui.cbArrowType->setItemIcon(4, pm);

	//semi-filled, small
	pm.fill(Qt::transparent);
	pa.begin( &pm );
	pa.setRenderHint(QPainter::Antialiasing);
	pa.setBrush(Qt::SolidPattern);
	pa.drawLine(3, 10, 17, 10);
	QPointF points5[4] = { QPointF(17, 10), QPointF(10, 10-4*cos_phi), QPointF(13, 10), QPointF(10, 10+4*cos_phi) };
	pa.drawPolygon(points5, 4);
	pa.end();
	ui.cbArrowType->setItemIcon(5, pm);

	//semi-filled, big
	pm.fill(Qt::transparent);
	pa.begin( &pm );
	pa.setRenderHint(QPainter::Antialiasing);
	pa.setBrush(Qt::SolidPattern);
	pa.drawLine(3, 10, 17, 10);
	QPointF points6[4] = { QPointF(17, 10), QPointF(10, 10-10*cos_phi), QPointF(13, 10), QPointF(10, 10+10*cos_phi) };
	pa.drawPolygon(points6, 4);
	pa.end();
	ui.cbArrowType->setItemIcon(6, pm);

	ui.cbArrowPosition->addItem( i18n("Left") );
	ui.cbArrowPosition->addItem( i18n("Right") );
	ui.cbArrowPosition->addItem( i18n("Both") );

	ui.cbMajorTicksDirection->addItem( i18n("None") );
	ui.cbMajorTicksDirection->addItem( i18n("In") );
	ui.cbMajorTicksDirection->addItem( i18n("Out") );
	ui.cbMajorTicksDirection->addItem( i18n("In and Out") );

	ui.cbMajorTicksType->addItem( i18n("Number") );
	ui.cbMajorTicksType->addItem( i18n("Spacing") );
	ui.cbMajorTicksType->addItem( i18n("Custom column") );

	ui.cbMinorTicksDirection->addItem( i18n("None") );
	ui.cbMinorTicksDirection->addItem( i18n("In") );
	ui.cbMinorTicksDirection->addItem( i18n("Out") );
	ui.cbMinorTicksDirection->addItem( i18n("In and Out") );

	ui.cbMinorTicksType->addItem( i18n("Number") );
	ui.cbMinorTicksType->addItem( i18n("Spacing") );
	ui.cbMinorTicksType->addItem( i18n("Custom column") );

	GuiTools::updatePenStyles(ui.cbLineStyle, QColor(Qt::black));
	GuiTools::updatePenStyles(ui.cbMajorTicksLineStyle, QColor(Qt::black));
	GuiTools::updatePenStyles(ui.cbMinorTicksLineStyle, QColor(Qt::black));
	GuiTools::updatePenStyles(ui.cbMajorGridStyle, QColor(Qt::black));
	GuiTools::updatePenStyles(ui.cbMinorGridStyle, QColor(Qt::black));

	//labels
	ui.cbLabelsPosition->addItem(i18n("No labels"));
	ui.cbLabelsPosition->addItem(i18n("Top"));
	ui.cbLabelsPosition->addItem(i18n("Bottom"));
	ui.cbLabelsTextType->addItem(i18n("Position values"));
	ui.cbLabelsTextType->addItem(i18n("Custom column"));

	// see Axis::labelsFormatToIndex() and Axis::indexToLabelsFormat()
	ui.cbLabelsFormat->addItem( i18n("Decimal notation") );
	ui.cbLabelsFormat->addItem( i18n("Scientific notation") );
	ui.cbLabelsFormat->addItem( i18n("Scientific E notation") );
	ui.cbLabelsFormat->addItem( i18n("Powers of 10") );
	ui.cbLabelsFormat->addItem( i18n("Powers of 2") );
	ui.cbLabelsFormat->addItem( i18n("Powers of e") );
	ui.cbLabelsFormat->addItem( i18n("Multiples of π") );

	ui.cbLabelsDateTimeFormat->addItems(AbstractColumn::dateTimeFormats());

	ui.cbLabelsBackgroundType->addItem(i18n("Transparent"));
	ui.cbLabelsBackgroundType->addItem(i18n("Color"));
}

void AxisDock::setModel() {
	QList<AspectType> list{AspectType::Folder, AspectType::Workbook, AspectType::Spreadsheet,
	                       AspectType::CantorWorksheet, AspectType::Column};
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
	Lock lock(m_initializing);
	m_axesList = list;
	m_axis = list.first();
	m_aspect = list.first();
	Q_ASSERT(m_axis != nullptr);
	m_aspectTreeModel = new AspectTreeModel(m_axis->project());
	this->setModel();

	labelWidget->setAxes(list);

	//if there are more then one axis in the list, disable the tab "general"
	if (list.size() == 1) {
		ui.lName->setEnabled(true);
		ui.leName->setEnabled(true);
		ui.lComment->setEnabled(true);
		ui.leComment->setEnabled(true);
		ui.leName->setText(m_axis->name());
		ui.leComment->setText(m_axis->comment());
		this->setModelIndexFromColumn(cbMajorTicksColumn, m_axis->majorTicksColumn());
		this->setModelIndexFromColumn(cbMinorTicksColumn, m_axis->minorTicksColumn());
		this->setModelIndexFromColumn(cbLabelsTextColumn, m_axis->labelsTextColumn());
	} else {
		ui.lName->setEnabled(false);
		ui.leName->setEnabled(false);
		ui.lComment->setEnabled(false);
		ui.leComment->setEnabled(false);
		ui.leName->setText(QString());
		ui.leComment->setText(QString());
		cbMajorTicksColumn->setCurrentModelIndex(QModelIndex());
		cbMinorTicksColumn->setCurrentModelIndex(QModelIndex());
		cbLabelsTextColumn->setCurrentModelIndex(QModelIndex());
	}
	ui.leName->setStyleSheet("");
	ui.leName->setToolTip("");

	//show the properties of the first axis
	this->load();

	updatePlotRanges();

	// general
	connect(m_axis, &Axis::aspectDescriptionChanged, this, &AxisDock::axisDescriptionChanged);
	connect(m_axis, &Axis::orientationChanged, this, QOverload<Axis::Orientation>::of(&AxisDock::axisOrientationChanged));
	connect(m_axis, QOverload<Axis::Position>::of(&Axis::positionChanged),
	        this, QOverload<Axis::Position>::of(&AxisDock::axisPositionChanged));
	connect(m_axis, &Axis::scaleChanged, this, &AxisDock::axisScaleChanged);
	connect(m_axis, &Axis::autoScaleChanged, this, &AxisDock::axisAutoScaleChanged);
	connect(m_axis, &Axis::startChanged, this, &AxisDock::axisStartChanged);
	connect(m_axis, &Axis::endChanged, this, &AxisDock::axisEndChanged);
	connect(m_axis, &Axis::zeroOffsetChanged, this, &AxisDock::axisZeroOffsetChanged);
	connect(m_axis, &Axis::scalingFactorChanged, this, &AxisDock::axisScalingFactorChanged);
	connect(m_axis, &WorksheetElement::plotRangeListChanged, this, &AxisDock::updatePlotRanges);

	// line
	connect(m_axis, &Axis::linePenChanged, this, &AxisDock::axisLinePenChanged);
	connect(m_axis, &Axis::lineOpacityChanged, this, &AxisDock::axisLineOpacityChanged);
	connect(m_axis, &Axis::arrowTypeChanged, this, &AxisDock::axisArrowTypeChanged);
	connect(m_axis, &Axis::arrowPositionChanged, this, &AxisDock::axisArrowPositionChanged);
	connect(m_axis, &Axis::arrowSizeChanged, this, &AxisDock::axisArrowSizeChanged);

	// ticks
	connect(m_axis, &Axis::majorTicksDirectionChanged, this, &AxisDock::axisMajorTicksDirectionChanged);
	connect(m_axis, &Axis::majorTicksTypeChanged, this, &AxisDock::axisMajorTicksTypeChanged);
	connect(m_axis, &Axis::majorTicksNumberChanged, this, &AxisDock::axisMajorTicksNumberChanged);
	connect(m_axis, &Axis::majorTicksSpacingChanged, this, &AxisDock::axisMajorTicksSpacingChanged);
	connect(m_axis, &Axis::majorTicksColumnChanged, this, &AxisDock::axisMajorTicksColumnChanged);
	connect(m_axis, &Axis::majorTicksPenChanged, this, &AxisDock::axisMajorTicksPenChanged);
	connect(m_axis, &Axis::majorTicksLengthChanged, this, &AxisDock::axisMajorTicksLengthChanged);
	connect(m_axis, &Axis::majorTicksOpacityChanged, this, &AxisDock::axisMajorTicksOpacityChanged);
	connect(m_axis, &Axis::minorTicksDirectionChanged, this, &AxisDock::axisMinorTicksDirectionChanged);
	connect(m_axis, &Axis::minorTicksTypeChanged, this, &AxisDock::axisMinorTicksTypeChanged);
	connect(m_axis, &Axis::minorTicksNumberChanged, this, &AxisDock::axisMinorTicksNumberChanged);
	connect(m_axis, &Axis::minorTicksIncrementChanged, this, &AxisDock::axisMinorTicksSpacingChanged);
	connect(m_axis, &Axis::minorTicksColumnChanged, this, &AxisDock::axisMinorTicksColumnChanged);
	connect(m_axis, &Axis::minorTicksPenChanged, this, &AxisDock::axisMinorTicksPenChanged);
	connect(m_axis, &Axis::minorTicksLengthChanged, this, &AxisDock::axisMinorTicksLengthChanged);
	connect(m_axis, &Axis::minorTicksOpacityChanged, this, &AxisDock::axisMinorTicksOpacityChanged);

	// labels
	connect(m_axis, &Axis::labelsFormatChanged, this, &AxisDock::axisLabelsFormatChanged);
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

	// grids
	connect(m_axis, &Axis::majorGridPenChanged, this, &AxisDock::axisMajorGridPenChanged);
	connect(m_axis, &Axis::majorGridOpacityChanged, this, &AxisDock::axisMajorGridOpacityChanged);
	connect(m_axis, &Axis::minorGridPenChanged, this, &AxisDock::axisMinorGridPenChanged);
	connect(m_axis, &Axis::minorGridOpacityChanged, this, &AxisDock::axisMinorGridOpacityChanged);

	connect(m_axis, &Axis::visibilityChanged, this, &AxisDock::axisVisibilityChanged);

}

/*
 * updates the locale in the widgets. called when the application settins are changed.
 */
void AxisDock::updateLocale() {
	SET_NUMBER_LOCALE
	ui.sbLineWidth->setLocale(numberLocale);
	ui.sbMajorTicksSpacingNumeric->setLocale(numberLocale);
	ui.sbMajorTicksWidth->setLocale(numberLocale);
	ui.sbMajorTicksLength->setLocale(numberLocale);
	ui.sbMinorTicksSpacingNumeric->setLocale(numberLocale);
	ui.sbMinorTicksWidth->setLocale(numberLocale);
	ui.sbMinorTicksLength->setLocale(numberLocale);
	ui.sbLabelsOffset->setLocale(numberLocale);
	ui.sbMajorGridWidth->setLocale(numberLocale);
	ui.sbMinorGridWidth->setLocale(numberLocale);

	//update the QLineEdits, avoid the change events
	Lock lock(m_initializing);
	ui.sbPosition->setLocale(numberLocale);
	ui.leStart->setText(numberLocale.toString(m_axis->range().start()));
	ui.leEnd->setText(numberLocale.toString(m_axis->range().end()));

	// scales
	ui.cbScale->clear();
	for (const auto& name: RangeT::scaleNames)
		ui.cbScale->addItem(name);

	//update the title label
	labelWidget->updateLocale();
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

void AxisDock::updatePlotRanges() const {
	updatePlotRangeList(ui.cbPlotRanges);

	if (m_axis->coordinateSystemCount() == 0)
		return;

	Axis::Orientation orientation = m_axis->orientation();
	Range<double> logicalRange;
	if (orientation == Axis::Orientation::Horizontal)
		logicalRange = m_axis->plot()->yRange(m_axis->plot()->coordinateSystem(m_axis->coordinateSystemIndex())->yIndex());
	else
		logicalRange = m_axis->plot()->xRange(m_axis->plot()->coordinateSystem(m_axis->coordinateSystemIndex())->xIndex());
	spinBoxCalculateMinMax(ui.sbPositionLogical, logicalRange, ui.sbPositionLogical->value());
}

void AxisDock::updateAutoScale() {
	m_axis->setAutoScale(ui.chkAutoScale->isChecked());
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
		ui.cbPosition->setItemText(Top_Left, i18n("Top") );
		ui.cbPosition->setItemText(Bottom_Right, i18n("Bottom") );
		//ui.cbPosition->setItemText(Center, i18n("Center") ); // must not updated
		ui.cbLabelsPosition->setItemText(1, i18n("Top") );
		ui.cbLabelsPosition->setItemText(2, i18n("Bottom") );
	} else { //vertical
		ui.cbPosition->setItemText(Top_Left, i18n("Left") );
		ui.cbPosition->setItemText(Bottom_Right, i18n("Right") );
		//ui.cbPosition->setItemText(Center, i18n("Center") ); // must not updated
		ui.cbLabelsPosition->setItemText(1, i18n("Right") );
		ui.cbLabelsPosition->setItemText(2, i18n("Left") );
	}

	if (m_initializing)
		return;

	//depending on the current orientation we need to update axis position and labels position

	//axis position, map from the current index in the combobox to the enum value in Axis::Position
	Axis::Position axisPosition;
	int posIndex = ui.cbPosition->currentIndex();
	if (orientation == Axis::Orientation::Horizontal) {
		if (posIndex > 1)
			posIndex += 2;
		axisPosition = Axis::Position(posIndex);
	} else
		axisPosition = Axis::Position(posIndex+2);

	//labels position
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
		return;	//we occasionally get -1 here, nothing to do in this case

	if (m_initializing)
		return;

	//map from the current index in the combo box to the enum value in Axis::Position,
	//depends on the current orientation
	bool logical = false;
	Axis::Position position;
	if (index == Logical) {
		position = Axis::Position::Logical;
		logical = true;
	} else if (index == Center)
		position = Axis::Position::Centered;
	else if ( ui.cbOrientation->currentIndex() == 0 ) {
		// horizontal
		switch(index) {
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
		switch(index) {
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

void AxisDock::autoScaleChanged(int index) {
	DEBUG(Q_FUNC_INFO << ", index = " << index)
	bool autoScale = index == Qt::Checked;
	ui.leStart->setEnabled(!autoScale);
	ui.leEnd->setEnabled(!autoScale);
	ui.dateTimeEditStart->setEnabled(!autoScale);
	ui.dateTimeEditEnd->setEnabled(!autoScale);

	if (m_initializing)
		return;

	for (auto* axis : m_axesList)
		axis->setAutoScale(autoScale);

	updateLocale();	// update values
}

void AxisDock::startChanged() {
	if (m_initializing)
		return;

	bool ok;
	SET_NUMBER_LOCALE
	double value{numberLocale.toDouble(ui.leStart->text(), &ok)};
	if (!ok)
		return;

	//check first, whether the value for the lower limit is valid for the log- and square root scaling. If not, set the default values.
	const auto scale = RangeT::Scale(ui.cbScale->currentIndex());
	if (scale == RangeT::Scale::Log10 || scale == RangeT::Scale::Log2 || scale == RangeT::Scale::Ln) {
		if (value <= 0) {
			KMessageBox::sorry(this,
			                   i18n("The axes lower limit has a non-positive value. Default minimal value will be used."),
			                   i18n("Wrong lower limit value") );
			// TODO: why is this a good value?
			value = 0.01;
			ui.leStart->setText(numberLocale.toString(value) );
		}
	} else if (scale == RangeT::Scale::Sqrt) {
		if (value < 0) {
			KMessageBox::sorry(this,
			                   i18n("The axes lower limit has a negative value. Default minimal value will be used."),
			                   i18n("Wrong lower limit value") );
			value = 0;
			ui.leStart->setText(numberLocale.toString(value) );
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

	//TODO: also check for negative values and log-scales?

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

void AxisDock::zeroOffsetChanged() {
	DEBUG(Q_FUNC_INFO)
	if (m_initializing)
		return;

	if (ui.leZeroOffset->text().isEmpty())	// default value
		ui.leZeroOffset->setText("0");

	bool ok;
	SET_NUMBER_LOCALE
	const double offset{numberLocale.toDouble(ui.leZeroOffset->text(), &ok)};
	if (!ok)
		return;

	ui.leZeroOffset->setClearButtonEnabled(offset != 0);

	//TODO: check for negative values and log scales?

	const Lock lock(m_initializing);
	for (auto* axis : m_axesList)
		axis->setZeroOffset(offset);
}

void AxisDock::scalingFactorChanged() {
	if (m_initializing)
		return;

	if (ui.leScalingFactor->text().isEmpty())	// default value
		ui.leScalingFactor->setText("1");

	bool ok;
	SET_NUMBER_LOCALE
	const double scalingFactor{numberLocale.toDouble(ui.leScalingFactor->text(), &ok)};
	if (!ok)
		return;

	ui.leScalingFactor->setClearButtonEnabled(scalingFactor != 1);

	//TODO: check negative values and log-scales?

	if (scalingFactor != 0.0) {
		const Lock lock(m_initializing);
		for (auto* axis : m_axesList)
			axis->setScalingFactor(scalingFactor);
	}
}

// "Line"-tab
void AxisDock::lineStyleChanged(int index) {
	if (index == -1)
		return;

	auto penStyle = Qt::PenStyle(index);

	bool b = (penStyle != Qt::NoPen);
	ui.lLineColor->setEnabled(b);
	ui.kcbLineColor->setEnabled(b);
	ui.lLineWidth->setEnabled(b);
	ui.sbLineWidth->setEnabled(b);
	ui.lLineOpacity->setEnabled(b);
	ui.sbLineOpacity->setEnabled(b);

	if (m_initializing)
		return;

	QPen pen;
	for (auto* axis : m_axesList) {
		pen = axis->linePen();
		pen.setStyle(penStyle);
		axis->setLinePen(pen);
	}
}

void AxisDock::lineColorChanged(const QColor& color) {
	if (m_initializing)
		return;

	QPen pen;
	for (auto* axis : m_axesList) {
		pen = axis->linePen();
		pen.setColor(color);
		axis->setLinePen(pen);
	}

	m_initializing = true;
	GuiTools::updatePenStyles(ui.cbLineStyle, color);
	m_initializing = false;
}

void AxisDock::lineWidthChanged(double value) {
	if (m_initializing)
		return;

	QPen pen;
	for (auto* axis : m_axesList) {
		pen = axis->linePen();
		pen.setWidthF(Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point));
		axis->setLinePen(pen);
	}
}

void AxisDock::lineOpacityChanged(int value) {
	if (m_initializing)
		return;

	qreal opacity = (double)value/100.;
	for (auto* axis : m_axesList)
		axis->setLineOpacity(opacity);
}

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
	called if the current style of the ticks (Number or Increment) is changed.
	Shows/hides the corresponding widgets.
*/
void AxisDock::majorTicksTypeChanged(int index) {
	if (!m_axis) // If elements are added to the combobox 'cbMajorTicksType' (at init of this class), then this function is called, which is a problem if no axis are available
		return;

	auto type = Axis::TicksType(index);
	if (type == Axis::TicksType::TotalNumber) {
		ui.lMajorTicksNumber->show();
		ui.sbMajorTicksNumber->show();
		ui.lMajorTicksSpacingNumeric->hide();
		ui.sbMajorTicksSpacingNumeric->hide();
		ui.lMajorTicksIncrementDateTime->hide();
		dtsbMajorTicksIncrement->hide();
		ui.lMajorTicksColumn->hide();
		cbMajorTicksColumn->hide();
	} else if (type == Axis::TicksType::Spacing) {
		ui.lMajorTicksNumber->hide();
		ui.sbMajorTicksNumber->hide();
		ui.lMajorTicksSpacingNumeric->show();

		const auto* plot = static_cast<const CartesianPlot*>(m_axis->parentAspect());
		const auto* cSystem{ plot->coordinateSystem(m_axis->coordinateSystemIndex()) };
		const int xIndex{cSystem->xIndex()}, yIndex{cSystem->yIndex()};
		bool numeric = ( (m_axis->orientation() == Axis::Orientation::Horizontal && plot->xRangeFormat(xIndex) == RangeT::Format::Numeric)
		                 || (m_axis->orientation() == Axis::Orientation::Vertical && plot->yRangeFormat(yIndex) == RangeT::Format::Numeric) );
		if (numeric) {
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

		// Check if spacing is not to small
		majorTicksSpacingChanged();
	} else {
		ui.lMajorTicksNumber->hide();
		ui.sbMajorTicksNumber->hide();
		ui.lMajorTicksSpacingNumeric->hide();
		ui.sbMajorTicksSpacingNumeric->hide();
		ui.lMajorTicksIncrementDateTime->hide();
		dtsbMajorTicksIncrement->hide();
		ui.lMajorTicksColumn->show();
		cbMajorTicksColumn->show();
	}

	if (m_initializing)
		return;

	for (auto* axis : m_axesList)
		axis->setMajorTicksType(type);
}

void AxisDock::majorTicksNumberChanged(int value) {
	if (m_initializing)
		return;

	for (auto* axis : m_axesList)
		axis->setMajorTicksNumber(value);
}

void AxisDock::majorTicksSpacingChanged() {
	if (m_initializing)
		return;

	const auto* plot = static_cast<const CartesianPlot*>(m_axis->parentAspect());
	const auto* cSystem{ plot->coordinateSystem(m_axis->coordinateSystemIndex()) };
	const int xIndex{cSystem->xIndex()}, yIndex{cSystem->yIndex()};
	bool numeric = ( (m_axis->orientation() == Axis::Orientation::Horizontal && plot->xRangeFormat(xIndex) == RangeT::Format::Numeric)
	                 || (m_axis->orientation() == Axis::Orientation::Vertical && plot->yRangeFormat(yIndex) == RangeT::Format::Numeric) );

	double spacing = numeric ? ui.sbMajorTicksSpacingNumeric->value() : dtsbMajorTicksIncrement->value();
	double range = m_axis->range().length();
	DEBUG("major spacing = " << spacing << ", range = " << range)

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
		} else	//TODO: check reversed axis
			dtsbMajorTicksIncrement->setValue(spacing);
		m_initializing = false;
	}

	for (auto* axis : m_axesList)
		axis->setMajorTicksSpacing(spacing);
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
		pen.setWidthF( Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point) );
		axis->setMajorTicksPen(pen);
	}
}

void AxisDock::majorTicksLengthChanged(double value) {
	if (m_initializing)
		return;

	for (auto* axis : m_axesList)
		axis->setMajorTicksLength( Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point) );
}

void AxisDock::majorTicksOpacityChanged(int value) {
	if (m_initializing)
		return;

	qreal opacity{ value/100. };
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
	if (!m_axis) // If elements are added to the combobox 'cbMajorTicksType' (at init of this class), then this function is called, which is a problem if no axis are available
		return;

	auto type = Axis::TicksType(index);
	if (type == Axis::TicksType::TotalNumber) {
		ui.lMinorTicksNumber->show();
		ui.sbMinorTicksNumber->show();
		ui.lMinorTicksSpacingNumeric->hide();
		ui.sbMinorTicksSpacingNumeric->hide();
		ui.lMinorTicksColumn->hide();
		cbMinorTicksColumn->hide();
		ui.lMinorTicksIncrementDateTime->hide();
		dtsbMinorTicksIncrement->hide();
	} else if ( type == Axis::TicksType::Spacing) {
		ui.lMinorTicksNumber->hide();
		ui.sbMinorTicksNumber->hide();

		const auto* plot = static_cast<const CartesianPlot*>(m_axis->parentAspect());
		const auto* cSystem{ plot->coordinateSystem(m_axis->coordinateSystemIndex()) };
		const int xIndex{cSystem->xIndex()}, yIndex{cSystem->yIndex()};
		bool numeric = ( (m_axis->orientation() == Axis::Orientation::Horizontal && plot->xRangeFormat(xIndex) == RangeT::Format::Numeric)
		                 || (m_axis->orientation() == Axis::Orientation::Vertical && plot->yRangeFormat(yIndex) == RangeT::Format::Numeric) );
		if (numeric) {
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

void AxisDock::minorTicksNumberChanged(int value) {
	if (m_initializing)
		return;

	for (auto* axis : m_axesList)
		axis->setMinorTicksNumber(value);
}

void AxisDock::minorTicksSpacingChanged() {
	if (m_initializing)
		return;

	const auto* plot = static_cast<const CartesianPlot*>(m_axis->parentAspect());
	const auto* cSystem{ plot->coordinateSystem(m_axis->coordinateSystemIndex()) };
	const int xIndex{cSystem->xIndex()}, yIndex{cSystem->yIndex()};
	bool numeric = ( (m_axis->orientation() == Axis::Orientation::Horizontal && plot->xRangeFormat(xIndex) == RangeT::Format::Numeric)
	                 || (m_axis->orientation() == Axis::Orientation::Vertical && plot->yRangeFormat(yIndex) == RangeT::Format::Numeric) );

	double spacing = numeric ? ui.sbMinorTicksSpacingNumeric->value() : dtsbMinorTicksIncrement->value();
	double range = m_axis->range().length();
	//DEBUG("minor spacing = " << spacing << ", range = " << range)
	int numberTicks = 0;

	int majorTicks = m_axis->majorTicksNumber();
	if (spacing > 0.)
		numberTicks = range / (majorTicks - 1) / spacing - 1; // recalc
	//DEBUG("	nticks = " << numberTicks)

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
		pen.setWidthF( Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point) );
		axis->setMinorTicksPen(pen);
	}
}

void AxisDock::minorTicksLengthChanged(double value) {
	if (m_initializing)
		return;

	for (auto* axis : m_axesList)
		axis->setMinorTicksLength( Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point) );
}

void AxisDock::minorTicksOpacityChanged(int value) {
	if (m_initializing)
		return;

	qreal opacity{ value/100. };
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

void AxisDock::labelsPrecisionChanged(int value) {
	if (m_initializing)
		return;

	for (auto* axis : m_axesList)
		axis->setLabelsPrecision(value);
}


void AxisDock::labelsAutoPrecisionChanged(int state) {
	bool checked = (state == Qt::Checked);
	ui.sbLabelsPrecision->setEnabled(!checked);

	if (m_initializing)
		return;

	for (auto* axis : m_axesList)
		axis->setLabelsAutoPrecision(checked);
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
		axis->setLabelsOffset( Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point) );
}

void AxisDock::labelsRotationChanged(int value) {
	if (m_initializing)
		return;

	for (auto* axis : m_axesList)
		axis->setLabelsRotationAngle(value);
}

void AxisDock::labelsTextTypeChanged(int index) {
	if (!m_axis)
		return; //don't do anything when we're addItem()'ing strings and the axis is not available yet

	auto type = Axis::LabelsTextType(index);
	if (type == Axis::LabelsTextType::PositionValues) {
		ui.lLabelsTextColumn->hide();
		cbLabelsTextColumn->hide();

		//TODO: duplication of the code in load()
		const auto* plot = static_cast<const CartesianPlot*>(m_axis->parentAspect());
		const auto* cSystem{ plot->coordinateSystem(m_axis->coordinateSystemIndex()) };
		const int xIndex{cSystem->xIndex()}, yIndex{cSystem->yIndex()};
		bool numeric = ( (m_axis->orientation() == Axis::Orientation::Horizontal && plot->xRangeFormat(xIndex) == RangeT::Format::Numeric)
		                 || (m_axis->orientation() == Axis::Orientation::Vertical && plot->yRangeFormat(yIndex) == RangeT::Format::Numeric) );
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
		//depending on data format of the column (numeric vs. datetime vs. text),
		//show/hide the corresponding widgets for the tick labels format
		switch (column->columnMode()) {
		case AbstractColumn::ColumnMode::Numeric:
		case AbstractColumn::ColumnMode::Integer:
		case AbstractColumn::ColumnMode::BigInt:
			ui.lLabelsFormat->show();
			ui.cbLabelsFormat->show();
			ui.chkLabelsAutoPrecision->show();
			ui.lLabelsPrecision->show();
			ui.sbLabelsPrecision->show();
			ui.lLabelsDateTimeFormat->hide();
			ui.cbLabelsDateTimeFormat->hide();
			break;
		case AbstractColumn::ColumnMode::Text:
			ui.lLabelsFormat->hide();
			ui.cbLabelsFormat->hide();
			ui.chkLabelsAutoPrecision->hide();
			ui.lLabelsPrecision->hide();
			ui.sbLabelsPrecision->hide();
			ui.lLabelsDateTimeFormat->hide();
			ui.cbLabelsDateTimeFormat->hide();
			break;
		case AbstractColumn::ColumnMode::DateTime:
		case AbstractColumn::ColumnMode::Month:
		case AbstractColumn::ColumnMode::Day:
			ui.lLabelsFormat->hide();
			ui.cbLabelsFormat->hide();
			ui.chkLabelsAutoPrecision->hide();
			ui.lLabelsPrecision->hide();
			ui.sbLabelsPrecision->hide();
			ui.lLabelsDateTimeFormat->show();
			ui.cbLabelsDateTimeFormat->show();
			break;
		}
	} else {
		auto type = Axis::LabelsTextType(ui.cbLabelsTextType->currentIndex());
		if (type == Axis::LabelsTextType::CustomValues) {
			ui.lLabelsFormat->hide();
			ui.cbLabelsFormat->hide();
			ui.chkLabelsAutoPrecision->hide();
			ui.lLabelsPrecision->hide();
			ui.sbLabelsPrecision->hide();
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

	QString prefix = ui.leLabelsPrefix->text();
	for (auto* axis : m_axesList)
		axis->setLabelsPrefix(prefix);
}

void AxisDock::labelsSuffixChanged() {
	if (m_initializing)
		return;

	QString suffix = ui.leLabelsSuffix->text();
	for (auto* axis : m_axesList)
		axis->setLabelsSuffix(suffix);
}

void AxisDock::labelsFontChanged(const QFont& font) {
	if (m_initializing)
		return;

	QFont labelsFont = font;
	labelsFont.setPixelSize( Worksheet::convertToSceneUnits(font.pointSizeF(), Worksheet::Unit::Point) );
	for (auto* axis : m_axesList)
		axis->setLabelsFont( labelsFont );
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

	qreal opacity{ value/100. };
	for (auto* axis : m_axesList)
		axis->setLabelsOpacity(opacity);
}

// "Grid"-tab
//major grid
void AxisDock::majorGridStyleChanged(int index) {
	if (index == -1)
		return;

	auto penStyle = Qt::PenStyle(index);

	bool b = (penStyle != Qt::NoPen);
	ui.lMajorGridColor->setEnabled(b);
	ui.kcbMajorGridColor->setEnabled(b);
	ui.lMajorGridWidth->setEnabled(b);
	ui.sbMajorGridWidth->setEnabled(b);
	ui.lMajorGridOpacity->setEnabled(b);
	ui.sbMajorGridOpacity->setEnabled(b);

	if (m_initializing)
		return;

	QPen pen;
	for (auto* axis : m_axesList) {
		pen = axis->majorGridPen();
		pen.setStyle(penStyle);
		axis->setMajorGridPen(pen);
	}
}

void AxisDock::majorGridColorChanged(const QColor& color) {
	if (m_initializing)
		return;

	QPen pen;
	for (auto* axis : m_axesList) {
		pen = axis->majorGridPen();
		pen.setColor(color);
		axis->setMajorGridPen(pen);
	}

	m_initializing = true;
	GuiTools::updatePenStyles(ui.cbMajorGridStyle, color);
	m_initializing = false;
}

void AxisDock::majorGridWidthChanged(double  value) {
	if (m_initializing)
		return;

	QPen pen;
	for (auto* axis : m_axesList) {
		pen = axis->majorGridPen();
		pen.setWidthF(Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point));
		axis->setMajorGridPen(pen);
	}
}

void AxisDock::majorGridOpacityChanged(int value) {
	if (m_initializing)
		return;

	qreal opacity{ value/100. };
	for (auto* axis : m_axesList)
		axis->setMajorGridOpacity(opacity);
}

//minor grid
void AxisDock::minorGridStyleChanged(int index) {
	if (index == -1)
		return;

	auto penStyle = Qt::PenStyle(index);

	bool b = (penStyle != Qt::NoPen);
	ui.lMinorGridColor->setEnabled(b);
	ui.kcbMinorGridColor->setEnabled(b);
	ui.lMinorGridWidth->setEnabled(b);
	ui.sbMinorGridWidth->setEnabled(b);
	ui.lMinorGridOpacity->setEnabled(b);
	ui.sbMinorGridOpacity->setEnabled(b);

	if (m_initializing)
		return;

	QPen pen;
	for (auto* axis : m_axesList) {
		pen = axis->minorGridPen();
		pen.setStyle(penStyle);
		axis->setMinorGridPen(pen);
	}
}

void AxisDock::minorGridColorChanged(const QColor& color) {
	if (m_initializing)
		return;

	QPen pen;
	for (auto* axis : m_axesList) {
		pen = axis->minorGridPen();
		pen.setColor(color);
		axis->setMinorGridPen(pen);
	}

	m_initializing = true;
	GuiTools::updatePenStyles(ui.cbMinorGridStyle, color);
	m_initializing = false;
}

void AxisDock::minorGridWidthChanged(double  value) {
	if (m_initializing)
		return;

	QPen pen;
	for (auto* axis : m_axesList) {
		pen = axis->minorGridPen();
		pen.setWidthF(Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point));
		axis->setMinorGridPen(pen);
	}
}

void AxisDock::minorGridOpacityChanged(int value) {
	if (m_initializing)
		return;

	qreal opacity{ value/100. };
	for (auto* axis : m_axesList)
		axis->setMinorGridOpacity(opacity);
}

//*************************************************************
//************ SLOTs for changes triggered in Axis ************
//*************************************************************
void AxisDock::axisDescriptionChanged(const AbstractAspect* aspect) {
	if (m_axis != aspect)
		return;

	const Lock lock(m_initializing);
	if (aspect->name() != ui.leName->text())
		ui.leName->setText(aspect->name());
	else if (aspect->comment() != ui.leComment->text())
		ui.leComment->setText(aspect->comment());
}

void AxisDock::axisOrientationChanged(Axis::Orientation orientation) {
	m_initializing = true;
	ui.cbOrientation->setCurrentIndex(static_cast<int>(orientation));
	m_initializing = false;
}

void AxisDock::axisPositionChanged(Axis::Position position) {
	m_initializing = true;

	//map from the enum Qt::Orientation to the index in the combo box
	int index{static_cast<int>(position)};
	switch(index) {
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
	m_initializing = true;
	ui.cbScale->setCurrentIndex(static_cast<int>(scale));
	m_initializing = false;
}

void AxisDock::axisAutoScaleChanged(bool on) {
	m_initializing = true;
	ui.chkAutoScale->setChecked(on);
	m_initializing = false;
}

void AxisDock::axisStartChanged(double value) {
	if (m_initializing) return;
	const Lock lock(m_initializing);

	SET_NUMBER_LOCALE
	ui.leStart->setText(numberLocale.toString(value));
	ui.dateTimeEditStart->setDateTime( QDateTime::fromMSecsSinceEpoch(value) );

	// determine stepsize and number of decimals
	const double range{ m_axis->range().length() };
	const int decimals{ nsl_math_rounded_decimals(range) + 1 };
	DEBUG("range = " << range << ", decimals = " << decimals)
	ui.sbMajorTicksSpacingNumeric->setDecimals(decimals);
	ui.sbMajorTicksSpacingNumeric->setSingleStep(gsl_pow_int(10., -decimals));
	ui.sbMajorTicksSpacingNumeric->setMaximum(range);
}

void AxisDock::axisEndChanged(double value) {
	if (m_initializing) return;
	const Lock lock(m_initializing);

	SET_NUMBER_LOCALE
	ui.leEnd->setText(numberLocale.toString(value));
	ui.dateTimeEditEnd->setDateTime( QDateTime::fromMSecsSinceEpoch(value) );

	// determine stepsize and number of decimals
	const double range{ m_axis->range().length() };
	const int decimals{ nsl_math_rounded_decimals(range) + 1 };
	DEBUG("range = " << range << ", decimals = " << decimals)
	ui.sbMajorTicksSpacingNumeric->setDecimals(decimals);
	ui.sbMajorTicksSpacingNumeric->setSingleStep(gsl_pow_int(10., -decimals));
	ui.sbMajorTicksSpacingNumeric->setMaximum(range);
}

void AxisDock::axisZeroOffsetChanged(qreal value) {
	DEBUG(Q_FUNC_INFO)
	if (m_initializing) return;
	const Lock lock(m_initializing);
	SET_NUMBER_LOCALE
	ui.leZeroOffset->setText(numberLocale.toString(value));
}

void AxisDock::axisScalingFactorChanged(qreal value) {
	if (m_initializing) return;
	const Lock lock(m_initializing);
	SET_NUMBER_LOCALE
	ui.leScalingFactor->setText(numberLocale.toString(value));
}

//line
void AxisDock::axisLinePenChanged(const QPen& pen) {
	m_initializing = true;
	ui.cbLineStyle->setCurrentIndex( pen.style() );
	ui.kcbLineColor->setColor( pen.color() );
	GuiTools::updatePenStyles(ui.cbLineStyle, pen.color() );
	ui.sbLineWidth->setValue( Worksheet::convertFromSceneUnits(pen.widthF(), Worksheet::Unit::Point) );
	m_initializing = false;
}

void AxisDock::axisArrowTypeChanged(Axis::ArrowType type) {
	m_initializing = true;
	ui.cbArrowType->setCurrentIndex( static_cast<int>(type) );
	m_initializing = false;
}

void AxisDock::axisLineOpacityChanged(qreal opacity) {
	m_initializing = true;
	ui.sbLineOpacity->setValue( round(opacity*100.0) );
	m_initializing = false;
}

void AxisDock::axisArrowPositionChanged(Axis::ArrowPosition position) {
	m_initializing = true;
	ui.cbArrowPosition->setCurrentIndex( static_cast<int>(position) );
	m_initializing = false;
}

void AxisDock::axisArrowSizeChanged(qreal size) {
	m_initializing = true;
	ui.sbArrowSize->setValue( (int)Worksheet::convertFromSceneUnits(size, Worksheet::Unit::Point) );
	m_initializing = false;
}

//major ticks
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
void AxisDock::axisMajorTicksNumberChanged(int number) {
	m_initializing = true;
	ui.sbMajorTicksNumber->setValue(number);
	m_initializing = false;
}
void AxisDock::axisMajorTicksSpacingChanged(qreal increment) {
	Lock lock(m_initializing);
	const auto* plot = dynamic_cast<const CartesianPlot*>(m_axis->parentAspect());
	const auto* cSystem{ plot->coordinateSystem(m_axis->coordinateSystemIndex()) };
	const int xIndex{cSystem->xIndex()}, yIndex{cSystem->yIndex()};
	bool numeric = ( (m_axis->orientation() == Axis::Orientation::Horizontal && plot->xRangeFormat(xIndex) == RangeT::Format::Numeric)
	                 || (m_axis->orientation() == Axis::Orientation::Vertical && plot->yRangeFormat(yIndex) == RangeT::Format::Numeric) );

	if (numeric)
		ui.sbMajorTicksSpacingNumeric->setValue(increment);
	else
		dtsbMajorTicksIncrement->setValue(increment);
}
void AxisDock::axisMajorTicksColumnChanged(const AbstractColumn* column) {
	Lock lock(m_initializing);
	cbMajorTicksColumn->setColumn(column, m_axis->majorTicksColumnPath());
}
void AxisDock::axisMajorTicksPenChanged(const QPen& pen) {
	m_initializing = true;
	ui.cbMajorTicksLineStyle->setCurrentIndex(pen.style());
	ui.kcbMajorTicksColor->setColor(pen.color());
	ui.sbMajorTicksWidth->setValue( Worksheet::convertFromSceneUnits(pen.widthF(), Worksheet::Unit::Point) );
	m_initializing = false;
}
void AxisDock::axisMajorTicksLengthChanged(qreal length) {
	m_initializing = true;
	ui.sbMajorTicksLength->setValue( Worksheet::convertFromSceneUnits(length, Worksheet::Unit::Point) );
	m_initializing = false;
}
void AxisDock::axisMajorTicksOpacityChanged(qreal opacity) {
	m_initializing = true;
	ui.sbMajorTicksOpacity->setValue( round(opacity * 100.0) );
	m_initializing = false;
}

//minor ticks
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
void AxisDock::axisMinorTicksNumberChanged(int number) {
	m_initializing = true;
	ui.sbMinorTicksNumber->setValue(number);
	m_initializing = false;
}
void AxisDock::axisMinorTicksSpacingChanged(qreal increment) {
	Lock lock(m_initializing);
	const auto* plot = dynamic_cast<const CartesianPlot*>(m_axis->parentAspect());
	const auto* cSystem{ plot->coordinateSystem(m_axis->coordinateSystemIndex()) };
	const int xIndex{cSystem->xIndex()}, yIndex {cSystem->yIndex()};
	bool numeric = ( (m_axis->orientation() == Axis::Orientation::Horizontal && plot->xRangeFormat(xIndex) == RangeT::Format::Numeric)
	                 || (m_axis->orientation() == Axis::Orientation::Vertical && plot->yRangeFormat(yIndex) == RangeT::Format::Numeric) );

	if (numeric)
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
	ui.sbMinorTicksWidth->setValue( Worksheet::convertFromSceneUnits(pen.widthF(), Worksheet::Unit::Point) );
	m_initializing = false;
}
void AxisDock::axisMinorTicksLengthChanged(qreal length) {
	m_initializing = true;
	ui.sbMinorTicksLength->setValue( Worksheet::convertFromSceneUnits(length, Worksheet::Unit::Point) );
	m_initializing = false;
}
void AxisDock::axisMinorTicksOpacityChanged(qreal opacity) {
	m_initializing = true;
	ui.sbMinorTicksOpacity->setValue(round(opacity*100.0));
	m_initializing = false;
}

//labels
void AxisDock::axisLabelsFormatChanged(Axis::LabelsFormat format) {
	m_initializing = true;
	ui.cbLabelsFormat->setCurrentIndex( Axis::labelsFormatToIndex(format) );
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
	ui.sbLabelsOffset->setValue( Worksheet::convertFromSceneUnits(offset, Worksheet::Unit::Point) );
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
	//we need to set the font size in points for KFontRequester
	QFont newFont(font);
	newFont.setPointSizeF( round(Worksheet::convertFromSceneUnits(font.pixelSize(), Worksheet::Unit::Point)) );
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
	ui.sbLabelsOpacity->setValue( round(opacity * 100.0) );
	m_initializing = false;
}

//grid
void AxisDock::axisMajorGridPenChanged(const QPen& pen) {
	m_initializing = true;
	ui.cbMajorGridStyle->setCurrentIndex((int) pen.style());
	ui.kcbMajorGridColor->setColor(pen.color());
	GuiTools::updatePenStyles(ui.cbMajorGridStyle, pen.color());
	ui.sbMajorGridWidth->setValue(Worksheet::convertFromSceneUnits(pen.widthF(), Worksheet::Unit::Point));
	m_initializing = false;
}
void AxisDock::axisMajorGridOpacityChanged(qreal opacity) {
	m_initializing = true;
	ui.sbMajorGridOpacity->setValue( round(opacity * 100.0) );
	m_initializing = false;
}
void AxisDock::axisMinorGridPenChanged(const QPen& pen) {
	m_initializing = true;
	ui.cbMinorGridStyle->setCurrentIndex( static_cast<int>(pen.style()) );
	ui.kcbMinorGridColor->setColor(pen.color());
	GuiTools::updatePenStyles(ui.cbMinorGridStyle, pen.color());
	ui.sbMinorGridWidth->setValue(Worksheet::convertFromSceneUnits(pen.widthF(), Worksheet::Unit::Point));
	m_initializing = false;
}
void AxisDock::axisMinorGridOpacityChanged(qreal opacity) {
	m_initializing = true;
	ui.sbMinorGridOpacity->setValue( round(opacity * 100.0) );
	m_initializing = false;
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
	//General
	ui.chkVisible->setChecked( m_axis->isVisible() );

	Axis::Orientation orientation = m_axis->orientation();
	ui.cbOrientation->setCurrentIndex( static_cast<int>(orientation) );

	Range<double> logicalRange;
	if (orientation == Axis::Orientation::Horizontal) {
		logicalRange = m_axis->plot()->yRange(m_axis->plot()->coordinateSystem(m_axis->coordinateSystemIndex())->yIndex());
		ui.cbPosition->setItemText(Top_Left, i18n("Top"));
		ui.cbPosition->setItemText(Bottom_Right, i18n("Bottom"));
		ui.cbPosition->setItemText(Center, i18n("Centered"));
	} else {
		logicalRange = m_axis->plot()->xRange(m_axis->plot()->coordinateSystem(m_axis->coordinateSystemIndex())->xIndex());
		ui.cbPosition->setItemText(Top_Left, i18n("Left"));
		ui.cbPosition->setItemText(Bottom_Right, i18n("Right"));
		ui.cbPosition->setItemText(Center, i18n("Centered"));
	}

	int index{ static_cast<int>(m_axis->position()) };
	bool logical = false;
	switch(index) {
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

	ui.cbScale->setCurrentIndex( (int)m_axis->scale() );
	ui.chkAutoScale->setChecked( m_axis->autoScale() );
	ui.leStart->setText( numberLocale.toString(m_axis->range().start()) );
	ui.leEnd->setText( numberLocale.toString(m_axis->range().end()) );

	ui.sbMajorTicksSpacingNumeric->setDecimals(0);
	ui.sbMajorTicksSpacingNumeric->setSingleStep(m_axis->majorTicksSpacing());

	//depending on the range format of the axis (numeric vs. datetime), show/hide the corresponding widgets
	const auto* plot = static_cast<const CartesianPlot*>(m_axis->parentAspect());
	const auto* cSystem{ plot->coordinateSystem(m_axis->coordinateSystemIndex()) };
	const int xIndex{cSystem->xIndex()}, yIndex{cSystem->yIndex()};
	const bool numeric = ( (m_axis->orientation() == Axis::Orientation::Horizontal && plot->xRangeFormat(xIndex) == RangeT::Format::Numeric)
	                       || (m_axis->orientation() == Axis::Orientation::Vertical && plot->yRangeFormat(yIndex) == RangeT::Format::Numeric) );

	//ranges
	ui.lStart->setVisible(numeric);
	ui.lEnd->setVisible(numeric);
	ui.leStart->setVisible(numeric);
	ui.leEnd->setVisible(numeric);
	ui.lStartDateTime->setVisible(!numeric);
	ui.dateTimeEditStart->setVisible(!numeric);
	ui.lEndDateTime->setVisible(!numeric);
	ui.dateTimeEditEnd->setVisible(!numeric);

	//tick labels format
	ui.lLabelsFormat->setVisible(numeric);
	ui.cbLabelsFormat->setVisible(numeric);
	ui.chkLabelsAutoPrecision->setVisible(numeric);
	ui.lLabelsPrecision->setVisible(numeric);
	ui.sbLabelsPrecision->setVisible(numeric);
	ui.lLabelsDateTimeFormat->setVisible(!numeric);
	ui.cbLabelsDateTimeFormat->setVisible(!numeric);

	if (!numeric) {
		if (m_axis->orientation() == Axis::Orientation::Horizontal) {
			ui.dateTimeEditStart->setDisplayFormat(plot->xRangeDateTimeFormat(xIndex));
			ui.dateTimeEditEnd->setDisplayFormat(plot->xRangeDateTimeFormat(xIndex));
		} else {
			//TODO
			ui.dateTimeEditStart->setDisplayFormat(plot->yRangeDateTimeFormat());
			ui.dateTimeEditEnd->setDisplayFormat(plot->yRangeDateTimeFormat());
		}
		ui.dateTimeEditStart->setDateTime(QDateTime::fromMSecsSinceEpoch(m_axis->range().start()));
		ui.dateTimeEditEnd->setDateTime(QDateTime::fromMSecsSinceEpoch(m_axis->range().end()));
	}

	ui.leZeroOffset->setText( numberLocale.toString(m_axis->zeroOffset()) );
	ui.leScalingFactor->setText( numberLocale.toString(m_axis->scalingFactor()) );

	//Line
	ui.cbLineStyle->setCurrentIndex( (int) m_axis->linePen().style() );
	ui.kcbLineColor->setColor( m_axis->linePen().color() );
	ui.sbLineWidth->setValue( Worksheet::convertFromSceneUnits(m_axis->linePen().widthF(), Worksheet::Unit::Point) );
	ui.sbLineOpacity->setValue( round(m_axis->lineOpacity()*100.0) );
	ui.cbArrowType->setCurrentIndex( (int)m_axis->arrowType() );
	ui.cbArrowPosition->setCurrentIndex( (int)m_axis->arrowPosition() );
	ui.sbArrowSize->setValue( (int)Worksheet::convertFromSceneUnits(m_axis->arrowSize(), Worksheet::Unit::Point) );

	//Major ticks
	ui.cbMajorTicksDirection->setCurrentIndex( (int) m_axis->majorTicksDirection() );
	ui.cbMajorTicksType->setCurrentIndex( (int) m_axis->majorTicksType() );
	ui.sbMajorTicksNumber->setValue( m_axis->majorTicksNumber() );
	ui.cbMajorTicksLineStyle->setCurrentIndex( (int) m_axis->majorTicksPen().style() );
	ui.kcbMajorTicksColor->setColor( m_axis->majorTicksPen().color() );
	ui.sbMajorTicksWidth->setValue( Worksheet::convertFromSceneUnits( m_axis->majorTicksPen().widthF(), Worksheet::Unit::Point) );
	ui.sbMajorTicksLength->setValue( Worksheet::convertFromSceneUnits( m_axis->majorTicksLength(), Worksheet::Unit::Point) );
	ui.sbMajorTicksOpacity->setValue( round(m_axis->majorTicksOpacity()*100.0) );

	//Minor ticks
	ui.cbMinorTicksDirection->setCurrentIndex( (int) m_axis->minorTicksDirection() );
	ui.cbMinorTicksType->setCurrentIndex( (int) m_axis->minorTicksType() );
	ui.sbMinorTicksNumber->setValue( m_axis->minorTicksNumber() );
	ui.cbMinorTicksLineStyle->setCurrentIndex( (int) m_axis->minorTicksPen().style() );
	ui.kcbMinorTicksColor->setColor( m_axis->minorTicksPen().color() );
	ui.sbMinorTicksWidth->setValue( Worksheet::convertFromSceneUnits(m_axis->minorTicksPen().widthF(), Worksheet::Unit::Point) );
	ui.sbMinorTicksLength->setValue( Worksheet::convertFromSceneUnits(m_axis->minorTicksLength(), Worksheet::Unit::Point) );
	ui.sbMinorTicksOpacity->setValue( round(m_axis->minorTicksOpacity()*100.0) );

	//Extra ticks
	//TODO

	// Tick label
	ui.cbLabelsPosition->setCurrentIndex( (int) m_axis->labelsPosition() );
	ui.sbLabelsOffset->setValue( Worksheet::convertFromSceneUnits(m_axis->labelsOffset(), Worksheet::Unit::Point) );
	ui.sbLabelsRotation->setValue( m_axis->labelsRotationAngle() );
	ui.cbLabelsTextType->setCurrentIndex((int) m_axis->labelsTextType());
	ui.cbLabelsFormat->setCurrentIndex( Axis::labelsFormatToIndex(m_axis->labelsFormat()) );
	ui.chkLabelsAutoPrecision->setChecked( (int) m_axis->labelsAutoPrecision() );
	ui.sbLabelsPrecision->setValue( (int)m_axis->labelsPrecision() );
	ui.cbLabelsDateTimeFormat->setCurrentText(m_axis->labelsDateTimeFormat());

	//we need to set the font size in points for KFontRequester
	QFont font = m_axis->labelsFont();
	font.setPointSizeF( round(Worksheet::convertFromSceneUnits(font.pixelSize(), Worksheet::Unit::Point)) );
	ui.kfrLabelsFont->setFont( font );
	ui.kcbLabelsFontColor->setColor( m_axis->labelsColor() );
	ui.cbLabelsBackgroundType->setCurrentIndex( (int) m_axis->labelsBackgroundType() );
	ui.kcbLabelsBackgroundColor->setColor( m_axis->labelsBackgroundColor() );
	ui.leLabelsPrefix->setText( m_axis->labelsPrefix() );
	ui.leLabelsSuffix->setText( m_axis->labelsSuffix() );
	ui.sbLabelsOpacity->setValue( round(m_axis->labelsOpacity()*100.0) );

	//Grid
	ui.cbMajorGridStyle->setCurrentIndex( (int) m_axis->majorGridPen().style() );
	ui.kcbMajorGridColor->setColor( m_axis->majorGridPen().color() );
	ui.sbMajorGridWidth->setValue( Worksheet::convertFromSceneUnits(m_axis->majorGridPen().widthF(), Worksheet::Unit::Point) );
	ui.sbMajorGridOpacity->setValue( round(m_axis->majorGridOpacity()*100.0) );

	ui.cbMinorGridStyle->setCurrentIndex( (int) m_axis->minorGridPen().style() );
	ui.kcbMinorGridColor->setColor( m_axis->minorGridPen().color() );
	ui.sbMinorGridWidth->setValue( Worksheet::convertFromSceneUnits(m_axis->minorGridPen().widthF(), Worksheet::Unit::Point) );
	ui.sbMinorGridOpacity->setValue( round(m_axis->minorGridOpacity()*100.0) );

	GuiTools::updatePenStyles(ui.cbLineStyle, ui.kcbLineColor->color());
	majorTicksTypeChanged(ui.cbMajorTicksType->currentIndex());
	GuiTools::updatePenStyles(ui.cbMajorTicksLineStyle, ui.kcbMajorTicksColor->color());
	minorTicksTypeChanged(ui.cbMinorTicksType->currentIndex());
	GuiTools::updatePenStyles(ui.cbMinorTicksLineStyle, ui.kcbMinorTicksColor->color());
	GuiTools::updatePenStyles(ui.cbMajorGridStyle, ui.kcbMajorGridColor->color());
	GuiTools::updatePenStyles(ui.cbMinorGridStyle, ui.kcbMinorGridColor->color());
	labelsTextTypeChanged(ui.cbLabelsTextType->currentIndex());
	labelsTextColumnChanged(cbLabelsTextColumn->currentModelIndex());
}

void AxisDock::loadConfigFromTemplate(KConfig& config) {
	//extract the name of the template from the file name
	QString name;
	int index = config.name().lastIndexOf(QLatin1String("/"));
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
	KConfigGroup group = config.group( "Axis" );

	const auto* plot = static_cast<const CartesianPlot*>(m_axis->parentAspect());
	const auto* cSystem{ plot->coordinateSystem(m_axis->coordinateSystemIndex()) };
	const int xIndex{cSystem->xIndex()}, yIndex{cSystem->yIndex()};
	const bool numeric = ( (m_axis->orientation() == Axis::Orientation::Horizontal && plot->xRangeFormat(xIndex) == RangeT::Format::Numeric)
	                       || (m_axis->orientation() == Axis::Orientation::Vertical && plot->yRangeFormat(yIndex) == RangeT::Format::Numeric) );

	//General
	ui.cbOrientation->setCurrentIndex( group.readEntry("Orientation", (int) m_axis->orientation()) );

	int index = group.readEntry("Position", (int) m_axis->position());
	if (index > 1)
		ui.cbPosition->setCurrentIndex(index-2);
	else
		ui.cbPosition->setCurrentIndex(index);

	SET_NUMBER_LOCALE
	ui.sbPositionLogical->setValue(group.readEntry("LogicalPosition", m_axis->logicalPosition()));
	ui.sbPosition->setValue(Worksheet::convertFromSceneUnits(group.readEntry("PositionOffset", m_axis->offset()), m_worksheetUnit));
	ui.cbScale->setCurrentIndex( group.readEntry("Scale", (int) m_axis->scale()) );
	ui.chkAutoScale->setChecked( group.readEntry("AutoScale", m_axis->autoScale()) );
	ui.leStart->setText( numberLocale.toString(group.readEntry("Start", m_axis->range().start())) );
	ui.leEnd->setText( numberLocale.toString(group.readEntry("End", m_axis->range().end())) );
	ui.leZeroOffset->setText( numberLocale.toString(group.readEntry("ZeroOffset", m_axis->zeroOffset())) );
	ui.leScalingFactor->setText( numberLocale.toString(group.readEntry("ScalingFactor", m_axis->scalingFactor())) );

	//Title
	KConfigGroup axisLabelGroup = config.group("AxisLabel");
	labelWidget->loadConfig(axisLabelGroup);

	//Line
	ui.cbLineStyle->setCurrentIndex( group.readEntry("LineStyle", (int) m_axis->linePen().style()) );
	ui.kcbLineColor->setColor( group.readEntry("LineColor", m_axis->linePen().color()) );
	ui.sbLineWidth->setValue( Worksheet::convertFromSceneUnits(group.readEntry("LineWidth", m_axis->linePen().widthF()), Worksheet::Unit::Point) );
	ui.sbLineOpacity->setValue( round(group.readEntry("LineOpacity", m_axis->lineOpacity())*100.0) );
	ui.cbArrowType->setCurrentIndex( group.readEntry("ArrowType", (int) m_axis->arrowType()) );
	ui.cbArrowPosition->setCurrentIndex( group.readEntry("ArrowPosition", (int) m_axis->arrowPosition()) );
	ui.sbArrowSize->setValue( Worksheet::convertFromSceneUnits(group.readEntry("ArrowSize", m_axis->arrowSize()), Worksheet::Unit::Point) );

	//Major ticks
	ui.cbMajorTicksDirection->setCurrentIndex( group.readEntry("MajorTicksDirection", (int) m_axis->majorTicksDirection()) );
	ui.cbMajorTicksType->setCurrentIndex( group.readEntry("MajorTicksType", (int) m_axis->majorTicksType()) );
	ui.sbMajorTicksNumber->setValue( group.readEntry("MajorTicksNumber", m_axis->majorTicksNumber()) );
	auto value{ group.readEntry("MajorTicksIncrement", m_axis->majorTicksSpacing()) };
	if (numeric)
		ui.sbMajorTicksSpacingNumeric->setValue(value);
	else
		dtsbMajorTicksIncrement->setValue(value);
	ui.cbMajorTicksLineStyle->setCurrentIndex( group.readEntry("MajorTicksLineStyle", (int) m_axis->majorTicksPen().style()) );
	ui.kcbMajorTicksColor->setColor( group.readEntry("MajorTicksColor", m_axis->majorTicksPen().color()) );
	ui.sbMajorTicksWidth->setValue( Worksheet::convertFromSceneUnits(group.readEntry("MajorTicksWidth", m_axis->majorTicksPen().widthF()), Worksheet::Unit::Point) );
	ui.sbMajorTicksLength->setValue( Worksheet::convertFromSceneUnits(group.readEntry("MajorTicksLength", m_axis->majorTicksLength()), Worksheet::Unit::Point) );
	ui.sbMajorTicksOpacity->setValue( round(group.readEntry("MajorTicksOpacity", m_axis->majorTicksOpacity())*100.0) );

	//Minor ticks
	ui.cbMinorTicksDirection->setCurrentIndex( group.readEntry("MinorTicksDirection", (int) m_axis->minorTicksDirection()) );
	ui.cbMinorTicksType->setCurrentIndex( group.readEntry("MinorTicksType", (int) m_axis->minorTicksType()) );
	ui.sbMinorTicksNumber->setValue( group.readEntry("MinorTicksNumber", m_axis->minorTicksNumber()) );
	value = group.readEntry("MinorTicksIncrement", m_axis->minorTicksSpacing());
	if (numeric)
		ui.sbMinorTicksSpacingNumeric->setValue(value);
	else
		dtsbMinorTicksIncrement->setValue(value);
	ui.cbMinorTicksLineStyle->setCurrentIndex( group.readEntry("MinorTicksLineStyle", (int) m_axis->minorTicksPen().style()) );
	ui.kcbMinorTicksColor->setColor( group.readEntry("MinorTicksColor", m_axis->minorTicksPen().color()) );
	ui.sbMinorTicksWidth->setValue( Worksheet::convertFromSceneUnits(group.readEntry("MinorTicksWidth", m_axis->minorTicksPen().widthF()), Worksheet::Unit::Point) );
	ui.sbMinorTicksLength->setValue( Worksheet::convertFromSceneUnits(group.readEntry("MinorTicksLength", m_axis->minorTicksLength()), Worksheet::Unit::Point) );
	ui.sbMinorTicksOpacity->setValue( round(group.readEntry("MinorTicksOpacity", m_axis->minorTicksOpacity())*100.0) );

	//Extra ticks
	//TODO

	// Tick label
	ui.cbLabelsFormat->setCurrentIndex( Axis::labelsFormatToIndex((Axis::LabelsFormat) ( group.readEntry("LabelsFormat", Axis::labelsFormatToIndex(m_axis->labelsFormat())) )) );
	ui.chkLabelsAutoPrecision->setChecked( group.readEntry("LabelsAutoPrecision", (int) m_axis->labelsAutoPrecision()) );
	ui.sbLabelsPrecision->setValue( group.readEntry("LabelsPrecision", (int)m_axis->labelsPrecision()) );
	ui.cbLabelsDateTimeFormat->setCurrentText( group.readEntry("LabelsDateTimeFormat", "yyyy-MM-dd hh:mm:ss") );
	ui.cbLabelsPosition->setCurrentIndex( group.readEntry("LabelsPosition", (int) m_axis->labelsPosition()) );
	ui.sbLabelsOffset->setValue( Worksheet::convertFromSceneUnits(group.readEntry("LabelsOffset", m_axis->labelsOffset()), Worksheet::Unit::Point) );
	ui.sbLabelsRotation->setValue( group.readEntry("LabelsRotation", m_axis->labelsRotationAngle()) );
	ui.cbLabelsTextType->setCurrentIndex( group.readEntry("LabelsTextType", (int) m_axis->labelsTextType()) );

	//we need to set the font size in points for KFontRequester
	QFont font = m_axis->labelsFont();
	font.setPointSizeF( round(Worksheet::convertFromSceneUnits(font.pixelSize(), Worksheet::Unit::Point)) );
	ui.kfrLabelsFont->setFont( group.readEntry("LabelsFont", font) );

	ui.kcbLabelsFontColor->setColor( group.readEntry("LabelsFontColor", m_axis->labelsColor()) );
	ui.cbLabelsBackgroundType->setCurrentIndex( group.readEntry("LabelsBackgroundType", (int) m_axis->labelsBackgroundType()) );
	ui.kcbLabelsBackgroundColor->setColor( group.readEntry("LabelsBackgroundColor", m_axis->labelsBackgroundColor()) );
	ui.leLabelsPrefix->setText( group.readEntry("LabelsPrefix", m_axis->labelsPrefix()) );
	ui.leLabelsSuffix->setText( group.readEntry("LabelsSuffix", m_axis->labelsSuffix()) );
	ui.sbLabelsOpacity->setValue( round(group.readEntry("LabelsOpacity", m_axis->labelsOpacity())*100.0) );

	//Grid
	ui.cbMajorGridStyle->setCurrentIndex( group.readEntry("MajorGridStyle", (int) m_axis->majorGridPen().style()) );
	ui.kcbMajorGridColor->setColor( group.readEntry("MajorGridColor", m_axis->majorGridPen().color()) );
	ui.sbMajorGridWidth->setValue( Worksheet::convertFromSceneUnits(group.readEntry("MajorGridWidth", m_axis->majorGridPen().widthF()), Worksheet::Unit::Point) );
	ui.sbMajorGridOpacity->setValue( round(group.readEntry("MajorGridOpacity", m_axis->majorGridOpacity())*100.0) );

	ui.cbMinorGridStyle->setCurrentIndex( group.readEntry("MinorGridStyle", (int) m_axis->minorGridPen().style()) );
	ui.kcbMinorGridColor->setColor( group.readEntry("MinorGridColor", m_axis->minorGridPen().color()) );
	ui.sbMinorGridWidth->setValue( Worksheet::convertFromSceneUnits(group.readEntry("MinorGridWidth", m_axis->minorGridPen().widthF()), Worksheet::Unit::Point) );
	ui.sbMinorGridOpacity->setValue( round(group.readEntry("MinorGridOpacity", m_axis->minorGridOpacity())*100.0) );

	m_initializing = true;
	GuiTools::updatePenStyles(ui.cbLineStyle, ui.kcbLineColor->color());
	this->majorTicksTypeChanged(ui.cbMajorTicksType->currentIndex());
	GuiTools::updatePenStyles(ui.cbMajorTicksLineStyle, ui.kcbMajorTicksColor->color());
	this->minorTicksTypeChanged(ui.cbMinorTicksType->currentIndex());
	GuiTools::updatePenStyles(ui.cbMinorTicksLineStyle, ui.kcbMinorTicksColor->color());
	GuiTools::updatePenStyles(ui.cbMajorGridStyle, ui.kcbMajorGridColor->color());
	GuiTools::updatePenStyles(ui.cbMinorGridStyle, ui.kcbMinorGridColor->color());
	m_initializing = false;
}

void AxisDock::saveConfigAsTemplate(KConfig& config) {
	KConfigGroup group = config.group( "Axis" );

	bool numeric = false;
	const auto* plot = dynamic_cast<const CartesianPlot*>(m_axis->parentAspect());
	const auto* cSystem{ plot->coordinateSystem(m_axis->coordinateSystemIndex()) };
	const int xIndex{cSystem->xIndex()}, yIndex{cSystem->yIndex()};
	if (plot) {
		numeric = ( (m_axis->orientation() == Axis::Orientation::Horizontal && plot->xRangeFormat(xIndex) == RangeT::Format::Numeric)
		            || (m_axis->orientation() == Axis::Orientation::Vertical && plot->yRangeFormat(yIndex) == RangeT::Format::Numeric) );
	}

	//General
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
	group.writeEntry("LogicalPosition",  ui.sbPositionLogical->value());
	group.writeEntry("PositionOffset",  Worksheet::convertToSceneUnits(ui.sbPosition->value(), m_worksheetUnit));
	group.writeEntry("Scale", ui.cbScale->currentIndex());
	group.writeEntry("Start", numberLocale.toDouble(ui.leStart->text()));
	group.writeEntry("End", numberLocale.toDouble(ui.leEnd->text()));
	group.writeEntry("ZeroOffset", numberLocale.toDouble(ui.leZeroOffset->text()));
	group.writeEntry("ScalingFactor", numberLocale.toDouble(ui.leScalingFactor->text()));

	//Title
	KConfigGroup axisLabelGroup = config.group("AxisLabel");
	labelWidget->saveConfig(axisLabelGroup);

	//Line
	group.writeEntry("LineStyle", ui.cbLineStyle->currentIndex());
	group.writeEntry("LineColor", ui.kcbLineColor->color());
	group.writeEntry("LineWidth", Worksheet::convertToSceneUnits(ui.sbLineWidth->value(), Worksheet::Unit::Point));
	group.writeEntry("LineOpacity", ui.sbLineOpacity->value()/100.);

	//Major ticks
	group.writeEntry("MajorTicksDirection", ui.cbMajorTicksDirection->currentIndex());
	group.writeEntry("MajorTicksType", ui.cbMajorTicksType->currentIndex());
	group.writeEntry("MajorTicksNumber", ui.sbMajorTicksNumber->value());
	if (numeric)
		group.writeEntry("MajorTicksIncrement", QString::number(ui.sbMajorTicksSpacingNumeric->value()));
	else
		group.writeEntry("MajorTicksIncrement", QString::number(dtsbMajorTicksIncrement->value()));
	group.writeEntry("MajorTicksLineStyle", ui.cbMajorTicksLineStyle->currentIndex());
	group.writeEntry("MajorTicksColor", ui.kcbMajorTicksColor->color());
	group.writeEntry("MajorTicksWidth", Worksheet::convertToSceneUnits(ui.sbMajorTicksWidth->value(), Worksheet::Unit::Point));
	group.writeEntry("MajorTicksLength", Worksheet::convertToSceneUnits(ui.sbMajorTicksLength->value(), Worksheet::Unit::Point));
	group.writeEntry("MajorTicksOpacity", ui.sbMajorTicksOpacity->value()/100.);

	//Minor ticks
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
	group.writeEntry("MinorTicksOpacity", ui.sbMinorTicksOpacity->value()/100.);

	//Extra ticks
	// TODO

	// Tick label
	group.writeEntry("LabelsFormat", static_cast<int>( Axis::indexToLabelsFormat(ui.cbLabelsFormat->currentIndex()) ));
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
	group.writeEntry("LabelsOpacity", ui.sbLabelsOpacity->value()/100.);

	//Grid
	group.writeEntry("MajorGridStyle", ui.cbMajorGridStyle->currentIndex());
	group.writeEntry("MajorGridColor", ui.kcbMajorGridColor->color());
	group.writeEntry("MajorGridWidth", Worksheet::convertToSceneUnits(ui.sbMajorGridWidth->value(), Worksheet::Unit::Point));
	group.writeEntry("MajorGridOpacity", ui.sbMajorGridOpacity->value()/100.);

	group.writeEntry("MinorGridStyle", ui.cbMinorGridStyle->currentIndex());
	group.writeEntry("MinorGridColor", ui.kcbMinorGridColor->color());
	group.writeEntry("MinorGridWidth", Worksheet::convertToSceneUnits(ui.sbMinorGridWidth->value(), Worksheet::Unit::Point));
	group.writeEntry("MinorGridOpacity", ui.sbMinorGridOpacity->value()/100.);
	config.sync();
}
