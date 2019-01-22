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
#include "backend/lib/macros.h"

extern "C" {
#include "backend/nsl/nsl_diff.h"
}

void NSLDiffTest::initTestCase() {
	const QString currentDir = __FILE__;
	m_dataDir = currentDir.left(currentDir.lastIndexOf(QDir::separator())) + QDir::separator() + QLatin1String("data") + QDir::separator();
}

//##############################################################################
//#################  first derivative tests
//##############################################################################

void NSLDiffTest::testFirst_order2() {
	double xdata[] = {1, 2, 4, 8, 16, 32, 64};
	double ydata[] = {1, 4, 16, 64, 256, 1024, 4096};
	const int N = 7;

	int status = nsl_diff_first_deriv(xdata, ydata, N, 2);
	QCOMPARE(status, 0);
	for (unsigned int i = 0; i < N; i++)
		QCOMPARE(ydata[i], 2 * xdata[i]);
}

void NSLDiffTest::testFirst_order4() {
	double xdata[] = {1, 2, 4, 8, 16, 32, 64};
	double ydata[] = {1, 8, 64, 512, 4096, 32768, 262144};
	const int N = 7;

	int status = nsl_diff_first_deriv(xdata, ydata, N, 4);
	QCOMPARE(status, 0);
	for (unsigned int i = 0; i < N; i++)
		QCOMPARE(ydata[i], 3 * xdata[i]*xdata[i]);
}

void NSLDiffTest::testFirst_avg() {
	double xdata[] = {1, 2, 4, 8, 16, 32, 64};
	double ydata[] = {1, 4, 16, 64, 256, 1024, 4096};
	double result[] = {3, 4.5, 9, 18, 36, 72, 96};
	const int N = 7;

	int status = nsl_diff_first_deriv_avg(xdata, ydata, N);
	QCOMPARE(status, 0);
	for (unsigned int i = 0; i < N; i++)
		QCOMPARE(ydata[i], result[i]);
}

//##############################################################################
//#################  performance
//##############################################################################

/*
void NSLDiffTest::testPerformance() {
	double* data = new double[NN];

	for (int i = 0;  i < NN; i++)
		data[i] = 1.;
	
	QBENCHMARK {
		nsl_dft_transform(data, 1, NN, ONESIDED, nsl_dft_result_real);
	}

	delete[] data;
}
*/

QTEST_MAIN(NSLDiffTest)
