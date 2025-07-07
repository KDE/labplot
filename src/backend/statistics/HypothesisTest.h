/***************************************************************************
	File                 : HypothesisTest.cpp
	Project              : LabPlot
	Description          : Hypothesis Test
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2019 Devanshu Agarwal <agarwaldevanshu8@gmail.com>
	SPDX-FileCopyrightText: 2023-205 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2025 Kuntal Bar <barkuntal6@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
***************************************************************************/

#ifndef HYPOTHESISTEST_H
#define HYPOTHESISTEST_H

#include "GeneralTest.h"

#include <QPair>

extern "C" {
#include "backend/nsl/nsl_statistical_test.h"
}

class HypothesisTest : public GeneralTest {
	Q_OBJECT

public:
	explicit HypothesisTest(const QString& name);
	~HypothesisTest() override;

	enum class Test { t_test_one_sample, t_test_two_sample, t_test_two_sample_paired, one_way_anova, mann_whitney_u_test, kruskal_wallis_test, log_rank_test };
	enum class NullHypothesisType {
		NullEquality, // H0: μ = μ₀
		NullLessEqual, // H0: μ ≤ μ₀
		NullGreaterEqual // H0: μ ≥ μ₀
	};

	void setTestMean(double mean);
	double testMean() const;

	void setSignificanceLevel(double alpha);
	double significanceLevel() const;

	void setNullHypothesis(NullHypothesisType);
	NullHypothesisType nullHypothesis() const;

	void setTest(HypothesisTest::Test);
	HypothesisTest::Test test() const;

	void recalculate();
	void resetResult();

	void setColumns(const QVector<Column*>&) override;

	static QPair<int, int> variableCount(HypothesisTest::Test);

private:
	void performOneSampleTTest();
	void performTwoSampleTTest(bool paired);
	void performOneWayANOVATest();
	void performMannWhitneyUTest();
	void performKruskalWallisTest();
	void performLogRankTest();

	nsl_stats_tail_type tail() const;

	// TODO: move to a private class
	Test m_test{Test::t_test_one_sample};
	NullHypothesisType m_nullHypothesisType{NullHypothesisType::NullEquality};
	double m_testMean{0.0};
	double m_significanceLevel{0.05};
	nsl_stats_tail_type m_tail{nsl_stats_tail_type_two};
	QList<double> m_pValue;
	QList<double> m_statisticValue;
};

#endif // HYPOTHESISTEST_H
