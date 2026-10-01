/*
	File                 : NSLKDETest.cpp
	Project              : LabPlot
	Description          : Tests for NSL kernel density estimation
	--------------------------------------------------------------------
        SPDX-FileCopyrightText: 2026 Stefan Gerlach <stefan.gerlach@uni.kn>
        
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "NSLKDETest.h"

#include <algorithm>
#include <cmath>

extern "C" {
#include "backend/nsl/nsl_kde.h"
}

void NSLKDETest::testDensityAndBandwidth() {
	double data[] = {0.};
	QVERIFY(std::abs(nsl_kde(data, 0., nsl_kernel_uniform, 2., 1) - 0.25) < 1.e-12);
	QVERIFY(std::abs(nsl_kde(data, 2., nsl_kernel_uniform, 2., 1) - 0.25) < 1.e-12);
	QCOMPARE(nsl_kde(data, 2.001, nsl_kernel_uniform, 2., 1), 0.);
	QVERIFY(std::abs(nsl_kde(data, 0., nsl_kernel_gauss, 2., 1) - 1. / (2. * std::sqrt(2. * M_PI))) < 1.e-12);

	const nsl_kernel_type kernels[] = {nsl_kernel_uniform, nsl_kernel_triangular, nsl_kernel_parabolic, nsl_kernel_quartic,
										nsl_kernel_triweight, nsl_kernel_tricube, nsl_kernel_cosine, nsl_kernel_gauss};
	for (const auto kernel : kernels)
		QVERIFY(std::isfinite(nsl_kde(data, 0.25, kernel, 1., 1)));

	const double n = 25.;
	const double sigma = 2.;
	const double iqr = 2.68;
	const double silverman = 0.9 * std::min(sigma, iqr / 1.34) * std::pow(n, -0.2);
	const double scott = 1.059 * sigma * std::pow(n, -0.2);
	QVERIFY(std::abs(nsl_kde_bandwidth(25, sigma, iqr, nsl_kde_bandwidth_silverman) - silverman) < 1.e-12);
	QVERIFY(std::abs(nsl_kde_bandwidth(25, sigma, iqr, nsl_kde_bandwidth_scott) - scott) < 1.e-12);
	QCOMPARE(nsl_kde_bandwidth(25, sigma, iqr, nsl_kde_bandwidth_custom), 1.e-6);

	double sample[] = {-2, -1, 0, 1, 2};
	const double dataScott = nsl_kde_scott_bandwidth(sample, 5);
	const double dataSilverman = nsl_kde_silverman_bandwidth(sample, 5);
	QVERIFY(std::isfinite(dataSilverman));
	QVERIFY(std::abs(nsl_kde_bandwidth_from_data(sample, 5, nsl_kde_bandwidth_silverman) - dataSilverman) < 1.e-12);
	QVERIFY(std::abs(dataScott - nsl_kde_bandwidth(5, std::sqrt(2.5), 2., nsl_kde_bandwidth_scott)) < 1.e-12);
	QVERIFY(std::abs(nsl_kde_bandwidth_from_data(sample, 5, nsl_kde_bandwidth_scott) - dataScott) < 1.e-12);
	QCOMPARE(nsl_kde_bandwidth_from_data(sample, 5, nsl_kde_bandwidth_custom), 1.e-6);
}

QTEST_MAIN(NSLKDETest)