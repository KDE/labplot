/*
    File                 : Symbol.cpp
    Project              : LabPlot
    Description          : Symbol
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2015-2021 Alexander Semke <alexander.semke@web.de>
    SPDX-FileCopyrightText: 2021 Stefan Gerlach <stefan.gerlach@uni.kn>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


/*!
  \class Symbol
  \brief

  \ingroup worksheet
*/

#include "Symbol.h"
#include "SymbolPrivate.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/worksheet/Worksheet.h"

#include <KLocalizedString>
#include <math.h>

// order of styles in UI comboboxes (defined in Symbol.h, order can be changed without breaking projects)
static QVector<Symbol::Style> StyleOrder = {Symbol::Style::NoSymbols, Symbol::Style::Circle, Symbol::Style::Square,
		Symbol::Style::Line, Symbol::Style::Cross, Symbol::Style::X,
		Symbol::Style::EquilateralTriangle, Symbol::Style::RightTriangle, Symbol::Style::Bar,
		Symbol::Style::PeakedBar, Symbol::Style::SkewedBar, Symbol::Style::Diamond, Symbol::Style::Lozenge,
		Symbol::Style::Tie, Symbol::Style::TinyTie, Symbol::Style::Plus, Symbol::Style::Boomerang,
		Symbol::Style::SmallBoomerang, Symbol::Style::Star4, Symbol::Style::Star5, Symbol::Style::Heart,
		Symbol::Style::Lightning};

Symbol::Symbol(const QString& name) : AbstractAspect(name, AspectType::AbstractAspect),
	d_ptr(new SymbolPrivate(this)) {
}

void Symbol::init(const KConfigGroup& group) {
	Q_D(Symbol);

	Symbol::Style defaultStyle = Symbol::Style::NoSymbols;
	double defaultSize = Worksheet::convertToSceneUnits(5, Worksheet::Unit::Point);
	QColor defaultBorderColor(Qt::black);
	double defaultBorderWidth = Worksheet::convertToSceneUnits(0.0, Worksheet::Unit::Point);

	auto type = parentAspect()->type();
	if (type == AspectType::CustomPoint)
		defaultStyle = Symbol::Style::Circle;
	else if (type == AspectType::DatapickerImage || type == AspectType::DatapickerCurve) {
		defaultStyle = Symbol::Style::Cross;
		defaultSize = Worksheet::convertToSceneUnits(7, Worksheet::Unit::Point);
		defaultBorderColor = Qt::red;
		defaultBorderWidth = Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Point);
	}

	d->style = (Symbol::Style)group.readEntry("SymbolStyle", (int)defaultStyle);
	d->size = group.readEntry("SymbolSize", defaultSize);
	d->rotationAngle = group.readEntry("SymbolRotation", 0.0);
	d->opacity = group.readEntry("SymbolOpacity", 1.0);
	d->brush.setStyle( (Qt::BrushStyle)group.readEntry("SymbolFillingStyle", (int)Qt::SolidPattern) );
	d->brush.setColor( group.readEntry("SymbolFillingColor", QColor(Qt::red)) );
	d->pen.setStyle( (Qt::PenStyle)group.readEntry("SymbolBorderStyle", (int)Qt::SolidLine) );
	d->pen.setColor( group.readEntry("SymbolBorderColor", defaultBorderColor) );
	d->pen.setWidthF( group.readEntry("SymbolBorderWidth", defaultBorderWidth) );
}

//##############################################################################
//##########################  getter methods  ##################################
//##############################################################################
BASIC_SHARED_D_READER_IMPL(Symbol, Symbol::Style, style, style)
BASIC_SHARED_D_READER_IMPL(Symbol, qreal, opacity, opacity)
BASIC_SHARED_D_READER_IMPL(Symbol, qreal, rotationAngle, rotationAngle)
BASIC_SHARED_D_READER_IMPL(Symbol, qreal, size, size)
BASIC_SHARED_D_READER_IMPL(Symbol, QBrush, brush, brush)
BASIC_SHARED_D_READER_IMPL(Symbol, QPen, pen, pen)

//##############################################################################
//#################  setter methods and undo commands ##########################
//##############################################################################
STD_SETTER_CMD_IMPL_F_S(Symbol, SetStyle, Symbol::Style, style, updateSymbols)
void Symbol::setStyle(Symbol::Style style) {
	Q_D(Symbol);
	if (style != d->style)
		exec(new SymbolSetStyleCmd(d, style, ki18n("%1: set symbol style")));
}

STD_SETTER_CMD_IMPL_F_S(Symbol, SetSize, qreal, size, updateSymbols)
void Symbol::setSize(qreal size) {
	Q_D(Symbol);
	if (!qFuzzyCompare(1 + size, 1 + d->size))
		exec(new SymbolSetSizeCmd(d, size, ki18n("%1: set symbol size")));
}

STD_SETTER_CMD_IMPL_F_S(Symbol, SetRotationAngle, qreal, rotationAngle, updateSymbols)
void Symbol::setRotationAngle(qreal angle) {
	Q_D(Symbol);
	if (!qFuzzyCompare(1 + angle, 1 + d->rotationAngle))
		exec(new SymbolSetRotationAngleCmd(d, angle, ki18n("%1: rotate symbols")));
}

STD_SETTER_CMD_IMPL_F_S(Symbol, SetBrush, QBrush, brush, updatePixmap)
void Symbol::setBrush(const QBrush &brush) {
	Q_D(Symbol);
	if (brush != d->brush)
		exec(new SymbolSetBrushCmd(d, brush, ki18n("%1: set symbol filling")));
}

STD_SETTER_CMD_IMPL_F_S(Symbol, SetPen, QPen, pen, updateSymbols)
void Symbol::setPen(const QPen &pen) {
	Q_D(Symbol);
	if (pen != d->pen)
		exec(new SymbolSetPenCmd(d, pen, ki18n("%1: set symbol outline style")));
}

STD_SETTER_CMD_IMPL_F_S(Symbol, SetOpacity, qreal, opacity, updatePixmap)
void Symbol::setOpacity(qreal opacity) {
	Q_D(Symbol);
	if (opacity != d->opacity)
		exec(new SymbolSetOpacityCmd(d, opacity, ki18n("%1: set symbols opacity")));
}

//##############################################################################
//####################### Private implementation ###############################
//##############################################################################
SymbolPrivate::SymbolPrivate(Symbol* owner) : q(owner) {

}

QString SymbolPrivate::name() const {
	return q->parentAspect()->name();
}

void SymbolPrivate::updateSymbols() {
	emit q->updateRequested();
}

void SymbolPrivate::updatePixmap() {
	emit q->updatePixmapRequested();
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
//! Save as XML
void Symbol::save(QXmlStreamWriter* writer) const {
	Q_D(const Symbol);

	if (parentAspect()->type() == AspectType::CustomPoint)
		writer->writeStartElement("symbol");
	else if (parentAspect()->type() == AspectType::BoxPlot)
		writer->writeStartElement(name()); //BoxPlot has multiple symbols, differentiated by their names
	else
		writer->writeStartElement("symbols"); //keep the backward compatibility for "symbols" used in XYCurve and Histogram

	writer->writeAttribute("symbolsStyle", QString::number(static_cast<int>(d->style)));
	writer->writeAttribute("opacity", QString::number(d->opacity));
	writer->writeAttribute("rotation", QString::number(d->rotationAngle));
	writer->writeAttribute("size", QString::number(d->size));
	WRITE_QBRUSH(d->brush);
	WRITE_QPEN(d->pen);
	writer->writeEndElement(); //close "Symbol" section
}

//! Load from XML
bool Symbol::load(XmlStreamReader* reader, bool preview) {
	if (preview)
		return true;

	Q_D(Symbol);
	KLocalizedString attributeWarning = ki18n("Attribute '%1' missing or empty, default value is used");
	QString str;
	const auto& attribs = reader->attributes();
	READ_INT_VALUE("symbolsStyle", style, Symbol::Style);
	READ_DOUBLE_VALUE("opacity", opacity);
	READ_DOUBLE_VALUE("rotation", rotationAngle);
	READ_DOUBLE_VALUE("size", size);
	READ_QBRUSH(d->brush);
	READ_QPEN(d->pen);

	return true;
}

//##############################################################################
//#########################  Theme management ##################################
//##############################################################################
void Symbol::loadThemeConfig(const KConfigGroup& group, const QColor& themeColor) {
	setOpacity(group.readEntry("SymbolOpacity", 1.0));
	QBrush brush;
	brush.setStyle((Qt::BrushStyle)group.readEntry("SymbolFillingStyle", (int)Qt::SolidPattern));
	brush.setColor(themeColor);
	setBrush(brush);

	QPen p;
	p.setStyle((Qt::PenStyle)group.readEntry("SymbolBorderStyle", (int)Qt::SolidLine));
	p.setColor(themeColor);
	p.setWidthF(group.readEntry("SymbolBorderWidth", Worksheet::convertToSceneUnits(0.0, Worksheet::Unit::Point)));
	setPen(p);
}

void Symbol::saveThemeConfig(const KConfigGroup& /*group*/) const {
	//TODO:
// 	group.writeEntry("SymbolOpacity", opacity());
}

//*************************************************************
//********************* static functions **********************
//*************************************************************

int Symbol::stylesCount() {
	return ENUM_COUNT(Symbol, Style);
}

QString Symbol::styleName(Symbol::Style style) {
	QString name;
	switch (style) {
	case Style::NoSymbols:
		name = i18n("none");
		break;
	case Style::Circle:
		name = i18n("circle");
		break;
	case Style::Square:
		name = i18n("square");
		break;
	case Style::EquilateralTriangle:
		name = i18n("equilateral triangle");
		break;
	case Style::RightTriangle:
		name = i18n("right triangle");
		break;
	case Style::Bar:
		name = i18n("bar");
		break;
	case Style::PeakedBar:
		name = i18n("peaked bar");
		break;
	case Style::SkewedBar:
		name = i18n("skewed bar");
		break;
	case Style::Diamond:
		name = i18n("diamond");
		break;
	case Style::Lozenge:
		name = i18n("lozenge");
		break;
	case Style::Tie:
		name = i18n("tie");
		break;
	case Style::TinyTie:
		name = i18n("tiny tie");
		break;
	case Style::Plus:
		name = i18n("plus");
		break;
	case Style::Boomerang:
		name = i18n("boomerang");
		break;
	case Style::SmallBoomerang:
		name = i18n("small boomerang");
		break;
	case Style::Star4:
		name = i18n("star4");
		break;
	case Style::Star5:
		name = i18n("star5");
		break;
	case Style::Line:
		name = i18n("line");
		break;
	case Style::Cross:
		name = i18n("cross");
		break;
	case Style::X:
		name = i18n("character 'X'");
		break;
	case Style::Heart:
		name = i18n("heart");
		break;
	case Style::Lightning:
		name = i18n("lightning");
		break;
	}

	return name;
}

Symbol::Style Symbol::indexToStyle(const int index) {
	return StyleOrder.at(index);
}

QPainterPath Symbol::stylePath(Symbol::Style style) {
	QPainterPath path;
	QPolygonF polygon;

	switch (style) {
	case Style::NoSymbols:
		break;
	case Style::Circle:
		path.addEllipse(QPoint(0,0), 0.5, 0.5);
		break;
	case Style::Square:
		path.addRect(QRectF(- 0.5, -0.5, 1.0, 1.0));
		break;
	case Style::EquilateralTriangle:
		polygon<<QPointF(-0.5, 0.5)<<QPointF(0, -0.5)<<QPointF(0.5, 0.5)<<QPointF(-0.5, 0.5);
		path.addPolygon(polygon);
		break;
	case Style::RightTriangle:
		polygon<<QPointF(-0.5, -0.5)<<QPointF(0.5, 0.5)<<QPointF(-0.5, 0.5)<<QPointF(-0.5, -0.5);
		path.addPolygon(polygon);
		break;
	case Style::Bar:
		path.addRect(QRectF(- 0.5, -0.2, 1.0, 0.4));
		break;
	case Style::PeakedBar:
		polygon<<QPointF(-0.5, 0)<<QPointF(-0.3, -0.2)<<QPointF(0.3, -0.2)<<QPointF(0.5, 0)
				<<QPointF(0.3, 0.2)<<QPointF(-0.3, 0.2)<<QPointF(-0.5, 0);
		path.addPolygon(polygon);
		break;
	case Style::SkewedBar:
		polygon<<QPointF(-0.5, 0.2)<<QPointF(-0.2, -0.2)<<QPointF(0.5, -0.2)<<QPointF(0.2, 0.2)<<QPointF(-0.5, 0.2);
		path.addPolygon(polygon);
		break;
	case Style::Diamond:
		polygon<<QPointF(-0.5, 0)<<QPointF(0, -0.5)<<QPointF(0.5, 0)<<QPointF(0, 0.5)<<QPointF(-0.5, 0);
		path.addPolygon(polygon);
		break;
	case Style::Lozenge:
		polygon<<QPointF(-0.25, 0)<<QPointF(0, -0.5)<<QPointF(0.25, 0)<<QPointF(0, 0.5)<<QPointF(-0.25, 0);
		path.addPolygon(polygon);
		break;
	case Style::Tie:
		polygon<<QPointF(-0.5, -0.5)<<QPointF(0.5, -0.5)<<QPointF(-0.5, 0.5)<<QPointF(0.5, 0.5)<<QPointF(-0.5, -0.5);
		path.addPolygon(polygon);
		break;
	case Style::TinyTie:
		polygon<<QPointF(-0.2, -0.5)<<QPointF(0.2, -0.5)<<QPointF(-0.2, 0.5)<<QPointF(0.2, 0.5)<<QPointF(-0.2, -0.5);
		path.addPolygon(polygon);
		break;
	case Style::Plus:
		polygon<<QPointF(-0.2, -0.5)<<QPointF(0.2, -0.5)<<QPointF(0.2, -0.2)<<QPointF(0.5, -0.2)<<QPointF(0.5, 0.2)
				<<QPointF(0.2, 0.2)<<QPointF(0.2, 0.5)<<QPointF(-0.2, 0.5)<<QPointF(-0.2, 0.2)<<QPointF(-0.5, 0.2)
				<<QPointF(-0.5, -0.2)<<QPointF(-0.2, -0.2)<<QPointF(-0.2, -0.5);
		path.addPolygon(polygon);
		break;
	case Style::Boomerang:
		polygon<<QPointF(-0.5, 0.5)<<QPointF(0, -0.5)<<QPointF(0.5, 0.5)<<QPointF(0, 0)<<QPointF(-0.5, 0.5);
		path.addPolygon(polygon);
		break;
	case Style::SmallBoomerang:
		polygon<<QPointF(-0.3, 0.5)<<QPointF(0, -0.5)<<QPointF(0.3, 0.5)<<QPointF(0, 0)<<QPointF(-0.3, 0.5);
		path.addPolygon(polygon);
		break;
	case Style::Star4:
		polygon<<QPointF(-0.5, 0)<<QPointF(-0.1, -0.1)<<QPointF(0, -0.5)<<QPointF(0.1, -0.1)<<QPointF(0.5, 0)
				<<QPointF(0.1, 0.1)<<QPointF(0, 0.5)<<QPointF(-0.1, 0.1)<<QPointF(-0.5, 0);
		path.addPolygon(polygon);
		break;
	case Style::Star5:
		polygon<<QPointF(-0.5, 0)<<QPointF(-0.1, -0.1)<<QPointF(0, -0.5)<<QPointF(0.1, -0.1)<<QPointF(0.5, 0)
				<<QPointF(0.1, 0.1)<<QPointF(0.5, 0.5)<<QPointF(0, 0.2)<<QPointF(-0.5, 0.5)
				<<QPointF(-0.1, 0.1)<<QPointF(-0.5, 0);
		path.addPolygon(polygon);
		break;
	case Style::Line:
		path = QPainterPath(QPointF(0, -0.5));
		path.lineTo(0, 0.5);
		break;
	case Style::Cross:
		path = QPainterPath(QPointF(0, -0.5));
		path.lineTo(0, 0.5);
		path.moveTo(-0.5, 0);
		path.lineTo(0.5, 0);
		break;
	case Style::X:
		path = QPainterPath(QPointF(-0.4, -0.5));
		path.lineTo(0.4, 0.5);
		path.moveTo(0.4, -0.5);
		path.lineTo(-0.4, 0.5);
		break;
	case Style::Heart: {
		//https://mathworld.wolfram.com/HeartCurve.html with additional
		//normalization to fit into a 1.0x1.0 rectangular
		int steps = 100;
		double range = 2*M_PI/(steps - 1);
		for (int i = 0; i < steps; ++i) {
			double t = i*range + M_PI/2;
			double x = pow(sin(t), 3);
			double y = -(13*cos(t) - 5*cos(2*t) - 2*cos(3*t) - cos(4*t))/17;
			polygon << QPointF(x/2, y/2);
		}
		double t = M_PI/2;
		double x = pow(sin(t), 3);
		double y = -(13*cos(t) - 5*cos(2*t) - 2*cos(3*t) - cos(4*t))/17;
		polygon << QPointF(x/2, y/2);
		path.addPolygon(polygon);
		break;
	}
	case Style::Lightning:
		polygon << QPointF(0, 0.5)
			<< QPointF(0.4, -0.03)
			<< QPointF(0, -0.03)
			<< QPointF(0.2, -0.5)
			<< QPointF(-0.4, 0.1)
			<< QPointF(0.06, 0.1)
			<< QPointF(0, 0.5);
		path.addPolygon(polygon);
		break;
	}

	return path;
}
