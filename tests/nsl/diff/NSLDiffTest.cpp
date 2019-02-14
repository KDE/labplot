/***************************************************************************
    File                 : NSLDiffTest.cpp
    Project              : LabPlot
    Description          : NSL Tests for numerical differentiation
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

#include "NSLDiffTest.h"

extern "C" {
#include "backend/nsl/nsl_diff.h"
}

//##############################################################################
//#################  first derivative tests
//##############################################################################

const int N = 7;
double xdata[] = {1, 2, 4, 8, 16, 32, 64};

void NSLDiffTest::testFirst_order2() {
	double ydata[] = {1, 4, 16, 64, 256, 1024, 4096};

	int status = nsl_diff_first_deriv(xdata, ydata, N, 2);
	QCOMPARE(status, 0);
	for (unsigned int i = 0; i < N; i++)
		QCOMPARE(ydata[i], 2 * xdata[i]);
}

void NSLDiffTest::testFirst_order4() {
	double ydata[] = {1, 8, 64, 512, 4096, 32768, 262144};

	int status = nsl_diff_first_deriv(xdata, ydata, N, 4);
	QCOMPARE(status, 0);
	for (unsigned int i = 0; i < N; i++)
		QCOMPARE(ydata[i], 3 * xdata[i]*xdata[i]);
}

void NSLDiffTest::testFirst_avg() {
	double ydata[] = {1, 4, 16, 64, 256, 1024, 4096};
	double result[] = {3, 4.5, 9, 18, 36, 72, 96};

	int status = nsl_diff_first_deriv_avg(xdata, ydata, N);
	QCOMPARE(status, 0);
	for (unsigned int i = 0; i < N; i++)
		QCOMPARE(ydata[i], result[i]);
}

//##############################################################################
//#################  second derivative tests
//##############################################################################

void NSLDiffTest::testSecond_order1() {
	double ydata[] = {1, 4, 16, 64, 256, 1024, 4096};

	int status = nsl_diff_second_deriv(xdata, ydata, N, 1);
	QCOMPARE(status, 0);
	for (unsigned int i = 0; i < N; i++)
		QCOMPARE(ydata[i], 2.);
}

void NSLDiffTest::testSecond_order2() {
	double ydata[] = {1, 4, 16, 64, 256, 1024, 4096};

	int status = nsl_diff_second_deriv(xdata, ydata, N, 2);
	QCOMPARE(status, 0);
	for (unsigned int i = 0; i < N; i++)
		QCOMPARE(ydata[i], 2.);
}

void NSLDiffTest::testSecond_order3() {
	double ydata[] = {1, 8, 64, 512, 4096, 32768, 262144};

	int status = nsl_diff_second_deriv(xdata, ydata, N, 3);
	QCOMPARE(status, 0);
	for (unsigned int i = 0; i < N; i++)
		QCOMPARE(ydata[i], 6. * xdata[i]);
}

//##############################################################################
//#################  higher derivative tests
//##############################################################################

void NSLDiffTest::testThird_order2() {
	double ydata[] = {1, 8, 64, 512, 4096, 32768, 262144};

	int status = nsl_diff_third_deriv(xdata, ydata, N, 2);
	QCOMPARE(status, 0);
	for (unsigned int i = 0; i < N; i++)
		QCOMPARE(ydata[i], 6.);
}

void NSLDiffTest::testFourth_order1() {
	double ydata[] = {1, 8, 64, 512, 4096, 32768, 262144};

	int status = nsl_diff_fourth_deriv(xdata, ydata, N, 1);
	QCOMPARE(status, 0);
	for (unsigned int i = 0; i < N; i++)
		QCOMPARE(ydata[i], 0.);
}

void NSLDiffTest::testFourth_order3() {
	double ydata[] = {1, 8, 64, 512, 4096, 32768, 262144};

	int status = nsl_diff_fourth_deriv(xdata, ydata, N, 3);
	QCOMPARE(status, 0);
	for (unsigned int i = 0; i < N; i++)
		QCOMPARE(ydata[i] + 1., 1.);
}

void NSLDiffTest::testFifth_order2() {
	double ydata[] = {1, 8, 64, 512, 4096, 32768, 262144};

	int status = nsl_diff_fifth_deriv(xdata, ydata, N, 2);
	QCOMPARE(status, 0);
	for (unsigned int i = 0; i < N; i++)
		QCOMPARE(ydata[i] + 1., 1.);
}

void NSLDiffTest::testSixth_order1() {
	double ydata[] = {1, 8, 64, 512, 4096, 32768, 262144};

	int status = nsl_diff_sixth_deriv(xdata, ydata, N, 1);
	QCOMPARE(status, 0);
	for (unsigned int i = 0; i < N; i++)
		QCOMPARE(ydata[i] + 1., 1.);
}

//##############################################################################
//#################  performance
//##############################################################################

void NSLDiffTest::testPerformance_first() {
	const int NN = 1e6;
	double* xdata = new double[NN];
	double* ydata = new double[NN];

	for (int i = 0;  i < NN; i++)
		xdata[i] = ydata[i] = (double)i;

	QBENCHMARK {
		int status = nsl_diff_first_deriv(xdata, ydata, NN, 2);
		QCOMPARE(status, 0);
	}

	delete[] xdata;
	delete[] ydata;
}

void NSLDiffTest::testPerformance_second() {
	const int NN = 1e6;
	double* xdata = new double[NN];
	double* ydata = new double[NN];

	for (int i = 0;  i < NN; i++)
		xdata[i] = ydata[i] = (double)i;

	QBENCHMARK {
		int status = nsl_diff_second_deriv(xdata, ydata, NN, 2);
		QCOMPARE(status, 0);
	}

	delete[] xdata;
	delete[] ydata;
}

void NSLDiffTest::testPerformance_third() {
	const int NN = 1e6;
	double* xdata = new double[NN];
	double* ydata = new double[NN];

	for (int i = 0;  i < NN; i++)
		xdata[i] = ydata[i] = (double)i;
	
	QBENCHMARK {
		int status = nsl_diff_third_deriv(xdata, ydata, NN, 2);
		QCOMPARE(status, 0);
	}

	delete[] xdata;
	delete[] ydata;
}

QTEST_MAIN(NSLDiffTest)
