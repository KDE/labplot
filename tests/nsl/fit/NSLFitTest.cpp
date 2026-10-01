/*
	File                 : NSLFitTest.cpp
	Project              : LabPlot
	Description          : NSL Tests for fitting
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2019-2026 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "NSLFitTest.h"

#include <cmath>
#include <limits>

extern "C" {
#include "backend/nsl/nsl_fit.h"
}

// ##############################################################################
// #################  bound test
// ##############################################################################

const int N = 11;
const double data_unbound[] = {-4, -3, -2, -1, -.1, 0, .1, 1, 2, 3, 4};
const double result_bound[] = {1.63520374296189,
							   0.288319987910199,
							   -0.863946140238523,
							   -0.762206477211845,
							   0.350249875029758,
							   0.5,
							   0.649750124970242,
							   1.76220647721184,
							   1.86394614023852,
							   0.711680012089801,
							   -0.635203742961892};
const double data_bound[] = {-1, -.99, -.5, 0, .49, .5, .51, 1, 1.5, 1.99, 2};
const double result_unbound[] = {-1.5707963267949,
								 -1.45526202651066,
								 -0.729727656226966,
								 -0.339836909454122,
								 -0.00666671605037044,
								 0,
								 0.00666671605037033,
								 0.339836909454122,
								 0.729727656226966,
								 1.45526202651066,
								 1.5707963267949};

void NSLFitTest::testBounds() {
	int i;
	for (i = 0; i < N; i++) {
		double x = nsl_fit_map_bound(data_unbound[i], -1, 2);
		//		printf("%g -> %.15g\n", data_unbound[i], x);
		QCOMPARE(x, result_bound[i]);
	}

	for (i = 0; i < N; ++i) {
		double x = nsl_fit_map_unbound(data_bound[i], -1, 2);
		//		printf("%g -> %.15g\n", data_bound[i], x);
		QCOMPARE(x, result_unbound[i]);
	}
}

void NSLFitTest::testNaNInfHandling() {
	QCOMPARE(nsl_fit_map_bound(NAN, -1.0, 2.0), DBL_MAX);
	QCOMPARE(nsl_fit_map_bound(std::numeric_limits<double>::infinity(), -1.0, 2.0), DBL_MAX);
	QCOMPARE(nsl_fit_map_bound(-std::numeric_limits<double>::infinity(), -1.0, 2.0), DBL_MAX);

	QCOMPARE(nsl_fit_map_unbound(NAN, -1.0, 2.0), -DBL_MAX);
	QCOMPARE(nsl_fit_map_unbound(std::numeric_limits<double>::infinity(), -1.0, 2.0), -DBL_MAX);
	QCOMPARE(nsl_fit_map_unbound(-std::numeric_limits<double>::infinity(), -1.0, 2.0), -DBL_MAX);

	QCOMPARE(nsl_fit_map_bound(0.0, -DBL_MAX, DBL_MAX), 0.0);
	QCOMPARE(nsl_fit_map_unbound(0.0, -DBL_MAX, DBL_MAX), 0.0);
}

void NSLFitTest::testModelParameterDerivatives() {
	const double x = 2.;
	const double weight = 1.;
	const double exponentialParameters[] = {1., 0.2, 0.8, -0.1};
	const double derivatives[] = {
		nsl_fit_model_polynomial_param_deriv(x, 2, weight),
		nsl_fit_model_power1_param_deriv(0, x, 2., 1.5, weight),
		nsl_fit_model_power2_param_deriv(0, x, 2., 1.5, weight),
		nsl_fit_model_exponentialn_param_deriv(0, x, exponentialParameters, weight),
		nsl_fit_model_inverse_exponential_param_deriv(0, x, 2., -0.2, weight),
		nsl_fit_model_fourier_param_deriv(0, 2, x, 0.5, weight),
		nsl_fit_model_gaussian_param_deriv(0, x, 1., 1., 0., weight),
		nsl_fit_model_lorentz_param_deriv(0, x, 1., 1., 0., weight),
		nsl_fit_model_sech_param_deriv(0, x, 1., 1., 0., weight),
		nsl_fit_model_logistic_param_deriv(0, x, 1., 1., 0., weight),
		nsl_fit_model_voigt_param_deriv(0, x, 1., 0., 1., 1., weight),
		nsl_fit_model_pseudovoigt1_param_deriv(0, x, 1., 0.5, 1., 0., weight),
		nsl_fit_model_atan_param_deriv(0, x, 1., 0., 1., weight),
		nsl_fit_model_tanh_param_deriv(0, x, 1., 0., 1., weight),
		nsl_fit_model_algebraic_sigmoid_param_deriv(0, x, 1., 0., 1., weight),
		nsl_fit_model_sigmoid_param_deriv(0, x, 1., 0., 1., weight),
		nsl_fit_model_erf_param_deriv(0, x, 1., 0., 1., weight),
		nsl_fit_model_hill_param_deriv(0, x, 1., 2., 1., weight),
		nsl_fit_model_gompertz_param_deriv(0, x, 1., 1., 1., weight),
		nsl_fit_model_gudermann_param_deriv(0, x, 1., 0., 1., weight),
		nsl_fit_model_gaussian_tail_param_deriv(0, x, 1., 1., 0.5, 0., weight),
		nsl_fit_model_exponential_param_deriv(0, x, 1., 1., 0., weight),
		nsl_fit_model_laplace_param_deriv(0, x, 1., 1., 0., weight),
		nsl_fit_model_exp_pow_param_deriv(0, x, 1., 1., 2., 0., weight),
		nsl_fit_model_poisson_param_deriv(0, x, 1., 2., weight),
		nsl_fit_model_lognormal_param_deriv(0, x, 1., 1., 0., weight),
		nsl_fit_model_gamma_param_deriv(0, x, 1., 2., 1., weight),
		nsl_fit_model_flat_param_deriv(0, x, 1., 0., 4., weight),
		nsl_fit_model_rayleigh_param_deriv(0, x, 1., 1., weight),
		nsl_fit_model_rayleigh_tail_param_deriv(0, x, 1., 1., 0., weight),
		nsl_fit_model_landau_param_deriv(0, x, weight),
		nsl_fit_model_chi_square_param_deriv(0, x, 1., 3., weight),
		nsl_fit_model_students_t_param_deriv(0, x, 1., 3., weight),
		nsl_fit_model_fdist_param_deriv(0, x, 1., 3., 5., weight),
		nsl_fit_model_beta_param_deriv(0, 0.5, 1., 2., 3., weight),
		nsl_fit_model_pareto_param_deriv(0, x, 1., 1., 3., weight),
		nsl_fit_model_weibull_param_deriv(0, x, 1., 2., 1., 0., weight),
		nsl_fit_model_gumbel1_param_deriv(0, x, 1., 1., 0., 1., weight),
		nsl_fit_model_gumbel2_param_deriv(0, x, 1., 1., 2., 0., weight),
		nsl_fit_model_binomial_param_deriv(0, 2., 1., 0.4, 10., weight),
		nsl_fit_model_negative_binomial_param_deriv(0, 2., 1., 0.4, 3., weight),
		nsl_fit_model_pascal_param_deriv(0, 2., 1., 0.4, 3., weight),
		nsl_fit_model_geometric_param_deriv(0, 2., 1., 0.4, weight),
		nsl_fit_model_hypergeometric_param_deriv(0, 2., 1., 5., 4., 3., weight),
		nsl_fit_model_logarithmic_param_deriv(0, 1., 1., 0.4, weight),
		nsl_fit_model_maxwell_param_deriv(0, x, 1., 1., weight),
		nsl_fit_model_sech_dist_param_deriv(0, x, 1., 1., 0., weight),
		nsl_fit_model_levy_param_deriv(0, x, 1., 1., 0., weight),
		nsl_fit_model_frechet_param_deriv(0, x, 1., 2., 1., 0., weight),
	};
	for (double derivative : derivatives)
		QVERIFY(std::isfinite(derivative));
}

// ##############################################################################
// #################  performance
// ##############################################################################

QTEST_MAIN(NSLFitTest)
