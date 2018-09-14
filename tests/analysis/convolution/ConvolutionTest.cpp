/***************************************************************************
    File                 : ConvolutionTest.cpp
    Project              : LabPlot
    Description          : Tests for data convolution
    --------------------------------------------------------------------
    Copyright            : (C) 2017 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2018 Stefan Gerlach (stefan.gerlach@uni.kn)
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

#include "ConvolutionTest.h"
#include "backend/core/column/Column.h"
#include "backend/worksheet/plots/cartesian/XYConvolutionCurve.h"

extern "C" {
#include "backend/nsl/nsl_sf_stats.h"
#include "backend/nsl/nsl_stats.h"
}

void ConvolutionTest::initTestCase() {
	// needed in order to have the signals triggered by SignallingUndoCommand, see LabPlot.cpp
	//TODO: redesign/remove this
	qRegisterMetaType<const AbstractAspect*>("const AbstractAspect*");
	qRegisterMetaType<const AbstractColumn*>("const AbstractColumn*");
}

//##############################################################################
//#################  linear regression with NIST datasets ######################
//##############################################################################
void ConvolutionTest::test1() {
/*	// data
	QVector<double> xData = {0.2,337.4,118.2,884.6,10.1,226.5,666.3,996.3,448.6,777.0,558.2,0.4,0.6,775.5,666.9,338.0,447.5,11.6,556.0,228.1,
		995.8,887.6,120.2,0.3,0.3,556.8,339.1,887.2,999.0,779.0,11.1,118.3,229.2,669.1,448.9,0.5};
	QVector<double> yData = {0.1,338.8,118.1,888.0,9.2,228.1,668.5,998.5,449.1,778.9,559.2,0.3,0.1,778.1,668.8,339.3,448.9,10.8,557.7,228.3,
		998.0,888.8,119.6,0.3,0.6,557.6,339.3,888.0,998.5,778.9,10.2,117.6,228.9,668.4,449.2,0.2};

	//data source columns
	Column xDataColumn("x", AbstractColumn::Numeric);
	xDataColumn.replaceValues(0, xData);

	Column yDataColumn("y", AbstractColumn::Numeric);
	yDataColumn.replaceValues(0, yData);

	XYConvolutionCurve fitCurve("fit");
	fitCurve.setXDataColumn(&xDataColumn);
	fitCurve.setYDataColumn(&yDataColumn);

	//prepare the fit
	XYConvolutionCurve::ConvolutionData fitData = fitCurve.fitData();
	fitData.modelCategory = nsl_fit_model_basic;
	fitData.modelType = nsl_fit_model_polynomial;
	fitData.degree = 1;
	XYConvolutionCurve::initConvolutionData(fitData);
	fitCurve.setConvolutionData(fitData);

	//perform the fit
	fitCurve.recalculate();
	const XYConvolutionCurve::ConvolutionResult& fitResult = fitCurve.fitResult();

	//check the results
	QCOMPARE(fitResult.available, true);
	QCOMPARE(fitResult.valid, true);

	const int np = fitData.paramNames.size();
	QCOMPARE(np, 2);

	QCOMPARE(fitResult.paramValues.at(0), -0.262323073774029);
	QCOMPARE(fitResult.errorValues.at(0), 0.232818234301152);
	QCOMPARE(fitResult.paramValues.at(1), 1.00211681802045);
	QCOMPARE(fitResult.errorValues.at(1), 0.429796848199937e-3);

	QCOMPARE(fitResult.rsd, 0.884796396144373);
	QCOMPARE(fitResult.rsquare, 0.999993745883712);
	QCOMPARE(fitResult.sse, 26.6173985294224);
	QCOMPARE(fitResult.rms, 0.782864662630069);
	DEBUG(std::setprecision(15) << fitResult.fdist_F);	// result: 5436419.54079774
	FuzzyCompare(fitResult.fdist_F, 5436385.54079785, 1.e-5);
*/
}

QTEST_MAIN(ConvolutionTest)
