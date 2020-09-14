/***************************************************************************
    File                 : Symbol.cpp
    Project              : LabPlot
    Description          : Symbol
    --------------------------------------------------------------------
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

/*!
  \class Symbol
  \brief 

  \ingroup worksheet
*/

#include "Symbol.h"
#include <KLocalizedString>

QPainterPath Symbol::pathFromStyle(Symbol::Style style) {
	QPainterPath path;
	QPolygonF polygon;
	if (style == Style::Circle) {
		path.addEllipse(QPoint(0,0), 0.5, 0.5);
	} else if (style == Style::Square) {
		path.addRect(QRectF(- 0.5, -0.5, 1.0, 1.0));
	} else if (style == Style::EquilateralTriangle) {
		polygon<<QPointF(-0.5, 0.5)<<QPointF(0, -0.5)<<QPointF(0.5, 0.5)<<QPointF(-0.5, 0.5);
		path.addPolygon(polygon);
	} else if (style == Style::RightTriangle) {
		polygon<<QPointF(-0.5, -0.5)<<QPointF(0.5, 0.5)<<QPointF(-0.5, 0.5)<<QPointF(-0.5, -0.5);
		path.addPolygon(polygon);
	} else if (style == Style::Bar) {
		path.addRect(QRectF(- 0.5, -0.2, 1.0, 0.4));
	} else if (style == Style::PeakedBar) {
		polygon<<QPointF(-0.5, 0)<<QPointF(-0.3, -0.2)<<QPointF(0.3, -0.2)<<QPointF(0.5, 0)
				<<QPointF(0.3, 0.2)<<QPointF(-0.3, 0.2)<<QPointF(-0.5, 0);
		path.addPolygon(polygon);
	} else if (style == Style::SkewedBar) {
		polygon<<QPointF(-0.5, 0.2)<<QPointF(-0.2, -0.2)<<QPointF(0.5, -0.2)<<QPointF(0.2, 0.2)<<QPointF(-0.5, 0.2);
		path.addPolygon(polygon);
	} else if (style == Style::Diamond) {
		polygon<<QPointF(-0.5, 0)<<QPointF(0, -0.5)<<QPointF(0.5, 0)<<QPointF(0, 0.5)<<QPointF(-0.5, 0);
		path.addPolygon(polygon);
	} else if (style == Style::Lozenge) {
		polygon<<QPointF(-0.25, 0)<<QPointF(0, -0.5)<<QPointF(0.25, 0)<<QPointF(0, 0.5)<<QPointF(-0.25, 0);
		path.addPolygon(polygon);
	} else if (style == Style::Tie) {
		polygon<<QPointF(-0.5, -0.5)<<QPointF(0.5, -0.5)<<QPointF(-0.5, 0.5)<<QPointF(0.5, 0.5)<<QPointF(-0.5, -0.5);
		path.addPolygon(polygon);
	} else if (style == Style::TinyTie) {
		polygon<<QPointF(-0.2, -0.5)<<QPointF(0.2, -0.5)<<QPointF(-0.2, 0.5)<<QPointF(0.2, 0.5)<<QPointF(-0.2, -0.5);
		path.addPolygon(polygon);
	} else if (style == Style::Plus) {
		polygon<<QPointF(-0.2, -0.5)<<QPointF(0.2, -0.5)<<QPointF(0.2, -0.2)<<QPointF(0.5, -0.2)<<QPointF(0.5, 0.2)
				<<QPointF(0.2, 0.2)<<QPointF(0.2, 0.5)<<QPointF(-0.2, 0.5)<<QPointF(-0.2, 0.2)<<QPointF(-0.5, 0.2)
				<<QPointF(-0.5, -0.2)<<QPointF(-0.2, -0.2)<<QPointF(-0.2, -0.5);
		path.addPolygon(polygon);
	} else if (style == Style::Boomerang) {
		polygon<<QPointF(-0.5, 0.5)<<QPointF(0, -0.5)<<QPointF(0.5, 0.5)<<QPointF(0, 0)<<QPointF(-0.5, 0.5);
		path.addPolygon(polygon);
	} else if (style == Style::SmallBoomerang) {
		polygon<<QPointF(-0.3, 0.5)<<QPointF(0, -0.5)<<QPointF(0.3, 0.5)<<QPointF(0, 0)<<QPointF(-0.3, 0.5);
		path.addPolygon(polygon);
	} else if (style == Style::Star4) {
		polygon<<QPointF(-0.5, 0)<<QPointF(-0.1, -0.1)<<QPointF(0, -0.5)<<QPointF(0.1, -0.1)<<QPointF(0.5, 0)
				<<QPointF(0.1, 0.1)<<QPointF(0, 0.5)<<QPointF(-0.1, 0.1)<<QPointF(-0.5, 0);
		path.addPolygon(polygon);
	} else if (style == Style::Star5) {
		polygon<<QPointF(-0.5, 0)<<QPointF(-0.1, -0.1)<<QPointF(0, -0.5)<<QPointF(0.1, -0.1)<<QPointF(0.5, 0)
				<<QPointF(0.1, 0.1)<<QPointF(0.5, 0.5)<<QPointF(0, 0.2)<<QPointF(-0.5, 0.5)
				<<QPointF(-0.1, 0.1)<<QPointF(-0.5, 0);
		path.addPolygon(polygon);
	} else if (style == Style::Line) {
		path = QPainterPath(QPointF(0, -0.5));
		path.lineTo(0, 0.5);
	} else if (style == Style::Cross) {
		path = QPainterPath(QPointF(0, -0.5));
		path.lineTo(0, 0.5);
		path.moveTo(-0.5, 0);
		path.lineTo(0.5, 0);
	}

	return path;
}

QString Symbol::nameFromStyle(Symbol::Style style) {
	QString name;
	//TODO: use 'switch'
	if (style == Style::Circle)
		name = i18n("circle");
	else if (style == Style::Square)
		name = i18n("square");
	else if (style == Style::EquilateralTriangle)
		name = i18n("equilateral triangle");
	else if (style == Style::RightTriangle)
		name = i18n("right triangle");
	else if (style == Style::Bar)
		name = i18n("bar");
	else if (style == Style::PeakedBar)
		name = i18n("peaked bar");
	else if (style == Style::SkewedBar)
		name = i18n("skewed bar");
	else if (style == Style::Diamond)
		name = i18n("diamond");
	else if (style == Style::Lozenge)
		name = i18n("lozenge");
	else if (style == Style::Tie)
		name = i18n("tie");
	else if (style == Style::TinyTie)
		name = i18n("tiny tie");
	else if (style == Style::Plus)
		name = i18n("plus");
	else if (style == Style::Boomerang)
		name = i18n("boomerang");
	else if (style == Style::SmallBoomerang)
		name = i18n("small boomerang");
	else if (style == Style::Star4)
		name = i18n("star4");
	else if (style == Style::Star5)
		name = i18n("star5");
	else if (style == Style::Line)
		name = i18n("line");
	else if (style == Style::Cross)
		name = i18n("cross");

	return name;
}
