/***************************************************************************
    File                 : CartesianPlotDock.cpp
    Project              : LabPlot
    Description          : widget for cartesian plot properties
    --------------------------------------------------------------------
    Copyright            : (C) 2011-2020 by Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2012-2013 by Stefan Gerlach (stefan.gerlach@uni-konstanz.de)

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

#include "CartesianPlotDock.h"
#include "backend/worksheet/plots/PlotArea.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"
#include "backend/core/column/Column.h"

#include "kdefrontend/widgets/LabelWidget.h"
#include "kdefrontend/GuiTools.h"
#include "kdefrontend/TemplateHandler.h"
#include "kdefrontend/ThemeHandler.h"

#include <QCompleter>
#include <QPainter>
#include <QTimer>
#include <QDir>
#include <QDirModel>
#include <QFileDialog>
#include <QImageReader>
#include <QButtonGroup>
#include <QIntValidator>

#include <KSharedConfig>

/*!
  \class CartesianPlotDock
  \brief  Provides a widget for editing the properties of the cartesian plot currently selected in the project explorer.

  \ingroup kdefrontend
*/

CartesianPlotDock::CartesianPlotDock(QWidget* parent) : BaseDock(parent) {
	ui.setupUi(this);
	m_leName = ui.leName;
	m_leComment = ui.leComment;

	//"General"-tab
	auto* rangeButtonsGroup(new QButtonGroup);
	rangeButtonsGroup->addButton(ui.rbRangeFirst);
	rangeButtonsGroup->addButton(ui.rbRangeLast);
	rangeButtonsGroup->addButton(ui.rbRangeFree);

	//"Range breaks"-tab
	ui.bAddXBreak->setIcon( QIcon::fromTheme("list-add") );
	ui.bRemoveXBreak->setIcon( QIcon::fromTheme("list-remove") );
	ui.cbXBreak->addItem("1");

	ui.bAddYBreak->setIcon( QIcon::fromTheme("list-add") );
	ui.bRemoveYBreak->setIcon( QIcon::fromTheme("list-remove") );
	ui.cbYBreak->addItem("1");

	//"Background"-tab
	ui.bOpen->setIcon( QIcon::fromTheme("document-open") );

	ui.leBackgroundFileName->setCompleter(new QCompleter(new QDirModel, this));

	//"Title"-tab
	auto* hboxLayout = new QHBoxLayout(ui.tabTitle);
	labelWidget = new LabelWidget(ui.tabTitle);
	hboxLayout->addWidget(labelWidget);
	hboxLayout->setContentsMargins(2,2,2,2);
	hboxLayout->setSpacing(2);
	//adjust layouts in the tabs
	for (int i = 0; i < ui.tabWidget->count(); ++i) {
		auto* layout = qobject_cast<QGridLayout*>(ui.tabWidget->widget(i)->layout());
		if (!layout)
			continue;

		layout->setContentsMargins(2,2,2,2);
		layout->setHorizontalSpacing(2);
		layout->setVerticalSpacing(2);
	}

	// "Cursor"-tab
	QStringList list = {i18n("NoPen"), i18n("SolidLine"), i18n("DashLine"), i18n("DotLine"), i18n("DashDotLine"), i18n("DashDotDotLine")};
	ui.cbCursorLineStyle->clear();
	for (int i = 0; i < list.count(); i++)
		ui.cbCursorLineStyle->addItem(list[i], i);

	//Validators
	ui.leRangeFirst->setValidator( new QIntValidator(ui.leRangeFirst) );
	ui.leRangeLast->setValidator( new QIntValidator(ui.leRangeLast) );
	ui.leXBreakStart->setValidator( new QDoubleValidator(ui.leXBreakStart) );
	ui.leXBreakEnd->setValidator( new QDoubleValidator(ui.leXBreakEnd) );
	ui.leYBreakStart->setValidator( new QDoubleValidator(ui.leYBreakStart) );
	ui.leYBreakEnd->setValidator( new QDoubleValidator(ui.leYBreakEnd) );

	//SIGNAL/SLOT
	//General
	connect(ui.leName, &QLineEdit::textChanged, this, &CartesianPlotDock::nameChanged);
	connect(ui.leComment, &QLineEdit::textChanged, this, &CartesianPlotDock::commentChanged);
	connect( ui.chkVisible, SIGNAL(clicked(bool)), this, SLOT(visibilityChanged(bool)) );
	connect( ui.sbLeft, SIGNAL(valueChanged(double)), this, SLOT(geometryChanged()) );
	connect( ui.sbTop, SIGNAL(valueChanged(double)), this, SLOT(geometryChanged()) );
	connect( ui.sbWidth, SIGNAL(valueChanged(double)), this, SLOT(geometryChanged()) );
	connect( ui.sbHeight, SIGNAL(valueChanged(double)), this, SLOT(geometryChanged()) );

	connect( ui.leRangeFirst, SIGNAL(textChanged(QString)), this, SLOT(rangeFirstChanged(QString)) );
	connect( ui.leRangeLast, SIGNAL(textChanged(QString)), this, SLOT(rangeLastChanged(QString)) );
	connect( rangeButtonsGroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(rangeTypeChanged()) );

	connect(ui.chkAutoScaleX, &QCheckBox::stateChanged, this, &CartesianPlotDock::autoScaleXChanged);
	connect(ui.leXMin, &QLineEdit::textChanged, this, &CartesianPlotDock::xMinChanged);
	connect(ui.leXMax, &QLineEdit::textChanged, this, &CartesianPlotDock::xMaxChanged);
	connect(ui.dateTimeEditXMin, &QDateTimeEdit::dateTimeChanged, this, &CartesianPlotDock::xMinDateTimeChanged);
	connect(ui.dateTimeEditXMax, &QDateTimeEdit::dateTimeChanged, this, &CartesianPlotDock::xMaxDateTimeChanged);
	connect( ui.cbXScaling, SIGNAL(currentIndexChanged(int)), this, SLOT(xScaleChanged(int)) );
	connect(ui.cbXRangeFormat, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &CartesianPlotDock::xRangeFormatChanged);

	connect(ui.chkAutoScaleY, &QCheckBox::stateChanged, this, &CartesianPlotDock::autoScaleYChanged);
	connect(ui.leYMin, &QLineEdit::textChanged, this, &CartesianPlotDock::yMinChanged);
	connect(ui.leYMax, &QLineEdit::textChanged, this, &CartesianPlotDock::yMaxChanged);
	connect(ui.dateTimeEditYMin, &QDateTimeEdit::dateTimeChanged, this, &CartesianPlotDock::yMinDateTimeChanged);
	connect(ui.dateTimeEditYMax, &QDateTimeEdit::dateTimeChanged, this, &CartesianPlotDock::yMaxDateTimeChanged);
	connect( ui.cbYScaling, SIGNAL(currentIndexChanged(int)), this, SLOT(yScaleChanged(int)) );
	connect(ui.cbYRangeFormat, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &CartesianPlotDock::yRangeFormatChanged);

	//Range breaks
	connect( ui.chkXBreak, SIGNAL(toggled(bool)), this, SLOT(toggleXBreak(bool)) );
	connect( ui.bAddXBreak, SIGNAL(clicked()), this, SLOT(addXBreak()) );
	connect( ui.bRemoveXBreak, SIGNAL(clicked()), this, SLOT(removeXBreak()) );
	connect( ui.cbXBreak, SIGNAL(currentIndexChanged(int)), this, SLOT(currentXBreakChanged(int)) );
	connect( ui.leXBreakStart, SIGNAL(textChanged(QString)), this, SLOT(xBreakStartChanged()) );
	connect( ui.leXBreakEnd, SIGNAL(textChanged(QString)), this, SLOT(xBreakEndChanged()) );
	connect( ui.sbXBreakPosition, SIGNAL(valueChanged(int)), this, SLOT(xBreakPositionChanged(int)) );
	connect( ui.cbXBreakStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(xBreakStyleChanged(int)) );

	connect( ui.chkYBreak, SIGNAL(toggled(bool)), this, SLOT(toggleYBreak(bool)) );
	connect( ui.bAddYBreak, SIGNAL(clicked()), this, SLOT(addYBreak()) );
	connect( ui.bRemoveYBreak, SIGNAL(clicked()), this, SLOT(removeYBreak()) );
	connect( ui.cbYBreak, SIGNAL(currentIndexChanged(int)), this, SLOT(currentYBreakChanged(int)) );
	connect( ui.leYBreakStart, SIGNAL(textChanged(QString)), this, SLOT(yBreakStartChanged()) );
	connect( ui.leYBreakEnd, SIGNAL(textChanged(QString)), this, SLOT(yBreakEndChanged()) );
	connect( ui.sbYBreakPosition, SIGNAL(valueChanged(int)), this, SLOT(yBreakPositionChanged(int)) );
	connect( ui.cbYBreakStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(yBreakStyleChanged(int)) );

	//Background
	connect( ui.cbBackgroundType, SIGNAL(currentIndexChanged(int)), this, SLOT(backgroundTypeChanged(int)) );
	connect( ui.cbBackgroundColorStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(backgroundColorStyleChanged(int)) );
	connect( ui.cbBackgroundImageStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(backgroundImageStyleChanged(int)) );
	connect( ui.cbBackgroundBrushStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(backgroundBrushStyleChanged(int)) );
	connect( ui.bOpen, SIGNAL(clicked(bool)), this, SLOT(selectFile()) );
	connect( ui.leBackgroundFileName, SIGNAL(textChanged(QString)), this, SLOT(fileNameChanged()) );
	connect( ui.leBackgroundFileName, SIGNAL(textChanged(QString)), this, SLOT(fileNameChanged()) );
	connect( ui.kcbBackgroundFirstColor, SIGNAL(changed(QColor)), this, SLOT(backgroundFirstColorChanged(QColor)) );
	connect( ui.kcbBackgroundSecondColor, SIGNAL(changed(QColor)), this, SLOT(backgroundSecondColorChanged(QColor)) );
	connect( ui.sbBackgroundOpacity, SIGNAL(valueChanged(int)), this, SLOT(backgroundOpacityChanged(int)) );

	//Border
	connect( ui.cbBorderStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(borderStyleChanged(int)) );
	connect( ui.kcbBorderColor, SIGNAL(changed(QColor)), this, SLOT(borderColorChanged(QColor)) );
	connect( ui.sbBorderWidth, SIGNAL(valueChanged(double)), this, SLOT(borderWidthChanged(double)) );
	connect( ui.sbBorderCornerRadius, SIGNAL(valueChanged(double)), this, SLOT(borderCornerRadiusChanged(double)) );
	connect( ui.sbBorderOpacity, SIGNAL(valueChanged(int)), this, SLOT(borderOpacityChanged(int)) );

	//Padding
	connect( ui.sbPaddingHorizontal, SIGNAL(valueChanged(double)), this, SLOT(horizontalPaddingChanged(double)) );
	connect( ui.sbPaddingVertical, SIGNAL(valueChanged(double)), this, SLOT(verticalPaddingChanged(double)) );
	connect( ui.sbPaddingRight, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &CartesianPlotDock::rightPaddingChanged);
	connect( ui.sbPaddingBottom, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &CartesianPlotDock::bottomPaddingChanged);
	connect( ui.cbPaddingSymmetric, &QCheckBox::toggled, this, &CartesianPlotDock::symmetricPaddingChanged);

	// Cursor
	connect(ui.sbCursorLineWidth, SIGNAL(valueChanged(int)), this, SLOT(cursorLineWidthChanged(int)));
	//connect(ui.sbCursorLineWidth, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &CartesianPlotDock::cursorLineWidthChanged);
	connect(ui.kcbCursorLineColor, &KColorButton::changed, this, &CartesianPlotDock::cursorLineColorChanged);
	//connect(ui.cbCursorLineStyle, qOverload<int>(&QComboBox::currentIndexChanged), this, &CartesianPlotDock::cursorLineStyleChanged);
	connect(ui.cbCursorLineStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(cursorLineStyleChanged(int)));
	//theme and template handlers
	auto* frame = new QFrame(this);
	auto* layout = new QHBoxLayout(frame);
	layout->setContentsMargins(0, 11, 0, 11);

	m_themeHandler = new ThemeHandler(this);
	layout->addWidget(m_themeHandler);
	connect(m_themeHandler, SIGNAL(loadThemeRequested(QString)), this, SLOT(loadTheme(QString)));
	connect(m_themeHandler, SIGNAL(saveThemeRequested(KConfig&)), this, SLOT(saveTheme(KConfig&)));
	connect(m_themeHandler, SIGNAL(info(QString)), this, SIGNAL(info(QString)));
	//connect(this, SIGNAL(saveThemeEnable(bool)), m_themeHandler, SLOT(saveThemeEnable(bool)));

	auto* templateHandler = new TemplateHandler(this, TemplateHandler::CartesianPlot);
	layout->addWidget(templateHandler);
	connect(templateHandler, SIGNAL(loadConfigRequested(KConfig&)), this, SLOT(loadConfigFromTemplate(KConfig&)));
	connect(templateHandler, SIGNAL(saveConfigRequested(KConfig&)), this, SLOT(saveConfigAsTemplate(KConfig&)));
	connect(templateHandler, SIGNAL(info(QString)), this, SIGNAL(info(QString)));

	ui.verticalLayout->addWidget(frame);

	//TODO: activate the tab again once the functionality is implemented
	ui.tabWidget->removeTab(2);

	init();
}

void CartesianPlotDock::init() {
	this->retranslateUi();

	/*
	 //TODO: activate later once range breaking is implemented
	//create icons for the different styles for scale breaking
	QPainter pa;
	pa.setPen( QPen(Qt::SolidPattern, 0) );
	QPixmap pm(20, 20);
	ui.cbXBreakStyle->setIconSize( QSize(20,20) );
	ui.cbYBreakStyle->setIconSize( QSize(20,20) );

	//simple
	pm.fill(Qt::transparent);
	pa.begin( &pm );
	pa.setRenderHint(QPainter::Antialiasing);
	pa.setBrush(Qt::SolidPattern);
	pa.drawLine(3,10,8,10);
	pa.drawLine(12,10,17,10);
	pa.end();
	ui.cbXBreakStyle->setItemIcon(0, pm);
	ui.cbYBreakStyle->setItemIcon(0, pm);

	//vertical
	pm.fill(Qt::transparent);
	pa.begin( &pm );
	pa.setRenderHint(QPainter::Antialiasing);
	pa.setBrush(Qt::SolidPattern);
	pa.drawLine(3,10,8,10);
	pa.drawLine(12,10,17,10);
	pa.drawLine(8,14,8,6);
	pa.drawLine(12,14,12,6);
	pa.end();
	ui.cbXBreakStyle->setItemIcon(1, pm);
	ui.cbYBreakStyle->setItemIcon(1, pm);

	//sloped
	pm.fill(Qt::transparent);
	pa.begin( &pm );
	pa.setRenderHint(QPainter::Antialiasing);
	pa.setBrush(Qt::SolidPattern);
	pa.drawLine(3,10,8,10);
	pa.drawLine(12,10,17,10);
	pa.drawLine(6,14,10,6);
	pa.drawLine(10,14,14,6);
	pa.end();
	ui.cbXBreakStyle->setItemIcon(2, pm);
	ui.cbYBreakStyle->setItemIcon(2, pm);
	*/
}

void CartesianPlotDock::setPlots(QList<CartesianPlot*> list) {
	m_initializing = true;
	m_plotList = list;
	m_plot = list.first();
	m_aspect = list.first();

	QList<TextLabel*> labels;
	for (auto* plot : list)
		labels.append(plot->title());

	labelWidget->setLabels(labels);

	//if there is more then one plot in the list, disable the name and comment fields in the tab "general"
	if (list.size() == 1) {
		ui.lName->setEnabled(true);
		ui.leName->setEnabled(true);
		ui.lComment->setEnabled(true);
		ui.leComment->setEnabled(true);

		ui.leName->setText(m_plot->name());
		ui.leComment->setText(m_plot->comment());
	} else {
		ui.lName->setEnabled(false);
		ui.leName->setEnabled(false);
		ui.lComment->setEnabled(false);
		ui.leComment->setEnabled(false);

		ui.leName->setText(QString());
		ui.leComment->setText(QString());
	}

	bool symmetric = m_plot->symmetricPadding();
	ui.lPaddingHorizontalRight->setVisible(!symmetric);
	ui.sbPaddingRight->setVisible(!symmetric);
	ui.lPaddingVerticalDown->setVisible(!symmetric);
	ui.sbPaddingBottom->setVisible(!symmetric);
	if (symmetric) {
		ui.lPaddingHorizontal->setText(i18n("Horizontal"));
		ui.lPaddingVertical->setText(i18n("Vertical"));
	} else {
		ui.lPaddingHorizontal->setText(i18n("Left"));
		ui.lPaddingVertical->setText(i18n("Top"));
	}

	ui.leName->setStyleSheet("");
	ui.leName->setToolTip("");

	//show the properties of the first plot
	this->load();

	//update active widgets
	backgroundTypeChanged(ui.cbBackgroundType->currentIndex());

	m_themeHandler->setCurrentTheme(m_plot->theme());

	//Deactivate the geometry related widgets, if the worksheet layout is active.
	//Currently, a plot can only be a child of the worksheet itself, so we only need to ask the parent aspect (=worksheet).
	//TODO redesign this, if the hierarchy will be changend in future (a plot is a child of a new object group/container or so)
	auto* w = dynamic_cast<Worksheet*>(m_plot->parentAspect());
	if (w) {
		bool b = (w->layout() == Worksheet::Layout::NoLayout);
		ui.sbTop->setEnabled(b);
		ui.sbLeft->setEnabled(b);
		ui.sbWidth->setEnabled(b);
		ui.sbHeight->setEnabled(b);
		connect(w, SIGNAL(layoutChanged(Worksheet::Layout)), this, SLOT(layoutChanged(Worksheet::Layout)));
	}

	//SIGNALs/SLOTs
	connect( m_plot, SIGNAL(aspectDescriptionChanged(const AbstractAspect*)), this, SLOT(plotDescriptionChanged(const AbstractAspect*)) );
	connect( m_plot, SIGNAL(rectChanged(QRectF&)), this, SLOT(plotRectChanged(QRectF&)) );
	connect( m_plot, SIGNAL(rangeTypeChanged(CartesianPlot::RangeType)), this, SLOT(plotRangeTypeChanged(CartesianPlot::RangeType)) );
	connect( m_plot, SIGNAL(rangeFirstValuesChanged(int)), this, SLOT(plotRangeFirstValuesChanged(int)) );
	connect( m_plot, SIGNAL(rangeLastValuesChanged(int)), this, SLOT(plotRangeLastValuesChanged(int)) );
	connect( m_plot, SIGNAL(xAutoScaleChanged(bool)), this, SLOT(plotXAutoScaleChanged(bool)) );
	connect( m_plot, SIGNAL(xMinChanged(double)), this, SLOT(plotXMinChanged(double)) );
	connect( m_plot, SIGNAL(xMaxChanged(double)), this, SLOT(plotXMaxChanged(double)) );
	connect( m_plot, SIGNAL(xScaleChanged(int)), this, SLOT(plotXScaleChanged(int)) );
	connect(m_plot, &CartesianPlot::xRangeFormatChanged, this, &CartesianPlotDock::plotXRangeFormatChanged);
	connect( m_plot, SIGNAL(yAutoScaleChanged(bool)), this, SLOT(plotYAutoScaleChanged(bool)) );
	connect( m_plot, SIGNAL(yMinChanged(double)), this, SLOT(plotYMinChanged(double)) );
	connect( m_plot, SIGNAL(yMaxChanged(double)), this, SLOT(plotYMaxChanged(double)) );
	connect( m_plot, SIGNAL(yScaleChanged(int)), this, SLOT(plotYScaleChanged(int)) );
	connect(m_plot, &CartesianPlot::yRangeFormatChanged, this, &CartesianPlotDock::plotYRangeFormatChanged);
	connect( m_plot, SIGNAL(visibleChanged(bool)), this, SLOT(plotVisibleChanged(bool)) );

	//range breaks
	connect( m_plot, SIGNAL(xRangeBreakingEnabledChanged(bool)), this, SLOT(plotXRangeBreakingEnabledChanged(bool)) );
	connect( m_plot, SIGNAL(xRangeBreaksChanged(CartesianPlot::RangeBreaks)), this, SLOT(plotXRangeBreaksChanged(CartesianPlot::RangeBreaks)) );
	connect( m_plot, SIGNAL(yRangeBreakingEnabledChanged(bool)), this, SLOT(plotYRangeBreakingEnabledChanged(bool)) );
	connect( m_plot, SIGNAL(yRangeBreaksChanged(CartesianPlot::RangeBreaks)), this, SLOT(plotYRangeBreaksChanged(CartesianPlot::RangeBreaks)) );

	// Plot Area
	connect( m_plot->plotArea(), SIGNAL(backgroundTypeChanged(PlotArea::BackgroundType)), this, SLOT(plotBackgroundTypeChanged(PlotArea::BackgroundType)) );
	connect( m_plot->plotArea(), SIGNAL(backgroundColorStyleChanged(PlotArea::BackgroundColorStyle)), this, SLOT(plotBackgroundColorStyleChanged(PlotArea::BackgroundColorStyle)) );
	connect( m_plot->plotArea(), SIGNAL(backgroundImageStyleChanged(PlotArea::BackgroundImageStyle)), this, SLOT(plotBackgroundImageStyleChanged(PlotArea::BackgroundImageStyle)) );
	connect( m_plot->plotArea(), SIGNAL(backgroundBrushStyleChanged(Qt::BrushStyle)), this, SLOT(plotBackgroundBrushStyleChanged(Qt::BrushStyle)) );
	connect( m_plot->plotArea(), SIGNAL(backgroundFirstColorChanged(QColor&)), this, SLOT(plotBackgroundFirstColorChanged(QColor&)) );
	connect( m_plot->plotArea(), SIGNAL(backgroundSecondColorChanged(QColor&)), this, SLOT(plotBackgroundSecondColorChanged(QColor&)) );
	connect( m_plot->plotArea(), SIGNAL(backgroundFileNameChanged(QString&)), this, SLOT(plotBackgroundFileNameChanged(QString&)) );
	connect( m_plot->plotArea(), SIGNAL(backgroundOpacityChanged(float)), this, SLOT(plotBackgroundOpacityChanged(float)) );
	connect( m_plot->plotArea(), SIGNAL(borderPenChanged(QPen&)), this, SLOT(plotBorderPenChanged(QPen&)) );
	connect( m_plot->plotArea(), SIGNAL(borderOpacityChanged(float)), this, SLOT(plotBorderOpacityChanged(float)) );
	connect( m_plot, SIGNAL(horizontalPaddingChanged(float)), this, SLOT(plotHorizontalPaddingChanged(float)) );
	connect( m_plot, SIGNAL(verticalPaddingChanged(float)), this, SLOT(plotVerticalPaddingChanged(float)) );
	connect(m_plot, &CartesianPlot::rightPaddingChanged, this, &CartesianPlotDock::plotRightPaddingChanged);
	connect(m_plot, &CartesianPlot::bottomPaddingChanged, this, &CartesianPlotDock::plotBottomPaddingChanged);
	connect(m_plot, &CartesianPlot::symmetricPaddingChanged, this, &CartesianPlotDock::plotSymmetricPaddingChanged);

	m_initializing = false;
}

void CartesianPlotDock::activateTitleTab() {
	ui.tabWidget->setCurrentWidget(ui.tabTitle);
}

void CartesianPlotDock::updateUnits() {
	const KConfigGroup group = KSharedConfig::openConfig()->group(QLatin1String("Settings_General"));
	BaseDock::Units units = (BaseDock::Units)group.readEntry("Units", (int)MetricUnits);
	if (units == m_units)
		return;

	m_units = units;
	Lock lock(m_initializing);
	QString suffix;
	if (m_units == BaseDock::MetricUnits) {
		//convert from imperial to metric
		m_worksheetUnit = Worksheet::Unit::Centimeter;
		suffix = QLatin1String("cm");
		ui.sbLeft->setValue(ui.sbLeft->value()*2.54);
		ui.sbTop->setValue(ui.sbTop->value()*2.54);
		ui.sbWidth->setValue(ui.sbWidth->value()*2.54);
		ui.sbHeight->setValue(ui.sbHeight->value()*2.54);
		ui.sbBorderCornerRadius->setValue(ui.sbBorderCornerRadius->value()*2.54);
		ui.sbPaddingHorizontal->setValue(ui.sbPaddingHorizontal->value()*2.54);
		ui.sbPaddingVertical->setValue(ui.sbPaddingVertical->value()*2.54);
		ui.sbPaddingRight->setValue(ui.sbPaddingRight->value()*2.54);
		ui.sbPaddingBottom->setValue(ui.sbPaddingBottom->value()*2.54);
	} else {
		//convert from metric to imperial
		m_worksheetUnit = Worksheet::Unit::Inch;
		suffix = QLatin1String("in");
		ui.sbLeft->setValue(ui.sbLeft->value()/2.54);
		ui.sbTop->setValue(ui.sbTop->value()/2.54);
		ui.sbWidth->setValue(ui.sbWidth->value()/2.54);
		ui.sbHeight->setValue(ui.sbHeight->value()/2.54);
		ui.sbBorderCornerRadius->setValue(ui.sbBorderCornerRadius->value()/2.54);
		ui.sbPaddingHorizontal->setValue(ui.sbPaddingHorizontal->value()/2.54);
		ui.sbPaddingVertical->setValue(ui.sbPaddingVertical->value()/2.54);
		ui.sbPaddingRight->setValue(ui.sbPaddingRight->value()/2.54);
		ui.sbPaddingBottom->setValue(ui.sbPaddingBottom->value()/2.54);
	}

	ui.sbLeft->setSuffix(suffix);
	ui.sbTop->setSuffix(suffix);
	ui.sbWidth->setSuffix(suffix);
	ui.sbHeight->setSuffix(suffix);
	ui.sbBorderCornerRadius->setSuffix(suffix);
	ui.sbPaddingHorizontal->setSuffix(suffix);
	ui.sbPaddingVertical->setSuffix(suffix);
	ui.sbPaddingRight->setSuffix(suffix);
	ui.sbPaddingBottom->setSuffix(suffix);

	labelWidget->updateUnits();
}
//************************************************************
//**** SLOTs for changes triggered in CartesianPlotDock ******
//************************************************************
void CartesianPlotDock::retranslateUi() {
	Lock lock(m_initializing);

	//general
	ui.cbXRangeFormat->addItem(i18n("numeric"));
	ui.cbXRangeFormat->addItem(i18n("datetime"));
	ui.cbYRangeFormat->addItem(i18n("numeric"));
	ui.cbYRangeFormat->addItem(i18n("datetime"));

	ui.cbXScaling->addItem( i18n("linear") );
	ui.cbXScaling->addItem( i18n("log(x)") );
	ui.cbXScaling->addItem( i18n("log2(x)") );
	ui.cbXScaling->addItem( i18n("ln(x)") );
	ui.cbXScaling->addItem( i18n("log(abs(x))") );
	ui.cbXScaling->addItem( i18n("log2(abs(x))") );
	ui.cbXScaling->addItem( i18n("ln(abs(x))") );

	ui.cbYScaling->addItem( i18n("linear") );
	ui.cbYScaling->addItem( i18n("log(y)") );
	ui.cbYScaling->addItem( i18n("log2(y)") );
	ui.cbYScaling->addItem( i18n("ln(y)") );
	ui.cbYScaling->addItem( i18n("log(abs(y))") );
	ui.cbYScaling->addItem( i18n("log2(abs(y))") );
	ui.cbYScaling->addItem( i18n("ln(abs(y))") );

	//scale breakings
	ui.cbXBreakStyle->addItem( i18n("Simple") );
	ui.cbXBreakStyle->addItem( i18n("Vertical") );
	ui.cbXBreakStyle->addItem( i18n("Sloped") );

	ui.cbYBreakStyle->addItem( i18n("Simple") );
	ui.cbYBreakStyle->addItem( i18n("Vertical") );
	ui.cbYBreakStyle->addItem( i18n("Sloped") );

	//plot area
	ui.cbBackgroundType->addItem(i18n("Color"));
	ui.cbBackgroundType->addItem(i18n("Image"));
	ui.cbBackgroundType->addItem(i18n("Pattern"));

	ui.cbBackgroundColorStyle->addItem(i18n("Single Color"));
	ui.cbBackgroundColorStyle->addItem(i18n("Horizontal Gradient"));
	ui.cbBackgroundColorStyle->addItem(i18n("Vertical Gradient"));
	ui.cbBackgroundColorStyle->addItem(i18n("Diag. Gradient (From Top Left)"));
	ui.cbBackgroundColorStyle->addItem(i18n("Diag. Gradient (From Bottom Left)"));
	ui.cbBackgroundColorStyle->addItem(i18n("Radial Gradient"));

	ui.cbBackgroundImageStyle->addItem(i18n("Scaled and Cropped"));
	ui.cbBackgroundImageStyle->addItem(i18n("Scaled"));
	ui.cbBackgroundImageStyle->addItem(i18n("Scaled, Keep Proportions"));
	ui.cbBackgroundImageStyle->addItem(i18n("Centered"));
	ui.cbBackgroundImageStyle->addItem(i18n("Tiled"));
	ui.cbBackgroundImageStyle->addItem(i18n("Center Tiled"));

	GuiTools::updatePenStyles(ui.cbBorderStyle, Qt::black);
	GuiTools::updateBrushStyles(ui.cbBackgroundBrushStyle, Qt::SolidPattern);

	QString suffix;
	if (m_units == BaseDock::MetricUnits)
		suffix = QLatin1String("cm");
	else
		suffix = QLatin1String("in");

	ui.sbLeft->setSuffix(suffix);
	ui.sbTop->setSuffix(suffix);
	ui.sbWidth->setSuffix(suffix);
	ui.sbHeight->setSuffix(suffix);
	ui.sbBorderCornerRadius->setSuffix(suffix);
	ui.sbPaddingHorizontal->setSuffix(suffix);
	ui.sbPaddingVertical->setSuffix(suffix);
	ui.sbPaddingRight->setSuffix(suffix);
	ui.sbPaddingBottom->setSuffix(suffix);
}

// "General"-tab
void CartesianPlotDock::visibilityChanged(bool state) {
	if (m_initializing)
		return;

	for (auto* plot : m_plotList)
		plot->setVisible(state);
}

void CartesianPlotDock::geometryChanged() {
	if (m_initializing)
		return;

	float x = Worksheet::convertToSceneUnits(ui.sbLeft->value(), m_worksheetUnit);
	float y = Worksheet::convertToSceneUnits(ui.sbTop->value(), m_worksheetUnit);
	float w = Worksheet::convertToSceneUnits(ui.sbWidth->value(), m_worksheetUnit);
	float h = Worksheet::convertToSceneUnits(ui.sbHeight->value(), m_worksheetUnit);

	QRectF rect(x, y, w, h);
	m_plot->setRect(rect);
}

/*!
    Called when the layout in the worksheet gets changed.
    Enables/disables the geometry widgets if the layout was deactivated/activated.
    Shows the new geometry values of the first plot if the layout was activated.
 */
void CartesianPlotDock::layoutChanged(Worksheet::Layout layout) {
	bool b = (layout == Worksheet::Layout::NoLayout);
	ui.sbTop->setEnabled(b);
	ui.sbLeft->setEnabled(b);
	ui.sbWidth->setEnabled(b);
	ui.sbHeight->setEnabled(b);
}

void CartesianPlotDock::rangeTypeChanged() {
	CartesianPlot::RangeType type;
	if (ui.rbRangeFirst->isChecked()) {
		ui.leRangeFirst->setEnabled(true);
		ui.leRangeLast->setEnabled(false);
		type = CartesianPlot::RangeFirst;
	} else if (ui.rbRangeLast->isChecked()) {
		ui.leRangeFirst->setEnabled(false);
		ui.leRangeLast->setEnabled(true);
		type = CartesianPlot::RangeLast;
	} else {
		ui.leRangeFirst->setEnabled(false);
		ui.leRangeLast->setEnabled(false);
		type = CartesianPlot::RangeFree;
	}

	if (m_initializing)
		return;

	for (auto* plot : m_plotList)
		plot->setRangeType(type);
}

void CartesianPlotDock::rangeFirstChanged(const QString& text) {
	if (m_initializing)
		return;

	const int value = text.toInt();
	for (auto* plot : m_plotList)
		plot->setRangeFirstValues(value);
}

void CartesianPlotDock::rangeLastChanged(const QString& text) {
	if (m_initializing)
		return;

	const int value = text.toInt();
	for (auto* plot : m_plotList)
		plot->setRangeLastValues(value);
}

void CartesianPlotDock::autoScaleXChanged(int state) {
	bool checked = (state == Qt::Checked);
	ui.cbXRangeFormat->setEnabled(!checked);
	ui.leXMin->setEnabled(!checked);
	ui.leXMax->setEnabled(!checked);
	ui.dateTimeEditXMin->setEnabled(!checked);
	ui.dateTimeEditXMax->setEnabled(!checked);

	if (m_initializing)
		return;

	for (auto* plot : m_plotList)
		plot->setAutoScaleX(checked);
}

void CartesianPlotDock::xMinChanged(const QString& value) {
	if (m_initializing)
		return;

	const Lock lock(m_initializing);
	const float min = value.toDouble();
	for (auto* plot : m_plotList)
		plot->setXMin(min);
}

void CartesianPlotDock::xMaxChanged(const QString& value) {
	if (m_initializing)
		return;

	const Lock lock(m_initializing);
	const float max = value.toDouble();
	for (auto* plot : m_plotList)
		plot->setXMax(max);
}

void CartesianPlotDock::xMinDateTimeChanged(const QDateTime& dateTime) {
	if (m_initializing)
		return;

	quint64 value = dateTime.toMSecsSinceEpoch();
	for (auto* plot : m_plotList)
		plot->setXMin(value);
}

void CartesianPlotDock::xMaxDateTimeChanged(const QDateTime& dateTime) {
	if (m_initializing)
		return;

	quint64 value = dateTime.toMSecsSinceEpoch();
	for (auto* plot : m_plotList)
		plot->setXMax(value);
}

/*!
    called on scale changes (linear, log) for the x-axis
 */
void CartesianPlotDock::xScaleChanged(int scale) {
	if (m_initializing)
		return;

	for (auto* plot : m_plotList)
		plot->setXScale((CartesianPlot::Scale) scale);
}

void CartesianPlotDock::xRangeFormatChanged(int index) {
	bool numeric = (index == 0);

	ui.lXMin->setVisible(numeric);
	ui.leXMin->setVisible(numeric);
	ui.lXMax->setVisible(numeric);
	ui.leXMax->setVisible(numeric);

	ui.lXMinDateTime->setVisible(!numeric);
	ui.dateTimeEditXMin->setVisible(!numeric);
	ui.lXMaxDateTime->setVisible(!numeric);
	ui.dateTimeEditXMax->setVisible(!numeric);

	if (m_initializing)
		return;

	auto format = (CartesianPlot::RangeFormat)index;
	for (auto* plot : m_plotList)
		plot->setXRangeFormat(format);
}

void CartesianPlotDock::autoScaleYChanged(int state) {
	bool checked = (state == Qt::Checked);
	ui.cbYRangeFormat->setEnabled(!checked);
	ui.leYMin->setEnabled(!checked);
	ui.leYMax->setEnabled(!checked);
	ui.dateTimeEditYMin->setEnabled(!checked);
	ui.dateTimeEditYMax->setEnabled(!checked);

	if (m_initializing)
		return;

	for (auto* plot : m_plotList)
		plot->setAutoScaleY(checked);
}

void CartesianPlotDock::yMinChanged(const QString& value) {
	if (m_initializing)
		return;

	const Lock lock(m_initializing);
	const float min = value.toDouble();
	for (auto* plot : m_plotList)
		plot->setYMin(min);
}

void CartesianPlotDock::yMaxChanged(const QString& value) {
	if (m_initializing)
		return;

	const Lock lock(m_initializing);
	const float max = value.toDouble();
	for (auto* plot : m_plotList)
		plot->setYMax(max);
}

void CartesianPlotDock::yMinDateTimeChanged(const QDateTime& dateTime) {
	if (m_initializing)
		return;

	quint64 value = dateTime.toMSecsSinceEpoch();
	for (auto* plot : m_plotList)
		plot->setXMin(value);
}

void CartesianPlotDock::yMaxDateTimeChanged(const QDateTime& dateTime) {
	if (m_initializing)
		return;

	quint64 value = dateTime.toMSecsSinceEpoch();
	for (auto* plot : m_plotList)
		plot->setXMax(value);
}

/*!
    called on scale changes (linear, log) for the y-axis
 */
void CartesianPlotDock::yScaleChanged(int index) {
	if (m_initializing)
		return;

	auto scale = (CartesianPlot::Scale)index;
	for (auto* plot : m_plotList)
		plot->setYScale(scale);
}

void CartesianPlotDock::yRangeFormatChanged(int index) {
	bool numeric = (index == 0);

	ui.lYMin->setVisible(numeric);
	ui.leYMin->setVisible(numeric);
	ui.lYMax->setVisible(numeric);
	ui.leYMax->setVisible(numeric);

	ui.lYMinDateTime->setVisible(!numeric);
	ui.dateTimeEditYMin->setVisible(!numeric);
	ui.lYMaxDateTime->setVisible(!numeric);
	ui.dateTimeEditYMax->setVisible(!numeric);

	if (m_initializing)
		return;

	auto format = (CartesianPlot::RangeFormat)index;
	for (auto* plot : m_plotList)
		plot->setYRangeFormat(format);
}

// "Range Breaks"-tab

// x-range breaks
void CartesianPlotDock::toggleXBreak(bool b) {
	ui.frameXBreakEdit->setEnabled(b);
	ui.leXBreakStart->setEnabled(b);
	ui.leXBreakEnd->setEnabled(b);
	ui.sbXBreakPosition->setEnabled(b);
	ui.cbXBreakStyle->setEnabled(b);

	if (m_initializing)
		return;

	for (auto* plot : m_plotList)
		plot->setXRangeBreakingEnabled(b);
}

void CartesianPlotDock::addXBreak() {
	ui.bRemoveXBreak->setVisible(true);

	CartesianPlot::RangeBreaks breaks = m_plot->xRangeBreaks();
	CartesianPlot::RangeBreak b;
	breaks.list<<b;
	breaks.lastChanged = breaks.list.size() - 1;
	for (auto* plot : m_plotList)
		plot->setXRangeBreaks(breaks);

	ui.cbXBreak->addItem(QString::number(ui.cbXBreak->count()+1));
	ui.cbXBreak->setCurrentIndex(ui.cbXBreak->count()-1);
}

void CartesianPlotDock::removeXBreak() {
	ui.bRemoveXBreak->setVisible(m_plot->xRangeBreaks().list.size()>1);
	int index = ui.cbXBreak->currentIndex();
	CartesianPlot::RangeBreaks breaks = m_plot->xRangeBreaks();
	breaks.list.takeAt(index);
	breaks.lastChanged = -1;
	for (auto* plot : m_plotList)
		plot->setXRangeBreaks(breaks);

	ui.cbXBreak->clear();
	for (int i = 1; i <= breaks.list.size(); ++i)
		ui.cbXBreak->addItem(QString::number(i));

	if (index < ui.cbXBreak->count()-1)
		ui.cbXBreak->setCurrentIndex(index);
	else
		ui.cbXBreak->setCurrentIndex(ui.cbXBreak->count()-1);

	ui.bRemoveXBreak->setVisible(ui.cbXBreak->count()!=1);
}

void CartesianPlotDock::currentXBreakChanged(int index) {
	if (m_initializing)
		return;

	if (index == -1)
		return;

	m_initializing = true;
	const CartesianPlot::RangeBreak rangeBreak = m_plot->xRangeBreaks().list.at(index);
	QString str = std::isnan(rangeBreak.start) ? QString() : QString::number(rangeBreak.start);
	ui.leXBreakStart->setText(str);
	str = std::isnan(rangeBreak.end) ? QString() : QString::number(rangeBreak.end);
	ui.leXBreakEnd->setText(str);
	ui.sbXBreakPosition->setValue(rangeBreak.position*100);
	ui.cbXBreakStyle->setCurrentIndex((int)rangeBreak.style);
	m_initializing = false;
}

void CartesianPlotDock::xBreakStartChanged() {
	if (m_initializing)
		return;

	int index = ui.cbXBreak->currentIndex();
	CartesianPlot::RangeBreaks breaks = m_plot->xRangeBreaks();
	breaks.list[index].start = ui.leXBreakStart->text().toDouble();
	breaks.lastChanged = index;

	for (auto* plot : m_plotList)
		plot->setXRangeBreaks(breaks);
}

void CartesianPlotDock::xBreakEndChanged() {
	if (m_initializing)
		return;

	int index = ui.cbXBreak->currentIndex();
	CartesianPlot::RangeBreaks breaks = m_plot->xRangeBreaks();
	breaks.list[index].end = ui.leXBreakEnd->text().toDouble();
	breaks.lastChanged = index;

	for (auto* plot : m_plotList)
		plot->setXRangeBreaks(breaks);
}

void CartesianPlotDock::xBreakPositionChanged(int value) {
	if (m_initializing)
		return;

	int index = ui.cbXBreak->currentIndex();
	CartesianPlot::RangeBreaks breaks = m_plot->xRangeBreaks();
	breaks.list[index].position = (float)value/100.;
	breaks.lastChanged = index;

	for (auto* plot : m_plotList)
		plot->setXRangeBreaks(breaks);
}

void CartesianPlotDock::xBreakStyleChanged(int styleIndex) {
	if (m_initializing)
		return;

	int index = ui.cbXBreak->currentIndex();
	auto style = CartesianPlot::RangeBreakStyle(styleIndex);
	CartesianPlot::RangeBreaks breaks = m_plot->xRangeBreaks();
	breaks.list[index].style = style;
	breaks.lastChanged = index;

	for (auto* plot : m_plotList)
		plot->setXRangeBreaks(breaks);
}

// y-range breaks
void CartesianPlotDock::toggleYBreak(bool b) {
	ui.frameYBreakEdit->setEnabled(b);
	ui.leYBreakStart->setEnabled(b);
	ui.leYBreakEnd->setEnabled(b);
	ui.sbYBreakPosition->setEnabled(b);
	ui.cbYBreakStyle->setEnabled(b);

	if (m_initializing)
		return;

	for (auto* plot : m_plotList)
		plot->setYRangeBreakingEnabled(b);
}

void CartesianPlotDock::addYBreak() {
	ui.bRemoveYBreak->setVisible(true);

	CartesianPlot::RangeBreaks breaks = m_plot->yRangeBreaks();
	CartesianPlot::RangeBreak b;
	breaks.list << b;
	breaks.lastChanged = breaks.list.size() - 1;
	for (auto* plot : m_plotList)
		plot->setYRangeBreaks(breaks);

	ui.cbYBreak->addItem(QString::number(ui.cbYBreak->count()+1));
	ui.cbYBreak->setCurrentIndex(ui.cbYBreak->count()-1);
}

void CartesianPlotDock::removeYBreak() {
	ui.bRemoveYBreak->setVisible(m_plot->yRangeBreaks().list.size()>1);
	int index = ui.cbYBreak->currentIndex();
	CartesianPlot::RangeBreaks breaks = m_plot->yRangeBreaks();
	breaks.list.takeAt(index);
	breaks.lastChanged = -1;
	for (auto* plot : m_plotList)
		plot->setYRangeBreaks(breaks);

	ui.cbYBreak->clear();
	for (int i = 1; i <= breaks.list.size(); ++i)
		ui.cbYBreak->addItem(QString::number(i));

	if (index < ui.cbYBreak->count()-1)
		ui.cbYBreak->setCurrentIndex(index);
	else
		ui.cbYBreak->setCurrentIndex(ui.cbYBreak->count()-1);

	ui.bRemoveYBreak->setVisible(ui.cbYBreak->count() != 1);
}

void CartesianPlotDock::currentYBreakChanged(int index) {
	if (m_initializing)
		return;

	if (index == -1)
		return;

	m_initializing = true;
	const CartesianPlot::RangeBreak rangeBreak = m_plot->yRangeBreaks().list.at(index);
	QString str = std::isnan(rangeBreak.start) ? QString() : QString::number(rangeBreak.start);
	ui.leYBreakStart->setText(str);
	str = std::isnan(rangeBreak.end) ? QString() : QString::number(rangeBreak.end);
	ui.leYBreakEnd->setText(str);
	ui.sbYBreakPosition->setValue(rangeBreak.position*100);
	ui.cbYBreakStyle->setCurrentIndex((int)rangeBreak.style);
	m_initializing = false;
}

void CartesianPlotDock::yBreakStartChanged() {
	if (m_initializing)
		return;

	int index = ui.cbYBreak->currentIndex();
	CartesianPlot::RangeBreaks breaks = m_plot->yRangeBreaks();
	breaks.list[index].start = ui.leYBreakStart->text().toDouble();
	breaks.lastChanged = index;

	for (auto* plot : m_plotList)
		plot->setYRangeBreaks(breaks);
}

void CartesianPlotDock::yBreakEndChanged() {
	if (m_initializing)
		return;

	int index = ui.cbYBreak->currentIndex();
	CartesianPlot::RangeBreaks breaks = m_plot->yRangeBreaks();
	breaks.list[index].end = ui.leYBreakEnd->text().toDouble();
	breaks.lastChanged = index;

	for (auto* plot : m_plotList)
		plot->setYRangeBreaks(breaks);
}

void CartesianPlotDock::yBreakPositionChanged(int value) {
	if (m_initializing)
		return;

	int index = ui.cbYBreak->currentIndex();
	CartesianPlot::RangeBreaks breaks = m_plot->yRangeBreaks();
	breaks.list[index].position = (float)value/100.;
	breaks.lastChanged = index;

	for (auto* plot : m_plotList)
		plot->setYRangeBreaks(breaks);
}

void CartesianPlotDock::yBreakStyleChanged(int styleIndex) {
	if (m_initializing)
		return;

	int index = ui.cbYBreak->currentIndex();
	auto style = CartesianPlot::RangeBreakStyle(styleIndex);
	CartesianPlot::RangeBreaks breaks = m_plot->yRangeBreaks();
	breaks.list[index].style = style;
	breaks.lastChanged = index;

	for (auto* plot : m_plotList)
		plot->setYRangeBreaks(breaks);
}

// "Plot area"-tab
void CartesianPlotDock::backgroundTypeChanged(int index) {
	auto type = (PlotArea::BackgroundType)index;

	if (type == PlotArea::BackgroundType::Color) {
		ui.lBackgroundColorStyle->show();
		ui.cbBackgroundColorStyle->show();
		ui.lBackgroundImageStyle->hide();
		ui.cbBackgroundImageStyle->hide();
		ui.lBackgroundBrushStyle->hide();
		ui.cbBackgroundBrushStyle->hide();

		ui.lBackgroundFileName->hide();
		ui.leBackgroundFileName->hide();
		ui.bOpen->hide();

		ui.lBackgroundFirstColor->show();
		ui.kcbBackgroundFirstColor->show();

		auto style = (PlotArea::BackgroundColorStyle) ui.cbBackgroundColorStyle->currentIndex();
		if (style == PlotArea::BackgroundColorStyle::SingleColor) {
			ui.lBackgroundFirstColor->setText(i18n("Color:"));
			ui.lBackgroundSecondColor->hide();
			ui.kcbBackgroundSecondColor->hide();
		} else {
			ui.lBackgroundFirstColor->setText(i18n("First color:"));
			ui.lBackgroundSecondColor->show();
			ui.kcbBackgroundSecondColor->show();
		}
	} else if (type == PlotArea::BackgroundType::Image) {
		ui.lBackgroundColorStyle->hide();
		ui.cbBackgroundColorStyle->hide();
		ui.lBackgroundImageStyle->show();
		ui.cbBackgroundImageStyle->show();
		ui.lBackgroundBrushStyle->hide();
		ui.cbBackgroundBrushStyle->hide();
		ui.lBackgroundFileName->show();
		ui.leBackgroundFileName->show();
		ui.bOpen->show();

		ui.lBackgroundFirstColor->hide();
		ui.kcbBackgroundFirstColor->hide();
		ui.lBackgroundSecondColor->hide();
		ui.kcbBackgroundSecondColor->hide();
	} else if (type == PlotArea::BackgroundType::Pattern) {
		ui.lBackgroundFirstColor->setText(i18n("Color:"));
		ui.lBackgroundColorStyle->hide();
		ui.cbBackgroundColorStyle->hide();
		ui.lBackgroundImageStyle->hide();
		ui.cbBackgroundImageStyle->hide();
		ui.lBackgroundBrushStyle->show();
		ui.cbBackgroundBrushStyle->show();
		ui.lBackgroundFileName->hide();
		ui.leBackgroundFileName->hide();
		ui.bOpen->hide();

		ui.lBackgroundFirstColor->show();
		ui.kcbBackgroundFirstColor->show();
		ui.lBackgroundSecondColor->hide();
		ui.kcbBackgroundSecondColor->hide();
	}

	if (m_initializing)
		return;

	for (auto* plot : m_plotList)
		plot->plotArea()->setBackgroundType(type);
}

void CartesianPlotDock::backgroundColorStyleChanged(int index) {
	auto style = (PlotArea::BackgroundColorStyle)index;

	if (style == PlotArea::BackgroundColorStyle::SingleColor) {
		ui.lBackgroundFirstColor->setText(i18n("Color:"));
		ui.lBackgroundSecondColor->hide();
		ui.kcbBackgroundSecondColor->hide();
	} else {
		ui.lBackgroundFirstColor->setText(i18n("First color:"));
		ui.lBackgroundSecondColor->show();
		ui.kcbBackgroundSecondColor->show();
		ui.lBackgroundBrushStyle->hide();
		ui.cbBackgroundBrushStyle->hide();
	}

	if (m_initializing)
		return;

	for (auto* plot : m_plotList)
		plot->plotArea()->setBackgroundColorStyle(style);
}

void CartesianPlotDock::backgroundImageStyleChanged(int index) {
	if (m_initializing)
		return;

	auto style = (PlotArea::BackgroundImageStyle)index;
	for (auto* plot : m_plotList)
		plot->plotArea()->setBackgroundImageStyle(style);
}

void CartesianPlotDock::backgroundBrushStyleChanged(int index) {
	if (m_initializing)
		return;

	auto style = (Qt::BrushStyle)index;
	for (auto* plot : m_plotList)
		plot->plotArea()->setBackgroundBrushStyle(style);
}

void CartesianPlotDock::backgroundFirstColorChanged(const QColor& c) {
	if (m_initializing)
		return;

	for (auto* plot : m_plotList)
		plot->plotArea()->setBackgroundFirstColor(c);
}

void CartesianPlotDock::backgroundSecondColorChanged(const QColor& c) {
	if (m_initializing)
		return;

	for (auto* plot : m_plotList)
		plot->plotArea()->setBackgroundSecondColor(c);
}

/*!
    opens a file dialog and lets the user select the image file.
*/
void CartesianPlotDock::selectFile() {
	KConfigGroup conf(KSharedConfig::openConfig(), "CartesianPlotDock");
	QString dir = conf.readEntry("LastImageDir", "");

	QString formats;
	for (const auto& format : QImageReader::supportedImageFormats()) {
		QString f = "*." + QString(format.constData());
		if (f == QLatin1String("*.svg"))
			continue;
		formats.isEmpty() ? formats += f : formats += ' ' + f;
	}

	QString path = QFileDialog::getOpenFileName(this, i18n("Select the image file"), dir, i18n("Images (%1)", formats));
	if (path.isEmpty())
		return; //cancel was clicked in the file-dialog

	int pos = path.lastIndexOf(QDir::separator());
	if (pos != -1) {
		QString newDir = path.left(pos);
		if (newDir != dir)
			conf.writeEntry("LastImageDir", newDir);
	}

	ui.leBackgroundFileName->setText( path );

	for (auto* plot : m_plotList)
		plot->plotArea()->setBackgroundFileName(path);
}

void CartesianPlotDock::fileNameChanged() {
	if (m_initializing)
		return;

	QString fileName = ui.leBackgroundFileName->text();
	if (!fileName.isEmpty() && !QFile::exists(fileName))
		ui.leBackgroundFileName->setStyleSheet("QLineEdit{background:red;}");
	else
		ui.leBackgroundFileName->setStyleSheet(QString());

	for (auto* plot : m_plotList)
		plot->plotArea()->setBackgroundFileName(fileName);
}

void CartesianPlotDock::backgroundOpacityChanged(int value) {
	if (m_initializing)
		return;

	qreal opacity = (float)value/100.;
	for (auto* plot : m_plotList)
		plot->plotArea()->setBackgroundOpacity(opacity);
}

// "Border"-tab
void CartesianPlotDock::borderStyleChanged(int index) {
	if (m_initializing)
		return;

	auto penStyle = Qt::PenStyle(index);
	QPen pen;
	for (auto* plot : m_plotList) {
		pen = plot->plotArea()->borderPen();
		pen.setStyle(penStyle);
		plot->plotArea()->setBorderPen(pen);
	}
}

void CartesianPlotDock::borderColorChanged(const QColor& color) {
	if (m_initializing)
		return;

	QPen pen;
	for (auto* plot : m_plotList) {
		pen = plot->plotArea()->borderPen();
		pen.setColor(color);
		plot->plotArea()->setBorderPen(pen);
	}

	m_initializing = true;
	GuiTools::updatePenStyles(ui.cbBorderStyle, color);
	m_initializing = false;
}

void CartesianPlotDock::borderWidthChanged(double value) {
	if (m_initializing)
		return;

	QPen pen;
	for (auto* plot : m_plotList) {
		pen = plot->plotArea()->borderPen();
		pen.setWidthF( Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point) );
		plot->plotArea()->setBorderPen(pen);
	}
}

void CartesianPlotDock::borderCornerRadiusChanged(double value) {
	if (m_initializing)
		return;

	for (auto* plot : m_plotList)
		plot->plotArea()->setBorderCornerRadius(Worksheet::convertToSceneUnits(value, m_worksheetUnit));
}

void CartesianPlotDock::borderOpacityChanged(int value) {
	if (m_initializing)
		return;

	qreal opacity = (float)value/100.;
	for (auto* plot : m_plotList)
		plot->plotArea()->setBorderOpacity(opacity);
}

void CartesianPlotDock::symmetricPaddingChanged(bool checked) {
	if (m_initializing)
		return;

	ui.lPaddingHorizontalRight->setVisible(!checked);
	ui.sbPaddingRight->setVisible(!checked);
	ui.lPaddingVerticalDown->setVisible(!checked);
	ui.sbPaddingBottom->setVisible(!checked);

	if (checked) {
		ui.lPaddingHorizontal->setText(i18n("Horizontal:"));
		ui.lPaddingVertical->setText(i18n("Vertical:"));
	} else {
		ui.lPaddingHorizontal->setText(i18n("Left:"));
		ui.lPaddingVertical->setText(i18n("Top:"));
	}

	for (auto* plot : m_plotList)
		plot->setSymmetricPadding(checked);

	if (checked) {
		rightPaddingChanged(ui.sbPaddingHorizontal->value());
		bottomPaddingChanged(ui.sbPaddingVertical->value());
	}
}

void CartesianPlotDock::horizontalPaddingChanged(double value) {
	if (m_initializing)
		return;
	double padding = Worksheet::convertToSceneUnits(value, m_worksheetUnit);
	for (auto* plot : m_plotList)
		plot->setHorizontalPadding(padding);

	if (m_plot->symmetricPadding()) {
		for (auto* plot: m_plotList)
			plot->setRightPadding(padding);
	}
}

void CartesianPlotDock::rightPaddingChanged(double value) {
	if (m_initializing)
		return;
	double padding = Worksheet::convertToSceneUnits(value, m_worksheetUnit);
	for (auto* plot : m_plotList)
		plot->setRightPadding(padding);
}

void CartesianPlotDock::verticalPaddingChanged(double value) {
	if (m_initializing)
		return;

	// TODO: find better solution (set spinbox range). When plot->rect().width() does change?
	double padding = Worksheet::convertToSceneUnits(value, m_worksheetUnit);
	for (auto* plot : m_plotList)
		plot->setVerticalPadding(padding);

	if (m_plot->symmetricPadding()) {
		for (auto* plot: m_plotList)
			plot->setBottomPadding(padding);
	}
}

void CartesianPlotDock::bottomPaddingChanged(double value) {
	if (m_initializing)
		return;
	double padding = Worksheet::convertToSceneUnits(value, m_worksheetUnit);
	for (auto* plot : m_plotList)
		plot->setBottomPadding(padding);
}

void CartesianPlotDock::cursorLineWidthChanged(int width) {
	if (m_initializing)
		return;

	for (auto* plot : m_plotList) {
		QPen pen = plot->cursorPen();
		pen.setWidthF( Worksheet::convertToSceneUnits(width, Worksheet::Unit::Point) );
		plot->setCursorPen(pen);
	}
}

void CartesianPlotDock::cursorLineColorChanged(QColor color) {
	if (m_initializing)
		return;

	for (auto* plot : m_plotList) {
		QPen pen = plot->cursorPen();
		pen.setColor(color);
		plot->setCursorPen(pen);
	}
}

void CartesianPlotDock::cursorLineStyleChanged(int index) {
	if (m_initializing)
		return;

	if (index > 5)
		return;

	for (auto* plot : m_plotList) {
		QPen pen = plot->cursorPen();
		pen.setStyle(static_cast<Qt::PenStyle>(index));
		plot->setCursorPen(pen);
	}
}

//*************************************************************
//****** SLOTs for changes triggered in CartesianPlot *********
//*************************************************************
//general
void CartesianPlotDock::plotDescriptionChanged(const AbstractAspect* aspect) {
	if (m_plot != aspect)
		return;

	m_initializing = true;
	if (aspect->name() != ui.leName->text())
		ui.leName->setText(aspect->name());
	else if (aspect->comment() != ui.leComment->text())
		ui.leComment->setText(aspect->comment());
	m_initializing = false;
}

void CartesianPlotDock::plotRectChanged(QRectF& rect) {
	m_initializing = true;
	ui.sbLeft->setValue(Worksheet::convertFromSceneUnits(rect.x(), m_worksheetUnit));
	ui.sbTop->setValue(Worksheet::convertFromSceneUnits(rect.y(), m_worksheetUnit));
	ui.sbWidth->setValue(Worksheet::convertFromSceneUnits(rect.width(), m_worksheetUnit));
	ui.sbHeight->setValue(Worksheet::convertFromSceneUnits(rect.height(), m_worksheetUnit));
	m_initializing = false;
}

void CartesianPlotDock::plotRangeTypeChanged(CartesianPlot::RangeType type) {
	m_initializing = true;
	switch (type) {
	case CartesianPlot::RangeFree:
		ui.rbRangeFree->setChecked(true);
		break;
	case CartesianPlot::RangeFirst:
		ui.rbRangeFirst->setChecked(true);
		break;
	case CartesianPlot::RangeLast:
		ui.rbRangeLast->setChecked(true);
		break;
	}
	m_initializing = false;
}

void CartesianPlotDock::plotRangeFirstValuesChanged(int value) {
	m_initializing = true;
	ui.leRangeFirst->setText(QString::number(value));
	m_initializing = false;
}

void CartesianPlotDock::plotRangeLastValuesChanged(int value) {
	m_initializing = true;
	ui.leRangeLast->setText(QString::number(value));
	m_initializing = false;
}

void CartesianPlotDock::plotXAutoScaleChanged(bool value) {
	m_initializing = true;
	ui.chkAutoScaleX->setChecked(value);
	m_initializing = false;
}

void CartesianPlotDock::plotXMinChanged(double value) {
	if (m_initializing)return;
	const Lock lock(m_initializing);
	ui.leXMin->setText( QString::number(value) );
	ui.dateTimeEditXMin->setDateTime( QDateTime::fromMSecsSinceEpoch(value) );
}

void CartesianPlotDock::plotXMaxChanged(double value) {
	if (m_initializing)return;
	const Lock lock(m_initializing);
	ui.leXMax->setText( QString::number(value) );
	ui.dateTimeEditXMax->setDateTime( QDateTime::fromMSecsSinceEpoch(value) );
}

void CartesianPlotDock::plotXScaleChanged(int scale) {
	m_initializing = true;
	ui.cbXScaling->setCurrentIndex( scale );
	m_initializing = false;
}

void CartesianPlotDock::plotXRangeFormatChanged(CartesianPlot::RangeFormat format) {
	m_initializing = true;
	ui.cbXRangeFormat->setCurrentIndex(format);
	m_initializing = false;
}

void CartesianPlotDock::plotYAutoScaleChanged(bool value) {
	m_initializing = true;
	ui.chkAutoScaleY->setChecked(value);
	m_initializing = false;
}

void CartesianPlotDock::plotYMinChanged(double value) {
	if (m_initializing)return;
	const Lock lock(m_initializing);
	ui.leYMin->setText( QString::number(value) );
	ui.dateTimeEditYMin->setDateTime( QDateTime::fromMSecsSinceEpoch(value) );
}

void CartesianPlotDock::plotYMaxChanged(double value) {
	if (m_initializing)return;
	const Lock lock(m_initializing);
	ui.leYMax->setText( QString::number(value) );
	ui.dateTimeEditYMax->setDateTime( QDateTime::fromMSecsSinceEpoch(value) );
}

void CartesianPlotDock::plotYScaleChanged(int scale) {
	m_initializing = true;
	ui.cbYScaling->setCurrentIndex( scale );
	m_initializing = false;
}

void CartesianPlotDock::plotYRangeFormatChanged(CartesianPlot::RangeFormat format) {
	m_initializing = true;
	ui.cbYRangeFormat->setCurrentIndex(format);
	m_initializing = false;
}

void CartesianPlotDock::plotVisibleChanged(bool on) {
	m_initializing = true;
	ui.chkVisible->setChecked(on);
	m_initializing = false;
}

//range breaks
void CartesianPlotDock::plotXRangeBreakingEnabledChanged(bool on) {
	m_initializing = true;
	ui.chkXBreak->setChecked(on);
	m_initializing = false;
}

void CartesianPlotDock::plotXRangeBreaksChanged(const CartesianPlot::RangeBreaks& breaks) {
	Q_UNUSED(breaks);
}

void CartesianPlotDock::plotYRangeBreakingEnabledChanged(bool on) {
	m_initializing = true;
	ui.chkYBreak->setChecked(on);
	m_initializing = false;
}

void CartesianPlotDock::plotYRangeBreaksChanged(const CartesianPlot::RangeBreaks& breaks) {
	Q_UNUSED(breaks);
}

//background
void CartesianPlotDock::plotBackgroundTypeChanged(PlotArea::BackgroundType type) {
	m_initializing = true;
	ui.cbBackgroundType->setCurrentIndex(static_cast<int>(type));
	m_initializing = false;
}

void CartesianPlotDock::plotBackgroundColorStyleChanged(PlotArea::BackgroundColorStyle style) {
	m_initializing = true;
	ui.cbBackgroundColorStyle->setCurrentIndex(static_cast<int>(style));
	m_initializing = false;
}

void CartesianPlotDock::plotBackgroundImageStyleChanged(PlotArea::BackgroundImageStyle style) {
	m_initializing = true;
	ui.cbBackgroundImageStyle->setCurrentIndex(static_cast<int>(style));
	m_initializing = false;
}

void CartesianPlotDock::plotBackgroundBrushStyleChanged(Qt::BrushStyle style) {
	m_initializing = true;
	ui.cbBackgroundBrushStyle->setCurrentIndex(style);
	m_initializing = false;
}

void CartesianPlotDock::plotBackgroundFirstColorChanged(QColor& color) {
	m_initializing = true;
	ui.kcbBackgroundFirstColor->setColor(color);
	m_initializing = false;
}

void CartesianPlotDock::plotBackgroundSecondColorChanged(QColor& color) {
	m_initializing = true;
	ui.kcbBackgroundSecondColor->setColor(color);
	m_initializing = false;
}

void CartesianPlotDock::plotBackgroundFileNameChanged(QString& filename) {
	m_initializing = true;
	ui.leBackgroundFileName->setText(filename);
	m_initializing = false;
}

void CartesianPlotDock::plotBackgroundOpacityChanged(float opacity) {
	m_initializing = true;
	ui.sbBackgroundOpacity->setValue( round(opacity*100.0) );
	m_initializing = false;
}

void CartesianPlotDock::plotBorderPenChanged(QPen& pen) {
	m_initializing = true;
	if (ui.cbBorderStyle->currentIndex() != pen.style())
		ui.cbBorderStyle->setCurrentIndex(pen.style());
	if (ui.kcbBorderColor->color() != pen.color())
		ui.kcbBorderColor->setColor(pen.color());
	if (ui.sbBorderWidth->value() != pen.widthF())
		ui.sbBorderWidth->setValue(Worksheet::convertFromSceneUnits(pen.widthF(), Worksheet::Unit::Point));
	m_initializing = false;
}

void CartesianPlotDock::plotBorderCornerRadiusChanged(float value) {
	m_initializing = true;
	ui.sbBorderCornerRadius->setValue(Worksheet::convertFromSceneUnits(value, m_worksheetUnit));
	m_initializing = false;
}

void CartesianPlotDock::plotBorderOpacityChanged(float value) {
	m_initializing = true;
	float v = (float)value*100.;
	ui.sbBorderOpacity->setValue(v);
	m_initializing = false;
}

void CartesianPlotDock::plotHorizontalPaddingChanged(float value) {
	m_initializing = true;
	ui.sbPaddingHorizontal->setValue(Worksheet::convertFromSceneUnits(value, m_worksheetUnit));
	m_initializing = false;
}

void CartesianPlotDock::plotVerticalPaddingChanged(float value) {
	m_initializing = true;
	ui.sbPaddingVertical->setValue(Worksheet::convertFromSceneUnits(value, m_worksheetUnit));
	m_initializing = false;
}

void CartesianPlotDock::plotRightPaddingChanged(double value) {
	m_initializing = true;
	ui.sbPaddingRight->setValue(Worksheet::convertFromSceneUnits(value, m_worksheetUnit));
	m_initializing = false;
}

void CartesianPlotDock::plotBottomPaddingChanged(double value) {
	m_initializing = true;
	ui.sbPaddingBottom->setValue(Worksheet::convertFromSceneUnits(value, m_worksheetUnit));
	m_initializing = false;
}

void CartesianPlotDock::plotSymmetricPaddingChanged(bool symmetric) {
	m_initializing = true;
	ui.cbPaddingSymmetric->setChecked(symmetric);
	m_initializing = false;
}

void CartesianPlotDock::plotCursorPenChanged(const QPen& pen) {
	m_initializing = true;
	ui.sbCursorLineWidth->setValue(Worksheet::convertFromSceneUnits(pen.widthF(), Worksheet::Unit::Point));
	ui.kcbCursorLineColor->setColor(pen.color());
	ui.cbCursorLineStyle->setCurrentIndex(pen.style());
	m_initializing = false;
}

//*************************************************************
//******************** SETTINGS *******************************
//*************************************************************
void CartesianPlotDock::loadConfigFromTemplate(KConfig& config) {
	//extract the name of the template from the file name
	QString name;
	int index = config.name().lastIndexOf(QDir::separator());
	if (index != -1)
		name = config.name().right(config.name().size() - index - 1);
	else
		name = config.name();

	int size = m_plotList.size();
	if (size > 1)
		m_plot->beginMacro(i18n("%1 cartesian plots: template \"%2\" loaded", size, name));
	else
		m_plot->beginMacro(i18n("%1: template \"%2\" loaded", m_plot->name(), name));

	this->loadConfig(config);

	m_plot->endMacro();
}

void CartesianPlotDock::load() {
	//General-tab
	ui.chkVisible->setChecked(m_plot->isVisible());
	ui.sbLeft->setValue(Worksheet::convertFromSceneUnits(m_plot->rect().x(), m_worksheetUnit));
	ui.sbTop->setValue(Worksheet::convertFromSceneUnits(m_plot->rect().y(), m_worksheetUnit));
	ui.sbWidth->setValue(Worksheet::convertFromSceneUnits(m_plot->rect().width(), m_worksheetUnit));
	ui.sbHeight->setValue(Worksheet::convertFromSceneUnits(m_plot->rect().height(), m_worksheetUnit));

	switch (m_plot->rangeType()) {
	case CartesianPlot::RangeFree:
		ui.rbRangeFree->setChecked(true);
		break;
	case CartesianPlot::RangeFirst:
		ui.rbRangeFirst->setChecked(true);
		break;
	case CartesianPlot::RangeLast:
		ui.rbRangeLast->setChecked(true);
		break;
	}
	rangeTypeChanged();
	ui.leRangeFirst->setText( QString::number(m_plot->rangeFirstValues()) );
	ui.leRangeLast->setText( QString::number(m_plot->rangeLastValues()) );

	ui.chkAutoScaleX->setChecked(m_plot->autoScaleX());
	ui.leXMin->setText( QString::number(m_plot->xMin()) );
	ui.leXMax->setText( QString::number(m_plot->xMax()) );
	ui.dateTimeEditXMin->setDisplayFormat(m_plot->xRangeDateTimeFormat());
	ui.dateTimeEditXMax->setDisplayFormat(m_plot->xRangeDateTimeFormat());
	ui.dateTimeEditXMin->setDateTime(QDateTime::fromMSecsSinceEpoch(m_plot->xMin()));
	ui.dateTimeEditXMax->setDateTime(QDateTime::fromMSecsSinceEpoch(m_plot->xMax()));
	ui.cbXScaling->setCurrentIndex( (int) m_plot->xScale() );
	ui.cbXRangeFormat->setCurrentIndex( (int) m_plot->xRangeFormat() );

	ui.chkAutoScaleY->setChecked(m_plot->autoScaleY());
	ui.leYMin->setText( QString::number(m_plot->yMin()) );
	ui.leYMax->setText( QString::number(m_plot->yMax()) );
	ui.dateTimeEditYMin->setDisplayFormat(m_plot->yRangeDateTimeFormat());
	ui.dateTimeEditYMax->setDisplayFormat(m_plot->yRangeDateTimeFormat());
	ui.dateTimeEditYMin->setDateTime(QDateTime::fromMSecsSinceEpoch(m_plot->yMin()));
	ui.dateTimeEditYMax->setDateTime(QDateTime::fromMSecsSinceEpoch(m_plot->yMax()));
	ui.cbYScaling->setCurrentIndex( (int)m_plot->yScale() );
	ui.cbYRangeFormat->setCurrentIndex( (int) m_plot->yRangeFormat() );

	//Title
	labelWidget->load();

	//x-range breaks, show the first break
	ui.chkXBreak->setChecked(m_plot->xRangeBreakingEnabled());
	this->toggleXBreak(m_plot->xRangeBreakingEnabled());
	ui.bRemoveXBreak->setVisible(m_plot->xRangeBreaks().list.size()>1);
	ui.cbXBreak->clear();
	if (!m_plot->xRangeBreaks().list.isEmpty()) {
		for (int i = 1; i <= m_plot->xRangeBreaks().list.size(); ++i)
			ui.cbXBreak->addItem(QString::number(i));
	} else
		ui.cbXBreak->addItem("1");
	ui.cbXBreak->setCurrentIndex(0);

	//y-range breaks, show the first break
	ui.chkYBreak->setChecked(m_plot->yRangeBreakingEnabled());
	this->toggleYBreak(m_plot->yRangeBreakingEnabled());
	ui.bRemoveYBreak->setVisible(m_plot->yRangeBreaks().list.size()>1);
	ui.cbYBreak->clear();
	if (!m_plot->yRangeBreaks().list.isEmpty()) {
		for (int i = 1; i <= m_plot->yRangeBreaks().list.size(); ++i)
			ui.cbYBreak->addItem(QString::number(i));
	} else
		ui.cbYBreak->addItem("1");
	ui.cbYBreak->setCurrentIndex(0);

	//"Plot Area"-tab
	//Background
	ui.cbBackgroundType->setCurrentIndex( (int)m_plot->plotArea()->backgroundType() );
	ui.cbBackgroundColorStyle->setCurrentIndex( (int) m_plot->plotArea()->backgroundColorStyle() );
	ui.cbBackgroundImageStyle->setCurrentIndex( (int) m_plot->plotArea()->backgroundImageStyle() );
	ui.cbBackgroundBrushStyle->setCurrentIndex( (int) m_plot->plotArea()->backgroundBrushStyle() );
	ui.leBackgroundFileName->setText( m_plot->plotArea()->backgroundFileName() );
	ui.kcbBackgroundFirstColor->setColor( m_plot->plotArea()->backgroundFirstColor() );
	ui.kcbBackgroundSecondColor->setColor( m_plot->plotArea()->backgroundSecondColor() );
	ui.sbBackgroundOpacity->setValue( round(m_plot->plotArea()->backgroundOpacity()*100.0) );

	//highlight the text field for the background image red if an image is used and cannot be found
	if (!m_plot->plotArea()->backgroundFileName().isEmpty() && !QFile::exists(m_plot->plotArea()->backgroundFileName()))
		ui.leBackgroundFileName->setStyleSheet("QLineEdit{background:red;}");
	else
		ui.leBackgroundFileName->setStyleSheet(QString());

	//Padding
	ui.sbPaddingHorizontal->setValue( Worksheet::convertFromSceneUnits(m_plot->horizontalPadding(), m_worksheetUnit) );
	ui.sbPaddingVertical->setValue( Worksheet::convertFromSceneUnits(m_plot->verticalPadding(), m_worksheetUnit) );
	ui.sbPaddingRight->setValue(Worksheet::convertFromSceneUnits(m_plot->rightPadding(), m_worksheetUnit));
	ui.sbPaddingBottom->setValue(Worksheet::convertFromSceneUnits(m_plot->bottomPadding(), m_worksheetUnit));
	ui.cbPaddingSymmetric->setChecked(m_plot->symmetricPadding());

	//Border
	ui.kcbBorderColor->setColor( m_plot->plotArea()->borderPen().color() );
	ui.cbBorderStyle->setCurrentIndex( (int) m_plot->plotArea()->borderPen().style() );
	ui.sbBorderWidth->setValue( Worksheet::convertFromSceneUnits(m_plot->plotArea()->borderPen().widthF(), Worksheet::Unit::Point) );
	ui.sbBorderCornerRadius->setValue( Worksheet::convertFromSceneUnits(m_plot->plotArea()->borderCornerRadius(), m_worksheetUnit) );
	ui.sbBorderOpacity->setValue( round(m_plot->plotArea()->borderOpacity()*100) );
	GuiTools::updatePenStyles(ui.cbBorderStyle, ui.kcbBorderColor->color());

	// Cursor
	QPen pen = m_plot->cursorPen();
	ui.cbCursorLineStyle->setCurrentIndex(pen.style());
	ui.kcbCursorLineColor->setColor(pen.color());
	ui.sbCursorLineWidth->setValue(pen.width());
}

void CartesianPlotDock::loadConfig(KConfig& config) {
	KConfigGroup group = config.group("CartesianPlot");

	//General
	//we don't load/save the settings in the general-tab, since they are not style related.
	//It doesn't make sense to load/save them in the template.
	//This data is read in CartesianPlotDock::setPlots().

	//Title
	KConfigGroup plotTitleGroup = config.group("CartesianPlotTitle");
	labelWidget->loadConfig(plotTitleGroup);

	//Scale breakings
	//TODO

	//Background-tab
	ui.cbBackgroundType->setCurrentIndex( group.readEntry("BackgroundType", (int) m_plot->plotArea()->backgroundType()) );
	ui.cbBackgroundColorStyle->setCurrentIndex( group.readEntry("BackgroundColorStyle", (int) m_plot->plotArea()->backgroundColorStyle()) );
	ui.cbBackgroundImageStyle->setCurrentIndex( group.readEntry("BackgroundImageStyle", (int) m_plot->plotArea()->backgroundImageStyle()) );
	ui.cbBackgroundBrushStyle->setCurrentIndex( group.readEntry("BackgroundBrushStyle", (int) m_plot->plotArea()->backgroundBrushStyle()) );
	ui.leBackgroundFileName->setText( group.readEntry("BackgroundFileName", m_plot->plotArea()->backgroundFileName()) );
	ui.kcbBackgroundFirstColor->setColor( group.readEntry("BackgroundFirstColor", m_plot->plotArea()->backgroundFirstColor()) );
	ui.kcbBackgroundSecondColor->setColor( group.readEntry("BackgroundSecondColor", m_plot->plotArea()->backgroundSecondColor()) );
	ui.sbBackgroundOpacity->setValue( round(group.readEntry("BackgroundOpacity", m_plot->plotArea()->backgroundOpacity())*100.0) );
	ui.sbPaddingHorizontal->setValue(Worksheet::convertFromSceneUnits(group.readEntry("HorizontalPadding", m_plot->horizontalPadding()), m_worksheetUnit));
	ui.sbPaddingVertical->setValue(Worksheet::convertFromSceneUnits(group.readEntry("VerticalPadding", m_plot->verticalPadding()), m_worksheetUnit));
	ui.sbPaddingRight->setValue(Worksheet::convertFromSceneUnits(group.readEntry("RightPadding", m_plot->rightPadding()), m_worksheetUnit));
	ui.sbPaddingBottom->setValue(Worksheet::convertFromSceneUnits(group.readEntry("BottomPadding", m_plot->bottomPadding()), m_worksheetUnit));
	ui.cbPaddingSymmetric->setChecked(group.readEntry("SymmetricPadding", m_plot->symmetricPadding()));

	//Border-tab
	ui.kcbBorderColor->setColor( group.readEntry("BorderColor", m_plot->plotArea()->borderPen().color()) );
	ui.cbBorderStyle->setCurrentIndex( group.readEntry("BorderStyle", (int) m_plot->plotArea()->borderPen().style()) );
	ui.sbBorderWidth->setValue( Worksheet::convertFromSceneUnits(group.readEntry("BorderWidth", m_plot->plotArea()->borderPen().widthF()), Worksheet::Unit::Point) );
	ui.sbBorderCornerRadius->setValue( Worksheet::convertFromSceneUnits(group.readEntry("BorderCornerRadius", m_plot->plotArea()->borderCornerRadius()), m_worksheetUnit) );
	ui.sbBorderOpacity->setValue( group.readEntry("BorderOpacity", m_plot->plotArea()->borderOpacity())*100 );

	m_initializing = true;
	GuiTools::updatePenStyles(ui.cbBorderStyle, ui.kcbBorderColor->color());
	m_initializing = false;
}

void CartesianPlotDock::saveConfigAsTemplate(KConfig& config) {
	KConfigGroup group = config.group("CartesianPlot");

	//General
	//we don't load/save the settings in the general-tab, since they are not style related.
	//It doesn't make sense to load/save them in the template.

	//Title
	KConfigGroup plotTitleGroup = config.group("CartesianPlotTitle");
	labelWidget->saveConfig(plotTitleGroup);

	//Scale breakings
	//TODO

	//Background
	group.writeEntry("BackgroundType", ui.cbBackgroundType->currentIndex());
	group.writeEntry("BackgroundColorStyle", ui.cbBackgroundColorStyle->currentIndex());
	group.writeEntry("BackgroundImageStyle", ui.cbBackgroundImageStyle->currentIndex());
	group.writeEntry("BackgroundBrushStyle", ui.cbBackgroundBrushStyle->currentIndex());
	group.writeEntry("BackgroundFileName", ui.leBackgroundFileName->text());
	group.writeEntry("BackgroundFirstColor", ui.kcbBackgroundFirstColor->color());
	group.writeEntry("BackgroundSecondColor", ui.kcbBackgroundSecondColor->color());
	group.writeEntry("BackgroundOpacity", ui.sbBackgroundOpacity->value()/100.0);
	group.writeEntry("HorizontalPadding", Worksheet::convertToSceneUnits(ui.sbPaddingHorizontal->value(), m_worksheetUnit));
	group.writeEntry("VerticalPadding", Worksheet::convertToSceneUnits(ui.sbPaddingVertical->value(), m_worksheetUnit));
	group.writeEntry("RightPadding", Worksheet::convertToSceneUnits(ui.sbPaddingRight->value(), m_worksheetUnit));
	group.writeEntry("BottomPadding", Worksheet::convertToSceneUnits(ui.sbPaddingBottom->value(), m_worksheetUnit));
	group.writeEntry("SymmetricPadding", ui.cbPaddingSymmetric->isChecked());

	//Border
	group.writeEntry("BorderStyle", ui.cbBorderStyle->currentIndex());
	group.writeEntry("BorderColor", ui.kcbBorderColor->color());
	group.writeEntry("BorderWidth", Worksheet::convertToSceneUnits(ui.sbBorderWidth->value(), Worksheet::Unit::Point));
	group.writeEntry("BorderCornerRadius", Worksheet::convertToSceneUnits(ui.sbBorderCornerRadius->value(), m_worksheetUnit));
	group.writeEntry("BorderOpacity", ui.sbBorderOpacity->value()/100.0);

	config.sync();
}

void CartesianPlotDock::loadTheme(const QString& theme) {
	for (auto* plot : m_plotList)
		plot->setTheme(theme);
}

void CartesianPlotDock::saveTheme(KConfig& config) const {
	if (!m_plotList.isEmpty())
		m_plotList.at(0)->saveTheme(config);
}
