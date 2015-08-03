/***************************************************************************
    File                 : Axes3DTest.cpp
    Project              : LabPlot
    Description          : Axes3D tests
    --------------------------------------------------------------------
    Copyright            : (C) 2015 Minh Ngo (minh@fedoraproject.org)

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
#include "Axes3DTest.h"
#include "backend/worksheet/plots/3d/Axes.h"
#include "backend/core/Project.h"

#include <QtTest>

#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkProperty.h>

Axes3DTest::Axes3DTest() {
	qRegisterMetaType<const AbstractAspect*>("const AbstractAspect*");
}

void Axes3DTest::test_fontSize() {
	Project project;
	vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
	Axes *axes = new Axes(renderer);
	project.addChild(axes);
	axes->setType(Axes::AxesType_Cube);
	axes->setFontSize(10);
	axes->show(true);

	vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
	renderWindow->AddRenderer(renderer);
	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	renderWindowInteractor->SetRenderWindow(renderWindow);

	renderer->SetBackground(.3, .6, .3);

	renderWindow->Render();
	// renderWindowInteractor->Start();
}

QTEST_MAIN(Axes3DTest)