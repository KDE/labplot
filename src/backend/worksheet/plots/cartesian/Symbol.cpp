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

#include <QFont>

extern "C" {
#include <gsl/gsl_math.h>
}

// order of styles in UI comboboxes (defined in Symbol.h, order can be changed without breaking projects)
static QVector<Symbol::Style> StyleOrder = {Symbol::Style::NoSymbols, Symbol::Style::Circle, Symbol::Style::Square,
		Symbol::Style::EquilateralTriangle, Symbol::Style::Line, Symbol::Style::Cross, Symbol::Style::Tri,
		Symbol::Style::X, Symbol::Style::Asterisk, Symbol::Style::XPlus, Symbol::Style::TallPlus,
		Symbol::Style::LatinCross, Symbol::Style::DotPlus, Symbol::Style::Pin, Symbol::Style::Hash,
		Symbol::Style::SquareX, Symbol::Style::SquarePlus, Symbol::Style::SquareHalf, Symbol::Style::SquareDot,
		Symbol::Style::SquareDiag, Symbol::Style::SquareTriangle,
		Symbol::Style::CircleHalf, Symbol::Style::CircleDot, Symbol::Style::CircleX, Symbol::Style::CircleTri,
		Symbol::Style::Peace, Symbol::Style::TriangleDot, Symbol::Style::TriangleLine, Symbol::Style::TriangleHalf,
		Symbol::Style::RightTriangle,
		Symbol::Style::Bar, Symbol::Style::PeakedBar, Symbol::Style::SkewedBar,
		Symbol::Style::Diamond, Symbol::Style::Lozenge, Symbol::Style::Tie, Symbol::Style::TinyTie,
		Symbol::Style::Boomerang, Symbol::Style::SmallBoomerang, Symbol::Style::Star, Symbol::Style::Star3,
		Symbol::Style::Star4, Symbol::Style::Star5, Symbol::Style::Star6,
		Symbol::Style::Plus, Symbol::Style::Latin, Symbol::Style::David, Symbol::Style::Home, Symbol::Style::Pentagon,
		Symbol::Style::Hexagon, Symbol::Style::Female, Symbol::Style::Male,
		Symbol::Style::Flower, Symbol::Style::Flower2, Symbol::Style::Flower3, Symbol::Style::Flower5, Symbol::Style::Flower6,
		Symbol::Style::Heart, Symbol::Style::Spade, Symbol::Style::Club, Symbol::Style::Lightning};

Symbol::Symbol(const QString& name) : AbstractAspect(name, AspectType::AbstractAspect),
	d_ptr(new SymbolPrivate(this)) {
}

Symbol::~Symbol() {
	delete d_ptr;
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
	Q_EMIT q->updateRequested();
}

void SymbolPrivate::updatePixmap() {
	Q_EMIT q->updatePixmapRequested();
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
	switch (style) {
	case Style::NoSymbols:
		return i18n("none");
	case Style::Circle:
		return i18n("circle");
	case Style::Square:
		return i18n("square");
	case Style::EquilateralTriangle:
		return i18n("equilateral triangle");
	case Style::RightTriangle:
		return i18n("right triangle");
	case Style::Bar:
		return i18n("bar");
	case Style::PeakedBar:
		return i18n("peaked bar");
	case Style::SkewedBar:
		return i18n("skewed bar");
	case Style::Diamond:
		return i18n("diamond");
	case Style::Lozenge:
		return i18n("lozenge");
	case Style::Tie:
		return i18n("tie");
	case Style::TinyTie:
		return i18n("tiny tie");
	case Style::Plus:
		return i18n("plus");
	case Style::Boomerang:
		return i18n("boomerang");
	case Style::SmallBoomerang:
		return i18n("small boomerang");
	case Style::Star4:
		return i18n("star4");
	case Style::Star5:
		return i18n("star5");
	case Style::Line:
		return i18n("line");
	case Style::Cross:
		return i18n("cross");
	case Style::Heart:
		return i18n("heart");
	case Style::Lightning:
		return i18n("lightning");
	case Style::X:
		return i18n("character 'X'");
	case Style::Asterisk:
		return i18n("asterisk");
	case Style::Tri:
		return i18n("tri");
	case Style::XPlus:
		return i18n("x plus");
	case Style::TallPlus:
		return i18n("tall plus");
	case Style::LatinCross:
		return i18n("latin cross");
	case Style::DotPlus:
		return i18n("dot plus");
	case Style::Hash:
		return i18n("hash");
	case Style::SquareX:
		return i18n("square x");
	case Style::SquarePlus:
		return i18n("square plus");
	case Style::SquareHalf:
		return i18n("half square");
	case Style::SquareDot:
		return i18n("square dot");
	case Style::SquareDiag:
		return i18n("diag square");
	case Style::SquareTriangle:
		return i18n("square triangle");
	case Style::CircleHalf:
		return i18n("circle half");
	case Style::CircleDot:
		return i18n("circle dot");
	case Style::CircleX:
		return i18n("circle x");
	case Style::CircleTri:
		return i18n("circle tri");
	case Style::Peace:
		return i18n("peace");
	case Style::TriangleDot:
		return i18n("triangle dot");
	case Style::TriangleLine:
		return i18n("triangle line");
	case Style::TriangleHalf:
		return i18n("half triangle");
	case Style::Flower:
		return i18n("flower");
	case Style::Flower2:
		return i18n("flower2");
	case Style::Flower3:
		return i18n("flower3");
	case Style::Flower5:
		return i18n("flower5");
	case Style::Flower6:
		return i18n("flower6");
	case Style::Star:
		return i18n("star");
	case Style::Star3:
		return i18n("star3");
	case Style::Star6:
		return i18n("star6");
	case Style::Pentagon:
		return i18n("pentagon");
	case Style::Hexagon:
		return i18n("hexagon");
	case Style::Latin:
		return i18n("latin");
	case Style::David:
		return i18n("david");
	case Style::Home:
		return i18n("home");
	case Style::Pin:
		return i18n("pin");
	case Style::Female:
		return i18n("female");
	case Style::Male:
		return i18n("male");
	case Style::Spade:
		return i18n("spade");
	case Style::Club:
		return i18n("club");
	}

	return QString();
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
		path.addEllipse(QPoint(0, 0), 0.5, 0.5);
		break;
	case Style::Square:
		path.addRect(QRectF(- 0.5, -0.5, 1.0, 1.0));
		break;
	case Style::EquilateralTriangle:
		polygon << QPointF(-0.5, 0.5)<<QPointF(0, -0.5)<<QPointF(0.5, 0.5)<<QPointF(-0.5, 0.5);
		path.addPolygon(polygon);
		break;
	case Style::RightTriangle:
		polygon << QPointF(-0.5, -0.5)<<QPointF(0.5, 0.5)<<QPointF(-0.5, 0.5)<<QPointF(-0.5, -0.5);
		path.addPolygon(polygon);
		break;
	case Style::Bar:
		path.addRect(QRectF(- 0.5, -0.2, 1.0, 0.4));
		break;
	case Style::PeakedBar:
		polygon << QPointF(-0.5, 0)<<QPointF(-0.3, -0.2)<<QPointF(0.3, -0.2)<<QPointF(0.5, 0)
				<<QPointF(0.3, 0.2)<<QPointF(-0.3, 0.2)<<QPointF(-0.5, 0);
		path.addPolygon(polygon);
		break;
	case Style::SkewedBar:
		polygon << QPointF(-0.5, 0.2)<<QPointF(-0.2, -0.2)<<QPointF(0.5, -0.2)<<QPointF(0.2, 0.2)<<QPointF(-0.5, 0.2);
		path.addPolygon(polygon);
		break;
	case Style::Diamond:
		polygon << QPointF(-0.5, 0)<<QPointF(0, -0.5)<<QPointF(0.5, 0)<<QPointF(0, 0.5)<<QPointF(-0.5, 0);
		path.addPolygon(polygon);
		break;
	case Style::Lozenge:
		polygon << QPointF(-0.25, 0)<<QPointF(0, -0.5)<<QPointF(0.25, 0)<<QPointF(0, 0.5)<<QPointF(-0.25, 0);
		path.addPolygon(polygon);
		break;
	case Style::Tie:
		polygon << QPointF(-0.5, -0.5)<<QPointF(0.5, -0.5)<<QPointF(-0.5, 0.5)<<QPointF(0.5, 0.5)<<QPointF(-0.5, -0.5);
		path.addPolygon(polygon);
		break;
	case Style::TinyTie:
		polygon << QPointF(-0.2, -0.5)<<QPointF(0.2, -0.5)<<QPointF(-0.2, 0.5)<<QPointF(0.2, 0.5)<<QPointF(-0.2, -0.5);
		path.addPolygon(polygon);
		break;
	case Style::Plus:
		polygon << QPointF(-0.2, -0.5)<<QPointF(0.2, -0.5)<<QPointF(0.2, -0.2)<<QPointF(0.5, -0.2)<<QPointF(0.5, 0.2)
				<<QPointF(0.2, 0.2)<<QPointF(0.2, 0.5)<<QPointF(-0.2, 0.5)<<QPointF(-0.2, 0.2)<<QPointF(-0.5, 0.2)
				<<QPointF(-0.5, -0.2)<<QPointF(-0.2, -0.2)<<QPointF(-0.2, -0.5);
		path.addPolygon(polygon);
		break;
	case Style::Boomerang:
		polygon << QPointF(-0.5, 0.5)<<QPointF(0, -0.5)<<QPointF(0.5, 0.5)<<QPointF(0, 0)<<QPointF(-0.5, 0.5);
		path.addPolygon(polygon);
		break;
	case Style::SmallBoomerang:
		polygon << QPointF(-0.3, 0.5)<<QPointF(0, -0.5)<<QPointF(0.3, 0.5)<<QPointF(0, 0)<<QPointF(-0.3, 0.5);
		path.addPolygon(polygon);
		break;
	case Style::Star4:
		polygon << QPointF(-0.5, 0)<<QPointF(-0.1, -0.1)<<QPointF(0, -0.5)<<QPointF(0.1, -0.1)<<QPointF(0.5, 0)
				<<QPointF(0.1, 0.1)<<QPointF(0, 0.5)<<QPointF(-0.1, 0.1)<<QPointF(-0.5, 0);
		path.addPolygon(polygon);
		break;
	case Style::Star5:
		polygon << QPointF(-0.5, 0)<<QPointF(-0.1, -0.1)<<QPointF(0, -0.5)<<QPointF(0.1, -0.1)<<QPointF(0.5, 0)
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
	case Style::Heart: {
		//https://mathworld.wolfram.com/HeartCurve.html with additional
		//normalization to fit into a 1.0x1.0 rectangular
		int steps = 100;
		double range = 2.*M_PI/(steps - 1);
		for (int i = 0; i < steps; ++i) {
			double t = i*range + M_PI/2;
			double x = gsl_pow_3(sin(t));
			double y = -(13*cos(t) - 5*cos(2*t) - 2*cos(3*t) - cos(4*t))/17;
			polygon << QPointF(x/2, y/2);
		}
		double t = M_PI/2.;
		double x = gsl_pow_3(sin(t));
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
	case Style::X:
		path = QPainterPath(QPointF(-0.4, -0.5));
		path.lineTo(0.4, 0.5);
		path.moveTo(0.4, -0.5);
		path.lineTo(-0.4, 0.5);
		break;
	case Style::Asterisk:
		path = QPainterPath(QPointF(0., .5));
		path.lineTo(0., -.5);
		path.moveTo(M_SQRT3 / 4., -.25);
		path.lineTo(-M_SQRT3 / 4., .25);
		path.moveTo(M_SQRT3 / 4., .25);
		path.lineTo(-M_SQRT3 / 4., -.25);
		break;
	case Style::Tri:
		path = QPainterPath(QPointF(0., 0.));
		path.lineTo(0., -1.);
		path.moveTo(0., 0.);
		path.lineTo(-M_SQRT3/2., 1./2.);
		path.moveTo(0., 0.);
		path.lineTo(M_SQRT3/2., 1./2.);
		break;
	case Style::XPlus:
		path = QPainterPath(QPointF(.5, 0.));
		path.lineTo(-.5, 0.);
		path.moveTo(0., .5);
		path.lineTo(0., -.5);
		path.moveTo(.5/M_SQRT2, .5/M_SQRT2);
		path.lineTo(-.5/M_SQRT2, -.5/M_SQRT2);
		path.moveTo(.5/M_SQRT2, -.5/M_SQRT2);
		path.lineTo(-.5/M_SQRT2, .5/M_SQRT2);
		break;
	case Style::TallPlus:
		path = QPainterPath(QPointF(.25, 0.));
		path.lineTo(-.25, 0.);
		path.moveTo(0., .5);
		path.lineTo(0., -.5);
		break;
	case Style::LatinCross:
		path = QPainterPath(QPointF(0., .5));
		path.lineTo(0., -.5);
		path.moveTo(-1./3., -1./6.);
		path.lineTo(1./3., -1./6.);
		break;
	case Style::DotPlus:
		path = QPainterPath(QPointF(0., .5));
		path.lineTo(0., .25);
		path.moveTo(0., -.5);
		path.lineTo(0., -.25);
		path.moveTo(.5, 0.);
		path.lineTo(.25, 0.);
		path.moveTo(-.5, 0.);
		path.lineTo(-.25, 0.);
		path.addEllipse(-.05, -.05, .1, .1);
		break;
	case Style::Hash:
		path = QPainterPath(QPointF(-.25, .5));
		path.lineTo(-.25, -.5);
		path.moveTo(.25, .5);
		path.lineTo(.25, -.5);
		path.moveTo(.5, .25);
		path.lineTo(-.5, .25);
		path.moveTo(.5, -.25);
		path.lineTo(-.5, -.25);
		break;
	case Style::SquareX:
		path = QPainterPath(QPointF(-.5, .5));
		path.lineTo(.5, -.5);
		path.lineTo(-.5, -.5);
		path.lineTo(.5, .5);
		path.lineTo(-.5, .5);
		path.moveTo(-.5, .5);
		path.lineTo(-.5, -.5);
		path.moveTo(.5, .5);
		path.lineTo(.5, -.5);
		break;
	case Style::SquarePlus:
		path = QPainterPath(QPointF(-.5, .5));
		path.lineTo(-.5, 0.);
		path.lineTo(.5, 0.);
		path.lineTo(.5, -.5);
		path.lineTo(0., -.5);
		path.lineTo(0., .5);
		path.lineTo(-.5, .5);
		path.moveTo(.5, .5);
		path.lineTo(0., .5);
		path.moveTo(.5, .5);
		path.lineTo(.5, 0);
		path.moveTo(-.5, -.5);
		path.lineTo(0., -.5);
		path.moveTo(-.5, -.5);
		path.lineTo(-.5, 0);
		break;
	case Style::SquareHalf:
		path = QPainterPath(QPointF(-.5, .5));
		path.lineTo(.5, .5);
		path.lineTo(.5, 0.);
		path.lineTo(-.5, 0.);
		path.lineTo(-.5, .5);
		path.moveTo(-.5, -.5);
		path.lineTo(-.5, 0);
		path.moveTo(-.5, -.5);
		path.lineTo(.5, -.5);
		path.moveTo(.5, -.5);
		path.lineTo(.5, 0.);
		break;
	case Style::SquareDot:
		path.addEllipse(-.1, -.1, .2, .2);
		path.addRect(QRectF(- 0.5, -0.5, 1.0, 1.0));
		break;
	case Style::SquareDiag:
		path = QPainterPath(QPointF(-.5, .5));
		path.lineTo(.5, .5);
		path.lineTo(-.5, -.5);
		path.lineTo(-.5, .5);
		path.moveTo(.5, -.5);
		path.lineTo(.5, .5);
		path.moveTo(.5, -.5);
		path.lineTo(-.5, -.5);
		break;
	case Style::SquareTriangle:
		path = QPainterPath(QPointF(-.5, .5));
		path.lineTo(0, -.5);
		path.lineTo(.5, .5);
		path.lineTo(-.5, .5);
		path.moveTo(.5, -.5);
		path.lineTo(-.5, -.5);
		path.moveTo(.5, -.5);
		path.lineTo(.5, .5);
		path.moveTo(-.5, -.5);
		path.lineTo(-.5, .5);
		break;
	case Style::CircleHalf:
		path = QPainterPath(QPointF(0., .5));
		path.arcTo(-.5, -.5, 1., 1., -90., 180.);
		path.closeSubpath();
		path.addEllipse(QPoint(0, 0), 0.5, 0.5);
		break;
	case Style::CircleDot:
		path.addEllipse(-.1, -.1, .2, .2);
		path.addEllipse(QPoint(0, 0), 0.5, 0.5);
		break;
	case Style::CircleX:
		path = QPainterPath(QPointF(.5/M_SQRT2, .5/M_SQRT2));
		path.lineTo(-.5/M_SQRT2, -.5/M_SQRT2);
		path.arcTo(-.5, -.5, 1., 1., -45., 90.);
		path.lineTo(-.5/M_SQRT2, .5/M_SQRT2);
		path.arcTo(-.5, -.5, 1., 1., 225., -90.);
		path.closeSubpath();
		path.addEllipse(QPoint(0, 0), 0.5, 0.5);
		break;
	case Style::CircleTri:
		path.addEllipse(QPoint(0, 0), 0.5, 0.5);
		path.moveTo(0., 0.);
		path.lineTo(0., -.5);
		path.moveTo(0., 0.);
		path.lineTo(-M_SQRT3/4., 1./4.);
		path.moveTo(0., 0.);
		path.lineTo(M_SQRT3/4., 1./4.);
		break;
	case Style::Peace:
		path = QPainterPath(QPointF(0, .5));
		path.lineTo(0, -.5);
		path.moveTo(0., 0.);
		path.lineTo(-.5/M_SQRT2, .5/M_SQRT2);
		path.moveTo(0., 0.);
		path.lineTo(.5/M_SQRT2, .5/M_SQRT2);
		path.closeSubpath();
		path.addEllipse(QPoint(0, 0), 0.5, 0.5);
		break;
	case Style::TriangleDot:
		path.addEllipse(-.1, -.1, .2, .2);
		polygon << QPointF(-0.5, 0.5)<<QPointF(0, -0.5)<<QPointF(0.5, 0.5)<<QPointF(-0.5, 0.5);
		path.addPolygon(polygon);
		break;
	case Style::TriangleHalf:
		path = QPainterPath(QPointF(-.25, 0));
		path.lineTo(0, -.5);
		path.lineTo(.25, 0);
		path.closeSubpath();
		path.moveTo(.5, .5);
		path.lineTo(.25, 0);
		path.moveTo(.5, .5);
		path.lineTo(-.5, .5);
		path.moveTo(-.5, .5);
		path.lineTo(-.25, 0);
		break;
	case Style::TriangleLine:
		path = QPainterPath(QPointF(-.5, .5));
		path.lineTo(0, -.5);
		path.lineTo(0, .5);
		path.closeSubpath();
		path.moveTo(.5, .5);
		path.lineTo(0, .5);
		path.moveTo(.5, .5);
		path.lineTo(0, -.5);
		break;
	case Style::Flower: {
		int steps = 100;
		double range = 2.*M_PI/(steps - 1);
		for (int i = 0; i < steps; ++i) {
			double t = i*range;
			double r = sin(2*t) * sin(2*t);
			double x = r * sin(t);
			double y = r * cos(t);
			polygon << QPointF(x/2, y/2);
		}
		path.addPolygon(polygon);
		break;
	}
	case Style::Flower2: {
		int steps = 100;
		double range = 2.*M_PI/(steps - 1);
		for (int i = 0; i < steps; ++i) {
			double t = i*range;
			double r = sin(t) * sin(t);
			double x = r * sin(t);
			double y = r * cos(t);
			polygon << QPointF(x/2, y/2);
		}
		path.addPolygon(polygon);
		break;
	}
	case Style::Flower3: {
		int steps = 100;
		double range = 2.*M_PI/(steps - 1);
		for (int i = 0; i < steps; ++i) {
			double t = i*range;
			double r = sin(3.*t/2.) * sin(3.*t/2.);
			double x = r * sin(t);
			double y = r * cos(t);
			polygon << QPointF(x/2, y/2);
		}
		path.addPolygon(polygon);
		break;
	}
	case Style::Flower5: {
		int steps = 100;
		double range = 2.*M_PI/(steps - 1);
		for (int i = 0; i < steps; ++i) {
			double t = i*range;
			double r = sin(5.*t/2.) * sin(5.*t/2.);
			double x = r * sin(t);
			double y = r * cos(t);
			polygon << QPointF(x/2, y/2);
		}
		path.addPolygon(polygon);
		break;
	}
	case Style::Flower6: {
		int steps = 100;
		double range = 2.*M_PI/(steps - 1);
		for (int i = 0; i < steps; ++i) {
			double t = i*range;
			double r = sin(3.*t) * sin(3.*t);
			double x = r * sin(t);
			double y = r * cos(t);
			polygon << QPointF(x/2, y/2);
		}
		path.addPolygon(polygon);
		break;
	}
	case Style::Star:
		for (int i = 0; i < 5; i++) {
			double angle = 2.*M_PI*i/5. - M_PI/10.;
			polygon << QPointF(.5 * cos(angle), .5 * sin(angle));
			polygon << QPointF(.2 * cos(angle + M_PI/5.), .2 * sin(angle + M_PI/5.));
		}
		path.addPolygon(polygon);
		path.closeSubpath();
		break;
	case Style::Star3:
		for (int i = 0; i < 3; i++) {
			double angle = 2.*M_PI*i/3. + M_PI/6.;
			polygon << QPointF(.5 * cos(angle), .5 * sin(angle));
			polygon << QPointF(.1 * cos(angle + M_PI/3.), .1 * sin(angle + M_PI/3.));
		}
		path.addPolygon(polygon);
		path.closeSubpath();
		break;
	case Style::Star6:
		for (int i = 0; i < 6; i++) {
			double angle = 2.*M_PI*i/6.;
			polygon << QPointF(.5 * cos(angle), .5 * sin(angle));
			polygon << QPointF(.1 * cos(angle + M_PI/6.), .1 * sin(angle + M_PI/6.));
		}
		path.addPolygon(polygon);
		path.closeSubpath();
		break;
	case Style::Pentagon:
		for (int i = 0; i < 5; i++) {
			double angle = 2.*M_PI*i/5. - M_PI/10.;
			polygon << QPointF(.5 * cos(angle), .5 * sin(angle));
		}
		path.addPolygon(polygon);
		path.closeSubpath();
		break;
	case Style::Hexagon:
		for (int i = 0; i < 6; i++) {
			double angle = 2.*M_PI*i/6.;
			polygon << QPointF(.5 * cos(angle), .5 * sin(angle));
		}
		path.addPolygon(polygon);
		path.closeSubpath();
		break;
	case Style::Latin:
		polygon<<QPointF(-0.1, -0.5)<<QPointF(0.1, -0.5)<<QPointF(0.1, -0.3)<<QPointF(0.5, -0.3)<<QPointF(0.5, -0.1)
				<<QPointF(0.1, -0.1)<<QPointF(0.1, 0.5)<<QPointF(-0.1, 0.5)<<QPointF(-0.1, -0.1)<<QPointF(-0.5, -0.1)
				<<QPointF(-0.5, -0.3)<<QPointF(-0.1, -0.3)<<QPointF(-0.1, -0.5);
		path.addPolygon(polygon);
		break;
	case Style::David:
		for (int i = 0; i < 4; i++) {
			double angle = 2.*M_PI*i/3. + M_PI/6.;
			polygon << QPointF(.5 * cos(angle), .5 * sin(angle));
		}
		path.setFillRule(Qt::FillRule::WindingFill);
		path.addPolygon(polygon);
		polygon.clear();
		for (int i = 0; i < 4; i++) {
			double angle = 2.*M_PI*i/3. - M_PI/6.;
			polygon << QPointF(.5 * cos(angle), .5 * sin(angle));
		}

		path.addPolygon(polygon);
		break;
	case Style::Home:
		path = QPainterPath(QPointF(-.25, .25));
		path.lineTo(.25, .25);
		path.lineTo(.25, -.25);
		path.lineTo(0., -.5);
		path.lineTo(-.25, -.25);
		path.lineTo(-.25, .25);
		break;
	case Style::Pin:
		path = QPainterPath(QPointF(0., 0.));
		path.lineTo(0., -.3);
		path.closeSubpath();
		path.addEllipse(QPointF(0., -.4), 0.2, 0.2);
		break;
	case Style::Female:
		path.addEllipse(QPointF(0., 0.), .25, .25);
		path.closeSubpath();
		path.moveTo(0., .25);
		path.lineTo(0., .5);
		path.moveTo(-.15, .375);
		path.lineTo(.15, .375);
		break;
	case Style::Male:
		path.addEllipse(QPointF(0., 0.), .3, .3);
		path.closeSubpath();
		path.moveTo(.3/M_SQRT2, -.3/M_SQRT2);
		path.lineTo(.5, -.5);
		path.moveTo(.5, -.5);
		path.lineTo(.35, -.5);
		path.moveTo(.5, -.5);
		path.lineTo(.5, -.35);
		break;
	case Style::Spade: {
		QFont font("Times", 1);
		path.addText(-.3, .3, font, UTF8_QSTRING("♠"));
		break;
	}
	case Style::Club:
		QFont font("Times", 1);
		path.addText(-.3, .3, font, UTF8_QSTRING("♣"));
		break;
		break;
	}

	return path;
}
