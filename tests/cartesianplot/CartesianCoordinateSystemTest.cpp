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
    CartesianPlot plot(QStringLiteral("plot"));
    
    CartesianCoordinateSystem cSystem(&plot);
    
    const Lines lines = {QLineF(1., 2., 3., 4.)};
    
    const auto mapLogicalToSceneCopyLines = cSystem.mapLogicalToSceneCopy(lines);
    
    auto mapLogicalToSceneNoMarkGapsLines = lines;
    cSystem.mapLogicalToSceneNoMarkGaps(mapLogicalToSceneNoMarkGapsLines);
    
    QCOMPARE(mapLogicalToSceneCopyLines.size(), mapLogicalToSceneNoMarkGapsLines.size());
    for (int i=0; i < mapLogicalToSceneCopyLines.size(); i++) {
        VALUES_EQUAL(mapLogicalToSceneCopyLines.at(i), mapLogicalToSceneNoMarkGapsLines.at(i));
    }
}
