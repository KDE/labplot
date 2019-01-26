/***************************************************************************
    File                 : AxisDock.cpp
    Project              : LabPlot
    Description          : axes widget class
    --------------------------------------------------------------------
    Copyright            : (C) 2011-2018 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2012-2013 Stefan Gerlach (stefan.gerlach@uni-konstanz.de)

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

#include <QTimer>
#include <QDir>
#include <QPainter>
#include <KLocalizedString>
#include <KMessageBox>

/*!
 \class AxisDock
 \brief Provides a widget for editing the properties of the axes currently selected in the project explorer.

 \ingroup kdefrontend
*/

AxisDock::AxisDock(QWidget* parent) : QWidget(parent) {
	ui.setupUi(this);

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
	cbMinorTicksColumn = new TreeViewComboBox(ui.tabTicks);
	layout->addWidget(cbMinorTicksColumn, 21, 2);
	dtsbMajorTicksIncrement = new DateTimeSpinBox(ui.tabTicks);
	layout->addWidget(dtsbMajorTicksIncrement, 6, 2);
	dtsbMinorTicksIncrement = new DateTimeSpinBox(ui.tabTicks);
	layout->addWidget(dtsbMinorTicksIncrement, 20, 2);

	//adjust layouts in the tabs
	for (int i = 0; i < ui.tabWidget->count(); ++i) {
		layout = dynamic_cast<QGridLayout*>(ui.tabWidget->widget(i)->layout());
		if (!layout)
			continue;

		layout->setContentsMargins(2,2,2,2);
		layout->setHorizontalSpacing(2);
		layout->setVerticalSpacing(2);
	}

	//**********************************  Slots **********************************************

	//"General"-tab
	connect(ui.leName, &QLineEdit::textChanged, this, &AxisDock::nameChanged);
	connect(ui.leComment, &QLineEdit::textChanged, this, &AxisDock::commentChanged);
	connect( ui.chkVisible, SIGNAL(clicked(bool)), this, SLOT(visibilityChanged(bool)) );

	connect( ui.cbOrientation, SIGNAL(currentIndexChanged(int)), this, SLOT(orientationChanged(int)) );
	connect( ui.cbPosition, SIGNAL(currentIndexChanged(int)), this, SLOT(positionChanged(int)) );
	connect( ui.lePosition, SIGNAL(textChanged(QString)), this, SLOT(positionChanged()) );
	connect( ui.cbScale, SIGNAL(currentIndexChanged(int)), this, SLOT(scaleChanged(int)) );

	connect( ui.chkAutoScale, SIGNAL(stateChanged(int)), this, SLOT(autoScaleChanged(int)) );
	connect( ui.leStart, SIGNAL(textChanged(QString)), this, SLOT(startChanged()) );
	connect( ui.leEnd, SIGNAL(textChanged(QString)), this, SLOT(endChanged()) );
	connect(ui.dateTimeEditStart, &QDateTimeEdit::dateTimeChanged, this, &AxisDock::startDateTimeChanged);
	connect(ui.dateTimeEditEnd, &QDateTimeEdit::dateTimeChanged, this, &AxisDock::endDateTimeChanged);
	connect( ui.leZeroOffset, SIGNAL(textChanged(QString)), this, SLOT(zeroOffsetChanged()) );
	connect( ui.leScalingFactor, SIGNAL(textChanged(QString)), this, SLOT(scalingFactorChanged()) );

	//"Line"-tab
	connect( ui.cbLineStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(lineStyleChanged(int)) );
	connect( ui.kcbLineColor, SIGNAL(changed(QColor)), this, SLOT(lineColorChanged(QColor)) );
	connect( ui.sbLineWidth, SIGNAL(valueChanged(double)), this, SLOT(lineWidthChanged(double)) );
	connect( ui.sbLineOpacity, SIGNAL(valueChanged(int)), this, SLOT(lineOpacityChanged(int)) );
	connect( ui.cbArrowPosition, SIGNAL(currentIndexChanged(int)), this, SLOT(arrowPositionChanged(int)) );
	connect( ui.cbArrowType, SIGNAL(currentIndexChanged(int)), this, SLOT(arrowTypeChanged(int)) );
	connect( ui.sbArrowSize, SIGNAL(valueChanged(int)), this, SLOT(arrowSizeChanged(int)) );

	//"Major ticks"-tab
	connect( ui.cbMajorTicksDirection, SIGNAL(currentIndexChanged(int)), this, SLOT(majorTicksDirectionChanged(int)) );
	connect( ui.cbMajorTicksType, SIGNAL(currentIndexChanged(int)), this, SLOT(majorTicksTypeChanged(int)) );
	connect( ui.sbMajorTicksNumber, SIGNAL(valueChanged(int)), this, SLOT(majorTicksNumberChanged(int)) );
	connect( ui.sbMajorTicksIncrementNumeric, SIGNAL(valueChanged(double)), this, SLOT(majorTicksIncrementChanged()) );
	connect( dtsbMajorTicksIncrement, SIGNAL(valueChanged()), this, SLOT(majorTicksIncrementChanged()) );
	//connect( ui.sbMajorTicksIncrementNumeric, &QDoubleSpinBox::valueChanged, this, &AxisDock::majorTicksIncrementChanged);
	connect( cbMajorTicksColumn, SIGNAL(currentModelIndexChanged(QModelIndex)), this, SLOT(majorTicksColumnChanged(QModelIndex)) );
	connect( ui.cbMajorTicksLineStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(majorTicksLineStyleChanged(int)) );
	connect( ui.kcbMajorTicksColor, SIGNAL(changed(QColor)), this, SLOT(majorTicksColorChanged(QColor)) );
	connect( ui.sbMajorTicksWidth, SIGNAL(valueChanged(double)), this, SLOT(majorTicksWidthChanged(double)) );
	connect( ui.sbMajorTicksLength, SIGNAL(valueChanged(double)), this, SLOT(majorTicksLengthChanged(double)) );
	connect( ui.sbMajorTicksOpacity, SIGNAL(valueChanged(int)), this, SLOT(majorTicksOpacityChanged(int)) );

	//"Minor ticks"-tab
	connect( ui.cbMinorTicksDirection, SIGNAL(currentIndexChanged(int)), this, SLOT(minorTicksDirectionChanged(int)) );
	connect( ui.cbMinorTicksType, SIGNAL(currentIndexChanged(int)), this, SLOT(minorTicksTypeChanged(int)) );
	connect( ui.sbMinorTicksNumber, SIGNAL(valueChanged(int)), this, SLOT(minorTicksNumberChanged(int)) );
	connect( ui.sbMajorTicksIncrementNumeric, SIGNAL(valueChanged(double)), this, SLOT(minorTicksIncrementChanged()) );
	connect( dtsbMinorTicksIncrement, SIGNAL(valueChanged()), this, SLOT(minorTicksIncrementChanged()) );
	connect( cbMinorTicksColumn, SIGNAL(currentModelIndexChanged(QModelIndex)), this, SLOT(minorTicksColumnChanged(QModelIndex)) );
	connect( ui.cbMinorTicksLineStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(minorTicksLineStyleChanged(int)) );
	connect( ui.kcbMinorTicksColor, SIGNAL(changed(QColor)), this, SLOT(minorTicksColorChanged(QColor)) );
	connect( ui.sbMinorTicksWidth, SIGNAL(valueChanged(double)), this, SLOT(minorTicksWidthChanged(double)) );
	connect( ui.sbMinorTicksLength, SIGNAL(valueChanged(double)), this, SLOT(minorTicksLengthChanged(double)) );
	connect( ui.sbMinorTicksOpacity, SIGNAL(valueChanged(int)), this, SLOT(minorTicksOpacityChanged(int)) );

	//"Extra ticks"-tab

	//"Tick labels"-tab
	connect( ui.cbLabelsFormat, SIGNAL(currentIndexChanged(int)), this, SLOT(labelsFormatChanged(int)) );
	connect( ui.sbLabelsPrecision, SIGNAL(valueChanged(int)), this, SLOT(labelsPrecisionChanged(int)) );
	connect( ui.chkLabelsAutoPrecision, SIGNAL(stateChanged(int)), this, SLOT(labelsAutoPrecisionChanged(int)) );
	connect(ui.cbLabelsDateTimeFormat, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &AxisDock::labelsDateTimeFormatChanged);
	connect( ui.cbLabelsPosition, SIGNAL(currentIndexChanged(int)), this, SLOT(labelsPositionChanged(int)) );
	connect( ui.sbLabelsOffset, SIGNAL(valueChanged(double)), this, SLOT(labelsOffsetChanged(double)) );
	connect( ui.sbLabelsRotation, SIGNAL(valueChanged(int)), this, SLOT(labelsRotationChanged(int)) );
	connect( ui.kfrLabelsFont, SIGNAL(fontSelected(QFont)), this, SLOT(labelsFontChanged(QFont)) );
	connect( ui.kcbLabelsFontColor, SIGNAL(changed(QColor)), this, SLOT(labelsFontColorChanged(QColor)) );
	connect( ui.leLabelsPrefix, SIGNAL(textChanged(QString)), this, SLOT(labelsPrefixChanged()) );
	connect( ui.leLabelsSuffix, SIGNAL(textChanged(QString)), this, SLOT(labelsSuffixChanged()) );
	connect( ui.sbLabelsOpacity, SIGNAL(valueChanged(int)), this, SLOT(labelsOpacityChanged(int)) );

	//"Grid"-tab
	connect( ui.cbMajorGridStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(majorGridStyleChanged(int)) );
	connect( ui.kcbMajorGridColor, SIGNAL(changed(QColor)), this, SLOT(majorGridColorChanged(QColor)) );
	connect( ui.sbMajorGridWidth, SIGNAL(valueChanged(double)), this, SLOT(majorGridWidthChanged(double)) );
	connect( ui.sbMajorGridOpacity, SIGNAL(valueChanged(int)), this, SLOT(majorGridOpacityChanged(int)) );

	connect( ui.cbMinorGridStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(minorGridStyleChanged(int)) );
	connect( ui.kcbMinorGridColor, SIGNAL(changed(QColor)), this, SLOT(minorGridColorChanged(QColor)) );
	connect( ui.sbMinorGridWidth, SIGNAL(valueChanged(double)), this, SLOT(minorGridWidthChanged(double)) );
	connect( ui.sbMinorGridOpacity, SIGNAL(valueChanged(int)), this, SLOT(minorGridOpacityChanged(int)) );


	auto* templateHandler = new TemplateHandler(this, TemplateHandler::Axis);
	ui.verticalLayout->addWidget(templateHandler);
	connect(templateHandler, SIGNAL(loadConfigRequested(KConfig&)), this, SLOT(loadConfigFromTemplate(KConfig&)));
	connect(templateHandler, SIGNAL(saveConfigRequested(KConfig&)), this, SLOT(saveConfigAsTemplate(KConfig&)));
	connect(templateHandler, SIGNAL(info(QString)), this, SIGNAL(info(QString)));

	init();
}

AxisDock::~AxisDock() {
	if (m_aspectTreeModel)
		delete m_aspectTreeModel;
}

void AxisDock::init() {
	m_initializing = true;

	//Validators
	ui.lePosition->setValidator( new QDoubleValidator(ui.lePosition) );
	ui.leStart->setValidator( new QDoubleValidator(ui.leStart) );
	ui.leEnd->setValidator( new QDoubleValidator(ui.leEnd) );
	ui.leZeroOffset->setValidator( new QDoubleValidator(ui.leZeroOffset) );
	ui.leScalingFactor->setValidator( new QDoubleValidator(ui.leScalingFactor) );

	//TODO move this stuff to retranslateUI()
	ui.cbPosition->addItem(i18n("Top"));
	ui.cbPosition->addItem(i18n("Bottom"));
	ui.cbPosition->addItem(i18n("Centered"));
	ui.cbPosition->addItem(i18n("Custom"));

	ui.cbScale->addItem( i18n("Linear") );
	ui.cbScale->addItem( QLatin1String("log(x)") );
	ui.cbScale->addItem( QLatin1String("log2(x)") );
	ui.cbScale->addItem( QLatin1String("ln(x)") );
	ui.cbScale->addItem( QLatin1String("sqrt(x)") );
	ui.cbScale->addItem( QLatin1String("x^2") );

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
	pa.drawLine(3,10,17,10);
	pa.end();
	ui.cbArrowType->setItemIcon(0, pm);

	//simple, small
	float cos_phi = cos(3.14159/6);
	pm.fill(Qt::transparent);
	pa.begin( &pm );
	pa.setRenderHint(QPainter::Antialiasing);
	pa.drawLine(3,10,17,10);
	pa.drawLine(17,10, 10, 10-5*cos_phi);
	pa.drawLine(17,10, 10, 10+5*cos_phi);
	pa.end();
	ui.cbArrowType->setItemIcon(1, pm);

	//simple, big
	pm.fill(Qt::transparent);
	pa.begin( &pm );
	pa.setRenderHint(QPainter::Antialiasing);
	pa.drawLine(3,10,17,10);
	pa.drawLine(17,10, 10, 10-10*cos_phi);
	pa.drawLine(17,10, 10, 10+10*cos_phi);
	pa.end();
	ui.cbArrowType->setItemIcon(2, pm);

	//filled, small
	pm.fill(Qt::transparent);
	pa.begin( &pm );
	pa.setRenderHint(QPainter::Antialiasing);
	pa.setBrush(Qt::SolidPattern);
	pa.drawLine(3,10,17,10);
	QPointF points3[3] = {QPointF(17, 10), QPointF(10, 10-4*cos_phi), QPointF(10, 10+4*cos_phi) };
	pa.drawPolygon(points3, 3);
	pa.end();
	ui.cbArrowType->setItemIcon(3, pm);

	//filled, big
	pm.fill(Qt::transparent);
	pa.begin( &pm );
	pa.setRenderHint(QPainter::Antialiasing);
	pa.setBrush(Qt::SolidPattern);
	pa.drawLine(3,10,17,10);
	QPointF points4[3] = {QPointF(17, 10), QPointF(10, 10-10*cos_phi), QPointF(10, 10+10*cos_phi) };
	pa.drawPolygon(points4, 3);
	pa.end();
	ui.cbArrowType->setItemIcon(4, pm);

	//semi-filled, small
	pm.fill(Qt::transparent);
	pa.begin( &pm );
	pa.setRenderHint(QPainter::Antialiasing);
	pa.setBrush(Qt::SolidPattern);
	pa.drawLine(3,10,17,10);
	QPointF points5[4] = {QPointF(17, 10), QPointF(10, 10-4*cos_phi), QPointF(13, 10), QPointF(10, 10+4*cos_phi) };
	pa.drawPolygon(points5, 4);
	pa.end();
	ui.cbArrowType->setItemIcon(5, pm);

	//semi-filled, big
	pm.fill(Qt::transparent);
	pa.begin( &pm );
	pa.setRenderHint(QPainter::Antialiasing);
	pa.setBrush(Qt::SolidPattern);
	pa.drawLine(3,10,17,10);
	QPointF points6[4] = {QPointF(17, 10), QPointF(10, 10-10*cos_phi), QPointF(13, 10), QPointF(10, 10+10*cos_phi) };
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
	ui.cbMajorTicksType->addItem( i18n("Increment") );
	ui.cbMajorTicksType->addItem( i18n("Custom column") );

	ui.cbMinorTicksDirection->addItem( i18n("None") );
	ui.cbMinorTicksDirection->addItem( i18n("In") );
	ui.cbMinorTicksDirection->addItem( i18n("Out") );
	ui.cbMinorTicksDirection->addItem( i18n("In and Out") );

	ui.cbMinorTicksType->addItem( i18n("Number") );
	ui.cbMinorTicksType->addItem( i18n("Increment") );
	ui.cbMinorTicksType->addItem( i18n("Custom column") );

	GuiTools::updatePenStyles(ui.cbLineStyle, QColor(Qt::black));
	GuiTools::updatePenStyles(ui.cbMajorTicksLineStyle, QColor(Qt::black));
	GuiTools::updatePenStyles(ui.cbMinorTicksLineStyle, QColor(Qt::black));

	//labels
	ui.cbLabelsPosition->addItem(i18n("No labels"));
	ui.cbLabelsPosition->addItem(i18n("Top"));
	ui.cbLabelsPosition->addItem(i18n("Bottom"));

	ui.cbLabelsFormat->addItem( i18n("Decimal notation") );
	ui.cbLabelsFormat->addItem( i18n("Scientific notation") );
	ui.cbLabelsFormat->addItem( i18n("Powers of 10") );
	ui.cbLabelsFormat->addItem( i18n("Powers of 2") );
	ui.cbLabelsFormat->addItem( i18n("Powers of e") );
	ui.cbLabelsFormat->addItem( i18n("Multiples of π") );

	ui.cbLabelsDateTimeFormat->addItems(AbstractColumn::dateTimeFormats());

	m_initializing = false;
}

void AxisDock::setModel() {
	QList<const char*>  list;
	list<<"Folder"<<"Spreadsheet"<<"FileDataSource"<<"Column";
	cbMajorTicksColumn->setTopLevelClasses(list);
	cbMinorTicksColumn->setTopLevelClasses(list);

	list.clear();
	list<<"Column";
	m_aspectTreeModel->setSelectableAspects(list);

	cbMajorTicksColumn->setModel(m_aspectTreeModel);
	cbMinorTicksColumn->setModel(m_aspectTreeModel);
}

/*!
  sets the axes. The properties of the axes in the list \c list can be edited in this widget.
*/
void AxisDock::setAxes(QList<Axis*> list) {
	m_initializing = true;
	m_axesList = list;
	m_axis = list.first();
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
	} else {
		ui.lName->setEnabled(false);
		ui.leName->setEnabled(false);
		ui.lComment->setEnabled(false);
		ui.leComment->setEnabled(false);
		ui.leName->setText("");
		ui.leComment->setText("");
		cbMajorTicksColumn->setCurrentModelIndex(QModelIndex());
		cbMinorTicksColumn->setCurrentModelIndex(QModelIndex());
	}

	//show the properties of the first axis
	this->load();

	// general
	connect(m_axis, SIGNAL(aspectDescriptionChanged(const AbstractAspect*)),this, SLOT(axisDescriptionChanged(const AbstractAspect*)));
	connect(m_axis, SIGNAL(orientationChanged(Axis::AxisOrientation)), this, SLOT(axisOrientationChanged(Axis::AxisOrientation)));
	connect(m_axis, SIGNAL(positionChanged(Axis::AxisPosition)), this, SLOT(axisPositionChanged(Axis::AxisPosition)));
	connect(m_axis, SIGNAL(scaleChanged(Axis::AxisScale)), this, SLOT(axisScaleChanged(Axis::AxisScale)));
	connect(m_axis, SIGNAL(autoScaleChanged(bool)), this, SLOT(axisAutoScaleChanged(bool)));
	connect(m_axis, SIGNAL(startChanged(double)), this, SLOT(axisStartChanged(double)));
	connect(m_axis, SIGNAL(endChanged(double)), this, SLOT(axisEndChanged(double)));
	connect(m_axis, SIGNAL(zeroOffsetChanged(qreal)), this, SLOT(axisZeroOffsetChanged(qreal)));
	connect(m_axis, SIGNAL(scalingFactorChanged(qreal)), this, SLOT(axisScalingFactorChanged(qreal)));

	// line
	connect(m_axis, SIGNAL(linePenChanged(QPen)), this, SLOT(axisLinePenChanged(QPen)));
	connect(m_axis, SIGNAL(lineOpacityChanged(qreal)), this, SLOT(axisLineOpacityChanged(qreal)));
	connect(m_axis, SIGNAL(arrowTypeChanged(Axis::ArrowType)), this, SLOT(axisArrowTypeChanged(Axis::ArrowType)));
	connect(m_axis, SIGNAL(arrowPositionChanged(Axis::ArrowPosition)), this, SLOT(axisArrowPositionChanged(Axis::ArrowPosition)));
	connect(m_axis, SIGNAL(arrowSizeChanged(qreal)), this, SLOT(axisArrowSizeChanged(qreal)));

	// ticks
	connect(m_axis, SIGNAL(majorTicksDirectionChanged(Axis::TicksDirection)), this, SLOT(axisMajorTicksDirectionChanged(Axis::TicksDirection)));
	connect(m_axis, SIGNAL(majorTicksTypeChanged(Axis::TicksType)), this, SLOT(axisMajorTicksTypeChanged(Axis::TicksType)));
	connect(m_axis, SIGNAL(majorTicksNumberChanged(int)), this, SLOT(axisMajorTicksNumberChanged(int)));
	connect(m_axis, SIGNAL(majorTicksIncrementChanged(qreal)), this, SLOT(axisMajorTicksIncrementChanged(qreal)));
	connect(m_axis, SIGNAL(majorTicksPenChanged(QPen)), this, SLOT(axisMajorTicksPenChanged(QPen)));
	connect(m_axis, SIGNAL(majorTicksLengthChanged(qreal)), this, SLOT(axisMajorTicksLengthChanged(qreal)));
	connect(m_axis, SIGNAL(majorTicksOpacityChanged(qreal)), this, SLOT(axisMajorTicksOpacityChanged(qreal)));
	connect(m_axis, SIGNAL(minorTicksDirectionChanged(Axis::TicksDirection)), this, SLOT(axisMinorTicksDirectionChanged(Axis::TicksDirection)));
	connect(m_axis, SIGNAL(minorTicksTypeChanged(Axis::TicksType)), this, SLOT(axisMinorTicksTypeChanged(Axis::TicksType)));
	connect(m_axis, SIGNAL(minorTicksNumberChanged(int)), this, SLOT(axisMinorTicksNumberChanged(int)));
	connect(m_axis, SIGNAL(minorTicksIncrementChanged(qreal)), this, SLOT(axisMinorTicksIncrementChanged(qreal)));
	connect(m_axis, SIGNAL(minorTicksPenChanged(QPen)), this, SLOT(axisMinorTicksPenChanged(QPen)));
	connect(m_axis, SIGNAL(minorTicksLengthChanged(qreal)), this, SLOT(axisMinorTicksLengthChanged(qreal)));
	connect(m_axis, SIGNAL(minorTicksOpacityChanged(qreal)), this, SLOT(axisMinorTicksOpacityChanged(qreal)));

	// labels
	connect(m_axis, SIGNAL(labelsFormatChanged(Axis::LabelsFormat)), this, SLOT(axisLabelsFormatChanged(Axis::LabelsFormat)));
	connect(m_axis, SIGNAL(labelsAutoPrecisionChanged(bool)), this, SLOT(axisLabelsAutoPrecisionChanged(bool)));
	connect(m_axis, SIGNAL(labelsPrecisionChanged(int)), this, SLOT(axisLabelsPrecisionChanged(int)));
	connect(m_axis, &Axis::labelsDateTimeFormatChanged, this, &AxisDock::axisLabelsDateTimeFormatChanged);
	connect(m_axis, SIGNAL(labelsPositionChanged(Axis::LabelsPosition)), this, SLOT(axisLabelsPositionChanged(Axis::LabelsPosition)));
	connect(m_axis, SIGNAL(labelsOffsetChanged(double)), this, SLOT(axisLabelsOffsetChanged(double)));
	connect(m_axis, SIGNAL(labelsRotationAngleChanged(qreal)), this, SLOT(axisLabelsRotationAngleChanged(qreal)));
	connect(m_axis, SIGNAL(labelsFontChanged(QFont)), this, SLOT(axisLabelsFontChanged(QFont)));
	connect(m_axis, SIGNAL(labelsColorChanged(QColor)), this, SLOT(axisLabelsFontColorChanged(QColor)));
	connect(m_axis, SIGNAL(labelsPrefixChanged(QString)), this, SLOT(axisLabelsPrefixChanged(QString)));
	connect(m_axis, SIGNAL(labelsSuffixChanged(QString)), this, SLOT(axisLabelsSuffixChanged(QString)));
	connect(m_axis, SIGNAL(labelsOpacityChanged(qreal)), this, SLOT(axisLabelsOpacityChanged(qreal)));

	// grids
	connect(m_axis, SIGNAL(majorGridPenChanged(QPen)), this, SLOT(axisMajorGridPenChanged(QPen)));
	connect(m_axis, SIGNAL(majorGridOpacityChanged(qreal)), this, SLOT(axisMajorGridOpacityChanged(qreal)));
	connect(m_axis, SIGNAL(minorGridPenChanged(QPen)), this, SLOT(axisMinorGridPenChanged(QPen)));
	connect(m_axis, SIGNAL(minorGridOpacityChanged(qreal)), this, SLOT(axisMinorGridOpacityChanged(qreal)));

	connect(m_axis, SIGNAL(visibilityChanged(bool)), this, SLOT(axisVisibilityChanged(bool)));

	m_initializing = false;
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

//*************************************************************
//********** SLOTs for changes triggered in AxisDock **********
//*************************************************************
//"General"-tab
void AxisDock::nameChanged() {
	if (m_initializing)
		return;

	m_axis->setName(ui.leName->text());
}

void AxisDock::commentChanged() {
	if (m_initializing)
		return;

	m_axis->setComment(ui.leComment->text());
}

void AxisDock::visibilityChanged(bool state) {
	if (m_initializing)
		return;

	for (auto* axis : m_axesList)
		axis->setVisible(state);
}

/*!
	called if the orientation (horizontal or vertical) of the current axis is changed.
*/
void AxisDock::orientationChanged(int index) {
	auto orientation = (Axis::AxisOrientation)index;
	if (orientation == Axis::AxisHorizontal) {
		ui.cbPosition->setItemText(0, i18n("Top") );
		ui.cbPosition->setItemText(1, i18n("Bottom") );
		ui.cbLabelsPosition->setItemText(1, i18n("Top") );
		ui.cbLabelsPosition->setItemText(2, i18n("Bottom") );

		ui.cbScale->setItemText(1, QLatin1String("log(x)") );
		ui.cbScale->setItemText(2, QLatin1String("log2(x)") );
		ui.cbScale->setItemText(3, QLatin1String("ln(x)") );
		ui.cbScale->setItemText(4, QLatin1String("sqrt(x)") );
		ui.cbScale->setItemText(5, QLatin1String("x^2") );
	} else { //vertical
		ui.cbPosition->setItemText(0, i18n("Left") );
		ui.cbPosition->setItemText(1, i18n("Right") );
		ui.cbLabelsPosition->setItemText(1, i18n("Right") );
		ui.cbLabelsPosition->setItemText(2, i18n("Left") );

		ui.cbScale->setItemText(1, QLatin1String("log(y)") );
		ui.cbScale->setItemText(2, QLatin1String("log2(y)") );
		ui.cbScale->setItemText(3, QLatin1String("ln(y)") );
		ui.cbScale->setItemText(4, QLatin1String("sqrt(y)") );
		ui.cbScale->setItemText(5, QLatin1String("y^2") );
	}

	if (m_initializing)
		return;

	//depending on the current orientation we need to update axis possition and labels position

	//axis position, map from the current index in the combobox to the enum value in Axis::AxisPosition
	Axis::AxisPosition axisPosition;
	int posIndex = ui.cbPosition->currentIndex();
	if (orientation == Axis::AxisHorizontal) {
		if (posIndex > 1)
			posIndex += 2;
		axisPosition = Axis::AxisPosition(posIndex);
	} else {
		axisPosition = Axis::AxisPosition(posIndex+2);
	}

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

	if (index == 3)
		ui.lePosition->setVisible(true);
	else
		ui.lePosition->setVisible(false);

	if (m_initializing)
		return;

	//map from the current index in the combo box to the enum value in Axis::AxisPosition,
	//depends on the current orientation
	Axis::AxisPosition position;
	if ( ui.cbOrientation->currentIndex() == 0 ) {
		if (index>1)
			index += 2;
		position = Axis::AxisPosition(index);
	} else {
		position = Axis::AxisPosition(index+2);
	}

	for (auto* axis : m_axesList)
		axis->setPosition(position);
}

/*!
	called when the custom position of the axis in the corresponding LineEdit is changed.
*/
void AxisDock::positionChanged() {
	if (m_initializing)
		return;

	double offset = ui.lePosition->text().toDouble();
	for (auto* axis : m_axesList)
		axis->setOffset(offset);
}

void AxisDock::scaleChanged(int index) {
	if (m_initializing)
		return;

	auto scale = (Axis::AxisScale)index;
	for (auto* axis : m_axesList)
		axis->setScale(scale);
}

void AxisDock::autoScaleChanged(int index) {
	bool autoScale = index == Qt::Checked;
	ui.leStart->setEnabled(!autoScale);
	ui.leEnd->setEnabled(!autoScale);
	ui.dateTimeEditStart->setEnabled(!autoScale);
	ui.dateTimeEditEnd->setEnabled(!autoScale);

	if (m_initializing)
		return;

	for (auto* axis : m_axesList)
		axis->setAutoScale(autoScale);
}

void AxisDock::startChanged() {
	if (m_initializing)
		return;

	double value = ui.leStart->text().toDouble();

	//check first, whether the value for the lower limit is valid for the log- and square root scaling. If not, set the default values.
	auto scale = Axis::AxisScale(ui.cbScale->currentIndex());
	if (scale == Axis::ScaleLog10 || scale == Axis::ScaleLog2 || scale == Axis::ScaleLn) {
		if (value <= 0) {
			KMessageBox::sorry(this,
			                   i18n("The axes lower limit has a non-positive value. Default minimal value will be used."),
			                   i18n("Wrong lower limit value") );
			ui.leStart->setText( "0.01" );
			value = 0.01;
		}
	} else if (scale == Axis::ScaleSqrt) {
		if (value < 0) {
			KMessageBox::sorry(this,
			                   i18n("The axes lower limit has a negative value. Default minimal value will be used."),
			                   i18n("Wrong lower limit value") );
			ui.leStart->setText( "0" );
			value = 0;
		}
	}

	for (auto* axis : m_axesList)
		axis->setStart(value);
}

void AxisDock::endChanged() {
	if (m_initializing)
		return;

	double value = ui.leEnd->text().toDouble();
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
	if (m_initializing)
		return;

	double offset = ui.leZeroOffset->text().toDouble();
	for (auto* axis : m_axesList)
		axis->setZeroOffset(offset);
}

void AxisDock::scalingFactorChanged() {
	if (m_initializing)
		return;

	double scalingFactor = ui.leScalingFactor->text().toDouble();
	if (scalingFactor != 0.0)
		for (auto* axis : m_axesList)
			axis->setScalingFactor(scalingFactor);
}

// "Line"-tab
void AxisDock::lineStyleChanged(int index) {
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

void AxisDock::lineWidthChanged(double  value) {
	if (m_initializing)
		return;

	QPen pen;
	for (auto* axis : m_axesList) {
		pen = axis->linePen();
		pen.setWidthF(Worksheet::convertToSceneUnits(value, Worksheet::Point));
		axis->setLinePen(pen);
	}
}

void AxisDock::lineOpacityChanged(int value) {
	if (m_initializing)
		return;

	qreal opacity = (float)value/100.;
	for (auto* axis : m_axesList)
		axis->setLineOpacity(opacity);
}

void AxisDock::arrowTypeChanged(int index) {
	auto type = (Axis::ArrowType)index;
	if (type == Axis::NoArrow) {
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

	float v = Worksheet::convertToSceneUnits(value, Worksheet::Point);
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
	ui.lMajorTicksIncrementNumeric->setEnabled(b);
	ui.sbMajorTicksIncrementNumeric->setEnabled(b);
	ui.lMajorTicksIncrementDateTime->setEnabled(b);
	dtsbMajorTicksIncrement->setEnabled(b);
	ui.lMajorTicksLineStyle->setEnabled(b);
	ui.cbMajorTicksLineStyle->setEnabled(b);
	dtsbMinorTicksIncrement->setEnabled(b);
	if (b) {
		auto penStyle = Qt::PenStyle(ui.cbMajorTicksLineStyle->currentIndex());
		b = (penStyle != Qt::NoPen);
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
	if (type == Axis::TicksTotalNumber) {
		ui.lMajorTicksNumber->show();
		ui.sbMajorTicksNumber->show();
		ui.lMajorTicksIncrementNumeric->hide();
		ui.sbMajorTicksIncrementNumeric->hide();
		ui.lMajorTicksIncrementDateTime->hide();
		dtsbMajorTicksIncrement->hide();
		ui.lMajorTicksColumn->hide();
		cbMajorTicksColumn->hide();
	} else if (type == Axis::TicksIncrement) {
		ui.lMajorTicksNumber->hide();
		ui.sbMajorTicksNumber->hide();
		ui.lMajorTicksIncrementNumeric->show();

		const auto* plot = dynamic_cast<const CartesianPlot*>(m_axis->parentAspect());
		bool numeric = ( (m_axis->orientation() == Axis::AxisHorizontal && plot->xRangeFormat() == CartesianPlot::Numeric)
					|| (m_axis->orientation() == Axis::AxisVertical && plot->yRangeFormat() == CartesianPlot::Numeric) );
		if (numeric) {
			ui.lMajorTicksIncrementDateTime->hide();
			dtsbMajorTicksIncrement->hide();
			ui.lMajorTicksIncrementNumeric->show();
			ui.sbMajorTicksIncrementNumeric->show();
		} else {
			ui.lMajorTicksIncrementDateTime->show();
			dtsbMajorTicksIncrement->show();
			ui.lMajorTicksIncrementNumeric->hide();
			ui.sbMajorTicksIncrementNumeric->hide();
		}

		ui.lMajorTicksColumn->hide();
		cbMajorTicksColumn->hide();

		// Check if Increment is not to small
		majorTicksIncrementChanged();
	} else {
		ui.lMajorTicksNumber->hide();
		ui.sbMajorTicksNumber->hide();
		ui.lMajorTicksIncrementNumeric->hide();
		ui.sbMajorTicksIncrementNumeric->hide();
		dtsbMajorTicksIncrement->hide();
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

void AxisDock::majorTicksIncrementChanged() {
	if (m_initializing)
		return;

	const auto* plot = dynamic_cast<const CartesianPlot*>(m_axis->parentAspect());

	bool numeric = ( (m_axis->orientation() == Axis::AxisHorizontal && plot->xRangeFormat() == CartesianPlot::Numeric)
		|| (m_axis->orientation() == Axis::AxisVertical && plot->yRangeFormat() == CartesianPlot::Numeric) );

	double value = numeric ? ui.sbMajorTicksIncrementNumeric->value() : dtsbMajorTicksIncrement->value();
	double diff = m_axis->end() - m_axis->start();

	if (value == 0 || diff / value > 100 || value < 0) { // maximum of 100 ticks

		if (value == 0)
			value = diff / ui.sbMajorTicksNumber->value();

		if (diff / value > 100)
			value = diff / 100;

		// determine stepsize and number of decimals
		m_initializing = true;
		if (numeric) {
			int decimal = determineDecimals(value * 10);
			ui.sbMajorTicksIncrementNumeric->setDecimals(decimal);
			ui.sbMajorTicksIncrementNumeric->setSingleStep(determineStep(diff, decimal));
			ui.sbMajorTicksIncrementNumeric->setValue(value);
		} else
			dtsbMajorTicksIncrement->setValue(value);
		m_initializing = false;
	}

	for (auto* axis : m_axesList)
		axis->setMajorTicksIncrement(value);
}

void AxisDock::majorTicksLineStyleChanged(int index) {
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
		pen.setWidthF( Worksheet::convertToSceneUnits(value, Worksheet::Point) );
		axis->setMajorTicksPen(pen);
	}
}

void AxisDock::majorTicksLengthChanged(double value) {
	if (m_initializing)
		return;

	for (auto* axis : m_axesList)
		axis->setMajorTicksLength( Worksheet::convertToSceneUnits(value, Worksheet::Point) );
}

void AxisDock::majorTicksOpacityChanged(int value) {
	if (m_initializing)
		return;

	qreal opacity = (float)value/100.;
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
	ui.lMinorTicksIncrementNumeric->setEnabled(b);
	ui.sbMinorTicksIncrementNumeric->setEnabled(b);
	ui.lMinorTicksIncrementDateTime->setEnabled(b);
	dtsbMinorTicksIncrement->setEnabled(b);
	ui.lMinorTicksLineStyle->setEnabled(b);
	ui.cbMinorTicksLineStyle->setEnabled(b);
	if (b) {
		auto penStyle = Qt::PenStyle(ui.cbMinorTicksLineStyle->currentIndex());
		b = (penStyle != Qt::NoPen);
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
	if (type == Axis::TicksTotalNumber) {
		ui.lMinorTicksNumber->show();
		ui.sbMinorTicksNumber->show();
		ui.lMinorTicksIncrementNumeric->hide();
		ui.sbMinorTicksIncrementNumeric->hide();
		ui.lMinorTicksColumn->hide();
		cbMinorTicksColumn->hide();
		ui.lMinorTicksIncrementDateTime->hide();
		dtsbMinorTicksIncrement->hide();
	} else if ( type == Axis::TicksIncrement) {
		ui.lMinorTicksNumber->hide();
		ui.sbMinorTicksNumber->hide();

		const auto* plot = dynamic_cast<const CartesianPlot*>(m_axis->parentAspect());
		bool numeric = ( (m_axis->orientation() == Axis::AxisHorizontal && plot->xRangeFormat() == CartesianPlot::Numeric)
					|| (m_axis->orientation() == Axis::AxisVertical && plot->yRangeFormat() == CartesianPlot::Numeric) );
		if (numeric) {
			ui.lMinorTicksIncrementNumeric->show();
			ui.sbMinorTicksIncrementNumeric->show();
			ui.lMinorTicksIncrementDateTime->hide();
			dtsbMinorTicksIncrement->hide();
		} else {
			ui.lMinorTicksIncrementNumeric->hide();
			ui.sbMinorTicksIncrementNumeric->hide();
			ui.lMinorTicksIncrementDateTime->show();
			dtsbMinorTicksIncrement->show();
		}

		ui.lMinorTicksColumn->hide();
		cbMinorTicksColumn->hide();

		// Check if Increment is not to small
		minorTicksIncrementChanged();
	} else {
		ui.lMinorTicksNumber->hide();
		ui.sbMinorTicksNumber->hide();
		ui.lMinorTicksIncrementNumeric->hide();
		ui.sbMinorTicksIncrementNumeric->hide();
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

void AxisDock::minorTicksIncrementChanged() {
	if (m_initializing)
		return;

	const auto* plot = dynamic_cast<const CartesianPlot*>(m_axis->parentAspect());

	bool numeric = ( (m_axis->orientation() == Axis::AxisHorizontal && plot->xRangeFormat() == CartesianPlot::Numeric)
		|| (m_axis->orientation() == Axis::AxisVertical && plot->yRangeFormat() == CartesianPlot::Numeric) );

	double value = numeric ? ui.sbMinorTicksIncrementNumeric->value() : dtsbMinorTicksIncrement->value();
	double numberTicks = 0.0;

	if (value > 0)
		numberTicks = (m_axis->end() - m_axis->start()) / (m_axis->majorTicksNumber() - 1) / value -1; // recal

	if (value == 0 || numberTicks > 100 || value < 0) {
		if (value == 0)
			value = (m_axis->end() - m_axis->start()) / (m_axis->majorTicksNumber() - 1) / (ui.sbMinorTicksNumber->value() + 1);

		numberTicks = (m_axis->end() - m_axis->start()) / (m_axis->majorTicksNumber() - 1) / value -1; // recalculate number of ticks

		if (numberTicks > 100) // maximum 100 minor ticks
			value = (m_axis->end() - m_axis->start()) / (m_axis->majorTicksNumber() - 1) / (100 + 1);

		// determine stepsize and number of decimals
		m_initializing = true;
		if (numeric) {
			int decimal = determineDecimals(value * 10);
			ui.sbMinorTicksIncrementNumeric->setDecimals(decimal);
			ui.sbMinorTicksIncrementNumeric->setSingleStep(determineStep((m_axis->end() - m_axis->start()) / (m_axis->majorTicksNumber() - 1), decimal));
			ui.sbMinorTicksIncrementNumeric->setValue(value);
		} else
			dtsbMinorTicksIncrement->setValue(value);
		m_initializing = false;
	}

	for (auto* axis : m_axesList)
		axis->setMinorTicksIncrement(value);
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
		pen.setWidthF( Worksheet::convertToSceneUnits(value, Worksheet::Point) );
		axis->setMinorTicksPen(pen);
	}
}

void AxisDock::minorTicksLengthChanged(double value) {
	if (m_initializing)
		return;

	for (auto* axis : m_axesList)
		axis->setMinorTicksLength( Worksheet::convertToSceneUnits(value, Worksheet::Point) );
}

void AxisDock::minorTicksOpacityChanged(int value) {
	if (m_initializing)
		return;

	qreal opacity = (float)value/100.;
	for (auto* axis : m_axesList)
		axis->setMinorTicksOpacity(opacity);
}

//"Tick labels"-tab
void AxisDock::labelsFormatChanged(int index) {
	if (m_initializing)
		return;

	for (auto* axis : m_axesList)
		axis->setLabelsFormat(Axis::LabelsFormat(index));
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

void AxisDock::labelsDateTimeFormatChanged(int) {
	if (m_initializing)
		return;

	for (auto* axis : m_axesList)
		axis->setLabelsDateTimeFormat(ui.cbLabelsDateTimeFormat->currentText());
}

void AxisDock::labelsPositionChanged(int index) {
	auto position = Axis::LabelsPosition(index);

	bool b = (position != Axis::NoLabels);
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
		axis->setLabelsOffset( Worksheet::convertToSceneUnits(value, Worksheet::Point) );
}

void AxisDock::labelsRotationChanged(int value) {
	if (m_initializing)
		return;

	for (auto* axis : m_axesList)
		axis->setLabelsRotationAngle(value);
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
	labelsFont.setPixelSize( Worksheet::convertToSceneUnits(font.pointSizeF(), Worksheet::Point) );
	for (auto* axis : m_axesList)
		axis->setLabelsFont( labelsFont );
}

void AxisDock::labelsFontColorChanged(const QColor& color) {
	if (m_initializing)
		return;

	for (auto* axis : m_axesList)
		axis->setLabelsColor(color);
}

void AxisDock::labelsOpacityChanged(int value) {
	if (m_initializing)
		return;

	qreal opacity = (float)value/100.;
	for (auto* axis : m_axesList)
		axis->setLabelsOpacity(opacity);
}

// "Grid"-tab
//major grid
void AxisDock::majorGridStyleChanged(int index) {
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
		pen.setWidthF(Worksheet::convertToSceneUnits(value, Worksheet::Point));
		axis->setMajorGridPen(pen);
	}
}

void AxisDock::majorGridOpacityChanged(int value) {
	if (m_initializing)
		return;

	qreal opacity = (float)value/100.;
	for (auto* axis : m_axesList)
		axis->setMajorGridOpacity(opacity);
}

//minor grid
void AxisDock::minorGridStyleChanged(int index) {
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
		pen.setWidthF(Worksheet::convertToSceneUnits(value, Worksheet::Point));
		axis->setMinorGridPen(pen);
	}
}

void AxisDock::minorGridOpacityChanged(int value) {
	if (m_initializing)
		return;

	qreal opacity = (float)value/100.;
	for (auto* axis : m_axesList)
		axis->setMinorGridOpacity(opacity);
}

//*************************************************************
//************ SLOTs for changes triggered in Axis ************
//*************************************************************
void AxisDock::axisDescriptionChanged(const AbstractAspect* aspect) {
	if (m_axis != aspect)
		return;

	m_initializing = true;
	if (aspect->name() != ui.leName->text()) {
		ui.leName->setText(aspect->name());
	} else if (aspect->comment() != ui.leComment->text()) {
		ui.leComment->setText(aspect->comment());
	}
	m_initializing = false;
}

void AxisDock::axisOrientationChanged(Axis::AxisOrientation orientation) {
	m_initializing = true;
	ui.cbOrientation->setCurrentIndex( (int)orientation );
	m_initializing = false;
}

void AxisDock::axisPositionChanged(Axis::AxisPosition position) {
	m_initializing = true;

	//map from the enum Axis::AxisOrientation to the index in the combo box
	int index(position);
	if (index > 1)
		ui.cbPosition->setCurrentIndex(index-2);
	else
		ui.cbPosition->setCurrentIndex(index);

	m_initializing = false;
}

void AxisDock::axisPositionChanged(float value) {
	m_initializing = true;
	ui.lePosition->setText( QString::number(value) );
	m_initializing = false;
}

void AxisDock::axisScaleChanged(Axis::AxisScale scale) {
	m_initializing = true;
	ui.cbScale->setCurrentIndex( (int)scale );
	m_initializing = false;
}

void AxisDock::axisAutoScaleChanged(bool on) {
	m_initializing = true;
	ui.chkAutoScale->setChecked(on);
	m_initializing = false;
}

void AxisDock::axisStartChanged(double value) {
	m_initializing = true;
	ui.leStart->setText( QString::number(value) );
	ui.dateTimeEditStart->setDateTime( QDateTime::fromMSecsSinceEpoch(value) );

	// determine stepsize and number of decimals
	double diff = m_axis->end() - m_axis->start();
	int decimal = determineDecimals(diff);
	ui.sbMajorTicksIncrementNumeric->setDecimals(decimal);
	ui.sbMajorTicksIncrementNumeric->setSingleStep(determineStep(diff, decimal));
	m_initializing = false;
}

void AxisDock::axisEndChanged(double value) {
	m_initializing = true;
	ui.leEnd->setText( QString::number(value) );
	ui.dateTimeEditEnd->setDateTime( QDateTime::fromMSecsSinceEpoch(value) );
	ui.sbMajorTicksIncrementNumeric->setSingleStep(floor(m_axis->end() - m_axis->start())/10);

	// determine stepsize and number of decimals
	double diff = m_axis->end() - m_axis->start();
	int decimal = determineDecimals(diff);
	ui.sbMajorTicksIncrementNumeric->setDecimals(decimal);
	ui.sbMajorTicksIncrementNumeric->setSingleStep(determineStep(diff, decimal));
	m_initializing = false;
}

void AxisDock::axisZeroOffsetChanged(qreal value) {
	m_initializing = true;
	ui.leZeroOffset->setText( QString::number(value) );
	m_initializing = false;
}

void AxisDock::axisScalingFactorChanged(qreal value) {
	m_initializing = true;
	ui.leScalingFactor->setText( QString::number(value) );
	m_initializing = false;
}

//line
void AxisDock::axisLinePenChanged(const QPen& pen) {
	m_initializing = true;
	ui.cbLineStyle->setCurrentIndex( pen.style() );
	ui.kcbLineColor->setColor( pen.color() );
	GuiTools::updatePenStyles(ui.cbLineStyle, pen.color() );
	ui.sbLineWidth->setValue( Worksheet::convertFromSceneUnits(pen.widthF(), Worksheet::Point) );
	m_initializing = false;
}

void AxisDock::axisArrowTypeChanged(Axis::ArrowType type) {
	m_initializing = true;
	ui.cbArrowType->setCurrentIndex((int)type);
	m_initializing = false;
}

void AxisDock::axisLineOpacityChanged(qreal opacity) {
	m_initializing = true;
	ui.sbLineOpacity->setValue( round(opacity*100.0) );
	m_initializing = false;
}

void AxisDock::axisArrowPositionChanged(Axis::ArrowPosition position) {
	m_initializing = true;
	ui.cbArrowPosition->setCurrentIndex( (int)position );
	m_initializing = false;
}

void AxisDock::axisArrowSizeChanged(qreal size) {
	m_initializing = true;
	ui.sbArrowSize->setValue( (int)Worksheet::convertFromSceneUnits(size, Worksheet::Point) );
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
	ui.cbMajorTicksType->setCurrentIndex(type);
	m_initializing = false;
}
void AxisDock::axisMajorTicksNumberChanged(int number) {
	m_initializing = true;
	ui.sbMajorTicksNumber->setValue(number);
	m_initializing = false;
}
void AxisDock::axisMajorTicksIncrementChanged(qreal increment) {
	m_initializing = true;
	const auto* plot = dynamic_cast<const CartesianPlot*>(m_axis->parentAspect());
	if (plot) {
		bool numeric = ( (m_axis->orientation() == Axis::AxisHorizontal && plot->xRangeFormat() == CartesianPlot::Numeric)
			|| (m_axis->orientation() == Axis::AxisVertical && plot->yRangeFormat() == CartesianPlot::Numeric) );

		if (numeric)
			ui.sbMajorTicksIncrementNumeric->setValue(increment);
		else {
			dtsbMajorTicksIncrement->setValue(increment);
		}
	}
	m_initializing = false;
}
void AxisDock::axisMajorTicksPenChanged(const QPen& pen) {
	m_initializing = true;
	ui.cbMajorTicksLineStyle->setCurrentIndex(pen.style());
	ui.kcbMajorTicksColor->setColor(pen.color());
	ui.sbMajorTicksWidth->setValue( Worksheet::convertFromSceneUnits(pen.widthF(),Worksheet::Point) );
	m_initializing = false;
}
void AxisDock::axisMajorTicksLengthChanged(qreal length) {
	m_initializing = true;
	ui.sbMajorTicksLength->setValue( Worksheet::convertFromSceneUnits(length,Worksheet::Point) );
	m_initializing = false;
}
void AxisDock::axisMajorTicksOpacityChanged(qreal opacity) {
	m_initializing = true;
	ui.sbMajorTicksOpacity->setValue( round(opacity*100.0));
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
	ui.cbMinorTicksType->setCurrentIndex(type);
	m_initializing = false;
}
void AxisDock::axisMinorTicksNumberChanged(int number) {
	m_initializing = true;
	ui.sbMinorTicksNumber->setValue(number);
	m_initializing = false;
}
void AxisDock::axisMinorTicksIncrementChanged(qreal increment) {
	m_initializing = true;
	const auto* plot = dynamic_cast<const CartesianPlot*>(m_axis->parentAspect());
	if (plot) {
		bool numeric = ( (m_axis->orientation() == Axis::AxisHorizontal && plot->xRangeFormat() == CartesianPlot::Numeric)
			|| (m_axis->orientation() == Axis::AxisVertical && plot->yRangeFormat() == CartesianPlot::Numeric) );

		if (numeric)
			ui.sbMinorTicksIncrementNumeric->setValue(increment);
		else {
			dtsbMinorTicksIncrement->setValue(increment);
		}
	}
	m_initializing = false;
}
void AxisDock::axisMinorTicksPenChanged(const QPen& pen) {
	m_initializing = true;
	ui.cbMinorTicksLineStyle->setCurrentIndex(pen.style());
	ui.kcbMinorTicksColor->setColor(pen.color());
	ui.sbMinorTicksWidth->setValue( Worksheet::convertFromSceneUnits(pen.widthF(),Worksheet::Point) );
	m_initializing = false;
}
void AxisDock::axisMinorTicksLengthChanged(qreal length) {
	m_initializing = true;
	ui.sbMinorTicksLength->setValue( Worksheet::convertFromSceneUnits(length,Worksheet::Point) );
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
	ui.cbLabelsFormat->setCurrentIndex(format);
	m_initializing = false;
}
void AxisDock::axisLabelsAutoPrecisionChanged(bool on) {
	m_initializing = true;
	ui.chkLabelsAutoPrecision->setChecked((int) on);
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
	ui.cbLabelsPosition->setCurrentIndex(position);
	m_initializing = false;
}
void AxisDock::axisLabelsOffsetChanged(double offset) {
	m_initializing = true;
	ui.sbLabelsOffset->setValue( Worksheet::convertFromSceneUnits(offset, Worksheet::Point) );
	m_initializing = false;
}
void AxisDock::axisLabelsRotationAngleChanged(qreal rotation) {
	m_initializing = true;
	ui.sbLabelsRotation->setValue(rotation);
	m_initializing = false;
}
void AxisDock::axisLabelsFontChanged(const QFont& font) {
	m_initializing = true;
	//we need to set the font size in points for KFontRequester
	QFont newFont(font);
	newFont.setPointSizeF( round(Worksheet::convertFromSceneUnits(font.pixelSize(), Worksheet::Point)) );
	ui.kfrLabelsFont->setFont(newFont);
	m_initializing = false;
}
void AxisDock::axisLabelsFontColorChanged(const QColor& color) {
	m_initializing = true;
	ui.kcbLabelsFontColor->setColor(color);
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
	ui.sbLabelsOpacity->setValue( round(opacity*100.0) );
	m_initializing = false;
}

//grid
void AxisDock::axisMajorGridPenChanged(const QPen& pen) {
	m_initializing = true;
	ui.cbMajorGridStyle->setCurrentIndex((int) pen.style());
	ui.kcbMajorGridColor->setColor(pen.color());
	GuiTools::updatePenStyles(ui.cbMajorGridStyle, pen.color());
	ui.sbMajorGridWidth->setValue(Worksheet::convertFromSceneUnits(pen.widthF(),Worksheet::Point));
	m_initializing = false;
}
void AxisDock::axisMajorGridOpacityChanged(qreal opacity) {
	m_initializing = true;
	ui.sbMajorGridOpacity->setValue( round(opacity*100.0) );
	m_initializing = false;
}
void AxisDock::axisMinorGridPenChanged(const QPen& pen) {
	m_initializing = true;
	ui.cbMinorGridStyle->setCurrentIndex((int) pen.style());
	ui.kcbMinorGridColor->setColor(pen.color());
	GuiTools::updatePenStyles(ui.cbMinorGridStyle, pen.color());
	ui.sbMinorGridWidth->setValue(Worksheet::convertFromSceneUnits(pen.widthF(),Worksheet::Point));
	m_initializing = false;
}
void AxisDock::axisMinorGridOpacityChanged(qreal opacity) {
	m_initializing = true;
	ui.sbMinorGridOpacity->setValue( round(opacity*100.0) );
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
	ui.cbOrientation->setCurrentIndex( (int) m_axis->orientation() );

	int index = (int)m_axis->position();
	if (index > 1)
		ui.cbPosition->setCurrentIndex(index-2);
	else
		ui.cbPosition->setCurrentIndex(index);

	ui.lePosition->setText( QString::number( m_axis->offset()) );
	ui.cbScale->setCurrentIndex(  (int) m_axis->scale() );
	ui.chkAutoScale->setChecked( m_axis->autoScale() );
	ui.leStart->setText( QString::number(m_axis->start()) );
	ui.leEnd->setText( QString::number(m_axis->end()) );


	ui.sbMajorTicksIncrementNumeric->setDecimals(0);
	ui.sbMajorTicksIncrementNumeric->setSingleStep(m_axis->majorTicksIncrement());

	//depending on range format of the axis (numeric vs. datetime), show/hide the corresponding widgets
	const auto* plot = dynamic_cast<const CartesianPlot*>(m_axis->parentAspect());
	if (plot) {
		bool numeric = ( (m_axis->orientation() == Axis::AxisHorizontal && plot->xRangeFormat() == CartesianPlot::Numeric)
			|| (m_axis->orientation() == Axis::AxisVertical && plot->yRangeFormat() == CartesianPlot::Numeric) );
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
		ui.cbLabelsDateTimeFormat->setVisible(numeric);
		ui.lLabelsDateTimeFormat->setVisible(!numeric);
		ui.cbLabelsDateTimeFormat->setVisible(!numeric);

		if (!numeric) {
			if (m_axis->orientation() == Axis::AxisHorizontal) {
				ui.dateTimeEditStart->setDisplayFormat(plot->xRangeDateTimeFormat());
				ui.dateTimeEditEnd->setDisplayFormat(plot->xRangeDateTimeFormat());
			} else {
				ui.dateTimeEditStart->setDisplayFormat(plot->yRangeDateTimeFormat());
				ui.dateTimeEditEnd->setDisplayFormat(plot->yRangeDateTimeFormat());
			}
			ui.dateTimeEditStart->setDateTime(QDateTime::fromMSecsSinceEpoch(m_axis->start()));
			ui.dateTimeEditEnd->setDateTime(QDateTime::fromMSecsSinceEpoch(m_axis->end()));

		}
	}

	ui.leZeroOffset->setText( QString::number(m_axis->zeroOffset()) );
	ui.leScalingFactor->setText( QString::number(m_axis->scalingFactor()) );

	//Line
	ui.cbLineStyle->setCurrentIndex( (int) m_axis->linePen().style() );
	ui.kcbLineColor->setColor( m_axis->linePen().color() );
	ui.sbLineWidth->setValue( Worksheet::convertFromSceneUnits(m_axis->linePen().widthF(),Worksheet::Point) );
	ui.sbLineOpacity->setValue( round(m_axis->lineOpacity()*100.0) );
	ui.cbArrowType->setCurrentIndex( (int)m_axis->arrowType() );
	ui.cbArrowPosition->setCurrentIndex( (int)m_axis->arrowPosition() );
	ui.sbArrowSize->setValue( (int)Worksheet::convertFromSceneUnits(m_axis->arrowSize(), Worksheet::Point) );

	//Major ticks
	ui.cbMajorTicksDirection->setCurrentIndex( (int) m_axis->majorTicksDirection() );
	ui.cbMajorTicksType->setCurrentIndex( (int) m_axis->majorTicksType() );
	ui.sbMajorTicksNumber->setValue( m_axis->majorTicksNumber() );
	ui.cbMajorTicksLineStyle->setCurrentIndex( (int) m_axis->majorTicksPen().style() );
	ui.kcbMajorTicksColor->setColor( m_axis->majorTicksPen().color() );
	ui.sbMajorTicksWidth->setValue( Worksheet::convertFromSceneUnits( m_axis->majorTicksPen().widthF(),Worksheet::Point) );
	ui.sbMajorTicksLength->setValue( Worksheet::convertFromSceneUnits( m_axis->majorTicksLength(),Worksheet::Point) );
	ui.sbMajorTicksOpacity->setValue( round(m_axis->majorTicksOpacity()*100.0) );

	//Minor ticks
	ui.cbMinorTicksDirection->setCurrentIndex( (int) m_axis->minorTicksDirection() );
	ui.cbMinorTicksType->setCurrentIndex( (int) m_axis->minorTicksType() );
	ui.sbMinorTicksNumber->setValue( m_axis->minorTicksNumber() );
	ui.cbMinorTicksLineStyle->setCurrentIndex( (int) m_axis->minorTicksPen().style() );
	ui.kcbMinorTicksColor->setColor( m_axis->minorTicksPen().color() );
	ui.sbMinorTicksWidth->setValue( Worksheet::convertFromSceneUnits(m_axis->minorTicksPen().widthF(),Worksheet::Point) );
	ui.sbMinorTicksLength->setValue( Worksheet::convertFromSceneUnits(m_axis->minorTicksLength(),Worksheet::Point) );
	ui.sbMinorTicksOpacity->setValue( round(m_axis->minorTicksOpacity()*100.0) );

	//Extra ticks
	//TODO

	// Tick label
	ui.cbLabelsPosition->setCurrentIndex( (int) m_axis->labelsPosition() );
	ui.sbLabelsOffset->setValue( Worksheet::convertFromSceneUnits(m_axis->labelsOffset(),Worksheet::Point) );
	ui.sbLabelsRotation->setValue( m_axis->labelsRotationAngle() );
	ui.cbLabelsFormat->setCurrentIndex( (int) m_axis->labelsFormat() );
	ui.chkLabelsAutoPrecision->setChecked( (int) m_axis->labelsAutoPrecision() );
	ui.sbLabelsPrecision->setValue( (int)m_axis->labelsPrecision() );
	ui.cbLabelsDateTimeFormat->setCurrentText(m_axis->labelsDateTimeFormat());
	//we need to set the font size in points for KFontRequester
	QFont font = m_axis->labelsFont();
	font.setPointSizeF( round(Worksheet::convertFromSceneUnits(font.pixelSize(), Worksheet::Point)) );
	ui.kfrLabelsFont->setFont( font );
	ui.kcbLabelsFontColor->setColor( m_axis->labelsColor() );
	ui.leLabelsPrefix->setText( m_axis->labelsPrefix() );
	ui.leLabelsSuffix->setText( m_axis->labelsSuffix() );
	ui.sbLabelsOpacity->setValue( round(m_axis->labelsOpacity()*100.0) );

	//Grid
	ui.cbMajorGridStyle->setCurrentIndex( (int) m_axis->majorGridPen().style() );
	ui.kcbMajorGridColor->setColor( m_axis->majorGridPen().color() );
	ui.sbMajorGridWidth->setValue( Worksheet::convertFromSceneUnits(m_axis->majorGridPen().widthF(),Worksheet::Point) );
	ui.sbMajorGridOpacity->setValue( round(m_axis->majorGridOpacity()*100.0) );

	ui.cbMinorGridStyle->setCurrentIndex( (int) m_axis->minorGridPen().style() );
	ui.kcbMinorGridColor->setColor( m_axis->minorGridPen().color() );
	ui.sbMinorGridWidth->setValue( Worksheet::convertFromSceneUnits(m_axis->minorGridPen().widthF(),Worksheet::Point) );
	ui.sbMinorGridOpacity->setValue( round(m_axis->minorGridOpacity()*100.0) );

	GuiTools::updatePenStyles(ui.cbLineStyle, ui.kcbLineColor->color());
	this->majorTicksTypeChanged(ui.cbMajorTicksType->currentIndex());
	GuiTools::updatePenStyles(ui.cbMajorTicksLineStyle, ui.kcbMajorTicksColor->color());
	this->minorTicksTypeChanged(ui.cbMinorTicksType->currentIndex());
	GuiTools::updatePenStyles(ui.cbMinorTicksLineStyle, ui.kcbMinorTicksColor->color());
	GuiTools::updatePenStyles(ui.cbMajorGridStyle, ui.kcbMajorGridColor->color());
	GuiTools::updatePenStyles(ui.cbMinorGridStyle, ui.kcbMinorGridColor->color());
}

/*!
 * Determine the number of decimals for using in a QDoubleSpinBox
 * \param diff
 * \return
 */
int AxisDock::determineDecimals(double diff) {
	diff /= 10; // step one decimal before
	double power10 = 1;
	for (int i = 0; i < 10; i++) {
		double nearest = round(diff * power10) / power10;
		if (nearest > 0) {
			return i;
		}
		power10 *= 10;
	}

	return 10;
}

/*!
 * Determine the step in a QDoubleSpinBox with specific decimals and diff
 * \param diff Difference between the largest value and smallest value
 * \param decimal
 * \return
 */
double AxisDock::determineStep(double diff, int decimal) {
	double ten = 1;
	if (decimal == 0) {
		for (unsigned int i = 1; i < 1000000000; i++) {
			if (diff/ten <= 10) {
				return ten/10; // use one decimal before
			}
			ten *= 10;
		}
		return 1;
	}

	return static_cast<double>(1)/(pow(10,decimal));
}

void AxisDock::loadConfigFromTemplate(KConfig& config) {
	//extract the name of the template from the file name
	QString name;
	int index = config.name().lastIndexOf(QDir::separator());
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

	bool numeric = false;
	const auto* plot = dynamic_cast<const CartesianPlot*>(m_axis->parentAspect());
	if (plot) {
		numeric = ( (m_axis->orientation() == Axis::AxisHorizontal && plot->xRangeFormat() == CartesianPlot::Numeric)
			|| (m_axis->orientation() == Axis::AxisVertical && plot->yRangeFormat() == CartesianPlot::Numeric) );
	}

	//General
	ui.cbOrientation->setCurrentIndex( group.readEntry("Orientation", (int) m_axis->orientation()) );

	int index = group.readEntry("Position", (int) m_axis->position());
	if (index > 1)
		ui.cbPosition->setCurrentIndex(index-2);
	else
		ui.cbPosition->setCurrentIndex(index);

	ui.lePosition->setText( QString::number( group.readEntry("PositionOffset", m_axis->offset())) );
	ui.cbScale->setCurrentIndex( group.readEntry("Scale", (int) m_axis->scale()) );
	ui.chkAutoScale->setChecked(group.readEntry("AutoScale", m_axis->autoScale()));
	ui.leStart->setText( QString::number( group.readEntry("Start", m_axis->start())) );
	ui.leEnd->setText( QString::number( group.readEntry("End", m_axis->end())) );
	ui.leZeroOffset->setText( QString::number( group.readEntry("ZeroOffset", m_axis->zeroOffset())) );
	ui.leScalingFactor->setText( QString::number( group.readEntry("ScalingFactor", m_axis->scalingFactor())) );

	//Title
	KConfigGroup axisLabelGroup = config.group("AxisLabel");
	labelWidget->loadConfig(axisLabelGroup);

	//Line
	ui.cbLineStyle->setCurrentIndex( group.readEntry("LineStyle", (int) m_axis->linePen().style()) );
	ui.kcbLineColor->setColor( group.readEntry("LineColor", m_axis->linePen().color()) );
	ui.sbLineWidth->setValue( Worksheet::convertFromSceneUnits(group.readEntry("LineWidth", m_axis->linePen().widthF()),Worksheet::Point) );
	ui.sbLineOpacity->setValue( round(group.readEntry("LineOpacity", m_axis->lineOpacity())*100.0) );
	ui.cbArrowType->setCurrentIndex( group.readEntry("ArrowType", (int) m_axis->arrowType()) );
	ui.cbArrowPosition->setCurrentIndex( group.readEntry("ArrowPosition", (int) m_axis->arrowPosition()) );
	ui.sbArrowSize->setValue( Worksheet::convertFromSceneUnits(group.readEntry("ArrowSize", m_axis->arrowSize()), Worksheet::Point) );

	//Major ticks
	ui.cbMajorTicksDirection->setCurrentIndex( group.readEntry("MajorTicksDirection", (int) m_axis->majorTicksDirection()) );
	ui.cbMajorTicksType->setCurrentIndex( group.readEntry("MajorTicksType", (int) m_axis->majorTicksType()) );
	ui.sbMajorTicksNumber->setValue( group.readEntry("MajorTicksNumber", m_axis->majorTicksNumber()) );
	if (numeric)
		ui.sbMajorTicksIncrementNumeric->setValue(group.readEntry("MajorTicksIncrement", m_axis->majorTicksIncrement()));
	else
		dtsbMajorTicksIncrement->setValue(group.readEntry("MajorTicksIncrement", m_axis->majorTicksIncrement()));
	ui.cbMajorTicksLineStyle->setCurrentIndex( group.readEntry("MajorTicksLineStyle", (int) m_axis->majorTicksPen().style()) );
	ui.kcbMajorTicksColor->setColor( group.readEntry("MajorTicksColor", m_axis->majorTicksPen().color()) );
	ui.sbMajorTicksWidth->setValue( Worksheet::convertFromSceneUnits(group.readEntry("MajorTicksWidth", m_axis->majorTicksPen().widthF()),Worksheet::Point) );
	ui.sbMajorTicksLength->setValue( Worksheet::convertFromSceneUnits(group.readEntry("MajorTicksLength", m_axis->majorTicksLength()),Worksheet::Point) );
	ui.sbMajorTicksOpacity->setValue( round(group.readEntry("MajorTicksOpacity", m_axis->majorTicksOpacity())*100.0) );

	//Minor ticks
	ui.cbMinorTicksDirection->setCurrentIndex( group.readEntry("MinorTicksDirection", (int) m_axis->minorTicksDirection()) );
	ui.cbMinorTicksType->setCurrentIndex( group.readEntry("MinorTicksType", (int) m_axis->minorTicksType()) );
	ui.sbMinorTicksNumber->setValue( group.readEntry("MinorTicksNumber", m_axis->minorTicksNumber()) );
	if (numeric)
		ui.sbMinorTicksIncrementNumeric->setValue(group.readEntry("MajorTicksIncrement", m_axis->majorTicksIncrement()));
	else
		dtsbMinorTicksIncrement->setValue(group.readEntry("MajorTicksIncrement", m_axis->majorTicksIncrement()));
	ui.cbMinorTicksLineStyle->setCurrentIndex( group.readEntry("MinorTicksLineStyle", (int) m_axis->minorTicksPen().style()) );
	ui.kcbMinorTicksColor->setColor( group.readEntry("MinorTicksColor", m_axis->minorTicksPen().color()) );
	ui.sbMinorTicksWidth->setValue( Worksheet::convertFromSceneUnits(group.readEntry("MinorTicksWidth", m_axis->minorTicksPen().widthF()),Worksheet::Point) );
	ui.sbMinorTicksLength->setValue( Worksheet::convertFromSceneUnits(group.readEntry("MinorTicksLength", m_axis->minorTicksLength()),Worksheet::Point) );
	ui.sbMinorTicksOpacity->setValue( round(group.readEntry("MinorTicksOpacity", m_axis->minorTicksOpacity())*100.0) );

	//Extra ticks
	//TODO

	// Tick label
	ui.cbLabelsFormat->setCurrentIndex( group.readEntry("LabelsFormat", (int) m_axis->labelsFormat()) );
	ui.chkLabelsAutoPrecision->setChecked( group.readEntry("LabelsAutoPrecision", (int) m_axis->labelsAutoPrecision()) );
	ui.sbLabelsPrecision->setValue( group.readEntry("LabelsPrecision", (int)m_axis->labelsPrecision()) );
	ui.cbLabelsDateTimeFormat->setCurrentText( group.readEntry("LabelsDateTimeFormat", "yyyy-MM-dd hh:mm:ss") );
	ui.cbLabelsPosition->setCurrentIndex( group.readEntry("LabelsPosition", (int) m_axis->labelsPosition()) );
	ui.sbLabelsOffset->setValue( Worksheet::convertFromSceneUnits(group.readEntry("LabelsOffset", m_axis->labelsOffset()), Worksheet::Point) );
	ui.sbLabelsRotation->setValue( group.readEntry("LabelsRotation", m_axis->labelsRotationAngle()) );
	//we need to set the font size in points for KFontRequester
	QFont font = m_axis->labelsFont();
	font.setPointSizeF( round(Worksheet::convertFromSceneUnits(font.pixelSize(), Worksheet::Point)) );
	ui.kfrLabelsFont->setFont( group.readEntry("LabelsFont", font) );
	ui.kcbLabelsFontColor->setColor( group.readEntry("LabelsFontColor", m_axis->labelsColor()) );
	ui.leLabelsPrefix->setText( group.readEntry("LabelsPrefix", m_axis->labelsPrefix()) );
	ui.leLabelsSuffix->setText( group.readEntry("LabelsSuffix", m_axis->labelsSuffix()) );
	ui.sbLabelsOpacity->setValue( round(group.readEntry("LabelsOpacity", m_axis->labelsOpacity())*100.0) );

	//Grid
	ui.cbMajorGridStyle->setCurrentIndex( group.readEntry("MajorGridStyle", (int) m_axis->majorGridPen().style()) );
	ui.kcbMajorGridColor->setColor( group.readEntry("MajorGridColor", m_axis->majorGridPen().color()) );
	ui.sbMajorGridWidth->setValue( Worksheet::convertFromSceneUnits(group.readEntry("MajorGridWidth", m_axis->majorGridPen().widthF()),Worksheet::Point) );
	ui.sbMajorGridOpacity->setValue( round(group.readEntry("MajorGridOpacity", m_axis->majorGridOpacity())*100.0) );

	ui.cbMinorGridStyle->setCurrentIndex( group.readEntry("MinorGridStyle", (int) m_axis->minorGridPen().style()) );
	ui.kcbMinorGridColor->setColor( group.readEntry("MinorGridColor", m_axis->minorGridPen().color()) );
	ui.sbMinorGridWidth->setValue( Worksheet::convertFromSceneUnits(group.readEntry("MinorGridWidth", m_axis->minorGridPen().widthF()),Worksheet::Point) );
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
	if (plot) {
		numeric = ( (m_axis->orientation() == Axis::AxisHorizontal && plot->xRangeFormat() == CartesianPlot::Numeric)
			|| (m_axis->orientation() == Axis::AxisVertical && plot->yRangeFormat() == CartesianPlot::Numeric) );
	}


	//General
	group.writeEntry("Orientation", ui.cbOrientation->currentIndex());

	if (ui.cbPosition->currentIndex() == 2) {
		group.writeEntry("Position", (int)Axis::AxisCentered);
	} else if (ui.cbPosition->currentIndex() == 3) {
		group.writeEntry("Position", (int)Axis::AxisCustom);
	} else {
		if ( ui.cbOrientation->currentIndex() == Axis::AxisHorizontal )
			group.writeEntry("Position", ui.cbPosition->currentIndex());
		else
			group.writeEntry("Position", ui.cbPosition->currentIndex()+2);
	}

	group.writeEntry("PositionOffset", ui.lePosition->text());
	group.writeEntry("Scale", ui.cbScale->currentIndex());
	group.writeEntry("Start", ui.leStart->text());
	group.writeEntry("End", ui.leEnd->text());
	group.writeEntry("ZeroOffset", ui.leZeroOffset->text());
	group.writeEntry("ScalingFactor", ui.leScalingFactor->text());

	//Title
	KConfigGroup axisLabelGroup = config.group("AxisLabel");
	labelWidget->saveConfig(axisLabelGroup);

	//Line
	group.writeEntry("LineStyle", ui.cbLineStyle->currentIndex());
	group.writeEntry("LineColor", ui.kcbLineColor->color());
	group.writeEntry("LineWidth", Worksheet::convertToSceneUnits(ui.sbLineWidth->value(), Worksheet::Point));
	group.writeEntry("LineOpacity", ui.sbLineOpacity->value()/100);

	//Major ticks
	group.writeEntry("MajorTicksDirection", ui.cbMajorTicksDirection->currentIndex());
	group.writeEntry("MajorTicksType", ui.cbMajorTicksType->currentIndex());
	group.writeEntry("MajorTicksNumber", ui.sbMajorTicksNumber->value());
	if (numeric)
		group.writeEntry("MajorTicksIncrement", QString::number(ui.sbMajorTicksIncrementNumeric->value()));
	else
		group.writeEntry("MajorTicksIncrement", QString::number(dtsbMajorTicksIncrement->value()));
	group.writeEntry("MajorTicksLineStyle", ui.cbMajorTicksLineStyle->currentIndex());
	group.writeEntry("MajorTicksColor", ui.kcbMajorTicksColor->color());
	group.writeEntry("MajorTicksWidth", Worksheet::convertToSceneUnits(ui.sbMajorTicksWidth->value(),Worksheet::Point));
	group.writeEntry("MajorTicksLength", Worksheet::convertToSceneUnits(ui.sbMajorTicksLength->value(),Worksheet::Point));
	group.writeEntry("MajorTicksOpacity", ui.sbMajorTicksOpacity->value()/100);

	//Minor ticks
	group.writeEntry("MinorTicksDirection", ui.cbMinorTicksDirection->currentIndex());
	group.writeEntry("MinorTicksType", ui.cbMinorTicksType->currentIndex());
	group.writeEntry("MinorTicksNumber", ui.sbMinorTicksNumber->value());
	if (numeric)
		group.writeEntry("MinorTicksIncrement", QString::number(ui.sbMinorTicksIncrementNumeric->value()));
	else
		group.writeEntry("MinorTicksIncrement", QString::number(dtsbMinorTicksIncrement->value()));
	group.writeEntry("MinorTicksLineStyle", ui.cbMinorTicksLineStyle->currentIndex());
	group.writeEntry("MinorTicksColor", ui.kcbMinorTicksColor->color());
	group.writeEntry("MinorTicksWidth", Worksheet::convertFromSceneUnits(ui.sbMinorTicksWidth->value(),Worksheet::Point));
	group.writeEntry("MinorTicksLength", Worksheet::convertFromSceneUnits(ui.sbMinorTicksLength->value(),Worksheet::Point));
	group.writeEntry("MinorTicksOpacity", ui.sbMinorTicksOpacity->value()/100);

	//Extra ticks
	// TODO

	// Tick label
	group.writeEntry("LabelsFormat", ui.cbLabelsFormat->currentIndex());
	group.writeEntry("LabelsAutoPrecision", ui.chkLabelsAutoPrecision->isChecked());
	group.writeEntry("LabelsPrecision", ui.sbLabelsPrecision->value());
	group.writeEntry("LabelsPosition", ui.cbLabelsPosition->currentIndex());
	group.writeEntry("LabelsOffset", Worksheet::convertToSceneUnits(ui.sbLabelsOffset->value(), Worksheet::Point));
	group.writeEntry("LabelsRotation", ui.sbLabelsRotation->value());
	group.writeEntry("LabelsFont", ui.kfrLabelsFont->font());
	group.writeEntry("LabelsFontColor", ui.kcbLabelsFontColor->color());
	group.writeEntry("LabelsPrefix", ui.leLabelsPrefix->text());
	group.writeEntry("LabelsSuffix", ui.leLabelsSuffix->text());
	group.writeEntry("LabelsOpacity", ui.sbLabelsOpacity->value()/100);

	//Grid
	group.writeEntry("MajorGridStyle", ui.cbMajorGridStyle->currentIndex());
	group.writeEntry("MajorGridColor", ui.kcbMajorGridColor->color());
	group.writeEntry("MajorGridWidth", Worksheet::convertToSceneUnits(ui.sbMajorGridWidth->value(), Worksheet::Point));
	group.writeEntry("MajorGridOpacity", ui.sbMajorGridOpacity->value()/100);

	group.writeEntry("MinorGridStyle", ui.cbMinorGridStyle->currentIndex());
	group.writeEntry("MinorGridColor", ui.kcbMinorGridColor->color());
	group.writeEntry("MinorGridWidth", Worksheet::convertToSceneUnits(ui.sbMinorGridWidth->value(), Worksheet::Point));
	group.writeEntry("MinorGridOpacity", ui.sbMinorGridOpacity->value()/100);
	config.sync();
}
