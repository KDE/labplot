/***************************************************************************
    File                 : CartesianCoordinateSystem.h
    Project              : LabPlot/SciDAVis
    Description          : Cartesian coordinate system for plots.
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs*gmx.net)
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

#ifndef CARTESIANCOORDINATESYSTEM_H
#define CARTESIANCOORDINATESYSTEM_H

#include "worksheet/AbstractCoordinateSystem.h"
#include "lib/macros.h"
#include <QObject>

class CartesianCoordinateSystem: public AbstractCoordinateSystem {
	Q_OBJECT

	public:
		CartesianCoordinateSystem(const QString &name);
		virtual ~CartesianCoordinateSystem();

		QPointF mapLogicalToScene(const QPointF &point) const;
		QPointF mapSceneToLogical(const QPointF &point) const;

		CLASS_D_ACCESSOR_DECL(QPointF, position, Position);
		BASIC_D_ACCESSOR_DECL(qreal, scaleX, ScaleX);
		BASIC_D_ACCESSOR_DECL(qreal, scaleY, ScaleY);

		class Private;
	private:
		friend class Private;
		Private * const d;
};

#endif


