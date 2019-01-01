/***************************************************************************
    File                 : ImageWidget.cpp
    Project              : LabPlot
    Description          : widget for datapicker properties
    --------------------------------------------------------------------
    Copyright            : (C) 2015 by Ankit Wagadre (wagadre.ankit@gmail.com)
    Copyright            : (C) 2015 Alexander Semke (alexander.semke@web.de)

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

#include <QPainter>

#include <KLocalizedString>

#include <cmath>

DatapickerCurveWidget::DatapickerCurveWidget(QWidget* parent) : QWidget(parent) {
	ui.setupUi(this);

	ui.cbXErrorType->addItem(i18n("No Error"));
	ui.cbXErrorType->addItem(i18n("Symmetric"));
	ui.cbXErrorType->addItem(i18n("Asymmetric"));

	ui.cbYErrorType->addItem(i18n("No Error"));
	ui.cbYErrorType->addItem(i18n("Symmetric"));
	ui.cbYErrorType->addItem(i18n("Asymmetric"));

	connect(ui.leName, &QLineEdit::textChanged, this, &DatapickerCurveWidget::nameChanged);
	connect(ui.leComment, &QLineEdit::textChanged, this, &DatapickerCurveWidget::commentChanged);
	connect( ui.cbXErrorType, SIGNAL(currentIndexChanged(int)), this, SLOT(xErrorTypeChanged(int)) );
	connect( ui.cbYErrorType, SIGNAL(currentIndexChanged(int)), this, SLOT(yErrorTypeChanged(int)) );

	//symbol
	connect( ui.cbStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(styleChanged(int)) );
	connect( ui.sbSize, SIGNAL(valueChanged(double)), this, SLOT(sizeChanged(double)) );
	connect( ui.sbRotation, SIGNAL(valueChanged(int)), this, SLOT(rotationChanged(int)) );
	connect( ui.sbOpacity, SIGNAL(valueChanged(int)), this, SLOT(opacityChanged(int)) );

	//Filling
	connect( ui.cbFillingStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(fillingStyleChanged(int)) );
	connect( ui.kcbFillingColor, SIGNAL(changed(QColor)), this, SLOT(fillingColorChanged(QColor)) );

	//border
	connect( ui.cbBorderStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(borderStyleChanged(int)) );
	connect( ui.kcbBorderColor, SIGNAL(changed(QColor)), this, SLOT(borderColorChanged(QColor)) );
	connect( ui.sbBorderWidth, SIGNAL(valueChanged(double)), this, SLOT(borderWidthChanged(double)) );

	connect( ui.chbVisible, SIGNAL(clicked(bool)), this, SLOT(visibilityChanged(bool)) );

	//error bar
	connect( ui.cbErrorBarFillingStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(errorBarFillingStyleChanged(int)) );
	connect( ui.kcbErrorBarFillingColor, SIGNAL(changed(QColor)), this, SLOT(errorBarFillingColorChanged(QColor)) );
	connect( ui.sbErrorBarSize, SIGNAL(valueChanged(double)), this, SLOT(errorBarSizeChanged(double)) );

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
		ui.leName->setText("");
		ui.leComment->setText("");
	}

	load();
	initConnections();
	updateSymbolWidgets();
}

void DatapickerCurveWidget::initConnections() {
	connect( m_curve, SIGNAL(aspectDescriptionChanged(const AbstractAspect*)),this, SLOT(curveDescriptionChanged(const AbstractAspect*)));
	connect( m_curve, SIGNAL(aspectRemoved(const AbstractAspect*,const AbstractAspect*,const AbstractAspect*)),
		 this, SLOT(updateSymbolWidgets()) );
	connect( m_curve, SIGNAL(aspectAdded(const AbstractAspect*)), this, SLOT(updateSymbolWidgets()) );
	connect( m_curve, SIGNAL(curveErrorTypesChanged(DatapickerCurve::Errors)), this, SLOT(curveErrorsChanged(DatapickerCurve::Errors)) );
	connect( m_curve, SIGNAL(pointStyleChanged(Symbol::Style)), this, SLOT(symbolStyleChanged(Symbol::Style)));
	connect( m_curve, SIGNAL(pointSizeChanged(qreal)), this, SLOT(symbolSizeChanged(qreal)));
	connect( m_curve, SIGNAL(pointRotationAngleChanged(qreal)), this, SLOT(symbolRotationAngleChanged(qreal)));
	connect( m_curve, SIGNAL(pointOpacityChanged(qreal)), this, SLOT(symbolOpacityChanged(qreal)));
	connect( m_curve, SIGNAL(pointBrushChanged(QBrush)), this, SLOT(symbolBrushChanged(QBrush)) );
	connect( m_curve, SIGNAL(pointPenChanged(QPen)), this, SLOT(symbolPenChanged(QPen)) );
	connect( m_curve, SIGNAL(pointVisibilityChanged(bool)), this, SLOT(symbolVisibleChanged(bool)) );
	connect( m_curve, SIGNAL(pointErrorBarBrushChanged(QBrush)), this, SLOT(symbolErrorBarBrushChanged(QBrush)) );
	connect( m_curve, SIGNAL(pointErrorBarSizeChanged(qreal)), this, SLOT(symbolErrorBarSizeChanged(qreal)) );

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

//*************************************************************
//**** SLOTs for changes triggered in DatapickerCurveWidget ***
//*************************************************************
//"General"-tab
void DatapickerCurveWidget::nameChanged() {
	if (m_initializing)
		return;

	m_curve->setName(ui.leName->text());
}

void DatapickerCurveWidget::commentChanged() {
	if (m_initializing)
		return;

	m_curve->setComment(ui.leComment->text());
}

void DatapickerCurveWidget::xErrorTypeChanged(int index) {
	if ( DatapickerCurve::ErrorType(index) != DatapickerCurve::NoError
		|| m_curve->curveErrorTypes().y != DatapickerCurve::NoError )
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
	if ( DatapickerCurve::ErrorType(index) != DatapickerCurve::NoError
		|| m_curve->curveErrorTypes().x != DatapickerCurve::NoError )
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
	if (style != Symbol::Line && style != Symbol::Cross) {
 		ui.cbFillingStyle->setEnabled(true);
		bool noBrush = (Qt::BrushStyle(ui.cbFillingStyle->currentIndex())==Qt::NoBrush);
		ui.kcbFillingColor->setEnabled(!noBrush);
	} else {
		ui.kcbFillingColor->setEnabled(false);
		ui.cbFillingStyle->setEnabled(false);
	}

	bool noLine = (Qt::PenStyle(ui.cbBorderStyle->currentIndex())== Qt::NoPen);
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
		curve->setPointSize( Worksheet::convertToSceneUnits(value, Worksheet::Point) );
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
		curve->setPointErrorBarSize( Worksheet::convertToSceneUnits(value, Worksheet::Point) );
}

void DatapickerCurveWidget::fillingStyleChanged(int index) {
	auto brushStyle = Qt::BrushStyle(index);
	ui.kcbFillingColor->setEnabled(!(brushStyle==Qt::NoBrush));

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
	ui.kcbErrorBarFillingColor->setEnabled(!(brushStyle==Qt::NoBrush));

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
		pen.setWidthF( Worksheet::convertToSceneUnits(value, Worksheet::Point) );
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
	const QVector<DatapickerPoint*> pointsList = m_curve->children<DatapickerPoint>(AbstractAspect::IncludeHidden);
	if (pointsList.isEmpty()) {
		ui.cbXErrorType->setEnabled(true);
		ui.cbYErrorType->setEnabled(true);
		ui.tSymbols->setEnabled(false);
		m_suppressTypeChange = false;
	} else {
		ui.cbXErrorType->setEnabled(false);
		ui.cbYErrorType->setEnabled(false);
		ui.tSymbols->setEnabled(true);
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
	if (aspect->name() != ui.leName->text()) {
		ui.leName->setText(aspect->name());
	} else if (aspect->comment() != ui.leComment->text()) {
		ui.leComment->setText(aspect->comment());
	}
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
	ui.sbSize->setValue( Worksheet::convertFromSceneUnits(size, Worksheet::Point) );
	m_initializing = false;
}

void DatapickerCurveWidget::symbolErrorBarSizeChanged(qreal size) {
	m_initializing = true;
	ui.sbErrorBarSize->setValue( Worksheet::convertFromSceneUnits(size, Worksheet::Point) );
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
	ui.sbBorderWidth->setValue( Worksheet::convertFromSceneUnits(pen.widthF(), Worksheet::Point));
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
	if(!m_curve)
		return;

	m_initializing = true;
	ui.cbXErrorType->setCurrentIndex((int) m_curve->curveErrorTypes().x);
	ui.cbYErrorType->setCurrentIndex((int) m_curve->curveErrorTypes().y);
	ui.cbStyle->setCurrentIndex( (int)m_curve->pointStyle() - 1 );
	ui.sbSize->setValue( Worksheet::convertFromSceneUnits(m_curve->pointSize(), Worksheet::Point) );
	ui.sbRotation->setValue( m_curve->pointRotationAngle() );
	ui.sbOpacity->setValue( round(m_curve->pointOpacity()*100.0) );
	ui.cbFillingStyle->setCurrentIndex( (int) m_curve->pointBrush().style() );
	ui.kcbFillingColor->setColor(  m_curve->pointBrush().color() );
	ui.cbBorderStyle->setCurrentIndex( (int) m_curve->pointPen().style() );
	ui.kcbBorderColor->setColor( m_curve->pointPen().color() );
	ui.sbBorderWidth->setValue( Worksheet::convertFromSceneUnits(m_curve->pointPen().widthF(), Worksheet::Point) );
	ui.chbVisible->setChecked( m_curve->pointVisibility() );
	ui.cbErrorBarFillingStyle->setCurrentIndex( (int) m_curve->pointErrorBarBrush().style() );
	ui.kcbErrorBarFillingColor->setColor(  m_curve->pointErrorBarBrush().color() );
	ui.sbErrorBarSize->setValue( Worksheet::convertFromSceneUnits(m_curve->pointErrorBarSize(), Worksheet::Point) );
	m_initializing = false;
}
