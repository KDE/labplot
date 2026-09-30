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
	VALUES_EQUAL(mapLogicalToSceneCopyLines.at(1).y2(),
				 (7.3 - logicalRangeY.start()) / (logicalRangeY.end() - logicalRangeY.start()) * (sceneRangeY.end() - sceneRangeY.start())
					 + sceneRangeY.start());

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
	VALUES_EQUAL(mapLogicalToSceneCopyLines.at(0).y1(),
				 (6 - logicalRangeY.start()) / (logicalRangeY.end() - logicalRangeY.start()) * (sceneRangeY.end() - sceneRangeY.start()) + sceneRangeY.start());
	VALUES_EQUAL(mapLogicalToSceneCopyLines.at(0).x2(),
				 (7 - logicalRangeX.start()) / (logicalRangeX.end() - logicalRangeX.start()) * (sceneRangeX.end() - sceneRangeX.start()) + sceneRangeX.start());
	VALUES_EQUAL(mapLogicalToSceneCopyLines.at(0).y2(), 50.);

	VALUES_EQUAL(mapLogicalToSceneCopyLines.at(1).x1(), 0.0);
	VALUES_EQUAL(mapLogicalToSceneCopyLines.at(1).y1(), -30.);
	VALUES_EQUAL(mapLogicalToSceneCopyLines.at(1).x2(), 0.1); // Clipped // At this point the line leaves the datarect area (Linear equation)
	VALUES_EQUAL(mapLogicalToSceneCopyLines.at(1).y2(), 50.); // Clipped

	VALUES_EQUAL(mapLogicalToSceneCopyLines.at(2).x1(), -0.5);
	VALUES_EQUAL(mapLogicalToSceneCopyLines.at(2).y1(), -50.);
	VALUES_EQUAL(mapLogicalToSceneCopyLines.at(2).x2(), 0.4);
	VALUES_EQUAL(mapLogicalToSceneCopyLines.at(2).y2(),
				 (7.3 - logicalRangeY.start()) / (logicalRangeY.end() - logicalRangeY.start()) * (sceneRangeY.end() - sceneRangeY.start())
					 + sceneRangeY.start());

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
		INFO("Diff: " << diff << " ms");
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
		INFO("Diff: " << diff << " ms");
	}
}

void CartesianCoordinateSystemTest::testMapLogicalToScenePoints() {
	CartesianPlot plot(QStringLiteral("plot"));
	plot.setHorizontalPadding(0.);
	plot.setVerticalPadding(0.);
	plot.setRightPadding(0.);
	plot.setBottomPadding(0.);
	plot.setRect(QRectF(0., 0., 100., 100.));
	CartesianCoordinateSystem cSystem(&plot);

	const Range<double> range(-100., 100.);
	cSystem.setScales(Dimension::X, {CartesianScale::createLinearScale(range, Range<double>(0., 100.), Range<double>(0., 10.))});
	cSystem.setScales(Dimension::Y, {CartesianScale::createLinearScale(range, Range<double>(0., 100.), Range<double>(0., 10.))});

	const Points points{QPointF(2., 3.), QPointF(-6., 3.), QPointF(2., 16.)};
	const auto mapped = cSystem.mapLogicalToScene(points);
	QCOMPARE(mapped.size(), 1);
	VALUES_EQUAL(mapped.at(0).x(), 20.);
	VALUES_EQUAL(mapped.at(0).y(), 30.);

	const auto mappedWithoutClipping = cSystem.mapLogicalToScene(points, AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);
	QCOMPARE(mappedWithoutClipping.size(), 3);
	VALUES_EQUAL(mappedWithoutClipping.at(1).x(), -60.);
	VALUES_EQUAL(mappedWithoutClipping.at(2).y(), 160.);

	const auto mappedWithoutY = cSystem.mapLogicalToScene(points, AbstractCoordinateSystem::MappingFlag::SuppressPageClippingY);
	QCOMPARE(mappedWithoutY.size(), 2);
	const auto dataRectCenterY = plot.dataRect().y() + plot.dataRect().height() / 2.;
	VALUES_EQUAL(mappedWithoutY.at(0).y(), dataRectCenterY);
	VALUES_EQUAL(mappedWithoutY.at(1).y(), dataRectCenterY);

	bool visible = true;
	const auto mappedPoint = cSystem.mapLogicalToScene(QPointF(16., 3.), visible, AbstractCoordinateSystem::MappingFlag::SuppressPageClippingVisible);
	VALUES_EQUAL(mappedPoint.x(), 160.);
	VALUES_EQUAL(mappedPoint.y(), 30.);
	QCOMPARE(visible, false);
}

void CartesianCoordinateSystemTest::testMapSceneToLogicalPoints() {
	CartesianPlot plot(QStringLiteral("plot"));
	plot.setHorizontalPadding(0.);
	plot.setVerticalPadding(0.);
	plot.setRightPadding(0.);
	plot.setBottomPadding(0.);
	plot.setRect(QRectF(0., 0., 100., 100.));
	CartesianCoordinateSystem cSystem(&plot);

	const Range<double> range(-100., 100.);
	cSystem.setScales(Dimension::X, {CartesianScale::createLinearScale(range, Range<double>(0., 100.), Range<double>(0., 10.))});
	cSystem.setScales(Dimension::Y, {CartesianScale::createLinearScale(range, Range<double>(0., 100.), Range<double>(0., 10.))});

	const Points scenePoints{QPointF(20., 30.), QPointF(-100., 30.), QPointF(200., 30.)};
	const auto logicalPoints = cSystem.mapSceneToLogical(scenePoints);
	QCOMPARE(logicalPoints.size(), 1);
	VALUES_EQUAL(logicalPoints.at(0).x(), 2.);
	VALUES_EQUAL(logicalPoints.at(0).y(), 3.);

	const auto logicalPointsLimited = cSystem.mapSceneToLogical(scenePoints, AbstractCoordinateSystem::MappingFlag::Limit);
	QCOMPARE(logicalPointsLimited.size(), 3);
	VALUES_EQUAL(logicalPointsLimited.at(0).x(), 2.);
	VALUES_EQUAL(logicalPointsLimited.at(1).x(), plot.dataRect().left() / 10.);
	VALUES_EQUAL(logicalPointsLimited.at(2).x(), plot.dataRect().right() / 10.);

	const auto logicalPoint = cSystem.mapSceneToLogical(QPointF(20., 30.));
	VALUES_EQUAL(logicalPoint.x(), 2.);
	VALUES_EQUAL(logicalPoint.y(), 3.);
}

void CartesianCoordinateSystemTest::testScalePropertiesAndValidity() {
	CartesianPlot plot(QStringLiteral("plot"));
	CartesianCoordinateSystem cSystem(&plot);

	QCOMPARE(cSystem.isValid(), false);
	QCOMPARE(CartesianCoordinateSystem::dimensionToString(Dimension::X), QStringLiteral("x"));
	QCOMPARE(CartesianCoordinateSystem::dimensionToString(Dimension::Y), QStringLiteral("y"));
	QCOMPARE(cSystem.direction(Dimension::X), 1);
	QCOMPARE(cSystem.direction(Dimension::Y), 1);

	cSystem.setIndex(Dimension::X, 2);
	cSystem.setIndex(Dimension::Y, 3);
	QCOMPARE(cSystem.index(Dimension::X), 2);
	QCOMPARE(cSystem.index(Dimension::Y), 3);

	auto* xScale = CartesianScale::createLinearScale(Range<double>(-10., 10.), Range<double>(0., 1.), Range<double>(0., 10.));
	auto* yScale = CartesianScale::createLinearScale(Range<double>(-10., 10.), Range<double>(0., 1.), Range<double>(0., 10.));
	QVERIFY(cSystem.setScales(Dimension::X, {xScale}));
	QVERIFY(cSystem.setScales(Dimension::Y, {yScale}));
	QCOMPARE(cSystem.scales(Dimension::X).size(), 1);
	QCOMPARE(cSystem.scales(Dimension::Y).size(), 1);
	QCOMPARE(cSystem.isValid(), true);
	QCOMPARE(cSystem.info().isEmpty(), false);
	cSystem.setName(QStringLiteral("test"));
	QCOMPARE(cSystem.info(), QStringLiteral("test"));
}

QTEST_MAIN(CartesianCoordinateSystemTest)
