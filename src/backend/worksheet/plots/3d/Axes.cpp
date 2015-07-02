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

#include <vtkRenderer.h>
#include <vtkCubeAxesActor.h>
#include <vtkTextProperty.h>
#include <vtkAxesActor.h>
#include <vtkProperty.h>


Axes::Properties::Properties()
	: type(AxesType_Cube)
	, fontSize(14)
	, width(10)
	, xLabelColor(Qt::red)
	, yLabelColor(Qt::green)
	, zLabelColor(Qt::blue) {
}

Axes::Axes(vtkRenderer& renderer, const Properties& props)
	: renderer(renderer) {
	init(props);
}

Axes::~Axes() {
	renderer.RemoveActor(vtkAxes);
}

bool Axes::operator==(vtkProp* prop) const {
	return vtkAxes == prop;
}

bool Axes::operator!=(vtkProp* prop) const {
	return !operator==(prop);
}

void Axes::init(const Properties& props) {
	if (props.type == AxesType_Cube){
		vtkSmartPointer<vtkCubeAxesActor> axes = vtkSmartPointer<vtkCubeAxesActor>::New();
		axes->SetCamera(renderer.GetActiveCamera());
		axes->DrawXGridlinesOn();
		axes->DrawYGridlinesOn();
		axes->DrawZGridlinesOn();

		// TODO: Check that values are from 0 to 1.
		const double colors[][3] = {
			{props.xLabelColor.redF(), props.xLabelColor.greenF(), props.xLabelColor.blueF()},
			{props.yLabelColor.redF(), props.yLabelColor.greenF(), props.yLabelColor.blueF()},
			{props.zLabelColor.redF(), props.zLabelColor.greenF(), props.zLabelColor.blueF()}
		};

		for (int i = 0; i < 3; ++i) {
			vtkTextProperty *titleProp = axes->GetTitleTextProperty(i);
			titleProp->SetColor(colors[i][0], colors[i][1], colors[i][2]);
			titleProp->SetBold(true);
			titleProp->SetFontSize(props.fontSize);

			vtkTextProperty *labelProp = axes->GetLabelTextProperty(i);
			labelProp->SetColor(colors[i][0], colors[i][1], colors[i][2]);
			labelProp->SetBold(true);
			labelProp->SetFontSize(props.fontSize);
		}

		axes->GetXAxesLinesProperty()->SetLineWidth(props.width);
		axes->GetYAxesLinesProperty()->SetLineWidth(props.width);
		axes->GetZAxesLinesProperty()->SetLineWidth(props.width);

		vtkAxes = axes;
	} else if (props.type == AxesType_Plain) {
		vtkSmartPointer<vtkAxesActor> axes = vtkSmartPointer<vtkAxesActor>::New();
		// TODO: Set properties here.

		vtkAxes = axes;
	}

	renderer.AddActor(vtkAxes);
}