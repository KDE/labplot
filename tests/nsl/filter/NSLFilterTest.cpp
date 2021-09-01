/*
    File                 : NSLFilterTest.cpp
    Project              : LabPlot
    Description          : NSL Tests for filter
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2019 Stefan Gerlach <stefan.gerlach@uni.kn>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "NSLFilterTest.h"

extern "C" {
#include "backend/nsl/nsl_filter.h"
}

void NSLFilterTest::initTestCase() {
	const QString currentDir = __FILE__;
	m_dataDir = currentDir.left(currentDir.lastIndexOf(QDir::separator())) + QDir::separator() + QLatin1String("data") + QDir::separator();
}

//##############################################################################
//#################  form test
//##############################################################################


void NSLFilterTest::testForm() {
	const int N = 1000;
	double data[N+2];

	int i;
	for (i = 0; i < N+2; i++)
		data[i] = 1.0;
	
	/*Bessel gain*/
	int n = 3;
	double x = 1.;
	double G = nsl_filter_gain_bessel(n, x);
	printf("G = %.15g\n", G);
	QCOMPARE(G, 0.901262652189164);

	/* filter form */
	nsl_filter_apply(data, N, nsl_filter_type_low_pass, nsl_filter_form_legendre, 2, 50, 2);
	nsl_filter_apply(data, N, nsl_filter_type_high_pass, nsl_filter_form_legendre, 2, 50, 2);
	nsl_filter_apply(data, N, nsl_filter_type_band_pass, nsl_filter_form_legendre, 2, 100, 50);
	nsl_filter_apply(data, N, nsl_filter_type_band_reject, nsl_filter_form_legendre, 2, 100, 100);
	nsl_filter_apply(data, N, nsl_filter_type_low_pass, nsl_filter_form_bessel, 2, 100, 100);
	nsl_filter_apply(data, N, nsl_filter_type_high_pass, nsl_filter_form_bessel, 2, 100, 100);
	nsl_filter_apply(data, N, nsl_filter_type_band_pass, nsl_filter_form_bessel, 2, 100, 100);
	nsl_filter_apply(data, N, nsl_filter_type_band_reject, nsl_filter_form_bessel, 2, 100, 100);

	//for(i=0; i < N/2; i++)
	//	printf("%d %g\n", i, data[2*i]);

	/* all pass order,cut,bw */
	nsl_filter_fourier(data, N, nsl_filter_type_high_pass, nsl_filter_form_ideal, 0, 0, 2);

	/* filter tests */
	nsl_filter_fourier(data, N, nsl_filter_type_low_pass, nsl_filter_form_ideal, 0, 3, 2);
	nsl_filter_fourier(data, N, nsl_filter_type_high_pass, nsl_filter_form_ideal, 0, 3, 2);
	nsl_filter_fourier(data, N, nsl_filter_type_band_pass, nsl_filter_form_ideal, 0, 2, 2);
	nsl_filter_fourier(data, N, nsl_filter_type_band_reject, nsl_filter_form_ideal, 0, 2, 2);
	nsl_filter_fourier(data, N, nsl_filter_type_low_pass, nsl_filter_form_butterworth, 1, 2, 2);
	nsl_filter_fourier(data, N, nsl_filter_type_high_pass, nsl_filter_form_butterworth, 1, 2, 2);
	nsl_filter_fourier(data, N, nsl_filter_type_band_pass, nsl_filter_form_butterworth, 2, 2, 2);
}

//##############################################################################
//#################  performance
//##############################################################################

QTEST_MAIN(NSLFilterTest)
