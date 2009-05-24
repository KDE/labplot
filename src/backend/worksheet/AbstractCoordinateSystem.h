/***************************************************************************
    File                 : AbstractCoordinateSystem.h
    Project              : LabPlot/SciDAVis
    Description          : Base class of all worksheet coordinate systems.
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

#ifndef ABSTRACTCOORDINATESYSTEM_H
#define ABSTRACTCOORDINATESYSTEM_H

#include "worksheet/WorksheetElementContainer.h"

class AbstractCoordinateSystem: public WorksheetElementContainer {
	Q_OBJECT

	public:
		enum MappingFlag {
			DefaultMapping = 0x00,
			SuppressPageClipping = 0x01,
		};
		Q_DECLARE_FLAGS(MappingFlags, MappingFlag)

		AbstractCoordinateSystem(const QString &name);
		virtual ~AbstractCoordinateSystem();

		virtual QList<QPointF> mapLogicalToScene(const QList<QPointF> &points, const MappingFlags &flags = DefaultMapping) const = 0;
		virtual QList<QLineF> mapLogicalToScene(const QList<QLineF> &lines, const MappingFlags &flags = DefaultMapping) const = 0;
		virtual QList<QPointF> mapSceneToLogical(const QList<QPointF> &points, const MappingFlags &flags = DefaultMapping) const = 0;

		static bool clipLineToRect(QLineF *line, const QRectF &rect);

	protected:
		AbstractCoordinateSystem(const QString &name, WorksheetElementContainerPrivate *dd);
};

#endif


