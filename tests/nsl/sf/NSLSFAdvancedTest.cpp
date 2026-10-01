/*
	File                 : NSLSFAdvancedTest.cpp
	Project              : LabPlot
	Description          : Tests for additional NSL special functions
	--------------------------------------------------------------------
        SPDX-FileCopyrightText: 2026 Stefan Gerlach <stefan.gerlach@uni.kn>
        
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "NSLSFAdvancedTest.h"

#include <cmath>
#include <iterator>

extern "C" {
#include "backend/nsl/nsl_randist.h"
#include "backend/nsl/nsl_sf_kernel.h"
#include "backend/nsl/nsl_sf_poly.h"
#include "backend/nsl/nsl_sf_stats.h"
}

void NSLSFAdvancedTest::testKernelFunctions() {
	using Kernel = double (*)(double);
	const Kernel compactKernels[] = {nsl_sf_kernel_uniform,
									 nsl_sf_kernel_triangular,
									 nsl_sf_kernel_parabolic,
									 nsl_sf_kernel_quartic,
									 nsl_sf_kernel_triweight,
									 nsl_sf_kernel_tricube,
									 nsl_sf_kernel_cosine,
									 nsl_sf_kernel_semicircle};
	const double centerValues[] = {0.5, 1., 0.75, 15. / 16., 35. / 32., 70. / 81., M_PI_4, 2. / M_PI};
	for (size_t i = 0; i < std::size(compactKernels); ++i) {
		QVERIFY(std::abs(compactKernels[i](0.) - centerValues[i]) < 1.e-12);
		QVERIFY(std::abs(compactKernels[i](-0.25) - compactKernels[i](0.25)) < 1.e-12);
		QCOMPARE(compactKernels[i](1.1), 0.);
	}

	const Kernel unboundedKernels[] = {nsl_sf_kernel_gaussian,
										nsl_sf_kernel_cauchy,
										nsl_sf_kernel_logistic,
										nsl_sf_kernel_picard,
										nsl_sf_kernel_sigmoid,
										nsl_sf_kernel_silverman};
	const double unboundedCenters[] = {1. / std::sqrt(2. * M_PI), 1. / M_PI, 0.25, 0.5, 1. / M_PI, std::sqrt(0.5) / 2.};
	for (size_t i = 0; i < std::size(unboundedKernels); ++i) {
		QVERIFY(std::abs(unboundedKernels[i](0.) - unboundedCenters[i]) < 1.e-12);
		QVERIFY(std::abs(unboundedKernels[i](-0.25) - unboundedKernels[i](0.25)) < 1.e-12);
	}
}

void NSLSFAdvancedTest::testPolynomialFunctions() {
	QVERIFY(std::abs(nsl_sf_poly_chebyshev_T(3, 0.5) + 1.) < 1.e-12);
	QVERIFY(std::abs(nsl_sf_poly_chebyshev_U(2, 2.) - 15.) < 1.e-12);
	QVERIFY(std::abs(nsl_sf_poly_optimal_legendre_L(2, 0.5) - 0.25) < 1.e-12);
	QCOMPARE(nsl_sf_poly_optimal_legendre_L(0, 0.5), 0.);

	const gsl_complex zero = gsl_complex_rect(0., 0.);
	const gsl_complex besselZero = nsl_sf_poly_bessel_y(0, zero);
	const gsl_complex reversedZero = nsl_sf_poly_reversed_bessel_theta(0, zero);
	QVERIFY(std::abs(GSL_REAL(besselZero) - 1.) < 1.e-12);
	QVERIFY(std::abs(GSL_REAL(reversedZero) - 1.) < 1.e-12);
	const gsl_complex bessel = nsl_sf_poly_bessel_y(1, zero);
	const gsl_complex reversed = nsl_sf_poly_reversed_bessel_theta(1, zero);
	QVERIFY(std::abs(GSL_REAL(bessel) - 1.) < 1.e-12);
	QVERIFY(std::abs(GSL_REAL(reversed) - 1.) < 1.e-12);

	const double x2[] = {0, 1};
	const double y2[] = {0, 1};
	QCOMPARE(nsl_sf_poly_interp_lagrange_0_int(x2, 2.), 2.);
	QVERIFY(std::abs(nsl_sf_poly_interp_lagrange_1(0.5, x2, y2) - 0.5) < 1.e-12);
	QVERIFY(std::abs(nsl_sf_poly_interp_lagrange_1_deriv(x2, y2) - 1.) < 1.e-12);
	QCOMPARE(nsl_sf_poly_interp_lagrange_1_int(x2, y2), 0.5);
	QCOMPARE(nsl_sf_poly_interp_lagrange_1_absint(x2, y2), 0.5);

	const double x3[] = {0, 1, 2};
	const double y3[] = {0, 1, 2};
	QVERIFY(std::abs(nsl_sf_poly_interp_lagrange_2(0.5, x3, y3) - 0.5) < 1.e-12);
	QVERIFY(std::abs(nsl_sf_poly_interp_lagrange_2_deriv(0.5, x3, y3) - 1.) < 1.e-12);
	QVERIFY(std::abs(nsl_sf_poly_interp_lagrange_2_deriv2(x3, y3)) < 1.e-12);
	QVERIFY(std::abs(nsl_sf_poly_interp_lagrange_2_int(x3, y3) - 2.) < 1.e-12);

	const double x4[] = {0, 1, 2, 3};
	const double y4[] = {0, 1, 2, 3};
	QVERIFY(std::isfinite(nsl_sf_poly_interp_lagrange_3(1.5, x4, y4)));
	QVERIFY(std::isfinite(nsl_sf_poly_interp_lagrange_3_deriv(1.5, x4, y4)));
	QVERIFY(std::isfinite(nsl_sf_poly_interp_lagrange_3_deriv2(1.5, x4, y4)));
	QVERIFY(std::isfinite(nsl_sf_poly_interp_lagrange_3_deriv3(x4, y4)));
	QVERIFY(std::isfinite(nsl_sf_poly_interp_lagrange_3_int(x4, y4)));

	const double x5[] = {0, 1, 2, 3, 4};
	const double y5[] = {0, 1, 2, 3, 4};
	QVERIFY(std::isfinite(nsl_sf_poly_interp_lagrange_4(2., x5, y5)));
	QVERIFY(std::isfinite(nsl_sf_poly_interp_lagrange_4_deriv(2., x5, y5)));
	QVERIFY(std::isfinite(nsl_sf_poly_interp_lagrange_4_deriv2(2., x5, y5)));
	QVERIFY(std::isfinite(nsl_sf_poly_interp_lagrange_4_deriv3(2., x5, y5)));
	QVERIFY(std::isfinite(nsl_sf_poly_interp_lagrange_4_deriv4(x5, y5)));

	const double x7[] = {0, 1, 2, 3, 4, 5, 6};
	const double y7[] = {0, 1, 2, 3, 4, 5, 6};
	QVERIFY(std::isfinite(nsl_sf_poly_interp_lagrange_6_deriv4(3., x7, y7)));
	QVERIFY(std::isfinite(nsl_sf_poly_interp_lagrange_6_deriv5(3., x7, y7)));
	QVERIFY(std::isfinite(nsl_sf_poly_interp_lagrange_6_deriv6(x7, y7)));
}

void NSLSFAdvancedTest::testDistributionMetadata() {
	for (int i = 0; i < NSL_SF_STATS_DISTRIBUTION_COUNT; ++i) {
		const auto distribution = static_cast<nsl_sf_stats_distribution>(i);
		const bool supportsRNG = i != nsl_sf_stats_maxwell_boltzmann && i != nsl_sf_stats_sech && i != nsl_sf_stats_levy && i != nsl_sf_stats_frechet;
		QVERIFY(nsl_sf_stats_distribution_supports_RNG(distribution) == supportsRNG);
		const bool supportsML = i == nsl_sf_stats_gaussian || i == nsl_sf_stats_exponential || i == nsl_sf_stats_laplace || i == nsl_sf_stats_cauchy_lorentz
								|| i == nsl_sf_stats_lognormal || i == nsl_sf_stats_poisson || i == nsl_sf_stats_binomial;
		QVERIFY(nsl_sf_stats_distribution_supports_ML(distribution) == supportsML);
	}
}

void NSLSFAdvancedTest::testTriangularRandomDistribution() {
	gsl_rng_env_setup();
	gsl_rng* rng = gsl_rng_alloc(gsl_rng_mt19937);
	QVERIFY(rng != nullptr);
	gsl_rng_set(rng, 1234);
	const double value = nsl_ran_triangular(rng, 0., 2., 1.);
	QVERIFY(value >= 0. && value <= 2.);
	QCOMPARE(nsl_ran_triangular(rng, 2., 1., 1.), 0.);
	gsl_rng_free(rng);
}

QTEST_MAIN(NSLSFAdvancedTest)