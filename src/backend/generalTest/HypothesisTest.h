/***************************************************************************
	File                 : HypothesisTest.h
	Project              : LabPlot
	Description          : Doing Hypothesis-Test on data provided
	--------------------------------------------------------------------
	Copyright            : (C) 2019 Devanshu Agarwal(agarwaldevanshu8@gmail.com)

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/

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

	void performTest(int test, bool categoricalVariable = true, bool equalVariance = true, bool calculateStats = true);
	void performLeveneTest(bool categoricalVariable);
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
	void m_performLeveneTest(bool categoricalVariable);

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
