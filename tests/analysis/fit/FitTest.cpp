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
	Column xDataColumn("x", AbstractColumn::ColumnMode::Numeric);
	xDataColumn.replaceValues(0, xData);

	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
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

	QCOMPARE(fitResult.paramValues.at(0), -0.262323073774029);
	QCOMPARE(fitResult.errorValues.at(0), 0.232818234301152);
	QCOMPARE(fitResult.paramValues.at(1), 1.00211681802045);
	QCOMPARE(fitResult.errorValues.at(1), 0.429796848199937e-3);

	QCOMPARE(fitResult.rsd, 0.884796396144373);
	QCOMPARE(fitResult.rsquare, 0.999993745883712);
	QCOMPARE(fitResult.sse, 26.6173985294224);
	QCOMPARE(fitResult.rms, 0.782864662630069);
	DEBUG(std::setprecision(15) << fitResult.fdist_F);	// result: 5436385.54083098
	FuzzyCompare(fitResult.fdist_F, 5436385.54079785, 1.e-9);
}

void FitTest::testLinearPontius() {
	//NIST data for Pontius dataset
	QVector<int> xData = {150000,300000,450000,600000,750000,900000,1050000,1200000,1350000,1500000,1650000,1800000,1950000,2100000,
		2250000,2400000,2550000,2700000,2850000,3000000,150000,300000,450000,600000,750000,900000,1050000,1200000,1350000,1500000,
		1650000,1800000,1950000,2100000,2250000,2400000,2550000,2700000,2850000,3000000};
	QVector<double> yData = {.11019,.21956,.32949,.43899,.54803,.65694,.76562,.87487,.98292,1.09146,1.20001,1.30822,1.41599,1.52399,
		1.63194,1.73947,1.84646,1.95392,2.06128,2.16844,.11052,.22018,.32939,.43886,.54798,.65739,.76596,.87474,
		.98300,1.09150,1.20004,1.30818,1.41613,1.52408,1.63159,1.73965,1.84696,1.95445,2.06177,2.16829};

	//data source columns
	Column xDataColumn("x", AbstractColumn::ColumnMode::Integer);
	xDataColumn.replaceInteger(0, xData);

	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	XYFitCurve fitCurve("fit");
	fitCurve.setXDataColumn(&xDataColumn);
	fitCurve.setYDataColumn(&yDataColumn);

	//prepare the fit
	XYFitCurve::FitData fitData = fitCurve.fitData();
	fitData.modelCategory = nsl_fit_model_basic;
	fitData.modelType = nsl_fit_model_polynomial;
	fitData.degree = 2;
	XYFitCurve::initFitData(fitData);
	fitCurve.setFitData(fitData);

	//perform the fit
	fitCurve.recalculate();
	const XYFitCurve::FitResult& fitResult = fitCurve.fitResult();

	//check the results
	QCOMPARE(fitResult.available, true);
	QCOMPARE(fitResult.valid, true);

	const int np = fitData.paramNames.size();
	QCOMPARE(np, 3);

	QCOMPARE(fitResult.paramValues.at(0), 0.673565789473684e-3);
	QCOMPARE(fitResult.errorValues.at(0), 0.107938612033077e-3);
	QCOMPARE(fitResult.paramValues.at(1), 0.732059160401003e-6);
	QCOMPARE(fitResult.errorValues.at(1), 0.157817399981659e-9);
	QCOMPARE(fitResult.paramValues.at(2), -0.316081871345029e-14);
	QCOMPARE(fitResult.errorValues.at(2), 0.486652849992036e-16);

	QCOMPARE(fitResult.rsd, 0.205177424076185e-3);
	QCOMPARE(fitResult.rsquare, 0.999999900178537);
	QCOMPARE(fitResult.sse, 0.155761768796992e-5);
	QCOMPARE(fitResult.rms, 0.420977753505385e-7);
	DEBUG(std::setprecision(15) << fitResult.fdist_F);	// result: 185330865.884471
	FuzzyCompare(fitResult.fdist_F, 185330865.995752, 1.e-9);
}

void FitTest::testLinearNoInt1() {
	//NIST data for NoInt1 dataset
	QVector<int> xData = {60,61,62,63,64,65,66,67,68,69,70};
	QVector<int> yData = {130,131,132,133,134,135,136,137,138,139,140};

	//data source columns
	Column xDataColumn("x", AbstractColumn::ColumnMode::Integer);
	xDataColumn.replaceInteger(0, xData);

	Column yDataColumn("y", AbstractColumn::ColumnMode::Integer);
	yDataColumn.replaceInteger(0, yData);

	XYFitCurve fitCurve("fit");
	fitCurve.setXDataColumn(&xDataColumn);
	fitCurve.setYDataColumn(&yDataColumn);

	//prepare the fit
	XYFitCurve::FitData fitData = fitCurve.fitData();
	fitData.modelCategory = nsl_fit_model_custom;
	XYFitCurve::initFitData(fitData);
	fitData.model = "b1*x";
	fitData.paramNames << "b1";
	const int np = fitData.paramNames.size();
	fitData.paramStartValues << 1.;
	fitData.paramLowerLimits << -std::numeric_limits<double>::max();
	fitData.paramUpperLimits << std::numeric_limits<double>::max();
	//fitData.eps = 1.e-15;
	fitCurve.setFitData(fitData);

	//perform the fit
	fitCurve.recalculate();
	const XYFitCurve::FitResult& fitResult = fitCurve.fitResult();

	//check the results
	QCOMPARE(fitResult.available, true);
	QCOMPARE(fitResult.valid, true);

	QCOMPARE(np, 1);

	DEBUG(std::setprecision(15) << fitResult.paramValues.at(0));	// result: 2.07438016513166
	FuzzyCompare(fitResult.paramValues.at(0), 2.07438016528926, 1.e-9);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(0));	// result: 0.0165289256079047
	FuzzyCompare(fitResult.errorValues.at(0), 0.165289256198347e-1, 1.e-9);

	QCOMPARE(fitResult.rsd, 3.56753034006338);
	QCOMPARE(fitResult.sse, 127.272727272727);
	QCOMPARE(fitResult.rms, 12.7272727272727);
	QCOMPARE(fitResult.rsquare, 0.999365492298663);
	DEBUG(std::setprecision(15) << fitResult.fdist_F);	// result: 15750.2500000027
	QCOMPARE(fitResult.fdist_F, 15750.25);
}

void FitTest::testLinearNoInt1_2() {
	//NIST data for NoInt1 dataset
	QVector<int> xData = {60,61,62,63,64,65,66,67,68,69,70};
	QVector<int> yData = {130,131,132,133,134,135,136,137,138,139,140};

	//data source columns
	Column xDataColumn("x", AbstractColumn::ColumnMode::Integer);
	xDataColumn.replaceInteger(0, xData);

	Column yDataColumn("y", AbstractColumn::ColumnMode::Integer);
	yDataColumn.replaceInteger(0, yData);

	XYFitCurve fitCurve("fit");
	fitCurve.setXDataColumn(&xDataColumn);
	fitCurve.setYDataColumn(&yDataColumn);

	//prepare the fit
	XYFitCurve::FitData fitData = fitCurve.fitData();
	fitData.modelCategory = nsl_fit_model_basic;
	fitData.modelType = nsl_fit_model_polynomial;
	fitData.degree = 1;
	XYFitCurve::initFitData(fitData);
	fitData.paramStartValues[0] = 0;
	fitData.paramFixed[0] = true;
	fitCurve.setFitData(fitData);

	//perform the fit
	fitCurve.recalculate();
	const XYFitCurve::FitResult& fitResult = fitCurve.fitResult();

	//check the results
	QCOMPARE(fitResult.available, true);
	QCOMPARE(fitResult.valid, true);

	const int np = fitData.paramNames.size();
	QCOMPARE(np, 2);

	QCOMPARE(fitResult.paramValues.at(0), 0.);
	QCOMPARE(fitResult.paramValues.at(1), 2.07438016528926);
	QCOMPARE(fitResult.errorValues.at(1), 0.165289256198347e-1);

	QCOMPARE(fitResult.rsd, 3.56753034006338);
	QCOMPARE(fitResult.sse, 127.272727272727);
	QCOMPARE(fitResult.rms, 12.7272727272727);
	QCOMPARE(fitResult.rsquare, 0.999365492298663);
	DEBUG(std::setprecision(15) << fitResult.fdist_F);	// result: 15760.25
	QCOMPARE(fitResult.fdist_F, 15750.25);
}

void FitTest::testLinearNoInt2() {
	//NIST data for NoInt2 dataset
	QVector<int> xData = {4,5,6};
	QVector<int> yData = {3,4,4};

	//data source columns
	Column xDataColumn("x", AbstractColumn::ColumnMode::Integer);
	xDataColumn.replaceInteger(0, xData);

	Column yDataColumn("y", AbstractColumn::ColumnMode::Integer);
	yDataColumn.replaceInteger(0, yData);

	XYFitCurve fitCurve("fit");
	fitCurve.setXDataColumn(&xDataColumn);
	fitCurve.setYDataColumn(&yDataColumn);

	//prepare the fit
	XYFitCurve::FitData fitData = fitCurve.fitData();
	fitData.modelCategory = nsl_fit_model_custom;
	XYFitCurve::initFitData(fitData);
	fitData.model = "c * x";
	fitData.paramNames << "c";
	const int np = fitData.paramNames.size();
	fitData.paramStartValues << 1.;
	fitData.paramLowerLimits << -std::numeric_limits<double>::max();
	fitData.paramUpperLimits << std::numeric_limits<double>::max();
	//fitData.eps = 1.e-15;
	fitCurve.setFitData(fitData);

	//perform the fit
	fitCurve.recalculate();
	const XYFitCurve::FitResult& fitResult = fitCurve.fitResult();

	//check the results
	QCOMPARE(fitResult.available, true);
	QCOMPARE(fitResult.valid, true);

	QCOMPARE(np, 1);

	DEBUG(std::setprecision(15) << fitResult.paramValues.at(0));	// result: 0.727272727152573
	FuzzyCompare(fitResult.paramValues.at(0), 0.727272727272727, 1.e-9);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(0));	// result: 0.0420827316561797
	FuzzyCompare(fitResult.errorValues.at(0), 0.420827318078432E-01, 1.e-8);

	QCOMPARE(fitResult.rsd, 0.369274472937998);
	QCOMPARE(fitResult.sse, 0.272727272727273);
	QCOMPARE(fitResult.rms, 0.136363636363636);
// can not detect that intercept is zero for a custom linear model
	DEBUG(std::setprecision(15) << fitResult.rsquare);	// result: 0.590909090909091
//	QCOMPARE(fitResult.rsquare, 0.993348115299335);
	DEBUG(std::setprecision(15) << fitResult.fdist_F);	// result: 2.88888888888889
//	FuzzyCompare(fitResult.fdist_F, 298.666666666667, 1.);
}

void FitTest::testLinearNoInt2_2() {
	//NIST data for NoInt2 dataset
	QVector<int> xData = {4,5,6};
	QVector<int> yData = {3,4,4};

	//data source columns
	Column xDataColumn("x", AbstractColumn::ColumnMode::Integer);
	xDataColumn.replaceInteger(0, xData);

	Column yDataColumn("y", AbstractColumn::ColumnMode::Integer);
	yDataColumn.replaceInteger(0, yData);

	XYFitCurve fitCurve("fit");
	fitCurve.setXDataColumn(&xDataColumn);
	fitCurve.setYDataColumn(&yDataColumn);

	//prepare the fit
	XYFitCurve::FitData fitData = fitCurve.fitData();
	fitData.modelCategory = nsl_fit_model_basic;
	fitData.modelType = nsl_fit_model_polynomial;
	fitData.degree = 1;
	XYFitCurve::initFitData(fitData);
	fitData.paramStartValues[0] = 0;
	fitData.paramFixed[0] = true;
	fitCurve.setFitData(fitData);

	//perform the fit
	fitCurve.recalculate();
	const XYFitCurve::FitResult& fitResult = fitCurve.fitResult();

	//check the results
	QCOMPARE(fitResult.available, true);
	QCOMPARE(fitResult.valid, true);

	const int np = fitData.paramNames.size();
	QCOMPARE(np, 2);

	QCOMPARE(fitResult.paramValues.at(0), 0.);
	QCOMPARE(fitResult.paramValues.at(1),  0.727272727272727);
	QCOMPARE(fitResult.errorValues.at(1), 0.420827318078432e-1);

	QCOMPARE(fitResult.rsd, 0.369274472937998);
	QCOMPARE(fitResult.sse, 0.272727272727273);
	QCOMPARE(fitResult.rms, 0.136363636363636);
	QCOMPARE(fitResult.rsquare, 0.993348115299335);
	DEBUG(std::setprecision(15) << fitResult.fdist_F);	// result: 300.666666666667
	QCOMPARE(fitResult.fdist_F, 298.666666666667);
}

void FitTest::testLinearFilip() {
	//NIST data for Filip dataset
	QVector<double> xData = {-6.860120914,-4.324130045,-4.358625055,-4.358426747,-6.955852379,-6.661145254,-6.355462942,-6.118102026,
		-7.115148017,-6.815308569,-6.519993057,-6.204119983,-5.853871964,-6.109523091,-5.79832982,-5.482672118,-5.171791386,-4.851705903,
		-4.517126416,-4.143573228,-3.709075441,-3.499489089,-6.300769497,-5.953504836,-5.642065153,-5.031376979,-4.680685696,-4.329846955,
		-3.928486195,-8.56735134,-8.363211311,-8.107682739,-7.823908741,-7.522878745,-7.218819279,-6.920818754,-6.628932138,-6.323946875,
		-5.991399828,-8.781464495,-8.663140179,-8.473531488,-8.247337057,-7.971428747,-7.676129393,-7.352812702,-7.072065318,-6.774174009,
		-6.478861916,-6.159517513,-6.835647144,-6.53165267,-6.224098421,-5.910094889,-5.598599459,-5.290645224,-4.974284616,-4.64454848,
		-4.290560426,-3.885055584,-3.408378962,-3.13200249,-8.726767166,-8.66695597,-8.511026475,-8.165388579,-7.886056648,-7.588043762,
		-7.283412422,-6.995678626,-6.691862621,-6.392544977,-6.067374056,-6.684029655,-6.378719832,-6.065855188,-5.752272167,-5.132414673,
		-4.811352704,-4.098269308,-3.66174277,-3.2644011};
	QVector<double> yData = {0.8116,0.9072,0.9052,0.9039,0.8053,0.8377,0.8667,0.8809,0.7975,0.8162,0.8515,0.8766,0.8885,0.8859,0.8959,0.8913,
		0.8959,0.8971,0.9021,0.909,0.9139,0.9199,0.8692,0.8872,0.89,0.891,0.8977,0.9035,0.9078,0.7675,0.7705,0.7713,0.7736,0.7775,0.7841,
		0.7971,0.8329,0.8641,0.8804,0.7668,0.7633,0.7678,0.7697,0.77,0.7749,0.7796,0.7897,0.8131,0.8498,0.8741,0.8061,0.846,0.8751,0.8856,
		0.8919,0.8934,0.894,0.8957,0.9047,0.9129,0.9209,0.9219,0.7739,0.7681,0.7665,0.7703,0.7702,0.7761,0.7809,0.7961,0.8253,0.8602,
		0.8809,0.8301,0.8664,0.8834,0.8898,0.8964,0.8963,0.9074,0.9119,0.9228};

	//data source columns
	Column xDataColumn("x", AbstractColumn::ColumnMode::Numeric);
	xDataColumn.replaceValues(0, xData);

	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	XYFitCurve fitCurve("fit");
	fitCurve.setXDataColumn(&xDataColumn);
	fitCurve.setYDataColumn(&yDataColumn);

	//prepare the fit
	XYFitCurve::FitData fitData = fitCurve.fitData();
	fitData.modelCategory = nsl_fit_model_basic;
	fitData.modelType = nsl_fit_model_polynomial;
	fitData.degree = 10;
	fitData.eps = 1.e-8;
	XYFitCurve::initFitData(fitData);
	const int np = fitData.paramNames.size();
	fitCurve.setFitData(fitData);

	//perform the fit
	fitCurve.recalculate();
	const XYFitCurve::FitResult& fitResult = fitCurve.fitResult();

	//check the results
	QCOMPARE(fitResult.available, true);
	QCOMPARE(fitResult.valid, true);

	QCOMPARE(np, 11);

	DEBUG(std::setprecision(15) << fitResult.paramValues.at(0));	// result: -1467.48962615175
	FuzzyCompare(fitResult.paramValues.at(0), -1467.48961422980, 1.e-5);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(0));	// result: 298.084524514884
	FuzzyCompare(fitResult.errorValues.at(0), 298.084530995537, 1.e-7);
	DEBUG(std::setprecision(15) << fitResult.paramValues.at(1));	// result: -2772.1796150428
	FuzzyCompare(fitResult.paramValues.at(1), -2772.17959193342, 1.e-5);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(1));	// result: 559.779853249694
	FuzzyCompare(fitResult.errorValues.at(1), 559.779865474950, 1.e-7);
	DEBUG(std::setprecision(15) << fitResult.paramValues.at(2));	// result: -2316.37110148409
	FuzzyCompare(fitResult.paramValues.at(2), -2316.37108160893, 1.e-5);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(2));	// result: 466.477561928144
	FuzzyCompare(fitResult.errorValues.at(2), 466.477572127796, 1.e-7);
	DEBUG(std::setprecision(15) << fitResult.paramValues.at(3));	// result: -1127.97395097195
	FuzzyCompare(fitResult.paramValues.at(3), -1127.97394098372, 1.e-5);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(3));	// result: 227.204269523115
	FuzzyCompare(fitResult.errorValues.at(3), 227.204274477751, 1.e-6);
	DEBUG(std::setprecision(15) << fitResult.paramValues.at(4));	// result: -354.478236951913
	FuzzyCompare(fitResult.paramValues.at(4), -354.478233703349, 1.e-5);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(4));	// result: 71.6478645361214
	FuzzyCompare(fitResult.errorValues.at(4), 71.6478660875927, 1.e-7);
	DEBUG(std::setprecision(15) << fitResult.paramValues.at(5));	// result: -75.1242024539908
	FuzzyCompare(fitResult.paramValues.at(5), -75.1242017393757, 1.e-5);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(5));	// result: 15.289717547564
	FuzzyCompare(fitResult.errorValues.at(5), 15.2897178747400, 1.e-7);
	DEBUG(std::setprecision(15) << fitResult.paramValues.at(6));	// result: -10.875318143236
	FuzzyCompare(fitResult.paramValues.at(6), -10.8753180355343, 1.e-5);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(6));	// result: 2.23691155110776
	FuzzyCompare(fitResult.errorValues.at(6), 2.23691159816033, 1.e-7);
	DEBUG(std::setprecision(15) << fitResult.paramValues.at(7));	// result: -1.06221499687347
	FuzzyCompare(fitResult.paramValues.at(7), -1.06221498588947, 1.e-5);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(7));	// result: 0.221624317377432
	FuzzyCompare(fitResult.errorValues.at(7), 0.221624321934227, 1.e-7);
	DEBUG(std::setprecision(15) << fitResult.paramValues.at(8));	// result: -0.0670191161850038
	FuzzyCompare(fitResult.paramValues.at(8), -0.670191154593408E-01, 1.e-5);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(8));	// result: 0.0142363760310402
	FuzzyCompare(fitResult.errorValues.at(8), 0.142363763154724E-01, 1.e-7);
	DEBUG(std::setprecision(15) << fitResult.paramValues.at(9));	// result: -0.00246781081080665
	FuzzyCompare(fitResult.paramValues.at(9), -0.246781078275479E-02, 1.e-5);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(9));	// result: 0.000535617398555022
	FuzzyCompare(fitResult.errorValues.at(9), 0.535617408889821E-03, 1.e-7);
	DEBUG(std::setprecision(15) << fitResult.paramValues.at(10));	// result: -4.02962529900222e-05
	FuzzyCompare(fitResult.paramValues.at(10), -0.402962525080404E-04, 1.e-5);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(10));	// result: 8.96632820770946e-06
	FuzzyCompare(fitResult.errorValues.at(10), 0.896632837373868E-05, 1.e-7);

	DEBUG(std::setprecision(15) << fitResult.rsd);	// result: 0.00334801050105949
	FuzzyCompare(fitResult.rsd, 0.334801051324544E-02, 1.e-7);
	DEBUG(std::setprecision(15) << fitResult.rsquare);	// result: 0.996727416209443
	FuzzyCompare(fitResult.rsquare, 0.996727416185620, 1.e-9);
	DEBUG(std::setprecision(15) << fitResult.sse);	// result: 0.00079585137637953
	FuzzyCompare(fitResult.sse, 0.795851382172941E-03, 1.e-7);
	DEBUG(std::setprecision(15) << fitResult.rms);	// result: 1.12091743152047e-05
	FuzzyCompare(fitResult.rms, 0.112091743968020E-04, 1.e-7);
	DEBUG(std::setprecision(15) << fitResult.fdist_F);	// result: 2162.43960297897
	FuzzyCompare(fitResult.fdist_F, 2162.43954511489, 1.e-7);
}


void FitTest::testLinearWampler1() {
	//NIST data for Wampler1 dataset
	QVector<int> xData = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};
	QVector<int> yData = {1,6,63,364,1365,3906,9331,19608,37449,66430,111111,
		177156,271453,402234,579195,813616,1118481,1508598,2000719,2613660,3368421};

	//data source columns
	Column xDataColumn("x", AbstractColumn::ColumnMode::Integer);
	xDataColumn.replaceInteger(0, xData);

	Column yDataColumn("y", AbstractColumn::ColumnMode::Integer);
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
		const double errorValue = fitResult.errorValues.at(i);
		QCOMPARE(paramValue, 1.0);
		QCOMPARE(errorValue, 0.0);
	}

	QCOMPARE(fitResult.rsd, 0.0);
	QCOMPARE(fitResult.rsquare,  1.0);
	QCOMPARE(fitResult.sse, 0.0);
	QCOMPARE(fitResult.rms, 0.0);
	QVERIFY(std::isinf(fitResult.fdist_F));
}

void FitTest::testLinearWampler2() {
	//NIST data for Wampler2 dataset
	QVector<int> xData = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};
	QVector<double> yData = {1.00000,1.11111,1.24992,1.42753,1.65984,1.96875,2.38336,2.94117,3.68928,4.68559,
		6.00000,7.71561,9.92992,12.75603,16.32384,20.78125,26.29536,33.05367,41.26528,51.16209,63.00000};

	//data source columns
	Column xDataColumn("x", AbstractColumn::ColumnMode::Integer);
	xDataColumn.replaceInteger(0, xData);

	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
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
	qDebug() << "STATUS " << fitResult.status;
	QCOMPARE(fitResult.available, true);
	QCOMPARE(fitResult.valid, true);

	const int np = fitData.paramNames.size();
	QCOMPARE(np, 6);

	for (int i = 0; i < np; i++) {
		DEBUG(std::setprecision(15) << fitResult.paramValues.at(i));
	}
	QCOMPARE(fitResult.paramValues.at(0), 1.0);
	QCOMPARE(fitResult.paramValues.at(1), 0.1);
	QCOMPARE(fitResult.paramValues.at(2), 0.01);
	QCOMPARE(fitResult.paramValues.at(3), 0.001);
	QCOMPARE(fitResult.paramValues.at(4), 0.0001);
	QCOMPARE(fitResult.paramValues.at(5), 0.00001);
	for (int i = 0; i < np; i++) {
		const double errorValue = fitResult.errorValues.at(i);
		DEBUG(std::setprecision(15) << errorValue);	// max. result: 2.32794076549904e-15
		FuzzyCompare(errorValue, 0., 1.e-14);
	}

	DEBUG(std::setprecision(15) << fitResult.rsd);	// result: 2.32458538254974e-15
	FuzzyCompare(fitResult.rsd, 0., 1.e-14);
	QCOMPARE(fitResult.rsquare, 1.);
	DEBUG(std::setprecision(15) << fitResult.sse);	// result: 8.1055458011459e-29
	FuzzyCompare(fitResult.sse, 0., 1.e-15);
	DEBUG(std::setprecision(15) << fitResult.rms);	// result: 5.40369720076393e-30
	FuzzyCompare(fitResult.rms, 0., 1.e-15);
	DEBUG(std::setprecision(15) << fitResult.fdist_F);	// result: 2.44385217688297e+32
	QVERIFY(fitResult.fdist_F > 1.e+32);
}

void FitTest::testLinearWampler3() {
	//NIST data for Wampler3 dataset
	QVector<int> xData = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};
	QVector<double> yData = {760.,-2042.,2111.,-1684.,3888.,1858.,11379.,17560.,39287.,64382.,113159.,
		175108.,273291.,400186.,581243.,811568.,1121004.,1506550.,2002767.,2611612.,3369180.};

	//data source columns
	Column xDataColumn("x", AbstractColumn::ColumnMode::Integer);
	xDataColumn.replaceInteger(0, xData);

	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
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
	QCOMPARE(fitResult.errorValues.at(0), 2152.32624678170);
	QCOMPARE(fitResult.errorValues.at(1), 2363.55173469681);
	QCOMPARE(fitResult.errorValues.at(2), 779.343524331583);
	QCOMPARE(fitResult.errorValues.at(3), 101.475507550350);
	QCOMPARE(fitResult.errorValues.at(4), 5.64566512170752);
	QCOMPARE(fitResult.errorValues.at(5), 0.112324854679312);

	QCOMPARE(fitResult.rsd, 2360.14502379268);
	QCOMPARE(fitResult.rsquare, 0.999995559025820);
	QCOMPARE(fitResult.sse, 83554268.0000000);
	QCOMPARE(fitResult.rms, 5570284.53333333);
	DEBUG(std::setprecision(15) << fitResult.fdist_F);	// result: 675524.458247789
	FuzzyCompare(fitResult.fdist_F, 675524.458240122, 1.e-7);
}

void FitTest::testLinearWampler4() {
	//NIST data for Wampler4 dataset
	QVector<int> xData = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};
	QVector<int> yData = {75901,-204794,204863,-204436,253665,-200894,214131,-185192,221249,-138370,
		315911,-27644,455253,197434,783995,608816,1370781,1303798,2205519,2408860,3444321};

	//data source columns
	Column xDataColumn("x", AbstractColumn::ColumnMode::Integer);
	xDataColumn.replaceInteger(0, xData);

	Column yDataColumn("y", AbstractColumn::ColumnMode::Integer);
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

	FuzzyCompare(fitResult.paramValues.at(0), 1.0, 3.e-9);	// i386: 1.00000000223515
	FuzzyCompare(fitResult.paramValues.at(1), 1.0, 5.e-9);	// i386: 0.999999995021441
	FuzzyCompare(fitResult.paramValues.at(2), 1.0, 2.e-9);	// i386: 1.00000000188395
	FuzzyCompare(fitResult.paramValues.at(3), 1.0, 1.e-9);	// i386: 0.999999999743725
	FuzzyCompare(fitResult.paramValues.at(4), 1.0, 2.e-11);	// i386: 1.00000000001441
	FuzzyCompare(fitResult.paramValues.at(5), 1.0);		// i386: 0.999999999999714

	QCOMPARE(fitResult.errorValues.at(0), 215232.624678170);
	QCOMPARE(fitResult.errorValues.at(1), 236355.173469681);
	QCOMPARE(fitResult.errorValues.at(2), 77934.3524331583);
	QCOMPARE(fitResult.errorValues.at(3), 10147.5507550350);
	QCOMPARE(fitResult.errorValues.at(4), 564.566512170752);
	QCOMPARE(fitResult.errorValues.at(5), 11.2324854679312);

	QCOMPARE(fitResult.rsd, 236014.502379268);
	QCOMPARE(fitResult.rsquare, 0.957478440825662);
	QCOMPARE(fitResult.sse, 835542680000.000);
	QCOMPARE(fitResult.rms, 55702845333.3333);
	DEBUG(std::setprecision(15) << fitResult.fdist_F);	// result: 67.5524458240122
	QCOMPARE(fitResult.fdist_F, 67.5524458240122);
}

void FitTest::testLinearWampler5() {
	//NIST data for Wampler5 dataset
	QVector<int> xData = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};
	QVector<int> yData = {7590001,-20479994,20480063,-20479636,25231365,-20476094,20489331,-20460392,18417449,-20413570,
				20591111,-20302844,18651453,-20077766,21059195,-19666384,26348481,-18971402,22480719,-17866340,10958421};

	//data source columns
	Column xDataColumn("x", AbstractColumn::ColumnMode::Integer);
	xDataColumn.replaceInteger(0, xData);

	Column yDataColumn("y", AbstractColumn::ColumnMode::Integer);
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
	QCOMPARE(fitResult.errorValues.at(0), 21523262.4678170);
	QCOMPARE(fitResult.errorValues.at(1), 23635517.3469681);
	QCOMPARE(fitResult.errorValues.at(2), 7793435.24331583);
	QCOMPARE(fitResult.errorValues.at(3), 1014755.07550350);
	QCOMPARE(fitResult.errorValues.at(4), 56456.6512170752);
	QCOMPARE(fitResult.errorValues.at(5), 1123.24854679312);

	QCOMPARE(fitResult.rsd, 23601450.2379268);
	QCOMPARE(fitResult.rsquare, 0.224668921574940E-02);
	QCOMPARE(fitResult.sse, 0.835542680000000E+16);
	QCOMPARE(fitResult.rms, 557028453333333.);
	DEBUG(std::setprecision(15) << fitResult.fdist_F);	// result: 0.00675524458240069
	QCOMPARE(fitResult.fdist_F, 0.675524458240122E-02);
}

// taken from https://en.wikipedia.org/wiki/Ordinary_least_squares
void FitTest::testLinearWP_OLS() {
	//data from  The World Almanac and Book of Facts, 1975
	QVector<double> xData = {1.47, 1.50, 1.52, 1.55, 1.57, 1.60, 1.63, 1.65, 1.68, 1.70, 1.73, 1.75, 1.78, 1.80, 1.83};
	QVector<double> yData = {52.21, 53.12, 54.48, 55.84, 57.20, 58.57, 59.93, 61.29, 63.11, 64.47, 66.28, 68.10, 69.92, 72.19, 74.46};

	//data source columns
	Column xDataColumn("x", AbstractColumn::ColumnMode::Numeric);
	xDataColumn.replaceValues(0, xData);

	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	XYFitCurve fitCurve("fit");
	fitCurve.setXDataColumn(&xDataColumn);
	fitCurve.setYDataColumn(&yDataColumn);

	//prepare the fit
	XYFitCurve::FitData fitData = fitCurve.fitData();
	fitData.modelCategory = nsl_fit_model_basic;
	fitData.modelType = nsl_fit_model_polynomial;
	fitData.degree = 2;
	XYFitCurve::initFitData(fitData);
	fitCurve.setFitData(fitData);

	//perform the fit
	fitCurve.recalculate();
	const XYFitCurve::FitResult& fitResult = fitCurve.fitResult();

	//check the results
	QCOMPARE(fitResult.available, true);
	QCOMPARE(fitResult.valid, true);

	const int np = fitData.paramNames.size();
	QCOMPARE(np, 3);

	DEBUG(std::setprecision(15) << fitResult.paramValues.at(0));	// result: 128.812803578436
	FuzzyCompare(fitResult.paramValues.at(0), 128.8128, 1.e-7);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(0));	// result: 16.3082821390367
	FuzzyCompare(fitResult.errorValues.at(0), 16.3083, 1.e-5);
	DEBUG(std::setprecision(15) << fitResult.tdist_tValues.at(0));	// result: 7.89861264848368
	FuzzyCompare(fitResult.tdist_tValues.at(0), 7.8986, 1.e-5);
	DEBUG(std::setprecision(15) << fitResult.tdist_pValues.at(0));	// result: 4.28330815316414e-06
	FuzzyCompare(fitResult.tdist_pValues.at(0), 0., 1.e-5);
	DEBUG(std::setprecision(15) << fitResult.paramValues.at(1));	// result: -143.16202286476
	FuzzyCompare(fitResult.paramValues.at(1), -143.1620, 1.e-6);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(1));	// result: 19.8331710430895
	FuzzyCompare(fitResult.errorValues.at(1), 19.8332, 1.e-5);
	DEBUG(std::setprecision(15) << fitResult.tdist_tValues.at(1));	// result: -7.21831231897945
	FuzzyCompare(fitResult.tdist_tValues.at(1), -7.2183, 1.e-5);
	DEBUG(std::setprecision(15) << fitResult.tdist_pValues.at(1));	// result: 1.05970640905074e-05
	FuzzyCompare(fitResult.tdist_pValues.at(1), 0., 2.e-5);
	DEBUG(std::setprecision(15) << fitResult.paramValues.at(2));	// result: 61.9603254424724
	FuzzyCompare(fitResult.paramValues.at(2), 61.9603, 1.e-6);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(2));	// result: 6.00842899301227
	FuzzyCompare(fitResult.errorValues.at(2), 6.0084, 1.e-5);
	DEBUG(std::setprecision(15) << fitResult.tdist_tValues.at(2));	// result: 10.3122339490958
	FuzzyCompare(fitResult.tdist_tValues.at(2), 10.3122, 1.e-5);
	DEBUG(std::setprecision(15) << fitResult.tdist_pValues.at(2));	// result: 2.56647515320682e-07
	FuzzyCompare(fitResult.tdist_pValues.at(2), 0., 1.e-6);

	DEBUG(std::setprecision(15) << fitResult.rsd);	// result: 0.25158650082898
	FuzzyCompare(fitResult.rsd, 0.2516, 1.e-4);
	DEBUG(std::setprecision(15) << fitResult.rsquare);	// result: 0.998904558436583
	FuzzyCompare(fitResult.rsquare, 0.9989, 1.e-5);
	DEBUG(std::setprecision(15) << fitResult.rsquareAdj);	// result: 0.99860580164656
	FuzzyCompare(fitResult.rsquareAdj, 0.9987, 1.e-4);
	DEBUG(std::setprecision(15) << fitResult.sse);	// result: 0.759549208792447
	FuzzyCompare(fitResult.sse, 0.7595, 1.e-4);
//	QCOMPARE(fitResult.rms, ???);	// result: 0.0632958
	DEBUG(std::setprecision(15) << fitResult.chisq_p);	// result: 0.999996987409119
//	FuzzyCompare(fitResult.chisq_p, ???, 1.e-8);
	DEBUG(std::setprecision(15) << fitResult.fdist_F);	// result: 5471.2433330734
	FuzzyCompare(fitResult.fdist_F, 5471.2, 1.e-5);
	DEBUG(std::setprecision(15) << fitResult.fdist_p);	// result: 0.0
	QCOMPARE(fitResult.fdist_p, 0.0);
	DEBUG(std::setprecision(15) << fitResult.logLik);	// result: 1.0890247702592
	FuzzyCompare(fitResult.logLik, 1.0890, 3.e-5);
	DEBUG(std::setprecision(15) << fitResult.aic);	// result: 5.82195045948161
// not reproducable
//	FuzzyCompare(fitResult.aic, 0.2548, 2.e-6);
	DEBUG(std::setprecision(15) << fitResult.bic);	// result: 8.65415126389045
// not reproducable
//	FuzzyCompare(fitResult.bic, 0.3964, 2.e-6);
}

// from http://sia.webpopix.org/polynomialRegression1.html
void FitTest::testLinearR_lm2() {
	QVector<int> xData = {4,4,7,7,8,9,10,10,10,11,11,12,12,12,12,13,13,13,13,14,14,14,14,15,15,15,16,16,17,
		17,17,18,18,18,18,19,19,19,20,20,20,20,20,22,23,24,24,24,24,25};
	QVector<int> yData = {2,10,4,22,16,10,18,26,34,17,28,14,20,24,28,26,34,34,46,26,36,60,80,20,26,54,32,40,32,40,50,42,56,76,84,36,46,
                68,32,48,52,56,64,66,54,70,92,93,120,85};

	//data source columns
	Column xDataColumn("x", AbstractColumn::ColumnMode::Integer);
	xDataColumn.replaceInteger(0, xData);

	Column yDataColumn("y", AbstractColumn::ColumnMode::Integer);
	yDataColumn.replaceInteger(0, yData);

	XYFitCurve fitCurve("fit");
	fitCurve.setXDataColumn(&xDataColumn);
	fitCurve.setYDataColumn(&yDataColumn);

	//prepare the fit
	XYFitCurve::FitData fitData = fitCurve.fitData();
	fitData.modelCategory = nsl_fit_model_basic;
	fitData.modelType = nsl_fit_model_polynomial;
	fitData.degree = 2;
	XYFitCurve::initFitData(fitData);
	fitCurve.setFitData(fitData);

	//perform the fit
	fitCurve.recalculate();
	const XYFitCurve::FitResult& fitResult = fitCurve.fitResult();

	//check the results
	QCOMPARE(fitResult.available, true);
	QCOMPARE(fitResult.valid, true);

	const int np = fitData.paramNames.size();
	QCOMPARE(np, 3);

	DEBUG(std::setprecision(15) << fitResult.paramValues.at(0));	// result: 2.47013778506623
	FuzzyCompare(fitResult.paramValues.at(0), 2.47014, 1.e-6);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(0));	// result: 14.8171647250237
	FuzzyCompare(fitResult.errorValues.at(0), 14.81716, 1.e-6);
	DEBUG(std::setprecision(15) << fitResult.tdist_tValues.at(0));	// result: 0.16670785746848
	FuzzyCompare(fitResult.tdist_tValues.at(0), 0.167, 2.e-3);
	DEBUG(std::setprecision(15) << fitResult.tdist_pValues.at(0));	// result: 0.868315075848582
	FuzzyCompare(fitResult.tdist_pValues.at(0), 0.868, 1.e-3);
	DEBUG(std::setprecision(15) << fitResult.paramValues.at(1));	// result: 0.913287614242592
	FuzzyCompare(fitResult.paramValues.at(1), 0.91329, 1.e-5);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(1));	// result: 2.03422044231195
	FuzzyCompare(fitResult.errorValues.at(1), 2.03422, 1.e-6);
	DEBUG(std::setprecision(15) << fitResult.tdist_tValues.at(1));	// result: 0.448961968548804
	FuzzyCompare(fitResult.tdist_tValues.at(1), 0.449, 1.e-4);
	DEBUG(std::setprecision(15) << fitResult.tdist_pValues.at(1));	// result: 0.655522449402813
	FuzzyCompare(fitResult.tdist_pValues.at(1), 0.656, 1.e-3);
	DEBUG(std::setprecision(15) << fitResult.paramValues.at(2));	// result: 0.0999593020698437
	FuzzyCompare(fitResult.paramValues.at(2), 0.09996, 1.e-5);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(2));	// result: 0.0659682106823392
	FuzzyCompare(fitResult.errorValues.at(2), 0.06597, 1.e-4);
	DEBUG(std::setprecision(15) << fitResult.tdist_tValues.at(2));	// result: 1.5152647166858
	FuzzyCompare(fitResult.tdist_tValues.at(2), 1.515, 1.e-3);
	DEBUG(std::setprecision(15) << fitResult.tdist_pValues.at(2));	// result: 0.136402432803739
	FuzzyCompare(fitResult.tdist_pValues.at(2), 0.136, 3.e-3);

	DEBUG(std::setprecision(15) << fitResult.rsd);	// result: 15.1760701243277
	FuzzyCompare(fitResult.rsd, 15.18, 1.e-3);
	DEBUG(std::setprecision(15) << fitResult.rsquare);	// result: 0.66733081652621
	FuzzyCompare(fitResult.rsquare, 0.6673, 1.e-4);
	DEBUG(std::setprecision(15) << fitResult.rsquareAdj);	// result: 0.645635000212702
	DEBUG(std::setprecision(15) << 1.-(1.-fitResult.rsquare)*(50.-1.)/(50.-np));	// result: 0.65317468105924
// reference calculates 1-(1-R^2)(n-1)/(n-p)
	FuzzyCompare(1.-(1.-fitResult.rsquare)*(50.-1.)/(50.-np), 0.6532, 1.e-4);
	DEBUG(std::setprecision(15) << fitResult.sse);	// result: 10824.71590767
	FuzzyCompare(fitResult.sse, 10825, 1.e-4);
	DEBUG(std::setprecision(15) << fitResult.rms);	// result: 230.313104418511
//	QCOMPARE(fitResult.rms, ???);
	DEBUG(std::setprecision(15) << fitResult.logLik);	// result: -205.386034235309
	FuzzyCompare(fitResult.logLik, -205.386, 1.e-6);
	DEBUG(std::setprecision(15) << fitResult.chisq_p);	// result:
//	FuzzyCompare(fitResult.chisq_p, ???, 1.e-8);
	DEBUG(std::setprecision(15) << fitResult.fdist_F);	// result:
// reference calculates sst/rms/np
	DEBUG(std::setprecision(15) << fitResult.sst/fitResult.rms/np);	// result: 47.0938320858956
	FuzzyCompare(fitResult.sst/fitResult.rms/np, 47.14, 1.e-3);
	DEBUG(std::setprecision(15) << fitResult.fdist_p);	// result: 0
	QCOMPARE(fitResult.fdist_p, 0.);		// exact: 5.852e-12
	DEBUG(std::setprecision(15) << fitResult.aic);	// result: 418.772068470618
	FuzzyCompare(fitResult.aic, 418.7721, 1.e-7);
	DEBUG(std::setprecision(15) << fitResult.bic);	// result: 426.42016049233
	FuzzyCompare(fitResult.bic, 426.4202, 1.e-7);
}

//##############################################################################
//#############  non-linear regression with NIST datasets  #####################
//##############################################################################

void FitTest::testNonLinearMisra1a() {
	//NIST data for Misra1a dataset
	QVector<double> xData = {77.6E0,114.9E0,141.1E0,190.8E0,239.9E0,289.0E0,332.8E0,378.4E0,434.8E0,477.3E0,536.8E0,593.1E0,689.1E0,760.0E0};
	QVector<double> yData = {10.07E0,14.73E0,17.94E0,23.93E0,29.61E0,35.18E0,40.02E0,44.82E0,50.76E0,55.05E0,61.01E0,66.40E0,75.47E0,81.78E0};

	//data source columns
	Column xDataColumn("x", AbstractColumn::ColumnMode::Numeric);
	xDataColumn.replaceValues(0, xData);

	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	XYFitCurve fitCurve("fit");
	fitCurve.setXDataColumn(&xDataColumn);
	fitCurve.setYDataColumn(&yDataColumn);

	//prepare the fit
	XYFitCurve::FitData fitData = fitCurve.fitData();
	fitData.modelCategory = nsl_fit_model_custom;
	XYFitCurve::initFitData(fitData);
	fitData.model = "b1*(1-exp(-b2*x))";
	fitData.paramNames << "b1" << "b2";
	fitData.eps = 1.e-12;
	const int np = fitData.paramNames.size();
	fitData.paramStartValues << 500. << 0.0001;
	fitData.paramLowerLimits << -std::numeric_limits<double>::max() << -std::numeric_limits<double>::max();
	fitData.paramUpperLimits << std::numeric_limits<double>::max() << std::numeric_limits<double>::max();
	fitCurve.setFitData(fitData);

	//perform the fit
	fitCurve.recalculate();
	const XYFitCurve::FitResult& fitResult = fitCurve.fitResult();

	//check the results
	QCOMPARE(fitResult.available, true);
	QCOMPARE(fitResult.valid, true);

	QCOMPARE(np, 2);

	DEBUG(std::setprecision(15) << fitResult.paramValues.at(0));	// result: 238.942305251573
	FuzzyCompare(fitResult.paramValues.at(0), 2.3894212918E+02, 1.e-6);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(0));	// result: 2.70651225218243
	FuzzyCompare(fitResult.errorValues.at(0), 2.7070075241E+00, 1.e-3);
	DEBUG(std::setprecision(15) << fitResult.paramValues.at(1));	// result: 0.000550155958419367
	FuzzyCompare(fitResult.paramValues.at(1), 5.5015643181E-04, 1.e-6);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(1));	// result: 7.26565480949189e-06
	FuzzyCompare(fitResult.errorValues.at(1), 7.2668688436E-06, 1.e-3);

	DEBUG(std::setprecision(15) << fitResult.rsd);	// result: 0.101878763320394
	FuzzyCompare(fitResult.rsd, 1.0187876330E-01, 1.e-9);
	DEBUG(std::setprecision(15) << fitResult.sse);	// result: 0.124551388988316
	FuzzyCompare(fitResult.sse, 1.2455138894E-01, 1.e-9);
}

void FitTest::testNonLinearMisra1a_2() {
	//NIST data for Misra1a dataset
	QVector<double> xData = {77.6E0,114.9E0,141.1E0,190.8E0,239.9E0,289.0E0,332.8E0,378.4E0,434.8E0,477.3E0,536.8E0,593.1E0,689.1E0,760.0E0};
	QVector<double> yData = {10.07E0,14.73E0,17.94E0,23.93E0,29.61E0,35.18E0,40.02E0,44.82E0,50.76E0,55.05E0,61.01E0,66.40E0,75.47E0,81.78E0};

	//data source columns
	Column xDataColumn("x", AbstractColumn::ColumnMode::Numeric);
	xDataColumn.replaceValues(0, xData);

	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	XYFitCurve fitCurve("fit");
	fitCurve.setXDataColumn(&xDataColumn);
	fitCurve.setYDataColumn(&yDataColumn);

	//prepare the fit
	XYFitCurve::FitData fitData = fitCurve.fitData();
	fitData.modelCategory = nsl_fit_model_custom;
	XYFitCurve::initFitData(fitData);
	fitData.model = "b1*(1-exp(-b2*x))";
	fitData.paramNames << "b1" << "b2";
	fitData.eps = 1.e-12;
	const int np = fitData.paramNames.size();
	fitData.paramStartValues << 250. << 5.e-4;
	fitData.paramLowerLimits << -std::numeric_limits<double>::max() << -std::numeric_limits<double>::max();
	fitData.paramUpperLimits << std::numeric_limits<double>::max() << std::numeric_limits<double>::max();
	fitCurve.setFitData(fitData);

	//perform the fit
	fitCurve.recalculate();
	const XYFitCurve::FitResult& fitResult = fitCurve.fitResult();

	//check the results
	QCOMPARE(fitResult.available, true);
	QCOMPARE(fitResult.valid, true);

	QCOMPARE(np, 2);

	DEBUG(std::setprecision(15) << fitResult.paramValues.at(0));	// result: 238.942305251573
	FuzzyCompare(fitResult.paramValues.at(0), 2.3894212918E+02, 1.e-6);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(0));	// result: 2.70651225218243
	FuzzyCompare(fitResult.errorValues.at(0), 2.7070075241E+00, 1.e-3);
	DEBUG(std::setprecision(15) << fitResult.paramValues.at(1));	// result: 0.000550155958419367
	FuzzyCompare(fitResult.paramValues.at(1), 5.5015643181E-04, 1.e-6);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(1));	// result: 7.26565480949189e-06
	FuzzyCompare(fitResult.errorValues.at(1), 7.2668688436E-06, 1.e-3);

	DEBUG(std::setprecision(15) << fitResult.rsd);	// result: 0.101878763320394
	FuzzyCompare(fitResult.rsd, 1.0187876330E-01, 1.e-9);
	DEBUG(std::setprecision(15) << fitResult.sse);	// result: 0.124551388988316
	FuzzyCompare(fitResult.sse, 1.2455138894E-01, 1.e-9);
}

void FitTest::testNonLinearMisra1a_3() {
	//NIST data for Misra1a dataset
	QVector<double> xData = {77.6E0,114.9E0,141.1E0,190.8E0,239.9E0,289.0E0,332.8E0,378.4E0,434.8E0,477.3E0,536.8E0,593.1E0,689.1E0,760.0E0};
	QVector<double> yData = {10.07E0,14.73E0,17.94E0,23.93E0,29.61E0,35.18E0,40.02E0,44.82E0,50.76E0,55.05E0,61.01E0,66.40E0,75.47E0,81.78E0};

	//data source columns
	Column xDataColumn("x", AbstractColumn::ColumnMode::Numeric);
	xDataColumn.replaceValues(0, xData);

	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	XYFitCurve fitCurve("fit");
	fitCurve.setXDataColumn(&xDataColumn);
	fitCurve.setYDataColumn(&yDataColumn);

	//prepare the fit
	XYFitCurve::FitData fitData = fitCurve.fitData();
	fitData.modelCategory = nsl_fit_model_custom;
	XYFitCurve::initFitData(fitData);
	fitData.model = "b1*(1-exp(-b2*x))";
	fitData.paramNames << "b1" << "b2";
	fitData.eps = 1.e-12;
	const int np = fitData.paramNames.size();
	fitData.paramStartValues << 2.3894212918E+02 << 5.5015643181E-04;
	fitData.paramLowerLimits << -std::numeric_limits<double>::max() << -std::numeric_limits<double>::max();
	fitData.paramUpperLimits << std::numeric_limits<double>::max() << std::numeric_limits<double>::max();
	fitCurve.setFitData(fitData);

	//perform the fit
	fitCurve.recalculate();
	const XYFitCurve::FitResult& fitResult = fitCurve.fitResult();

	//check the results
	QCOMPARE(fitResult.available, true);
	QCOMPARE(fitResult.valid, true);

	QCOMPARE(np, 2);

	DEBUG(std::setprecision(15) << fitResult.paramValues.at(0));	// result: 238.942305251573
	FuzzyCompare(fitResult.paramValues.at(0), 2.3894212918E+02, 1.e-6);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(0));	// result: 2.70651225218243
	FuzzyCompare(fitResult.errorValues.at(0), 2.7070075241E+00, 1.e-3);
	DEBUG(std::setprecision(15) << fitResult.paramValues.at(1));	// result: 0.000550155958419367
	FuzzyCompare(fitResult.paramValues.at(1), 5.5015643181E-04, 1.e-6);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(1));	// result: 7.26565480949189e-06
	FuzzyCompare(fitResult.errorValues.at(1), 7.2668688436E-06, 1.e-3);

	DEBUG(std::setprecision(15) << fitResult.rsd);	// result: 0.101878763320394
	FuzzyCompare(fitResult.rsd, 1.0187876330E-01, 1.e-9);
	DEBUG(std::setprecision(15) << fitResult.sse);	// result: 0.124551388988316
	FuzzyCompare(fitResult.sse, 1.2455138894E-01, 1.e-9);
}


void FitTest::testNonLinearMisra1b() {
	//NIST data for Misra1b dataset
	QVector<double> xData = {77.6E0,114.9E0,141.1E0,190.8E0,239.9E0,289.0E0,332.8E0,378.4E0,434.8E0,477.3E0,536.8E0,593.1E0,689.1E0,760.0E0};
	QVector<double> yData = {10.07E0,14.73E0,17.94E0,23.93E0,29.61E0,35.18E0,40.02E0,44.82E0,50.76E0,55.05E0,61.01E0,66.40E0,75.47E0,81.78E0};

	//data source columns
	Column xDataColumn("x", AbstractColumn::ColumnMode::Numeric);
	xDataColumn.replaceValues(0, xData);

	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	XYFitCurve fitCurve("fit");
	fitCurve.setXDataColumn(&xDataColumn);
	fitCurve.setYDataColumn(&yDataColumn);

	//prepare the fit
	XYFitCurve::FitData fitData = fitCurve.fitData();
	fitData.modelCategory = nsl_fit_model_custom;
	XYFitCurve::initFitData(fitData);
	fitData.model = "b1*(1-1/(1+b2*x/2)^2)";
	fitData.paramNames << "b1" << "b2";
	fitData.eps = 1.e-12;
	const int np = fitData.paramNames.size();
	fitData.paramStartValues << 500. << 0.0001;
	fitData.paramLowerLimits << -std::numeric_limits<double>::max() << -std::numeric_limits<double>::max();
	fitData.paramUpperLimits << std::numeric_limits<double>::max() << std::numeric_limits<double>::max();
	fitCurve.setFitData(fitData);

	//perform the fit
	fitCurve.recalculate();
	const XYFitCurve::FitResult& fitResult = fitCurve.fitResult();

	//check the results
	QCOMPARE(fitResult.available, true);
	QCOMPARE(fitResult.valid, true);

	QCOMPARE(np, 2);

	DEBUG(std::setprecision(15) << fitResult.paramValues.at(0));	// result: 337.99775062098
	FuzzyCompare(fitResult.paramValues.at(0), 3.3799746163E+02, 1.e-6);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(0));	// result: 3.16358581006192
	FuzzyCompare(fitResult.errorValues.at(0), 3.1643950207E+00, 1.e-3);
	DEBUG(std::setprecision(15) << fitResult.paramValues.at(1));	// result: 0.000390390523934039
	FuzzyCompare(fitResult.paramValues.at(1), 3.9039091287E-04, 1.e-6);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(1));	// result: 4.25373670682006e-06
	FuzzyCompare(fitResult.errorValues.at(1), 4.2547321834E-06, 1.e-3);

	DEBUG(std::setprecision(15) << fitResult.rsd);	// result: 0.0793014720259488
	FuzzyCompare(fitResult.rsd, 7.9301471998E-02, 1.e-9);
	DEBUG(std::setprecision(15) << fitResult.sse);	// result: 0.0754646815857881
	FuzzyCompare(fitResult.sse, 7.5464681533E-02, 1.e-9);
}

void FitTest::testNonLinearMisra1b_2() {
	//NIST data for Misra1b dataset
	QVector<double> xData = {77.6E0,114.9E0,141.1E0,190.8E0,239.9E0,289.0E0,332.8E0,378.4E0,434.8E0,477.3E0,536.8E0,593.1E0,689.1E0,760.0E0};
	QVector<double> yData = {10.07E0,14.73E0,17.94E0,23.93E0,29.61E0,35.18E0,40.02E0,44.82E0,50.76E0,55.05E0,61.01E0,66.40E0,75.47E0,81.78E0};

	//data source columns
	Column xDataColumn("x", AbstractColumn::ColumnMode::Numeric);
	xDataColumn.replaceValues(0, xData);

	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	XYFitCurve fitCurve("fit");
	fitCurve.setXDataColumn(&xDataColumn);
	fitCurve.setYDataColumn(&yDataColumn);

	//prepare the fit
	XYFitCurve::FitData fitData = fitCurve.fitData();
	fitData.modelCategory = nsl_fit_model_custom;
	XYFitCurve::initFitData(fitData);
	fitData.model = "b1*(1-1/(1+b2*x/2)^2)";
	fitData.paramNames << "b1" << "b2";
	fitData.eps = 1.e-12;
	const int np = fitData.paramNames.size();
	fitData.paramStartValues << 300. << 2.e-4;
	fitData.paramLowerLimits << -std::numeric_limits<double>::max() << -std::numeric_limits<double>::max();
	fitData.paramUpperLimits << std::numeric_limits<double>::max() << std::numeric_limits<double>::max();
	fitCurve.setFitData(fitData);

	//perform the fit
	fitCurve.recalculate();
	const XYFitCurve::FitResult& fitResult = fitCurve.fitResult();

	//check the results
	QCOMPARE(fitResult.available, true);
	QCOMPARE(fitResult.valid, true);

	QCOMPARE(np, 2);

	DEBUG(std::setprecision(15) << fitResult.paramValues.at(0));	// result: 337.99775062098
	FuzzyCompare(fitResult.paramValues.at(0), 3.3799746163E+02, 1.e-6);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(0));	// result: 3.16358581006192
	FuzzyCompare(fitResult.errorValues.at(0), 3.1643950207E+00, 1.e-3);
	DEBUG(std::setprecision(15) << fitResult.paramValues.at(1));	// result: 0.000390390523934039
	FuzzyCompare(fitResult.paramValues.at(1), 3.9039091287E-04, 1.e-5);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(1));	// result: 4.25373670682006e-06
	FuzzyCompare(fitResult.errorValues.at(1), 4.2547321834E-06, 1.e-3);

	DEBUG(std::setprecision(15) << fitResult.rsd);	// result: 0.0793014720259488
	FuzzyCompare(fitResult.rsd, 7.9301471998E-02, 1.e-9);
	DEBUG(std::setprecision(15) << fitResult.sse);	// result: 0.0754646815857881
	FuzzyCompare(fitResult.sse, 7.5464681533E-02, 1.e-9);
}

void FitTest::testNonLinearMisra1b_3() {
	//NIST data for Misra1b dataset
	QVector<double> xData = {77.6E0,114.9E0,141.1E0,190.8E0,239.9E0,289.0E0,332.8E0,378.4E0,434.8E0,477.3E0,536.8E0,593.1E0,689.1E0,760.0E0};
	QVector<double> yData = {10.07E0,14.73E0,17.94E0,23.93E0,29.61E0,35.18E0,40.02E0,44.82E0,50.76E0,55.05E0,61.01E0,66.40E0,75.47E0,81.78E0};

	//data source columns
	Column xDataColumn("x", AbstractColumn::ColumnMode::Numeric);
	xDataColumn.replaceValues(0, xData);

	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	XYFitCurve fitCurve("fit");
	fitCurve.setXDataColumn(&xDataColumn);
	fitCurve.setYDataColumn(&yDataColumn);

	//prepare the fit
	XYFitCurve::FitData fitData = fitCurve.fitData();
	fitData.modelCategory = nsl_fit_model_custom;
	XYFitCurve::initFitData(fitData);
	fitData.model = "b1*(1-1/(1+b2*x/2)^2)";
	fitData.paramNames << "b1" << "b2";
	fitData.eps = 1.e-12;
	const int np = fitData.paramNames.size();
	fitData.paramStartValues << 3.3799746163E+02 << 3.9039091287E-04;
	fitData.paramLowerLimits << -std::numeric_limits<double>::max() << -std::numeric_limits<double>::max();
	fitData.paramUpperLimits << std::numeric_limits<double>::max() << std::numeric_limits<double>::max();
	fitCurve.setFitData(fitData);

	//perform the fit
	fitCurve.recalculate();
	const XYFitCurve::FitResult& fitResult = fitCurve.fitResult();

	//check the results
	QCOMPARE(fitResult.available, true);
	QCOMPARE(fitResult.valid, true);

	QCOMPARE(np, 2);

	DEBUG(std::setprecision(15) << fitResult.paramValues.at(0));	// result: 337.99775062098
	FuzzyCompare(fitResult.paramValues.at(0), 3.3799746163E+02, 1.e-6);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(0));	// result: 3.16358581006192
	FuzzyCompare(fitResult.errorValues.at(0), 3.1643950207E+00, 1.e-3);
	DEBUG(std::setprecision(15) << fitResult.paramValues.at(1));	// result: 0.000390390523934039
	FuzzyCompare(fitResult.paramValues.at(1), 3.9039091287E-04, 1.e-6);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(1));	// result: 4.25373670682006e-06
	FuzzyCompare(fitResult.errorValues.at(1), 4.2547321834E-06, 1.e-3);

	DEBUG(std::setprecision(15) << fitResult.rsd);	// result: 0.0793014720259488
	FuzzyCompare(fitResult.rsd, 7.9301471998E-02, 1.e-9);
	DEBUG(std::setprecision(15) << fitResult.sse);	// result: 0.0754646815857881
	FuzzyCompare(fitResult.sse, 7.5464681533E-02, 1.e-9);
}

void FitTest::testNonLinearMisra1c() {
	//NIST data for Misra1c dataset
	QVector<double> xData = {77.6E0,114.9E0,141.1E0,190.8E0,239.9E0,289.0E0,332.8E0,378.4E0,434.8E0,477.3E0,536.8E0,593.1E0,689.1E0,760.0E0};
	QVector<double> yData = {10.07E0,14.73E0,17.94E0,23.93E0,29.61E0,35.18E0,40.02E0,44.82E0,50.76E0,55.05E0,61.01E0,66.40E0,75.47E0,81.78E0};

	//data source columns
	Column xDataColumn("x", AbstractColumn::ColumnMode::Numeric);
	xDataColumn.replaceValues(0, xData);

	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	XYFitCurve fitCurve("fit");
	fitCurve.setXDataColumn(&xDataColumn);
	fitCurve.setYDataColumn(&yDataColumn);

	//prepare the fit
	XYFitCurve::FitData fitData = fitCurve.fitData();
	fitData.modelCategory = nsl_fit_model_custom;
	XYFitCurve::initFitData(fitData);
	fitData.model = "b1*(1-1/sqrt(1+2*b2*x))";
	fitData.paramNames << "b1" << "b2";
	fitData.eps = 1.e-12;
	const int np = fitData.paramNames.size();
	fitData.paramStartValues << 500. << 0.0001;
	fitData.paramLowerLimits << -std::numeric_limits<double>::max() << -std::numeric_limits<double>::max();
	fitData.paramUpperLimits << std::numeric_limits<double>::max() << std::numeric_limits<double>::max();
	fitCurve.setFitData(fitData);

	//perform the fit
	fitCurve.recalculate();
	const XYFitCurve::FitResult& fitResult = fitCurve.fitResult();

	//check the results
	QCOMPARE(fitResult.available, true);
	QCOMPARE(fitResult.valid, true);

	QCOMPARE(np, 2);

	DEBUG(std::setprecision(15) << fitResult.paramValues.at(0));	// result: 636.427904767969
	FuzzyCompare(fitResult.paramValues.at(0), 6.3642725809E+02, 1.e-5);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(0));	// result: 4.66168062054875
	FuzzyCompare(fitResult.errorValues.at(0), 4.6638326572E+00, 1.e-3);
	DEBUG(std::setprecision(15) << fitResult.paramValues.at(1));	// result: 0.000208136026420746
	FuzzyCompare(fitResult.paramValues.at(1), 2.0813627256E-04, 1.e-5);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(1));	// result: 1.77209416674174e-06
	FuzzyCompare(fitResult.errorValues.at(1), 1.7728423155E-06, 1.e-3);

	DEBUG(std::setprecision(15) << fitResult.rsd);	// result: 0.0584286153041661
	FuzzyCompare(fitResult.rsd, 5.8428615257E-02, 1.e-9);
	DEBUG(std::setprecision(15) << fitResult.sse);	// result: 0.0409668370363468
	FuzzyCompare(fitResult.sse, 4.0966836971E-02, 1.e-8);
}

void FitTest::testNonLinearMisra1c_2() {
	//NIST data for Misra1c dataset
	QVector<double> xData = {77.6E0,114.9E0,141.1E0,190.8E0,239.9E0,289.0E0,332.8E0,378.4E0,434.8E0,477.3E0,536.8E0,593.1E0,689.1E0,760.0E0};
	QVector<double> yData = {10.07E0,14.73E0,17.94E0,23.93E0,29.61E0,35.18E0,40.02E0,44.82E0,50.76E0,55.05E0,61.01E0,66.40E0,75.47E0,81.78E0};

	//data source columns
	Column xDataColumn("x", AbstractColumn::ColumnMode::Numeric);
	xDataColumn.replaceValues(0, xData);

	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	XYFitCurve fitCurve("fit");
	fitCurve.setXDataColumn(&xDataColumn);
	fitCurve.setYDataColumn(&yDataColumn);

	//prepare the fit
	XYFitCurve::FitData fitData = fitCurve.fitData();
	fitData.modelCategory = nsl_fit_model_custom;
	XYFitCurve::initFitData(fitData);
	fitData.model = "b1*(1-1/sqrt(1+2*b2*x))";
	fitData.paramNames << "b1" << "b2";
	fitData.eps = 1.e-12;
	const int np = fitData.paramNames.size();
	fitData.paramStartValues << 600. << 2.e-4;
	fitData.paramLowerLimits << -std::numeric_limits<double>::max() << -std::numeric_limits<double>::max();
	fitData.paramUpperLimits << std::numeric_limits<double>::max() << std::numeric_limits<double>::max();
	fitCurve.setFitData(fitData);

	//perform the fit
	fitCurve.recalculate();
	const XYFitCurve::FitResult& fitResult = fitCurve.fitResult();

	//check the results
	QCOMPARE(fitResult.available, true);
	QCOMPARE(fitResult.valid, true);

	QCOMPARE(np, 2);

	DEBUG(std::setprecision(15) << fitResult.paramValues.at(0));	// result: 636.427904767969
	FuzzyCompare(fitResult.paramValues.at(0), 6.3642725809E+02, 1.e-5);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(0));	// result: 4.66168062054875
	FuzzyCompare(fitResult.errorValues.at(0), 4.6638326572E+00, 1.e-3);
	DEBUG(std::setprecision(15) << fitResult.paramValues.at(1));	// result: 0.000208136026420746
	FuzzyCompare(fitResult.paramValues.at(1), 2.0813627256E-04, 1.e-5);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(1));	// result: 1.77209416674174e-06
	FuzzyCompare(fitResult.errorValues.at(1), 1.7728423155E-06, 1.e-3);

	DEBUG(std::setprecision(15) << fitResult.rsd);	// result: 0.0584286153041661	(WIN: 0.0584286153252961)
	FuzzyCompare(fitResult.rsd, 5.8428615257E-02, 1.e-8);
	DEBUG(std::setprecision(15) << fitResult.sse);	// result: 0.0409668370363468
	FuzzyCompare(fitResult.sse, 4.0966836971E-02, 1.e-8);
}

void FitTest::testNonLinearMisra1c_3() {
	//NIST data for Misra1c dataset
	QVector<double> xData = {77.6E0,114.9E0,141.1E0,190.8E0,239.9E0,289.0E0,332.8E0,378.4E0,434.8E0,477.3E0,536.8E0,593.1E0,689.1E0,760.0E0};
	QVector<double> yData = {10.07E0,14.73E0,17.94E0,23.93E0,29.61E0,35.18E0,40.02E0,44.82E0,50.76E0,55.05E0,61.01E0,66.40E0,75.47E0,81.78E0};

	//data source columns
	Column xDataColumn("x", AbstractColumn::ColumnMode::Numeric);
	xDataColumn.replaceValues(0, xData);

	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	XYFitCurve fitCurve("fit");
	fitCurve.setXDataColumn(&xDataColumn);
	fitCurve.setYDataColumn(&yDataColumn);

	//prepare the fit
	XYFitCurve::FitData fitData = fitCurve.fitData();
	fitData.modelCategory = nsl_fit_model_custom;
	XYFitCurve::initFitData(fitData);
	fitData.model = "b1*(1-1/sqrt(1+2*b2*x))";
	fitData.paramNames << "b1" << "b2";
	fitData.eps = 1.e-12;
	const int np = fitData.paramNames.size();
	fitData.paramStartValues << 6.3642725809E+02 << 2.0813627256E-04;
	fitData.paramLowerLimits << -std::numeric_limits<double>::max() << -std::numeric_limits<double>::max();
	fitData.paramUpperLimits << std::numeric_limits<double>::max() << std::numeric_limits<double>::max();
	fitCurve.setFitData(fitData);

	//perform the fit
	fitCurve.recalculate();
	const XYFitCurve::FitResult& fitResult = fitCurve.fitResult();

	//check the results
	QCOMPARE(fitResult.available, true);
	QCOMPARE(fitResult.valid, true);

	QCOMPARE(np, 2);

	DEBUG(std::setprecision(15) << fitResult.paramValues.at(0));	// result: 636.427904767969
	FuzzyCompare(fitResult.paramValues.at(0), 6.3642725809E+02, 1.e-5);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(0));	// result: 4.66168062054875
	FuzzyCompare(fitResult.errorValues.at(0), 4.6638326572E+00, 1.e-3);
	DEBUG(std::setprecision(15) << fitResult.paramValues.at(1));	// result: 0.000208136026420746
	FuzzyCompare(fitResult.paramValues.at(1), 2.0813627256E-04, 1.e-5);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(1));	// result: 1.77209416674174e-06
	FuzzyCompare(fitResult.errorValues.at(1), 1.7728423155E-06, 1.e-3);

	DEBUG(std::setprecision(15) << fitResult.rsd);	// result: 0.0584286153041661
	FuzzyCompare(fitResult.rsd, 5.8428615257E-02, 1.e-9);
	DEBUG(std::setprecision(15) << fitResult.sse);	// result: 0.0409668370363468
	FuzzyCompare(fitResult.sse, 4.0966836971E-02, 1.e-8);
}

void FitTest::testNonLinearMisra1d() {
	//NIST data for Misra1d dataset
	QVector<double> xData = {77.6E0,114.9E0,141.1E0,190.8E0,239.9E0,289.0E0,332.8E0,378.4E0,434.8E0,477.3E0,536.8E0,593.1E0,689.1E0,760.0E0};
	QVector<double> yData = {10.07E0,14.73E0,17.94E0,23.93E0,29.61E0,35.18E0,40.02E0,44.82E0,50.76E0,55.05E0,61.01E0,66.40E0,75.47E0,81.78E0};

	//data source columns
	Column xDataColumn("x", AbstractColumn::ColumnMode::Numeric);
	xDataColumn.replaceValues(0, xData);

	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	XYFitCurve fitCurve("fit");
	fitCurve.setXDataColumn(&xDataColumn);
	fitCurve.setYDataColumn(&yDataColumn);

	//prepare the fit
	XYFitCurve::FitData fitData = fitCurve.fitData();
	fitData.modelCategory = nsl_fit_model_custom;
	XYFitCurve::initFitData(fitData);
	fitData.model = "b1*b2*x/(1+b2*x)";
	fitData.paramNames << "b1" << "b2";
	fitData.eps = 1.e-12;
	const int np = fitData.paramNames.size();
	fitData.paramStartValues << 500. << 0.0001;
	fitData.paramLowerLimits << -std::numeric_limits<double>::max() << -std::numeric_limits<double>::max();
	fitData.paramUpperLimits << std::numeric_limits<double>::max() << std::numeric_limits<double>::max();
	fitCurve.setFitData(fitData);

	//perform the fit
	fitCurve.recalculate();
	const XYFitCurve::FitResult& fitResult = fitCurve.fitResult();

	//check the results
	QCOMPARE(fitResult.available, true);
	QCOMPARE(fitResult.valid, true);

	QCOMPARE(np, 2);

	DEBUG(std::setprecision(15) << fitResult.paramValues.at(0));	// result: 437.370039987725
	FuzzyCompare(fitResult.paramValues.at(0), 4.3736970754E+02, 1.e-6);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(0));	// result: 3.64772833062694
	FuzzyCompare(fitResult.errorValues.at(0), 3.6489174345E+00, 1.e-3);
	DEBUG(std::setprecision(15) << fitResult.paramValues.at(1));	// result: 0.000302272976784709
	FuzzyCompare(fitResult.paramValues.at(1), 3.0227324449E-04, 1.e-6);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(1));	// result: 2.93256059733558e-06
	FuzzyCompare(fitResult.errorValues.at(1), 2.9334354479E-06, 1.e-3);

	DEBUG(std::setprecision(15) << fitResult.rsd);	// result: 0.068568272134244
	FuzzyCompare(fitResult.rsd, 6.8568272111E-02, 1.e-9);
	DEBUG(std::setprecision(15) << fitResult.sse);	// result: 0.056419295321709 (new: 0.0564192953613416)
	FuzzyCompare(fitResult.sse, 5.6419295283E-02, 1.e-8);
}

void FitTest::testNonLinearMisra1d_2() {
	//NIST data for Misra1d dataset
	QVector<double> xData = {77.6E0,114.9E0,141.1E0,190.8E0,239.9E0,289.0E0,332.8E0,378.4E0,434.8E0,477.3E0,536.8E0,593.1E0,689.1E0,760.0E0};
	QVector<double> yData = {10.07E0,14.73E0,17.94E0,23.93E0,29.61E0,35.18E0,40.02E0,44.82E0,50.76E0,55.05E0,61.01E0,66.40E0,75.47E0,81.78E0};

	//data source columns
	Column xDataColumn("x", AbstractColumn::ColumnMode::Numeric);
	xDataColumn.replaceValues(0, xData);

	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	XYFitCurve fitCurve("fit");
	fitCurve.setXDataColumn(&xDataColumn);
	fitCurve.setYDataColumn(&yDataColumn);

	//prepare the fit
	XYFitCurve::FitData fitData = fitCurve.fitData();
	fitData.modelCategory = nsl_fit_model_custom;
	XYFitCurve::initFitData(fitData);
	fitData.model = "b1*b2*x/(1+b2*x)";
	fitData.paramNames << "b1" << "b2";
	fitData.eps = 1.e-12;
	const int np = fitData.paramNames.size();
	fitData.paramStartValues << 450. << 3.e-4;
	fitData.paramLowerLimits << -std::numeric_limits<double>::max() << -std::numeric_limits<double>::max();
	fitData.paramUpperLimits << std::numeric_limits<double>::max() << std::numeric_limits<double>::max();
	fitCurve.setFitData(fitData);

	//perform the fit
	fitCurve.recalculate();
	const XYFitCurve::FitResult& fitResult = fitCurve.fitResult();

	//check the results
	QCOMPARE(fitResult.available, true);
	QCOMPARE(fitResult.valid, true);

	QCOMPARE(np, 2);

	DEBUG(std::setprecision(15) << fitResult.paramValues.at(0));	// result: 437.370039987725
	FuzzyCompare(fitResult.paramValues.at(0), 4.3736970754E+02, 1.e-5);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(0));	// result: 3.64772833062694
	FuzzyCompare(fitResult.errorValues.at(0), 3.6489174345E+00, 1.e-3);
	DEBUG(std::setprecision(15) << fitResult.paramValues.at(1));	// result: 0.000302272976784709
	FuzzyCompare(fitResult.paramValues.at(1), 3.0227324449E-04, 1.e-5);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(1));	// result: 2.93256059733558e-06
	FuzzyCompare(fitResult.errorValues.at(1), 2.9334354479E-06, 1.e-3);

	DEBUG(std::setprecision(15) << fitResult.rsd);	// result: 0.068568272134244
	FuzzyCompare(fitResult.rsd, 6.8568272111E-02, 1.e-9);
	DEBUG(std::setprecision(15) << fitResult.sse);	// result: 0.056419295321709
	FuzzyCompare(fitResult.sse, 5.6419295283E-02, 1.e-8);
}

void FitTest::testNonLinearMisra1d_3() {
	//NIST data for Misra1d dataset
	QVector<double> xData = {77.6E0,114.9E0,141.1E0,190.8E0,239.9E0,289.0E0,332.8E0,378.4E0,434.8E0,477.3E0,536.8E0,593.1E0,689.1E0,760.0E0};
	QVector<double> yData = {10.07E0,14.73E0,17.94E0,23.93E0,29.61E0,35.18E0,40.02E0,44.82E0,50.76E0,55.05E0,61.01E0,66.40E0,75.47E0,81.78E0};

	//data source columns
	Column xDataColumn("x", AbstractColumn::ColumnMode::Numeric);
	xDataColumn.replaceValues(0, xData);

	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	XYFitCurve fitCurve("fit");
	fitCurve.setXDataColumn(&xDataColumn);
	fitCurve.setYDataColumn(&yDataColumn);

	//prepare the fit
	XYFitCurve::FitData fitData = fitCurve.fitData();
	fitData.modelCategory = nsl_fit_model_custom;
	XYFitCurve::initFitData(fitData);
	fitData.model = "b1*b2*x/(1+b2*x)";
	fitData.paramNames << "b1" << "b2";
	fitData.eps = 1.e-12;
	const int np = fitData.paramNames.size();
	fitData.paramStartValues << 4.3736970754E+02 << 3.0227324449E-04;
	fitData.paramLowerLimits << -std::numeric_limits<double>::max() << -std::numeric_limits<double>::max();
	fitData.paramUpperLimits << std::numeric_limits<double>::max() << std::numeric_limits<double>::max();
	fitCurve.setFitData(fitData);

	//perform the fit
	fitCurve.recalculate();
	const XYFitCurve::FitResult& fitResult = fitCurve.fitResult();

	//check the results
	QCOMPARE(fitResult.available, true);
	QCOMPARE(fitResult.valid, true);

	QCOMPARE(np, 2);

	DEBUG(std::setprecision(15) << fitResult.paramValues.at(0));	// result: 437.370039987725
	FuzzyCompare(fitResult.paramValues.at(0), 4.3736970754E+02, 1.e-6);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(0));	// result: 3.64772833062694
	FuzzyCompare(fitResult.errorValues.at(0), 3.6489174345E+00, 1.e-3);
	DEBUG(std::setprecision(15) << fitResult.paramValues.at(1));	// result: 0.000302272976784709
	FuzzyCompare(fitResult.paramValues.at(1), 3.0227324449E-04, 1.e-6);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(1));	// result: 2.93256059733558e-06
	FuzzyCompare(fitResult.errorValues.at(1), 2.9334354479E-06, 1.e-3);

	DEBUG(std::setprecision(15) << fitResult.rsd);	// result: 0.068568272134244
	FuzzyCompare(fitResult.rsd, 6.8568272111E-02, 1.e-9);
	DEBUG(std::setprecision(15) << fitResult.sse);	// result: 0.056419295321709
	FuzzyCompare(fitResult.sse, 5.6419295283E-02, 1.e-9);
}

void FitTest::testNonLinearMGH09() {
	//NIST data for MGH09 dataset
	QVector<double> xData = {4.000000E+00,2.000000E+00,1.000000E+00,5.000000E-01,2.500000E-01,1.670000E-01,1.250000E-01,1.000000E-01,
		8.330000E-02,7.140000E-02,6.250000E-02};
	QVector<double> yData = {1.957000E-01,1.947000E-01,1.735000E-01,1.600000E-01,8.440000E-02,6.270000E-02,4.560000E-02,3.420000E-02,
		3.230000E-02,2.350000E-02,2.460000E-02};

	//data source columns
	Column xDataColumn("x", AbstractColumn::ColumnMode::Numeric);
	xDataColumn.replaceValues(0, xData);

	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	XYFitCurve fitCurve("fit");
	fitCurve.setXDataColumn(&xDataColumn);
	fitCurve.setYDataColumn(&yDataColumn);

	//prepare the fit
	XYFitCurve::FitData fitData = fitCurve.fitData();
	fitData.modelCategory = nsl_fit_model_custom;
	XYFitCurve::initFitData(fitData);
	fitData.model = "b1*(x^2 + b2*x)/(x^2 + x*b3 + b4)";
	fitData.paramNames << "b1" << "b2" << "b3" << "b4";
	//fitData.eps = 1.e-12;
	const int np = fitData.paramNames.size();
	fitData.paramStartValues << 2.5000000000E+01 << 3.9000000000E+01 << 4.1500000000E+01 << 3.9000000000E+01;
	fitData.paramLowerLimits << -std::numeric_limits<double>::max() << -std::numeric_limits<double>::max() << -std::numeric_limits<double>::max() << -std::numeric_limits<double>::max();
	fitData.paramUpperLimits << std::numeric_limits<double>::max() << std::numeric_limits<double>::max() << std::numeric_limits<double>::max() << std::numeric_limits<double>::max();
	fitCurve.setFitData(fitData);

	//perform the fit
	fitCurve.recalculate();
	const XYFitCurve::FitResult& fitResult = fitCurve.fitResult();

	//check the results
	QCOMPARE(fitResult.available, true);
	QCOMPARE(fitResult.valid, true);

	QCOMPARE(np, 4);

// TODO: fit does not find global minimum
/*	DEBUG(std::setprecision(15) << fitResult.paramValues.at(0));	// result:
	FuzzyCompare(fitResult.paramValues.at(0), 1.9280693458E-01, 1.e-6);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(0));	// result:
	FuzzyCompare(fitResult.errorValues.at(0), 1.1435312227E-02, 1.e-3);
	DEBUG(std::setprecision(15) << fitResult.paramValues.at(1));	// result:
	FuzzyCompare(fitResult.paramValues.at(1), 1.9128232873E-01, 1.e-6);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(1));	// result:
	FuzzyCompare(fitResult.errorValues.at(1), 1.9633220911E-01, 1.e-3);
	DEBUG(std::setprecision(15) << fitResult.paramValues.at(2));	// result:
	FuzzyCompare(fitResult.paramValues.at(2), 1.2305650693E-01, 1.e-6);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(2));	// result:
	FuzzyCompare(fitResult.errorValues.at(2), 8.0842031232E-02, 1.e-3);
	DEBUG(std::setprecision(15) << fitResult.paramValues.at(3));	// result:
	FuzzyCompare(fitResult.paramValues.at(3), 1.3606233068E-01, 1.e-6);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(3));	// result:
	FuzzyCompare(fitResult.errorValues.at(3), 9.0025542308E-02, 1.e-3);

	DEBUG(std::setprecision(15) << fitResult.rsd);	// result:
	FuzzyCompare(fitResult.rsd, 6.6279236551E-03, 1.e-9);
	DEBUG(std::setprecision(15) << fitResult.sse);	// result:
	FuzzyCompare(fitResult.sse, 3.0750560385E-04, 1.e-9);
*/
}

void FitTest::testNonLinearMGH09_2() {
	//NIST data for MGH09 dataset
	QVector<double> xData = {4.000000E+00,2.000000E+00,1.000000E+00,5.000000E-01,2.500000E-01,1.670000E-01,1.250000E-01,1.000000E-01,
		8.330000E-02,7.140000E-02,6.250000E-02};
	QVector<double> yData = {1.957000E-01,1.947000E-01,1.735000E-01,1.600000E-01,8.440000E-02,6.270000E-02,4.560000E-02,3.420000E-02,
		3.230000E-02,2.350000E-02,2.460000E-02};

	//data source columns
	Column xDataColumn("x", AbstractColumn::ColumnMode::Numeric);
	xDataColumn.replaceValues(0, xData);

	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	XYFitCurve fitCurve("fit");
	fitCurve.setXDataColumn(&xDataColumn);
	fitCurve.setYDataColumn(&yDataColumn);

	//prepare the fit
	XYFitCurve::FitData fitData = fitCurve.fitData();
	fitData.modelCategory = nsl_fit_model_custom;
	XYFitCurve::initFitData(fitData);
	fitData.model = "b1*(x^2 + b2*x)/(x^2 + x*b3 + b4)";
	fitData.paramNames << "b1" << "b2" << "b3" << "b4";
	//fitData.eps = 1.e-12;
	const int np = fitData.paramNames.size();
	fitData.paramStartValues << 2.5000000000E-01 << 3.9000000000E-01 << 4.1500000000E-01 << 3.9000000000E-01;
	fitData.paramLowerLimits << -std::numeric_limits<double>::max() << -std::numeric_limits<double>::max() << -std::numeric_limits<double>::max() << -std::numeric_limits<double>::max();
	fitData.paramUpperLimits << std::numeric_limits<double>::max() << std::numeric_limits<double>::max() << std::numeric_limits<double>::max() << std::numeric_limits<double>::max();
	fitCurve.setFitData(fitData);

	//perform the fit
	fitCurve.recalculate();
	const XYFitCurve::FitResult& fitResult = fitCurve.fitResult();

	//check the results
	QCOMPARE(fitResult.available, true);
	QCOMPARE(fitResult.valid, true);

	QCOMPARE(np, 4);

	DEBUG(std::setprecision(15) << fitResult.paramValues.at(0));	// result:
	FuzzyCompare(fitResult.paramValues.at(0), 1.9280693458E-01, 1.e-5);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(0));	// result:
	FuzzyCompare(fitResult.errorValues.at(0), 1.1435312227E-02, 1.e-4);
	DEBUG(std::setprecision(15) << fitResult.paramValues.at(1));	// result:
	FuzzyCompare(fitResult.paramValues.at(1), 1.9128232873E-01, 1.e-3);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(1));	// result: (new: 0.196361378435283)
	FuzzyCompare(fitResult.errorValues.at(1), 1.9633220911E-01, 1.e-3);
	DEBUG(std::setprecision(15) << fitResult.paramValues.at(2));	// result:
	FuzzyCompare(fitResult.paramValues.at(2), 1.2305650693E-01, 1.e-4);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(2));	// result:
	FuzzyCompare(fitResult.errorValues.at(2), 8.0842031232E-02, 1.e-4);
	DEBUG(std::setprecision(15) << fitResult.paramValues.at(3));	// result:
	FuzzyCompare(fitResult.paramValues.at(3), 1.3606233068E-01, 1.e-4);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(3));	// result:
	FuzzyCompare(fitResult.errorValues.at(3), 9.0025542308E-02, 1.e-4);

	DEBUG(std::setprecision(15) << fitResult.rsd);	// result:
	FuzzyCompare(fitResult.rsd, 6.6279236551E-03, 1.e-8);
	DEBUG(std::setprecision(15) << fitResult.sse);	// result:
	FuzzyCompare(fitResult.sse, 3.0750560385E-04, 1.e-8);
}

void FitTest::testNonLinearMGH09_3() {
	//NIST data for MGH09 dataset
	QVector<double> xData = {4.000000E+00,2.000000E+00,1.000000E+00,5.000000E-01,2.500000E-01,1.670000E-01,1.250000E-01,1.000000E-01,
		8.330000E-02,7.140000E-02,6.250000E-02};
	QVector<double> yData = {1.957000E-01,1.947000E-01,1.735000E-01,1.600000E-01,8.440000E-02,6.270000E-02,4.560000E-02,3.420000E-02,
		3.230000E-02,2.350000E-02,2.460000E-02};

	//data source columns
	Column xDataColumn("x", AbstractColumn::ColumnMode::Numeric);
	xDataColumn.replaceValues(0, xData);

	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	XYFitCurve fitCurve("fit");
	fitCurve.setXDataColumn(&xDataColumn);
	fitCurve.setYDataColumn(&yDataColumn);

	//prepare the fit
	XYFitCurve::FitData fitData = fitCurve.fitData();
	fitData.modelCategory = nsl_fit_model_custom;
	XYFitCurve::initFitData(fitData);
	fitData.model = "b1*(x^2 + b2*x)/(x^2 + x*b3 + b4)";
	fitData.paramNames << "b1" << "b2" << "b3" << "b4";
	//fitData.eps = 1.e-12;
	const int np = fitData.paramNames.size();
	fitData.paramStartValues << 1.9280693458E-01 << 1.9128232873E-01 << 1.2305650693E-01 << 1.3606233068E-01;
	fitData.paramLowerLimits << -std::numeric_limits<double>::max() << -std::numeric_limits<double>::max() << -std::numeric_limits<double>::max() << -std::numeric_limits<double>::max();
	fitData.paramUpperLimits << std::numeric_limits<double>::max() << std::numeric_limits<double>::max() << std::numeric_limits<double>::max() << std::numeric_limits<double>::max();
	fitCurve.setFitData(fitData);

	//perform the fit
	fitCurve.recalculate();
	const XYFitCurve::FitResult& fitResult = fitCurve.fitResult();

	//check the results
	QCOMPARE(fitResult.available, true);
	QCOMPARE(fitResult.valid, true);

	QCOMPARE(np, 4);

	DEBUG(std::setprecision(15) << fitResult.paramValues.at(0));	// result:
	FuzzyCompare(fitResult.paramValues.at(0), 1.9280693458E-01, 1.e-9);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(0));	// result:
	FuzzyCompare(fitResult.errorValues.at(0), 1.1435312227E-02, 1.e-5);
	DEBUG(std::setprecision(15) << fitResult.paramValues.at(1));	// result:
	FuzzyCompare(fitResult.paramValues.at(1), 1.9128232873E-01, 1.e-9);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(1));	// result:
	FuzzyCompare(fitResult.errorValues.at(1), 1.9633220911E-01, 1.e-5);
	DEBUG(std::setprecision(15) << fitResult.paramValues.at(2));	// result:
	FuzzyCompare(fitResult.paramValues.at(2), 1.2305650693E-01, 1.e-9);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(2));	// result:
	FuzzyCompare(fitResult.errorValues.at(2), 8.0842031232E-02, 1.e-5);
	DEBUG(std::setprecision(15) << fitResult.paramValues.at(3));	// result:
	FuzzyCompare(fitResult.paramValues.at(3), 1.3606233068E-01, 1.e-9);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(3));	// result:
	FuzzyCompare(fitResult.errorValues.at(3), 9.0025542308E-02, 1.e-5);

	DEBUG(std::setprecision(15) << fitResult.rsd);	// result:
	FuzzyCompare(fitResult.rsd, 6.6279236551E-03, 1.e-9);
	DEBUG(std::setprecision(15) << fitResult.sse);	// result:
	FuzzyCompare(fitResult.sse, 3.0750560385E-04, 1.e-9);
}

void FitTest::testNonLinearMGH10() {
	//NIST data for MGH10 dataset
	QVector<double> xData = {5.000000E+01,5.500000E+01,6.000000E+01,6.500000E+01,7.000000E+01,7.500000E+01,8.000000E+01,8.500000E+01,9.000000E+01,
		9.500000E+01,1.000000E+02,1.050000E+02,1.100000E+02,1.150000E+02,1.200000E+02,1.250000E+02};
	QVector<double> yData = {3.478000E+04,2.861000E+04,2.365000E+04,1.963000E+04,1.637000E+04,1.372000E+04,1.154000E+04,9.744000E+03,8.261000E+03,
		7.030000E+03,6.005000E+03,5.147000E+03,4.427000E+03,3.820000E+03,3.307000E+03,2.872000E+03};

	//data source columns
	Column xDataColumn("x", AbstractColumn::ColumnMode::Numeric);
	xDataColumn.replaceValues(0, xData);

	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	XYFitCurve fitCurve("fit");
	fitCurve.setXDataColumn(&xDataColumn);
	fitCurve.setYDataColumn(&yDataColumn);

	//prepare the fit
	XYFitCurve::FitData fitData = fitCurve.fitData();
	fitData.modelCategory = nsl_fit_model_custom;
	XYFitCurve::initFitData(fitData);
	fitData.model = "b1*exp(b2/(x+b3))";
	fitData.paramNames << "b1" << "b2" << "b3";
	//fitData.eps = 1.e-12;
	const int np = fitData.paramNames.size();
	fitData.paramStartValues << 2.0000000000E+00 << 4.0000000000E+05 << 2.5000000000E+04;
	fitData.paramLowerLimits << -std::numeric_limits<double>::max() << -std::numeric_limits<double>::max() << -std::numeric_limits<double>::max();
	fitData.paramUpperLimits << std::numeric_limits<double>::max() << std::numeric_limits<double>::max() << std::numeric_limits<double>::max();
	fitCurve.setFitData(fitData);

	//perform the fit
	fitCurve.recalculate();
	const XYFitCurve::FitResult& fitResult = fitCurve.fitResult();

	//check the results
	QCOMPARE(fitResult.available, true);
	QCOMPARE(fitResult.valid, true);

	QCOMPARE(np, 3);

	DEBUG(std::setprecision(15) << fitResult.paramValues.at(0));	// result: 0.00560963848336205 (Windows: 0.00560964247264364)
	FuzzyCompare(fitResult.paramValues.at(0), 5.6096364710E-03, 1.e-5);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(0));	// result: 0.000156888682057687
	FuzzyCompare(fitResult.errorValues.at(0), 1.5687892471E-04, 1.e-4);
	DEBUG(std::setprecision(15) << fitResult.paramValues.at(1));	// result: 6181.34604697191 (Windows: 6181.34545410281)
	FuzzyCompare(fitResult.paramValues.at(1), 6.1813463463E+03, 1.e-6);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(1));	// result: 23.3105479190063
	FuzzyCompare(fitResult.errorValues.at(1), 2.3309021107E+01, 1.e-4);
	DEBUG(std::setprecision(15) << fitResult.paramValues.at(2));	// result: 345.223624540718 (Windows: 345.223579103645)
	FuzzyCompare(fitResult.paramValues.at(2), 3.4522363462E+02, 1.e-6);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(2));	// result: 0.784915645388214
	FuzzyCompare(fitResult.errorValues.at(2), 7.8486103508E-01, 1.e-4);

	DEBUG(std::setprecision(15) << fitResult.rsd);	// result: 2.6009740064923	(Windows: 2.60097400662837, 2.60097400697918)
	FuzzyCompare(fitResult.rsd, 2.6009740065E+00, 1.e-9);
	DEBUG(std::setprecision(15) << fitResult.sse);	// result: 87.9458551718321	(FreeBSD: 87.9458551726946, Windows: 87.9458551810338)
	FuzzyCompare(fitResult.sse, 8.7945855171E+01, 1.e-9);
}

void FitTest::testNonLinearMGH10_2() {
	//NIST data for MGH10 dataset
	QVector<double> xData = {5.000000E+01,5.500000E+01,6.000000E+01,6.500000E+01,7.000000E+01,7.500000E+01,8.000000E+01,8.500000E+01,9.000000E+01,
		9.500000E+01,1.000000E+02,1.050000E+02,1.100000E+02,1.150000E+02,1.200000E+02,1.250000E+02};
	QVector<double> yData = {3.478000E+04,2.861000E+04,2.365000E+04,1.963000E+04,1.637000E+04,1.372000E+04,1.154000E+04,9.744000E+03,8.261000E+03,
		7.030000E+03,6.005000E+03,5.147000E+03,4.427000E+03,3.820000E+03,3.307000E+03,2.872000E+03};

	//data source columns
	Column xDataColumn("x", AbstractColumn::ColumnMode::Numeric);
	xDataColumn.replaceValues(0, xData);

	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	XYFitCurve fitCurve("fit");
	fitCurve.setXDataColumn(&xDataColumn);
	fitCurve.setYDataColumn(&yDataColumn);

	//prepare the fit
	XYFitCurve::FitData fitData = fitCurve.fitData();
	fitData.modelCategory = nsl_fit_model_custom;
	XYFitCurve::initFitData(fitData);
	fitData.model = "b1*exp(b2/(x+b3))";
	fitData.paramNames << "b1" << "b2" << "b3";
	//fitData.eps = 1.e-12;
	const int np = fitData.paramNames.size();
	fitData.paramStartValues << 2.0000000000E-02 << 4.0000000000E+03 << 2.5000000000E+02;
	fitData.paramLowerLimits << -std::numeric_limits<double>::max() << -std::numeric_limits<double>::max() << -std::numeric_limits<double>::max();
	fitData.paramUpperLimits << std::numeric_limits<double>::max() << std::numeric_limits<double>::max() << std::numeric_limits<double>::max();
	fitCurve.setFitData(fitData);

	//perform the fit
	fitCurve.recalculate();
	const XYFitCurve::FitResult& fitResult = fitCurve.fitResult();

	//check the results
	QCOMPARE(fitResult.available, true);
	QCOMPARE(fitResult.valid, true);

	QCOMPARE(np, 3);

	DEBUG(std::setprecision(15) << fitResult.paramValues.at(0));	// result: 0.00560963848336205
	FuzzyCompare(fitResult.paramValues.at(0), 5.6096364710E-03, 1.e-5);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(0));	// result: 0.000156888586677272
	FuzzyCompare(fitResult.errorValues.at(0), 1.5687892471E-04, 1.e-4);
	DEBUG(std::setprecision(15) << fitResult.paramValues.at(1));	// result: 6181.34604697191
	FuzzyCompare(fitResult.paramValues.at(1), 6.1813463463E+03, 1.e-6);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(1));	// result: 23.3105167849511
	FuzzyCompare(fitResult.errorValues.at(1), 2.3309021107E+01, 1.e-4);
	DEBUG(std::setprecision(15) << fitResult.paramValues.at(2));	// result: 345.223624540718
	FuzzyCompare(fitResult.paramValues.at(2), 3.4522363462E+02, 1.e-7);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(2));	// result: 0.784914752553929
	FuzzyCompare(fitResult.errorValues.at(2), 7.8486103508E-01, 1.e-4);

	DEBUG(std::setprecision(15) << fitResult.rsd);	// result: 2.6009740064923
	FuzzyCompare(fitResult.rsd, 2.6009740065E+00, 1.e-10);
	DEBUG(std::setprecision(15) << fitResult.sse);	// result: 87.9458551718321
	FuzzyCompare(fitResult.sse, 8.7945855171E+01, 1.e-9);
}

void FitTest::testNonLinearMGH10_3() {
	//NIST data for MGH10 dataset
	QVector<double> xData = {5.000000E+01,5.500000E+01,6.000000E+01,6.500000E+01,7.000000E+01,7.500000E+01,8.000000E+01,8.500000E+01,9.000000E+01,
		9.500000E+01,1.000000E+02,1.050000E+02,1.100000E+02,1.150000E+02,1.200000E+02,1.250000E+02};
	QVector<double> yData = {3.478000E+04,2.861000E+04,2.365000E+04,1.963000E+04,1.637000E+04,1.372000E+04,1.154000E+04,9.744000E+03,8.261000E+03,
		7.030000E+03,6.005000E+03,5.147000E+03,4.427000E+03,3.820000E+03,3.307000E+03,2.872000E+03};

	//data source columns
	Column xDataColumn("x", AbstractColumn::ColumnMode::Numeric);
	xDataColumn.replaceValues(0, xData);

	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	XYFitCurve fitCurve("fit");
	fitCurve.setXDataColumn(&xDataColumn);
	fitCurve.setYDataColumn(&yDataColumn);

	//prepare the fit
	XYFitCurve::FitData fitData = fitCurve.fitData();
	fitData.modelCategory = nsl_fit_model_custom;
	XYFitCurve::initFitData(fitData);
	fitData.model = "b1*exp(b2/(x+b3))";
	fitData.paramNames << "b1" << "b2" << "b3";
	//fitData.eps = 1.e-12;
	const int np = fitData.paramNames.size();
	fitData.paramStartValues << 5.6096364710E-03 << 6.1813463463E+03 << 3.4522363462E+02;
	fitData.paramLowerLimits << -std::numeric_limits<double>::max() << -std::numeric_limits<double>::max() << -std::numeric_limits<double>::max();
	fitData.paramUpperLimits << std::numeric_limits<double>::max() << std::numeric_limits<double>::max() << std::numeric_limits<double>::max();
	fitCurve.setFitData(fitData);

	//perform the fit
	fitCurve.recalculate();
	const XYFitCurve::FitResult& fitResult = fitCurve.fitResult();

	//check the results
	QCOMPARE(fitResult.available, true);
	QCOMPARE(fitResult.valid, true);

	QCOMPARE(np, 3);

	DEBUG(std::setprecision(15) << fitResult.paramValues.at(0));	// result: 0.00560963848336205
	FuzzyCompare(fitResult.paramValues.at(0), 5.6096364710E-03, 1.e-6);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(0));	// result: 0.000156888348998521
	FuzzyCompare(fitResult.errorValues.at(0), 1.5687892471E-04, 1.e-4);
	DEBUG(std::setprecision(15) << fitResult.paramValues.at(1));	// result: 6181.34604697191
	FuzzyCompare(fitResult.paramValues.at(1), 6.1813463463E+03, 1.e-7);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(1));	// result: 23.310508065631
	FuzzyCompare(fitResult.errorValues.at(1), 2.3309021107E+01, 1.e-4);
	DEBUG(std::setprecision(15) << fitResult.paramValues.at(2));	// result: 345.223624540718
	FuzzyCompare(fitResult.paramValues.at(2), 3.4522363462E+02, 1.e-7);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(2));	// result: 0.784914424350028
	FuzzyCompare(fitResult.errorValues.at(2), 7.8486103508E-01, 1.e-4);

	DEBUG(std::setprecision(15) << fitResult.rsd);	// result: 2.6009740064923
	FuzzyCompare(fitResult.rsd, 2.6009740065E+00, 1.e-11);
	DEBUG(std::setprecision(15) << fitResult.sse);	// result: 87.9458551718321
	FuzzyCompare(fitResult.sse, 8.7945855171E+01, 1.e-11);
}

void FitTest::testNonLinearRat43() {
	//NIST data for Rat43 dataset
	QVector<double> xData = {1.0E0,2.0E0,3.0E0,4.0E0,5.0E0,6.0E0,7.0E0,8.0E0,9.0E0,10.0E0,11.0E0,12.0E0,13.0E0,14.0E0,15.0E0};
	QVector<double> yData = {16.08E0,33.83E0,65.80E0,97.20E0,191.55E0,326.20E0,386.87E0,520.53E0,590.03E0,651.92E0,724.93E0,699.56E0,689.96E0,637.56E0,717.41E0};

	//data source columns
	Column xDataColumn("x", AbstractColumn::ColumnMode::Numeric);
	xDataColumn.replaceValues(0, xData);

	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	XYFitCurve fitCurve("fit");
	fitCurve.setXDataColumn(&xDataColumn);
	fitCurve.setYDataColumn(&yDataColumn);

	//prepare the fit
	XYFitCurve::FitData fitData = fitCurve.fitData();
	fitData.modelCategory = nsl_fit_model_custom;
	XYFitCurve::initFitData(fitData);
	fitData.model = "b1/pow(1 + exp(b2-b3*x), 1/b4)";
	fitData.paramNames << "b1" << "b2" << "b3" << "b4";
	fitData.eps = 1.e-9;
	const int np = fitData.paramNames.size();
	fitData.paramStartValues << 1.0000000000E+02 << 1.0000000000E+01 << 1.0000000000E+00 << 1.0000000000E+00;
	fitData.paramLowerLimits << -std::numeric_limits<double>::max() << -std::numeric_limits<double>::max() << -std::numeric_limits<double>::max() << -std::numeric_limits<double>::max();
	fitData.paramUpperLimits << std::numeric_limits<double>::max() << std::numeric_limits<double>::max() << std::numeric_limits<double>::max() << std::numeric_limits<double>::max();
	fitCurve.setFitData(fitData);

	//perform the fit
	fitCurve.recalculate();
	const XYFitCurve::FitResult& fitResult = fitCurve.fitResult();

	//check the results
	QCOMPARE(fitResult.available, true);
	QCOMPARE(fitResult.valid, true);

	QCOMPARE(np, 4);

	DEBUG(std::setprecision(15) << fitResult.paramValues.at(0));	// result: 699.641340982193
	FuzzyCompare(fitResult.paramValues.at(0), 6.9964151270E+02, 1.e-6);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(0));	// result: 16.3022524293302
	FuzzyCompare(fitResult.errorValues.at(0), 1.6302297817E+01, 1.e-5);
	DEBUG(std::setprecision(15) << fitResult.paramValues.at(1));	// result: 5.2771555758844
	FuzzyCompare(fitResult.paramValues.at(1), 5.2771253025E+00, 1.e-5);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(1));	// result: 2.08290034908325
	FuzzyCompare(fitResult.errorValues.at(1), 2.0828735829E+00, 1.e-4);
	DEBUG(std::setprecision(15) << fitResult.paramValues.at(2));	// result: 0.759632366113637
	FuzzyCompare(fitResult.paramValues.at(2), 7.5962938329E-01, 1.e-5);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(2));	// result: 0.195664372178938
	FuzzyCompare(fitResult.errorValues.at(2), 1.9566123451E-01, 1.e-4);
	DEBUG(std::setprecision(15) << fitResult.paramValues.at(3));	// result: 1.27925748993867
	FuzzyCompare(fitResult.paramValues.at(3), 1.2792483859E+00, 1.e-5);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(3));	// result: 0.687627478905195
	FuzzyCompare(fitResult.errorValues.at(3), 6.8761936385E-01, 1.e-4);

	DEBUG(std::setprecision(15) << fitResult.rsd);	// result: 28.2624146626284
	FuzzyCompare(fitResult.rsd, 2.8262414662E+01, 1.e-10);
	DEBUG(std::setprecision(15) << fitResult.sse);	// result: 8786.4049081859
	FuzzyCompare(fitResult.sse, 8.7864049080E+03, 1.e-10);
}

void FitTest::testNonLinearRat43_2() {
	//NIST data for Rat43 dataset
	QVector<double> xData = {1.0E0,2.0E0,3.0E0,4.0E0,5.0E0,6.0E0,7.0E0,8.0E0,9.0E0,10.0E0,11.0E0,12.0E0,13.0E0,14.0E0,15.0E0};
	QVector<double> yData = {16.08E0,33.83E0,65.80E0,97.20E0,191.55E0,326.20E0,386.87E0,520.53E0,590.03E0,651.92E0,724.93E0,699.56E0,689.96E0,637.56E0,717.41E0};

	//data source columns
	Column xDataColumn("x", AbstractColumn::ColumnMode::Numeric);
	xDataColumn.replaceValues(0, xData);

	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	XYFitCurve fitCurve("fit");
	fitCurve.setXDataColumn(&xDataColumn);
	fitCurve.setYDataColumn(&yDataColumn);

	//prepare the fit
	XYFitCurve::FitData fitData = fitCurve.fitData();
	fitData.modelCategory = nsl_fit_model_custom;
	XYFitCurve::initFitData(fitData);
	fitData.model = "b1/pow(1 + exp(b2-b3*x), 1/b4)";
	fitData.paramNames << "b1" << "b2" << "b3" << "b4";
	fitData.eps = 1.e-10;
	const int np = fitData.paramNames.size();
	fitData.paramStartValues << 7.0000000000E+02 << 5.0000000000E+00 << 7.5000000000E-01 << 1.3000000000E+00;
	fitData.paramLowerLimits << -std::numeric_limits<double>::max() << -std::numeric_limits<double>::max() << -std::numeric_limits<double>::max() << -std::numeric_limits<double>::max();
	fitData.paramUpperLimits << std::numeric_limits<double>::max() << std::numeric_limits<double>::max() << std::numeric_limits<double>::max() << std::numeric_limits<double>::max();
	fitCurve.setFitData(fitData);

	//perform the fit
	fitCurve.recalculate();
	const XYFitCurve::FitResult& fitResult = fitCurve.fitResult();

	//check the results
	QCOMPARE(fitResult.available, true);
	QCOMPARE(fitResult.valid, true);

	QCOMPARE(np, 4);

	DEBUG(std::setprecision(15) << fitResult.paramValues.at(0));	// result: 699.641340982193
	FuzzyCompare(fitResult.paramValues.at(0), 6.9964151270E+02, 1.e-6);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(0));	// result: 16.3023134145464 (FreeBSD: 16.3023141645004)
	FuzzyCompare(fitResult.errorValues.at(0), 1.6302297817E+01, 1.e-5);
	DEBUG(std::setprecision(15) << fitResult.paramValues.at(1));	// result: 5.2771555758844
	FuzzyCompare(fitResult.paramValues.at(1), 5.2771253025E+00, 1.e-5);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(1));	// result: 2.08288970703906
	FuzzyCompare(fitResult.errorValues.at(1), 2.0828735829E+00, 1.e-5);
	DEBUG(std::setprecision(15) << fitResult.paramValues.at(2));	// result: 0.759632366113637
	FuzzyCompare(fitResult.paramValues.at(2), 7.5962938329E-01, 1.e-5);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(2));	// result: 0.195662802156063
	FuzzyCompare(fitResult.errorValues.at(2), 1.9566123451E-01, 1.e-5);
	DEBUG(std::setprecision(15) << fitResult.paramValues.at(3));	// result: 1.27925748993867
	FuzzyCompare(fitResult.paramValues.at(3), 1.2792483859E+00, 1.e-5);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(3));	// result: 0.687623222160242
	FuzzyCompare(fitResult.errorValues.at(3), 6.8761936385E-01, 1.e-5);

	DEBUG(std::setprecision(15) << fitResult.rsd);	// result: 28.2624146626284
	FuzzyCompare(fitResult.rsd, 2.8262414662E+01, 1.e-10);
	DEBUG(std::setprecision(15) << fitResult.sse);	// result: 8786.4049081859
	FuzzyCompare(fitResult.sse, 8.7864049080E+03, 1.e-11);
}

void FitTest::testNonLinearRat43_3() {
	//NIST data for Rat43 dataset
	QVector<double> xData = {1.0E0,2.0E0,3.0E0,4.0E0,5.0E0,6.0E0,7.0E0,8.0E0,9.0E0,10.0E0,11.0E0,12.0E0,13.0E0,14.0E0,15.0E0};
	QVector<double> yData = {16.08E0,33.83E0,65.80E0,97.20E0,191.55E0,326.20E0,386.87E0,520.53E0,590.03E0,651.92E0,724.93E0,699.56E0,689.96E0,637.56E0,717.41E0};

	//data source columns
	Column xDataColumn("x", AbstractColumn::ColumnMode::Numeric);
	xDataColumn.replaceValues(0, xData);

	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	XYFitCurve fitCurve("fit");
	fitCurve.setXDataColumn(&xDataColumn);
	fitCurve.setYDataColumn(&yDataColumn);

	//prepare the fit
	XYFitCurve::FitData fitData = fitCurve.fitData();
	fitData.modelCategory = nsl_fit_model_custom;
	XYFitCurve::initFitData(fitData);
	fitData.model = "b1/pow(1 + exp(b2-b3*x), 1/b4)";
	fitData.paramNames << "b1" << "b2" << "b3" << "b4";
	//fitData.eps = 1.e-12;
	const int np = fitData.paramNames.size();
	fitData.paramStartValues << 6.9964151270E+02 << 5.2771253025E+00 << 7.5962938329E-01 << 1.2792483859E+00;
	fitData.paramLowerLimits << -std::numeric_limits<double>::max() << -std::numeric_limits<double>::max() << -std::numeric_limits<double>::max() << -std::numeric_limits<double>::max();
	fitData.paramUpperLimits << std::numeric_limits<double>::max() << std::numeric_limits<double>::max() << std::numeric_limits<double>::max() << std::numeric_limits<double>::max();
	fitCurve.setFitData(fitData);

	//perform the fit
	fitCurve.recalculate();
	const XYFitCurve::FitResult& fitResult = fitCurve.fitResult();

	//check the results
	QCOMPARE(fitResult.available, true);
	QCOMPARE(fitResult.valid, true);

	QCOMPARE(np, 4);

	DEBUG(std::setprecision(15) << fitResult.paramValues.at(0));	// result: 699.641340982193
	FuzzyCompare(fitResult.paramValues.at(0), 6.9964151270E+02, 1.e-8);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(0));	// result: 16.3022905400761
	FuzzyCompare(fitResult.errorValues.at(0), 1.6302297817E+01, 1.e-6);
	DEBUG(std::setprecision(15) << fitResult.paramValues.at(1));	// result: 5.2771555758844
	FuzzyCompare(fitResult.paramValues.at(1), 5.2771253025E+00, 1.e-8);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(1));	// result: 2.08289316520407
	FuzzyCompare(fitResult.errorValues.at(1), 2.0828735829E+00, 1.e-5);
	DEBUG(std::setprecision(15) << fitResult.paramValues.at(2));	// result: 0.759632366113637
	FuzzyCompare(fitResult.paramValues.at(2), 7.5962938329E-01, 1.e-8);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(2));	// result: 0.195663312668779
	FuzzyCompare(fitResult.errorValues.at(2), 1.9566123451E-01, 1.e-4);
	DEBUG(std::setprecision(15) << fitResult.paramValues.at(3));	// result: 1.27925748993867
	FuzzyCompare(fitResult.paramValues.at(3), 1.2792483859E+00, 1.e-8);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(3));	// result: 0.687624541478887
	FuzzyCompare(fitResult.errorValues.at(3), 6.8761936385E-01, 1.e-5);

	DEBUG(std::setprecision(15) << fitResult.rsd);	// result: 28.2624146626284
	FuzzyCompare(fitResult.rsd, 2.8262414662E+01, 1.e-11);
	DEBUG(std::setprecision(15) << fitResult.sse);	// result: 8786.4049081859
	FuzzyCompare(fitResult.sse, 8.7864049080E+03, 1.e-11);
}

// https://bugs.kde.org/show_bug.cgi?id=393213
void FitTest::testNonLinearMichaelis_Menten() {
	// generic data
	QVector<double> xData = {0.0,0.2,0.4,0.6,0.8,1.0,1.2,1.4,1.6,1.8,2.0};
	QVector<double> yData = {0.0,0.6,0.65,0.7,0.75,0.75,0.8,0.9,0.85,0.95,0.9};

	//data source columns
	Column xDataColumn("x", AbstractColumn::ColumnMode::Numeric);
	xDataColumn.replaceValues(0, xData);

	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	XYFitCurve fitCurve("fit");
	fitCurve.setXDataColumn(&xDataColumn);
	fitCurve.setYDataColumn(&yDataColumn);

	//prepare the fit
	XYFitCurve::FitData fitData = fitCurve.fitData();
	fitData.modelCategory = nsl_fit_model_custom;
	XYFitCurve::initFitData(fitData);
	fitData.model = "Vm * x/(Km + x)";
	fitData.paramNames << "Vm" << "Km";
	fitData.eps = 1.e-12;
	const int np = fitData.paramNames.size();
	fitData.paramStartValues << 1.0 << 1.0 ;
	for (int i = 0; i < np; i++) {
		fitData.paramLowerLimits << -std::numeric_limits<double>::max();
		fitData.paramUpperLimits << std::numeric_limits<double>::max();
	}
	fitCurve.setFitData(fitData);

	//perform the fit
	fitCurve.recalculate();
	const XYFitCurve::FitResult& fitResult = fitCurve.fitResult();

	//check the results
	QCOMPARE(fitResult.available, true);
	QCOMPARE(fitResult.valid, true);

	QCOMPARE(np, 2);

	DEBUG(std::setprecision(15) << fitResult.paramValues.at(0));	// result: 0.945561082955744
	FuzzyCompare(fitResult.paramValues.at(0), 0.94556434933256, 1.e-5);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(0));	// result: 0.0388724403232334
	FuzzyCompare(fitResult.errorValues.at(0), 0.0388803714011844, 3.e-4);
	DEBUG(std::setprecision(15) << fitResult.paramValues.at(1));	// result: 0.159396945453528
	FuzzyCompare(fitResult.paramValues.at(1), 0.159400761666661, 3.e-5);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(1));	// result: 0.0388244491752518
	FuzzyCompare(fitResult.errorValues.at(1), 0.0388429738447119, 5.e-4);

	DEBUG(std::setprecision(15) << fitResult.rms);	// result: 0.00280486748619082
	FuzzyCompare(fitResult.rms, 0.00280486748877263, 1.e-9);
	DEBUG(std::setprecision(15) << fitResult.rsd);	// result: 0.0529609996713697
	FuzzyCompare(fitResult.rsd, 0.0529609996957444, 1.e-9);
	DEBUG(std::setprecision(15) << fitResult.sse);	// result: 0.0252438073757174
	FuzzyCompare(fitResult.sse, 0.0252438073989537, 1.e-9);
}
//##############################################################################
//#########################  Fits with weights #################################
//##############################################################################

// see http://gnuplot.sourceforge.net/demo_5.2/fit.html
void FitTest::testNonLinearGP_lcdemo() {
	// data from https://github.com/gnuplot/gnuplot/blob/master/demo/lcdemo.dat
	QVector<double> xData = {39.471,40.091,40.602,41.058,41.438,41.880,42.437,42.836,43.209,43.599,43.997,44.313,44.908,45.169,45.594,45.743,45.796,45.816,
		45.841,45.876,45.908,45.959,46.008,46.040,46.060,46.096,46.126,46.149,46.372,46.625,46.945,47.326,47.708,48.095,48.540,48.927,49.314};
	QVector<double> yData = {1.03307,1.03246,1.03197,1.03153,1.03117,1.03074,1.03021,1.02982,1.02946,1.02907,1.02867,1.02833,1.02765,1.02735,1.02683,
		1.02661,1.02650,1.02644,1.02634,1.02623,1.02611,1.02592,1.02561,1.02526,1.02506,1.02500,1.02496,1.02494,1.02474,1.02452,1.02425,1.02393,
		1.02361,1.02329,1.02293,1.02262,1.02231};
	QVector<double> yError = {0.010,0.010,0.010,0.010,0.010,0.010,0.010,0.010,0.010,0.010,0.010,0.010,0.010,0.010,0.010,0.010,0.010,0.010,0.010,0.010,0.010,
		0.010,0.001,0.001,0.001,0.001,0.001,0.001,0.001,0.001,0.001,0.001,0.001,0.001,0.001,0.001,0.001};

	//data source columns
	Column xDataColumn("x", AbstractColumn::ColumnMode::Numeric);
	xDataColumn.replaceValues(0, xData);

	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	Column yErrorColumn("yerr", AbstractColumn::ColumnMode::Numeric);
	yErrorColumn.replaceValues(0, yError);

	XYFitCurve fitCurve("fit");
	fitCurve.setXDataColumn(&xDataColumn);
	fitCurve.setYDataColumn(&yDataColumn);
	fitCurve.setYErrorColumn(&yErrorColumn);

	//prepare the fit
	XYFitCurve::FitData fitData = fitCurve.fitData();
	fitData.modelCategory = nsl_fit_model_custom;
	XYFitCurve::initFitData(fitData);
	// x > Tc : d + mh(x-Tc)
	// x < Tc : d + ml(x-Tc) + b tanh(g(Tc-x))
	fitData.model = "d + theta(x-Tc)*mh*(x-Tc) + theta(Tc-x)*(ml*(x-Tc)+b*tanh(g*(Tc-x)))";
	fitData.paramNames << "d" << "Tc" << "mh" << "ml" << "b" << "g";
	fitData.eps = 1.e-12;
	const int np = fitData.paramNames.size();
	fitData.paramStartValues << 1.02 << 45. << -0.0005 << -0.0005 << 0.01002 << 1.0;
	for (int i = 0; i < np; i++) {
		fitData.paramLowerLimits << -std::numeric_limits<double>::max();
		fitData.paramUpperLimits << std::numeric_limits<double>::max();
	}
	fitData.yWeightsType = nsl_fit_weight_instrumental;
	fitCurve.setFitData(fitData);

	//perform the fit
	fitCurve.recalculate();
	const XYFitCurve::FitResult& fitResult = fitCurve.fitResult();

	//check the results
	QCOMPARE(fitResult.available, true);
	QCOMPARE(fitResult.valid, true);

	QCOMPARE(np, 6);

	DEBUG(std::setprecision(15) << fitResult.paramValues.at(0));	// result: 1.02499979307627 (Windows: 1.02561781433026)
	FuzzyCompare(fitResult.paramValues.at(0), 1.02499621370905, 1.e-3);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(0));	// result: 4.81672854812941e-06
//TODO	FuzzyCompare(fitResult.errorValues.at(0), 7.27819513635249e-06, 1.e-6);
	DEBUG(std::setprecision(15) << fitResult.paramValues.at(1));	// result: 46.0647953740441 (Windows: 45.4871250830364)
	FuzzyCompare(fitResult.paramValues.at(1), 46.0665367045608, 2.e-2);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(1));	// result: 9.90288940612482
//TODO	FuzzyCompare(fitResult.errorValues.at(1), 0.00159887430059728, 1.e-6);
	DEBUG(std::setprecision(15) << fitResult.paramValues.at(2));	// result:  -0.000835023995828296 (Windows: -0.000883960665773456)
	FuzzyCompare(fitResult.paramValues.at(2), -0.0008340717673769, 1.e-1);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(2));	// result:
//TODO	FuzzyCompare(fitResult.errorValues.at(2), , 1.e-6);
	DEBUG(std::setprecision(15) << fitResult.paramValues.at(3));	// result: -0.000987547207638997
	FuzzyCompare(fitResult.paramValues.at(3), -0.00103152542276233, 1.e-1);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(3));	// result:
//TODO	FuzzyCompare(fitResult.errorValues.at(3), , 1.e-6);
	DEBUG(std::setprecision(15) << fitResult.paramValues.at(4));	// result: 0.00158880319355268
	FuzzyCompare(fitResult.paramValues.at(4), 0.00139548391000006, 1.5e-1);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(4));	// result:
//TODO	FuzzyCompare(fitResult.errorValues.at(4), , 1.e-6);
	DEBUG(std::setprecision(15) << fitResult.paramValues.at(5));	// result: 6.34254053273612 (Windows: 4.45482397068125)
//	FuzzyCompare(fitResult.paramValues.at(5), 6.92493866108287, 1.e-1);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(5));	// result:
//TODO	FuzzyCompare(fitResult.errorValues.at(5), , 1.e-6);

	DEBUG(std::setprecision(15) << fitResult.rms);	// result: 98.0672185899393
//	FuzzyCompare(fitResult.rms, 0.000188776, 1.e-11);
	DEBUG(std::setprecision(15) << fitResult.rsd);	// result: 9.90288940612482
//	FuzzyCompare(fitResult.rsd, 0.0137395924378767, 1.e-11);
	DEBUG(std::setprecision(15) << fitResult.sse);	// result: 3040.08377628812
//	FuzzyCompare(fitResult.sse, 0.00585206841112775, 1.e-11);
}

// see http://gnuplot.sourceforge.net/demo_5.2/fit.html
void FitTest::testLinearGP_PY_noerror() {
	// Pearson's data and York's weights
	QVector<double> xData = {0.0,0.9,1.8,2.6,3.3,4.4,5.2,6.1,6.5,7.4};
	QVector<double> yData = {5.9,5.4,4.4,4.6,3.5,3.7,2.8,2.8,2.4,1.5};
//	QVector<double> xError = {1000.,1000.,500.,800.,200.,80.,60.,20.,1.8,1.0};
//	QVector<double> yError = {1.0,1.8,4.,8.,20.,20.,70.,70.,100.,500.};

	//data source columns
	Column xDataColumn("x", AbstractColumn::ColumnMode::Numeric);
	xDataColumn.replaceValues(0, xData);

	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

//	Column yErrorColumn("yerr", AbstractColumn::ColumnMode::Numeric);
//	yErrorColumn.replaceValues(0, yError);

	XYFitCurve fitCurve("fit");
	fitCurve.setXDataColumn(&xDataColumn);
	fitCurve.setYDataColumn(&yDataColumn);
//	fitCurve.setYErrorColumn(&yErrorColumn);

	//prepare the fit
	XYFitCurve::FitData fitData = fitCurve.fitData();
	fitData.modelCategory = nsl_fit_model_custom;
	XYFitCurve::initFitData(fitData);
	fitData.model = "a1 + a2 * x";
	fitData.paramNames << "a1" << "a2";
	fitData.eps = 1.e-9;
	const int np = fitData.paramNames.size();
	fitData.paramStartValues << 5. << -0.5;
	for (int i = 0; i < np; i++) {
		fitData.paramLowerLimits << -std::numeric_limits<double>::max();
		fitData.paramUpperLimits << std::numeric_limits<double>::max();
	}
//	fitData.yWeightsType = nsl_fit_weight_instrumental;
	fitCurve.setFitData(fitData);

	//perform the fit
	fitCurve.recalculate();
	const XYFitCurve::FitResult& fitResult = fitCurve.fitResult();

	//check the results
	QCOMPARE(fitResult.available, true);
	QCOMPARE(fitResult.valid, true);

	QCOMPARE(np, 2);

	DEBUG(std::setprecision(15) << fitResult.paramValues.at(0));	// result: 5.76118518568804
	FuzzyCompare(fitResult.paramValues.at(0), 5.76118519043878, 1.e-9);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(0));	// result: 0.189485192391863
	FuzzyCompare(fitResult.errorValues.at(0), 0.189485195921141, 1.e-7);
	DEBUG(std::setprecision(15) << fitResult.paramValues.at(1));	// result: -0.539577274076964
	FuzzyCompare(fitResult.paramValues.at(1), -0.539577274983977, 1.e-8);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(1));	// result: 0.0421265487265946
	FuzzyCompare(fitResult.errorValues.at(1), 0.0421265483886995, 1.e-8);

	DEBUG(std::setprecision(15) << fitResult.rms);	// result: 0.100082940279452
	QCOMPARE(fitResult.rms, 0.100082940279452);
	DEBUG(std::setprecision(15) << fitResult.rsd);	// result: 0.316358878932538
	QCOMPARE(fitResult.rsd, 0.316358878932538);
	DEBUG(std::setprecision(15) << fitResult.sse);	// result: 0.800663522235619
	QCOMPARE(fitResult.sse, 0.800663522235619);
}

// see http://gnuplot.sourceforge.net/demo_5.2/fit.html
void FitTest::testLinearGP_PY_yerror_polynomial() {
	// Pearson's data and York's weights
	QVector<double> xData = {0.0,0.9,1.8,2.6,3.3,4.4,5.2,6.1,6.5,7.4};
	QVector<double> yData = {5.9,5.4,4.4,4.6,3.5,3.7,2.8,2.8,2.4,1.5};
//	QVector<double> xError = {1000.,1000.,500.,800.,200.,80.,60.,20.,1.8,1.0};
	QVector<double> yError = {1.0,1.8,4.,8.,20.,20.,70.,70.,100.,500.};

	//data source columns
	Column xDataColumn("x", AbstractColumn::ColumnMode::Numeric);
	xDataColumn.replaceValues(0, xData);

	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	Column yErrorColumn("yerr", AbstractColumn::ColumnMode::Numeric);
	yErrorColumn.replaceValues(0, yError);

	XYFitCurve fitCurve("fit");
	fitCurve.setXDataColumn(&xDataColumn);
	fitCurve.setYDataColumn(&yDataColumn);
	fitCurve.setYErrorColumn(&yErrorColumn);

	//prepare the fit
	XYFitCurve::FitData fitData = fitCurve.fitData();
	fitData.modelCategory = nsl_fit_model_basic;
	fitData.modelType = nsl_fit_model_polynomial;
	fitData.degree = 1;
	XYFitCurve::initFitData(fitData);
//	fitData.eps = 1.e-12;
	const int np = fitData.paramNames.size();
	fitData.paramStartValues << 5. << -0.5;
	fitData.yWeightsType = nsl_fit_weight_direct;
	fitCurve.setFitData(fitData);

	//perform the fit
	fitCurve.recalculate();
	const XYFitCurve::FitResult& fitResult = fitCurve.fitResult();

	//check the results
	QCOMPARE(fitResult.available, true);
	QCOMPARE(fitResult.valid, true);

	QCOMPARE(np, 2);

	DEBUG(std::setprecision(15) << fitResult.paramValues.at(0));	// result: 6.10010931666575
	FuzzyCompare(fitResult.paramValues.at(0), 6.10010931635002, 1.e-10);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(0));	// result: 0.424059452104775, i386: 0.424059429083679
	FuzzyCompare(fitResult.errorValues.at(0), 0.424059452104785, 1.e-10);
	DEBUG(std::setprecision(15) << fitResult.paramValues.at(1));	// result: -0.610812956583933, i386: -0.610812954591566
	FuzzyCompare(fitResult.paramValues.at(1), -0.610812956537254, 1.e-9);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(1));	// result: 0.0623409539388997, i386: 0.0623409508171503
	FuzzyCompare(fitResult.errorValues.at(1), 0.0623409539389024, 1.e-10);

	QCOMPARE(fitResult.rms, 4.29315093729054);
	QCOMPARE(fitResult.rsd, 2.07199202153158);
	QCOMPARE(fitResult.sse, 34.3452074983243);
	DEBUG(std::setprecision(15) << fitResult.fdist_p);	// result: 0.000101015996328551
//TODO	QCOMPARE(fitResult.fdist_p, 3.51725605201025e-05);
}

// see http://gnuplot.sourceforge.net/demo_5.2/fit.html
void FitTest::testLinearGP_PY_yerror_custom() {
	// Pearson's data and York's weights
	QVector<double> xData = {0.0,0.9,1.8,2.6,3.3,4.4,5.2,6.1,6.5,7.4};
	QVector<double> yData = {5.9,5.4,4.4,4.6,3.5,3.7,2.8,2.8,2.4,1.5};
//	QVector<double> xError = {1000.,1000.,500.,800.,200.,80.,60.,20.,1.8,1.0};
	QVector<double> yError = {1.0,1.8,4.,8.,20.,20.,70.,70.,100.,500.};

	//data source columns
	Column xDataColumn("x", AbstractColumn::ColumnMode::Numeric);
	xDataColumn.replaceValues(0, xData);

	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	Column yErrorColumn("yerr", AbstractColumn::ColumnMode::Numeric);
	yErrorColumn.replaceValues(0, yError);

	XYFitCurve fitCurve("fit");
	fitCurve.setXDataColumn(&xDataColumn);
	fitCurve.setYDataColumn(&yDataColumn);
	fitCurve.setYErrorColumn(&yErrorColumn);

	//prepare the fit
	XYFitCurve::FitData fitData = fitCurve.fitData();
	fitData.modelCategory = nsl_fit_model_custom;
	XYFitCurve::initFitData(fitData);
	fitData.model = "a1 + a2 * x";
	fitData.paramNames << "a1" << "a2";
	fitData.eps = 1.e-9;
	const int np = fitData.paramNames.size();
	fitData.paramStartValues << 5. << -0.5;
	for (int i = 0; i < np; i++) {
		fitData.paramLowerLimits << -std::numeric_limits<double>::max();
		fitData.paramUpperLimits << std::numeric_limits<double>::max();
	}

	fitData.yWeightsType = nsl_fit_weight_direct;
	fitCurve.setFitData(fitData);

	//perform the fit
	fitCurve.recalculate();
	const XYFitCurve::FitResult& fitResult = fitCurve.fitResult();

	//check the results
	QCOMPARE(fitResult.available, true);
	QCOMPARE(fitResult.valid, true);

	QCOMPARE(np, 2);

	DEBUG(std::setprecision(15) << fitResult.paramValues.at(0));	// result: 6.10010932451396
	FuzzyCompare(fitResult.paramValues.at(0), 6.10010931635002, 1.e-8);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(0));	// result: 0.424059453530443 (new: 0.424059492144904)
	FuzzyCompare(fitResult.errorValues.at(0), 0.424059452104785, 1.e-7);
	DEBUG(std::setprecision(15) << fitResult.paramValues.at(1));	// result: -0.610812957040808 (new: -0.610812955065274)
	FuzzyCompare(fitResult.paramValues.at(1), -0.610812956537254, 1.e-8);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(1));	// result: 0.0623409543258892 (new: 0.0623409596911536)
	FuzzyCompare(fitResult.errorValues.at(1), 0.0623409539389024, 1.e-7);

	QCOMPARE(fitResult.rms, 4.29315093729054);
	QCOMPARE(fitResult.rsd, 2.07199202153158);
	QCOMPARE(fitResult.sse, 34.3452074983243);
	DEBUG(std::setprecision(15) << fitResult.fdist_p);	// result: 0.000101015996328551
//TODO	QCOMPARE(fitResult.fdist_p, 3.51725605201025e-05);
}

// see http://gnuplot.sourceforge.net/demo_5.2/fit.html
void FitTest::testLinearGP_PY_xyerror_polynomial() {
	// Pearson's data and York's weights
	QVector<double> xData = {0.0,0.9,1.8,2.6,3.3,4.4,5.2,6.1,6.5,7.4};
	QVector<double> yData = {5.9,5.4,4.4,4.6,3.5,3.7,2.8,2.8,2.4,1.5};
	QVector<double> xError = {1000.,1000.,500.,800.,200.,80.,60.,20.,1.8,1.0};
	QVector<double> yError = {1.0,1.8,4.,8.,20.,20.,70.,70.,100.,500.};

	//data source columns
	Column xDataColumn("x", AbstractColumn::ColumnMode::Numeric);
	xDataColumn.replaceValues(0, xData);

	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	Column xErrorColumn("xerr", AbstractColumn::ColumnMode::Numeric);
	xErrorColumn.replaceValues(0, xError);

	Column yErrorColumn("yerr", AbstractColumn::ColumnMode::Numeric);
	yErrorColumn.replaceValues(0, yError);

	XYFitCurve fitCurve("fit");
	fitCurve.setXDataColumn(&xDataColumn);
	fitCurve.setYDataColumn(&yDataColumn);
	fitCurve.setXErrorColumn(&xErrorColumn);
	fitCurve.setYErrorColumn(&yErrorColumn);

	//prepare the fit
	XYFitCurve::FitData fitData = fitCurve.fitData();
	fitData.modelCategory = nsl_fit_model_basic;
	fitData.modelType = nsl_fit_model_polynomial;
	fitData.degree = 1;
	XYFitCurve::initFitData(fitData);
	fitData.eps = 1.e-12;
	const int np = fitData.paramNames.size();
	fitData.paramStartValues << 5. << -0.5;
	fitData.xWeightsType = nsl_fit_weight_direct;
	fitData.yWeightsType = nsl_fit_weight_direct;
	fitCurve.setFitData(fitData);

	//perform the fit
	fitCurve.recalculate();
	const XYFitCurve::FitResult& fitResult = fitCurve.fitResult();

	//check the results
	QCOMPARE(fitResult.available, true);
	QCOMPARE(fitResult.valid, true);

	QCOMPARE(np, 2);

	DEBUG(std::setprecision(15) << fitResult.paramValues.at(0));	// result: 5.3960522989993
	FuzzyCompare(fitResult.paramValues.at(0), 5.39749958415886, 3.e-4);	// gnuplot result ("effective variance" method)
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(0));	// result: 0.361458387983792
	FuzzyCompare(fitResult.errorValues.at(0), 0.361439886824914, 1.e-4);	// -""-
	DEBUG(std::setprecision(15) << fitResult.paramValues.at(1));	// result: -0.463448925094435
	FuzzyCompare(fitResult.paramValues.at(1), -0.463744669606207, 1.e-3);	// -""-
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(1));	// result: 0.0706627287153768
	FuzzyCompare(fitResult.errorValues.at(1), 0.0706637562101211, 1.e-4);	// -""-

	DEBUG(std::setprecision(15) << fitResult.rms);	// result: 1.49455608152396
	FuzzyCompare(fitResult.rms, 1.49417194665446, 1.e-3);	// gnuplot result ("effective variance" method)
	DEBUG(std::setprecision(15) << fitResult.rsd);	// result: 1.22252038082151
	FuzzyCompare(fitResult.rsd, 1.22236326296828, 1.e-3);	// -""-
	DEBUG(std::setprecision(15) << fitResult.sse);	// result: 11.9564486521917
	FuzzyCompare(fitResult.sse, 11.9533755732357, 1.e-3);	// -""-
	DEBUG(std::setprecision(15) << fitResult.fdist_p);	// result: 0.00441031749154456
//TODO	QCOMPARE(fitResult.fdist_p, 0.153296328355244);
}

// see http://gnuplot.sourceforge.net/demo_5.2/fit.html
void FitTest::testLinearGP_PY_xyerror_custom() {
	// Pearson's data and York's weights
	QVector<double> xData = {0.0,0.9,1.8,2.6,3.3,4.4,5.2,6.1,6.5,7.4};
	QVector<double> yData = {5.9,5.4,4.4,4.6,3.5,3.7,2.8,2.8,2.4,1.5};
	QVector<double> xError = {1000.,1000.,500.,800.,200.,80.,60.,20.,1.8,1.0};
	QVector<double> yError = {1.0,1.8,4.,8.,20.,20.,70.,70.,100.,500.};

	//data source columns
	Column xDataColumn("x", AbstractColumn::ColumnMode::Numeric);
	xDataColumn.replaceValues(0, xData);

	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	Column xErrorColumn("xerr", AbstractColumn::ColumnMode::Numeric);
	xErrorColumn.replaceValues(0, xError);

	Column yErrorColumn("yerr", AbstractColumn::ColumnMode::Numeric);
	yErrorColumn.replaceValues(0, yError);

	XYFitCurve fitCurve("fit");
	fitCurve.setXDataColumn(&xDataColumn);
	fitCurve.setYDataColumn(&yDataColumn);
	fitCurve.setXErrorColumn(&xErrorColumn);
	fitCurve.setYErrorColumn(&yErrorColumn);

	//prepare the fit
	XYFitCurve::FitData fitData = fitCurve.fitData();
	fitData.modelCategory = nsl_fit_model_custom;
	XYFitCurve::initFitData(fitData);
	fitData.model = "a1 + a2 * x";
	fitData.paramNames << "a1" << "a2";
	fitData.eps = 1.e-12;
	const int np = fitData.paramNames.size();
	fitData.paramStartValues << 5. << -0.5;
	for (int i = 0; i < np; i++) {
		fitData.paramLowerLimits << -std::numeric_limits<double>::max();
		fitData.paramUpperLimits << std::numeric_limits<double>::max();
	}

	fitData.xWeightsType = nsl_fit_weight_direct;
	fitData.yWeightsType = nsl_fit_weight_direct;
	fitCurve.setFitData(fitData);

	//perform the fit
	fitCurve.recalculate();
	const XYFitCurve::FitResult& fitResult = fitCurve.fitResult();

	//check the results
	QCOMPARE(fitResult.available, true);
	QCOMPARE(fitResult.valid, true);

	QCOMPARE(np, 2);

	DEBUG(std::setprecision(15) << fitResult.paramValues.at(0));	// result: 5.39605223831693
	FuzzyCompare(fitResult.paramValues.at(0), 5.39749958415886, 3.e-4);	// gnuplot result ("effective variance" method)
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(0));	// result: 0.361458369696386
	FuzzyCompare(fitResult.errorValues.at(0), 0.361439886824914, 1.e-4);	// -""-
	DEBUG(std::setprecision(15) << fitResult.paramValues.at(1));	// result: -0.463448913528306
	FuzzyCompare(fitResult.paramValues.at(1), -0.463744669606207, 1.e-3);	// -""-
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(1));	// result: 0.0706627257036884
	FuzzyCompare(fitResult.errorValues.at(1), 0.0706637562101211, 1.e-4);	// -""-

	DEBUG(std::setprecision(15) << fitResult.rms);	// result: 1.49455596317092
	FuzzyCompare(fitResult.rms, 1.49417194665446, 1.e-3);	// gnuplot result ("effective variance" method)
	DEBUG(std::setprecision(15) << fitResult.rsd);	// result: 1.22252033241616
	FuzzyCompare(fitResult.rsd, 1.22236326296828, 1.e-3);	// -""-
	DEBUG(std::setprecision(15) << fitResult.sse);	// result: 11.9564477053674
	FuzzyCompare(fitResult.sse, 11.9533755732357, 1.e-3);	// -""-
	DEBUG(std::setprecision(15) << fitResult.fdist_p);	// result: 0.00441031645455255
//TODO	QCOMPARE(fitResult.fdist_p, 0.153296328355244);
}

// see http://gnuplot.sourceforge.net/demo_5.2/fit.html
void FitTest::testLinearGP_PY_xyerror_custom_instrumental_weight() {
	// Pearson's data and York's weights
	QVector<double> xData = {0.0,0.9,1.8,2.6,3.3,4.4,5.2,6.1,6.5,7.4};
	QVector<double> yData = {5.9,5.4,4.4,4.6,3.5,3.7,2.8,2.8,2.4,1.5};
	QVector<double> xError = {1000.,1000.,500.,800.,200.,80.,60.,20.,1.8,1.0};
	QVector<double> yError = {1.0,1.8,4.,8.,20.,20.,70.,70.,100.,500.};

	// w_i -> s_i
	for (int i = 0; i < xError.size(); i++) {
		xError[i] = 1./sqrt(xError[i]);
		yError[i] = 1./sqrt(yError[i]);
	}

	//data source columns
	Column xDataColumn("x", AbstractColumn::ColumnMode::Numeric);
	xDataColumn.replaceValues(0, xData);

	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	Column xErrorColumn("xerr", AbstractColumn::ColumnMode::Numeric);
	xErrorColumn.replaceValues(0, xError);

	Column yErrorColumn("yerr", AbstractColumn::ColumnMode::Numeric);
	yErrorColumn.replaceValues(0, yError);

	XYFitCurve fitCurve("fit");
	fitCurve.setXDataColumn(&xDataColumn);
	fitCurve.setYDataColumn(&yDataColumn);
	fitCurve.setXErrorColumn(&xErrorColumn);
	fitCurve.setYErrorColumn(&yErrorColumn);

	//prepare the fit
	XYFitCurve::FitData fitData = fitCurve.fitData();
	fitData.modelCategory = nsl_fit_model_custom;
	XYFitCurve::initFitData(fitData);
	fitData.model = "a1 + a2 * x";
	fitData.paramNames << "a1" << "a2";
//	fitData.eps = 1.e-12;
	const int np = fitData.paramNames.size();
	fitData.paramStartValues << 5. << -0.5;
	for (int i = 0; i < np; i++) {
		fitData.paramLowerLimits << -std::numeric_limits<double>::max();
		fitData.paramUpperLimits << std::numeric_limits<double>::max();
	}

	fitData.xWeightsType = nsl_fit_weight_instrumental;
	fitData.yWeightsType = nsl_fit_weight_instrumental;
	fitCurve.setFitData(fitData);

	//perform the fit
	fitCurve.recalculate();
	const XYFitCurve::FitResult& fitResult = fitCurve.fitResult();

	//check the results
	QCOMPARE(fitResult.available, true);
	QCOMPARE(fitResult.valid, true);

	QCOMPARE(np, 2);

	DEBUG(std::setprecision(15) << fitResult.paramValues.at(0));	// result: 5.3960521880058
	FuzzyCompare(fitResult.paramValues.at(0), 5.39749958415886, 3.e-4);	// gnuplot result ("effective variance" method)
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(0));	// result: 0.361458367406729
	FuzzyCompare(fitResult.errorValues.at(0), 0.361439886824914, 1.e-4);	// -""-
	DEBUG(std::setprecision(15) << fitResult.paramValues.at(1));	// result: -0.463448904801834
	FuzzyCompare(fitResult.paramValues.at(1), -0.463744669606207, 1.e-3);	// -""-
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(1));	// result: 0.0706627229337577
	FuzzyCompare(fitResult.errorValues.at(1), 0.0706637562101211, 1.e-4);	// -""-

	DEBUG(std::setprecision(15) << fitResult.rms);	// result: 1.49455610878403
	FuzzyCompare(fitResult.rms, 1.49417194665446, 1.e-3);	// gnuplot result ("effective variance" method)
	DEBUG(std::setprecision(15) << fitResult.rsd);	// result: 1.22252039197063
	FuzzyCompare(fitResult.rsd, 1.22236326296828, 1.e-3);	// -""-
	DEBUG(std::setprecision(15) << fitResult.sse);	// result: 11.9564488702722
	FuzzyCompare(fitResult.sse, 11.9533755732357, 1.e-3);	// -""-
	DEBUG(std::setprecision(15) << fitResult.fdist_p);	// result: 0.00441031773039329
//TODO	QCOMPARE(fitResult.fdist_p, 0.153296328355244);
}

// see http://gnuplot.sourceforge.net/demo_5.2/fit.html
void FitTest::testLinearGP_PY_xyerror_custom_inverse_weight() {
	// Pearson's data and York's weights
	QVector<double> xData = {0.0,0.9,1.8,2.6,3.3,4.4,5.2,6.1,6.5,7.4};
	QVector<double> yData = {5.9,5.4,4.4,4.6,3.5,3.7,2.8,2.8,2.4,1.5};
	QVector<double> xError = {1000.,1000.,500.,800.,200.,80.,60.,20.,1.8,1.0};
	QVector<double> yError = {1.0,1.8,4.,8.,20.,20.,70.,70.,100.,500.};

	// w_i -> 1/w_i
	for (int i = 0; i < xError.size(); i++) {
		xError[i] = 1./xError[i];
		yError[i] = 1./yError[i];
	}

	//data source columns
	Column xDataColumn("x", AbstractColumn::ColumnMode::Numeric);
	xDataColumn.replaceValues(0, xData);

	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	Column xErrorColumn("xerr", AbstractColumn::ColumnMode::Numeric);
	xErrorColumn.replaceValues(0, xError);

	Column yErrorColumn("yerr", AbstractColumn::ColumnMode::Numeric);
	yErrorColumn.replaceValues(0, yError);

	XYFitCurve fitCurve("fit");
	fitCurve.setXDataColumn(&xDataColumn);
	fitCurve.setYDataColumn(&yDataColumn);
	fitCurve.setXErrorColumn(&xErrorColumn);
	fitCurve.setYErrorColumn(&yErrorColumn);

	//prepare the fit
	XYFitCurve::FitData fitData = fitCurve.fitData();
	fitData.modelCategory = nsl_fit_model_custom;
	XYFitCurve::initFitData(fitData);
	fitData.model = "a1 + a2 * x";
	fitData.paramNames << "a1" << "a2";
//	fitData.eps = 1.e-12;
	const int np = fitData.paramNames.size();
	fitData.paramStartValues << 5. << -0.5;
	for (int i = 0; i < np; i++) {
		fitData.paramLowerLimits << -std::numeric_limits<double>::max();
		fitData.paramUpperLimits << std::numeric_limits<double>::max();
	}

	fitData.xWeightsType = nsl_fit_weight_inverse;
	fitData.yWeightsType = nsl_fit_weight_inverse;
	fitCurve.setFitData(fitData);

	//perform the fit
	fitCurve.recalculate();
	const XYFitCurve::FitResult& fitResult = fitCurve.fitResult();

	//check the results
	QCOMPARE(fitResult.available, true);
	QCOMPARE(fitResult.valid, true);

	QCOMPARE(np, 2);

	DEBUG(std::setprecision(15) << fitResult.paramValues.at(0));	// result: 5.39605219384715
	FuzzyCompare(fitResult.paramValues.at(0), 5.39749958415886, 3.e-4);	// gnuplot result ("effective variance" method)
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(0));	// result: 0.361458367208767
	FuzzyCompare(fitResult.errorValues.at(0), 0.361439886824914, 1.e-4);	// -""-
	DEBUG(std::setprecision(15) << fitResult.paramValues.at(1));	// result: -0.463448906090293
	FuzzyCompare(fitResult.paramValues.at(1), -0.463744669606207, 1.e-3);	// -""-
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(1));	// result: 0.0706627222052917
	FuzzyCompare(fitResult.errorValues.at(1), 0.0706637562101211, 1.e-4);	// -""-

	DEBUG(std::setprecision(15) << fitResult.rms);	// result: 1.49455610679827
	FuzzyCompare(fitResult.rms, 1.49417194665446, 1.e-3);	// gnuplot result ("effective variance" method)
	DEBUG(std::setprecision(15) << fitResult.rsd);	// result: 1.22252039115847
	FuzzyCompare(fitResult.rsd, 1.22236326296828, 1.e-3);	// -""-
	DEBUG(std::setprecision(15) << fitResult.sse);	// result: 11.9564488543861
	FuzzyCompare(fitResult.sse, 11.9533755732357, 1.e-3);	// -""-
	DEBUG(std::setprecision(15) << fitResult.fdist_p);	// result: 0.00441031771299435
//TODO	QCOMPARE(fitResult.fdist_p, 0.153296328355244);
}

// see https://bugs.kde.org/show_bug.cgi?id=408535
void FitTest::testNonLinear_yerror_zero_bug408535() {
	QVector<double> xData = {0.0,320.,320.,360.,360.,400.,400.,440.,440.,480.,480.,520.,520.,560.,560.,600.,600.,640.,640.,680.,680.,720.,720.,760.,760.,800.,800.,840.,840.,880., 880.,1200.,1200.,1200.,1200.};
	QVector<double> yData = {0.0,160.,120.,190.,120.,210.,140.,220.,170.,230.,190.,240.,210.,250.,230.,270.,240.,260.,270.,290.,240.,310.,270.,320.,290.,330.,290.,320.,300.,330.,310.,370.,390.,390.,400.};
	QVector<double> yError = {0.0,15.,15.,15.,15.,15.,15.,15.,15.,15.,15.,15.,15.,15.,15.,15.,15.,15.,15.,15.,15.,15.,15.,15.,15.,15.,15.,15.,15.,15.,15.,15.,15.,15.,15.};

	//data source columns
	Column xDataColumn("x", AbstractColumn::ColumnMode::Numeric);
	xDataColumn.replaceValues(0, xData);

	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	Column yErrorColumn("yerr", AbstractColumn::ColumnMode::Numeric);
	yErrorColumn.replaceValues(0, yError);

	XYFitCurve fitCurve("fit");
	fitCurve.setXDataColumn(&xDataColumn);
	fitCurve.setYDataColumn(&yDataColumn);
	fitCurve.setYErrorColumn(&yErrorColumn);

	//prepare the fit
	XYFitCurve::FitData fitData = fitCurve.fitData();
	fitData.modelCategory = nsl_fit_model_custom;
	XYFitCurve::initFitData(fitData);
	fitData.model = "A*exp(-B/x)";
	fitData.paramNames << "A" << "B";
//	fitData.eps = 1.e-12;
	const int np = fitData.paramNames.size();
	fitData.paramStartValues << 100. << 100.;
	for (int i = 0; i < np; i++) {
		fitData.paramLowerLimits << -std::numeric_limits<double>::max();
		fitData.paramUpperLimits << std::numeric_limits<double>::max();
	}

	fitData.yWeightsType = nsl_fit_weight_instrumental;
	fitCurve.setFitData(fitData);

	//perform the fit
	fitCurve.recalculate();
	const XYFitCurve::FitResult& fitResult = fitCurve.fitResult();

	//check the results
	QCOMPARE(fitResult.available, true);
	QCOMPARE(fitResult.valid, true);

	QCOMPARE(np, 2);

	DEBUG(std::setprecision(15) << fitResult.paramValues.at(0));	// result: 557.463875234563, Win: 557.463888957409
	FuzzyCompare(fitResult.paramValues.at(0), 557.463875234563, 1.e-7);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(0));	// result: 22.0490332546127, FreeBSD: 22.0490429285665
	FuzzyCompare(fitResult.errorValues.at(0), 22.0490332546127, 1.e-6);
	DEBUG(std::setprecision(15) << fitResult.paramValues.at(1));	// result: 468.319724706721, Win: 468.319742522772
	FuzzyCompare(fitResult.paramValues.at(1), 468.319724706721, 1.e-7);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(1));	// result: 26.5105791274139, FreeBSD: 26.510591238283
	FuzzyCompare(fitResult.errorValues.at(1), 26.5105791274139, 1.e-6);

	DEBUG(std::setprecision(15) << fitResult.rms);	// result: 1.96172656494138
	FuzzyCompare(fitResult.rms, 1.96172656494138, 1.e-9);
	DEBUG(std::setprecision(15) << fitResult.rsd);	// result: 1.4006164945985
	FuzzyCompare(fitResult.rsd, 1.4006164945985, 1.e-9);
	DEBUG(std::setprecision(15) << fitResult.sse);	// result: 64.7369766430654
	FuzzyCompare(fitResult.sse, 64.7369766430654, 1.e-9);
	DEBUG(std::setprecision(15) << fitResult.rsquare);	// result: 0.999740417228134
	FuzzyCompare(fitResult.rsquare, 0.999740417228134, 1.e-9);
	DEBUG(std::setprecision(15) << fitResult.rsquareAdj);	// result: 0.999724193304893
	FuzzyCompare(fitResult.rsquareAdj, 0.999724193304893, 1.e-9);
}

QTEST_MAIN(FitTest)
