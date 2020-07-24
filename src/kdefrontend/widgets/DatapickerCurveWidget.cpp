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

#include <KLocalizedString>
#include <KConfigGroup>
#include <KSharedConfig>

#include <QPainter>

#include <cmath>

DatapickerCurveWidget::DatapickerCurveWidget(QWidget* parent) : BaseDock(parent) {
	ui.setupUi(this);
	m_leName = ui.leName;
	m_leComment = ui.leComment;

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

	updateLocale();

	connect(ui.leName, &QLineEdit::textChanged, this, &DatapickerCurveWidget::nameChanged);
	connect(ui.leComment, &QLineEdit::textChanged, this, &DatapickerCurveWidget::commentChanged);
	connect(ui.cbXErrorType, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			this, &DatapickerCurveWidget::xErrorTypeChanged);
	connect(ui.cbYErrorType, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			this, &DatapickerCurveWidget::yErrorTypeChanged);

	//symbol
	connect(ui.cbStyle, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			this, &DatapickerCurveWidget::styleChanged);
	connect(ui.sbSize, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
			this, &DatapickerCurveWidget::sizeChanged);
	connect(ui.sbRotation, static_cast<void (QSpinBox::*) (int)>(&QSpinBox::valueChanged),
			this, &DatapickerCurveWidget::rotationChanged);
	connect(ui.sbOpacity, static_cast<void (QSpinBox::*) (int)>(&QSpinBox::valueChanged),
			this, &DatapickerCurveWidget::opacityChanged);

	//Filling
	connect(ui.cbFillingStyle, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			this, &DatapickerCurveWidget::fillingStyleChanged);
	connect(ui.kcbFillingColor, &KColorButton::changed, this, &DatapickerCurveWidget::fillingColorChanged);

	//border
	connect(ui.cbBorderStyle, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			this, &DatapickerCurveWidget::borderStyleChanged);
	connect(ui.kcbBorderColor, &KColorButton::changed, this, &DatapickerCurveWidget::borderColorChanged);
	connect(ui.sbBorderWidth, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
			this, &DatapickerCurveWidget::borderWidthChanged);

	connect(ui.chbVisible, &QCheckBox::clicked, this, &DatapickerCurveWidget::visibilityChanged);

	//error bar
	connect(ui.cbErrorBarFillingStyle, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			this, &DatapickerCurveWidget::errorBarFillingStyleChanged);
	connect(ui.kcbErrorBarFillingColor, &KColorButton::changed, this, &DatapickerCurveWidget::errorBarFillingColorChanged);
	connect(ui.sbErrorBarSize, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
			this, &DatapickerCurveWidget::errorBarSizeChanged);

	init();
	hideErrorBarWidgets(true);
}

DatapickerCurveWidget::~DatapickerCurveWidget() = default;

void DatapickerCurveWidget::init() {
	m_initializing = true;
	GuiTools::updatePenStyles(ui.cbBorderStyle, Qt::black);

	QPainter pa;
	int iconSize = 20;
	QPixmap pm(iconSize, iconSize);
	QPen pen(Qt::SolidPattern, 0);
	ui.cbStyle->setIconSize(QSize(iconSize, iconSize));
	QTransform trafo;
	trafo.scale(15, 15);
	//TODO: constant?
	for (int i = 1; i < 19; ++i) {
		auto style = (Symbol::Style)i;
		pm.fill(Qt::transparent);
		pa.begin(&pm);
		pa.setPen(pen);
		pa.setRenderHint(QPainter::Antialiasing);
		pa.translate(iconSize/2,iconSize/2);
		pa.drawPath(trafo.map(Symbol::pathFromStyle(style)));
		pa.end();
		ui.cbStyle->addItem(QIcon(pm), Symbol::nameFromStyle(style));
	}
	GuiTools::updateBrushStyles(ui.cbFillingStyle, Qt::black);
	GuiTools::updateBrushStyles(ui.cbErrorBarFillingStyle, Qt::black);
	m_initializing = false;
}

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
		ui.leComment->setEnabled(true);
		ui.leName->setText(m_curve->name());
		ui.leComment->setText(m_curve->comment());
	} else {
		ui.lName->setEnabled(false);
		ui.leName->setEnabled(false);
		ui.lComment->setEnabled(false);
		ui.leComment->setEnabled(false);
		ui.leName->setText(QString());
		ui.leComment->setText(QString());
	}

	load();
	updateSymbolWidgets();

	connect(m_curve, &AbstractAspect::aspectDescriptionChanged,this, &DatapickerCurveWidget::curveDescriptionChanged);
	connect(m_curve, &AbstractAspect::aspectRemoved,this, &DatapickerCurveWidget::updateSymbolWidgets);
	connect(m_curve, &AbstractAspect::aspectAdded, this, &DatapickerCurveWidget::updateSymbolWidgets);
	connect(m_curve, &DatapickerCurve::curveErrorTypesChanged, this, &DatapickerCurveWidget::curveErrorsChanged);
	connect(m_curve, &DatapickerCurve::pointStyleChanged, this, &DatapickerCurveWidget::symbolStyleChanged);
	connect(m_curve, &DatapickerCurve::pointSizeChanged, this, &DatapickerCurveWidget::symbolSizeChanged);
	connect(m_curve, &DatapickerCurve::pointRotationAngleChanged, this, &DatapickerCurveWidget::symbolRotationAngleChanged);
	connect(m_curve, &DatapickerCurve::pointOpacityChanged, this, &DatapickerCurveWidget::symbolOpacityChanged);
	connect(m_curve, &DatapickerCurve::pointBrushChanged, this, &DatapickerCurveWidget::symbolBrushChanged);
	connect(m_curve, &DatapickerCurve::pointPenChanged, this, &DatapickerCurveWidget::symbolPenChanged);
	connect(m_curve, &DatapickerCurve::pointVisibilityChanged, this, &DatapickerCurveWidget::symbolVisibleChanged);
	connect(m_curve, &DatapickerCurve::pointErrorBarBrushChanged, this, &DatapickerCurveWidget::symbolErrorBarBrushChanged);
	connect(m_curve, &DatapickerCurve::pointErrorBarSizeChanged, this, &DatapickerCurveWidget::symbolErrorBarSizeChanged);
}

void DatapickerCurveWidget::hideErrorBarWidgets(bool on) {
	ui.lErrorBar->setVisible(!on);
	ui.cbErrorBarFillingStyle->setVisible(!on);
	ui.kcbErrorBarFillingColor->setVisible(!on);
	ui.lErrorBarFillingColor->setVisible(!on);
	ui.lErrorBarFillingStyle->setVisible(!on);
	ui.sbErrorBarSize->setVisible(!on);
	ui.lErrorBarSize->setVisible(!on);
}

void DatapickerCurveWidget::updateLocale() {
	SET_NUMBER_LOCALE
	ui.sbSize->setLocale(numberLocale);
	ui.sbBorderWidth->setLocale(numberLocale);
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

void DatapickerCurveWidget::styleChanged(int index) {
	auto style = Symbol::Style(index + 1);
	//enable/disable the  filling options in the GUI depending on the currently selected points.
	if (style != Symbol::Style::Line && style != Symbol::Style::Cross) {
 		ui.cbFillingStyle->setEnabled(true);
		bool noBrush = (Qt::BrushStyle(ui.cbFillingStyle->currentIndex()) == Qt::NoBrush);
		ui.kcbFillingColor->setEnabled(!noBrush);
	} else {
		ui.kcbFillingColor->setEnabled(false);
		ui.cbFillingStyle->setEnabled(false);
	}

	bool noLine = (Qt::PenStyle(ui.cbBorderStyle->currentIndex()) == Qt::NoPen);
	ui.kcbBorderColor->setEnabled(!noLine);
	ui.sbBorderWidth->setEnabled(!noLine);

	if (m_initializing)
		return;

	for (auto* curve : m_curveList)
		curve->setPointStyle(style);
}

void DatapickerCurveWidget::sizeChanged(double value) {
	if (m_initializing)
		return;

	for (auto* curve : m_curveList)
		curve->setPointSize( Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point) );
}

void DatapickerCurveWidget::rotationChanged(int value) {
	if (m_initializing)
		return;

	for (auto* curve : m_curveList)
		curve->setPointRotationAngle(value);
}

void DatapickerCurveWidget::opacityChanged(int value) {
	if (m_initializing)
		return;

	qreal opacity = (float)value/100.;
	for (auto* curve : m_curveList)
		curve->setPointOpacity(opacity);
}

void DatapickerCurveWidget::errorBarSizeChanged(double value) {
	if (m_initializing)
		return;

	for (auto* curve : m_curveList)
		curve->setPointErrorBarSize( Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point) );
}

void DatapickerCurveWidget::fillingStyleChanged(int index) {
	auto brushStyle = Qt::BrushStyle(index);
	ui.kcbFillingColor->setEnabled(!(brushStyle == Qt::NoBrush));

	if (m_initializing)
		return;

	QBrush brush;
	for (auto* curve : m_curveList) {
		brush = curve->pointBrush();
		brush.setStyle(brushStyle);
		curve->setPointBrush(brush);
	}
}

void DatapickerCurveWidget::errorBarFillingStyleChanged(int index) {
	auto brushStyle = Qt::BrushStyle(index);
	ui.kcbErrorBarFillingColor->setEnabled(!(brushStyle == Qt::NoBrush));

	if (m_initializing)
		return;

	QBrush brush;
	for (auto* curve : m_curveList) {
		brush = curve->pointBrush();
		brush.setStyle(brushStyle);
		curve->setPointErrorBarBrush(brush);
	}
}

void DatapickerCurveWidget::fillingColorChanged(const QColor& color) {
	if (m_initializing)
		return;

	QBrush brush;
	for (auto* curve : m_curveList) {
		brush = curve->pointBrush();
		brush.setColor(color);
		curve->setPointBrush(brush);
	}

	m_initializing = true;
	GuiTools::updateBrushStyles(ui.cbFillingStyle, color );
	m_initializing = false;
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

void DatapickerCurveWidget::borderStyleChanged(int index) {
	auto penStyle = Qt::PenStyle(index);

	if ( penStyle == Qt::NoPen ) {
		ui.kcbBorderColor->setEnabled(false);
		ui.sbBorderWidth->setEnabled(false);
	} else {
		ui.kcbBorderColor->setEnabled(true);
		ui.sbBorderWidth->setEnabled(true);
	}

	if (m_initializing)
		return;

	QPen pen;
	for (auto* curve : m_curveList) {
		pen = curve->pointPen();
		pen.setStyle(penStyle);
		curve->setPointPen(pen);
	}
}

void DatapickerCurveWidget::borderColorChanged(const QColor& color) {
	if (m_initializing)
		return;

	QPen pen;
	for (auto* curve : m_curveList) {
		pen = curve->pointPen();
		pen.setColor(color);
		curve->setPointPen(pen);
	}

	m_initializing = true;
	GuiTools::updatePenStyles(ui.cbBorderStyle, color);
	m_initializing = false;
}

void DatapickerCurveWidget::borderWidthChanged(double value) {
	if (m_initializing)
		return;

	QPen pen;
	for (auto* curve : m_curveList) {
		pen = curve->pointPen();
		pen.setWidthF( Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point) );
		curve->setPointPen(pen);
	}
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
void DatapickerCurveWidget::curveDescriptionChanged(const AbstractAspect* aspect) {
	if (m_curve != aspect)
		return;

	m_initializing = true;
	if (aspect->name() != ui.leName->text())
		ui.leName->setText(aspect->name());
	else if (aspect->comment() != ui.leComment->text())
		ui.leComment->setText(aspect->comment());

	m_initializing = false;
}

void DatapickerCurveWidget::curveErrorsChanged(DatapickerCurve::Errors errors) {
	m_initializing = true;
	ui.cbXErrorType->setCurrentIndex((int) errors.x);
	ui.cbYErrorType->setCurrentIndex((int) errors.y);
	m_initializing = false;
}

void DatapickerCurveWidget::symbolStyleChanged(Symbol::Style style) {
	m_initializing = true;
	ui.cbStyle->setCurrentIndex((int)style - 1);
	m_initializing = false;
}

void DatapickerCurveWidget::symbolSizeChanged(qreal size) {
	m_initializing = true;
	ui.sbSize->setValue( Worksheet::convertFromSceneUnits(size, Worksheet::Unit::Point) );
	m_initializing = false;
}

void DatapickerCurveWidget::symbolErrorBarSizeChanged(qreal size) {
	m_initializing = true;
	ui.sbErrorBarSize->setValue( Worksheet::convertFromSceneUnits(size, Worksheet::Unit::Point) );
	m_initializing = false;
}

void DatapickerCurveWidget::symbolRotationAngleChanged(qreal angle) {
	m_initializing = true;
	ui.sbRotation->setValue(round(angle));
	m_initializing = false;
}

void DatapickerCurveWidget::symbolOpacityChanged(qreal opacity) {
	m_initializing = true;
	ui.sbOpacity->setValue( round(opacity*100.0) );
	m_initializing = false;
}

void DatapickerCurveWidget::symbolBrushChanged(const QBrush& brush) {
	m_initializing = true;
	ui.cbFillingStyle->setCurrentIndex((int) brush.style());
	ui.kcbFillingColor->setColor(brush.color());
	GuiTools::updateBrushStyles(ui.cbFillingStyle, brush.color());
	m_initializing = false;
}

void DatapickerCurveWidget::symbolErrorBarBrushChanged(const QBrush& brush) {
	m_initializing = true;
	ui.cbErrorBarFillingStyle->setCurrentIndex((int) brush.style());
	ui.kcbErrorBarFillingColor->setColor(brush.color());
	GuiTools::updateBrushStyles(ui.cbErrorBarFillingStyle, brush.color());
	m_initializing = false;
}

void DatapickerCurveWidget::symbolPenChanged(const QPen& pen) {
	m_initializing = true;
	ui.cbBorderStyle->setCurrentIndex( (int) pen.style());
	ui.kcbBorderColor->setColor( pen.color());
	GuiTools::updatePenStyles(ui.cbBorderStyle, pen.color());
	ui.sbBorderWidth->setValue( Worksheet::convertFromSceneUnits(pen.widthF(), Worksheet::Unit::Point));
	m_initializing = false;
}

void DatapickerCurveWidget::symbolVisibleChanged(bool on) {
	m_initializing = true;
	ui.chbVisible->setChecked(on);
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
	ui.cbStyle->setCurrentIndex( (int)m_curve->pointStyle() - 1 );
	ui.sbSize->setValue( Worksheet::convertFromSceneUnits(m_curve->pointSize(), Worksheet::Unit::Point) );
	ui.sbRotation->setValue( m_curve->pointRotationAngle() );
	ui.sbOpacity->setValue( round(m_curve->pointOpacity()*100.0) );
	ui.cbFillingStyle->setCurrentIndex( (int) m_curve->pointBrush().style() );
	ui.kcbFillingColor->setColor(  m_curve->pointBrush().color() );
	ui.cbBorderStyle->setCurrentIndex( (int) m_curve->pointPen().style() );
	ui.kcbBorderColor->setColor( m_curve->pointPen().color() );
	ui.sbBorderWidth->setValue( Worksheet::convertFromSceneUnits(m_curve->pointPen().widthF(), Worksheet::Unit::Point) );
	ui.chbVisible->setChecked( m_curve->pointVisibility() );
	ui.cbErrorBarFillingStyle->setCurrentIndex( (int) m_curve->pointErrorBarBrush().style() );
	ui.kcbErrorBarFillingColor->setColor(  m_curve->pointErrorBarBrush().color() );
	ui.sbErrorBarSize->setValue( Worksheet::convertFromSceneUnits(m_curve->pointErrorBarSize(), Worksheet::Unit::Point) );
	m_initializing = false;
}
