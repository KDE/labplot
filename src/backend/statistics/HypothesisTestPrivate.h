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

class HypothesisTestPrivate {
public:
	explicit HypothesisTestPrivate(HypothesisTest*);
	QString name() const;

	void recalculate();
	void resetResult();

	void performOneSampleTTest();
	void performTwoSampleTTest(bool);
	void performOneWayANOVATest();
	void performMannWhitneyUTest();
	void performKruskalWallisTest();
	void performLogRankTest();

	HypothesisTest* const q;

    QString result; // result html text
	QVector<const AbstractColumn*> dataColumns; // colums with the data for the test
	QVector<QString> dataColumnPaths; // paths of the columns with the data for the test
	HypothesisTest::Test test{HypothesisTest::Test::t_test_one_sample};
	double testMean{0.0};
	double significanceLevel{0.05};
	nsl_stats_tail_type tail{nsl_stats_tail_type_two};

private:
	void addResultTitle(const QString&);
	void addResultSection(const QString&);
	void addResultLine(const QString& name, const QString& value);
	void addResultLine(const QString& name, double value);
	void addResultLine(const QString& name);

	static bool rowIsValid(const AbstractColumn*, int);
	static QVector<double> filterColumn(const AbstractColumn* col);
	static QVector<QVector<double>> filterColumns(const QVector<const AbstractColumn*>&);
	static QVector<QVector<double>> filterColumnsParallel(const QVector<const AbstractColumn*>& columns);
	static double** toArrayOfArrays(QVector<QVector<double>>& data);
};

#endif
