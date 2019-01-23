/***************************************************************************
    File                 : NSLIntTest.cpp
    Project              : LabPlot
    Description          : NSL Tests for numerical integration
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

#include "NSLIntTest.h"
#include "backend/lib/macros.h"

extern "C" {
#include "backend/nsl/nsl_int.h"
}

void NSLIntTest::initTestCase() {
	const QString currentDir = __FILE__;
	m_dataDir = currentDir.left(currentDir.lastIndexOf(QDir::separator())) + QDir::separator() + QLatin1String("data") + QDir::separator();
}

//##############################################################################
//#################  rule integral/area tests
//##############################################################################

const int N = 5;
double xdata[] = {1, 2, 3, 5, 7};

void NSLIntTest::testRectangle_integral() {
	double ydata[] = {2, 2, 2, -2, -2};

	int status = nsl_int_rectangle(xdata, ydata, N, 0);
	QCOMPARE(status, 0);
	QCOMPARE(ydata[N - 1], 4.);
}

void NSLIntTest::testRectangle_area() {
	double ydata[] = {2, 2, 2, -2, -2};

	int status = nsl_int_rectangle(xdata, ydata, N, 1);
	QCOMPARE(status, 0);
	QCOMPARE(ydata[N - 1], 12.);
}

void NSLIntTest::testTrapezoid_integral() {
	double ydata[] = {1, 2, 3, -1, -3};

	int status = nsl_int_trapezoid(xdata, ydata, N, 0);
	QCOMPARE(status, 0);
	QCOMPARE(ydata[N - 1], 2.);
}

void NSLIntTest::testTrapezoid_area() {
	double ydata[] = {1, 2, 3, -1, -3};

	int status = nsl_int_trapezoid(xdata, ydata, N, 1);
	QCOMPARE(status, 0);
	QCOMPARE(ydata[N - 1], 10.5);
}

void NSLIntTest::test3Point_integral() {
	double ydata[] = {1, 2, 3, -1, -3};

	int np = (int)nsl_int_simpson(xdata, ydata, N, 0);
	QCOMPARE(np, 3);
	QCOMPARE(ydata[np - 1], 4/3.);
}

void NSLIntTest::test4Point_integral() {
	double xdata2[]={1, 2, 3, 5, 7, 8, 9};
	double ydata[] = {2, 2, 2, 2, 2, 2, 2, 2};
	const int n = 7;

	int np = (int)nsl_int_simpson_3_8(xdata2, ydata, n, 0);
	QCOMPARE(np, 3);
	QCOMPARE(ydata[np - 1], 16.);
}

//##############################################################################
//#################  performance
//##############################################################################

void NSLIntTest::testPerformanceRectangle() {
	const size_t n = 1e6;
	double* xdata = new double[n];
	double* ydata = new double[n];

	for (size_t i = 0;  i < n; i++)
		xdata[i] = (double)i;

	QBENCHMARK {
		for (size_t i = 0;  i < n; i++)
			ydata[i] = 1.;
		int status = nsl_int_rectangle(xdata, ydata, n, 0);
		QCOMPARE(status, 0);
	}
	QCOMPARE(ydata[n - 1], (double)(n - 1));

	delete[] xdata;
	delete[] ydata;
}

void NSLIntTest::testPerformanceTrapezoid() {
	const int n = 1e6;
	double* xdata = new double[n];
	double* ydata = new double[n];

	for (int i = 0;  i < n; i++)
		xdata[i] = (double)i;

	QBENCHMARK {
		for (int i = 0;  i < n; i++)
			ydata[i] = 1.;
		int status = nsl_int_trapezoid(xdata, ydata, n, 0);
		QCOMPARE(status, 0);
	}
	QCOMPARE(ydata[n - 1], (double)(n - 1));

	delete[] xdata;
	delete[] ydata;
}

void NSLIntTest::testPerformance3Point() {
	const int n = 1e6;
	double* xdata = new double[n];
	double* ydata = new double[n];

	for (int i = 0;  i < n; i++)
		xdata[i] = (double)i;

	int np;
	QBENCHMARK {
		for (int i = 0;  i < n; i++)
			ydata[i] = 1.;
		np = (int)nsl_int_simpson(xdata, ydata, n, 0);
		QCOMPARE(np, n/2 + 1);
	}
	QCOMPARE(ydata[np - 1], (double)(n - 1));

	delete[] xdata;
	delete[] ydata;
}

void NSLIntTest::testPerformance4Point() {
	const int n = 1e6;
	double* xdata = new double[n];
	double* ydata = new double[n];

	for (int i = 0;  i < n; i++)
		xdata[i] = (double)i;

	int np;
	QBENCHMARK {
		for (int i = 0;  i < n; i++)
			ydata[i] = 1.;
		np = (int)nsl_int_simpson_3_8(xdata, ydata, n, 0);
		QCOMPARE(np, n/3 + 1);
	}

	//TODO:
	//QCOMPARE(ydata[np - 1], (double)(n - 1));
	printf("%.15g %.15g\n", ydata[np - 1], (double)(n-1));

	delete[] xdata;
	delete[] ydata;
}

QTEST_MAIN(NSLIntTest)
