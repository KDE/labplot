/***************************************************************************
    File                 : Axes.cpp
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

#include "Axes.h"

#include <QDebug>

#include <vtkRenderer.h>
#include <vtkCubeAxesActor.h>
#include <vtkTextProperty.h>
#include <vtkAxesActor.h>
#include <vtkProperty.h>
#include <vtkBoundingBox.h>

struct Axes::Properties {
	AxesType type;
	int fontSize;
	double width;
	QColor xLabelColor;
	QColor yLabelColor;
	QColor zLabelColor;

	Properties();
};

Axes::Properties::Properties()
	: type(AxesType_Cube)
	, fontSize(14)
	, width(10)
	, xLabelColor(Qt::red)
	, yLabelColor(Qt::green)
	, zLabelColor(Qt::blue) {
}

Axes::Axes(vtkRenderer& renderer)
	: renderer(renderer)
	, props(new Properties) {
	init();
}

Axes::~Axes() {
	hide();
}

void Axes::updateBounds() {
	if (!vtkAxes)
		return;

	if (props->type == AxesType_Cube) {
		vtkActorCollection* actors = renderer.GetActors();
		actors->InitTraversal();

		vtkBoundingBox bb;

		vtkActor* actor = 0;
		while ((actor = actors->GetNextActor()) != 0) {
			if (operator!=(actor))
				bb.AddBounds(actor->GetBounds());
		}

		vtkCubeAxesActor *axes = dynamic_cast<vtkCubeAxesActor*>(vtkAxes.GetPointer());

		double bounds[6];
		bb.GetBounds(bounds);
		axes->SetBounds(bounds);
	}
}

bool Axes::operator==(vtkProp* prop) const {
	return vtkAxes == prop;
}

bool Axes::operator!=(vtkProp* prop) const {
	return !operator==(prop);
}

void Axes::init() {
	if (vtkAxes) {
		hide();
	}

	if (props->type == AxesType_Cube) {
		vtkSmartPointer<vtkCubeAxesActor> axes = vtkSmartPointer<vtkCubeAxesActor>::New();
		axes->SetCamera(renderer.GetActiveCamera());
		axes->DrawXGridlinesOn();
		axes->DrawYGridlinesOn();
		axes->DrawZGridlinesOn();

		const double colors[][3] = {
			{props->xLabelColor.redF(), props->xLabelColor.greenF(), props->xLabelColor.blueF()},
			{props->yLabelColor.redF(), props->yLabelColor.greenF(), props->yLabelColor.blueF()},
			{props->zLabelColor.redF(), props->zLabelColor.greenF(), props->zLabelColor.blueF()}
		};

		for (int i = 0; i < 3; ++i) {
			vtkTextProperty *titleProp = axes->GetTitleTextProperty(i);
			titleProp->SetColor(colors[i][0], colors[i][1], colors[i][2]);
			titleProp->SetBold(true);
			titleProp->SetFontSize(VTK_INT_MAX);

			vtkTextProperty *labelProp = axes->GetLabelTextProperty(i);
			labelProp->SetColor(colors[i][0], colors[i][1], colors[i][2]);
			labelProp->SetBold(true);
		}

		axes->GetXAxesLinesProperty()->SetLineWidth(props->width);
		axes->GetYAxesLinesProperty()->SetLineWidth(props->width);
		axes->GetZAxesLinesProperty()->SetLineWidth(props->width);

		vtkAxes = axes;
	} else if (props->type == AxesType_Plain) {
		vtkSmartPointer<vtkAxesActor> axes = vtkSmartPointer<vtkAxesActor>::New();
		// TODO: Set properties here.

		vtkAxes = axes;
	}

	renderer.AddActor(vtkAxes);
}

void Axes::show(bool pred) {
	if (pred)
		init();
	else
		hide();
}

void Axes::hide() {
	renderer.RemoveActor(vtkAxes);
}

bool Axes::isShown() const {
	return vtkAxes;
}

void Axes::setType(AxesType type) {
	props->type = type;
}

void Axes::setFontSize(int fontSize) {
	props->fontSize = fontSize;
}

void Axes::setWidth(double width) {
	props->width = width;
}

void Axes::setXLabelColor(const QColor& color){
	props->xLabelColor = color;
}

void Axes::setYLabelColor(const QColor& color){
	props->yLabelColor = color;
}

void Axes::setZLabelColor(const QColor& color){
	props->zLabelColor = color;
}