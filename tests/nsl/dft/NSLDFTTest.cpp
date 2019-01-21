/***************************************************************************
    File                 : NSLDFTTest.cpp
    Project              : LabPlot
    Description          : NSL Tests for DFT
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

#include "NSLDFTTest.h"

extern "C" {
#include "backend/nsl/nsl_dft.h"
}

void NSLDFTTest::initTestCase() {
	const QString currentDir = __FILE__;
	m_dataDir = currentDir.left(currentDir.lastIndexOf(QDir::separator())) + QDir::separator() + QLatin1String("data") + QDir::separator();
}

#define ONESIDED 0
#define TWOSIDED 1
#define N 10

//##############################################################################
//#################  one sided tests
//##############################################################################

void NSLDFTTest::test_onesided_real() {
	double data[] = {1, 1, 3, 3, 1, -1, 0, 1, 1, 0};
	double result[] = {10, 2, -5.85410196624968, 2, 0.854101966249685};

	nsl_dft_transform(data, 1, N, ONESIDED, nsl_dft_result_real);
	for (unsigned int i = 0; i < N/2; i++)
		QCOMPARE(data[i], result[i]);
}

void NSLDFTTest::test_onesided_imag() {
	double data[] = {1, 1, 3, 3, 1, -1, 0, 1, 1, 0};
	double result[] = {0, -4.97979656976556, 0, 0.449027976579585, 0};

	nsl_dft_transform(data, 1, N, ONESIDED, nsl_dft_result_imag);
	for (unsigned int i = 0; i < N/2; i++)
		QCOMPARE(data[i], result[i]);
}

void NSLDFTTest::test_onesided_magnitude() {
	double data[] = {1, 1, 3, 3, 1, -1, 0, 1, 1, 0};
	double result[] = {10, 5.36641163872553, 5.85410196624968, 2.04978684837013, 0.854101966249685};

	nsl_dft_transform(data, 1, N, ONESIDED, nsl_dft_result_magnitude);
	for (unsigned int i = 0; i < N/2; i++)
		QCOMPARE(data[i], result[i]);
}

void NSLDFTTest::test_onesided_amplitude() {
	double data[] = {1, 1, 3, 3, 1, -1, 0, 1, 1, 0};
	double result[] = {1, 1.07328232774511, 1.17082039324994, 0.409957369674026, 0.170820393249937};

	nsl_dft_transform(data, 1, N, ONESIDED, nsl_dft_result_amplitude);
	for (unsigned int i = 0; i < N/2; i++)
		QCOMPARE(data[i], result[i]);
}

void NSLDFTTest::test_onesided_power() {
	double data[] = {1, 1, 3, 3, 1, -1, 0, 1, 1, 0};
	double result[] = {10, 5.75967477524977, 6.85410196624968, 0.840325224750231, 0.145898033750316};

	nsl_dft_transform(data, 1, N, ONESIDED, nsl_dft_result_power);
	for (unsigned int i = 0; i < N/2; i++)
		QCOMPARE(data[i], result[i]);
}

void NSLDFTTest::test_onesided_phase() {
	double data[] = {1, 1, 3, 3, 1, -1, 0, 1, 1, 0};
	double result[] = {0, 1.18889174012102, -3.14159265358979, -0.220851801285041, 0};

	nsl_dft_transform(data, 1, N, ONESIDED, nsl_dft_result_phase);
	for (unsigned int i = 0; i < N/2; i++)
		QCOMPARE(data[i], result[i]);
}

void NSLDFTTest::test_onesided_dB() {
	double data[] = {1, 1, 3, 3, 1, -1, 0, 1, 1, 0};
	double result[] = {0, 0.614279569861449, 1.36980556663896, -7.7452260401377, -15.3492056533593};

	nsl_dft_transform(data, 1, N, ONESIDED, nsl_dft_result_dB);
	for (unsigned int i = 0; i < N/2; i++)
		QCOMPARE(data[i], result[i]);
}

void NSLDFTTest::test_onesided_squaremagnitude() {
	double data[] = {1, 1, 3, 3, 1, -1, 0, 1, 1, 0};
	double result[] = {100, 28.7983738762488, 34.2705098312484, 4.20162612375116, 0.729490168751578};

	nsl_dft_transform(data, 1, N, ONESIDED, nsl_dft_result_squaremagnitude);
	for (unsigned int i = 0; i < N/2; i++)
		QCOMPARE(data[i], result[i]);
}

void NSLDFTTest::test_onesided_squareamplitude() {
	double data[] = {1, 1, 3, 3, 1, -1, 0, 1, 1, 0};
	double result[] = {1, 1.15193495504995, 1.37082039324994, 0.168065044950046, 0.0291796067500631};

	nsl_dft_transform(data, 1, N, ONESIDED, nsl_dft_result_squareamplitude);
	for (unsigned int i = 0; i < N/2; i++)
		QCOMPARE(data[i], result[i]);
}

void NSLDFTTest::test_onesided_normdB() {
	double data[] = {1, 1, 3, 3, 1, -1, 0, 1, 1, 0};
	double result[] = {-1.36980556663896, -0.755525996777514, 0, -9.11503160677666, -16.7190112199983};

	nsl_dft_transform(data, 1, N, ONESIDED, nsl_dft_result_normdB);
	for (unsigned int i = 0; i < N/2; i++)
		QCOMPARE(data[i], result[i]);
}

//##############################################################################
//#################  two sided tests
//##############################################################################

void NSLDFTTest::test_twosided_real() {
	double data[] = {1, 1, 3, 3, 1, -1, 0, 1, 1, 0};
	double result[] = {10, 2, -5.85410196624968, 2, 0.854101966249685, 2, 0.854101966249685, 2, -5.85410196624968, 2};

	nsl_dft_transform(data, 1, N, TWOSIDED, nsl_dft_result_real);
	for (unsigned int i = 0; i < N; i++)
		QCOMPARE(data[i], result[i]);
}

void NSLDFTTest::test_twosided_imag() {
	double data[] = {1, 1, 3, 3, 1, -1, 0, 1, 1, 0};
	double result[] = {0, -4.97979656976556, 0, 0.449027976579585, 0, 0, 0, -0.449027976579585, 0, 4.97979656976556};

	nsl_dft_transform(data, 1, N, TWOSIDED, nsl_dft_result_imag);
	for (unsigned int i = 0; i < N; i++)
		QCOMPARE(data[i], result[i]);
}

void NSLDFTTest::test_twosided_magnitude() {
	double data[] = {1, 1, 3, 3, 1, -1, 0, 1, 1, 0};
	double result[] = {10, 5.36641163872553, 5.85410196624968, 2.04978684837013, 0.854101966249685, 2, 0.854101966249685, 2.04978684837013, 5.85410196624968, 5.36641163872553};

	nsl_dft_transform(data, 1, N, TWOSIDED, nsl_dft_result_magnitude);
	for (unsigned int i = 0; i < N; i++)
		QCOMPARE(data[i], result[i]);
}

void NSLDFTTest::test_twosided_amplitude() {
	double data[] = {1, 1, 3, 3, 1, -1, 0, 1, 1, 0};
	double result[] = {1, 1.07328232774511, 1.17082039324994, 0.409957369674026, 0.170820393249937, 0.4, 0.170820393249937, 0.409957369674026, 1.17082039324994, 1.07328232774511};

	nsl_dft_transform(data, 1, N, TWOSIDED, nsl_dft_result_amplitude);
	for (unsigned int i = 0; i < N; i++)
		QCOMPARE(data[i], result[i]);
}

void NSLDFTTest::test_twosided_power() {
	double data[] = {1, 1, 3, 3, 1, -1, 0, 1, 1, 0};
	double result[] = {10, 5.75967477524977, 6.85410196624968, 0.840325224750231, 0.145898033750316, 0.8, 0.145898033750316, 0.840325224750231, 6.85410196624968, 5.75967477524977};

	nsl_dft_transform(data, 1, N, TWOSIDED, nsl_dft_result_power);
	for (unsigned int i = 0; i < N; i++)
		QCOMPARE(data[i], result[i]);
}

void NSLDFTTest::test_twosided_phase() {
	double data[] = {1, 1, 3, 3, 1, -1, 0, 1, 1, 0};
	double result[] = {0, 1.18889174012102, -3.14159265358979, -0.220851801285041, 0, 0, 0, 0.220851801285041, 3.14159265358979, -1.18889174012102};

	nsl_dft_transform(data, 1, N, TWOSIDED, nsl_dft_result_phase);
	for (unsigned int i = 0; i < N; i++)
		QCOMPARE(data[i], result[i]);
}

void NSLDFTTest::test_twosided_dB() {
	double data[] = {1, 1, 3, 3, 1, -1, 0, 1, 1, 0};
	double result[] = {0, 0.614279569861449, 1.36980556663896, -7.7452260401377, -15.3492056533593, -7.95880017344075, -15.3492056533593, -7.7452260401377, 1.36980556663896, 0.614279569861449};

	nsl_dft_transform(data, 1, N, TWOSIDED, nsl_dft_result_dB);
	for (unsigned int i = 0; i < N; i++)
		QCOMPARE(data[i], result[i]);
}

void NSLDFTTest::test_twosided_squaremagnitude() {
	double data[] = {1, 1, 3, 3, 1, -1, 0, 1, 1, 0};
	double result[] = {100, 28.7983738762488, 34.2705098312484, 4.20162612375116, 0.729490168751578, 4, 0.729490168751578, 4.20162612375116, 34.2705098312484, 28.7983738762488};

	nsl_dft_transform(data, 1, N, TWOSIDED, nsl_dft_result_squaremagnitude);
	for (unsigned int i = 0; i < N; i++)
		QCOMPARE(data[i], result[i]);
}

void NSLDFTTest::test_twosided_squareamplitude() {
	double data[] = {1, 1, 3, 3, 1, -1, 0, 1, 1, 0};
	double result[] = {1, 1.15193495504995, 1.37082039324994, 0.168065044950046, 0.0291796067500631, 0.16, 0.0291796067500631, 0.168065044950046, 1.37082039324994, 1.15193495504995};

	nsl_dft_transform(data, 1, N, TWOSIDED, nsl_dft_result_squareamplitude);
	for (unsigned int i = 0; i < N; i++)
		QCOMPARE(data[i], result[i]);
}

void NSLDFTTest::test_twosided_normdB() {
	double data[] = {1, 1, 3, 3, 1, -1, 0, 1, 1, 0};
	double result[] = {-1.36980556663896, -0.755525996777514, 0, -9.11503160677666, -16.7190112199983, -9.32860574007971, -16.7190112199983, -9.11503160677666, 0, -0.755525996777514};

	nsl_dft_transform(data, 1, N, TWOSIDED, nsl_dft_result_normdB);
	for (unsigned int i = 0; i < N; i++)
		QCOMPARE(data[i], result[i]);
}

//##############################################################################
//#################  performance
//##############################################################################

/*	QBENCHMARK {
		for (unsigned int i = 1; i < 1e7; i++)
			Q_UNUSED((int)log2(i));
	}
*/

QTEST_MAIN(NSLDFTTest)
