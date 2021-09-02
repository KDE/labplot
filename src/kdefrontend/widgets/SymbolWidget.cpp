/*
    File                 : SymbolWidget.cc
    Project              : LabPlot
    Description          : label settings widget
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2008-2021 Alexander Semke <alexander.semke@web.de>
    SPDX-FileCopyrightText: 2012-2017 Stefan Gerlach <stefan.gerlach@uni-konstanz.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "SymbolWidget.h"
#include "backend/worksheet/Worksheet.h"
#include "kdefrontend/GuiTools.h"
#include "kdefrontend/dockwidgets/BaseDock.h"

/*!
	\class SymbolWidget
	\brief Widget for editing the properties of a Symbol object, mostly used in an appropriate dock widget.

	In order the properties of the label to be shown, \c loadConfig() has to be called with the corresponding KConfigGroup
	(settings for a label in *Plot, Axis etc. or for an independent label on the worksheet).

	\ingroup kdefrontend
 */
SymbolWidget::SymbolWidget(QWidget* parent) : QWidget(parent) {
	ui.setupUi(this);

	GuiTools::updatePenStyles(ui.cbBorderStyle, Qt::black);
	GuiTools::addSymbolStyles(ui.cbStyle);
	GuiTools::updateBrushStyles(ui.cbFillingStyle, Qt::black);

	connect( ui.cbStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SymbolWidget::styleChanged);
	connect( ui.sbSize, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &SymbolWidget::sizeChanged);
	connect( ui.sbRotation, QOverload<int>::of(&QSpinBox::valueChanged), this, &SymbolWidget::rotationChanged);
	connect( ui.sbOpacity, QOverload<int>::of(&QSpinBox::valueChanged), this, &SymbolWidget::opacityChanged);

	connect( ui.cbFillingStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SymbolWidget::fillingStyleChanged);
	connect( ui.kcbFillingColor, &KColorButton::changed, this, &SymbolWidget::fillingColorChanged);

	connect( ui.cbBorderStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SymbolWidget::borderStyleChanged);
	connect( ui.kcbBorderColor, &KColorButton::changed, this, &SymbolWidget::borderColorChanged);
	connect( ui.sbBorderWidth, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &SymbolWidget::borderWidthChanged);
}

void SymbolWidget::setSymbols(QList<Symbol*> symbols) {
	m_symbols = symbols;
	m_symbol = m_symbols.first();

	load();

	//Symbol-Tab
	connect(m_symbol, &Symbol::styleChanged, this, &SymbolWidget::symbolStyleChanged);
	connect(m_symbol, &Symbol::sizeChanged, this, &SymbolWidget::symbolSizeChanged);
	connect(m_symbol, &Symbol::rotationAngleChanged, this, &SymbolWidget::symbolRotationAngleChanged);
	connect(m_symbol, &Symbol::opacityChanged, this, &SymbolWidget::symbolOpacityChanged);
	connect(m_symbol, &Symbol::brushChanged, this, &SymbolWidget::symbolBrushChanged);
	connect(m_symbol, &Symbol::penChanged, this, &SymbolWidget::symbolPenChanged);
}

/*
 * updates the locale in the widgets. called when the application settins are changed.
 */
void SymbolWidget::updateLocale() {
	SET_NUMBER_LOCALE
	ui.sbSize->setLocale(numberLocale);
	ui.sbBorderWidth->setLocale(numberLocale);
}

//*************************************************************
//******** SLOTs for changes triggered in SymbolWidget ********
//*************************************************************
void SymbolWidget::styleChanged(int index) {
	auto style = static_cast<Symbol::Style>(ui.cbStyle->itemData(index).toInt());

	if (style == Symbol::Style::NoSymbols) {
		ui.sbSize->setEnabled(false);
		ui.sbRotation->setEnabled(false);
		ui.sbOpacity->setEnabled(false);

		ui.kcbFillingColor->setEnabled(false);
		ui.cbFillingStyle->setEnabled(false);

		ui.cbBorderStyle->setEnabled(false);
		ui.kcbBorderColor->setEnabled(false);
		ui.sbBorderWidth->setEnabled(false);
	} else {
		ui.sbSize->setEnabled(true);
		ui.sbRotation->setEnabled(true);
		ui.sbOpacity->setEnabled(true);

		//enable/disable the symbol filling options in the GUI depending on the currently selected symbol.
		if (style != Symbol::Style::Line && style != Symbol::Style::Cross) {
			ui.cbFillingStyle->setEnabled(true);
			bool noBrush = (Qt::BrushStyle(ui.cbFillingStyle->currentIndex()) == Qt::NoBrush);
			ui.kcbFillingColor->setEnabled(!noBrush);
		} else {
			ui.kcbFillingColor->setEnabled(false);
			ui.cbFillingStyle->setEnabled(false);
		}

		ui.cbBorderStyle->setEnabled(true);
		bool noLine = (Qt::PenStyle(ui.cbBorderStyle->currentIndex()) == Qt::NoPen);
		ui.kcbBorderColor->setEnabled(!noLine);
		ui.sbBorderWidth->setEnabled(!noLine);
	}

	if (m_initializing)
		return;

	for (auto* symbol : m_symbols)
		symbol->setStyle(style);
}

void SymbolWidget::sizeChanged(double value) {
	if (m_initializing)
		return;

	for (auto* symbol : m_symbols)
		symbol->setSize( Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point) );
}

void SymbolWidget::rotationChanged(int value) {
	if (m_initializing)
		return;

	for (auto* symbol : m_symbols)
		symbol->setRotationAngle(value);
}

void SymbolWidget::opacityChanged(int value) {
	if (m_initializing)
		return;

	qreal opacity = (float)value/100.;
	for (auto* symbol : m_symbols)
		symbol->setOpacity(opacity);
}

void SymbolWidget::fillingStyleChanged(int index) {
	if (index == -1)
		return;

	const auto brushStyle = Qt::BrushStyle(index);
	ui.kcbFillingColor->setEnabled(!(brushStyle == Qt::NoBrush));

	if (m_initializing)
		return;

	QBrush brush;
	for (auto* symbol : m_symbols) {
		brush = symbol->brush();
		brush.setStyle(brushStyle);
		symbol->setBrush(brush);
	}
}

void SymbolWidget::fillingColorChanged(const QColor& color) {
	if (m_initializing)
		return;

	QBrush brush;
	for (auto* symbol : m_symbols) {
		brush = symbol->brush();
		brush.setColor(color);
		symbol->setBrush(brush);
	}

	m_initializing = true;
	GuiTools::updateBrushStyles(ui.cbFillingStyle, color );
	m_initializing = false;
}

void SymbolWidget::borderStyleChanged(int index) {
	if (index == -1)
		return;

	const auto penStyle = Qt::PenStyle(index);
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
	for (auto* symbol : m_symbols) {
		pen = symbol->pen();
		pen.setStyle(penStyle);
		symbol->setPen(pen);
	}
}

void SymbolWidget::borderColorChanged(const QColor& color) {
	if (m_initializing)
		return;

	QPen pen;
	for (auto* symbol : m_symbols) {
		pen = symbol->pen();
		pen.setColor(color);
		symbol->setPen(pen);
	}

	m_initializing = true;
	GuiTools::updatePenStyles(ui.cbBorderStyle, color);
	m_initializing = false;
}

void SymbolWidget::borderWidthChanged(double value) {
	if (m_initializing)
		return;

	QPen pen;
	for (auto* symbol : m_symbols) {
		pen = symbol->pen();
		pen.setWidthF( Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point) );
		symbol->setPen(pen);
	}
}

//*************************************************************
//*********** SLOTs for changes triggered in Symbol ***********
//*************************************************************
void SymbolWidget::symbolStyleChanged(Symbol::Style style) {
	const Lock lock(m_initializing);
	int index = ui.cbStyle->findData((int)style);
	ui.cbStyle->setCurrentIndex(index);
}

void SymbolWidget::symbolSizeChanged(qreal size) {
	const Lock lock(m_initializing);
	ui.sbSize->setValue( Worksheet::convertFromSceneUnits(size, Worksheet::Unit::Point) );
}

void SymbolWidget::symbolRotationAngleChanged(qreal angle) {
	const Lock lock(m_initializing);
	ui.sbRotation->setValue(angle);
}

void SymbolWidget::symbolOpacityChanged(qreal opacity) {
	const Lock lock(m_initializing);
	ui.sbOpacity->setValue( round(opacity*100.0) );
}

void SymbolWidget::symbolBrushChanged(const QBrush& brush) {
	const Lock lock(m_initializing);
	ui.cbFillingStyle->setCurrentIndex((int) brush.style());
	ui.kcbFillingColor->setColor(brush.color());
	GuiTools::updateBrushStyles(ui.cbFillingStyle, brush.color());
}

void SymbolWidget::symbolPenChanged(const QPen& pen) {
	const Lock lock(m_initializing);
	ui.cbBorderStyle->setCurrentIndex( (int) pen.style());
	ui.kcbBorderColor->setColor( pen.color());
	GuiTools::updatePenStyles(ui.cbBorderStyle, pen.color());
	ui.sbBorderWidth->setValue( Worksheet::convertFromSceneUnits(pen.widthF(), Worksheet::Unit::Point));
}

//**********************************************************
//******************** SETTINGS ****************************
//**********************************************************
void SymbolWidget::load() {
	const Lock lock(m_initializing);

	int index = ui.cbStyle->findData((int)m_symbol->style());
	ui.cbStyle->setCurrentIndex(index);
	ui.sbSize->setValue( Worksheet::convertFromSceneUnits(m_symbol->size(), Worksheet::Unit::Point) );
	ui.sbRotation->setValue( m_symbol->rotationAngle() );
	ui.sbOpacity->setValue( round(m_symbol->opacity()*100.0) );
	ui.cbFillingStyle->setCurrentIndex( (int) m_symbol->brush().style() );
	ui.kcbFillingColor->setColor(  m_symbol->brush().color() );
	ui.cbBorderStyle->setCurrentIndex( (int) m_symbol->pen().style() );
	ui.kcbBorderColor->setColor( m_symbol->pen().color() );
	ui.sbBorderWidth->setValue( Worksheet::convertFromSceneUnits(m_symbol->pen().widthF(), Worksheet::Unit::Point) );

	GuiTools::updateBrushStyles(ui.cbFillingStyle, ui.kcbFillingColor->color());
	GuiTools::updatePenStyles(ui.cbBorderStyle, ui.kcbBorderColor->color());
	styleChanged(ui.cbStyle->currentIndex());
}

void SymbolWidget::loadConfig(const KConfigGroup& group) {
	const Lock lock(m_initializing);

	int index = ui.cbStyle->findData((int)m_symbol->style());
	ui.cbStyle->setCurrentIndex(group.readEntry("SymbolStyle", index));
	ui.sbSize->setValue( Worksheet::convertFromSceneUnits(group.readEntry("SymbolSize", m_symbol->size()), Worksheet::Unit::Point) );
	ui.sbRotation->setValue( group.readEntry("SymbolRotation", m_symbol->rotationAngle()) );
	ui.sbOpacity->setValue( round(group.readEntry("SymbolOpacity", m_symbol->opacity())*100.0) );
	ui.cbFillingStyle->setCurrentIndex( group.readEntry("SymbolFillingStyle", (int) m_symbol->brush().style()) );
	ui.kcbFillingColor->setColor(  group.readEntry("SymbolFillingColor", m_symbol->brush().color()) );
	ui.cbBorderStyle->setCurrentIndex( group.readEntry("SymbolBorderStyle", (int) m_symbol->pen().style()) );
	ui.kcbBorderColor->setColor( group.readEntry("SymbolBorderColor", m_symbol->pen().color()) );
	ui.sbBorderWidth->setValue( Worksheet::convertFromSceneUnits(group.readEntry("SymbolBorderWidth",m_symbol->pen().widthF()), Worksheet::Unit::Point) );

	GuiTools::updateBrushStyles(ui.cbFillingStyle, ui.kcbFillingColor->color());
	GuiTools::updatePenStyles(ui.cbBorderStyle, ui.kcbBorderColor->color());
	styleChanged(ui.cbStyle->currentIndex());
}

void SymbolWidget::saveConfig(KConfigGroup& group) const {
	group.writeEntry("SymbolStyle", ui.cbStyle->itemData(ui.cbStyle->currentIndex()));
	group.writeEntry("SymbolSize", Worksheet::convertToSceneUnits(ui.sbSize->value(),Worksheet::Unit::Point));
	group.writeEntry("SymbolRotation", ui.sbRotation->value());
	group.writeEntry("SymbolOpacity", ui.sbOpacity->value()/100.0);
	group.writeEntry("SymbolFillingStyle", ui.cbFillingStyle->currentIndex());
	group.writeEntry("SymbolFillingColor", ui.kcbFillingColor->color());
	group.writeEntry("SymbolBorderStyle", ui.cbBorderStyle->currentIndex());
	group.writeEntry("SymbolBorderColor", ui.kcbBorderColor->color());
	group.writeEntry("SymbolBorderWidth", Worksheet::convertToSceneUnits(ui.sbBorderWidth->value(), Worksheet::Unit::Point));
}
