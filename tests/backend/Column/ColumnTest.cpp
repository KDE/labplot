/*
	File                 : ColumnTest.cpp
	Project              : LabPlot
	Description          : Tests for Column
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-FileCopyrightText: 2022-2023 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-FileCopyrightText: 2022-2023 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ColumnTest.h"
#include "backend/core/Project.h"
#include "backend/core/column/Column.h"
#include "backend/core/column/ColumnPrivate.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/trace.h"
#include "backend/spreadsheet/Spreadsheet.h"

#include <QUndoStack>

#define SETUP_C1_C2_COLUMNS(c1Vector, c2Vector)                                                                                                                \
	auto c1 = Column(QStringLiteral("DataColumn"), Column::ColumnMode::Double);                                                                                \
	c1.replaceValues(-1, c1Vector);                                                                                                                            \
	auto c2 = Column(QStringLiteral("FormulaColumn"), Column::ColumnMode::Double);                                                                             \
	c2.replaceValues(-1, c2Vector);

#define COLUMN2_SET_FORMULA_AND_EVALUATE(formula, result)                                                                                                      \
	c2.setFormula(QStringLiteral(formula), {QStringLiteral("x")}, QVector<Column*>({&c1}), true);                                                              \
	c2.updateFormula();                                                                                                                                        \
	QCOMPARE(c2.rowCount(), 8);                                                                                                                                \
	for (int i = 0; i < c2.rowCount(); i++)                                                                                                                    \
		VALUES_EQUAL(c2.valueAt(i), result);

void ColumnTest::doubleMinimum() {
	Column c(QStringLiteral("Double column"), Column::ColumnMode::Double);
	c.setValues({-1.0, 2.0, 5.0});
	QCOMPARE(c.properties(), Column::Properties::MonotonicIncreasing);
	QCOMPARE(c.minimum(0, 2), -1.0);
	QCOMPARE(c.minimum(1, 2), 2.0);

	c.setValues({-1.0, -3.0, -4.0});
	QCOMPARE(c.properties(), Column::Properties::MonotonicDecreasing);
	QCOMPARE(c.minimum(0, 2), -4.0);
	QCOMPARE(c.minimum(1, 2), -4.0);

	c.setValues({-1.0, 2.0, -4.0});
	QCOMPARE(c.properties(), Column::Properties::NonMonotonic);
	QCOMPARE(c.minimum(0, 2), -4.0);
	QCOMPARE(c.minimum(0, 1), -1.0);
}

void ColumnTest::doubleMaximum() {
	Column c(QStringLiteral("Double column"), Column::ColumnMode::Double);
	c.setValues({-1.0, 2.0, 5.0});
	QCOMPARE(c.maximum(0, 2), 5.0);
	QCOMPARE(c.maximum(1, 1), 2.0);

	c.setValues({-1.0, -3.0, -4.0});
	QCOMPARE(c.maximum(0, 2), -1.0);
	QCOMPARE(c.maximum(1, 2), -3.0);

	c.setValues({-1.0, 2.0, -4.0});
	QCOMPARE(c.maximum(0, 2), 2.0);
	QCOMPARE(c.maximum(0, 1), 2.0);
}

void ColumnTest::integerMinimum() {
	Column c(QStringLiteral("Integer column"), Column::ColumnMode::Integer);
	c.setIntegers({-1, 2, 5});
	QCOMPARE(c.properties(), Column::Properties::MonotonicIncreasing);
	QCOMPARE(c.minimum(0, 2), -1);
	QCOMPARE(c.minimum(1, 2), 2);

	c.setIntegers({-1, -3, -4});
	QCOMPARE(c.properties(), Column::Properties::MonotonicDecreasing);
	QCOMPARE(c.minimum(0, 2), -4);
	QCOMPARE(c.minimum(1, 2), -4);

	c.setIntegers({-1, 2, -4});
	QCOMPARE(c.properties(), Column::Properties::NonMonotonic);
	QCOMPARE(c.minimum(0, 2), -4);
	QCOMPARE(c.minimum(0, 1), -1);
}

void ColumnTest::integerMaximum() {
	Column c(QStringLiteral("Integer column"), Column::ColumnMode::Integer);
	c.setIntegers({-1, 2, 5});
	QCOMPARE(c.maximum(0, 2), 5);
	QCOMPARE(c.maximum(1, 1), 2);

	c.setIntegers({-1, -3, -4});
	QCOMPARE(c.maximum(0, 2), -1);
	QCOMPARE(c.maximum(1, 2), -3);

	c.setIntegers({-1, 2, -4});
	QCOMPARE(c.maximum(0, 2), 2);
	QCOMPARE(c.maximum(0, 1), 2);
}

void ColumnTest::bigIntMinimum() {
	Column c(QStringLiteral("BigInt column"), Column::ColumnMode::BigInt);
	c.setBigInts({-1, 2, 5});
	QCOMPARE(c.properties(), Column::Properties::MonotonicIncreasing);
	QCOMPARE(c.minimum(0, 2), -1);
	QCOMPARE(c.minimum(1, 2), 2);

	c.setBigInts({-1, -3, -4});
	QCOMPARE(c.properties(), Column::Properties::MonotonicDecreasing);
	QCOMPARE(c.minimum(0, 2), -4);
	QCOMPARE(c.minimum(1, 2), -4);

	c.setBigInts({-1, 2, -4});
	QCOMPARE(c.properties(), Column::Properties::NonMonotonic);
	QCOMPARE(c.minimum(0, 2), -4);
	QCOMPARE(c.minimum(0, 1), -1);
}

void ColumnTest::bigIntMaximum() {
	Column c(QStringLiteral("BigInt column"), Column::ColumnMode::BigInt);
	c.setBigInts({-1, 2, 5});
	QCOMPARE(c.maximum(0, 2), 5);
	QCOMPARE(c.maximum(0, 1), 2);

	c.setBigInts({-1, -3, -4});
	QCOMPARE(c.maximum(0, 2), -1);
	QCOMPARE(c.maximum(1, 2), -3);

	c.setBigInts({-1, 2, -4});
	QCOMPARE(c.maximum(0, 2), 2);
	QCOMPARE(c.maximum(0, 1), 2);
}

/////////////////////////////////////////////////////

void ColumnTest::statisticsDouble() {
	Column c(QStringLiteral("Double column"), Column::ColumnMode::Double);
	c.setValues({1.0, 1.0, 2.0, 5.0});

	const auto& stats = c.statistics();

	QCOMPARE(stats.size, 4);
	QCOMPARE(stats.minimum, 1.);
	QCOMPARE(stats.maximum, 5.);
	QCOMPARE(stats.arithmeticMean, 2.25);
	QCOMPARE(stats.geometricMean, pow(10., 0.25));
	QCOMPARE(stats.harmonicMean, 40. / 27.);
	QCOMPARE(stats.contraharmonicMean, 31. / 9.);

	QCOMPARE(stats.mode, 1.);
	QCOMPARE(stats.firstQuartile, 1.);
	QCOMPARE(stats.median, 1.5);
	QCOMPARE(stats.thirdQuartile, 2.75);
	QCOMPARE(stats.iqr, 1.75);
	QCOMPARE(stats.percentile_1, 1.);
	QCOMPARE(stats.percentile_5, 1.);
	QCOMPARE(stats.percentile_10, 1.);
	QCOMPARE(stats.percentile_90, 4.1);
	QCOMPARE(stats.percentile_95, 4.55);
	QCOMPARE(stats.percentile_99, 4.91);
	QCOMPARE(stats.trimean, 1.6875);
	QCOMPARE(stats.variance, 3.58333333333);
	QCOMPARE(stats.standardDeviation, 1.8929694486);
	QCOMPARE(stats.meanDeviation, 1.375);
	QCOMPARE(stats.meanDeviationAroundMedian, 1.25);
	QCOMPARE(stats.medianDeviation, 0.5);
	QCOMPARE(stats.skewness, 0.95754916255356);
	QCOMPARE(stats.kurtosis, 2.1487290427258);
	QCOMPARE(stats.entropy, 1.5);
}
void ColumnTest::statisticsDoubleNegative() {
	Column c(QStringLiteral("Double column"), Column::ColumnMode::Double);
	c.setValues({-1.0, 0.0, 2.0, 5.0});

	const auto& stats = c.statistics();

	QCOMPARE(stats.size, 4);
	QCOMPARE(stats.minimum, -1.);
	QCOMPARE(stats.maximum, 5.);
	QCOMPARE(stats.arithmeticMean, 1.5);
	QCOMPARE(stats.geometricMean, 1.474323891188); // special case
	QCOMPARE(stats.harmonicMean, 0.);
	QCOMPARE(stats.contraharmonicMean, 5.);

	QCOMPARE(stats.mode, NAN);
	QCOMPARE(stats.firstQuartile, -.25);
	QCOMPARE(stats.median, 1.);
	QCOMPARE(stats.thirdQuartile, 2.75);
	QCOMPARE(stats.iqr, 3.);
	QCOMPARE(stats.percentile_1, -.97);
	QCOMPARE(stats.percentile_5, -.85);
	QCOMPARE(stats.percentile_10, -.7);
	QCOMPARE(stats.percentile_90, 4.1);
	QCOMPARE(stats.percentile_95, 4.55);
	QCOMPARE(stats.percentile_99, 4.91);
	QCOMPARE(stats.trimean, 1.125);
	QCOMPARE(stats.variance, 7.);
	QCOMPARE(stats.standardDeviation, 2.64575131106459);
	QCOMPARE(stats.meanDeviation, 2.);
	QCOMPARE(stats.meanDeviationAroundMedian, 2.);
	QCOMPARE(stats.medianDeviation, 1.5);
	QCOMPARE(stats.skewness, 0.49878374911084);
	QCOMPARE(stats.kurtosis, 1.7619047619048);
	QCOMPARE(stats.entropy, 2.);
}
void ColumnTest::statisticsDoubleBigNegative() {
	Column c(QStringLiteral("Double column"), Column::ColumnMode::Double);
	c.setValues({-100.0, 0.0, 2.0, 5.0});

	const auto& stats = c.statistics();

	QCOMPARE(stats.size, 4);
	QCOMPARE(stats.minimum, -100.);
	QCOMPARE(stats.maximum, 5.);
	QCOMPARE(stats.arithmeticMean, -23.25);
	QCOMPARE(stats.geometricMean, NAN); // special case
	QCOMPARE(stats.harmonicMean, 0.);
	QCOMPARE(stats.contraharmonicMean, -3343. / 31.);

	QCOMPARE(stats.mode, NAN);
	QCOMPARE(stats.firstQuartile, -25.);
	QCOMPARE(stats.median, 1.);
	QCOMPARE(stats.thirdQuartile, 2.75);
	QCOMPARE(stats.iqr, 27.75);
	QCOMPARE(stats.percentile_1, -97.);
	QCOMPARE(stats.percentile_5, -85.);
	QCOMPARE(stats.percentile_10, -70.);
	QCOMPARE(stats.percentile_90, 4.1);
	QCOMPARE(stats.percentile_95, 4.55);
	QCOMPARE(stats.percentile_99, 4.91);
	QCOMPARE(stats.trimean, -5.0625);
	QCOMPARE(stats.variance, 2622.25);
	QCOMPARE(stats.standardDeviation, 51.2079095453036);
	QCOMPARE(stats.meanDeviation, 38.375);
	QCOMPARE(stats.meanDeviationAroundMedian, 26.75);
	QCOMPARE(stats.medianDeviation, 2.5);
	QCOMPARE(stats.skewness, -1.1491083404244);
	QCOMPARE(stats.kurtosis, 2.3290867987696);
	QCOMPARE(stats.entropy, 2.);
}
void ColumnTest::statisticsDoubleZero() {
	Column c(QStringLiteral("Double column"), Column::ColumnMode::Double);
	c.setValues({1.0, 0.0, 2.0, 5.0});

	const auto& stats = c.statistics();

	QCOMPARE(stats.size, 4);
	QCOMPARE(stats.minimum, 0.);
	QCOMPARE(stats.maximum, 5.);
	QCOMPARE(stats.arithmeticMean, 2.);
	QCOMPARE(stats.geometricMean, 1.77827941003892); // special case
	QCOMPARE(stats.harmonicMean, 0.);
	QCOMPARE(stats.contraharmonicMean, 3.75);

	QCOMPARE(stats.mode, NAN);
	QCOMPARE(stats.firstQuartile, 0.75);
	QCOMPARE(stats.median, 1.5);
	QCOMPARE(stats.thirdQuartile, 2.75);
	QCOMPARE(stats.iqr, 2.);
	QCOMPARE(stats.percentile_1, 0.03);
	QCOMPARE(stats.percentile_5, 0.15);
	QCOMPARE(stats.percentile_10, 0.3);
	QCOMPARE(stats.percentile_90, 4.1);
	QCOMPARE(stats.percentile_95, 4.55);
	QCOMPARE(stats.percentile_99, 4.91);
	QCOMPARE(stats.trimean, 1.625);
	QCOMPARE(stats.variance, 4.6666666666667);
	QCOMPARE(stats.standardDeviation, 2.16024689946929);
	QCOMPARE(stats.meanDeviation, 1.5);
	QCOMPARE(stats.meanDeviationAroundMedian, 1.5);
	QCOMPARE(stats.medianDeviation, 1.);
	QCOMPARE(stats.skewness, 0.68724319348909);
	QCOMPARE(stats.kurtosis, 2.);
	QCOMPARE(stats.entropy, 2.);
}

void ColumnTest::statisticsInt() {
	Column c(QStringLiteral("Integer column"), Column::ColumnMode::Integer);
	c.setIntegers({1, 1, 2, 5});

	const auto& stats = c.statistics();

	QCOMPARE(stats.size, 4);
	QCOMPARE(stats.minimum, 1.);
	QCOMPARE(stats.maximum, 5.);
	QCOMPARE(stats.arithmeticMean, 2.25);
	QCOMPARE(stats.geometricMean, pow(10., 0.25));
	QCOMPARE(stats.harmonicMean, 40. / 27.);
	QCOMPARE(stats.contraharmonicMean, 31. / 9.);

	QCOMPARE(stats.mode, 1.);
	QCOMPARE(stats.firstQuartile, 1.);
	QCOMPARE(stats.median, 1.5);
	QCOMPARE(stats.thirdQuartile, 2.75);
	QCOMPARE(stats.iqr, 1.75);
	QCOMPARE(stats.percentile_1, 1.);
	QCOMPARE(stats.percentile_5, 1.);
	QCOMPARE(stats.percentile_10, 1.);
	QCOMPARE(stats.percentile_90, 4.1);
	QCOMPARE(stats.percentile_95, 4.55);
	QCOMPARE(stats.percentile_99, 4.91);
	QCOMPARE(stats.trimean, 1.6875);
	QCOMPARE(stats.variance, 3.58333333333);
	QCOMPARE(stats.standardDeviation, 1.8929694486);
	QCOMPARE(stats.meanDeviation, 1.375);
	QCOMPARE(stats.meanDeviationAroundMedian, 1.25);
	QCOMPARE(stats.medianDeviation, 0.5);
	QCOMPARE(stats.skewness, 0.95754916255356);
	QCOMPARE(stats.kurtosis, 2.1487290427258);
	QCOMPARE(stats.entropy, 1.5);
}
void ColumnTest::statisticsIntNegative() {
	Column c(QStringLiteral("Integer column"), Column::ColumnMode::Integer);
	c.setIntegers({-1, 0, 2, 5});

	const auto& stats = c.statistics();

	QCOMPARE(stats.size, 4);
	QCOMPARE(stats.minimum, -1.);
	QCOMPARE(stats.maximum, 5.);
	QCOMPARE(stats.arithmeticMean, 1.5);
	QCOMPARE(stats.geometricMean, 1.474323891188); // special case
	QCOMPARE(stats.harmonicMean, 0.);
	QCOMPARE(stats.contraharmonicMean, 5.);

	QCOMPARE(stats.mode, NAN);
	QCOMPARE(stats.firstQuartile, -.25);
	QCOMPARE(stats.median, 1.);
	QCOMPARE(stats.thirdQuartile, 2.75);
	QCOMPARE(stats.iqr, 3.);
	QCOMPARE(stats.percentile_1, -.97);
	QCOMPARE(stats.percentile_5, -.85);
	QCOMPARE(stats.percentile_10, -.7);
	QCOMPARE(stats.percentile_90, 4.1);
	QCOMPARE(stats.percentile_95, 4.55);
	QCOMPARE(stats.percentile_99, 4.91);
	QCOMPARE(stats.trimean, 1.125);
	QCOMPARE(stats.variance, 7.);
	QCOMPARE(stats.standardDeviation, 2.64575131106459);
	QCOMPARE(stats.meanDeviation, 2.);
	QCOMPARE(stats.meanDeviationAroundMedian, 2.);
	QCOMPARE(stats.medianDeviation, 1.5);
	QCOMPARE(stats.skewness, 0.49878374911084);
	QCOMPARE(stats.kurtosis, 1.7619047619048);
	QCOMPARE(stats.entropy, 2.);
}
void ColumnTest::statisticsIntBigNegative() {
	Column c(QStringLiteral("Integer column"), Column::ColumnMode::Integer);
	c.setIntegers({-100, 0, 2, 5});

	const auto& stats = c.statistics();

	QCOMPARE(stats.size, 4);
	QCOMPARE(stats.minimum, -100.);
	QCOMPARE(stats.maximum, 5.);
	QCOMPARE(stats.arithmeticMean, -23.25);
	QCOMPARE(stats.geometricMean, NAN); // special case
	QCOMPARE(stats.harmonicMean, 0.);
	QCOMPARE(stats.contraharmonicMean, -3343. / 31.);

	QCOMPARE(stats.mode, NAN);
	QCOMPARE(stats.firstQuartile, -25.);
	QCOMPARE(stats.median, 1.);
	QCOMPARE(stats.thirdQuartile, 2.75);
	QCOMPARE(stats.iqr, 27.75);
	QCOMPARE(stats.percentile_1, -97.);
	QCOMPARE(stats.percentile_5, -85.);
	QCOMPARE(stats.percentile_10, -70.);
	QCOMPARE(stats.percentile_90, 4.1);
	QCOMPARE(stats.percentile_95, 4.55);
	QCOMPARE(stats.percentile_99, 4.91);
	QCOMPARE(stats.trimean, -5.0625);
	QCOMPARE(stats.variance, 2622.25);
	QCOMPARE(stats.standardDeviation, 51.2079095453036);
	QCOMPARE(stats.meanDeviation, 38.375);
	QCOMPARE(stats.meanDeviationAroundMedian, 26.75);
	QCOMPARE(stats.medianDeviation, 2.5);
	QCOMPARE(stats.skewness, -1.1491083404244);
	QCOMPARE(stats.kurtosis, 2.3290867987696);
	QCOMPARE(stats.entropy, 2.);
}
void ColumnTest::statisticsIntZero() {
	Column c(QStringLiteral("Integer column"), Column::ColumnMode::Integer);
	c.setIntegers({1, 0, 2, 5});

	const auto& stats = c.statistics();

	QCOMPARE(stats.size, 4);
	QCOMPARE(stats.minimum, 0.);
	QCOMPARE(stats.maximum, 5.);
	QCOMPARE(stats.arithmeticMean, 2.);
	QCOMPARE(stats.geometricMean, 1.77827941003892); // special case
	QCOMPARE(stats.harmonicMean, 0.);
	QCOMPARE(stats.contraharmonicMean, 3.75);

	QCOMPARE(stats.mode, NAN);
	QCOMPARE(stats.firstQuartile, 0.75);
	QCOMPARE(stats.median, 1.5);
	QCOMPARE(stats.thirdQuartile, 2.75);
	QCOMPARE(stats.iqr, 2.);
	QCOMPARE(stats.percentile_1, 0.03);
	QCOMPARE(stats.percentile_5, 0.15);
	QCOMPARE(stats.percentile_10, 0.3);
	QCOMPARE(stats.percentile_90, 4.1);
	QCOMPARE(stats.percentile_95, 4.55);
	QCOMPARE(stats.percentile_99, 4.91);
	QCOMPARE(stats.trimean, 1.625);
	QCOMPARE(stats.variance, 4.6666666666667);
	QCOMPARE(stats.standardDeviation, 2.16024689946929);
	QCOMPARE(stats.meanDeviation, 1.5);
	QCOMPARE(stats.meanDeviationAroundMedian, 1.5);
	QCOMPARE(stats.medianDeviation, 1.);
	QCOMPARE(stats.skewness, 0.68724319348909);
	QCOMPARE(stats.kurtosis, 2.);
	QCOMPARE(stats.entropy, 2.);
}
void ColumnTest::statisticsIntOverflow() {
	Column c(QStringLiteral("Integer column"), Column::ColumnMode::Integer);
	c.setIntegers({1000000000, 1100000000, 1200000000, 1300000000});

	const auto& stats = c.statistics();

	QCOMPARE(stats.size, 4);
	QCOMPARE(stats.minimum, 1000000000);
	QCOMPARE(stats.maximum, 1300000000);
	QCOMPARE(stats.arithmeticMean, 1150000000);
	QCOMPARE(stats.geometricMean, 1144535640.12);
	QCOMPARE(stats.harmonicMean, 1139064055.75838);
	QCOMPARE(stats.contraharmonicMean, 1160869565.21739);

	QCOMPARE(stats.mode, NAN);
	QCOMPARE(stats.firstQuartile, 1075000000);
	QCOMPARE(stats.median, 1150000000);
	QCOMPARE(stats.thirdQuartile, 1225000000);
	QCOMPARE(stats.iqr, 150000000);
	QCOMPARE(stats.percentile_1, 1003000000);
	QCOMPARE(stats.percentile_5, 1015000000);
	QCOMPARE(stats.percentile_10, 1030000000);
	QCOMPARE(stats.percentile_90, 1270000000);
	QCOMPARE(stats.percentile_95, 1285000000);
	QCOMPARE(stats.percentile_99, 1297000000);
	QCOMPARE(stats.trimean, 1150000000);
	QCOMPARE(stats.variance, 1.66666666666667e+16);
	QCOMPARE(stats.standardDeviation, 129099444.873581);
	QCOMPARE(stats.meanDeviation, 100000000);
	QCOMPARE(stats.meanDeviationAroundMedian, 100000000);
	QCOMPARE(stats.medianDeviation, 100000000);
	QCOMPARE(stats.skewness, 0.);
	QCOMPARE(stats.kurtosis, 1.64);
	QCOMPARE(stats.entropy, 2.);
}
void ColumnTest::statisticsBigInt() {
	Column c(QStringLiteral("BigInt column"), Column::ColumnMode::BigInt);
	c.setBigInts({-10000000000, 0, 1000000000, 10000000000});

	const auto& stats = c.statistics();

	QCOMPARE(stats.size, 4);
	QCOMPARE(stats.minimum, -10000000000);
	QCOMPARE(stats.maximum, 10000000000);
	QCOMPARE(stats.arithmeticMean, 250000000);
	QCOMPARE(stats.geometricMean, NAN);
	QCOMPARE(stats.harmonicMean, 0.);
	QCOMPARE(stats.contraharmonicMean, 201000000000);

	QCOMPARE(stats.mode, NAN);
// Windows CI fails here
#ifndef HAVE_WINDOWS
	QCOMPARE(stats.firstQuartile, -2500000000);
	QCOMPARE(stats.median, 500000000);
	QCOMPARE(stats.thirdQuartile, 3250000000);
	QCOMPARE(stats.iqr, 5750000000);
	QCOMPARE(stats.percentile_1, -9700000000);
	QCOMPARE(stats.percentile_5, -8500000000);
	QCOMPARE(stats.percentile_10, -7000000000);
	FuzzyCompare(stats.percentile_90, 7300000000.);
	FuzzyCompare(stats.percentile_95, 8650000000.);
	FuzzyCompare(stats.percentile_99, 9730000000.);
	QCOMPARE(stats.trimean, 437500000);
	QCOMPARE(stats.variance, 6.69166666666667e+19);
	QCOMPARE(stats.standardDeviation, 8180260794.53868);
	QCOMPARE(stats.meanDeviation, 5250000000);
	QCOMPARE(stats.meanDeviationAroundMedian, 5250000000);
	QCOMPARE(stats.medianDeviation, 5000000000);
	QCOMPARE(stats.skewness, -0.10520849985915);
	QCOMPARE(stats.kurtosis, 1.9925605877089);
	QCOMPARE(stats.entropy, 2.);
#endif
}

void ColumnTest::statisticsText() {
	Column c(QStringLiteral("Text column"), Column::ColumnMode::Text);
	c.setTextAt(0, QStringLiteral("yes"));
	c.setTextAt(1, QStringLiteral("no"));
	c.setTextAt(2, QStringLiteral("no"));
	c.setTextAt(3, QStringLiteral("yes"));
	c.setTextAt(4, QStringLiteral("yes"));

	const auto& stats = c.statistics();

	QCOMPARE(stats.size, 5);
	QCOMPARE(stats.unique, 2);
}

void ColumnTest::statisticsMaskValues() {
	Project project;
	auto* c = new Column(QStringLiteral("Integer column"), Column::ColumnMode::Integer);
	c->setIntegers({1, 2, 3});
	project.addChild(c);

	// check the statistics
	const auto& stats1 = c->statistics();
	QCOMPARE(stats1.size, 3);
	QCOMPARE(stats1.minimum, 1.);
	QCOMPARE(stats1.maximum, 3.);

	// mask the last value and check the statistics
	c->setMasked(2);
	const auto& stats2 = c->statistics();
	QCOMPARE(stats2.size, 2);
	QCOMPARE(stats2.minimum, 1.);
	QCOMPARE(stats2.maximum, 2.);

	// undo the masking change and check the statistics
	project.undoStack()->undo();
	const auto& stats3 = c->statistics();
	QCOMPARE(stats3.size, 3);
	QCOMPARE(stats3.minimum, 1.);
	QCOMPARE(stats3.maximum, 3.);

	// redo the masking change and check the statistics
	project.undoStack()->redo();
	const auto& stats4 = c->statistics();
	QCOMPARE(stats4.size, 2);
	QCOMPARE(stats4.minimum, 1.);
	QCOMPARE(stats4.maximum, 2.);
}

void ColumnTest::statisticsClearSpreadsheetMasks() {
	Project project;

	auto* spreadsheet = new Spreadsheet(QStringLiteral("spreadsheet"));
	project.addChild(spreadsheet);
	spreadsheet->setColumnCount(1);
	spreadsheet->setRowCount(3);
	auto* c = spreadsheet->column(0);
	c->setValues({1., 2., 3.});

	// check the statistics
	const auto& stats1 = c->statistics();
	QCOMPARE(stats1.size, 3);
	QCOMPARE(stats1.minimum, 1.);
	QCOMPARE(stats1.maximum, 3.);

	// mask the last value and check the statistics
	c->setMasked(2);
	const auto& stats2 = c->statistics();
	QCOMPARE(stats2.size, 2);
	QCOMPARE(stats2.minimum, 1.);
	QCOMPARE(stats2.maximum, 2.);

	// clear the masked values in the spreadsheet
	spreadsheet->clearMasks();
	const auto& stats3 = c->statistics();
	QCOMPARE(stats3.size, 3);
	QCOMPARE(stats3.minimum, 1.);
	QCOMPARE(stats3.maximum, 3.);

	// undo the "clear masked values"-change and check the statistics
	project.undoStack()->undo();
	const auto& stats4 = c->statistics();
	QCOMPARE(stats4.size, 2);
	QCOMPARE(stats4.minimum, 1.);
	QCOMPARE(stats4.maximum, 2.);

	// redo the "clear masked values"-change and check the statistics
	project.undoStack()->redo();
	const auto& stats5 = c->statistics();
	QCOMPARE(stats5.size, 3);
	QCOMPARE(stats5.minimum, 1.);
	QCOMPARE(stats5.maximum, 3.);
}

void ColumnTest::testFormulaAutoUpdateEnabled() {
	Column sourceColumn(QStringLiteral("source"), Column::ColumnMode::Integer);
	sourceColumn.setIntegers({1, 2, 3});

	Column targetColumn(QStringLiteral("target"), Column::ColumnMode::Integer);
	targetColumn.setIntegers({3, 2, 1});

	// evaluatue 2*x and check the generated values in the target column
	targetColumn.setColumnMode(AbstractColumn::ColumnMode::Double); // should happen automatically in Column::setFormula()
	targetColumn.setFormula(QStringLiteral("2*x"),
							QStringList{QStringLiteral("x")},
							QVector<Column*>({&sourceColumn}),
							true /* autoUpdate */,
							false /* autoResize */);
	targetColumn.updateFormula();
	QCOMPARE(targetColumn.rowCount(), 3);
	QCOMPARE(targetColumn.valueAt(0), 2);
	QCOMPARE(targetColumn.valueAt(1), 4);
	QCOMPARE(targetColumn.valueAt(2), 6);

	// modify value in the source column and check the target column again which should be updated
	sourceColumn.setIntegers({3, 2, 1});
	QCOMPARE(targetColumn.rowCount(), 3);
	QCOMPARE(targetColumn.valueAt(0), 6);
	QCOMPARE(targetColumn.valueAt(1), 4);
	QCOMPARE(targetColumn.valueAt(2), 2);
}

void ColumnTest::testFormulaAutoUpdateDisabled() {
	Column sourceColumn(QStringLiteral("source"), Column::ColumnMode::Integer);
	sourceColumn.setIntegers({1, 2, 3});

	Column targetColumn(QStringLiteral("target"), Column::ColumnMode::Integer);
	targetColumn.setIntegers({3, 2, 1});

	// evaluatue 2*x and check the generated values in the target column
	targetColumn.setColumnMode(AbstractColumn::ColumnMode::Double);
	targetColumn.setFormula(QStringLiteral("2*x"),
							QStringList{QStringLiteral("x")},
							QVector<Column*>({&sourceColumn}),
							false /* autoUpdate */,
							false /* autoResize */);
	targetColumn.updateFormula();
	QCOMPARE(targetColumn.rowCount(), 3);
	QCOMPARE(targetColumn.valueAt(0), 2);
	QCOMPARE(targetColumn.valueAt(1), 4);
	QCOMPARE(targetColumn.valueAt(2), 6);

	// modify value in the source column and check the target column again which shouldn't be updated
	sourceColumn.setIntegers({3, 2, 1});
	QCOMPARE(targetColumn.rowCount(), 3);
	QCOMPARE(targetColumn.valueAt(0), 2);
	QCOMPARE(targetColumn.valueAt(1), 4);
	QCOMPARE(targetColumn.valueAt(2), 6);
}

void ColumnTest::testFormulaAutoResizeEnabled() {
	Column sourceColumn1(QStringLiteral("source1"), Column::ColumnMode::Integer);
	sourceColumn1.setIntegers({1, 2, 3});

	Column sourceColumn2(QStringLiteral("source2"), Column::ColumnMode::Integer);
	sourceColumn2.setIntegers({1, 2, 3});

	// spreadsheet needs to be created since the resize of the column is happening via the resize of the spreadsheet
	Spreadsheet targetSpreadsheet(QStringLiteral("target"));
	targetSpreadsheet.setColumnCount(1);
	targetSpreadsheet.setRowCount(1);
	Column* targetColumn = targetSpreadsheet.column(0);

	// evaluatue x+y
	targetColumn->setColumnMode(AbstractColumn::ColumnMode::Double);
	targetColumn->setFormula(QStringLiteral("x+y"),
							 QStringList{QStringLiteral("x"), QStringLiteral("y")},
							 QVector<Column*>({&sourceColumn1, &sourceColumn2}),
							 false /* autoUpdate */,
							 true /* autoResize */);
	targetColumn->updateFormula();

	// check the generated values in the target column which should have been resized
	QCOMPARE(targetColumn->rowCount(), 3);
	QCOMPARE(targetColumn->valueAt(0), 2);
	QCOMPARE(targetColumn->valueAt(1), 4);
	QCOMPARE(targetColumn->valueAt(2), 6);
}

void ColumnTest::testFormulaAutoResizeDisabled() {
	Column sourceColumn1(QStringLiteral("source1"), Column::ColumnMode::Integer);
	sourceColumn1.setIntegers({1, 2, 3});

	Column sourceColumn2(QStringLiteral("source2"), Column::ColumnMode::Integer);
	sourceColumn2.setIntegers({1, 2, 3});

	Column targetColumn(QStringLiteral("target"), Column::ColumnMode::Integer);
	targetColumn.setIntegers({1});

	// evaluatue x+y
	targetColumn.setColumnMode(AbstractColumn::ColumnMode::Double);
	targetColumn.setFormula(QStringLiteral("x+y"),
							QStringList{QStringLiteral("x"), QStringLiteral("y")},
							QVector<Column*>({&sourceColumn1, &sourceColumn2}),
							false /* autoUpdate */,
							false /* autoResize */);
	targetColumn.updateFormula();

	// check the generated values in the target column which should not have been resized
	QCOMPARE(targetColumn.rowCount(), 1);
	QCOMPARE(targetColumn.valueAt(0), 2);
}

void ColumnTest::testFormulaAutoUpdateEnabledResize() {
	Column sourceColumn(QStringLiteral("source"), Column::ColumnMode::Integer);
	sourceColumn.setIntegers({1, 2, 3, 4});

	Column targetColumn(QStringLiteral("target"), Column::ColumnMode::Integer);
	targetColumn.setIntegers({3, 2, 1});

	// evaluatue 2*x and check the generated values in the target column
	targetColumn.setColumnMode(AbstractColumn::ColumnMode::Double); // should happen automatically in Column::setFormula()
	targetColumn.setFormula(QStringLiteral("2*x"),
							QStringList{QStringLiteral("x")},
							QVector<Column*>({&sourceColumn}),
							true /* autoUpdate */,
							false /* autoResize */);
	targetColumn.updateFormula();
	QCOMPARE(targetColumn.rowCount(), 3);
	QCOMPARE(targetColumn.valueAt(0), 2);
	QCOMPARE(targetColumn.valueAt(1), 4);
	QCOMPARE(targetColumn.valueAt(2), 6);

	targetColumn.insertRows(3, 1); // Rowcount of the target changes

	// Values updated
	QCOMPARE(targetColumn.rowCount(), 4);
	QCOMPARE(targetColumn.valueAt(0), 2);
	QCOMPARE(targetColumn.valueAt(1), 4);
	QCOMPARE(targetColumn.valueAt(2), 6);
	QCOMPARE(targetColumn.valueAt(3), 8);
}

void ColumnTest::testDictionaryIndex() {
	Column c(QStringLiteral("Text column"), Column::ColumnMode::Text);
	c.setTextAt(0, QStringLiteral("yes"));
	c.setTextAt(1, QStringLiteral("no"));
	c.setTextAt(2, QStringLiteral("no"));
	c.setTextAt(3, QStringLiteral("yes"));
	c.setTextAt(4, QStringLiteral("yes"));

	// check the position of the distinct values in the dictionary
	QCOMPARE(c.dictionaryIndex(0), 0);
	QCOMPARE(c.dictionaryIndex(1), 1);
	QCOMPARE(c.dictionaryIndex(2), 1);
	QCOMPARE(c.dictionaryIndex(3), 0);
	QCOMPARE(c.dictionaryIndex(4), 0);

	// modify a value which will invalidate the dictionary and verify it again
	c.setTextAt(1, QStringLiteral("yes"));

	QCOMPARE(c.dictionaryIndex(0), 0);
	QCOMPARE(c.dictionaryIndex(1), 0);
	QCOMPARE(c.dictionaryIndex(2), 1);
	QCOMPARE(c.dictionaryIndex(3), 0);
	QCOMPARE(c.dictionaryIndex(4), 0);
}

void ColumnTest::testTextFrequencies() {
	Column c(QStringLiteral("Text column"), Column::ColumnMode::Text);
	c.setTextAt(0, QStringLiteral("yes"));
	c.setTextAt(1, QStringLiteral("no"));
	c.setTextAt(2, QStringLiteral("no"));
	c.setTextAt(3, QStringLiteral("yes"));
	c.setTextAt(4, QStringLiteral("yes"));

	const auto& frequencies = c.frequencies();

	QCOMPARE(frequencies[QStringLiteral("yes")], 3);
	QCOMPARE(frequencies[QStringLiteral("no")], 2);
}

//////////////////////////////////////////////////

void ColumnTest::saveLoadDateTime() {
	Column c(QStringLiteral("Datetime column"), Column::ColumnMode::DateTime);
	c.setDateTimes({
		QDateTime::fromString(
			QStringLiteral("2017-03-26T02:14:34.000Z"),
			Qt::DateFormat::ISODateWithMs), // without the timezone declaration it would be invalid (in some regions), because of the daylight time
		QDateTime::fromString(QStringLiteral("2018-03-26T02:14:34.000Z"), Qt::DateFormat::ISODateWithMs),
		QDateTime::fromString(QStringLiteral("2019-03-26T02:14:34.000Z"), Qt::DateFormat::ISODateWithMs),
		QDateTime::fromString(QStringLiteral("2019-26-03 02:14:34:000"), QStringLiteral("yyyy-dd-MM hh:mm:ss:zzz")),
	});

	QByteArray array;
	QXmlStreamWriter writer(&array);
	c.save(&writer);

	QDEBUG(array);

	Column c2(QStringLiteral("Datetime 2 column"), Column::ColumnMode::DateTime);
	XmlStreamReader reader(array);
	bool found = false;
	while (!reader.atEnd()) {
		reader.readNext();
		if (reader.isStartElement() && reader.name() == QLatin1String("column")) {
			found = true;
			break;
		}
	}
	QCOMPARE(found, true);
	QCOMPARE(c2.load(&reader, false), true);

	// wait until all columns are decoded from base64-encoded data
	QThreadPool::globalInstance()->waitForDone();

	QCOMPARE(c2.rowCount(), 4);
	QCOMPARE(c2.dateTimeAt(0).isValid(), true);
	QCOMPARE(c2.dateTimeAt(0), QDateTime::fromString(QStringLiteral("2017-03-26T02:14:34.000Z"), Qt::DateFormat::ISODateWithMs));
	QCOMPARE(c2.dateTimeAt(1).isValid(), true);
	QCOMPARE(c2.dateTimeAt(1), QDateTime::fromString(QStringLiteral("2018-03-26T02:14:34.000Z"), Qt::DateFormat::ISODateWithMs));
	QCOMPARE(c2.dateTimeAt(2).isValid(), true);
	QCOMPARE(c2.dateTimeAt(2), QDateTime::fromString(QStringLiteral("2019-03-26T02:14:34.000Z"), Qt::DateFormat::ISODateWithMs));
	QCOMPARE(c2.dateTimeAt(3).isValid(), true);
	QCOMPARE(c2.dateTimeAt(3), QDateTime::fromString(QStringLiteral("2019-03-26T02:14:34.000Z"), Qt::DateFormat::ISODateWithMs));
}

void ColumnTest::loadDoubleFromProject() {
	Project project;
	project.load(QFINDTESTDATA(QLatin1String("data/Load.lml")));

	auto* doublespreadSheet = project.child<AbstractAspect>(0);
	QVERIFY(doublespreadSheet != nullptr);
	QCOMPARE(doublespreadSheet->name(), QLatin1String("Double"));
	QCOMPARE(doublespreadSheet->type(), AspectType::Spreadsheet);

	auto childs = doublespreadSheet->children(AspectType::Column);
	QVERIFY(childs.count() >= 1);
	auto* doubleColumn = static_cast<Column*>(childs.at(0));
	QCOMPARE(doubleColumn->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(doubleColumn->rowCount(), 100);
	const double doubleValues[] = {0.625,
								   1,
								   1,
								   4,
								   1.125,
								   1.66666666666667,
								   11,
								   6,
								   2.16666666666667,
								   14,
								   1.5,
								   2.28571428571429,
								   2.125,
								   2,
								   1.9,
								   2,
								   3.5,
								   22,
								   2.3,
								   2.66666666666667,
								   5,
								   26,
								   3,
								   4,
								   4.14285714285714,
								   3.33333333333333,
								   10.3333333333333,
								   32,
								   8.25,
								   3.4,
								   8.75,
								   5.14285714285714,
								   9.25,
								   6.33333333333333,
								   4.33333333333333,
								   4.44444444444444,
								   4.55555555555556,
								   4.2,
								   6.14285714285714,
								   5.5,
								   7.5,
								   46,
								   4.7,
								   12,
								   7,
								   7.14285714285714,
								   6.375,
								   5.2,
								   26.5,
								   18,
								   27.5,
								   14,
								   11.4,
								   8.28571428571429,
								   29.5,
								   10,
								   15.25,
								   12.4,
								   12.6,
								   64,
								   65,
								   33,
								   8.375,
								   13.6,
								   17.25,
								   14,
								   23.6666666666667,
								   8,
								   8.11111111111111,
								   10.5714285714286,
								   12.5,
								   8.44444444444444,
								   19.25,
								   8.66666666666667,
								   11.2857142857143,
								   8,
								   20.25,
								   27.3333333333333,
								   20.75,
								   16.8,
								   42.5,
								   9.55555555555556,
								   9.66666666666667,
								   17.6,
								   44.5,
								   90,
								   18.2,
								   46,
								   31,
								   47,
								   23.75,
								   96,
								   12.125,
								   19.6,
								   16.5,
								   11.1111111111111,
								   33.6666666666667,
								   12.75,
								   25.75,
								   13};
	for (int i = 0; i < 100; i++)
		QCOMPARE(doubleColumn->valueAt(i), doubleValues[i]);
}

void ColumnTest::loadIntegerFromProject() {
	Project project;
	project.load(QFINDTESTDATA(QLatin1String("data/Load.lml")));

	auto* integerSpreadsheet = project.child<AbstractAspect>(1);
	QVERIFY(integerSpreadsheet != nullptr);
	QCOMPARE(integerSpreadsheet->name(), QLatin1String("Integer"));
	QCOMPARE(integerSpreadsheet->type(), AspectType::Spreadsheet);

	auto childs = integerSpreadsheet->children(AspectType::Column);
	QVERIFY(childs.count() >= 1);
	auto* integerColumn = static_cast<Column*>(childs.at(0));
	QCOMPARE(integerColumn->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(integerColumn->rowCount(), 133);
	const int integerValues[133] = {291, 75,  21,  627, 163, 677, 712, 66,	733, 653, 502, 515, 379, 70,  762, 261, 304, 541, 298, 462, 623, 382, 94,
									232, 679, 132, 124, 212, 122, 118, 486, 126, 107, 677, 386, 118, 731, 484, 638, 127, 779, 109, 708, 298, 680, 249,
									591, 155, 351, 178, 70,	 768, 2,   504, 179, 747, 789, 213, 144, 143, 61,  761, 113, 766, 18,  617, 406, 489, 299,
									658, 326, 181, 773, 228, 653, 242, 382, 11,	 267, 29,  283, 30,	 251, 453, 699, 286, 739, 406, 729, 159, 506, 20,
									766, 443, 646, 161, 545, 400, 160, 693, 722, 463, 121, 350, 194, 558, 503, 72,	516, 509, 118, 340, 342, 495, 50,
									549, 643, 241, 248, 483, 408, 768, 634, 589, 159, 518, 475, 403, 165, 122, 268, 537, 33};
	for (int i = 0; i < 133; i++)
		QCOMPARE(integerColumn->integerAt(i), integerValues[i]);
}

void ColumnTest::loadBigIntegerFromProject() {
	Project project;
	project.load(QFINDTESTDATA(QLatin1String("data/Load.lml")));

	auto* bigIntegerSpreadsheet = project.child<AbstractAspect>(2);
	QVERIFY(bigIntegerSpreadsheet != nullptr);
	QCOMPARE(bigIntegerSpreadsheet->name(), QLatin1String("BigInteger"));
	QCOMPARE(bigIntegerSpreadsheet->type(), AspectType::Spreadsheet);

	auto childs = bigIntegerSpreadsheet->children(AspectType::Column);
	QVERIFY(childs.count() >= 1);
	auto* bigIntegerColumn = static_cast<Column*>(childs.at(0));
	QCOMPARE(bigIntegerColumn->columnMode(), AbstractColumn::ColumnMode::BigInt);
	QCOMPARE(bigIntegerColumn->rowCount(), 98);
	const qint64 bigIntegerValues[] = {
		423448954198, 5641410204,  30408827812, 28654888657, 49080407041, 49609860873, 3687941775,	19532027992, 35894087224, 5820636581,  28739047077,
		13946526866,  36153607843, 3240340694,	2375891120,	 3014999117,  17758738424, 31303772749, 36400461519, 29813286102, 14068980943, 24595715523,
		390096547,	  27927541822, 35442936843, 33577242277, 34966078315, 45550480998, 11834545810, 25714661808, 6979002160,  35138449350, 3597002515,
		707044300,	  27971594979, 25699976843, 35231282278, 11659858605, 45858935838, 25070072891, 15136182059, 6266852861,  42582813575, 23784333993,
		14361566136,  27840747719, 41099229867, 40403331476, 21708972571, 10995493445, 36292237893, 4264327752,	 45637575339, 13530360473, 40816873119,
		15346300490,  30807486688, 23771858665, 36762855436, 351630653,	  22270715573, 31792268673, 25001237450, 16558491573, 21771715873, 20963298299,
		25197909817,  41130528918, 13134975803, 43222173019, 17071520882, 8356069280,  27671796182, 29309739294, 9377292482,  30451803959, 47318250898,
		21100469009,  28764337224, 36898356693, 36091695104, 12019973504, 15605135996, 13711330940, 13010481591, 45193969649, 25444985954, 34831527437,
		8208098526,	  29897950771, 5631513384,	47590874807, 4659417951,  28338882094, 14853737313, 22965578753, 6544735402,  32209366817};
	for (int i = 0; i < 98; i++)
		QCOMPARE(bigIntegerColumn->bigIntAt(i), bigIntegerValues[i]);
}

void ColumnTest::loadTextFromProject() {
	Project project;
	project.load(QFINDTESTDATA(QLatin1String("data/Load.lml")));

	auto* textSpreadsheet = project.child<AbstractAspect>(3);
	QVERIFY(textSpreadsheet != nullptr);
	QCOMPARE(textSpreadsheet->name(), QLatin1String("Text"));
	QCOMPARE(textSpreadsheet->type(), AspectType::Spreadsheet);

	auto childs = textSpreadsheet->children(AspectType::Column);
	QVERIFY(childs.count() >= 1);
	auto* textColumn = static_cast<Column*>(childs.at(0));
	QCOMPARE(textColumn->columnMode(), AbstractColumn::ColumnMode::Text);
	QCOMPARE(textColumn->rowCount(), 10);
	QStringList texts = {QStringLiteral("first value"),
						 QStringLiteral("second value"),
						 QStringLiteral("third value"),
						 QStringLiteral("fourth value"),
						 QStringLiteral("fifth value"),
						 QStringLiteral("sixt value"),
						 QStringLiteral("sevent value"),
						 QStringLiteral("eigth value"),
						 QStringLiteral("ninth value"),
						 QStringLiteral("tenth value")};
	for (int i = 0; i < 10; i++) {
		QCOMPARE(textColumn->textAt(i), texts.at(i));
	}
}

void ColumnTest::loadDateTimeFromProject() {
	Project project;
	project.load(QFINDTESTDATA(QLatin1String("data/Load.lml")));

	auto* dateTimeSpreadsheet = project.child<AbstractAspect>(4);
	QVERIFY(dateTimeSpreadsheet != nullptr);
	QCOMPARE(dateTimeSpreadsheet->name(), QLatin1String("Datetime"));
	QCOMPARE(dateTimeSpreadsheet->type(), AspectType::Spreadsheet);

	auto childs = dateTimeSpreadsheet->children(AspectType::Column);
	QVERIFY(childs.count() == 3);
	auto* dateTimeColumn = static_cast<Column*>(childs.at(0));
	QCOMPARE(dateTimeColumn->rowCount(), 8);
	// TODO:
	// auto* dayColumn = static_cast<Column*>(childs.at(1));
	// auto* monthColumn = static_cast<Column*>(childs.at(2));

	// TODO: must be implemented
	//	for (int i=0; i < 8; i++) {
	//		QCOMPARE(dateTimeColumn->dateTimeAt(i), QDateTime::fromString("2022-01-12T12:30:24.920"));
	//	}
}

void ColumnTest::testIndexForValueStrictlyMonotonouslyRising() {
	Column c(QStringLiteral("Double column"), Column::ColumnMode::Double);
	c.setValues({1.0, 2.0, 3.0, 4.0});

	QCOMPARE(c.indexForValue(0.5, true), 0);
	QCOMPARE(c.indexForValue(4.5, false), 3);
}

void ColumnTest::testIndexForValueMonotonouslyRising() {
	Column c(QStringLiteral("Double column"), Column::ColumnMode::Double);
	c.setValues({1.0, 1.0, 2.0, 2.0, 3.0, 4.0, 4.0});

	QCOMPARE(c.indexForValue(0.5, true), 0);

	QCOMPARE(c.indexForValue(2.0, true), 2);
	QCOMPARE(c.indexForValue(2.0, false), 3);

	QCOMPARE(c.indexForValue(4.5, false), 6);
}

void ColumnTest::indexForValueDatetimeStrictlyMonotonouslyRising() {
	Column c(QStringLiteral("column"), Column::ColumnMode::DateTime);
	c.setDateTimes({
		QDateTime::fromString(QStringLiteral("2017-07-24T00:00:00Z"), Qt::ISODate),
		QDateTime::fromString(QStringLiteral("2017-11-24T12:03:00Z"), Qt::ISODate),
		QDateTime::fromString(QStringLiteral("2019-12-24T00:05:03Z"), Qt::ISODate),
		QDateTime::fromString(QStringLiteral("2019-12-25T12:03:00Z"), Qt::ISODate),
		QDateTime::fromString(QStringLiteral("2019-12-26T00:05:03Z"), Qt::ISODate),
	});

	QCOMPARE(c.indexForValue(QDateTime::fromString(QStringLiteral("2000-07-24T00:00:00Z"), Qt::ISODate).toMSecsSinceEpoch(), true), 0);
	QCOMPARE(c.indexForValue(QDateTime::fromString(QStringLiteral("2020-07-24T00:00:00Z"), Qt::ISODate).toMSecsSinceEpoch(), false), 4);
}

void ColumnTest::indexForValueDatetimeMonotonouslyRising() {
	Column c(QStringLiteral("column"), Column::ColumnMode::DateTime);
	c.setDateTimes({
		QDateTime::fromString(QStringLiteral("2017-07-24T00:00:00Z"), Qt::ISODate),
		QDateTime::fromString(QStringLiteral("2017-07-24T00:00:00Z"), Qt::ISODate),
		QDateTime::fromString(QStringLiteral("2017-11-24T12:03:00Z"), Qt::ISODate),
		QDateTime::fromString(QStringLiteral("2019-12-24T00:05:03Z"), Qt::ISODate),
		QDateTime::fromString(QStringLiteral("2019-12-24T00:05:03Z"), Qt::ISODate),
		QDateTime::fromString(QStringLiteral("2019-12-25T12:03:00Z"), Qt::ISODate),
		QDateTime::fromString(QStringLiteral("2019-12-26T00:05:03Z"), Qt::ISODate),
		QDateTime::fromString(QStringLiteral("2019-12-26T00:05:03Z"), Qt::ISODate),
	});

	QCOMPARE(c.indexForValue(QDateTime::fromString(QStringLiteral("2000-07-24T00:00:00Z"), Qt::ISODate).toMSecsSinceEpoch(), true), 0);
	QCOMPARE(c.indexForValue(QDateTime::fromString(QStringLiteral("2000-07-24T00:00:00Z"), Qt::ISODate).toMSecsSinceEpoch(), false), 1);

	QCOMPARE(c.indexForValue(QDateTime::fromString(QStringLiteral("2019-12-24T00:05:03Z"), Qt::ISODate).toMSecsSinceEpoch(), true), 3);
	QCOMPARE(c.indexForValue(QDateTime::fromString(QStringLiteral("2019-12-24T00:05:03Z"), Qt::ISODate).toMSecsSinceEpoch(), false), 4);

	QCOMPARE(c.indexForValue(QDateTime::fromString(QStringLiteral("2020-07-24T00:00:00Z"), Qt::ISODate).toMSecsSinceEpoch(), false), 7);
	QCOMPARE(c.indexForValue(QDateTime::fromString(QStringLiteral("2020-07-24T00:00:00Z"), Qt::ISODate).toMSecsSinceEpoch(), true), 6);
}

/* Testing points and lines algorithm */
void ColumnTest::testIndexForValuePoints() {
	{
		const double value = 5;
		QVector<QPointF> points{};
		Column::Properties properties = Column::Properties::MonotonicIncreasing;
		QCOMPARE(Column::indexForValue(value, points, properties, false), -1);
	}

	{
		// Non strictly monotonic increasing
		QVector<QPointF> points{QPointF(10, 1), QPointF(10, 1), QPointF(20, 1), QPointF(20, 1), QPointF(30, 1), QPointF(40, 1), QPointF(50, 1), QPointF(50, 1)};
		Column::Properties properties = Column::Properties::MonotonicIncreasing;
		QCOMPARE(Column::indexForValue(0, points, properties, false), 1);
		QCOMPARE(Column::indexForValue(0, points, properties, true), 0);

		QCOMPARE(Column::indexForValue(20, points, properties, false), 3);
		QCOMPARE(Column::indexForValue(20, points, properties, true), 2);

		QCOMPARE(Column::indexForValue(60, points, properties, false), 7);
		QCOMPARE(Column::indexForValue(60, points, properties, true), 6);
	}

	{
		const double value = 5;
		QVector<QPointF> points{QPointF(10, 1), QPointF(20, 1), QPointF(30, 1), QPointF(40, 1), QPointF(50, 1)};
		Column::Properties properties = Column::Properties::MonotonicIncreasing;
		QCOMPARE(Column::indexForValue(value, points, properties, false), 0);
	}

	{
		const double value = 60;
		QVector<QPointF> points{QPointF(10, 1), QPointF(20, 1), QPointF(30, 1), QPointF(40, 1), QPointF(50, 1)};
		Column::Properties properties = Column::Properties::MonotonicIncreasing;
		QCOMPARE(Column::indexForValue(value, points, properties, false), 4);
	}

	{
		const double value = 16;
		QVector<QPointF> points{QPointF(10, 1), QPointF(20, 1), QPointF(30, 1), QPointF(40, 1), QPointF(50, 1)};
		Column::Properties properties = Column::Properties::MonotonicIncreasing;
		QCOMPARE(Column::indexForValue(value, points, properties, false), 1);
	}

	{
		const double value = 20;
		QVector<QPointF> points{QPointF(10, 1), QPointF(20, 1), QPointF(30, 1), QPointF(40, 1), QPointF(50, 1)};
		Column::Properties properties = Column::Properties::MonotonicIncreasing;
		QCOMPARE(Column::indexForValue(value, points, properties, false), 1);
	}
}

void ColumnTest::testIndexForValueLines() {
	{
		const double value = 5;
		Lines lines{};
		Column::Properties properties = Column::Properties::MonotonicIncreasing;
		QCOMPARE(Column::indexForValue(value, lines, properties, false), -1);
	}

	// The second point does not matter, because only the first one is used

	{
		// Non strictly monotonic increasing
		QVector<QLineF> lines{QLineF(QPointF(10, 1), QPointF(0, 0)),
							  QLineF(QPointF(10, 1), QPointF(0, 0)),
							  QLineF(QPointF(20, 1), QPointF(0, 0)),
							  QLineF(QPointF(20, 1), QPointF(0, 0)),
							  QLineF(QPointF(30, 1), QPointF(0, 0)),
							  QLineF(QPointF(40, 1), QPointF(0, 0)),
							  QLineF(QPointF(50, 1), QPointF(0, 0)),
							  QLineF(QPointF(50, 1), QPointF(0, 0))};
		Column::Properties properties = Column::Properties::MonotonicIncreasing;
		QCOMPARE(Column::indexForValue(0, lines, properties, false), 1);
		QCOMPARE(Column::indexForValue(0, lines, properties, true), 0);

		QCOMPARE(Column::indexForValue(20, lines, properties, false), 3);
		QCOMPARE(Column::indexForValue(20, lines, properties, true), 2);

		QCOMPARE(Column::indexForValue(60, lines, properties, false), 7);
		QCOMPARE(Column::indexForValue(60, lines, properties, true), 6);
	}

	{
		const double value = 5;
		QVector<QLineF> lines{QLineF(QPointF(10, 1), QPointF(0, 0)),
							  QLineF(QPointF(20, 1), QPointF(0, 0)),
							  QLineF(QPointF(30, 1), QPointF(0, 0)),
							  QLineF(QPointF(40, 1), QPointF(0, 0)),
							  QLineF(QPointF(50, 1), QPointF(0, 0))};
		Column::Properties properties = Column::Properties::MonotonicIncreasing;
		QCOMPARE(Column::indexForValue(value, lines, properties, false), 0);
	}

	{
		const double value = 60;
		QVector<QLineF> lines{QLineF(QPointF(10, 1), QPointF(0, 0)),
							  QLineF(QPointF(20, 1), QPointF(0, 0)),
							  QLineF(QPointF(30, 1), QPointF(0, 0)),
							  QLineF(QPointF(40, 1), QPointF(0, 0)),
							  QLineF(QPointF(50, 1), QPointF(0, 0))};
		Column::Properties properties = Column::Properties::MonotonicIncreasing;
		QCOMPARE(Column::indexForValue(value, lines, properties, false), 4);
	}

	{
		const double value = 16;
		QVector<QLineF> lines{QLineF(QPointF(10, 1), QPointF(0, 0)),
							  QLineF(QPointF(20, 1), QPointF(0, 0)),
							  QLineF(QPointF(30, 1), QPointF(0, 0)),
							  QLineF(QPointF(40, 1), QPointF(0, 0)),
							  QLineF(QPointF(50, 1), QPointF(0, 0))};
		Column::Properties properties = Column::Properties::MonotonicIncreasing;
		QCOMPARE(Column::indexForValue(value, lines, properties, false), 1);
	}

	{
		const double value = 20;
		QVector<QLineF> lines{QLineF(QPointF(10, 1), QPointF(0, 0)),
							  QLineF(QPointF(20, 1), QPointF(0, 0)),
							  QLineF(QPointF(30, 1), QPointF(0, 0)),
							  QLineF(QPointF(40, 1), QPointF(0, 0)),
							  QLineF(QPointF(50, 1), QPointF(0, 0))};
		Column::Properties properties = Column::Properties::MonotonicIncreasing;
		QCOMPARE(Column::indexForValue(value, lines, properties, false), 1);
	}
}

void ColumnTest::testIndexForValueDoubleVector() {
	{
		const double value = 5;
		QVector<double> points{};
		Column::Properties properties = Column::Properties::MonotonicIncreasing;
		QCOMPARE(Column::indexForValue(value, points, properties, false), -1);
	}

	{
		const double value = 5;
		QVector<double> points{10, 20, 30, 40, 50};
		Column::Properties properties = Column::Properties::MonotonicIncreasing;
		QCOMPARE(Column::indexForValue(value, points, properties, false), 0);
	}

	{
		const double value = 60;
		QVector<double> points{10, 20, 30, 40, 50};
		Column::Properties properties = Column::Properties::MonotonicIncreasing;
		QCOMPARE(Column::indexForValue(value, points, properties, false), 4);
	}

	{
		const double value = 16;
		QVector<double> points{10, 20, 30, 40, 50};
		Column::Properties properties = Column::Properties::MonotonicIncreasing;
		QCOMPARE(Column::indexForValue(value, points, properties, false), 1);
	}

	{
		const double value = 20;
		QVector<double> points{10, 20, 30, 40, 50};
		Column::Properties properties = Column::Properties::MonotonicIncreasing;
		QCOMPARE(Column::indexForValue(value, points, properties, false), 1);
	}
}

void ColumnTest::testInsertRow() {
	Project project;
	auto* c = new Column(QStringLiteral("Test"), Column::ColumnMode::Double);
	project.addChild(c);
	c->resizeTo(100);
	QCOMPARE(c->rowCount(), 100);

	int rowsAboutToBeInsertedCounter = 0;
	connect(c, &Column::rowsAboutToBeInserted, [&rowsAboutToBeInsertedCounter, c](const AbstractColumn* source, int before, int count) {
		QCOMPARE(source, c);
		switch (rowsAboutToBeInsertedCounter) {
		case 0:
			QCOMPARE(before, 100);
			QCOMPARE(count, 2);
			break;
		case 1:
			QCOMPARE(before, 102);
			QCOMPARE(count, 3);
			break;
		case 3: // redo()
			QCOMPARE(before, 102);
			QCOMPARE(count, 3);
			break;
		}
		rowsAboutToBeInsertedCounter++;
	});

	int rowsAboutToBeRemovedCounter = 0;
	connect(c, &Column::rowsAboutToBeRemoved, [&rowsAboutToBeRemovedCounter, c](const AbstractColumn* source, int first, int count) {
		QCOMPARE(source, c);
		switch (rowsAboutToBeRemovedCounter) {
		case 0:
			QCOMPARE(first, 102);
			QCOMPARE(count, 3);
			break;
		}
		rowsAboutToBeRemovedCounter++;
	});

	int rowsInsertedCounter = 0;
	connect(c, &Column::rowsInserted, [&rowsInsertedCounter, c](const AbstractColumn* source, int before, int count) {
		QCOMPARE(source, c);

		switch (rowsInsertedCounter) {
		case 0:
			QCOMPARE(before, 100);
			QCOMPARE(count, 2);
			break;
		case 1:
			QCOMPARE(before, 102);
			QCOMPARE(count, 3);
			break;
		case 3: // redo()
			QCOMPARE(before, 102);
			QCOMPARE(count, 3);
			break;
		}

		rowsInsertedCounter++;
	});

	int rowsRemovedCounter = 0;
	connect(c, &Column::rowsRemoved, [&rowsRemovedCounter, c](const AbstractColumn* source, int first, int count) {
		QCOMPARE(source, c);

		switch (rowsRemovedCounter) {
		case 0:
			QCOMPARE(first, 102);
			QCOMPARE(count, 3);
			break;
		}

		rowsRemovedCounter++;
	});

	c->insertRows(c->rowCount(), 2);
	QCOMPARE(c->rowCount(), 102);
	c->insertRows(c->rowCount(), 3);
	QCOMPARE(c->rowCount(), 105);

	c->undoStack()->undo();
	QCOMPARE(c->rowCount(), 102);
	c->undoStack()->redo();
	QCOMPARE(c->rowCount(), 105);

	QCOMPARE(rowsAboutToBeInsertedCounter, 3);
	QCOMPARE(rowsAboutToBeRemovedCounter, 1);
	QCOMPARE(rowsInsertedCounter, 3);
	QCOMPARE(rowsRemovedCounter, 1);
}

void ColumnTest::testRemoveRow() {
	Project project;
	auto* c = new Column(QStringLiteral("Test"), Column::ColumnMode::Double);
	project.addChild(c);
	c->resizeTo(100);
	QCOMPARE(c->rowCount(), 100);

	int rowsAboutToBeInsertedCounter = 0;
	connect(c, &Column::rowsAboutToBeInserted, [&rowsAboutToBeInsertedCounter, c](const AbstractColumn* source, int before, int count) {
		QCOMPARE(source, c);
		QCOMPARE(before, 96);
		QCOMPARE(count, 3);
		rowsAboutToBeInsertedCounter++;
	});

	int rowsAboutToBeRemovedCounter = 0;
	connect(c, &Column::rowsAboutToBeRemoved, [&rowsAboutToBeRemovedCounter, c](const AbstractColumn* source, int first, int count) {
		QCOMPARE(source, c);
		switch (rowsAboutToBeRemovedCounter) {
		case 0:
			QCOMPARE(first, 99);
			QCOMPARE(count, 1);
			break;
		case 1:
			QCOMPARE(first, 96);
			QCOMPARE(count, 3);
			break;
		case 2: // redo()
			QCOMPARE(first, 96);
			QCOMPARE(count, 3);
			break;
		}
		rowsAboutToBeRemovedCounter++;
	});

	int rowsInsertedCounter = 0;
	connect(c, &Column::rowsInserted, [&rowsInsertedCounter, c](const AbstractColumn* source, int before, int count) {
		QCOMPARE(source, c);

		QCOMPARE(before, 96);
		QCOMPARE(count, 3);

		rowsInsertedCounter++;
	});

	int rowsRemovedCounter = 0;
	connect(c, &Column::rowsRemoved, [&rowsRemovedCounter, c](const AbstractColumn* source, int first, int count) {
		QCOMPARE(source, c);

		switch (rowsRemovedCounter) {
		case 0:
			QCOMPARE(first, 99);
			QCOMPARE(count, 1);
			break;
		case 1:
			QCOMPARE(first, 96);
			QCOMPARE(count, 3);
			break;
		case 2: // redo()
			QCOMPARE(first, 96);
			QCOMPARE(count, 3);
			break;
		}

		rowsRemovedCounter++;
	});

	c->removeRows(c->rowCount() - 1, 1);
	QCOMPARE(c->rowCount(), 99);
	c->removeRows(c->rowCount() - 3, 3);
	QCOMPARE(c->rowCount(), 96);

	c->undoStack()->undo();
	QCOMPARE(c->rowCount(), 99);
	c->undoStack()->redo();
	QCOMPARE(c->rowCount(), 96);

	QCOMPARE(rowsAboutToBeInsertedCounter, 1);
	QCOMPARE(rowsAboutToBeRemovedCounter, 3);
	QCOMPARE(rowsInsertedCounter, 1);
	QCOMPARE(rowsRemovedCounter, 3);
}

void ColumnTest::testFormula() {
	auto c1 = Column(QStringLiteral("DataColumn"), Column::ColumnMode::Double);
	c1.replaceValues(-1, {1., 2., 3.});

	auto c2 = Column(QStringLiteral("FormulaColumn"), Column::ColumnMode::Double);
	c2.replaceValues(-1, {11., 12., 13., 14., 15., 16., 17.});

	c2.setFormula(QStringLiteral("mean(x)"), {QStringLiteral("x")}, QVector<Column*>({&c1}), true);
	c2.updateFormula();
	QCOMPARE(c2.rowCount(), 7);
	for (int i = 0; i < c2.rowCount(); i++) {
		VALUES_EQUAL(c2.valueAt(i), 2.);
	}
}

void ColumnTest::testFormula2() {
	auto c2 = Column(QStringLiteral("FormulaColumn"), Column::ColumnMode::Double);
	c2.replaceValues(-1, {11., 12., 13.});
	c2.setFormula(QStringLiteral("sin(i)"), {}, QVector<Column*>({}), true);
	c2.updateFormula();

	auto c3 = Column(QStringLiteral("DataColumn"), Column::ColumnMode::Double);
	c3.replaceValues(-1, {1., 2., 3.});
	c3.setFormula(QStringLiteral("cell(i; x)"), {QStringLiteral("x")}, QVector<Column*>({&c2}), true);
	c3.updateFormula();

	c3.setFormula(QStringLiteral("cell(i; x) + sin(x)"), {QStringLiteral("x")}, QVector<Column*>({&c2}), true);
	c3.updateFormula();
}

void ColumnTest::testFormulaCell() {
	auto c1 = Column(QStringLiteral("DataColumn"), Column::ColumnMode::Double);
	c1.replaceValues(-1, {1., 5., -1.});

	auto c3 = Column(QStringLiteral("DataColumn"), Column::ColumnMode::Double);
	c3.replaceValues(-1, {3., 2., 1.});

	auto c2 = Column(QStringLiteral("FormulaColumn"), Column::ColumnMode::Double);
	c2.replaceValues(-1, {11., 12., 13., 14., 15., 16., 17.});

	c2.setFormula(QStringLiteral("cell(y; x)"), {QStringLiteral("x"), QStringLiteral("y")}, QVector<Column*>({&c1, &c3}), true);
	c2.updateFormula();
	QCOMPARE(c2.rowCount(), 7);
	VALUES_EQUAL(c2.valueAt(0), -1.);
	VALUES_EQUAL(c2.valueAt(1), 5.);
	VALUES_EQUAL(c2.valueAt(2), 1.);
	VALUES_EQUAL(c2.valueAt(3), NAN);
	VALUES_EQUAL(c2.valueAt(4), NAN);
	VALUES_EQUAL(c2.valueAt(5), NAN);
	VALUES_EQUAL(c2.valueAt(6), NAN);
}

/*!
 * \brief ColumnTest::testFormulaCellDefault
 * Use default value if the index is smaller than zero
 */
void ColumnTest::testFormulaCellDefault() {
	auto c1 = Column(QStringLiteral("DataColumn"), Column::ColumnMode::Double);
	c1.replaceValues(-1, {1., 5., -1.});

	auto c2 = Column(QStringLiteral("FormulaColumn"), Column::ColumnMode::Double);
	c2.replaceValues(-1, {11., 12., 13., 14., 15., 16., 17.});

	c2.setFormula(QStringLiteral("cell_with_default(i - 1; -113; x)"), {QStringLiteral("x")}, QVector<Column*>({&c1}), true);
	c2.updateFormula();
	QCOMPARE(c2.rowCount(), 7);
	VALUES_EQUAL(c2.valueAt(0), -113.);
	VALUES_EQUAL(c2.valueAt(1), 1.);
	VALUES_EQUAL(c2.valueAt(2), 5.);
	VALUES_EQUAL(c2.valueAt(3), NAN); // There are only 3 elements in c1
	VALUES_EQUAL(c2.valueAt(4), NAN);
	VALUES_EQUAL(c2.valueAt(5), NAN);
	VALUES_EQUAL(c2.valueAt(6), NAN);
}

/*!
 * index in cell higher than rownumber
 */
void ColumnTest::testFormulaCellInvalid() {
	auto c1 = Column(QStringLiteral("DataColumn"), Column::ColumnMode::Double);
	c1.replaceValues(-1, {1., 2., 3.});

	auto c2 = Column(QStringLiteral("FormulaColumn"), Column::ColumnMode::Double);
	c2.replaceValues(-1, {11., 12., 13., 14., 15., 16., 17.});

	c2.setFormula(QStringLiteral("cell(10,x)"), {QStringLiteral("x")}, QVector<Column*>({&c1}), true);
	c2.updateFormula();
	QCOMPARE(c2.rowCount(), 7);
	// All invalid
	for (int i = 0; i < c2.rowCount(); i++)
		VALUES_EQUAL(c2.valueAt(i), NAN);
}

void ColumnTest::testFormulaCellConstExpression() {
	auto c1 = Column(QStringLiteral("DataColumn"), Column::ColumnMode::Double);
	c1.replaceValues(-1, {1., -1., 5.});

	auto c2 = Column(QStringLiteral("FormulaColumn"), Column::ColumnMode::Double);
	c2.replaceValues(-1, {11., 12., 13., 14., 15., 16., 17.});

	c2.setFormula(QStringLiteral("cell(2; x)"), {QStringLiteral("x")}, QVector<Column*>({&c1}), true);
	c2.updateFormula();
	QCOMPARE(c2.rowCount(), 7);
	// All invalid
	for (int i = 0; i < c2.rowCount(); i++)
		VALUES_EQUAL(c2.valueAt(i), -1.);
}

void ColumnTest::testFormulaCellMulti() {
	auto c1 = Column(QStringLiteral("DataColumn"), Column::ColumnMode::Double);
	c1.replaceValues(-1, {1., -1., 5.});

	auto c3 = Column(QStringLiteral("DataColumn"), Column::ColumnMode::Double);
	c3.replaceValues(-1, {-5., 100., 3});

	auto c2 = Column(QStringLiteral("FormulaColumn"), Column::ColumnMode::Double);
	c2.replaceValues(-1, {11., 12., 13., 14., 15., 16., 17.});

	c2.setFormula(QStringLiteral("cell(2; x) + cell(1; y)"), {QStringLiteral("x"), QStringLiteral("y")}, QVector<Column*>({&c1, &c3}), true);
	c2.updateFormula();
	QCOMPARE(c2.rowCount(), 7);
	for (int i = 0; i < c2.rowCount(); i++)
		VALUES_EQUAL(c2.valueAt(i), -6.);
}

void ColumnTest::testFormulaCurrentColumnCell() {
	auto c2 = Column(QStringLiteral("FormulaColumn"), Column::ColumnMode::Double);
	c2.replaceValues(-1, {11., 12., 13., 14., 15., 16., 17.});

	c2.setFormula(QStringLiteral("cell_curr_column(i - 1)"), {}, QVector<Column*>({}), true);
	c2.updateFormula();
	QCOMPARE(c2.rowCount(), 7);
	VALUES_EQUAL(c2.valueAt(0), NAN);
	VALUES_EQUAL(c2.valueAt(1), NAN);
	VALUES_EQUAL(c2.valueAt(2), NAN);
	VALUES_EQUAL(c2.valueAt(3), NAN);
	VALUES_EQUAL(c2.valueAt(4), NAN);
	VALUES_EQUAL(c2.valueAt(5), NAN);
	VALUES_EQUAL(c2.valueAt(6), NAN);
}

/*!
 * \brief ColumnTest::testFormulaCellDefault
 * Use default value if the index is smaller than zero
 */
void ColumnTest::testFormulaCurrentColumnCellDefaultValue() {
	auto c2 = Column(QStringLiteral("FormulaColumn"), Column::ColumnMode::Double);
	c2.replaceValues(-1, {11., 12., 13., 14., 15., 16., 17.});

	c2.setFormula(QStringLiteral("cell_curr_column_with_default(i - 1; -143) * 2"), {}, QVector<Column*>({}), true);
	c2.updateFormula();
	QCOMPARE(c2.rowCount(), 7);
	VALUES_EQUAL(c2.valueAt(0), -143. * 2.);
	VALUES_EQUAL(c2.valueAt(1), c2.valueAt(0) * 2);
	VALUES_EQUAL(c2.valueAt(2), c2.valueAt(1) * 2);
	VALUES_EQUAL(c2.valueAt(3), c2.valueAt(2) * 2);
	VALUES_EQUAL(c2.valueAt(4), c2.valueAt(3) * 2);
	VALUES_EQUAL(c2.valueAt(5), c2.valueAt(4) * 2);
	VALUES_EQUAL(c2.valueAt(6), c2.valueAt(5) * 2);
}

void ColumnTest::testFormulasmmin() {
	auto c1 = Column(QStringLiteral("DataColumn"), Column::ColumnMode::Double);
	c1.replaceValues(-1, {1., -1., 5., 5., 3., 8., 10., -5});

	auto c2 = Column(QStringLiteral("FormulaColumn"), Column::ColumnMode::Double);
	c2.replaceValues(-1, {11., 12., 13., 14., 15., 16., 17., 18.});

	c2.setFormula(QStringLiteral("smmin(3; x)"), {QStringLiteral("x")}, {&c1}, true);
	c2.updateFormula();
	QCOMPARE(c2.rowCount(), 8);
	VALUES_EQUAL(c2.valueAt(0), 1.);
	VALUES_EQUAL(c2.valueAt(1), -1.);
	VALUES_EQUAL(c2.valueAt(2), -1.);
	VALUES_EQUAL(c2.valueAt(3), -1.);
	VALUES_EQUAL(c2.valueAt(4), 3.);
	VALUES_EQUAL(c2.valueAt(5), 3.);
	VALUES_EQUAL(c2.valueAt(6), 3.);
	VALUES_EQUAL(c2.valueAt(7), -5.);
}

void ColumnTest::testFormulasmmax() {
	auto c1 = Column(QStringLiteral("DataColumn"), Column::ColumnMode::Double);
	c1.replaceValues(-1, {1., -1., 5., 5., 3., 8., 10., -5});

	auto c2 = Column(QStringLiteral("FormulaColumn"), Column::ColumnMode::Double);
	c2.replaceValues(-1, {11., 12., 13., 14., 15., 16., 17., 18.});

	c2.setFormula(QStringLiteral("smmax(3; x)"), {QStringLiteral("x")}, QVector<Column*>({&c1}), true);
	c2.updateFormula();
	QCOMPARE(c2.rowCount(), 8);
	VALUES_EQUAL(c2.valueAt(0), 1.);
	VALUES_EQUAL(c2.valueAt(1), 1.);
	VALUES_EQUAL(c2.valueAt(2), 5.);
	VALUES_EQUAL(c2.valueAt(3), 5.);
	VALUES_EQUAL(c2.valueAt(4), 5.);
	VALUES_EQUAL(c2.valueAt(5), 8.);
	VALUES_EQUAL(c2.valueAt(6), 10.);
	VALUES_EQUAL(c2.valueAt(7), 10.);
}

void ColumnTest::testFormulasma() {
	auto c1 = Column(QStringLiteral("DataColumn"), Column::ColumnMode::Double);
	c1.replaceValues(-1, {1., -1., 5., 5., 3., 8., 10., -5});

	auto c2 = Column(QStringLiteral("FormulaColumn"), Column::ColumnMode::Double);
	c2.replaceValues(-1, {11., 12., 13., 14., 15., 16., 17., 18.});

	c2.setFormula(QStringLiteral("sma(3; x)"), {QStringLiteral("x")}, QVector<Column*>({&c1}), true);
	c2.updateFormula();
	QCOMPARE(c2.rowCount(), 8);
	VALUES_EQUAL(c2.valueAt(0), 1. / 3.);
	VALUES_EQUAL(c2.valueAt(1), 0.);
	VALUES_EQUAL(c2.valueAt(2), 5. / 3.);
	VALUES_EQUAL(c2.valueAt(3), 3.);
	VALUES_EQUAL(c2.valueAt(4), 13. / 3.);
	VALUES_EQUAL(c2.valueAt(5), 16. / 3.);
	VALUES_EQUAL(c2.valueAt(6), 7.);
	VALUES_EQUAL(c2.valueAt(7), 13. / 3.);
}

void ColumnTest::testFormulapsample() {
	auto c1 = Column(QStringLiteral("DataColumn"), Column::ColumnMode::Double);
	c1.replaceValues(-1, {10., 9., 8., 7., 6., 5., 4., 3., 2., 1.});

	auto c2 = Column(QStringLiteral("FormulaColumn"), Column::ColumnMode::Double);
	c2.replaceValues(-1, {1., 2., 3., 4., 5.});

	c2.setFormula(QStringLiteral("psample(2; x)"), {QStringLiteral("x")}, QVector<Column*>({&c1}), true);
	c2.updateFormula();
	QCOMPARE(c2.rowCount(), 5); // set before
	VALUES_EQUAL(c2.valueAt(0), 10.);
	VALUES_EQUAL(c2.valueAt(1), 8.);
	VALUES_EQUAL(c2.valueAt(2), 6.);
	VALUES_EQUAL(c2.valueAt(3), 4.);
	VALUES_EQUAL(c2.valueAt(4), 2.);
}

void ColumnTest::testFormularsample() {
	auto c1 = Column(QStringLiteral("DataColumn"), Column::ColumnMode::Double);
	c1.replaceValues(-1, {1., 1., 1., 1., 2.});

	auto c2 = Column(QStringLiteral("FormulaColumn"), Column::ColumnMode::Double);
	c2.replaceValues(-1, {3., 3., 3., 3., 3.});

	c2.setFormula(QStringLiteral("rsample(x)"), {QStringLiteral("x")}, QVector<Column*>({&c1}), true);
	c2.updateFormula();
	QCOMPARE(c2.rowCount(), 5); // set before

	std::cout << c2.valueAt(0) << std::endl;
	std::cout << c2.valueAt(1) << std::endl;
	std::cout << c2.valueAt(2) << std::endl;
	std::cout << c2.valueAt(3) << std::endl;
	std::cout << c2.valueAt(4) << std::endl;

	// 1. or 2. randomly
	QVERIFY(c2.valueAt(0) == 1. || c2.valueAt(0) == 2.);
	QVERIFY(c2.valueAt(1) == 1. || c2.valueAt(1) == 2.);
	QVERIFY(c2.valueAt(2) == 1. || c2.valueAt(2) == 2.);
	QVERIFY(c2.valueAt(3) == 1. || c2.valueAt(3) == 2.);
	QVERIFY(c2.valueAt(4) == 1. || c2.valueAt(4) == 2.);
}

/////////////////////////////////////////////////////

void ColumnTest::testFormulasMinColumnInvalid() {
	const QVector<double> c1Vector = {1., -1., 5., 5., 3., 8., 10., -5}, c2Vector = {11., 12., 13., 14., 15., 16., 17., 18.};
	SETUP_C1_C2_COLUMNS(c1Vector, c2Vector)
	COLUMN2_SET_FORMULA_AND_EVALUATE("min()", NAN) // All invalid, because no column variable provided
}

/////////////////////////////////////////////////////

void ColumnTest::testFormulasSize() {
	const QVector<double> c1Vector = {1., -1., 8., 10., -5}, c2Vector = {11., 12., 13., 14., 15., 16., 17., 18.};
	SETUP_C1_C2_COLUMNS(c1Vector, c2Vector)
	COLUMN2_SET_FORMULA_AND_EVALUATE("size(x)", 5.)
}
void ColumnTest::testFormulasSum() {
	const QVector<double> c1Vector = {1., -1., 8., 10., -5}, c2Vector = {11., 12., 13., 14., 15., 16., 17., 18.};
	SETUP_C1_C2_COLUMNS(c1Vector, c2Vector)
	COLUMN2_SET_FORMULA_AND_EVALUATE("sum(x)", 13.)
}
void ColumnTest::testFormulasMin() {
	const QVector<double> c1Vector = {1., -1., 8., 10., -5}, c2Vector = {11., 12., 13., 14., 15., 16., 17., 18.};
	SETUP_C1_C2_COLUMNS(c1Vector, c2Vector)
	COLUMN2_SET_FORMULA_AND_EVALUATE("min(x)", -5.)
}
void ColumnTest::testFormulasMax() {
	const QVector<double> c1Vector = {1., -1., 8., 10., -5}, c2Vector = {11., 12., 13., 14., 15., 16., 17., 18.};
	SETUP_C1_C2_COLUMNS(c1Vector, c2Vector)
	COLUMN2_SET_FORMULA_AND_EVALUATE("max(x)", 10.)
}
void ColumnTest::testFormulasMean() {
	const QVector<double> c1Vector = {1., -1., 8., 10., -5}, c2Vector = {11., 12., 13., 14., 15., 16., 17., 18.};
	SETUP_C1_C2_COLUMNS(c1Vector, c2Vector)
	COLUMN2_SET_FORMULA_AND_EVALUATE("mean(x)", 13. / 5.)
}
void ColumnTest::testFormulasMedian() {
	const QVector<double> c1Vector = {1., -1., 8., 10., -5}, c2Vector = {11., 12., 13., 14., 15., 16., 17., 18.};
	SETUP_C1_C2_COLUMNS(c1Vector, c2Vector)
	COLUMN2_SET_FORMULA_AND_EVALUATE("median(x)", 1.)
}
void ColumnTest::testFormulasStdev() {
	const QVector<double> c1Vector = {1., -1., 8., 10., -5}, c2Vector = {11., 12., 13., 14., 15., 16., 17., 18.};
	SETUP_C1_C2_COLUMNS(c1Vector, c2Vector)
	COLUMN2_SET_FORMULA_AND_EVALUATE("stdev(x)", 6.2689712074) // calculated with octave "std"
}
void ColumnTest::testFormulasVar() {
	const QVector<double> c1Vector = {1., -1., 8., 10., -5}, c2Vector = {11., 12., 13., 14., 15., 16., 17., 18.};
	SETUP_C1_C2_COLUMNS(c1Vector, c2Vector)
	COLUMN2_SET_FORMULA_AND_EVALUATE("var(x)", 39.3) // calculated with octave "var"
}
void ColumnTest::testFormulasGm() {
	const QVector<double> c1Vector = {1., 100., 8., 10., 3}, c2Vector = {11., 12., 13., 14., 15., 16., 17., 18.};
	SETUP_C1_C2_COLUMNS(c1Vector, c2Vector)
	COLUMN2_SET_FORMULA_AND_EVALUATE("gm(x)", 7.51696) // Calculated with R exp(mean(log(x)))
}
void ColumnTest::testFormulasHm() {
	const QVector<double> c1Vector = {1., -3., 8., 10., -5}, c2Vector = {11., 12., 13., 14., 15., 16., 17., 18.};
	SETUP_C1_C2_COLUMNS(c1Vector, c2Vector)
	COLUMN2_SET_FORMULA_AND_EVALUATE("hm(x)", 7.228916) // calculated with R harmonic.mean(x)
}
void ColumnTest::testFormulasChm() {
	const QVector<double> c1Vector = {1.0, 0.0, 2.0, 5.0}, c2Vector = {11., 12., 13., 14., 15., 16., 17., 18.};
	SETUP_C1_C2_COLUMNS(c1Vector, c2Vector)
	COLUMN2_SET_FORMULA_AND_EVALUATE("chm(x)", 3.75) // Result used from: statisticsDoubleZero()
}
void ColumnTest::testFormulasStatisticsMode() {
	const QVector<double> c1Vector = {1., -1., 8., 10., -5}, c2Vector = {11., 12., 13., 14., 15., 16., 17., 18.};

	// Calculated with R:
	// Mode <- function(x) {
	// ux <- unique(x)
	//	 ux[which.max(tabulate(match(x, ux)))]
	// }
	// Mode(x)
	SETUP_C1_C2_COLUMNS(c1Vector, c2Vector)
	COLUMN2_SET_FORMULA_AND_EVALUATE("mode(x)", NAN)
}
void ColumnTest::testFormulasQuartile1() {
	const QVector<double> c1Vector = {1., -1., 8., 10., -5}, c2Vector = {11., 12., 13., 14., 15., 16., 17., 18.};
	SETUP_C1_C2_COLUMNS(c1Vector, c2Vector)
	COLUMN2_SET_FORMULA_AND_EVALUATE("quartile1(x)", -1.) // Calculated with R: summary(x)
}
void ColumnTest::testFormulasQuartile3() {
	const QVector<double> c1Vector = {1., -1., 8., 10., -5}, c2Vector = {11., 12., 13., 14., 15., 16., 17., 18.};

	SETUP_C1_C2_COLUMNS(c1Vector, c2Vector)
	COLUMN2_SET_FORMULA_AND_EVALUATE("quartile3(x)", 8.) // Calculated with R: summary(x)
}
void ColumnTest::testFormulasIqr() {
	const QVector<double> c1Vector = {1., -1., 8., 10., -5}, c2Vector = {11., 12., 13., 14., 15., 16., 17., 18.};

	SETUP_C1_C2_COLUMNS(c1Vector, c2Vector)
	COLUMN2_SET_FORMULA_AND_EVALUATE("iqr(x)", 9.); // Calculated with R: IQR(x)
}
void ColumnTest::testFormulasPercentile1() {
	const QVector<double> c1Vector = {1., -1., 8., 10., -5}, c2Vector = {11., 12., 13., 14., 15., 16., 17., 18.};

	SETUP_C1_C2_COLUMNS(c1Vector, c2Vector)
	COLUMN2_SET_FORMULA_AND_EVALUATE("percentile1(x)", -4.84); // Calculated with R: quantile(x, 1/100)
}
void ColumnTest::testFormulasPercentile5() {
	const QVector<double> c1Vector = {1., -1., 8., 10., -5}, c2Vector = {11., 12., 13., 14., 15., 16., 17., 18.};

	SETUP_C1_C2_COLUMNS(c1Vector, c2Vector)
	COLUMN2_SET_FORMULA_AND_EVALUATE("percentile5(x)", -4.2); // Calculated with R: quantile(x, 5/100)
}
void ColumnTest::testFormulasPercentile10() {
	const QVector<double> c1Vector = {1., -1., 8., 10., -5}, c2Vector = {11., 12., 13., 14., 15., 16., 17., 18.};

	SETUP_C1_C2_COLUMNS(c1Vector, c2Vector)
	COLUMN2_SET_FORMULA_AND_EVALUATE("percentile10(x)", -3.4); // Calculated with R: quantile(x, 10/100)
}
void ColumnTest::testFormulasPercentile90() {
	const QVector<double> c1Vector = {1., -1., 8., 10., -5}, c2Vector = {11., 12., 13., 14., 15., 16., 17., 18.};

	SETUP_C1_C2_COLUMNS(c1Vector, c2Vector)
	COLUMN2_SET_FORMULA_AND_EVALUATE("percentile90(x)", 9.2); // Calculated with R: quantile(x, 90/100)
}
void ColumnTest::testFormulasPercentile95() {
	const QVector<double> c1Vector = {1., -1., 8., 10., -5}, c2Vector = {11., 12., 13., 14., 15., 16., 17., 18.};

	SETUP_C1_C2_COLUMNS(c1Vector, c2Vector)
	COLUMN2_SET_FORMULA_AND_EVALUATE("percentile95(x)", 9.6); // Calculated with R: quantile(x, 95/100)
}
void ColumnTest::testFormulasPercentile99() {
	const QVector<double> c1Vector = {1., -1., 8., 10., -5}, c2Vector = {11., 12., 13., 14., 15., 16., 17., 18.};

	SETUP_C1_C2_COLUMNS(c1Vector, c2Vector)
	COLUMN2_SET_FORMULA_AND_EVALUATE("percentile99(x)", 9.92); // Calculated with R: quantile(x, 99/100)
}
void ColumnTest::testFormulasTrimean() {
	const QVector<double> c1Vector = {1.0, 0.0, 2.0, 5.0}, c2Vector = {11., 12., 13., 14., 15., 16., 17., 18.};

	SETUP_C1_C2_COLUMNS(c1Vector, c2Vector)
	COLUMN2_SET_FORMULA_AND_EVALUATE("trimean(x)", 1.625); // Value used from statisticsDoubleZero()
}
void ColumnTest::testFormulasMeandev() {
	const QVector<double> c1Vector = {1.0, 0.0, 2.0, 5.0}, c2Vector = {11., 12., 13., 14., 15., 16., 17., 18.};

	SETUP_C1_C2_COLUMNS(c1Vector, c2Vector)
	COLUMN2_SET_FORMULA_AND_EVALUATE("meandev(x)", 1.5); // Value used from statisticsDoubleZero()
}
void ColumnTest::testFormulasMeandevmedian() {
	const QVector<double> c1Vector = {1.0, 0.0, 2.0, 5.0}, c2Vector = {11., 12., 13., 14., 15., 16., 17., 18.};

	SETUP_C1_C2_COLUMNS(c1Vector, c2Vector)
	COLUMN2_SET_FORMULA_AND_EVALUATE("meandevmedian(x)", 1.5); // Value used from statisticsDoubleZero()
}
void ColumnTest::testFormulasMediandev() {
	const QVector<double> c1Vector = {1.0, 0.0, 2.0, 5.0}, c2Vector = {11., 12., 13., 14., 15., 16., 17., 18.};

	SETUP_C1_C2_COLUMNS(c1Vector, c2Vector)
	COLUMN2_SET_FORMULA_AND_EVALUATE("mediandev(x)", 1.); // Value used from statisticsDoubleZero()
}
void ColumnTest::testFormulasSkew() {
	const QVector<double> c1Vector = {1., -1., 8., 10., -5}, c2Vector = {11., 12., 13., 14., 15., 16., 17., 18.};

	SETUP_C1_C2_COLUMNS(c1Vector, c2Vector)
	COLUMN2_SET_FORMULA_AND_EVALUATE("skew(x)", 0.082773441985478); // Calculated with R: skewness(x)
}
void ColumnTest::testFormulasKurt() {
	const QVector<double> c1Vector = {1., -1., 8., 10., -5}, c2Vector = {11., 12., 13., 14., 15., 16., 17., 18.};

	SETUP_C1_C2_COLUMNS(c1Vector, c2Vector)
	COLUMN2_SET_FORMULA_AND_EVALUATE("kurt(x)", 1.4891031991143); // Calculated with R: kurtosis(x)
}
void ColumnTest::testFormulasEntropy() {
	const QVector<double> c1Vector = {1.0, 0.0, 2.0, 5.0}, c2Vector = {11., 12., 13., 14., 15., 16., 17., 18.};

	SETUP_C1_C2_COLUMNS(c1Vector, c2Vector)
	COLUMN2_SET_FORMULA_AND_EVALUATE("entropy(x)", 2.); // Value from statisticsDoubleZero()
}

void ColumnTest::testFormulasQuantile() {
	QLocale::setDefault(QLocale::C); // . as decimal separator
	const QVector<double> c1Vector = {1., -1., 8., 10., -5}, c2Vector = {11., 12., 13., 14., 15., 16., 17., 18.};

	SETUP_C1_C2_COLUMNS(c1Vector, c2Vector)
	COLUMN2_SET_FORMULA_AND_EVALUATE("quantile(0.1;x)", -3.4); // Calculated with R: quantile(x, 0.1)
}

void ColumnTest::testFormulasPercentile() {
	const QVector<double> c1Vector = {1., -1., 8., 10., -5}, c2Vector = {11., 12., 13., 14., 15., 16., 17., 18.};

	SETUP_C1_C2_COLUMNS(c1Vector, c2Vector)
	COLUMN2_SET_FORMULA_AND_EVALUATE("percentile(30;x)", -0.6); // Calculated with R: quantile(x, 30/100)
}

void ColumnTest::clearContentNoFormula() {
	Project project;
	auto* c = new Column(QStringLiteral("Test"), Column::ColumnMode::Double);
	project.addChild(c);
	c->resizeTo(100);
	QCOMPARE(c->rowCount(), 100);

	for (int i = 0; i < c->rowCount(); i++)
		c->setValueAt(i, i);

	QCOMPARE(c->rowCount(), 100);
	for (int i = 0; i < c->rowCount(); i++)
		QCOMPARE(c->valueAt(i), i);

	c->clear();

	QCOMPARE(c->rowCount(), 100);
	for (int i = 0; i < c->rowCount(); i++)
		QCOMPARE(c->valueAt(i), NAN);
}

void ColumnTest::clearContentFormula() {
	Project project;
	auto* c = new Column(QStringLiteral("Test"), Column::ColumnMode::Double);
	project.addChild(c);
	c->resizeTo(100);
	QCOMPARE(c->rowCount(), 100);

	for (int i = 0; i < c->rowCount(); i++)
		c->setValueAt(i, i);

	QCOMPARE(c->rowCount(), 100);
	for (int i = 0; i < c->rowCount(); i++)
		QCOMPARE(c->valueAt(i), i);

	c->setFormula(QStringLiteral("x"), {QStringLiteral("zet")}, QVector<Column*>({c}), true);
	c->clear();

	QCOMPARE(c->rowCount(), 100);
	for (int i = 0; i < c->rowCount(); i++)
		QCOMPARE(c->valueAt(i), NAN);

	QCOMPARE(c->formula().isEmpty(), true);

	c->undoStack()->undo();

	QCOMPARE(c->rowCount(), 100);
	for (int i = 0; i < c->rowCount(); i++)
		QCOMPARE(c->valueAt(i), i);

	QCOMPARE(c->formula(), QStringLiteral("x"));
	QCOMPARE(c->formulaData().count(), 1);
	QCOMPARE(c->formulaData().at(0).column(), c);
	QCOMPARE(c->formulaData().at(0).variableName(), QStringLiteral("zet"));
}

void ColumnTest::testRowCountMonotonIncrease() {
	Column c(QStringLiteral("Test"), Column::ColumnMode::Double);
	c.replaceValues(-1, {-3., 1., 2., 5., 11.});

	QCOMPARE(c.rowCount(1., 5.), 3);
}

void ColumnTest::testRowCountMonotonDecrease() {
	Column c(QStringLiteral("Test"), Column::ColumnMode::Double);
	c.replaceValues(-1, {11., 9., 3., -4., -154.});

	QCOMPARE(c.rowCount(9., -4.), 3);
}

void ColumnTest::testRowCountNonMonoton() {
	Column c(QStringLiteral("Test"), Column::ColumnMode::Double);
	c.replaceValues(-1, {-3., 1., -5., 5., 11., 9., 100., 2., 4.});

	QCOMPARE(c.rowCount(1., 5.), 4);
}

void ColumnTest::testRowCountDateTime() {
	Column c(QStringLiteral("Test"), Column::ColumnMode::DateTime);
	c.replaceDateTimes(-1,
					   {QDateTime::fromString(QStringLiteral("2018-03-26T02:14:34.000Z"), Qt::DateFormat::ISODateWithMs),
						QDateTime::fromString(QStringLiteral("2018-03-27T02:14:34.000Z"), Qt::DateFormat::ISODateWithMs),
						QDateTime::fromString(QStringLiteral("2018-03-28T02:14:34.000Z"), Qt::DateFormat::ISODateWithMs),
						QDateTime::fromString(QStringLiteral("2018-03-29T02:14:34.000Z"), Qt::DateFormat::ISODateWithMs),
						QDateTime::fromString(QStringLiteral("2018-03-30T02:14:34.000Z"), Qt::DateFormat::ISODateWithMs),
						QDateTime::fromString(QStringLiteral("2018-03-31T02:14:34.000Z"), Qt::DateFormat::ISODateWithMs)});

	QCOMPARE(c.rowCount(QDateTime::fromString(QStringLiteral("2018-03-27T02:14:34.000Z"), Qt::DateFormat::ISODateWithMs).toMSecsSinceEpoch(),
						QDateTime::fromString(QStringLiteral("2018-03-30T02:14:34.000Z"), Qt::DateFormat::ISODateWithMs).toMSecsSinceEpoch()),
			 4);
}

void ColumnTest::testRowCountDateTimeMonotonDecrease() {
	Column c(QStringLiteral("Test"), Column::ColumnMode::DateTime);
	c.replaceDateTimes(-1,
					   {
						   QDateTime::fromString(QStringLiteral("2018-03-31T02:14:34.000Z"), Qt::DateFormat::ISODateWithMs),
						   QDateTime::fromString(QStringLiteral("2018-03-30T02:14:34.000Z"), Qt::DateFormat::ISODateWithMs),
						   QDateTime::fromString(QStringLiteral("2018-03-29T02:14:34.000Z"), Qt::DateFormat::ISODateWithMs),
						   QDateTime::fromString(QStringLiteral("2018-03-28T02:14:34.000Z"), Qt::DateFormat::ISODateWithMs),
						   QDateTime::fromString(QStringLiteral("2018-03-27T02:14:34.000Z"), Qt::DateFormat::ISODateWithMs),
						   QDateTime::fromString(QStringLiteral("2018-03-26T02:14:34.000Z"), Qt::DateFormat::ISODateWithMs),
					   });

	QCOMPARE(c.rowCount(QDateTime::fromString(QStringLiteral("2018-03-27T02:14:34.000Z"), Qt::DateFormat::ISODateWithMs).toMSecsSinceEpoch(),
						QDateTime::fromString(QStringLiteral("2018-03-30T02:14:34.000Z"), Qt::DateFormat::ISODateWithMs).toMSecsSinceEpoch()),
			 4);
}

void ColumnTest::testRowCountValueLabels() {
	Column c(QStringLiteral("Test"), Column::ColumnMode::Double);
	c.addValueLabel(-2., QStringLiteral("Status 1"));
	c.addValueLabel(-1., QStringLiteral("Status 2"));
	c.addValueLabel(4., QStringLiteral("Status 3"));
	c.addValueLabel(5., QStringLiteral("Status 2"));
	c.addValueLabel(6., QStringLiteral("Status 3"));

	QCOMPARE(c.valueLabelsCount(-1., 5.), 3);
}

void ColumnTest::testRowCountValueLabelsDateTime() {
	Column c(QStringLiteral("Test"), Column::ColumnMode::DateTime);
	c.addValueLabel(QDateTime::fromString(QStringLiteral("2018-03-26T02:14:34.000Z"), Qt::DateFormat::ISODateWithMs), QStringLiteral("Status 1"));
	c.addValueLabel(QDateTime::fromString(QStringLiteral("2018-03-27T02:14:34.000Z"), Qt::DateFormat::ISODateWithMs), QStringLiteral("Status 2"));
	c.addValueLabel(QDateTime::fromString(QStringLiteral("2018-03-28T02:14:34.000Z"), Qt::DateFormat::ISODateWithMs), QStringLiteral("Status 3"));
	c.addValueLabel(QDateTime::fromString(QStringLiteral("2018-03-30T02:14:34.000Z"), Qt::DateFormat::ISODateWithMs), QStringLiteral("Status 4"));
	c.addValueLabel(QDateTime::fromString(QStringLiteral("2018-03-31T02:14:34.000Z"), Qt::DateFormat::ISODateWithMs), QStringLiteral("Status 5"));

	QCOMPARE(c.valueLabelsCount(QDateTime::fromString(QStringLiteral("2018-03-27T02:14:34.000Z"), Qt::DateFormat::ISODateWithMs).toMSecsSinceEpoch(),
								QDateTime::fromString(QStringLiteral("2018-03-30T02:14:34.000Z"), Qt::DateFormat::ISODateWithMs).toMSecsSinceEpoch()),
			 3);
}

void ColumnTest::testLoadSaveNoData() {
	QString savePath;
	QString columnName;

	{
		Project project;
		project.setSaveData(false);
		auto* s = new Spreadsheet(QStringLiteral("MySpreadsheet"));
		project.addChild(s);
		s->setColumnCount(1);
		auto* c = s->column(0);
		columnName = c->name();
		c->resizeTo(100);
		QCOMPARE(c->rowCount(), 100);

		for (int i = 0; i < c->rowCount(); i++)
			c->setValueAt(i, i);

		SAVE_PROJECT("testLoadSaveNoData");
	}

	{
		Project project;
		QVERIFY(project.load(savePath));

		const auto& spreadsheets = project.children<Spreadsheet>();
		QCOMPARE(spreadsheets.size(), 1);

		const auto& columns = spreadsheets.at(0)->children<Column>();
		QCOMPARE(columns.size(), 1);
		const auto* column = columns.at(0);
		QCOMPARE(column->name(), columnName);
		QCOMPARE(column->rowCount(), 0);
	}
}

void ColumnTest::testLoadSaveWithData() {
	QString savePath;
	QString columnName;
	{
		Project project;
		project.setSaveData(true);
		auto* s = new Spreadsheet(QStringLiteral("MySpreadsheet"));
		project.addChild(s);
		s->setColumnCount(1);
		auto* c = s->column(0);
		columnName = c->name();
		c->resizeTo(100);
		QCOMPARE(c->rowCount(), 100);

		for (int i = 0; i < c->rowCount(); i++)
			c->setValueAt(i, i);

		SAVE_PROJECT("testLoadSaveNoData");
	}

	{
		Project project;
		QVERIFY(project.load(savePath));

		const auto& spreadsheets = project.children<Spreadsheet>();
		QCOMPARE(spreadsheets.size(), 1);

		const auto& columns = spreadsheets.at(0)->children<Column>();
		QCOMPARE(columns.size(), 1);
		const auto* column = columns.at(0);
		QCOMPARE(column->name(), columnName);
		QCOMPARE(column->rowCount(), 100);

		for (int i = 0; i < column->rowCount(); i++)
			QCOMPARE(column->valueAt(i), i);
	}
}

QTEST_MAIN(ColumnTest)
