/*
    File                 : CartesianCoordinateSystemTest.cpp
    Project              : LabPlot
    Description          : Tests for cartesian coordinate system
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2025 Martin Marmsoler <martin.marmsoler@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "CartesianCoordinateSystemTest.cpp"
#include "backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h"

void CartesianCoordinateSystemTest::testMapLogicalToSceneLines() {
    using Dimension = CartesianCoordinateSystem::Dimension;

    CartesianPlot plot(QStringLiteral("plot"));
    
    CartesianCoordinateSystem cSystem(&plot);
    QVERIFY(cSystem.isValid());

    Range<double> range(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max());
    Range<double> sceneRangeX(0., 1.);
    Range<double> logicalRangeX(-10., 10.);
    std::unique_ptr<CartesianScale*> xScale(CartesianScale::createLinearScale(range, sceneRangeX, logicalRangeX));

    Range<double> sceneRangeY(10., 100.);
    Range<double> logicalRangeY(5., 10.);
    std::unique_ptr<CartesianScale*> yScale(CartesianScale::createLinearScale(range, sceneRangeY, logicalRangeY));

    cSystem.setScales(Dimension::X, {xScale});
    cSystem.setScales(Dimension::Y, {yScale});
    
    const Lines lines = {QLineF(0., 6., 7., 10.), QLineF(-10., 5., 8., 7.3)};
    
    const auto mapLogicalToSceneCopyLines = cSystem.mapLogicalToSceneCopy(lines);

    QCOMPARE(mapLogicalToSceneCopyLines.size(), 2);
    VALUES_EQUAL(mapLogicalToSceneCopyLines.at(0).x1(), 0.5);
    VALUES_EQUAL(mapLogicalToSceneCopyLines.at(0).y1(), 28.);
    VALUES_EQUAL(mapLogicalToSceneCopyLines.at(0).x2(), 0.85);
    VALUES_EQUAL(mapLogicalToSceneCopyLines.at(0).y2(), 100.);

    VALUES_EQUAL(mapLogicalToSceneCopyLines.at(1).x1(), 0.);
    VALUES_EQUAL(mapLogicalToSceneCopyLines.at(1).y1(), 10.);
    VALUES_EQUAL(mapLogicalToSceneCopyLines.at(1).x2(), 0.9);
    VALUES_EQUAL(mapLogicalToSceneCopyLines.at(1).y2(), 51.4); // (7.3 - logicalYRange.start())/(logicalYRange.end() - logicalYRange.start()) * (sceneRangeY.end() - sceneRangeY.start()) + sceneRangeY.start()
    
    auto mapLogicalToSceneNoMarkGapsLines = lines;
    cSystem.mapLogicalToSceneNoMarkGaps(mapLogicalToSceneNoMarkGapsLines);
    
    QCOMPARE(mapLogicalToSceneCopyLines.size(), mapLogicalToSceneNoMarkGapsLines.size());
    for (int i=0; i < mapLogicalToSceneCopyLines.size(); i++) {
        VALUES_EQUAL(mapLogicalToSceneCopyLines.at(i), mapLogicalToSceneNoMarkGapsLines.at(i));
    }
}
