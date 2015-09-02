/***************************************************************************
    File                 : MouseInteractor.cpp
    Project              : LabPlot
    Description          : 3D plot mouse interactor
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

#include "MouseInteractor.h"
#include "Axes.h"
#include "BoundingBox.h"

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/adapted/boost_tuple.hpp>

BOOST_GEOMETRY_REGISTER_BOOST_TUPLE_CS(cs::cartesian)

#include <QDebug>
#include <QPolygon>

#include <vtkObjectFactory.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <vtkPicker.h>
#include <vtkBoundingBox.h>
#include <vtkNew.h>
#include <vtkProp.h>
#include <vtkRenderer.h>
#include <vtkCamera.h>
#include <vtkCoordinate.h>

vtkStandardNewMacro(MouseInteractor);

MouseInteractor::MouseInteractor()
	: prevHovered(0)
	, prevClicked(0)
	, axes(0) {

}

vtkProp* MouseInteractor::getPickedObject(int* pos) {
	if (pos == 0)
		pos = GetInteractor()->GetEventPosition();

	vtkNew<vtkPicker> picker;
	picker->SetTolerance(0.0);
	picker->Pick(pos[0], pos[1], 0, GetDefaultRenderer());

	return picker->GetViewProp();
}

void MouseInteractor::setAxes(Axes* axes) {
	this->axes = axes;
}

namespace {
	typedef boost::tuple<double, double> Point;
    typedef boost::geometry::model::polygon<Point> Polygon;
	Polygon convexHull(const Polygon& poly) {
		Polygon hull;
		boost::geometry::convex_hull(poly, hull);
		return hull;
	}
}

bool MouseInteractor::isAxesPicked(int* pos) {
	if (axes == 0)
		return false;

	BoundingBox bounds(axes->bounds());
	bounds.Scale(1.2, 1.2, 1.2);
	Polygon coordinates;
	const double xRange[] = {bounds.xMin(), bounds.xMax()};
	const double yRange[] = {bounds.yMin(), bounds.yMax()};
	const double zRange[] = {bounds.zMin(), bounds.zMax()};
	vtkNew<vtkCoordinate> coordinate;
	coordinate->SetCoordinateSystemToWorld();
	for (int i = 0; i < 2; ++i) {
		for (int j = 0; j < 2; ++j) {
			for (int k = 0; k < 2; ++k) {
				coordinate->SetValue(xRange[i], yRange[j], zRange[k]);
				int* display = coordinate->GetComputedDisplayValue(GetCurrentRenderer());
				boost::geometry::append(coordinates, boost::geometry::make<Point>(display[0], display[1]));
			}
		}
	}

	const Polygon& hull = convexHull(coordinates);
	bool result = boost::geometry::within(Point(pos[0], pos[1]), hull);
	return result;
}

void MouseInteractor::OnMouseMove() {
	int pos[2];
	GetInteractor()->GetEventPosition(pos);
	vtkProp* object = getPickedObject(pos);
	if (object == 0) {
		if (axes && isAxesPicked(pos)) {
			if (axes != prevHovered) {
				prevHovered = axes;
				emit broadcaster.axesHovered();
			}
			return;
		} else
			vtkInteractorStyleTrackballCamera::OnMouseMove();
	}

	if (object != prevHovered) {
		prevHovered = object;
		emit broadcaster.objectHovered(object);
	}
}

void MouseInteractor::OnLeftButtonDown() {
	int pos[2];
	GetInteractor()->GetEventPosition(pos);
	vtkProp* object = getPickedObject();
	if (object == 0) {
		if (axes && isAxesPicked(pos)) {
			if (axes != prevClicked) {
				prevClicked = axes;
				emit broadcaster.axesClicked();
			}
			return;
		} else
			vtkInteractorStyleTrackballCamera::OnLeftButtonDown();
	}
	if (object != prevClicked) {
		prevClicked = object;
		emit broadcaster.objectClicked(object);
	}
}