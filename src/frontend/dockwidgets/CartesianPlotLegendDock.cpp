/*
	File                 : CartesianPlotLegendDock.cpp
	Project              : LabPlot
	Description          : widget for cartesian plot legend properties
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2013-2025 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2024 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "CartesianPlotLegendDock.h"
#include "backend/core/Settings.h"
#include "frontend/TemplateHandler.h"
#include "frontend/widgets/BackgroundWidget.h"
#include "frontend/widgets/LabelWidget.h"
#include "frontend/widgets/LineWidget.h"

#include <KLocalizedString>

#include <gsl/gsl_const_cgs.h>

/*!
  \class CartesianPlotLegendDock
  \brief  Provides a widget for editing the properties of the cartesian plot legend currently selected in the project explorer.

  \ingroup frontend
*/
CartesianPlotLegendDock::CartesianPlotLegendDock(QWidget* parent)
	: BaseDock(parent) {
	ui.setupUi(this);
	setBaseWidgets(ui.leName, ui.teComment);
	setVisibilityWidgets(ui.chkVisible);

	//"Title"-tab
	auto hboxLayout = new QHBoxLayout(ui.tabTitle);
	labelWidget = new LabelWidget(ui.tabTitle);
	labelWidget->setGeometryAvailable(false);
	labelWidget->setBorderAvailable(false);
	hboxLayout->addWidget(labelWidget);
	hboxLayout->setContentsMargins(2, 2, 2, 2);
	hboxLayout->setSpacing(2);

	//"Background"-tab
	auto* gridLayout = static_cast<QGridLayout*>(ui.tabBackground->layout());
	backgroundWidget = new BackgroundWidget(ui.tabBackground);
	gridLayout->addWidget(backgroundWidget, 1, 0, 1, 3);

	borderLineWidget = new LineWidget(ui.tabBackground);
	gridLayout->addWidget(borderLineWidget, 4, 0, 1, 3);

	// "Layout"-tab
	QString suffix;
	if (m_units == Units::Metric)
		suffix = QLatin1String(" cm");
	else
		suffix = QLatin1String(" in");

	ui.sbLineSymbolWidth->setSuffix(suffix);
	ui.sbPositionX->setSuffix(suffix);
	ui.sbPositionY->setSuffix(suffix);
	ui.sbBorderCornerRadius->setSuffix(suffix);
	ui.sbLayoutTopMargin->setSuffix(suffix);
	ui.sbLayoutBottomMargin->setSuffix(suffix);
	ui.sbLayoutLeftMargin->setSuffix(suffix);
	ui.sbLayoutRightMargin->setSuffix(suffix);
	ui.sbLayoutHorizontalSpacing->setSuffix(suffix);
	ui.sbLayoutVerticalSpacing->setSuffix(suffix);

	// adjust layouts in the tabs
	for (int i = 0; i < ui.tabWidget->count(); ++i) {
		auto layout = dynamic_cast<QGridLayout*>(ui.tabWidget->widget(i)->layout());
		if (!layout)
			continue;

		layout->setContentsMargins(2, 2, 2, 2);
		layout->setHorizontalSpacing(2);
		layout->setVerticalSpacing(2);
	}

	updateLocale();
	retranslateUi();

	// SIGNAL/SLOT

	// General
	connect(ui.chbLock, &QCheckBox::clicked, this, &CartesianPlotLegendDock::lockChanged);
	connect(ui.kfrLabelFont, &KFontRequester::fontSelected, this, &CartesianPlotLegendDock::labelFontChanged);
	connect(ui.chkUsePlotColor, &QCheckBox::clicked, this, &CartesianPlotLegendDock::usePlotColorChanged);
	connect(ui.kcbLabelColor, &KColorButton::changed, this, &CartesianPlotLegendDock::labelColorChanged);
	connect(ui.cbOrder, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CartesianPlotLegendDock::labelOrderChanged);
	connect(ui.sbLineSymbolWidth, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &CartesianPlotLegendDock::lineSymbolWidthChanged);

	connect(ui.chbBindLogicalPos, &QCheckBox::clicked, this, &CartesianPlotLegendDock::bindingChanged);
	connect(ui.cbPositionX, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CartesianPlotLegendDock::positionXChanged);
	connect(ui.cbPositionY, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CartesianPlotLegendDock::positionYChanged);
	connect(ui.sbPositionX, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &CartesianPlotLegendDock::customPositionXChanged);
	connect(ui.sbPositionY, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &CartesianPlotLegendDock::customPositionYChanged);

	connect(ui.cbHorizontalAlignment, QOverload<int>::of(&KComboBox::currentIndexChanged), this, &CartesianPlotLegendDock::horizontalAlignmentChanged);
	connect(ui.cbVerticalAlignment, QOverload<int>::of(&KComboBox::currentIndexChanged), this, &CartesianPlotLegendDock::verticalAlignmentChanged);
	connect(ui.sbRotation, QOverload<int>::of(&QSpinBox::valueChanged), this, &CartesianPlotLegendDock::rotationChanged);

	// Border
	connect(ui.sbBorderCornerRadius, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &CartesianPlotLegendDock::borderCornerRadiusChanged);

	// Layout
	connect(ui.sbLayoutTopMargin, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &CartesianPlotLegendDock::layoutTopMarginChanged);
	connect(ui.sbLayoutBottomMargin, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &CartesianPlotLegendDock::layoutBottomMarginChanged);
	connect(ui.sbLayoutLeftMargin, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &CartesianPlotLegendDock::layoutLeftMarginChanged);
	connect(ui.sbLayoutRightMargin, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &CartesianPlotLegendDock::layoutRightMarginChanged);
	connect(ui.sbLayoutHorizontalSpacing, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &CartesianPlotLegendDock::layoutHorizontalSpacingChanged);
	connect(ui.sbLayoutVerticalSpacing, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &CartesianPlotLegendDock::layoutVerticalSpacingChanged);
	connect(ui.sbLayoutColumnCount, QOverload<int>::of(&QSpinBox::valueChanged), this, &CartesianPlotLegendDock::layoutColumnCountChanged);

	// template handler
	auto* frame = new QFrame(this);
	auto* layout = new QHBoxLayout(frame);
	layout->setContentsMargins(0, 11, 0, 11);

	auto* templateHandler = new TemplateHandler(this, QLatin1String("CartesianPlotLegend"));
	layout->addWidget(templateHandler);
	connect(templateHandler, &TemplateHandler::loadConfigRequested, this, &CartesianPlotLegendDock::loadConfigFromTemplate);
	connect(templateHandler, &TemplateHandler::saveConfigRequested, this, &CartesianPlotLegendDock::saveConfigAsTemplate);
	connect(templateHandler, &TemplateHandler::info, this, &CartesianPlotLegendDock::info);

	ui.verticalLayout->addWidget(frame);
}

void CartesianPlotLegendDock::setLegends(QList<CartesianPlotLegend*> list) {
	CONDITIONAL_LOCK_RETURN;
	m_legendList = list;
	m_legend = list.first();
	setAspects(list);

	// show the properties of the first curve
	this->load();

	// on the very first start the column count shown in UI is 1.
	// if the this count for m_legend is also 1 then the slot layoutColumnCountChanged is not called
	// and we need to disable the "order" widgets here.
	ui.lOrder->setVisible(m_legend->layoutColumnCount() != 1);
	ui.cbOrder->setVisible(m_legend->layoutColumnCount() != 1);

	// SIGNALs/SLOTs
	// General
	connect(m_legend, &CartesianPlotLegend::labelFontChanged, this, &CartesianPlotLegendDock::legendLabelFontChanged);
	connect(m_legend, &CartesianPlotLegend::usePlotColorChanged, this, &CartesianPlotLegendDock::legendUsePlotColorChanged);
	connect(m_legend, &CartesianPlotLegend::labelColorChanged, this, &CartesianPlotLegendDock::legendLabelColorChanged);
	connect(m_legend, &CartesianPlotLegend::labelColumnMajorChanged, this, &CartesianPlotLegendDock::legendLabelOrderChanged);
	connect(m_legend, &CartesianPlotLegend::positionChanged, this, &CartesianPlotLegendDock::legendPositionChanged);
	connect(m_legend, &CartesianPlotLegend::positionLogicalChanged, this, &CartesianPlotLegendDock::legendPositionLogicalChanged);
	connect(m_legend, &CartesianPlotLegend::horizontalAlignmentChanged, this, &CartesianPlotLegendDock::legendHorizontalAlignmentChanged);
	connect(m_legend, &CartesianPlotLegend::verticalAlignmentChanged, this, &CartesianPlotLegendDock::legendVerticalAlignmentChanged);
	connect(m_legend, &CartesianPlotLegend::rotationAngleChanged, this, &CartesianPlotLegendDock::legendRotationAngleChanged);
	connect(m_legend, &CartesianPlotLegend::lineSymbolWidthChanged, this, &CartesianPlotLegendDock::legendLineSymbolWidthChanged);
	connect(m_legend, &CartesianPlotLegend::lockChanged, this, &CartesianPlotLegendDock::legendLockChanged);

	// layout
	connect(m_legend, &CartesianPlotLegend::layoutTopMarginChanged, this, &CartesianPlotLegendDock::legendLayoutTopMarginChanged);
	connect(m_legend, &CartesianPlotLegend::layoutBottomMarginChanged, this, &CartesianPlotLegendDock::legendLayoutBottomMarginChanged);
	connect(m_legend, &CartesianPlotLegend::layoutLeftMarginChanged, this, &CartesianPlotLegendDock::legendLayoutLeftMarginChanged);
	connect(m_legend, &CartesianPlotLegend::layoutRightMarginChanged, this, &CartesianPlotLegendDock::legendLayoutRightMarginChanged);
	connect(m_legend, &CartesianPlotLegend::layoutVerticalSpacingChanged, this, &CartesianPlotLegendDock::legendLayoutVerticalSpacingChanged);
	connect(m_legend, &CartesianPlotLegend::layoutHorizontalSpacingChanged, this, &CartesianPlotLegendDock::legendLayoutHorizontalSpacingChanged);
	connect(m_legend, &CartesianPlotLegend::layoutColumnCountChanged, this, &CartesianPlotLegendDock::legendLayoutColumnCountChanged);
}

void CartesianPlotLegendDock::activateTitleTab() const {
	ui.tabWidget->setCurrentWidget(ui.tabTitle);
}

/*
 * updates the locale in the widgets. called when the application settins are changed.
 */
void CartesianPlotLegendDock::updateLocale() {
	const auto numberLocale = QLocale();
	ui.sbLineSymbolWidth->setLocale(numberLocale);
	ui.sbPositionX->setLocale(numberLocale);
	ui.sbPositionY->setLocale(numberLocale);
	ui.sbPositionXLogical->setLocale(numberLocale);
	ui.sbPositionYLogical->setLocale(numberLocale);
	ui.sbBorderCornerRadius->setLocale(numberLocale);
	ui.sbLayoutTopMargin->setLocale(numberLocale);
	ui.sbLayoutBottomMargin->setLocale(numberLocale);
	ui.sbLayoutLeftMargin->setLocale(numberLocale);
	ui.sbLayoutRightMargin->setLocale(numberLocale);
	borderLineWidget->updateLocale();
}

void CartesianPlotLegendDock::updateUnits() {
	const KConfigGroup group = Settings::group(QStringLiteral("Settings_General"));
	BaseDock::Units units = (BaseDock::Units)group.readEntry("Units", static_cast<int>(Units::Metric));
	if (units == m_units)
		return;

	m_units = units;
	CONDITIONAL_LOCK_RETURN;
	QString suffix;
	auto xPosition = ui.cbPositionX->currentIndex();
	auto yPosition = ui.cbPositionY->currentIndex();
	if (m_units == Units::Metric) {
		// convert from imperial to metric
		m_worksheetUnit = Worksheet::Unit::Centimeter;
		suffix = QLatin1String(" cm");
		ui.sbLineSymbolWidth->setValue(roundValue(ui.sbLineSymbolWidth->value() * GSL_CONST_CGS_INCH));
		if (xPosition != static_cast<int>(WorksheetElement::HorizontalPosition::Relative))
			ui.sbPositionX->setValue(roundValue(ui.sbPositionX->value() * GSL_CONST_CGS_INCH));
		if (yPosition != static_cast<int>(WorksheetElement::VerticalPosition::Relative))
			ui.sbPositionY->setValue(roundValue(ui.sbPositionY->value() * GSL_CONST_CGS_INCH));
		ui.sbBorderCornerRadius->setValue(roundValue(ui.sbBorderCornerRadius->value() * GSL_CONST_CGS_INCH));
		ui.sbLayoutTopMargin->setValue(roundValue(ui.sbLayoutTopMargin->value() * GSL_CONST_CGS_INCH));
		ui.sbLayoutBottomMargin->setValue(roundValue(ui.sbLayoutBottomMargin->value() * GSL_CONST_CGS_INCH));
		ui.sbLayoutLeftMargin->setValue(roundValue(ui.sbLayoutLeftMargin->value() * GSL_CONST_CGS_INCH));
		ui.sbLayoutRightMargin->setValue(roundValue(ui.sbLayoutRightMargin->value() * GSL_CONST_CGS_INCH));
		ui.sbLayoutHorizontalSpacing->setValue(roundValue(ui.sbLayoutHorizontalSpacing->value() * GSL_CONST_CGS_INCH));
		ui.sbLayoutVerticalSpacing->setValue(roundValue(ui.sbLayoutVerticalSpacing->value() * GSL_CONST_CGS_INCH));
	} else {
		// convert from metric to imperial
		m_worksheetUnit = Worksheet::Unit::Inch;
		suffix = QLatin1String(" in");
		ui.sbLineSymbolWidth->setValue(roundValue(ui.sbLineSymbolWidth->value() / GSL_CONST_CGS_INCH));
		if (xPosition != static_cast<int>(WorksheetElement::HorizontalPosition::Relative))
			ui.sbPositionX->setValue(roundValue(ui.sbPositionX->value() / GSL_CONST_CGS_INCH));
		if (yPosition != static_cast<int>(WorksheetElement::VerticalPosition::Relative))
			ui.sbPositionY->setValue(roundValue(ui.sbPositionY->value() / GSL_CONST_CGS_INCH));
		ui.sbBorderCornerRadius->setValue(roundValue(ui.sbBorderCornerRadius->value() / GSL_CONST_CGS_INCH));
		ui.sbLayoutTopMargin->setValue(roundValue(ui.sbLayoutTopMargin->value() / GSL_CONST_CGS_INCH));
		ui.sbLayoutBottomMargin->setValue(roundValue(ui.sbLayoutBottomMargin->value() / GSL_CONST_CGS_INCH));
		ui.sbLayoutLeftMargin->setValue(roundValue(ui.sbLayoutLeftMargin->value() / GSL_CONST_CGS_INCH));
		ui.sbLayoutRightMargin->setValue(roundValue(ui.sbLayoutRightMargin->value() / GSL_CONST_CGS_INCH));
		ui.sbLayoutHorizontalSpacing->setValue(roundValue(ui.sbLayoutHorizontalSpacing->value() / GSL_CONST_CGS_INCH));
		ui.sbLayoutVerticalSpacing->setValue(roundValue(ui.sbLayoutVerticalSpacing->value() / GSL_CONST_CGS_INCH));
	}

	ui.sbLineSymbolWidth->setSuffix(suffix);
	if (xPosition != static_cast<int>(WorksheetElement::HorizontalPosition::Relative))
		ui.sbPositionX->setSuffix(suffix);
	if (yPosition != static_cast<int>(WorksheetElement::VerticalPosition::Relative))
		ui.sbPositionY->setSuffix(suffix);
	ui.sbBorderCornerRadius->setSuffix(suffix);
	ui.sbLayoutTopMargin->setSuffix(suffix);
	ui.sbLayoutBottomMargin->setSuffix(suffix);
	ui.sbLayoutLeftMargin->setSuffix(suffix);
	ui.sbLayoutRightMargin->setSuffix(suffix);
	ui.sbLayoutHorizontalSpacing->setSuffix(suffix);
	ui.sbLayoutVerticalSpacing->setSuffix(suffix);

	labelWidget->updateUnits();
}

//************************************************************
//** SLOTs for changes triggered in CartesianPlotLegendDock **
//************************************************************
void CartesianPlotLegendDock::retranslateUi() {
	CONDITIONAL_LOCK_RETURN;

	ui.cbOrder->clear();
	ui.cbOrder->addItem(i18n("Column Major"));
	ui.cbOrder->addItem(i18n("Row Major"));

	// Positioning and alignment
	ui.cbPositionX->clear();
	ui.cbPositionX->addItem(i18n("Left"));
	ui.cbPositionX->addItem(i18n("Center"));
	ui.cbPositionX->addItem(i18n("Right"));
	ui.cbPositionX->addItem(i18n("Relative to plot"));

	ui.cbPositionY->clear();
	ui.cbPositionY->addItem(i18n("Top"));
	ui.cbPositionY->addItem(i18n("Center"));
	ui.cbPositionY->addItem(i18n("Bottom"));
	ui.cbPositionY->addItem(i18n("Relative to plot"));

	ui.cbHorizontalAlignment->clear();
	ui.cbHorizontalAlignment->addItem(i18n("Left"));
	ui.cbHorizontalAlignment->addItem(i18n("Center"));
	ui.cbHorizontalAlignment->addItem(i18n("Right"));

	ui.cbVerticalAlignment->clear();
	ui.cbVerticalAlignment->addItem(i18n("Top"));
	ui.cbVerticalAlignment->addItem(i18n("Center"));
	ui.cbVerticalAlignment->addItem(i18n("Bottom"));

	// tooltip texts
	QString info = i18n("Use the main color of the plot (line, symbol, etc.) for the color of the name in the legend.");
	ui.lUsePlotColor->setToolTip(info);
	ui.chkUsePlotColor->setToolTip(info);
}

// "General"-tab
void CartesianPlotLegendDock::lockChanged(bool locked) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* legend : m_legendList)
		legend->setLock(locked);
}

// General
void CartesianPlotLegendDock::labelFontChanged(const QFont& font) {
	CONDITIONAL_LOCK_RETURN;

	QFont labelsFont = font;
	labelsFont.setPointSizeF(Worksheet::convertToSceneUnits(font.pointSizeF(), Worksheet::Unit::Point));
	for (auto* legend : m_legendList)
		legend->setLabelFont(labelsFont);
}

void CartesianPlotLegendDock::usePlotColorChanged(bool checked) {
	ui.lFontColor->setVisible(!checked);
	ui.kcbLabelColor->setVisible(!checked);

	CONDITIONAL_LOCK_RETURN;

	for (auto* legend : m_legendList)
		legend->setUsePlotColor(checked);
}

void CartesianPlotLegendDock::labelColorChanged(const QColor& color) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* legend : m_legendList)
		legend->setLabelColor(color);
}

void CartesianPlotLegendDock::labelOrderChanged(const int index) {
	CONDITIONAL_LOCK_RETURN;

	bool columnMajor = (index == 0);
	for (auto* legend : m_legendList)
		legend->setLabelColumnMajor(columnMajor);
}

void CartesianPlotLegendDock::lineSymbolWidthChanged(double value) {
	CONDITIONAL_RETURN_NO_LOCK;

	for (auto* legend : m_legendList)
		legend->setLineSymbolWidth(Worksheet::convertToSceneUnits(value, m_worksheetUnit));
}

/*!
	called when legend's current horizontal position relative to its parent (left, center, right, relative) is changed.
*/
void CartesianPlotLegendDock::positionXChanged(int index) {
	CONDITIONAL_LOCK_RETURN;

	auto position = m_legend->position();
	auto oldHorizontalPosition = position.horizontalPosition;
	position.horizontalPosition = CartesianPlotLegend::HorizontalPosition(index);
	double x = 0.;
	if (position.horizontalPosition == WorksheetElement::HorizontalPosition::Relative) {
		switch (oldHorizontalPosition) {
		case WorksheetElement::HorizontalPosition::Left:
		case WorksheetElement::HorizontalPosition::Relative:
			break;
		case WorksheetElement::HorizontalPosition::Center:
			x = 0.5;
			break;
		case WorksheetElement::HorizontalPosition::Right:
			x = 1.0;
		}
		ui.sbPositionX->setSuffix(QStringLiteral(" %"));
	} else {
		if (m_units == Units::Metric)
			ui.sbPositionX->setSuffix(QStringLiteral(" cm"));
		else
			ui.sbPositionX->setSuffix(QStringLiteral(" in"));
	}

	position.point.setX(x);
	ui.sbPositionX->setValue(std::round(100. * x));

	for (auto* legend : m_legendList)
		legend->setPosition(position);
}

/*!
	called when legend's current vertical position relative to its parent (top, center, bottom, custom) is changed.
*/
void CartesianPlotLegendDock::positionYChanged(int index) {
	CONDITIONAL_LOCK_RETURN;

	CartesianPlotLegend::PositionWrapper position = m_legend->position();
	auto oldVerticalPosition = position.verticalPosition;
	position.verticalPosition = CartesianPlotLegend::VerticalPosition(index);
	double y = 0.;
	if (position.verticalPosition == WorksheetElement::VerticalPosition::Relative) {
		switch (oldVerticalPosition) {
		case WorksheetElement::VerticalPosition::Top:
		case WorksheetElement::VerticalPosition::Relative:
			break;
		case WorksheetElement::VerticalPosition::Center:
			y = 0.5;
			break;
		case WorksheetElement::VerticalPosition::Bottom:
			y = 1.0;
		}
		ui.sbPositionY->setSuffix(QStringLiteral(" %"));
	} else {
		if (m_units == Units::Metric)
			ui.sbPositionY->setSuffix(QStringLiteral(" cm"));
		else
			ui.sbPositionY->setSuffix(QStringLiteral(" in"));
	}

	position.point.setY(y);
	ui.sbPositionY->setValue(std::round(100. * y));

	for (auto* legend : m_legendList)
		legend->setPosition(position);
}

void CartesianPlotLegendDock::customPositionXChanged(double value) {
	CONDITIONAL_RETURN_NO_LOCK;

	for (auto* legend : m_legendList) {
		auto position = legend->position();
		if (position.horizontalPosition == WorksheetElement::HorizontalPosition::Relative)
			position.point.setX(value / 100.);
		else
			position.point.setX(Worksheet::convertToSceneUnits(value, m_worksheetUnit));
		legend->setPosition(position);
	}
}

void CartesianPlotLegendDock::customPositionYChanged(double value) {
	CONDITIONAL_RETURN_NO_LOCK;

	for (auto* legend : m_legendList) {
		auto position = legend->position();
		if (position.verticalPosition == WorksheetElement::VerticalPosition::Relative)
			position.point.setY(value / 100.);
		else
			position.point.setY(Worksheet::convertToSceneUnits(value, m_worksheetUnit));
		legend->setPosition(position);
	}
}
void CartesianPlotLegendDock::horizontalAlignmentChanged(int index) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* legend : m_legendList)
		legend->setHorizontalAlignment(WorksheetElement::HorizontalAlignment(index));
}

void CartesianPlotLegendDock::verticalAlignmentChanged(int index) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* legend : m_legendList)
		legend->setVerticalAlignment(WorksheetElement::VerticalAlignment(index));
}

void CartesianPlotLegendDock::rotationChanged(int value) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* legend : m_legendList)
		legend->setRotationAngle(value);
}

/*!
 * \brief CartesianPlotLegendDock::bindingChanged
 * Bind CartesianPlotLegend to the cartesian plot coords or not
 * \param checked
 */
void CartesianPlotLegendDock::bindingChanged(bool checked) {
	// widgets for positioning using absolute plot distances
	ui.lPositionX->setVisible(!checked);
	ui.cbPositionX->setVisible(!checked);
	ui.sbPositionX->setVisible(!checked);

	ui.lPositionY->setVisible(!checked);
	ui.cbPositionY->setVisible(!checked);
	ui.sbPositionY->setVisible(!checked);

	// widgets for positioning using logical plot coordinates
	const auto* plot = static_cast<const CartesianPlot*>(m_legend->parent(AspectType::CartesianPlot));
	if (plot && plot->xRangeFormatDefault() == RangeT::Format::DateTime) {
		ui.lPositionXLogicalDateTime->setVisible(checked);
		ui.dtePositionXLogical->setVisible(checked);

		ui.lPositionXLogical->setVisible(false);
		ui.sbPositionXLogical->setVisible(false);
	} else {
		ui.lPositionXLogicalDateTime->setVisible(false);
		ui.dtePositionXLogical->setVisible(false);

		ui.lPositionXLogical->setVisible(checked);
		ui.sbPositionXLogical->setVisible(checked);
	}

	ui.lPositionYLogical->setVisible(checked);
	ui.sbPositionYLogical->setVisible(checked);

	CONDITIONAL_LOCK_RETURN;

	for (auto* legend : m_legendList)
		legend->setCoordinateBindingEnabled(checked);
}

// "Border"-tab
void CartesianPlotLegendDock::borderCornerRadiusChanged(double value) {
	CONDITIONAL_RETURN_NO_LOCK;

	for (auto* legend : m_legendList)
		legend->setBorderCornerRadius(Worksheet::convertToSceneUnits(value, m_worksheetUnit));
}

// Layout
void CartesianPlotLegendDock::layoutTopMarginChanged(double margin) {
	CONDITIONAL_RETURN_NO_LOCK;

	for (auto* legend : m_legendList)
		legend->setLayoutTopMargin(Worksheet::convertToSceneUnits(margin, m_worksheetUnit));
}

void CartesianPlotLegendDock::layoutBottomMarginChanged(double margin) {
	CONDITIONAL_RETURN_NO_LOCK;

	for (auto* legend : m_legendList)
		legend->setLayoutBottomMargin(Worksheet::convertToSceneUnits(margin, m_worksheetUnit));
}

void CartesianPlotLegendDock::layoutLeftMarginChanged(double margin) {
	CONDITIONAL_RETURN_NO_LOCK;

	for (auto* legend : m_legendList)
		legend->setLayoutLeftMargin(Worksheet::convertToSceneUnits(margin, m_worksheetUnit));
}

void CartesianPlotLegendDock::layoutRightMarginChanged(double margin) {
	CONDITIONAL_RETURN_NO_LOCK;

	for (auto* legend : m_legendList)
		legend->setLayoutRightMargin(Worksheet::convertToSceneUnits(margin, m_worksheetUnit));
}

void CartesianPlotLegendDock::layoutHorizontalSpacingChanged(double spacing) {
	CONDITIONAL_RETURN_NO_LOCK;

	for (auto* legend : m_legendList)
		legend->setLayoutHorizontalSpacing(Worksheet::convertToSceneUnits(spacing, m_worksheetUnit));
}

void CartesianPlotLegendDock::layoutVerticalSpacingChanged(double spacing) {
	CONDITIONAL_RETURN_NO_LOCK;

	for (auto* legend : m_legendList)
		legend->setLayoutVerticalSpacing(Worksheet::convertToSceneUnits(spacing, m_worksheetUnit));
}

void CartesianPlotLegendDock::layoutColumnCountChanged(int count) {
	ui.lOrder->setVisible(count != 1);
	ui.cbOrder->setVisible(count != 1);

	CONDITIONAL_LOCK_RETURN;

	for (auto* legend : m_legendList)
		legend->setLayoutColumnCount(count);
}

//*************************************************************
//**** SLOTs for changes triggered in CartesianPlotLegend *****
//*************************************************************
// General
void CartesianPlotLegendDock::legendLabelFontChanged(QFont& font) {
	CONDITIONAL_LOCK_RETURN;
	// we need to set the font size in points for KFontRequester
	QFont f(font);
	f.setPointSizeF(round(Worksheet::convertFromSceneUnits(f.pointSizeF(), Worksheet::Unit::Point)));
	ui.kfrLabelFont->setFont(f);
}

void CartesianPlotLegendDock::legendUsePlotColorChanged(bool usePlotColor) {
	CONDITIONAL_LOCK_RETURN;
	ui.chkUsePlotColor->setChecked(usePlotColor);
}

void CartesianPlotLegendDock::legendLabelColorChanged(QColor& color) {
	CONDITIONAL_LOCK_RETURN;
	ui.kcbLabelColor->setColor(color);
}

void CartesianPlotLegendDock::legendLabelOrderChanged(bool b) {
	CONDITIONAL_LOCK_RETURN;
	if (b)
		ui.cbOrder->setCurrentIndex(0); // column major
	else
		ui.cbOrder->setCurrentIndex(1); // row major
}

void CartesianPlotLegendDock::legendLineSymbolWidthChanged(float value) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbLineSymbolWidth->setValue(Worksheet::convertFromSceneUnits(roundSceneValue(value, m_units), m_worksheetUnit));
}

void CartesianPlotLegendDock::legendHorizontalAlignmentChanged(TextLabel::HorizontalAlignment index) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbHorizontalAlignment->setCurrentIndex(static_cast<int>(index));
}

void CartesianPlotLegendDock::legendVerticalAlignmentChanged(TextLabel::VerticalAlignment index) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbVerticalAlignment->setCurrentIndex(static_cast<int>(index));
}

// called when anchor changes
void CartesianPlotLegendDock::legendPositionChanged(const CartesianPlotLegend::PositionWrapper& position) {
	CONDITIONAL_LOCK_RETURN;

	ui.cbPositionX->setCurrentIndex(static_cast<int>(position.horizontalPosition));
	ui.cbPositionY->setCurrentIndex(static_cast<int>(position.verticalPosition));
	if (position.horizontalPosition == WorksheetElement::HorizontalPosition::Relative) {
		ui.sbPositionX->setValue(std::round(position.point.x() * 100.));
		ui.sbPositionX->setSuffix(QStringLiteral(" %"));
	} else
		ui.sbPositionX->setValue(Worksheet::convertFromSceneUnits(roundSceneValue(position.point.x(), m_units), m_worksheetUnit));

	if (position.verticalPosition == WorksheetElement::VerticalPosition::Relative) {
		ui.sbPositionY->setValue(std::round(position.point.y() * 100.));
		ui.sbPositionY->setSuffix(QStringLiteral(" %"));
	} else
		ui.sbPositionY->setValue(Worksheet::convertFromSceneUnits(roundSceneValue(position.point.y(), m_units), m_worksheetUnit));
}

// called when position relative to anchor changes (aka. position.point)
void CartesianPlotLegendDock::legendPositionLogicalChanged(QPointF pos) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbPositionXLogical->setValue(pos.x());
	ui.dtePositionXLogical->setMSecsSinceEpochUTC(pos.x());
	ui.sbPositionYLogical->setValue(pos.y());
	// TODO: why not ui.dtePositionYLogical->setMSecsSinceEpochUTC(pos.y());
}

void CartesianPlotLegendDock::legendRotationAngleChanged(qreal angle) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbRotation->setValue(angle);
}

void CartesianPlotLegendDock::legendLockChanged(bool on) {
	CONDITIONAL_LOCK_RETURN;
	ui.chbLock->setChecked(on);
}

// Border
void CartesianPlotLegendDock::legendBorderCornerRadiusChanged(float value) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbBorderCornerRadius->setValue(Worksheet::convertFromSceneUnits(value, m_worksheetUnit));
}

// Layout
void CartesianPlotLegendDock::legendLayoutTopMarginChanged(float value) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbLayoutTopMargin->setValue(Worksheet::convertFromSceneUnits(value, m_worksheetUnit));
}

void CartesianPlotLegendDock::legendLayoutBottomMarginChanged(float value) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbLayoutBottomMargin->setValue(Worksheet::convertFromSceneUnits(value, m_worksheetUnit));
}

void CartesianPlotLegendDock::legendLayoutLeftMarginChanged(float value) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbLayoutLeftMargin->setValue(Worksheet::convertFromSceneUnits(value, m_worksheetUnit));
}

void CartesianPlotLegendDock::legendLayoutRightMarginChanged(float value) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbLayoutRightMargin->setValue(Worksheet::convertFromSceneUnits(value, m_worksheetUnit));
}

void CartesianPlotLegendDock::legendLayoutVerticalSpacingChanged(float value) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbLayoutVerticalSpacing->setValue(Worksheet::convertFromSceneUnits(value, m_worksheetUnit));
}

void CartesianPlotLegendDock::legendLayoutHorizontalSpacingChanged(float value) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbLayoutHorizontalSpacing->setValue(Worksheet::convertFromSceneUnits(value, m_worksheetUnit));
}

void CartesianPlotLegendDock::legendLayoutColumnCountChanged(int value) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbLayoutColumnCount->setValue(value);
}

//*************************************************************
//******************** SETTINGS *******************************
//*************************************************************
void CartesianPlotLegendDock::load() {
	// General-tab

	// Format
	// we need to set the font size in points for KFontRequester
	QFont font = m_legend->labelFont();
	font.setPointSizeF(std::round(Worksheet::convertFromSceneUnits(font.pointSizeF(), Worksheet::Unit::Point)));
	ui.kfrLabelFont->setFont(font);

	ui.chkUsePlotColor->setChecked(m_legend->usePlotColor());
	usePlotColorChanged(ui.chkUsePlotColor->isChecked());
	ui.kcbLabelColor->setColor(m_legend->labelColor());
	bool columnMajor = m_legend->labelColumnMajor();
	if (columnMajor)
		ui.cbOrder->setCurrentIndex(0); // column major
	else
		ui.cbOrder->setCurrentIndex(1); // row major

	ui.sbLineSymbolWidth->setValue(Worksheet::convertFromSceneUnits(roundSceneValue(m_legend->lineSymbolWidth(), m_units), m_worksheetUnit));

	// Geometry

	// widgets for positioning using absolute plot distances
	ui.cbPositionX->setCurrentIndex((int)m_legend->position().horizontalPosition);
	// positionXChanged(ui.cbPositionX->currentIndex());
	if (m_legend->position().horizontalPosition == WorksheetElement::HorizontalPosition::Relative) {
		ui.sbPositionX->setValue(std::round(m_legend->position().point.x() * 100.));
		ui.sbPositionX->setSuffix(QStringLiteral(" %"));
	} else
		ui.sbPositionX->setValue(Worksheet::convertFromSceneUnits(roundSceneValue(m_legend->position().point.x(), m_units), m_worksheetUnit));
	ui.cbPositionY->setCurrentIndex((int)m_legend->position().verticalPosition);
	// positionYChanged(ui.cbPositionY->currentIndex());
	if (m_legend->position().verticalPosition == WorksheetElement::VerticalPosition::Relative) {
		ui.sbPositionY->setValue(std::round(m_legend->position().point.y() * 100.));
		ui.sbPositionY->setSuffix(QStringLiteral(" %"));
	} else
		ui.sbPositionY->setValue(Worksheet::convertFromSceneUnits(roundSceneValue(m_legend->position().point.y(), m_units), m_worksheetUnit));

	ui.cbHorizontalAlignment->setCurrentIndex((int)m_legend->horizontalAlignment());
	ui.cbVerticalAlignment->setCurrentIndex((int)m_legend->verticalAlignment());

	// widgets for positioning using logical plot coordinates
	bool allowLogicalCoordinates = (m_legend->plot() != nullptr);
	ui.lBindLogicalPos->setVisible(allowLogicalCoordinates);
	ui.chbBindLogicalPos->setVisible(allowLogicalCoordinates);

	if (allowLogicalCoordinates) {
		const auto* plot = static_cast<const CartesianPlot*>(m_legend->plot());
		if (plot->xRangeFormatDefault() == RangeT::Format::Numeric) {
			ui.lPositionXLogical->show();
			ui.sbPositionXLogical->show();
			ui.lPositionXLogicalDateTime->hide();
			ui.dtePositionXLogical->hide();

			ui.sbPositionXLogical->setValue(m_legend->positionLogical().x());
			ui.sbPositionYLogical->setValue(m_legend->positionLogical().y());
		} else { // DateTime
			ui.lPositionXLogical->hide();
			ui.sbPositionXLogical->hide();
			ui.lPositionXLogicalDateTime->show();
			ui.dtePositionXLogical->show();

			ui.dtePositionXLogical->setDisplayFormat(plot->rangeDateTimeFormat(Dimension::X));
			ui.dtePositionXLogical->setMSecsSinceEpochUTC(m_legend->positionLogical().x());
		}

		ui.chbBindLogicalPos->setChecked(m_legend->coordinateBindingEnabled());
		bindingChanged(m_legend->coordinateBindingEnabled());
	} else {
		ui.lPositionXLogical->hide();
		ui.sbPositionXLogical->hide();
		ui.lPositionYLogical->hide();
		ui.sbPositionYLogical->hide();
		ui.lPositionXLogicalDateTime->hide();
		ui.dtePositionXLogical->hide();
	}

	ui.sbRotation->setValue(m_legend->rotationAngle());
	ui.chbLock->setChecked(m_legend->isLocked());
	ui.chkVisible->setChecked(m_legend->isVisible());

	// legend title, background and border line
	QList<Background*> backgrounds;
	QList<TextLabel*> labels;
	QList<Line*> borderLines;
	for (auto* legend : m_legendList) {
		labels << legend->title();
		backgrounds << legend->background();
		borderLines << legend->borderLine();
	}

	labelWidget->setLabels(labels);
	backgroundWidget->setBackgrounds(backgrounds);
	borderLineWidget->setLines(borderLines);

	// Border
	ui.sbBorderCornerRadius->setValue(Worksheet::convertFromSceneUnits(roundSceneValue(m_legend->borderCornerRadius(), m_units), m_worksheetUnit));

	// Layout
	ui.sbLayoutTopMargin->setValue(Worksheet::convertFromSceneUnits(roundSceneValue(m_legend->layoutTopMargin(), m_units), m_worksheetUnit));
	ui.sbLayoutBottomMargin->setValue(Worksheet::convertFromSceneUnits(roundSceneValue(m_legend->layoutBottomMargin(), m_units), m_worksheetUnit));
	ui.sbLayoutLeftMargin->setValue(Worksheet::convertFromSceneUnits(roundSceneValue(m_legend->layoutLeftMargin(), m_units), m_worksheetUnit));
	ui.sbLayoutRightMargin->setValue(Worksheet::convertFromSceneUnits(roundSceneValue(m_legend->layoutRightMargin(), m_units), m_worksheetUnit));
	ui.sbLayoutHorizontalSpacing->setValue(Worksheet::convertFromSceneUnits(roundSceneValue(m_legend->layoutHorizontalSpacing(), m_units), m_worksheetUnit));
	ui.sbLayoutVerticalSpacing->setValue(Worksheet::convertFromSceneUnits(roundSceneValue(m_legend->layoutVerticalSpacing(), m_units), m_worksheetUnit));

	ui.sbLayoutColumnCount->setValue(m_legend->layoutColumnCount());
}

void CartesianPlotLegendDock::loadConfigFromTemplate(KConfig& config) {
	auto name = TemplateHandler::templateName(config);
	int size = m_legendList.size();
	if (size > 1)
		m_legend->beginMacro(i18n("%1 cartesian plot legends: template \"%2\" loaded", size, name));
	else
		m_legend->beginMacro(i18n("%1: template \"%2\" loaded", m_legend->name(), name));

	this->loadConfig(config);

	m_legend->endMacro();
}

void CartesianPlotLegendDock::loadConfig(KConfig& config) {
	KConfigGroup group = config.group(QStringLiteral("CartesianPlotLegend"));

	// General-tab

	// Format
	// we need to set the font size in points for KFontRequester
	QFont font = m_legend->labelFont();
	font.setPointSizeF(std::round(Worksheet::convertFromSceneUnits(font.pointSizeF(), Worksheet::Unit::Point)));
	ui.kfrLabelFont->setFont(group.readEntry(QStringLiteral("LabelFont"), font));

	ui.chkUsePlotColor->setChecked(group.readEntry(QStringLiteral("UsePlotColor"), m_legend->usePlotColor()));
	usePlotColorChanged(ui.chkUsePlotColor->isChecked());
	ui.kcbLabelColor->setColor(group.readEntry(QStringLiteral("LabelColor"), m_legend->labelColor()));

	bool columnMajor = group.readEntry(QStringLiteral("LabelColumMajor"), m_legend->labelColumnMajor());
	if (columnMajor)
		ui.cbOrder->setCurrentIndex(0); // column major
	else
		ui.cbOrder->setCurrentIndex(1); // row major

	ui.sbLineSymbolWidth->setValue(
		roundValue(group.readEntry(QStringLiteral("LineSymbolWidth"),
								   Worksheet::convertFromSceneUnits(roundSceneValue(m_legend->lineSymbolWidth(), m_units), m_worksheetUnit))));

	// Geometry
	ui.cbPositionX->setCurrentIndex(group.readEntry(QStringLiteral("PositionX"), (int)m_legend->position().horizontalPosition));
	ui.sbPositionX->setValue(
		Worksheet::convertFromSceneUnits(roundSceneValue(group.readEntry(QStringLiteral("PositionXValue"), m_legend->position().point.x()), m_units),
										 m_worksheetUnit));
	ui.cbPositionY->setCurrentIndex(group.readEntry(QStringLiteral("PositionY"), (int)m_legend->position().verticalPosition));
	ui.sbPositionY->setValue(
		Worksheet::convertFromSceneUnits(roundSceneValue(group.readEntry(QStringLiteral("PositionYValue"), m_legend->position().point.y()), m_units),
										 m_worksheetUnit));
	ui.sbRotation->setValue(group.readEntry(QStringLiteral("Rotation"), std::round(m_legend->rotationAngle())));

	ui.chkVisible->setChecked(group.readEntry(QStringLiteral("Visible"), m_legend->isVisible()));

	// Background-tab
	backgroundWidget->loadConfig(group);

	// Border
	borderLineWidget->loadConfig(group);
	ui.sbBorderCornerRadius->setValue(
		Worksheet::convertFromSceneUnits(roundSceneValue(group.readEntry(QStringLiteral("BorderCornerRadius"), m_legend->borderCornerRadius()), m_units),
										 m_worksheetUnit));

	// Layout
	ui.sbLayoutTopMargin->setValue(group.readEntry(QStringLiteral("LayoutTopMargin"),
												   Worksheet::convertFromSceneUnits(roundSceneValue(m_legend->layoutTopMargin(), m_units), m_worksheetUnit)));
	ui.sbLayoutBottomMargin->setValue(
		group.readEntry(QStringLiteral("LayoutBottomMargin"),
						Worksheet::convertFromSceneUnits(roundSceneValue(m_legend->layoutBottomMargin(), m_units), m_worksheetUnit)));
	ui.sbLayoutLeftMargin->setValue(group.readEntry(QStringLiteral("LayoutLeftMargin"),
													Worksheet::convertFromSceneUnits(roundSceneValue(m_legend->layoutLeftMargin(), m_units), m_worksheetUnit)));
	ui.sbLayoutRightMargin->setValue(
		group.readEntry(QStringLiteral("LayoutRightMargin"),
						Worksheet::convertFromSceneUnits(roundSceneValue(m_legend->layoutRightMargin(), m_units), m_worksheetUnit)));
	ui.sbLayoutHorizontalSpacing->setValue(
		group.readEntry(QStringLiteral("LayoutHorizontalSpacing"),
						Worksheet::convertFromSceneUnits(roundSceneValue(m_legend->layoutHorizontalSpacing(), m_units), m_worksheetUnit)));
	ui.sbLayoutVerticalSpacing->setValue(
		group.readEntry(QStringLiteral("LayoutVerticalSpacing"),
						Worksheet::convertFromSceneUnits(roundSceneValue(m_legend->layoutVerticalSpacing(), m_units), m_worksheetUnit)));
	ui.sbLayoutColumnCount->setValue(group.readEntry(QStringLiteral("LayoutColumnCount"), m_legend->layoutColumnCount()));

	// Title
	group = config.group(QStringLiteral("PlotLegend"));
	labelWidget->loadConfig(group);
}

void CartesianPlotLegendDock::saveConfigAsTemplate(KConfig& config) {
	KConfigGroup group = config.group(QStringLiteral("CartesianPlotLegend"));

	// General-tab
	// Format
	QFont font = m_legend->labelFont();
	font.setPointSizeF(Worksheet::convertFromSceneUnits(font.pointSizeF(), Worksheet::Unit::Point));
	group.writeEntry(QStringLiteral("LabelFont"), font);
	group.writeEntry(QStringLiteral("LabelColor"), ui.kcbLabelColor->color());
	group.writeEntry(QStringLiteral("LabelColumMajorOrder"), ui.cbOrder->currentIndex() == 0); // true for "column major", false for "row major"
	group.writeEntry(QStringLiteral("LineSymbolWidth"), Worksheet::convertToSceneUnits(ui.sbLineSymbolWidth->value(), m_worksheetUnit));

	// Geometry
	group.writeEntry(QStringLiteral("PositionX"), ui.cbPositionX->currentIndex());
	group.writeEntry(QStringLiteral("PositionXValue"), Worksheet::convertToSceneUnits(ui.sbPositionX->value(), m_worksheetUnit));
	group.writeEntry(QStringLiteral("PositionY"), ui.cbPositionY->currentIndex());
	group.writeEntry(QStringLiteral("PositionYValue"), Worksheet::convertToSceneUnits(ui.sbPositionY->value(), m_worksheetUnit));
	group.writeEntry(QStringLiteral("Rotation"), ui.sbRotation->value());

	group.writeEntry(QStringLiteral("Visible"), ui.chkVisible->isChecked());

	// Background
	backgroundWidget->saveConfig(group);

	// Border
	borderLineWidget->saveConfig(group);
	group.writeEntry(QStringLiteral("BorderCornerRadius"), Worksheet::convertToSceneUnits(ui.sbBorderCornerRadius->value(), m_worksheetUnit));

	// Layout
	group.writeEntry(QStringLiteral("LayoutTopMargin"), Worksheet::convertToSceneUnits(ui.sbLayoutTopMargin->value(), m_worksheetUnit));
	group.writeEntry(QStringLiteral("LayoutBottomMargin"), Worksheet::convertToSceneUnits(ui.sbLayoutBottomMargin->value(), m_worksheetUnit));
	group.writeEntry(QStringLiteral("LayoutLeftMargin"), Worksheet::convertToSceneUnits(ui.sbLayoutLeftMargin->value(), m_worksheetUnit));
	group.writeEntry(QStringLiteral("LayoutRightMargin"), Worksheet::convertToSceneUnits(ui.sbLayoutRightMargin->value(), m_worksheetUnit));
	group.writeEntry(QStringLiteral("LayoutVerticalSpacing"), Worksheet::convertToSceneUnits(ui.sbLayoutVerticalSpacing->value(), m_worksheetUnit));
	group.writeEntry(QStringLiteral("LayoutHorizontalSpacing"), Worksheet::convertToSceneUnits(ui.sbLayoutHorizontalSpacing->value(), m_worksheetUnit));
	group.writeEntry(QStringLiteral("LayoutColumnCount"), ui.sbLayoutColumnCount->value());

	// Title
	group = config.group(QStringLiteral("PlotLegend"));
	labelWidget->saveConfig(group);

	config.sync();
}
