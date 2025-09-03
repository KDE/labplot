/*
	File                 : HypothesisTestPrivate.h
	Project              : LabPlot
	Description          : Private members of HypothesisTest
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2025 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef HYPOTHESISTESTPRIVATE_H
#define HYPOTHESISTESTPRIVATE_H

#include "HypothesisTest.h"
#include "backend/core/AbstractColumn.h"
#include <cstddef>

class Column;

class HypothesisTestPrivate {
public:
	explicit HypothesisTestPrivate(HypothesisTest*);
	QString name() const;

	void recalculate();
	void resetResult();

	QString performOneSampleTTest();
	QString performTwoSampleTTest(bool);
	QString performWelchTTest();
	QString performOneWayANOVATest();
	QString performOneWayANOVARepeatedTest();
	QString performMannWhitneyUTest();
	QString performKruskalWallisTest();
	QString performWilcoxonTest();
	QString performFriedmanTest();
	QString performChisqGoodnessOfFitTest();
	QString performChisqIndependenceTest();
	QString performLogRankTest();

	size_t minSampleCount(HypothesisTest::Test);

	HypothesisTest* const q;

	QString result; // result html text
	QVector<const AbstractColumn*> dataColumns; // colums with the data for the test
	QVector<QString> dataColumnPaths; // paths of the columns with the data for the test
	HypothesisTest::Test test{HypothesisTest::Test::t_test_one_sample};
	double testMean{0.0};
	double significanceLevel{0.05};
	nsl_stats_tail_type tail{nsl_stats_tail_type_two};

private:
	static QString addResultTitle(const QString&);
	static QString addResultSection(const QString&);
	static QString addResultLine(const QString& name, const QString& value);
	static QString addResultLine(const QString& name, double value);
	static QString addResultLine(const QString& name);
	static QString addResultTable(const QVector<QVector<QString>>& data, bool horizontal);
	static QString addResultColumnStatistics(const QVector<const AbstractColumn*>& columns);

	static bool rowIsValid(const AbstractColumn*, int);

	template<typename T>
	static QVector<T> filterColumn(const AbstractColumn* col);
	template<typename T>
	static QVector<QVector<T>> filterColumns(const QVector<const AbstractColumn*>&);
	template<typename T>
	static QVector<QVector<T>> filterColumnsParallel(const QVector<const AbstractColumn*>& columns);
	template<typename T>
	static T** toArrayOfArrays(const QVector<QVector<T>>& data);

	static QString oneSampleTTestResultTemplate();
	static QString independentTwoSampleTTestResultTemplate();
	static QString pairedTwoSampleTTestResultTemplate();
	static QString welchTTestResultTemplate();
	static QString oneWayANOVAResultTemplate();
	static QString oneWayANOVARepeatedResultTemplate();
	static QString mannWhitneyUTestResultTemplate();
	static QString kruskalWallisTestResultTemplate();
	static QString wilcoxonTestResultTemplate();
	static QString friedmanTestResultTemplate();
	static QString chisqGoodnessOfFitTestResultTemplate();
	static QString chisqIndependenceTestResultTemplate();
	static QString logRankTestResultTemplate();
	static QString notAvailable();
	static QString testResultNotAvailable();
	static QVector<QString> columnStatisticsHeaders();
	static QString emptyResultColumnStatistics();
};

#endif
