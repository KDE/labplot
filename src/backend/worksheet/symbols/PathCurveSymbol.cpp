/***************************************************************************
    File                 : PathCurveSymbol.cpp
    Project              : LabPlot/SciDAVis
    Description          : A standard curve symbol defined by a painter path and a string id.
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs*gmx.net)
    Copyright            : (C) 2010 Alexander Semke (alexander.semke*web.de)
                           (replace * with @ in the email addresses) 
                           
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

#include "worksheet/symbols/PathCurveSymbol.h"
#include "worksheet/symbols/PathCurveSymbolPrivate.h"
#include "worksheet/AbstractWorksheetElement.h"
#include <QPainter>

/**
 * \class PathCurveSymbol
 * \brief A standard curve symbol defined by a painter path and a string id.
 *
 * The constructor takes a painter path which represents the symbol at
 * size 1.0. The shape must fit into a circle with radius 1.0.
 * This class does not implement undo commands. These should be
 * implemented by the owner of the symbol prototype.
 */

PathCurveSymbol::PathCurveSymbol(const QString &symbolId)
	: QObject(0), d_ptr(new PathCurveSymbolPrivate(symbolId)) {
}
		
PathCurveSymbol::PathCurveSymbol(PathCurveSymbolPrivate *dd)
    : QObject(0), d_ptr(dd) {
}

PathCurveSymbolPrivate::PathCurveSymbolPrivate(const QString &symbolId)
	: id(symbolId) {
	size = 1;
	aspectRatio = 1;
	rotationAngle = 0;
	fillingEnabled = true;
}

PathCurveSymbol::~PathCurveSymbol() {
}

PathCurveSymbolPrivate::~PathCurveSymbolPrivate() {
}

void PathCurveSymbol::setPath(const QPainterPath &path) {
	Q_D(PathCurveSymbol);

	d->path = path;
	Q_ASSERT((path.boundingRect().width() <= 1.0) && (path.boundingRect().height() <= 1.0));
}
		
QPainterPath PathCurveSymbol::path() const {
	Q_D(const PathCurveSymbol);

	return d->path;
}
		
QString PathCurveSymbol::id() const {
	Q_D(const PathCurveSymbol);

	return d->id;
}
		
void PathCurveSymbol::setSize(qreal size) { 
	Q_D(PathCurveSymbol);

	if (size != d->size) {
		d->size = size;
		d->boundingRect = QRectF();
	}
}

qreal PathCurveSymbol::size() const {
	Q_D(const PathCurveSymbol);

	return d->size;
}

void PathCurveSymbol::setAspectRatio(qreal aspectRatio) {
	Q_D(PathCurveSymbol);

	if (aspectRatio != d->aspectRatio) {
		d->aspectRatio = aspectRatio;
		d->boundingRect = QRectF();
	}
}

qreal PathCurveSymbol::aspectRatio() const {
	Q_D(const PathCurveSymbol);

	return d->aspectRatio;
}

void PathCurveSymbol::setRotationAngle(qreal angle) {
	Q_D(PathCurveSymbol);

	if (angle != d->rotationAngle) {
		d->rotationAngle = angle;
	}
}

qreal PathCurveSymbol::rotationAngle() const {
	Q_D(const PathCurveSymbol);

	return d->rotationAngle;
}

void PathCurveSymbol::setBrush (const QBrush &brush) {
	Q_D(PathCurveSymbol);

	d->brush = brush;
}

QBrush PathCurveSymbol::brush() const {
	Q_D(const PathCurveSymbol);

	return d->brush;
}

void PathCurveSymbol::setPen(const QPen &pen) {
	Q_D(PathCurveSymbol);
	
	d->pen = pen;

	d->pen.setCosmetic(true);
	d->boundingRect = QRectF();
}

QPen PathCurveSymbol::pen() const {
	Q_D(const PathCurveSymbol);

	return d->pen;
}
		
void PathCurveSymbol::setFillingEnabled(bool b){
	Q_D(PathCurveSymbol);

	d->fillingEnabled=b;
}

bool PathCurveSymbol::fillingEnabled() const{
	Q_D(const PathCurveSymbol);

	return d->fillingEnabled;
}

void PathCurveSymbolPrivate::cloneHelper(const PathCurveSymbolPrivate *other) {
	size = other->size;
	aspectRatio = other->aspectRatio;
	rotationAngle = other->rotationAngle;
	brush = other->brush;
	pen = other->pen;
	path = other->path;
	id = other->id;
	boundingRect = other->boundingRect;
	fillingEnabled = other->fillingEnabled;
}

void PathCurveSymbol::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
	Q_UNUSED(option)
	Q_UNUSED(widget)

	paint(painter);
}

void PathCurveSymbol::paint(QPainter *painter) {
	Q_D(PathCurveSymbol);

	painter->save();

	painter->setBrush(d->brush);
	painter->setPen(d->pen);

	qreal xSize = d->size;
	qreal ySize = d->size / d->aspectRatio;
	painter->scale(xSize, ySize);
	painter->rotate(d->rotationAngle);		

	painter->drawPath(d->path);

	painter->restore();
}

QRectF PathCurveSymbol::boundingRect() const {
	Q_D(const PathCurveSymbol);

	if (d->boundingRect.isNull()) {
		qreal rectSize = qMax(d->size, d->size / d->aspectRatio) + d->pen.widthF();
		d->boundingRect = QRectF(-rectSize/2, -rectSize/2, rectSize, rectSize);
	}
	return d->boundingRect;
}

QPainterPath PathCurveSymbol::shape() const {
	Q_D(const PathCurveSymbol);

	return AbstractWorksheetElement::shapeFromPath(d->path, d->pen);
}

AbstractCurveSymbol *PathCurveSymbol::clone() const {
	Q_D(const PathCurveSymbol);

	PathCurveSymbol *twin = new PathCurveSymbol(d->id);
	twin->d_ptr->cloneHelper(d);
	return twin;
}
