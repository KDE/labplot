/***************************************************************************
    File                 : FitTest.cpp
    Project              : LabPlot
    Description          : Tests for data fitting
    --------------------------------------------------------------------
    Copyright            : (C) 2017 Alexander Semke (alexander.semke@web.de)
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

//##############################################################################
//#################  linear regression with NIST datasets ######################
//##############################################################################
void FitTest::testLinearWampler1() {
	//NIST data for Wample5 dataset
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
	//NIST data for Wample5 dataset
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
	//NIST data for Wample3 dataset
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
	//NIST data for Wample4 dataset
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
	//NIST data for Wample5 dataset
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

//##############################################################################
//#########################  Fits with weights #################################
//##############################################################################

QTEST_MAIN(FitTest)
