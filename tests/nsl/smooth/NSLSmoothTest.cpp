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
#include "backend/lib/macros.h"

extern "C" {
#include "backend/nsl/nsl_smooth.h"
}

void NSLSmoothTest::initTestCase() {
	const QString currentDir = __FILE__;
	m_dataDir = currentDir.left(currentDir.lastIndexOf(QDir::separator())) + QDir::separator() + QLatin1String("data") + QDir::separator();
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
//#################  performance
//##############################################################################

//TODO

QTEST_MAIN(NSLSmoothTest)
