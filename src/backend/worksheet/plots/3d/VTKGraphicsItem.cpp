/***************************************************************************
    File                 : VTKGraphicsItem.cpp
    Project              : LabPlot
    Description          : Custom QVTKGraphicsItem class
    --------------------------------------------------------------------
    Copyright            : (C) 2015 by Minh Ngo (minh@fedoraproject.org)

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

#include "VTKGraphicsItem.h"
#include "Plot3DPrivate.h"
#include <QGraphicsSceneMouseEvent>

VTKGraphicsItem::VTKGraphicsItem(QGLContext* ctx, QGraphicsItem* p)
	: QVTKGraphicsItem(ctx, p) {
}

void VTKGraphicsItem::refresh() {
	QGraphicsSceneMouseEvent pressEvent(QEvent::MouseButtonPress);
	pressEvent.setButton(Qt::MiddleButton);
	QVTKGraphicsItem::mousePressEvent(&pressEvent);
	QGraphicsSceneMouseEvent releaseEvent(QEvent::MouseButtonRelease);
	releaseEvent.setButton(Qt::MiddleButton);
	QVTKGraphicsItem::mouseReleaseEvent(&releaseEvent);
}

void VTKGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
	dynamic_cast<Plot3DPrivate*>(parentItem())->mousePressEvent(event);
	QVTKGraphicsItem::mousePressEvent(event);
}

void VTKGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
	dynamic_cast<Plot3DPrivate*>(parentItem())->mouseReleaseEvent(event);
	QVTKGraphicsItem::mouseReleaseEvent(event);
}