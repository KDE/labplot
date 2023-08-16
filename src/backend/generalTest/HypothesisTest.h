/*
	File                 : HypothesisTest.h
	Project              : LabPlot
	Description          : Hypothesis Test
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2019  Devanshu Agarwal(agarwaldevanshu8@gmail.com)
	SPDX-FileCopyrightText: 2023 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef HYPOTHESISTEST_H
#define HYPOTHESISTEST_H

#include "GeneralTest.h"

class HypothesisTest : public GeneralTest {
	Q_OBJECT

public:
	explicit HypothesisTest(const QString& name);
	~HypothesisTest() override;

	enum HypothesisTestType {
		// Type
			TTest = 0x01,
			ZTest = 0x02,
			Anova = 0x03,
		// SubType
			TwoSampleIndependent = 0x10,
			TwoSamplePaired = 0x20,
			OneSample = 0x30,
			OneWay = 0x40,
			TwoWay = 0x50
	};
	enum HypothesisTailType {Positive, Negative, Two};

	void setPopulationMean(QVariant populationMean);
	void setSignificanceLevel(QVariant alpha);
	void setTail(HypothesisTailType tail);

	void test(int test, bool categoricalVariable = true, bool equalVariance = true, bool calculateStats = true);
	void leveneTest(bool categoricalVariable);
	void initInputStatsTable(int test, bool calculateStats);

	QList<double>& statisticValue();
	QList<double>& pValue();
	QWidget* view() const override;

private:
	void performTwoSampleIndependentTest(int test, bool categoricalVariable = false, bool equalVariance = true, bool calculateStats = true);
	void performTwoSamplePairedTest(int test);
	void performOneSampleTest(int test);
	void performOneWayAnova();
	void performTwoWayAnova();
	void performLeveneTest(bool categoricalVariable);

	double getPValue(const int &test, double& value,
					 const QString& col1Name, const QString& col2name,
					 const int df);

	QString getPValueTooltip(const double& pValue);

	double m_populationMean;
	double m_significanceLevel;
	HypothesisTailType m_tail;
	QList<double> m_pValue;
	QList<double> m_statisticValue;
};

#endif // HypothesisTest_H
