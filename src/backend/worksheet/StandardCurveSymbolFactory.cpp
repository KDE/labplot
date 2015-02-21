/***************************************************************************
    File                 : StandardCurveSymbolFactory.cpp
    Project              : LabPlot
    Description          : Factory of built-in curve symbols.
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs@gmx.net)
    Copyright            : (C) 2010 Alexander Semke (alexander.semke@web.de)

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

#include "backend/worksheet/StandardCurveSymbolFactory.h"
#include "backend/worksheet/symbols/PathCurveSymbol.h"
#ifndef QT_STATICPLUGIN
#define QT_STATICPLUGIN
#endif
#include <QtGlobal>

/**
 * \class StandardCurveSymbolFactory
 * \brief Factory of built-in curve symbols.
 *
 *  \ingroup worksheet
 */

StandardCurveSymbolFactory::~StandardCurveSymbolFactory(){
	qDeleteAll(m_prototypes);
}

/*!
  creates the built-in symbols. The size of the symbol is 1.0, the point (0,0) corresponds to the center of the symbol
  (s.a. the implementation of \sa PathCurveSymbol ).
*/
void StandardCurveSymbolFactory::init(){
	QPainterPath path;
	PathCurveSymbol *symbol = NULL;
	QPolygonF polygon;

	symbol = new PathCurveSymbol("none");
	symbol->setFillingEnabled(false);
	m_prototypes.append(symbol);

	path = QPainterPath();
	path.addEllipse(QPoint(0,0), 0.5, 0.5);
	symbol = new PathCurveSymbol("circle");
	symbol->setPath(path);
	m_prototypes.append(symbol);

	path = QPainterPath();
	path.addRect(QRectF(- 0.5, -0.5, 1.0, 1.0));
	symbol = new PathCurveSymbol("square");
	symbol->setPath(path);
	m_prototypes.append(symbol);

	path = QPainterPath();
 	polygon.clear();
	polygon<<QPointF(-0.5, 0.5)<<QPointF(0, -0.5)<<QPointF(0.5, 0.5)<<QPointF(-0.5, 0.5);
	path.addPolygon(polygon);
	symbol = new PathCurveSymbol("equilateral triangle");
	symbol->setPath(path);
	m_prototypes.append(symbol);

	path = QPainterPath();
	polygon.clear();
	polygon<<QPointF(-0.5, -0.5)<<QPointF(0.5, 0.5)<<QPointF(-0.5, 0.5)<<QPointF(-0.5, -0.5);
	path.addPolygon(polygon);
	symbol = new PathCurveSymbol("right triangle");
	symbol->setPath(path);
	m_prototypes.append(symbol);

	path = QPainterPath();
	path.addRect(QRectF(- 0.5, -0.2, 1.0, 0.4));
	symbol = new PathCurveSymbol("bar");
	symbol->setPath(path);
	m_prototypes.append(symbol);

	path = QPainterPath();
	polygon.clear();
	polygon<<QPointF(-0.5, 0)<<QPointF(-0.3, -0.2)<<QPointF(0.3, -0.2)<<QPointF(0.5, 0)
				<<QPointF(0.3, 0.2)<<QPointF(-0.3, 0.2)<<QPointF(-0.5, 0);
	path.addPolygon(polygon);
	symbol = new PathCurveSymbol("peaked bar");
	symbol->setPath(path);
	m_prototypes.append(symbol);

	path = QPainterPath();
	polygon.clear();
	polygon<<QPointF(-0.5, 0.2)<<QPointF(-0.2, -0.2)<<QPointF(0.5, -0.2)<<QPointF(0.2, 0.2)<<QPointF(-0.5, 0.2);
	path.addPolygon(polygon);
	symbol = new PathCurveSymbol("skewed bar");
	symbol->setPath(path);
	m_prototypes.append(symbol);

	path = QPainterPath();
	polygon.clear();
	polygon<<QPointF(-0.5, 0)<<QPointF(0, -0.5)<<QPointF(0.5, 0)<<QPointF(0, 0.5)<<QPointF(-0.5, 0);
	path.addPolygon(polygon);
	symbol = new PathCurveSymbol("diamond");
	symbol->setPath(path);
	m_prototypes.append(symbol);

	path = QPainterPath();
	polygon.clear();
	polygon<<QPointF(-0.25, 0)<<QPointF(0, -0.5)<<QPointF(0.25, 0)<<QPointF(0, 0.5)<<QPointF(-0.25, 0);
	path.addPolygon(polygon);
	symbol = new PathCurveSymbol("lozenge");
	symbol->setPath(path);
	m_prototypes.append(symbol);

	path = QPainterPath();
	polygon.clear();
	polygon<<QPointF(-0.5, -0.5)<<QPointF(0.5, -0.5)<<QPointF(-0.5, 0.5)<<QPointF(0.5, 0.5)<<QPointF(-0.5, -0.5);
	path.addPolygon(polygon);
	symbol = new PathCurveSymbol("tie");
	symbol->setPath(path);
	m_prototypes.append(symbol);

	path = QPainterPath();
	polygon.clear();
	polygon<<QPointF(-0.2, -0.5)<<QPointF(0.2, -0.5)<<QPointF(-0.2, 0.5)<<QPointF(0.2, 0.5)<<QPointF(-0.2, -0.5);
	path.addPolygon(polygon);
	symbol = new PathCurveSymbol("tiny tie");
	symbol->setPath(path);
	m_prototypes.append(symbol);

	path = QPainterPath();
	polygon.clear();
	polygon<<QPointF(-0.2, -0.5)<<QPointF(0.2, -0.5)<<QPointF(0.2, -0.2)<<QPointF(0.5, -0.2)<<QPointF(0.5, 0.2)
	<<QPointF(0.2, 0.2)<<QPointF(0.2, 0.5)<<QPointF(-0.2, 0.5)<<QPointF(-0.2, 0.2)<<QPointF(-0.5, 0.2)
	<<QPointF(-0.5, -0.2)<<QPointF(-0.2, -0.2)<<QPointF(-0.2, -0.5);
	path.addPolygon(polygon);
	symbol = new PathCurveSymbol("plus");
	symbol->setPath(path);
	m_prototypes.append(symbol);

	path = QPainterPath();
	polygon.clear();
	polygon<<QPointF(-0.5, 0.5)<<QPointF(0, -0.5)<<QPointF(0.5, 0.5)<<QPointF(0, 0)<<QPointF(-0.5, 0.5);
	path.addPolygon(polygon);
	symbol = new PathCurveSymbol("boomerang");
	symbol->setPath(path);
	m_prototypes.append(symbol);

	path = QPainterPath();
	polygon.clear();
	polygon<<QPointF(-0.3, 0.5)<<QPointF(0, -0.5)<<QPointF(0.3, 0.5)<<QPointF(0, 0)<<QPointF(-0.3, 0.5);
	path.addPolygon(polygon);
	symbol = new PathCurveSymbol("small boomerang");
	symbol->setPath(path);
	m_prototypes.append(symbol);

	path = QPainterPath();
	polygon.clear();
	polygon<<QPointF(-0.5, 0)<<QPointF(-0.1, -0.1)<<QPointF(0, -0.5)<<QPointF(0.1, -0.1)<<QPointF(0.5, 0)
				<<QPointF(0.1, 0.1)<<QPointF(0, 0.5)<<QPointF(-0.1, 0.1)<<QPointF(-0.5, 0);
	path.addPolygon(polygon);
	symbol = new PathCurveSymbol("star4");
	symbol->setPath(path);
	m_prototypes.append(symbol);

	path = QPainterPath();
	polygon.clear();
	polygon<<QPointF(-0.5, 0)<<QPointF(-0.1, -0.1)<<QPointF(0, -0.5)<<QPointF(0.1, -0.1)<<QPointF(0.5, 0)
				<<QPointF(0.1, 0.1)<<QPointF(0.5, 0.5)<<QPointF(0, 0.2)<<QPointF(-0.5, 0.5)
				<<QPointF(-0.1, 0.1)<<QPointF(-0.5, 0);
	path.addPolygon(polygon);
	symbol = new PathCurveSymbol("star5");
	symbol->setPath(path);
	m_prototypes.append(symbol);

	path = QPainterPath(QPointF(0, -0.5));
	path.lineTo(0, 0.5);
	symbol = new PathCurveSymbol("line");
	symbol->setPath(path);
	symbol->setFillingEnabled(false);
	m_prototypes.append(symbol);

	path = QPainterPath(QPointF(0, -0.5));
	path.lineTo(0, 0.5);
	path.moveTo(-0.5, 0);
	path.lineTo(0.5, 0);
	symbol = new PathCurveSymbol("cross");
	symbol->setPath(path);
	symbol->setFillingEnabled(false);
	m_prototypes.append(symbol);
}

QList<const AbstractCurveSymbol *> StandardCurveSymbolFactory::prototypes(){
	if (m_prototypes.isEmpty())
		init();

	return m_prototypes;
}

QStringList StandardCurveSymbolFactory::ids(){
	if (m_prototypes.isEmpty())
		init();

	QStringList result;
	foreach (const AbstractCurveSymbol *prototype, m_prototypes)
		result.append(prototype->id());

	return result;
}

const AbstractCurveSymbol *StandardCurveSymbolFactory::prototype(const QString &id){
	if (m_prototypes.isEmpty())
		init();

	foreach (const AbstractCurveSymbol *prototype, m_prototypes) {
		if (id == prototype->id())
			return prototype;
	}
	return NULL;
}

//Q_EXPORT_PLUGIN2(scidavis_standardcurvesymbolfactory, StandardCurveSymbolFactory)
//Q_IMPORT_PLUGIN(scidavis_standardcurvesymbolfactory)
