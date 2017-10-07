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
	const double rsquare = nsl_stats_rsquare(fitResult.sse,fitResult.sst);
	QCOMPARE(rsquare, 0.224668921574940E-02);
	QCOMPARE(fitResult.sse, 0.835542680000000E+16);
	QCOMPARE(fitResult.rms, 557028453333333.);
}

QTEST_MAIN(FitTest)
