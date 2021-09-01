/*
    File                 : NSLSmoothTest.cpp
    Project              : LabPlot
    Description          : NSL Tests for smoothing
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2019 Stefan Gerlach (stefan.gerlach@uni.kn)
*/

/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
	double result[] = {2., 2., 2.6, 2.6, 2.4, 2, 1.8, 1.6, 3};

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
	double result[] = {2, 2, 2, 2, 1, 1, 1, 4, 9};

	int status = nsl_smooth_percentile(data, N, points, percentile, nsl_smooth_pad_none);
	QCOMPARE(status, 0);
	for(int i = 0; i < N; i++)
		QCOMPARE(data[i], result[i]);
}

void NSLSmoothTest::testPercentile_padmirror() {
	double data[] = {2, 2, 5, 2, 1, 0, 1, 4, 9};
	double result[] = {2, 2, 2, 2, 1, 1, 1, 4, 4};

	int status = nsl_smooth_percentile(data, N, points, percentile, nsl_smooth_pad_mirror);
	QCOMPARE(status, 0);
	for(int i = 0; i < N; i++)
		QCOMPARE(data[i], result[i]);
}

void NSLSmoothTest::testPercentile_padnearest() {
	double data[] = {2, 2, 5, 2, 1, 0, 1, 4, 9};
	double result[] = {2, 2, 2, 2, 1, 1, 1, 4, 9};

	int status = nsl_smooth_percentile(data, N, points, percentile, nsl_smooth_pad_nearest);
	QCOMPARE(status, 0);
	for(int i = 0; i < N; i++)
		QCOMPARE(data[i], result[i]);
}

void NSLSmoothTest::testPercentile_padconstant() {
	double data[] = {2, 2, 5, 2, 1, 0, 1, 4, 9};
	double result[] = {2, 2, 2, 2, 1, 1, 1, 1, 1};

	int status = nsl_smooth_percentile(data, N, points, percentile, nsl_smooth_pad_constant);
	QCOMPARE(status, 0);
	for(int i = 0; i < N; i++)
		QCOMPARE(data[i], result[i]);
}

void NSLSmoothTest::testPercentile_padperiodic() {
	double data[] = {2, 2, 5, 2, 1, 0, 1, 4, 9};
	double result[] = {4, 2, 2, 2, 1, 1, 1, 2, 2};

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

	for(int i = 0; i < points; i++) {
		FuzzyCompare(result[i], 231 * gsl_matrix_get(h,(points-1)/2, i), 1.e-10);
		//QCOMPARE(231 * gsl_matrix_get(h,(points-1)/2, i), result[i]);
	}

	gsl_matrix_free(h);
}

void NSLSmoothTest::testSG_coeff92() {
	int points = 9, order = 2;
	gsl_matrix *h = gsl_matrix_alloc(points, points);
	double result[] = {-21, 14, 39, 54, 59, 54, 39, 14, -21};

	int status = nsl_smooth_savgol_coeff(points, order, h);
	QCOMPARE(status, 0);

	for(int i = 0; i < points; i++) {
		//FuzzyCompare(result[i], 231 * gsl_matrix_get(h,(points-1)/2, i), 1.e-10);
		QCOMPARE(231 * gsl_matrix_get(h,(points-1)/2, i), result[i]);
	}

	gsl_matrix_free(h);
}

void NSLSmoothTest::testSG_coeff94() {
	int points = 9, order = 4;
	gsl_matrix *h = gsl_matrix_alloc(points, points);
	double result[] = {15, -55, 30, 135, 179, 135, 30, -55, 15};

	int status = nsl_smooth_savgol_coeff(points, order, h);
	QCOMPARE(status, 0);

	for(int i = 0; i < points; i++) {
		FuzzyCompare(result[i], 429 * gsl_matrix_get(h,(points-1)/2, i), 1.e-10);
		//QCOMPARE(429 * gsl_matrix_get(h,(points-1)/2, i), result[i]);
	}

	gsl_matrix_free(h);
}

//##############################################################################
//#################  Savitzky-Golay modes
//##############################################################################

const int n = 9, m = 5, order = 2;

void NSLSmoothTest::testSG_mode_interp() {
	double data[] = {2, 2, 5, 2, 1, 0, 1, 4, 9};
	double result[] = {1.65714285714286, 3.17142857142857, 3.54285714285714, 2.85714285714285, 0.65714285714287, 0.17142857142858, 1., 4., 9.};

	int status = nsl_smooth_savgol(data, n, m, order, nsl_smooth_pad_interp);
	QCOMPARE(status, 0);

	for(int i = 0; i < n; i++)
		QCOMPARE(data[i], result[i]);
		//FuzzyCompare(data[i], result[i], 1.e-11);
}

void NSLSmoothTest::testSG_mode_mirror() {
	double data[] = {2, 2, 5, 2, 1, 0, 1, 4, 9};
	double result[] = {1.48571428571430, 3.02857142857143, 3.542857142857, 2.857142857143, 0.657142857143, 0.171428571429, 1., 5.02857142857142, 6.94285714285713};

	int status = nsl_smooth_savgol(data, n, m, order, nsl_smooth_pad_mirror);
	QCOMPARE(status, 0);

	for(int i = 0; i < n; i++)
		FuzzyCompare(data[i], result[i], 1.e-11);
		//QCOMPARE(data[i], result[i]);
}

void NSLSmoothTest::testSG_mode_nearest() {
	double data[] = {2, 2, 5, 2, 1, 0, 1, 4, 9};
	double result[] = {1.74285714285715, 3.02857142857143, 3.542857142857, 2.857142857143, 0.657142857143, 0.171428571429, 1., 4.6, 7.97142857142856};

	int status = nsl_smooth_savgol(data, n, m, order, nsl_smooth_pad_nearest);
	QCOMPARE(status, 0);

	for(int i = 0; i < n; i++)
		FuzzyCompare(data[i], result[i], 1.e-11);
		//QCOMPARE(data[i], result[i]);
}

void NSLSmoothTest::testSG_mode_constant() {
	double data[] = {2, 2, 5, 2, 1, 0, 1, 4, 9};
	double result[] = {1.22857142857143, 3.2, 3.542857142857, 2.857142857143, 0.657142857143, 0.171428571429, 1., 5.37142857142855, 5.65714285714284};

	int status = nsl_smooth_savgol(data, n, m, order, nsl_smooth_pad_constant);
	QCOMPARE(status, 0);

	for(int i = 0; i < n; i++)
		FuzzyCompare(data[i], result[i], 1.e-11);
		//QCOMPARE(data[i], result[i]);
}

void NSLSmoothTest::testSG_mode_periodic() {
	double data[] = {2, 2, 5, 2, 1, 0, 1, 4, 9};
	double result[] = {3.97142857142858, 2.42857142857144, 3.542857142857, 2.857142857143, 0.657142857143, 0.171428571429, 1., 5.2, 6.17142857142856};

	int status = nsl_smooth_savgol(data, n, m, order, nsl_smooth_pad_periodic);
	QCOMPARE(status, 0);

	for(int i = 0; i < n; i++)
		FuzzyCompare(data[i], result[i], 1.e-11);
		//QCOMPARE(data[i], result[i]);
}

//##############################################################################
//#################  performance
//##############################################################################

const int nn = 1e6;

void NSLSmoothTest::testPerformance_interp() {
	QScopedArrayPointer<double> data(new double[nn]);

	QBENCHMARK {
		for (int i = 0;  i < nn; i++)
			data[i] = i;
		int status = nsl_smooth_savgol(data.data(), nn, m, order, nsl_smooth_pad_interp);
		QCOMPARE(status, 0);
	}
}

void NSLSmoothTest::testPerformance_mirror() {
	QScopedArrayPointer<double> data(new double[nn]);

	QBENCHMARK {
		for (int i = 0;  i < nn; i++)
			data[i] = i;
		int status = nsl_smooth_savgol(data.data(), nn, m, order, nsl_smooth_pad_mirror);
		QCOMPARE(status, 0);
	}
}

void NSLSmoothTest::testPerformance_nearest() {
	QScopedArrayPointer<double> data(new double[nn]);

	QBENCHMARK {
		for (int i = 0;  i < nn; i++)
			data[i] = i;
		int status = nsl_smooth_savgol(data.data(), nn, m, order, nsl_smooth_pad_nearest);
		QCOMPARE(status, 0);
	}
}

void NSLSmoothTest::testPerformance_constant() {
	QScopedArrayPointer<double> data(new double[nn]);

	QBENCHMARK {
		for (int i = 0;  i < nn; i++)
			data[i] = i;
		int status = nsl_smooth_savgol(data.data(), nn, m, order, nsl_smooth_pad_constant);
		QCOMPARE(status, 0);
	}
}

void NSLSmoothTest::testPerformance_periodic() {
	QScopedArrayPointer<double> data(new double[nn]);

	QBENCHMARK {
		for (int i = 0;  i < nn; i++)
			data[i] = i;
		int status = nsl_smooth_savgol(data.data(), nn, m, order, nsl_smooth_pad_periodic);
		QCOMPARE(status, 0);
	}
}

QTEST_MAIN(NSLSmoothTest)
