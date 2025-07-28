/*
	File                 : CartesianCoordinateSystemTest.cpp
	Project              : LabPlot
	Description          : Tests for cartesian coordinate system
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2025 Martin Marmsoler <martin.marmsoler@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "CartesianCoordinateSystemTest.h"
#include "backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"

#include <chrono>

void CartesianCoordinateSystemTest::testMapLogicalToSceneLines() {
	using Dimension = CartesianCoordinateSystem::Dimension;

	CartesianPlot plot(QStringLiteral("plot"));

	CartesianCoordinateSystem cSystem(&plot);

	Range<double> range(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max());
	Range<double> sceneRangeX(0., 1.);
	Range<double> logicalRangeX(-10., 10.);
	auto* xScale = CartesianScale::createLinearScale(range, sceneRangeX, logicalRangeX);

	Range<double> sceneRangeY(10., 100.);
	Range<double> logicalRangeY(5., 10.);
	auto* yScale = CartesianScale::createLinearScale(range, sceneRangeY, logicalRangeY);

	// Takes ownership of the scales!
	cSystem.setScales(Dimension::X, {xScale});
	cSystem.setScales(Dimension::Y, {yScale});

	QVERIFY(cSystem.isValid());

	const Lines lines = {QLineF(0., 6., 7., 10.), QLineF(-10., 5., 8., 7.3)};

	const auto mapLogicalToSceneCopyLines = cSystem.mapLogicalToScene(lines);

	QCOMPARE(mapLogicalToSceneCopyLines.size(), 2);
	VALUES_EQUAL(mapLogicalToSceneCopyLines.at(0).x1(), 0.5);
	VALUES_EQUAL(mapLogicalToSceneCopyLines.at(0).y1(), 28.);
	VALUES_EQUAL(mapLogicalToSceneCopyLines.at(0).x2(), 0.85);
	VALUES_EQUAL(mapLogicalToSceneCopyLines.at(0).y2(), 100.);

	VALUES_EQUAL(mapLogicalToSceneCopyLines.at(1).x1(), 0.);
	VALUES_EQUAL(mapLogicalToSceneCopyLines.at(1).y1(), 10.);
	VALUES_EQUAL(mapLogicalToSceneCopyLines.at(1).x2(), 0.9);
	VALUES_EQUAL(
		mapLogicalToSceneCopyLines.at(1).y2(),
		51.4); // (7.3 - logicalYRange.start())/(logicalYRange.end() - logicalYRange.start()) * (sceneRangeY.end() - sceneRangeY.start()) + sceneRangeY.start()

	auto mapLogicalToSceneDefaultMappingLines = lines;
	cSystem.mapLogicalToSceneDefaultMapping(mapLogicalToSceneDefaultMappingLines);

	QCOMPARE(mapLogicalToSceneCopyLines.size(), mapLogicalToSceneDefaultMappingLines.size());
	for (int i = 0; i < mapLogicalToSceneCopyLines.size(); i++) {
		VALUES_EQUAL(mapLogicalToSceneCopyLines.at(i).x1(), mapLogicalToSceneDefaultMappingLines.at(i).x1());
		VALUES_EQUAL(mapLogicalToSceneCopyLines.at(i).y1(), mapLogicalToSceneDefaultMappingLines.at(i).y1());
		VALUES_EQUAL(mapLogicalToSceneCopyLines.at(i).x2(), mapLogicalToSceneDefaultMappingLines.at(i).x2());
		VALUES_EQUAL(mapLogicalToSceneCopyLines.at(i).y2(), mapLogicalToSceneDefaultMappingLines.at(i).y2());
	}
}

void CartesianCoordinateSystemTest::testMapLogicalToSceneLinesClipping() {
	using Dimension = CartesianCoordinateSystem::Dimension;

	CartesianPlot plot(QStringLiteral("plot"));
	plot.setSymmetricPadding(true);
	plot.setHorizontalPadding(0.);
	plot.setVerticalPadding(0.);
	plot.setRightPadding(0.);
	plot.setBottomPadding(0.);
	plot.setRect(QRectF(0., 0., 1., 100.));
	CartesianCoordinateSystem cSystem(&plot);

	Range<double> range(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max());
	Range<double> sceneRangeX(-0.5, 0.5);
	Range<double> logicalRangeX(-10., 10.);
	auto* xScale = CartesianScale::createLinearScale(range, sceneRangeX, logicalRangeX);

	Range<double> sceneRangeY(-50., 50.);
	Range<double> logicalRangeY(5., 10.);
	auto* yScale = CartesianScale::createLinearScale(range, sceneRangeY, logicalRangeY);

	// Takes ownership of the scales!
	cSystem.setScales(Dimension::X, {xScale});
	cSystem.setScales(Dimension::Y, {yScale});

	QVERIFY(cSystem.isValid());

	const Lines lines = {
		QLineF(0., 6., 7., 10.),
		QLineF(0., 6., 100., 206.), // Second point outside -> clipping
		QLineF(-10., 5., 8., 7.3),
		QLineF(-100., 700., 800., 800), // Both points outside
		QLineF(-10., 5., 8., 7.3),
	};

	const auto mapLogicalToSceneCopyLines = cSystem.mapLogicalToScene(lines);

	QCOMPARE(mapLogicalToSceneCopyLines.size(), 4);
	VALUES_EQUAL(mapLogicalToSceneCopyLines.at(0).x1(), 0.0);
	VALUES_EQUAL(mapLogicalToSceneCopyLines.at(0).y1(), -30.); // (6 - logicalYStart) / (logicalYEnd - logicalYStart) * (sceneYEnd - sceneYStart) + sceneYStart
	VALUES_EQUAL(mapLogicalToSceneCopyLines.at(0).x2(), 0.35); // (7 - logicalXStart) / (logicalXEnd - logicalXStart) * (sceneXEnd - sceneXStart) + sceneXStart
	VALUES_EQUAL(mapLogicalToSceneCopyLines.at(0).y2(), 50.);

	VALUES_EQUAL(mapLogicalToSceneCopyLines.at(1).x1(), 0.0);
	VALUES_EQUAL(mapLogicalToSceneCopyLines.at(1).y1(), -30.);
	VALUES_EQUAL(mapLogicalToSceneCopyLines.at(1).x2(), 0.1); // Clipped // At this point the line leaves the datarect area (Linear equation)
	VALUES_EQUAL(mapLogicalToSceneCopyLines.at(1).y2(), 50.); // Clipped

	VALUES_EQUAL(mapLogicalToSceneCopyLines.at(2).x1(), -0.5);
	VALUES_EQUAL(mapLogicalToSceneCopyLines.at(2).y1(), -50.);
	VALUES_EQUAL(mapLogicalToSceneCopyLines.at(2).x2(), 0.4);
	VALUES_EQUAL(
		mapLogicalToSceneCopyLines.at(2).y2(),
		-4); // (7.3 - logicalYRange.start())/(logicalYRange.end() - logicalYRange.start()) * (sceneRangeY.end() - sceneRangeY.start()) + sceneRangeY.start()

	VALUES_EQUAL(mapLogicalToSceneCopyLines.at(3).x1(), -0.5);
	VALUES_EQUAL(mapLogicalToSceneCopyLines.at(3).y1(), -50.);
	VALUES_EQUAL(mapLogicalToSceneCopyLines.at(3).x2(), 0.4);
	VALUES_EQUAL(mapLogicalToSceneCopyLines.at(3).y2(), -4);

	auto mapLogicalToSceneDefaultMappingLines = lines;
	cSystem.mapLogicalToSceneDefaultMapping(mapLogicalToSceneDefaultMappingLines);

	QCOMPARE(mapLogicalToSceneCopyLines.size(), mapLogicalToSceneDefaultMappingLines.size());
	for (int i = 0; i < mapLogicalToSceneCopyLines.size(); i++) {
		VALUES_EQUAL(mapLogicalToSceneCopyLines.at(i).x1(), mapLogicalToSceneDefaultMappingLines.at(i).x1());
		VALUES_EQUAL(mapLogicalToSceneCopyLines.at(i).y1(), mapLogicalToSceneDefaultMappingLines.at(i).y1());
		VALUES_EQUAL(mapLogicalToSceneCopyLines.at(i).x2(), mapLogicalToSceneDefaultMappingLines.at(i).x2());
		VALUES_EQUAL(mapLogicalToSceneCopyLines.at(i).y2(), mapLogicalToSceneDefaultMappingLines.at(i).y2());
	}
}

void CartesianCoordinateSystemTest::testMapLogicalToSceneLinesPerformance() {
	using Dimension = CartesianCoordinateSystem::Dimension;

	const size_t NUM_ITERATIONS = 100000;

	CartesianPlot plot(QStringLiteral("plot"));
	CartesianCoordinateSystem cSystem(&plot);

	Range<double> range(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max());
	Range<double> sceneRangeX(0., 1.);
	Range<double> logicalRangeX(-10., 10.);
	auto* xScale = CartesianScale::createLinearScale(range, sceneRangeX, logicalRangeX);

	Range<double> sceneRangeY(10., 100.);
	Range<double> logicalRangeY(5., 10.);
	auto* yScale = CartesianScale::createLinearScale(range, sceneRangeY, logicalRangeY);

	// Takes ownership of the scales!
	cSystem.setScales(Dimension::X, {xScale});
	cSystem.setScales(Dimension::Y, {yScale});

	QVERIFY(cSystem.isValid());

	{
		Lines linesPerformance;
		for (size_t i = 0; i < NUM_ITERATIONS; i++)
			linesPerformance.push_back(QLineF(0., 6., 7., 10.));

		const auto start = std::chrono::high_resolution_clock::now();
		auto mapLogicalToSceneCopyLines = cSystem.mapLogicalToScene(linesPerformance);
		const auto end = std::chrono::high_resolution_clock::now();
		QCOMPARE(mapLogicalToSceneCopyLines.size(), NUM_ITERATIONS);
		for (size_t i = 0; i < NUM_ITERATIONS; i++) {
			VALUES_EQUAL(mapLogicalToSceneCopyLines.at(i).x1(), 0.5);
			VALUES_EQUAL(mapLogicalToSceneCopyLines.at(i).y1(), 28.);
			VALUES_EQUAL(mapLogicalToSceneCopyLines.at(i).x2(), 0.85);
			VALUES_EQUAL(mapLogicalToSceneCopyLines.at(i).y2(), 100.);
		}
		const auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
		std::cout << "Diff: " << diff << " ms" << std::endl;
	}

	{
		Lines linesPerformance;
		for (size_t i = 0; i < NUM_ITERATIONS; i++)
			linesPerformance.push_back(QLineF(0., 6., 7., 10.));

		const auto start = std::chrono::high_resolution_clock::now();
		cSystem.mapLogicalToSceneDefaultMapping(linesPerformance);
		const auto end = std::chrono::high_resolution_clock::now();
		QCOMPARE(linesPerformance.size(), NUM_ITERATIONS);
		for (size_t i = 0; i < NUM_ITERATIONS; i++) {
			VALUES_EQUAL(linesPerformance.at(i).x1(), 0.5);
			VALUES_EQUAL(linesPerformance.at(i).y1(), 28.);
			VALUES_EQUAL(linesPerformance.at(i).x2(), 0.85);
			VALUES_EQUAL(linesPerformance.at(i).y2(), 100.);
		}
		const auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
		std::cout << "Diff: " << diff << " ms" << std::endl;
	}
}

QTEST_MAIN(CartesianCoordinateSystemTest)
