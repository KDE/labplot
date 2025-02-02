/***************************************************************************
	File                 : HypothesisTest.h
	Project              : LabPlot
	Description          : Hypothesis Test – One Sample T-Test
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2019  Devanshu Agarwal (agarwaldevanshu8@gmail.com)
	SPDX-FileCopyrightText: 2023  Alexander Semke (alexander.semke@web.de)
	SPDX-FileCopyrightText: 2025  Kuntal Bar (barkuntal6@gmail.com)
	SPDX-License-Identifier: GPL-2.0-or-later
***************************************************************************/

#ifndef HYPOTHESISTEST_H
#define HYPOTHESISTEST_H

#include "GeneralTest.h"
#include <QVector>

class HypothesisTest : public GeneralTest {
	Q_OBJECT

public:
	explicit HypothesisTest(const QString& name);
	~HypothesisTest() override;

	enum NullHypothesisType {
		NullEquality,       // H0: μ = μ₀
		NullLessEqual,      // H0: μ ≤ μ₀
		NullGreaterEqual    // H0: μ ≥ μ₀
	};

	enum HypothesisTailType {
		TailPositive,
		TailNegative,
		TailTwo
	};

	// Set test parameters
	void setPopulationMean(double mean);
	void setSignificanceLevel(double alpha);
	void setTail(HypothesisTailType tail);
	void setNullHypothesis(NullHypothesisType type);
	NullHypothesisType nullHypothesis() const;
	// Run the test
	void runTest();
	void performOneSampleTTest();
	double calculatePValue(double& tValue, const QString& col1Name, const QString& col2Name, int df);

private:
	void displayTestResults(double tStatistic, double pValue, int df);
	void logError(const QString& errorMsg);
	QString generatePValueTooltip(const double& pValue);
	NullHypothesisType m_nullHypothesisType = NullEquality; // default
	double m_populationMean;
	double m_significanceLevel;
	HypothesisTailType m_tail;
	QList<double> m_pValue;
	QList<double> m_statisticValue;
};

#endif // HYPOTHESISTEST_H
