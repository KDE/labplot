/***************************************************************************
    File                 : NSLSFBasicTest.cpp
    Project              : LabPlot
    Description          : NSL Tests for basic special functions
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

#include "NSLSFBasicTest.h"

extern "C" {
#include "backend/nsl/nsl_sf_basic.h"
}

void NSLSFBasicTest::initTestCase() {
	const QString currentDir = __FILE__;
	m_dataDir = currentDir.left(currentDir.lastIndexOf(QDir::separator())) + QDir::separator() + QLatin1String("data") + QDir::separator();
}

//##############################################################################
//#################  handling of empty and sparse files ########################
//##############################################################################
void NSLSFBasicTest::testlog2_int_C99() {
	QBENCHMARK {
		for (unsigned int i = 1; i < 1e7; i++)
			Q_UNUSED((int)log2(i));
	}
}

void NSLSFBasicTest::testlog2_int() {
	for (unsigned int i = 1; i < 1e6; i++) {
		int result = nsl_sf_log2_int(i);
		QCOMPARE(result, (int)log2(i));
	}

	QBENCHMARK {
		for (unsigned int i = 1; i < 1e7; i++)
			nsl_sf_log2_int(i);
	}
}
void NSLSFBasicTest::testlog2_longlong() {
	for (unsigned long long i = 1; i < 1e6; i++) {
		int result = nsl_sf_log2_longlong(i);
		QCOMPARE(result, (int)log2(i));
	}

	QBENCHMARK {
		for (unsigned long long i = 1; i < 1e7; i++)
			nsl_sf_log2_int(i);
	}
}


void NSLSFBasicTest::testlog2_int2() {
	for (int i = 1; i < 1e6; i++) {
		int result = nsl_sf_log2_int2(i);
		QCOMPARE(result, (int)log2(i));
	}

	QBENCHMARK {
		for (int i = 1; i < 1e7; i++)
			nsl_sf_log2_int2(i);
	}
}

void NSLSFBasicTest::testlog2_int3() {
	for (unsigned int i = 1; i < 1e6; i++) {
		int result = nsl_sf_log2_int3(i);
		QCOMPARE(result, (int)log2(i));
	}

	QBENCHMARK {
		for (unsigned int i = 1; i < 1e7; i++)
			nsl_sf_log2_int3(i);
	}
}

void NSLSFBasicTest::testlog2p1_int() {
	for (int i = 1; i < 1e6; i++) {
		int result = nsl_sf_log2p1_int(i);
		QCOMPARE(result, (int)log2(i) + 1);
	}

	QBENCHMARK {
		for (int i = 1; i < 1e7; i++)
			nsl_sf_log2p1_int(i);
	}

}

QTEST_MAIN(NSLSFBasicTest)
