/***************************************************************************
    File                 : NSLSmoothTest.cpp
    Project              : LabPlot
    Description          : NSL Tests for smoothing
    --------------------------------------------------------------------
    Copyright            : (C) 2019 Stefan Gerlach (stefan.gerlach@uni.kn)
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

#include "NSLSmoothTest.h"

extern "C" {
#include "backend/nsl/nsl_smooth.h"
}

//##############################################################################
//#################  moving average tests
//##############################################################################

const int N = 9;
const int points = 5;
const nsl_smooth_weight_type weight = nsl_smooth_weight_uniform;

void NSLSmoothTest::testMA_padnone() {
	double data[] = {2, 2, 5, 2, 1, 0, 1, 4, 9};
	double result[] = {2, 3, 2.4, 2, 1.8, 1.6, 3, 14/3., 9};

	int status = nsl_smooth_moving_average(data, N, points, weight, nsl_smooth_pad_none);
	QCOMPARE(status, 0);
	for(int i = 0; i < N; i++)
		QCOMPARE(data[i], result[i]);
}

void NSLSmoothTest::testMA_padmirror() {
	double data[] = {2, 2, 5, 2, 1, 0, 1, 4, 9};
	double result[] = {3.2, 2.6, 2.4, 2, 1.8, 1.6, 3, 3.6, 3.8};

	int status = nsl_smooth_moving_average(data, N, points, weight, nsl_smooth_pad_mirror);
	QCOMPARE(status, 0);
	for(int i = 0; i < N; i++)
		QCOMPARE(data[i], result[i]);
}

void NSLSmoothTest::testMA_padnearest() {
	double data[] = {2, 2, 5, 2, 1, 0, 1, 4, 9};
	double result[] = {2.6, 2.6, 2.4, 2, 1.8, 1.6, 3, 4.6, 6.4};

	int status = nsl_smooth_moving_average(data, N, points, weight, nsl_smooth_pad_nearest);
	QCOMPARE(status, 0);
	for(int i = 0; i < N; i++)
		QCOMPARE(data[i], result[i]);
}

void NSLSmoothTest::testMA_padconstant() {
	double data[] = {2, 2, 5, 2, 1, 0, 1, 4, 9};
	double result[] = {1.8, 2.2, 2.4, 2, 1.8, 1.6, 3, 2.8, 2.8};

	int status = nsl_smooth_moving_average(data, N, points, weight, nsl_smooth_pad_constant);
	QCOMPARE(status, 0);
	for(int i = 0; i < N; i++)
		QCOMPARE(data[i], result[i]);
}

void NSLSmoothTest::testMA_padperiodic() {
	double data[] = {2, 2, 5, 2, 1, 0, 1, 4, 9};
	double result[] = {4.4, 4, 2.4, 2, 1.8, 1.6, 3, 3.2, 3.6};

	int status = nsl_smooth_moving_average(data, N, points, weight, nsl_smooth_pad_periodic);
	QCOMPARE(status, 0);
	for(int i = 0; i < N; i++)
		QCOMPARE(data[i], result[i]);
}

//##############################################################################
//#################  lagged moving average tests
//##############################################################################

void NSLSmoothTest::testMAL_padnone() {
	double data[] = {2, 2, 5, 2, 1, 0, 1, 4, 9};
	double result[] = {2, 2, 3, 2.75, 2.4, 2, 1.8, 1.6, 3};

	int status = nsl_smooth_moving_average_lagged(data, N, points, weight, nsl_smooth_pad_none);
	QCOMPARE(status, 0);
	for(int i = 0; i < N; i++)
		QCOMPARE(data[i], result[i]);
}

void NSLSmoothTest::testMAL_padmirror() {
	double data[] = {2, 2, 5, 2, 1, 0, 1, 4, 9};
	double result[] = {2.4, 2.6, 3.2, 2.6, 2.4, 2, 1.8, 1.6, 3};

	int status = nsl_smooth_moving_average_lagged(data, N, points, weight, nsl_smooth_pad_mirror);
	QCOMPARE(status, 0);
	for(int i = 0; i < N; i++)
		QCOMPARE(data[i], result[i]);
}

void NSLSmoothTest::testMAL_padnearest() {
	double data[] = {2, 2, 5, 2, 1, 0, 1, 4, 9};
	double result[] = {0.4, 0.8, 1.8, 2.2, 2.4, 2, 1.8, 1.6, 3};

	int status = nsl_smooth_moving_average_lagged(data, N, points, weight, nsl_smooth_pad_nearest);
	QCOMPARE(status, 0);
	for(int i = 0; i < N; i++)
		QCOMPARE(data[i], result[i]);
}

void NSLSmoothTest::testMAL_padconstant() {
	double data[] = {2, 2, 5, 2, 1, 0, 1, 4, 9};
	double result[] = {0.4, 0.8, 1.8, 2.2, 2.4, 2, 1.8, 1.6, 3};

	int status = nsl_smooth_moving_average_lagged(data, N, points, weight, nsl_smooth_pad_constant);
	QCOMPARE(status, 0);
	for(int i = 0; i < N; i++)
		QCOMPARE(data[i], result[i]);
}

void NSLSmoothTest::testMAL_padperiodic() {
	double data[] = {2, 2, 5, 2, 1, 0, 1, 4, 9};
	double result[] = {3.2, 3.6, 4.4, 4, 2.4, 2, 1.8, 1.6, 3};

	int status = nsl_smooth_moving_average_lagged(data, N, points, weight, nsl_smooth_pad_periodic);
	QCOMPARE(status, 0);
	for(int i = 0; i < N; i++)
		QCOMPARE(data[i], result[i]);
}

//##############################################################################
//#################  percentile tests
//##############################################################################

const double percentile = 0.5;

void NSLSmoothTest::testPercentile_padnone() {
	double data[] = {2, 2, 5, 2, 1, 0, 1, 4, 9};
	double result[] = {2, 2, 2, 1.5, 1, 1, 1, 2.5, 9};

	int status = nsl_smooth_percentile(data, N, points, percentile, nsl_smooth_pad_none);
	QCOMPARE(status, 0);
	for(int i = 0; i < N; i++)
		QCOMPARE(data[i], result[i]);
}

void NSLSmoothTest::testPercentile_padmirror() {
	double data[] = {2, 2, 5, 2, 1, 0, 1, 4, 9};
	double result[] = {2, 2, 2, 1.5, 1, 1, 1, 2.5, 2.5};

	int status = nsl_smooth_percentile(data, N, points, percentile, nsl_smooth_pad_mirror);
	QCOMPARE(status, 0);
	for(int i = 0; i < N; i++)
		QCOMPARE(data[i], result[i]);
}

void NSLSmoothTest::testPercentile_padnearest() {
	double data[] = {2, 2, 5, 2, 1, 0, 1, 4, 9};
	double result[] = {2, 2, 2, 1.5, 1, 1, 1, 2.5, 6.5};

	int status = nsl_smooth_percentile(data, N, points, percentile, nsl_smooth_pad_nearest);
	QCOMPARE(status, 0);
	for(int i = 0; i < N; i++)
		QCOMPARE(data[i], result[i]);
}

void NSLSmoothTest::testPercentile_padconstant() {
	double data[] = {2, 2, 5, 2, 1, 0, 1, 4, 9};
	double result[] = {1, 2, 2, 1.5, 1, 1, 1, 0.5, 0.5};

	int status = nsl_smooth_percentile(data, N, points, percentile, nsl_smooth_pad_constant);
	QCOMPARE(status, 0);
	for(int i = 0; i < N; i++)
		QCOMPARE(data[i], result[i]);
}

void NSLSmoothTest::testPercentile_padperiodic() {
	double data[] = {2, 2, 5, 2, 1, 0, 1, 4, 9};
	double result[] = {3, 2, 2, 1.5, 1, 1, 1, 1.5, 2};

	int status = nsl_smooth_percentile(data, N, points, percentile, nsl_smooth_pad_periodic);
	QCOMPARE(status, 0);
	for(int i = 0; i < N; i++)
		QCOMPARE(data[i], result[i]);
}

//##############################################################################
//#################  Savitzky-Golay coeff tests
//##############################################################################

void NSLSmoothTest::testSG_coeff31() {
	int points = 3, order = 1;
	gsl_matrix *h = gsl_matrix_alloc(points, points);
	double result[] = {1, 1, 1};

	int status = nsl_smooth_savgol_coeff(points, order, h);
	QCOMPARE(status, 0);
	for(int i = 0; i < points; i++)
		QCOMPARE(points * gsl_matrix_get(h,(points-1)/2, i), result[i]);

	gsl_matrix_free(h);
}

void NSLSmoothTest::testSG_coeff51() {
	int points = 5, order = 1;
	gsl_matrix *h = gsl_matrix_alloc(points, points);
	double result[] = {1, 1, 1, 1, 1};

	int status = nsl_smooth_savgol_coeff(points, order, h);
	QCOMPARE(status, 0);
	for(int i = 0; i < points; i++)
		QCOMPARE(points * gsl_matrix_get(h,(points-1)/2, i), result[i]);

	gsl_matrix_free(h);
}

void NSLSmoothTest::testSG_coeff53() {
	int points = 5, order = 3;
	gsl_matrix *h = gsl_matrix_alloc(points, points);
	double result[] = {-3, 12, 17, 12, -3};

	int status = nsl_smooth_savgol_coeff(points, order, h);
	QCOMPARE(status, 0);
	for(int i = 0; i < points; i++)
		QCOMPARE(35 * gsl_matrix_get(h,(points-1)/2, i), result[i]);

	gsl_matrix_free(h);
}

void NSLSmoothTest::testSG_coeff73() {
	int points = 7, order = 3;
	gsl_matrix *h = gsl_matrix_alloc(points, points);
	double result[] = {-2, 3, 6, 7, 6, 3, -2};

	int status = nsl_smooth_savgol_coeff(points, order, h);
	QCOMPARE(status, 0);
	for(int i = 0; i < points; i++)
		QCOMPARE(21 * gsl_matrix_get(h,(points-1)/2, i), result[i]);

	gsl_matrix_free(h);
}

void NSLSmoothTest::testSG_coeff74() {
	int points = 7, order = 4;
	gsl_matrix *h = gsl_matrix_alloc(points, points);
	double result[] = {5, -30, 75, 131, 75, -30, 5};

	int status = nsl_smooth_savgol_coeff(points, order, h);
	QCOMPARE(status, 0);

	// see AnalysisTests.h
	//FuzzyCompare(231 * gsl_matrix_get(h,(points-1)/2, 0), 5., 1.e-8);
	for(int i = 1; i < points; i++) {
		DEBUG(std::setprecision(15) << 231 * gsl_matrix_get(h,(points-1)/2, i));
//TODO		QCOMPARE(231 * gsl_matrix_get(h,(points-1)/2, i), result[i]);
	}

	gsl_matrix_free(h);
}


//##############################################################################
//#################  performance
//##############################################################################

//TODO

QTEST_MAIN(NSLSmoothTest)
