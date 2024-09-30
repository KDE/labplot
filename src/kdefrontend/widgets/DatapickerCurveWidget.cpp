/*
	File                 : ImageWidget.cpp
	Project              : LabPlot
	Description          : widget for datapicker properties
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2015 Ankit Wagadre <wagadre.ankit@gmail.com>
	SPDX-FileCopyrightText: 2015-2019 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "DatapickerCurveWidget.h"
#include "backend/datapicker/DatapickerPoint.h"
#include "backend/worksheet/Worksheet.h"
#include "kdefrontend/GuiTools.h"
#include "kdefrontend/widgets/SymbolWidget.h"

#include <KLocalizedString>

#include <QPainter>

#include <cmath>

DatapickerCurveWidget::DatapickerCurveWidget(QWidget* parent)
	: BaseDock(parent) {
	ui.setupUi(this);
	setBaseWidgets(ui.leName, ui.teComment);

	ui.cbXErrorType->addItem(i18n("No Error"));
	ui.cbXErrorType->addItem(i18n("Symmetric"));
	ui.cbXErrorType->addItem(i18n("Asymmetric"));

	ui.cbYErrorType->addItem(i18n("No Error"));
	ui.cbYErrorType->addItem(i18n("Symmetric"));
	ui.cbYErrorType->addItem(i18n("Asymmetric"));

	QString info = i18n(
		"Specify whether the data points have errors and of which type.\n"
		"Note, changing this type is not possible once at least one point was read.");
	ui.lXErrorType->setToolTip(info);
	ui.cbXErrorType->setToolTip(info);
	ui.lYErrorType->setToolTip(info);
	ui.cbYErrorType->setToolTip(info);

	//"Symbol"-tab
	auto* hboxLayout = new QHBoxLayout(ui.tabSymbol);
	symbolWidget = new SymbolWidget(ui.tabSymbol);
	hboxLayout->addWidget(symbolWidget);
	hboxLayout->setContentsMargins(2, 2, 2, 2);
	hboxLayout->setSpacing(2);

	GuiTools::updateBrushStyles(ui.cbErrorBarFillingStyle, Qt::black);
	DatapickerCurveWidget::updateLocale();

	// General
	connect(ui.cbXErrorType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DatapickerCurveWidget::xErrorTypeChanged);
	connect(ui.cbYErrorType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DatapickerCurveWidget::yErrorTypeChanged);
	connect(ui.chkVisible, &QCheckBox::clicked, this, &DatapickerCurveWidget::visibilityChanged);

	// error bar
	connect(ui.cbErrorBarFillingStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DatapickerCurveWidget::errorBarFillingStyleChanged);
	connect(ui.kcbErrorBarFillingColor, &KColorButton::changed, this, &DatapickerCurveWidget::errorBarFillingColorChanged);
	connect(ui.sbErrorBarSize, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &DatapickerCurveWidget::errorBarSizeChanged);

	hideErrorBarWidgets(true);
}

DatapickerCurveWidget::~DatapickerCurveWidget() = default;

void DatapickerCurveWidget::setCurves(QList<DatapickerCurve*> list) {
	if (list.isEmpty())
		return;

	m_curveList = list;
	m_curve = list.first();
	setAspects(list);

	load();
	updateSymbolWidgets();

	QList<Symbol*> symbols;
	for (auto* curve : m_curveList)
		symbols << curve->symbol();

	symbolWidget->setSymbols(symbols);

	connect(m_curve, &AbstractAspect::childAspectRemoved, this, &DatapickerCurveWidget::updateSymbolWidgets);
	connect(m_curve, &AbstractAspect::childAspectAdded, this, &DatapickerCurveWidget::updateSymbolWidgets);
	connect(m_curve, &DatapickerCurve::curveErrorTypesChanged, this, &DatapickerCurveWidget::curveErrorsChanged);
	connect(m_curve, &DatapickerCurve::pointVisibilityChanged, this, &DatapickerCurveWidget::symbolVisibleChanged);
	connect(m_curve, &DatapickerCurve::pointErrorBarBrushChanged, this, &DatapickerCurveWidget::symbolErrorBarBrushChanged);
	connect(m_curve, &DatapickerCurve::pointErrorBarSizeChanged, this, &DatapickerCurveWidget::symbolErrorBarSizeChanged);
}

void DatapickerCurveWidget::hideErrorBarWidgets(bool on) {
	ui.cbErrorBarFillingStyle->setEnabled(!on);
	ui.kcbErrorBarFillingColor->setEnabled(!on);
	ui.lErrorBarFillingColor->setEnabled(!on);
	ui.lErrorBarFillingStyle->setEnabled(!on);
	ui.sbErrorBarSize->setEnabled(!on);
	ui.lErrorBarSize->setEnabled(!on);
}

void DatapickerCurveWidget::updateLocale() {
	ui.sbErrorBarSize->setLocale(QLocale());
}

//*************************************************************
//**** SLOTs for changes triggered in DatapickerCurveWidget ***
//*************************************************************
//"General"-tab
void DatapickerCurveWidget::xErrorTypeChanged(int index) {
	if (DatapickerCurve::ErrorType(index) != DatapickerCurve::ErrorType::NoError || m_curve->curveErrorTypes().y != DatapickerCurve::ErrorType::NoError)
		hideErrorBarWidgets(false);
	else
		hideErrorBarWidgets(true);

	if (m_suppressTypeChange)
		return;

	CONDITIONAL_LOCK_RETURN;

	DatapickerCurve::Errors errors = m_curve->curveErrorTypes();
	errors.x = DatapickerCurve::ErrorType(index);

	for (auto* curve : m_curveList)
		curve->setCurveErrorTypes(errors);
}

void DatapickerCurveWidget::yErrorTypeChanged(int index) {
	if (DatapickerCurve::ErrorType(index) != DatapickerCurve::ErrorType::NoError || m_curve->curveErrorTypes().x != DatapickerCurve::ErrorType::NoError)
		hideErrorBarWidgets(false);
	else
		hideErrorBarWidgets(true);

	if (m_suppressTypeChange)
		return;

	CONDITIONAL_LOCK_RETURN;

	DatapickerCurve::Errors errors = m_curve->curveErrorTypes();
	errors.y = DatapickerCurve::ErrorType(index);

	for (auto* curve : m_curveList)
		curve->setCurveErrorTypes(errors);
}

void DatapickerCurveWidget::errorBarSizeChanged(double value) {
	CONDITIONAL_RETURN_NO_LOCK;

	for (auto* curve : m_curveList)
		curve->setPointErrorBarSize(Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point));
}

void DatapickerCurveWidget::errorBarFillingStyleChanged(int index) {
	auto brushStyle = Qt::BrushStyle(index);
	ui.kcbErrorBarFillingColor->setEnabled(!(brushStyle == Qt::NoBrush));

	CONDITIONAL_LOCK_RETURN;

	QBrush brush;
	for (auto* curve : m_curveList) {
		brush = curve->pointErrorBarBrush();
		brush.setStyle(brushStyle);
		curve->setPointErrorBarBrush(brush);
	}
}

void DatapickerCurveWidget::errorBarFillingColorChanged(const QColor& color) {
	CONDITIONAL_LOCK_RETURN;

	QBrush brush;
	for (auto* curve : m_curveList) {
		brush = curve->pointErrorBarBrush();
		brush.setColor(color);
		curve->setPointErrorBarBrush(brush);
	}
	GuiTools::updateBrushStyles(ui.cbErrorBarFillingStyle, color);
}

void DatapickerCurveWidget::visibilityChanged(bool state) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* curve : m_curveList)
		curve->setPointVisibility(state);
}

void DatapickerCurveWidget::updateSymbolWidgets() {
	auto list = m_curve->children<DatapickerPoint>(AbstractAspect::ChildIndexFlag::IncludeHidden);
	if (list.isEmpty()) {
		ui.cbXErrorType->setEnabled(true);
		ui.cbYErrorType->setEnabled(true);
		m_suppressTypeChange = false;
	} else {
		ui.cbXErrorType->setEnabled(false);
		ui.cbYErrorType->setEnabled(false);
		m_suppressTypeChange = true;
	}
}

//*************************************************************
//******** SLOTs for changes triggered in DatapickerCurve *****
//*************************************************************
void DatapickerCurveWidget::curveErrorsChanged(DatapickerCurve::Errors errors) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbXErrorType->setCurrentIndex((int)errors.x);
	ui.cbYErrorType->setCurrentIndex((int)errors.y);
}

void DatapickerCurveWidget::symbolErrorBarSizeChanged(qreal size) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbErrorBarSize->setValue(Worksheet::convertFromSceneUnits(size, Worksheet::Unit::Point));
}

void DatapickerCurveWidget::symbolErrorBarBrushChanged(const QBrush& brush) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbErrorBarFillingStyle->setCurrentIndex((int)brush.style());
	ui.kcbErrorBarFillingColor->setColor(brush.color());
	GuiTools::updateBrushStyles(ui.cbErrorBarFillingStyle, brush.color());
}

void DatapickerCurveWidget::symbolVisibleChanged(bool on) {
	CONDITIONAL_LOCK_RETURN;
	ui.chkVisible->setChecked(on);
}

//**********************************************************
//******************** SETTINGS ****************************
//**********************************************************
void DatapickerCurveWidget::load() {
	if (!m_curve)
		return;

	CONDITIONAL_LOCK_RETURN;
	ui.cbXErrorType->setCurrentIndex((int)m_curve->curveErrorTypes().x);
	ui.cbYErrorType->setCurrentIndex((int)m_curve->curveErrorTypes().y);
	ui.chkVisible->setChecked(m_curve->pointVisibility());
	ui.cbErrorBarFillingStyle->setCurrentIndex((int)m_curve->pointErrorBarBrush().style());
	ui.kcbErrorBarFillingColor->setColor(m_curve->pointErrorBarBrush().color());
	ui.sbErrorBarSize->setValue(Worksheet::convertFromSceneUnits(m_curve->pointErrorBarSize(), Worksheet::Unit::Point));
}
