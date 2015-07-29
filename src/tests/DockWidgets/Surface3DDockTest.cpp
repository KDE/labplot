/***************************************************************************
    File                 : Surface3DDockTest.cpp
    Project              : LabPlot
    Description          : Surface3DDock tests
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
#include "Surface3DDockTest.h"
#include "backend/worksheet/plots/3d/Surface3D.h"
#include "kdefrontend/dockwidgets/Surface3DDock.h"
#include "backend/core/Project.h"

#include <QtTest>

void Surface3DDockTest::test_colorFillingTypeChangedCall() {
	Project project;
	Surface3D *surf = new Surface3D;
	project.addChild(surf);
	Surface3DDock w(0);
	connect(&w, SIGNAL(elementVisibilityChanged()), SLOT(onElementVisibilityChanged()));
	visibilityCounter = 0;
	w.setSurface(surf);
	QCOMPARE(visibilityCounter, 1);

	surf = new Surface3D;
	project.addChild(surf);
	surf->setColorFilling(Surface3D::ColorFilling_ColorMap);
	surf->setVisualizationType(Surface3D::VisualizationType_Wireframe);
	visibilityCounter = 0;
	w.setSurface(surf);
	QCOMPARE(visibilityCounter, 2);
}

void Surface3DDockTest::onElementVisibilityChanged() {
	++visibilityCounter;
}

QTEST_MAIN(Surface3DDockTest)