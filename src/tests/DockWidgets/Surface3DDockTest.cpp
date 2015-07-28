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