/*
	File                 : MultiRangeTest_macros.h
	Project              : LabPlot
	Description          : Tests for multi ranges
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2021 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-FileCopyrightText: 2021 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "MultiRangeTest.h"

#include "backend/core/Project.h"
#include "backend/core/Workbook.h"
#include "backend/lib/UndoStack.h"
#include "backend/lib/macros.h"
#include "backend/matrix/Matrix.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/Axis.h"
#include "backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/CartesianPlotPrivate.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"
#include "backend/worksheet/plots/cartesian/XYEquationCurve.h"
#include "frontend/dockwidgets/BaseDock.h"
#include "frontend/dockwidgets/XYCurveDock.h"
#include "frontend/worksheet/WorksheetView.h"

#include <QAction>
#include <QComboBox>
#include <QGraphicsSceneWheelEvent>

// ##############################################################################
// #####################  import of LabPlot projects ############################
// ##############################################################################

#define LOAD_PROJECT                                                                                                                                           \
	Project project;                                                                                                                                           \
	project.load(QFINDTESTDATA(QLatin1String("data/TestMultiRange.lml")));                                                                                     \
	/* check the project tree for the imported project */                                                                                                      \
	/* first child of the root folder */                                                                                                                       \
	auto* aspect = project.child<AbstractAspect>(0);                                                                                                           \
	QVERIFY(aspect != nullptr);                                                                                                                                \
	if (aspect)                                                                                                                                                \
		QCOMPARE(aspect->name(), QLatin1String("Arbeitsblatt"));                                                                                               \
	QVERIFY(aspect->type() == AspectType::Worksheet);                                                                                                          \
	auto w = dynamic_cast<Worksheet*>(aspect);                                                                                                                 \
	if (!w)                                                                                                                                                    \
		return;                                                                                                                                                \
                                                                                                                                                               \
	auto p1 = dynamic_cast<CartesianPlot*>(aspect->child<CartesianPlot>(0));                                                                                   \
	QVERIFY(p1 != nullptr);                                                                                                                                    \
	auto p2 = dynamic_cast<CartesianPlot*>(aspect->child<CartesianPlot>(1));                                                                                   \
	QVERIFY(p2 != nullptr);                                                                                                                                    \
	if (!p1 || !p2)                                                                                                                                            \
		return;                                                                                                                                                \
                                                                                                                                                               \
	auto* view = dynamic_cast<WorksheetView*>(w->view());                                                                                                      \
	QVERIFY(view != nullptr);                                                                                                                                  \
	view->initActions(); /* needed by SET_CARTESIAN_MOUSE_MODE() */                                                                                            \
                                                                                                                                                               \
	/* axis selected */                                                                                                                                        \
	auto sinCurve = dynamic_cast<XYCurve*>(p1->child<XYCurve>(0));                                                                                             \
	QVERIFY(sinCurve != nullptr);                                                                                                                              \
	if (!sinCurve)                                                                                                                                             \
		return;                                                                                                                                                \
	QCOMPARE(sinCurve->name(), QStringLiteral("sinCurve"));                                                                                                    \
	auto tanCurve = dynamic_cast<XYCurve*>(p1->child<XYCurve>(1));                                                                                             \
	QVERIFY(tanCurve != nullptr);                                                                                                                              \
	if (!tanCurve)                                                                                                                                             \
		return;                                                                                                                                                \
	QCOMPARE(tanCurve->name(), QStringLiteral("tanCurve"));                                                                                                    \
	auto logCurve = dynamic_cast<XYCurve*>(p1->child<XYCurve>(2));                                                                                             \
	QVERIFY(logCurve != nullptr);                                                                                                                              \
	if (!logCurve)                                                                                                                                             \
		return;                                                                                                                                                \
	QCOMPARE(logCurve->name(), QStringLiteral("logx"));                                                                                                        \
                                                                                                                                                               \
	auto cosCurve = dynamic_cast<XYCurve*>(p2->child<XYCurve>(0));                                                                                             \
	QVERIFY(cosCurve != nullptr);                                                                                                                              \
	if (!cosCurve)                                                                                                                                             \
		return;                                                                                                                                                \
	QCOMPARE(cosCurve->name(), QStringLiteral("cosCurve"));                                                                                                    \
                                                                                                                                                               \
	auto horAxisP1 = static_cast<Axis*>(p1->child<Axis>(0));                                                                                                   \
	QVERIFY(horAxisP1 != nullptr);                                                                                                                             \
	QCOMPARE(horAxisP1->orientation() == Axis::Orientation::Horizontal, true);                                                                                 \
                                                                                                                                                               \
	auto vertAxisP1 = static_cast<Axis*>(p1->child<Axis>(1));                                                                                                  \
	QVERIFY(vertAxisP1 != nullptr);                                                                                                                            \
	QCOMPARE(vertAxisP1->orientation() == Axis::Orientation::Vertical, true);                                                                                  \
                                                                                                                                                               \
	auto vertAxis2P1 = static_cast<Axis*>(p1->child<Axis>(2));                                                                                                 \
	QVERIFY(vertAxis2P1 != nullptr);                                                                                                                           \
	QCOMPARE(vertAxis2P1->orientation() == Axis::Orientation::Vertical, true);                                                                                 \
                                                                                                                                                               \
	auto vertAxis3P1 = static_cast<Axis*>(p1->child<Axis>(3));                                                                                                 \
	QVERIFY(vertAxis3P1 != nullptr);                                                                                                                           \
	QCOMPARE(vertAxis3P1->orientation() == Axis::Orientation::Vertical, true);                                                                                 \
	QCOMPARE(vertAxis3P1->name(), QStringLiteral("y-axis 1"));                                                                                                 \
                                                                                                                                                               \
	auto horAxisP2 = static_cast<Axis*>(p2->child<Axis>(0));                                                                                                   \
	QVERIFY(horAxisP2 != nullptr);                                                                                                                             \
	QCOMPARE(horAxisP2->orientation() == Axis::Orientation::Horizontal, true);

#define SET_CARTESIAN_MOUSE_MODE(mode)                                                                                                                         \
	QAction a(nullptr);                                                                                                                                        \
	a.setData(static_cast<int>(mode));                                                                                                                         \
	view->changePlotMouseMode(&a);
