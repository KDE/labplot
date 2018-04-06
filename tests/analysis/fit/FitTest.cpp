/***************************************************************************
    File                 : FitTest.cpp
    Project              : LabPlot
    Description          : Tests for data fitting
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

#include "FitTest.h"
#include "backend/core/column/Column.h"
#include "backend/worksheet/plots/cartesian/XYFitCurve.h"

extern "C" {
#include "backend/nsl/nsl_sf_stats.h"
#include "backend/nsl/nsl_stats.h"
}

void FitTest::initTestCase() {
	// needed in order to have the signals triggered by SignallingUndoCommand, see LabPlot.cpp
	//TODO: redesign/remove this
	qRegisterMetaType<const AbstractAspect*>("const AbstractAspect*");
	qRegisterMetaType<const AbstractColumn*>("const AbstractColumn*");
}

//##############################################################################
//#################  linear regression with NIST datasets ######################
//##############################################################################
void FitTest::testLinearNorris() {
	//NIST data for Norris dataset
	QVector<double> xData = {0.2,337.4,118.2,884.6,10.1,226.5,666.3,996.3,448.6,777.0,558.2,0.4,0.6,775.5,666.9,338.0,447.5,11.6,556.0,228.1,
		995.8,887.6,120.2,0.3,0.3,556.8,339.1,887.2,999.0,779.0,11.1,118.3,229.2,669.1,448.9,0.5};
	QVector<double> yData = {0.1,338.8,118.1,888.0,9.2,228.1,668.5,998.5,449.1,778.9,559.2,0.3,0.1,778.1,668.8,339.3,448.9,10.8,557.7,228.3,
		998.0,888.8,119.6,0.3,0.6,557.6,339.3,888.0,998.5,778.9,10.2,117.6,228.9,668.4,449.2,0.2};

	//data source columns
	Column xDataColumn("x", AbstractColumn::Numeric);
	xDataColumn.replaceValues(0, xData);

	Column yDataColumn("y", AbstractColumn::Numeric);
	yDataColumn.replaceValues(0, yData);

	XYFitCurve fitCurve("fit");
	fitCurve.setXDataColumn(&xDataColumn);
	fitCurve.setYDataColumn(&yDataColumn);

	//prepare the fit
	XYFitCurve::FitData fitData = fitCurve.fitData();
	fitData.modelCategory = nsl_fit_model_basic;
	fitData.modelType = nsl_fit_model_polynomial;
	fitData.degree = 1;
	XYFitCurve::initFitData(fitData);
	fitCurve.setFitData(fitData);

	//perform the fit
	fitCurve.recalculate();
	const XYFitCurve::FitResult& fitResult = fitCurve.fitResult();

	//check the results
	QCOMPARE(fitResult.available, true);
	QCOMPARE(fitResult.valid, true);

	const int np = fitData.paramNames.size();
	QCOMPARE(np, 2);

	QCOMPARE(fitResult.paramValues.at(0),  -0.262323073774029);
	QCOMPARE(fitResult.paramValues.at(1), 1.00211681802045);

	QCOMPARE(fitResult.rsd, 0.884796396144373);
	QCOMPARE(fitResult.rsquare, 0.999993745883712);
	QCOMPARE(fitResult.sse, 26.6173985294224);
	QCOMPARE(fitResult.rms, 0.782864662630069);
}

void FitTest::testLinearWampler1() {
	//NIST data for Wampler1 dataset
	QVector<int> xData = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};
	QVector<int> yData = {1,6,63,364,1365,3906,9331,19608,37449,66430,111111,
		177156,271453,402234,579195,813616,1118481,1508598,2000719,2613660,3368421};

	//data source columns
	Column xDataColumn("x", AbstractColumn::Integer);
	xDataColumn.replaceInteger(0, xData);

	Column yDataColumn("y", AbstractColumn::Integer);
	yDataColumn.replaceInteger(0, yData);

	XYFitCurve fitCurve("fit");
	fitCurve.setXDataColumn(&xDataColumn);
	fitCurve.setYDataColumn(&yDataColumn);

	//prepare the fit
	XYFitCurve::FitData fitData = fitCurve.fitData();
	fitData.modelCategory = nsl_fit_model_basic;
	fitData.modelType = nsl_fit_model_polynomial;
	fitData.degree = 5;
	XYFitCurve::initFitData(fitData);
	fitCurve.setFitData(fitData);

	//perform the fit
	fitCurve.recalculate();
	const XYFitCurve::FitResult& fitResult = fitCurve.fitResult();

	//check the results
	QCOMPARE(fitResult.available, true);
	QCOMPARE(fitResult.valid, true);

	const int np = fitData.paramNames.size();
	QCOMPARE(np, 6);

	for (int i = 0; i < np; i++) {
		const double paramValue = fitResult.paramValues.at(i);
		QCOMPARE(paramValue, 1.0);
	}

	QCOMPARE(fitResult.rsd, 0.0);
	QCOMPARE(fitResult.rsquare,  1.0);
	QCOMPARE(fitResult.sse, 0.0);
	QCOMPARE(fitResult.rms, 0.0);
}

void FitTest::testLinearWampler2() {
	//NIST data for Wampler2 dataset
	QVector<int> xData = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};
	QVector<double> yData = {1.00000,1.11111,1.24992,1.42753,1.65984,1.96875,2.38336,2.94117,3.68928,4.68559,
		6.00000,7.71561,9.92992,12.75603,16.32384,20.78125,26.29536,33.05367,41.26528,51.16209,63.00000};

	//data source columns
	Column xDataColumn("x", AbstractColumn::Integer);
	xDataColumn.replaceInteger(0, xData);

	Column yDataColumn("y", AbstractColumn::Numeric);
	yDataColumn.replaceValues(0, yData);

	XYFitCurve fitCurve("fit");
	fitCurve.setXDataColumn(&xDataColumn);
	fitCurve.setYDataColumn(&yDataColumn);

	//prepare the fit
	XYFitCurve::FitData fitData = fitCurve.fitData();
	fitData.modelCategory = nsl_fit_model_basic;
	fitData.modelType = nsl_fit_model_polynomial;
	fitData.degree = 5;
	XYFitCurve::initFitData(fitData);
	fitCurve.setFitData(fitData);

	//perform the fit
	fitCurve.recalculate();
	const XYFitCurve::FitResult& fitResult = fitCurve.fitResult();

	//check the results
	qDebug()<<"STATUS " << fitResult.status;
	QCOMPARE(fitResult.available, true);
	QCOMPARE(fitResult.valid, true);

	const int np = fitData.paramNames.size();
	QCOMPARE(np, 6);

	QCOMPARE(fitResult.paramValues.at(0), 1.0);
	QCOMPARE(fitResult.paramValues.at(1), 0.1);
	QCOMPARE(fitResult.paramValues.at(2), 0.01);
	QCOMPARE(fitResult.paramValues.at(3), 0.001);
	QCOMPARE(fitResult.paramValues.at(4), 0.0001);
	QCOMPARE(fitResult.paramValues.at(5), 0.00001);

		//TODO: rsd, sse and rms fails with
	//FAIL!  : FitTest::testLinearWampler2() Compared doubles are not the same (fuzzy compare)
	//Actual   (fitResult.rsd): 2,32459e-15
	//Expected (0.0)          : 0
	//etc.
// 	QCOMPARE(fitResult.rsd, 0.000000000000000);
	QCOMPARE(fitResult.rsquare,  1.00000000000000);
// 	QCOMPARE(fitResult.sse, 0.000000000000000);
// 	QCOMPARE(fitResult.rms, 0.000000000000000);
}

void FitTest::testLinearWampler3() {
	//NIST data for Wampler3 dataset
	QVector<int> xData = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};
	QVector<double> yData = {760.,-2042.,2111.,-1684.,3888.,1858.,11379.,17560.,39287.,64382.,113159.,
		175108.,273291.,400186.,581243.,811568.,1121004.,1506550.,2002767.,2611612.,3369180.};

	//data source columns
	Column xDataColumn("x", AbstractColumn::Integer);
	xDataColumn.replaceInteger(0, xData);

	Column yDataColumn("y", AbstractColumn::Numeric);
	yDataColumn.replaceValues(0, yData);

	XYFitCurve fitCurve("fit");
	fitCurve.setXDataColumn(&xDataColumn);
	fitCurve.setYDataColumn(&yDataColumn);

	//prepare the fit
	XYFitCurve::FitData fitData = fitCurve.fitData();
	fitData.modelCategory = nsl_fit_model_basic;
	fitData.modelType = nsl_fit_model_polynomial;
	fitData.degree = 5;
	XYFitCurve::initFitData(fitData);
	fitCurve.setFitData(fitData);

	//perform the fit
	fitCurve.recalculate();
	const XYFitCurve::FitResult& fitResult = fitCurve.fitResult();

	//check the results
	QCOMPARE(fitResult.available, true);
	QCOMPARE(fitResult.valid, true);

	const int np = fitData.paramNames.size();
	QCOMPARE(np, 6);

	for (int i = 0; i < np; i++) {
		const double paramValue = fitResult.paramValues.at(i);
		QCOMPARE(paramValue, 1.0);
	}

	QCOMPARE(fitResult.rsd, 2360.14502379268);
	QCOMPARE(fitResult.rsquare,  0.999995559025820);
	QCOMPARE(fitResult.sse,  83554268.0000000);
	QCOMPARE(fitResult.rms, 5570284.53333333);
}

void FitTest::testLinearWampler4() {
	//NIST data for Wampler4 dataset
	QVector<int> xData = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};
	QVector<int> yData = {75901,-204794,204863,-204436,253665,-200894,214131,-185192,221249,-138370,
		315911,-27644,455253,197434,783995,608816,1370781,1303798,2205519,2408860,3444321};

	//data source columns
	Column xDataColumn("x", AbstractColumn::Integer);
	xDataColumn.replaceInteger(0, xData);

	Column yDataColumn("y", AbstractColumn::Integer);
	yDataColumn.replaceInteger(0, yData);

	XYFitCurve fitCurve("fit");
	fitCurve.setXDataColumn(&xDataColumn);
	fitCurve.setYDataColumn(&yDataColumn);

	//prepare the fit
	XYFitCurve::FitData fitData = fitCurve.fitData();
	fitData.modelCategory = nsl_fit_model_basic;
	fitData.modelType = nsl_fit_model_polynomial;
	fitData.degree = 5;
	XYFitCurve::initFitData(fitData);
	fitCurve.setFitData(fitData);

	//perform the fit
	fitCurve.recalculate();
	const XYFitCurve::FitResult& fitResult = fitCurve.fitResult();

	//check the results
	QCOMPARE(fitResult.available, true);
	QCOMPARE(fitResult.valid, true);

	const int np = fitData.paramNames.size();
	QCOMPARE(np, 6);

	for (int i = 0; i < np; i++) {
		const double paramValue = fitResult.paramValues.at(i);
		QCOMPARE(paramValue, 1.0);
	}

	QCOMPARE(fitResult.rsd, 236014.502379268);
	QCOMPARE(fitResult.rsquare,  0.957478440825662);
	QCOMPARE(fitResult.sse, 835542680000.000);
	QCOMPARE(fitResult.rms, 55702845333.3333);
}

void FitTest::testLinearWampler5() {
	//NIST data for Wampler5 dataset
	QVector<int> xData = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};
	QVector<int> yData = {7590001,-20479994,20480063,-20479636,25231365,-20476094,20489331,-20460392,18417449,-20413570,
				20591111,-20302844,18651453,-20077766,21059195,-19666384,26348481,-18971402,22480719,-17866340,10958421};

	//data source columns
	Column xDataColumn("x", AbstractColumn::Integer);
	xDataColumn.replaceInteger(0, xData);

	Column yDataColumn("y", AbstractColumn::Integer);
	yDataColumn.replaceInteger(0, yData);

	XYFitCurve fitCurve("fit");
	fitCurve.setXDataColumn(&xDataColumn);
	fitCurve.setYDataColumn(&yDataColumn);

	//prepare the fit
	XYFitCurve::FitData fitData = fitCurve.fitData();
	fitData.modelCategory = nsl_fit_model_basic;
	fitData.modelType = nsl_fit_model_polynomial;
	fitData.degree = 5;
	XYFitCurve::initFitData(fitData);
	fitCurve.setFitData(fitData);

	//perform the fit
	fitCurve.recalculate();
	const XYFitCurve::FitResult& fitResult = fitCurve.fitResult();

	//check the results
	QCOMPARE(fitResult.available, true);
	QCOMPARE(fitResult.valid, true);

	const int np = fitData.paramNames.size();
	QCOMPARE(np, 6);

	for (int i = 0; i < np; i++) {
		const double paramValue = fitResult.paramValues.at(i);
		QCOMPARE(paramValue, 1.0);
	}

	QCOMPARE(fitResult.rsd, 23601450.2379268);
	QCOMPARE(fitResult.rsquare, 0.224668921574940E-02);
	QCOMPARE(fitResult.sse, 0.835542680000000E+16);
	QCOMPARE(fitResult.rms, 557028453333333.);
}

//##############################################################################
//#############  non-linear regression with NIST datasets  #####################
//##############################################################################

//TODO

//##############################################################################
//#########################  Fits with weights #################################
//##############################################################################

//TODO

QTEST_MAIN(FitTest)
