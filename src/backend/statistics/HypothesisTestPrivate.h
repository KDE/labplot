/*
	File                 : HypothesisTestPrivate.h
	Project              : LabPlot
	Description          : Private members of HypothesisTest
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2025 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2025 Israel Galadima <izzygaladima@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef HYPOTHESISTESTPRIVATE_H
#define HYPOTHESISTESTPRIVATE_H

#include "HypothesisTest.h"

class AbstractColumn;

class HypothesisTestPrivate {
public:
	explicit HypothesisTestPrivate(HypothesisTest*);
	QString name() const;

	void recalculate();
	void resetResult();

	void performOneSampleTTest();
	void performTwoSampleTTest(bool);
	void performWelchTTest();
	void performOneWayANOVATest();
	void performOneWayANOVARepeatedTest();
	void performMannWhitneyUTest();
	void performKruskalWallisTest();
	void performWilcoxonTest();
	void performFriedmanTest();
	void performChisqGoodnessOfFitTest();
	void performChisqIndependenceTest();
	void performLogRankTest();

	size_t minSampleCount(HypothesisTest::Test);

	HypothesisTest* const q;

	QString result; // result html text
	QVector<const AbstractColumn*> dataColumns; // columns with the data for the test
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

	static QString notAvailable();
	static QString testResultNotAvailable();
	static QString twoColumnsRequired();
	static QString atLeastTwoColumnsRequired();
	static QString atLeastXSamplesRequired();
	static QString samplesMustBeEqualSize();
	static QVector<QString> columnStatisticsHeaders();
	static QString emptyResultColumnStatistics();
	static QString resultTemplate(HypothesisTest::Test);
};

#endif
