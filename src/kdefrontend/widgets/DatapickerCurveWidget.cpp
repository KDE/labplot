/***************************************************************************
    File                 : ImageWidget.cpp
    Project              : LabPlot
    Description          : widget for datapicker properties
    --------------------------------------------------------------------
    Copyright            : (C) 2015 by Ankit Wagadre (wagadre.ankit@gmail.com)
    Copyright            : (C) 2015-2019 Alexander Semke (alexander.semke@web.de)

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

#include "DatapickerCurveWidget.h"
#include "backend/datapicker/DatapickerPoint.h"
#include "backend/worksheet/Worksheet.h"
#include "kdefrontend/GuiTools.h"
#include "kdefrontend/widgets/SymbolWidget.h"

#include <KLocalizedString>

#include <QPainter>

#include <cmath>

DatapickerCurveWidget::DatapickerCurveWidget(QWidget* parent) : BaseDock(parent) {
	ui.setupUi(this);
	m_leName = ui.leName;
	m_teComment = ui.teComment;

	ui.cbXErrorType->addItem(i18n("No Error"));
	ui.cbXErrorType->addItem(i18n("Symmetric"));
	ui.cbXErrorType->addItem(i18n("Asymmetric"));

	ui.cbYErrorType->addItem(i18n("No Error"));
	ui.cbYErrorType->addItem(i18n("Symmetric"));
	ui.cbYErrorType->addItem(i18n("Asymmetric"));

	QString info = i18n("Specify whether the data points have errors and of which type.\n"
						"Note, changing this type is not possible once at least one point was read.");
	ui.lXErrorType->setToolTip(info);
	ui.cbXErrorType->setToolTip(info);
	ui.lYErrorType->setToolTip(info);
	ui.cbYErrorType->setToolTip(info);

	//"Symbol"-tab
	auto* hboxLayout = new QHBoxLayout(ui.tabSymbol);
	symbolWidget = new SymbolWidget(ui.tabSymbol);
	hboxLayout->addWidget(symbolWidget);
	hboxLayout->setContentsMargins(2,2,2,2);
	hboxLayout->setSpacing(2);

	GuiTools::updateBrushStyles(ui.cbErrorBarFillingStyle, Qt::black);
	DatapickerCurveWidget::updateLocale();

	//General
	connect(ui.leName, &QLineEdit::textChanged, this, &DatapickerCurveWidget::nameChanged);
	connect(ui.teComment, &QTextEdit::textChanged, this, &DatapickerCurveWidget::commentChanged);
	connect(ui.cbXErrorType, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			this, &DatapickerCurveWidget::xErrorTypeChanged);
	connect(ui.cbYErrorType, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			this, &DatapickerCurveWidget::yErrorTypeChanged);
	connect(ui.chkVisible, &QCheckBox::clicked, this, &DatapickerCurveWidget::visibilityChanged);

	//error bar
	connect(ui.cbErrorBarFillingStyle, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			this, &DatapickerCurveWidget::errorBarFillingStyleChanged);
	connect(ui.kcbErrorBarFillingColor, &KColorButton::changed, this, &DatapickerCurveWidget::errorBarFillingColorChanged);
	connect(ui.sbErrorBarSize, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
			this, &DatapickerCurveWidget::errorBarSizeChanged);

	hideErrorBarWidgets(true);
}

DatapickerCurveWidget::~DatapickerCurveWidget() = default;

void DatapickerCurveWidget::setCurves(QList<DatapickerCurve*> list) {
	if (list.isEmpty())
		return;

	m_curveList = list;
	m_curve = list.first();
	m_aspect = list.first();

	if (list.size() == 1) {
		ui.lName->setEnabled(true);
		ui.leName->setEnabled(true);
		ui.lComment->setEnabled(true);
		ui.teComment->setEnabled(true);
		ui.leName->setText(m_curve->name());
		ui.teComment->setText(m_curve->comment());
	} else {
		ui.lName->setEnabled(false);
		ui.leName->setEnabled(false);
		ui.lComment->setEnabled(false);
		ui.teComment->setEnabled(false);
		ui.leName->setText(QString());
		ui.teComment->setText(QString());
	}

	load();
	updateSymbolWidgets();

	QList<Symbol*> symbols;
	for (auto* curve : m_curveList)
		symbols << curve->symbol();

	symbolWidget->setSymbols(symbols);

	connect(m_curve, &AbstractAspect::aspectDescriptionChanged,this, &DatapickerCurveWidget::aspectDescriptionChanged);
	connect(m_curve, &AbstractAspect::aspectRemoved,this, &DatapickerCurveWidget::updateSymbolWidgets);
	connect(m_curve, &AbstractAspect::aspectAdded, this, &DatapickerCurveWidget::updateSymbolWidgets);
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
	SET_NUMBER_LOCALE
	ui.sbErrorBarSize->setLocale(numberLocale);
}

//*************************************************************
//**** SLOTs for changes triggered in DatapickerCurveWidget ***
//*************************************************************
//"General"-tab
void DatapickerCurveWidget::xErrorTypeChanged(int index) {
	if ( DatapickerCurve::ErrorType(index) != DatapickerCurve::ErrorType::NoError
		|| m_curve->curveErrorTypes().y != DatapickerCurve::ErrorType::NoError )
		hideErrorBarWidgets(false);
	else
		hideErrorBarWidgets(true);

	if (m_initializing || m_suppressTypeChange)
		return;

	DatapickerCurve::Errors errors = m_curve->curveErrorTypes();
	errors.x = DatapickerCurve::ErrorType(index);

	for (auto* curve : m_curveList)
		curve->setCurveErrorTypes(errors);
}

void DatapickerCurveWidget::yErrorTypeChanged(int index) {
	if ( DatapickerCurve::ErrorType(index) != DatapickerCurve::ErrorType::NoError
		|| m_curve->curveErrorTypes().x != DatapickerCurve::ErrorType::NoError )
		hideErrorBarWidgets(false);
	else
		hideErrorBarWidgets(true);

	if (m_initializing || m_suppressTypeChange)
		return;

	DatapickerCurve::Errors errors = m_curve->curveErrorTypes();
	errors.y = DatapickerCurve::ErrorType(index);

	for (auto* curve : m_curveList)
		curve->setCurveErrorTypes(errors);
}

void DatapickerCurveWidget::errorBarSizeChanged(double value) {
	if (m_initializing)
		return;

	for (auto* curve : m_curveList)
		curve->setPointErrorBarSize( Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point) );
}

void DatapickerCurveWidget::errorBarFillingStyleChanged(int index) {
	auto brushStyle = Qt::BrushStyle(index);
	ui.kcbErrorBarFillingColor->setEnabled(!(brushStyle == Qt::NoBrush));

	if (m_initializing)
		return;

	QBrush brush;
	for (auto* curve : m_curveList) {
		brush = curve->pointErrorBarBrush();
		brush.setStyle(brushStyle);
		curve->setPointErrorBarBrush(brush);
	}
}

void DatapickerCurveWidget::errorBarFillingColorChanged(const QColor& color) {
	if (m_initializing)
		return;

	QBrush brush;
	for (auto* curve : m_curveList) {
		brush = curve->pointErrorBarBrush();
		brush.setColor(color);
		curve->setPointErrorBarBrush(brush);
	}

	m_initializing = true;
	GuiTools::updateBrushStyles(ui.cbErrorBarFillingStyle, color );
	m_initializing = false;
}

void DatapickerCurveWidget::visibilityChanged(bool state) {
	if (m_initializing)
		return;

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
	m_initializing = true;
	ui.cbXErrorType->setCurrentIndex((int) errors.x);
	ui.cbYErrorType->setCurrentIndex((int) errors.y);
	m_initializing = false;
}

void DatapickerCurveWidget::symbolErrorBarSizeChanged(qreal size) {
	m_initializing = true;
	ui.sbErrorBarSize->setValue( Worksheet::convertFromSceneUnits(size, Worksheet::Unit::Point) );
	m_initializing = false;
}

void DatapickerCurveWidget::symbolErrorBarBrushChanged(const QBrush& brush) {
	m_initializing = true;
	ui.cbErrorBarFillingStyle->setCurrentIndex((int) brush.style());
	ui.kcbErrorBarFillingColor->setColor(brush.color());
	GuiTools::updateBrushStyles(ui.cbErrorBarFillingStyle, brush.color());
	m_initializing = false;
}

void DatapickerCurveWidget::symbolVisibleChanged(bool on) {
	m_initializing = true;
	ui.chkVisible->setChecked(on);
	m_initializing = false;
}

//**********************************************************
//******************** SETTINGS ****************************
//**********************************************************
void DatapickerCurveWidget::load() {
	if (!m_curve)
		return;

	m_initializing = true;
	ui.cbXErrorType->setCurrentIndex((int) m_curve->curveErrorTypes().x);
	ui.cbYErrorType->setCurrentIndex((int) m_curve->curveErrorTypes().y);
	ui.chkVisible->setChecked( m_curve->pointVisibility() );
	ui.cbErrorBarFillingStyle->setCurrentIndex( (int) m_curve->pointErrorBarBrush().style() );
	ui.kcbErrorBarFillingColor->setColor(  m_curve->pointErrorBarBrush().color() );
	ui.sbErrorBarSize->setValue( Worksheet::convertFromSceneUnits(m_curve->pointErrorBarSize(), Worksheet::Unit::Point) );
	m_initializing = false;
}
