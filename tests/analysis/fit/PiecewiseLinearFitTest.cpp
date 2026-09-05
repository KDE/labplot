/*
	File                 : PiecewiseLinearFitTest.cpp
	Project              : LabPlot
	Description          : Tests for piecewise linear regression
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2026 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "PiecewiseLinearFitTest.h"
#include "backend/core/Project.h"
#include "backend/core/column/Column.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/worksheet/plots/cartesian/XYPiecewiseLinearFitCurve.h"

#include <QBuffer>
#include <QXmlStreamWriter>
#include <cmath>

extern "C" {
#include "backend/nsl/nsl_changepoint.h"
}

// ##############################################################################
// #####################  Basic Functionality Tests  ############################
// ##############################################################################

void PiecewiseLinearFitTest::testTwoSegments() {
	// Generate data with one known changepoint at x=50
	// Segment 1: y = 2x + 10 (x < 50)
	// Segment 2: y = -x + 200 (x >= 50)
	const int n = 100;
	const double changepoint = 50.0;

	QVector<double> xData(n);
	QVector<double> yData(n);

	// Seed for reproducibility
	srand(42);

	for (int i = 0; i < n; ++i) {
		xData[i] = i;
		if (i < changepoint)
			yData[i] = 2.0 * i + 10.0; // Segment 1
		else
			yData[i] = -1.0 * i + 200.0; // Segment 2

		// Add small noise
		yData[i] += (rand() / (double)RAND_MAX - 0.5) * 2.0;
	}

	Column xCol(QStringLiteral("x"), AbstractColumn::ColumnMode::Double);
	xCol.replaceValues(0, xData);

	Column yCol(QStringLiteral("y"), AbstractColumn::ColumnMode::Double);
	yCol.replaceValues(0, yData);

	XYPiecewiseLinearFitCurve fitCurve(QStringLiteral("test"));
	fitCurve.setXDataColumn(&xCol);
	fitCurve.setYDataColumn(&yCol);

	auto fitData = fitCurve.fitData();
	fitData.changepointMethod = nsl_changepoint_method_binary_segmentation;
	fitData.penalty = 10.0;
	fitData.minSegmentSize = 10;
	fitData.maxChangepoints = 5;
	fitCurve.setFitData(fitData);

	fitCurve.recalculate();

	const auto& result = fitCurve.fitResult();

	// Assertions
	QCOMPARE(result.available, true);
	QCOMPARE(result.valid, true);
	QCOMPARE(result.numSegments, (size_t)2);
	QCOMPARE(result.changepoints.size(), 1);

	// Changepoint should be near 50
	QVERIFY(qAbs(result.changepoints[0] - changepoint) < 5.0);

	// Check slopes approximately correct
	double slope1 = result.segmentResults[0].paramValues[1];
	double slope2 = result.segmentResults[1].paramValues[1];

	QVERIFY(qAbs(slope1 - 2.0) < 0.2); // ~2.0
	QVERIFY(qAbs(slope2 - (-1.0)) < 0.2); // ~-1.0

	// Overall R² should be high
	QVERIFY(result.rsquare > 0.95);
}

void PiecewiseLinearFitTest::testThreeSegments() {
	// Generate data with two changepoints at x=30, x=70
	// Segment 1: y = 3x + 5 (x < 30)
	// Segment 2: y = 0.5x + 80 (30 <= x < 70)
	// Segment 3: y = -2x + 250 (x >= 70)
	const int n = 120;
	const double cp1 = 30.0;
	const double cp2 = 70.0;

	QVector<double> xData(n);
	QVector<double> yData(n);

	srand(42);

	for (int i = 0; i < n; ++i) {
		xData[i] = i;
		if (i < cp1)
			yData[i] = 3.0 * i + 5.0;
		else if (i < cp2)
			yData[i] = 0.5 * i + 80.0;
		else
			yData[i] = -2.0 * i + 250.0;

		yData[i] += (rand() / (double)RAND_MAX - 0.5) * 3.0;
	}

	Column xCol(QStringLiteral("x"), AbstractColumn::ColumnMode::Double);
	xCol.replaceValues(0, xData);

	Column yCol(QStringLiteral("y"), AbstractColumn::ColumnMode::Double);
	yCol.replaceValues(0, yData);

	XYPiecewiseLinearFitCurve fitCurve(QStringLiteral("test"));
	fitCurve.setXDataColumn(&xCol);
	fitCurve.setYDataColumn(&yCol);

	auto fitData = fitCurve.fitData();
	fitData.changepointMethod = nsl_changepoint_method_binary_segmentation;
	fitData.penalty = 15.0;
	fitData.minSegmentSize = 10;
	fitData.maxChangepoints = 5;
	fitCurve.setFitData(fitData);

	fitCurve.recalculate();

	const auto& result = fitCurve.fitResult();

	QCOMPARE(result.available, true);
	QCOMPARE(result.valid, true);

	// Should detect 2-4 segments (exact count depends on noise and penalty tuning)
	QVERIFY(result.numSegments >= 2);
	QVERIFY(result.numSegments <= 4);
	QVERIFY(result.changepoints.size() >= 1);

	// At least one changepoint should be near cp1 or cp2
	bool foundCP1 = false;
	bool foundCP2 = false;
	for (int i = 0; i < result.changepoints.size(); ++i) {
		if (qAbs(result.changepoints[i] - cp1) < 10.0)
			foundCP1 = true;
		if (qAbs(result.changepoints[i] - cp2) < 10.0)
			foundCP2 = true;
	}
	QVERIFY(foundCP1 || foundCP2); // At least one major changepoint detected

	// Overall R² should be high
	QVERIFY(result.rsquare > 0.90);
}

void PiecewiseLinearFitTest::testNoChangepoint() {
	// Pure linear data - single segment
	// y = 1.5x + 20
	const int n = 100;

	QVector<double> xData(n);
	QVector<double> yData(n);

	srand(42);

	for (int i = 0; i < n; ++i) {
		xData[i] = i;
		yData[i] = 1.5 * i + 20.0;
		yData[i] += (rand() / (double)RAND_MAX - 0.5) * 1.0; // Small noise
	}

	Column xCol(QStringLiteral("x"), AbstractColumn::ColumnMode::Double);
	xCol.replaceValues(0, xData);

	Column yCol(QStringLiteral("y"), AbstractColumn::ColumnMode::Double);
	yCol.replaceValues(0, yData);

	XYPiecewiseLinearFitCurve fitCurve(QStringLiteral("test"));
	fitCurve.setXDataColumn(&xCol);
	fitCurve.setYDataColumn(&yCol);

	auto fitData = fitCurve.fitData();
	fitData.changepointMethod = nsl_changepoint_method_binary_segmentation;
	fitData.penalty = 50.0; // High penalty - should not split
	fitData.minSegmentSize = 10;
	fitData.maxChangepoints = 5;
	fitCurve.setFitData(fitData);

	fitCurve.recalculate();

	const auto& result = fitCurve.fitResult();

	QCOMPARE(result.available, true);
	QCOMPARE(result.valid, true);
	QCOMPARE(result.numSegments, (size_t)1);
	QCOMPARE(result.changepoints.size(), 0);

	// Slope should be close to 1.5
	double slope = result.segmentResults[0].paramValues[1];
	QVERIFY(qAbs(slope - 1.5) < 0.1);

	// R² should be very high
	QVERIFY(result.rsquare > 0.98);
}

// ##############################################################################
// #####################  Algorithm Comparison Tests  ###########################
// ##############################################################################

void PiecewiseLinearFitTest::testBinarySegmentationVsPELT() {
	// Generate clean data with clear changepoints
	// Both algorithms should find the same solution
	const int n = 100;
	const double changepoint = 50.0;

	QVector<double> xData(n);
	QVector<double> yData(n);

	srand(42);

	for (int i = 0; i < n; ++i) {
		xData[i] = i;
		if (i < changepoint)
			yData[i] = 2.0 * i + 10.0;
		else
			yData[i] = -1.0 * i + 200.0;

		yData[i] += (rand() / (double)RAND_MAX - 0.5) * 1.0; // Very small noise
	}

	Column xCol(QStringLiteral("x"), AbstractColumn::ColumnMode::Double);
	xCol.replaceValues(0, xData);

	Column yCol(QStringLiteral("y"), AbstractColumn::ColumnMode::Double);
	yCol.replaceValues(0, yData);

	// Test Binary Segmentation
	XYPiecewiseLinearFitCurve fitCurveBinSeg(QStringLiteral("binseg"));
	fitCurveBinSeg.setXDataColumn(&xCol);
	fitCurveBinSeg.setYDataColumn(&yCol);

	auto fitDataBinSeg = fitCurveBinSeg.fitData();
	fitDataBinSeg.changepointMethod = nsl_changepoint_method_binary_segmentation;
	fitDataBinSeg.penalty = 10.0;
	fitDataBinSeg.minSegmentSize = 10;
	fitDataBinSeg.maxChangepoints = 5;
	fitCurveBinSeg.setFitData(fitDataBinSeg);

	fitCurveBinSeg.recalculate();

	// Test PELT
	XYPiecewiseLinearFitCurve fitCurvePELT(QStringLiteral("pelt"));
	fitCurvePELT.setXDataColumn(&xCol);
	fitCurvePELT.setYDataColumn(&yCol);

	auto fitDataPELT = fitCurvePELT.fitData();
	fitDataPELT.changepointMethod = nsl_changepoint_method_pelt;
	fitDataPELT.penalty = 10.0;
	fitDataPELT.minSegmentSize = 10;
	fitDataPELT.maxChangepoints = 5;
	fitCurvePELT.setFitData(fitDataPELT);

	fitCurvePELT.recalculate();

	const auto& resultBinSeg = fitCurveBinSeg.fitResult();
	const auto& resultPELT = fitCurvePELT.fitResult();

	// Both should succeed
	QCOMPARE(resultBinSeg.available, true);
	QCOMPARE(resultBinSeg.valid, true);
	QCOMPARE(resultPELT.available, true);
	QCOMPARE(resultPELT.valid, true);

	// Both should detect at least one changepoint (the clear one at x=50)
	QVERIFY(resultBinSeg.numSegments >= 2);
	QVERIFY(resultPELT.numSegments >= 2);

	// PELT may find more segments (it's more sensitive/optimal)
	QVERIFY(resultPELT.numSegments >= resultBinSeg.numSegments);

	// SSE should be reasonable - PELT should have equal or better (lower) SSE
	if (resultBinSeg.sse > 0 && resultPELT.sse > 0) {
		QVERIFY(resultPELT.sse <= resultBinSeg.sse * 1.1); // Within 10%
	}
}

// ##############################################################################
// #########################  Edge Cases Tests  #################################
// ##############################################################################

void PiecewiseLinearFitTest::testMinimumData() {
	// Minimum data: exactly 2 * minSegmentSize points
	const int n = 20;

	QVector<double> xData(n);
	QVector<double> yData(n);

	for (int i = 0; i < n; ++i) {
		xData[i] = i;
		yData[i] = 2.0 * i + 5.0;
	}

	Column xCol(QStringLiteral("x"), AbstractColumn::ColumnMode::Double);
	xCol.replaceValues(0, xData);

	Column yCol(QStringLiteral("y"), AbstractColumn::ColumnMode::Double);
	yCol.replaceValues(0, yData);

	XYPiecewiseLinearFitCurve fitCurve(QStringLiteral("test"));
	fitCurve.setXDataColumn(&xCol);
	fitCurve.setYDataColumn(&yCol);

	auto fitData = fitCurve.fitData();
	fitData.changepointMethod = nsl_changepoint_method_binary_segmentation;
	fitData.penalty = 10.0;
	fitData.minSegmentSize = 10; // Half the data
	fitData.maxChangepoints = 5;
	fitCurve.setFitData(fitData);

	fitCurve.recalculate();

	const auto& result = fitCurve.fitResult();

	// Should succeed and fit 1 segment
	QCOMPARE(result.available, true);
	QCOMPARE(result.valid, true);
	QVERIFY(result.numSegments >= 1);
}

void PiecewiseLinearFitTest::testConstantData() {
	// All y-values identical
	const int n = 50;

	QVector<double> xData(n);
	QVector<double> yData(n);

	for (int i = 0; i < n; ++i) {
		xData[i] = i;
		yData[i] = 42.0; // Constant
	}

	Column xCol(QStringLiteral("x"), AbstractColumn::ColumnMode::Double);
	xCol.replaceValues(0, xData);

	Column yCol(QStringLiteral("y"), AbstractColumn::ColumnMode::Double);
	yCol.replaceValues(0, yData);

	XYPiecewiseLinearFitCurve fitCurve(QStringLiteral("test"));
	fitCurve.setXDataColumn(&xCol);
	fitCurve.setYDataColumn(&yCol);

	auto fitData = fitCurve.fitData();
	fitData.changepointMethod = nsl_changepoint_method_binary_segmentation;
	fitData.penalty = 10.0;
	fitData.minSegmentSize = 5;
	fitData.maxChangepoints = 5;
	fitCurve.setFitData(fitData);

	fitCurve.recalculate();

	const auto& result = fitCurve.fitResult();

	QCOMPARE(result.available, true);
	QCOMPARE(result.valid, true);

	// Should fit horizontal line (slope ~0)
	if (result.numSegments > 0) {
		double slope = result.segmentResults[0].paramValues[1];
		QVERIFY(qAbs(slope) < 1e-10);
	}
}

void PiecewiseLinearFitTest::testNaNHandling() {
	// Data with some NaN values
	const int n = 60;

	QVector<double> xData(n);
	QVector<double> yData(n);

	for (int i = 0; i < n; ++i) {
		xData[i] = i;
		yData[i] = 1.5 * i + 10.0;

		// Insert NaN at specific points
		if (i == 10 || i == 25 || i == 40)
			yData[i] = std::nan("");
	}

	Column xCol(QStringLiteral("x"), AbstractColumn::ColumnMode::Double);
	xCol.replaceValues(0, xData);

	Column yCol(QStringLiteral("y"), AbstractColumn::ColumnMode::Double);
	yCol.replaceValues(0, yData);

	XYPiecewiseLinearFitCurve fitCurve(QStringLiteral("test"));
	fitCurve.setXDataColumn(&xCol);
	fitCurve.setYDataColumn(&yCol);

	auto fitData = fitCurve.fitData();
	fitData.changepointMethod = nsl_changepoint_method_binary_segmentation;
	fitData.penalty = 50.0;
	fitData.minSegmentSize = 5;
	fitData.maxChangepoints = 5;
	fitCurve.setFitData(fitData);

	fitCurve.recalculate();

	const auto& result = fitCurve.fitResult();

	// Should succeed (NaN values filtered out)
	QCOMPARE(result.available, true);
	QCOMPARE(result.valid, true);
}

// ##############################################################################
// #######################  Penalty Parameter Tests  ############################
// ##############################################################################

void PiecewiseLinearFitTest::testLowPenalty() {
	// Low penalty should detect more changepoints
	const int n = 100;

	QVector<double> xData(n);
	QVector<double> yData(n);

	srand(42);

	for (int i = 0; i < n; ++i) {
		xData[i] = i;
		yData[i] = 1.5 * i + 10.0;
		yData[i] += (rand() / (double)RAND_MAX - 0.5) * 5.0; // Moderate noise
	}

	Column xCol(QStringLiteral("x"), AbstractColumn::ColumnMode::Double);
	xCol.replaceValues(0, xData);

	Column yCol(QStringLiteral("y"), AbstractColumn::ColumnMode::Double);
	yCol.replaceValues(0, yData);

	XYPiecewiseLinearFitCurve fitCurve(QStringLiteral("test"));
	fitCurve.setXDataColumn(&xCol);
	fitCurve.setYDataColumn(&yCol);

	auto fitData = fitCurve.fitData();
	fitData.changepointMethod = nsl_changepoint_method_binary_segmentation;
	fitData.penalty = 1.0; // Very low penalty
	fitData.minSegmentSize = 5;
	fitData.maxChangepoints = 10;
	fitCurve.setFitData(fitData);

	fitCurve.recalculate();

	const auto& result = fitCurve.fitResult();

	QCOMPARE(result.available, true);
	QCOMPARE(result.valid, true);

	// With low penalty and noise, should detect multiple segments
	// (even though true data is single linear)
	QVERIFY(result.numSegments >= 2);
}

void PiecewiseLinearFitTest::testHighPenalty() {
	// High penalty should detect fewer changepoints
	const int n = 100;
	const double changepoint = 50.0;

	QVector<double> xData(n);
	QVector<double> yData(n);

	srand(42);

	for (int i = 0; i < n; ++i) {
		xData[i] = i;
		if (i < changepoint)
			yData[i] = 2.0 * i + 10.0;
		else
			yData[i] = -1.0 * i + 200.0;

		yData[i] += (rand() / (double)RAND_MAX - 0.5) * 2.0;
	}

	Column xCol(QStringLiteral("x"), AbstractColumn::ColumnMode::Double);
	xCol.replaceValues(0, xData);

	Column yCol(QStringLiteral("y"), AbstractColumn::ColumnMode::Double);
	yCol.replaceValues(0, yData);

	XYPiecewiseLinearFitCurve fitCurve(QStringLiteral("test"));
	fitCurve.setXDataColumn(&xCol);
	fitCurve.setYDataColumn(&yCol);

	auto fitData = fitCurve.fitData();
	fitData.changepointMethod = nsl_changepoint_method_binary_segmentation;
	fitData.penalty = 500.0; // Very high penalty
	fitData.minSegmentSize = 5;
	fitData.maxChangepoints = 5;
	fitCurve.setFitData(fitData);

	fitCurve.recalculate();

	const auto& result = fitCurve.fitResult();

	QCOMPARE(result.available, true);
	QCOMPARE(result.valid, true);

	// With very high penalty, might fit only 1 segment despite clear changepoint
	QVERIFY(result.numSegments <= 2);
}

// ##############################################################################
// #######################  Connection Type Tests  ##############################
// ##############################################################################

void PiecewiseLinearFitTest::testContinuousFit() {
	// Continuous fit - segments must meet at changepoints
	const int n = 100;
	const double changepoint = 50.0;

	QVector<double> xData(n);
	QVector<double> yData(n);

	srand(42);

	for (int i = 0; i < n; ++i) {
		xData[i] = i;
		if (i < changepoint)
			yData[i] = 2.0 * i + 10.0;
		else
			yData[i] = -1.0 * i + 200.0;

		yData[i] += (rand() / (double)RAND_MAX - 0.5) * 1.0;
	}

	Column xCol(QStringLiteral("x"), AbstractColumn::ColumnMode::Double);
	xCol.replaceValues(0, xData);

	Column yCol(QStringLiteral("y"), AbstractColumn::ColumnMode::Double);
	yCol.replaceValues(0, yData);

	XYPiecewiseLinearFitCurve fitCurve(QStringLiteral("test"));
	fitCurve.setXDataColumn(&xCol);
	fitCurve.setYDataColumn(&yCol);

	auto fitData = fitCurve.fitData();
	fitData.changepointMethod = nsl_changepoint_method_binary_segmentation;
	fitData.connectionType = XYPiecewiseLinearFitCurve::ConnectionType::Continuous;
	fitData.penalty = 10.0;
	fitData.minSegmentSize = 10;
	fitData.maxChangepoints = 5;
	fitCurve.setFitData(fitData);

	fitCurve.recalculate();

	const auto& result = fitCurve.fitResult();

	QCOMPARE(result.available, true);
	QCOMPARE(result.valid, true);

	// If we have a changepoint, check continuity
	if (result.numSegments > 1 && result.changepoints.size() > 0) {
		double xcp = result.changepoints[0];

		// Evaluate both segments at changepoint
		double y1 = result.segmentResults[0].paramValues[0] + result.segmentResults[0].paramValues[1] * xcp;
		double y2 = result.segmentResults[1].paramValues[0] + result.segmentResults[1].paramValues[1] * xcp;

		// Should be continuous (within numerical tolerance)
		QVERIFY(qAbs(y1 - y2) < 1e-6);
	}
}

void PiecewiseLinearFitTest::testDiscontinuousFit() {
	// Discontinuous fit - segments fitted independently
	const int n = 100;
	const double changepoint = 50.0;

	QVector<double> xData(n);
	QVector<double> yData(n);

	srand(42);

	for (int i = 0; i < n; ++i) {
		xData[i] = i;
		if (i < changepoint)
			yData[i] = 2.0 * i + 10.0;
		else
			yData[i] = -1.0 * i + 200.0;

		yData[i] += (rand() / (double)RAND_MAX - 0.5) * 1.0;
	}

	Column xCol(QStringLiteral("x"), AbstractColumn::ColumnMode::Double);
	xCol.replaceValues(0, xData);

	Column yCol(QStringLiteral("y"), AbstractColumn::ColumnMode::Double);
	yCol.replaceValues(0, yData);

	XYPiecewiseLinearFitCurve fitCurve(QStringLiteral("test"));
	fitCurve.setXDataColumn(&xCol);
	fitCurve.setYDataColumn(&yCol);

	auto fitData = fitCurve.fitData();
	fitData.changepointMethod = nsl_changepoint_method_binary_segmentation;
	fitData.connectionType = XYPiecewiseLinearFitCurve::ConnectionType::Discontinuous;
	fitData.penalty = 10.0;
	fitData.minSegmentSize = 10;
	fitData.maxChangepoints = 5;
	fitCurve.setFitData(fitData);

	fitCurve.recalculate();

	const auto& result = fitCurve.fitResult();

	QCOMPARE(result.available, true);
	QCOMPARE(result.valid, true);

	// Discontinuous fit should succeed
	// No continuity constraint, so segments may not meet
}

// ##############################################################################
// #####################  Statistical Validation Tests  #########################
// ##############################################################################

void PiecewiseLinearFitTest::testRsquare() {
	// Test R² validity
	const int n = 100;

	QVector<double> xData(n);
	QVector<double> yData(n);

	srand(42);

	for (int i = 0; i < n; ++i) {
		xData[i] = i;
		if (i < 50)
			yData[i] = 2.0 * i + 10.0;
		else
			yData[i] = -1.0 * i + 200.0;

		yData[i] += (rand() / (double)RAND_MAX - 0.5) * 2.0;
	}

	Column xCol(QStringLiteral("x"), AbstractColumn::ColumnMode::Double);
	xCol.replaceValues(0, xData);

	Column yCol(QStringLiteral("y"), AbstractColumn::ColumnMode::Double);
	yCol.replaceValues(0, yData);

	XYPiecewiseLinearFitCurve fitCurve(QStringLiteral("test"));
	fitCurve.setXDataColumn(&xCol);
	fitCurve.setYDataColumn(&yCol);

	auto fitData = fitCurve.fitData();
	fitData.changepointMethod = nsl_changepoint_method_binary_segmentation;
	fitData.penalty = 10.0;
	fitData.minSegmentSize = 10;
	fitData.maxChangepoints = 5;
	fitCurve.setFitData(fitData);

	fitCurve.recalculate();

	const auto& result = fitCurve.fitResult();

	QCOMPARE(result.available, true);
	QCOMPARE(result.valid, true);

	// Overall R² should be between 0 and 1
	QVERIFY(result.rsquare >= 0.0);
	QVERIFY(result.rsquare <= 1.0);

	// Per-segment R² should also be valid
	for (size_t i = 0; i < result.numSegments; ++i) {
		QVERIFY(result.segmentResults[i].rsquare >= 0.0);
		QVERIFY(result.segmentResults[i].rsquare <= 1.0);
	}
}

void PiecewiseLinearFitTest::testSSE() {
	// Test SSE validity and relationship
	const int n = 100;

	QVector<double> xData(n);
	QVector<double> yData(n);

	srand(42);

	for (int i = 0; i < n; ++i) {
		xData[i] = i;
		if (i < 50)
			yData[i] = 2.0 * i + 10.0;
		else
			yData[i] = -1.0 * i + 200.0;

		yData[i] += (rand() / (double)RAND_MAX - 0.5) * 2.0;
	}

	Column xCol(QStringLiteral("x"), AbstractColumn::ColumnMode::Double);
	xCol.replaceValues(0, xData);

	Column yCol(QStringLiteral("y"), AbstractColumn::ColumnMode::Double);
	yCol.replaceValues(0, yData);

	XYPiecewiseLinearFitCurve fitCurve(QStringLiteral("test"));
	fitCurve.setXDataColumn(&xCol);
	fitCurve.setYDataColumn(&yCol);

	auto fitData = fitCurve.fitData();
	fitData.changepointMethod = nsl_changepoint_method_binary_segmentation;
	fitData.penalty = 10.0;
	fitData.minSegmentSize = 10;
	fitData.maxChangepoints = 5;
	fitCurve.setFitData(fitData);

	fitCurve.recalculate();

	const auto& result = fitCurve.fitResult();

	QCOMPARE(result.available, true);
	QCOMPARE(result.valid, true);

	// SSE should be non-negative
	QVERIFY(result.sse >= 0.0);

	// Total SSE should equal sum of segment SSEs (approximately)
	double sumSSE = 0.0;
	for (size_t i = 0; i < result.numSegments; ++i) {
		QVERIFY(result.segmentResults[i].sse >= 0.0);
		sumSSE += result.segmentResults[i].sse;
	}

	// Allow small numerical difference
	QVERIFY(qAbs(result.sse - sumSSE) < 0.01 * result.sse);
}

QTEST_MAIN(PiecewiseLinearFitTest)
