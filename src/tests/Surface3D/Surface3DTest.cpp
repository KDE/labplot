/***************************************************************************
    File                 : Surface3DTest.cpp
    Project              : LabPlot
    Description          : Surface3D tests
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
#include "Surface3DTest.h"
#include "backend/worksheet/plots/3d/Surface3D.h"
#include "backend/worksheet/plots/3d/DataHandlers.h"
#include "backend/core/Project.h"

#include <QtTest>

#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkProperty.h>

Surface3DTest::Surface3DTest() {
	qRegisterMetaType<const AbstractAspect*>("const AbstractAspect*");
}

void Surface3DTest::test_solidColorFilling() {
	Project project;
	vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
	Surface3D *surf = new Surface3D(renderer);
	project.addChild(surf);
	surf->setColorFilling(Surface3D::ColorFilling_SolidColor);
	const QColor red(Qt::red);
	const double opacity = 0.7;
	surf->setColor(red);
	surf->setOpacity(opacity);

	vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
	renderWindow->AddRenderer(renderer);
	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	renderWindowInteractor->SetRenderWindow(renderWindow);

	renderer->SetBackground(.3, .6, .3);

	renderWindow->Render();
	vtkProperty* prop = renderer->GetActors()->GetLastActor()->GetProperty();
	double rgb[3];
	prop->GetColor(rgb);
	QVERIFY(qFuzzyCompare(rgb[0], red.redF()));
	QVERIFY(qFuzzyCompare(rgb[1], red.greenF()));
	QVERIFY(qFuzzyCompare(rgb[2], red.blueF()));
	
	QVERIFY(qFuzzyCompare(prop->GetOpacity(), opacity));
}

QTEST_MAIN(Surface3DTest)