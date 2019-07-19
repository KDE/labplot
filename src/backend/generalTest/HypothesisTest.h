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

#include "backend/core/AbstractPart.h"
#include "GeneralTest.h"
#include "backend/lib/macros.h"

class HypothesisTestView;
class Spreadsheet;
class QString;
class Column;
class QVBoxLayout;
class QLabel;

class HypothesisTest : public GeneralTest {
	Q_OBJECT

public:
	explicit HypothesisTest(const QString& name);
	~HypothesisTest() override;

    struct Test {
        enum Type {
            NoneType  = 0,
            TTest = 1 << 0,
            ZTest = 1 << 1,
            Anova = 1 << 2
        };
        enum SubType {
            NoneSubType = 0,
            TwoSampleIndependent    = 1 << 0,
            TwoSamplePaired         = 1 << 1,
            OneSample               = 1 << 2,
            OneWay                  = 1 << 3,
            TwoWay                  = 1 << 4
        };
        enum Tail {Positive, Negative, Two};
        Type type = NoneType;
        SubType subtype = NoneSubType;
        Tail tail;
    };

	void setPopulationMean(QVariant populationMean);
	void setSignificanceLevel(QVariant alpha);

    void performTest(Test m_test, bool categoricalVariable = true, bool equalVariance = true);
    void performLeveneTest(bool categoricalVariable);

    QList<double> statisticValue();
    QList<double> pValue();
    QWidget* view() const override;

private:
    void performTwoSampleIndependentTest(HypothesisTest::Test::Type test, bool categoricalVariable = false, bool equalVariance = true);
    void performTwoSamplePairedTest(HypothesisTest::Test::Type test);
    void performOneSampleTest(HypothesisTest::Test::Type test);
    void performOneWayAnova();
    void performTwoWayAnova();
    void m_performLeveneTest(bool categoricalVariable);

    double getPValue(const HypothesisTest::Test::Type& test, double& value,
                     const QString& col1Name, const QString& col2name,
                     const double mean, const double sp, const int df);

    double m_populationMean;
    double m_significanceLevel;
    HypothesisTest::Test::Tail m_tailType;
    QList<double> m_pValue;
    QList<double> m_statisticValue;
};

#endif // HypothesisTest_H
