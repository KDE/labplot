/***************************************************************************
    File                 : AxesPrivate.h
    Project              : LabPlot
    Description          : 3D plot axes
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

#ifndef PLOT3D_AXESPRIVATE_H
#define PLOT3D_AXESPRIVATE_H

#include "Axes.h"
#include "Base3DPrivate.h"

#include <QColor>

class vtkCubeAxesActor;

struct AxesPrivate : public Base3DPrivate {
		Axes* const q;

		int fontSize;
		QColor xLabelColor;
		QColor yLabelColor;
		QColor zLabelColor;
		QString xLabel;
		QString yLabel;
		QString zLabel;

		AxesPrivate(const QString& name, Axes* parent);
		~AxesPrivate();

		void modifyActor(vtkRenderer* renderer, vtkActor* actor) const;

	protected:
		// Update methods
		void objectScaled(vtkActor* actor) const;
		void updateBounds(vtkActor* actor) const;
		void updateLabels(const BoundingBox& bounds, vtkCubeAxesActor* cubeAxes) const;
		void makeCube(BoundingBox& bounds) const;
};

#endif