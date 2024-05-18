/*
	File                 : nsl_sf_randist.c
	Project              : LabPlot
	Description          : NSL random number distributions
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "nsl_randist.h"
#include <math.h>

// calculate triangular distributed random number from uniform [0,1] distributed random number u
// a - lower limit, b - upper limit, c - mode (max value)
// see https://en.wikipedia.org/wiki/Triangular_distribution#Generating_random_variates
double nsl_ran_triangular(gsl_rng* r, double a, double b, double c) {
	if (b <= a || c < a || c > b)
		return 0.;

	double u = gsl_rng_uniform(r); // u \in [0,1)

	double f = (c - a) / (b - a);

	if (u < f)
		return a + sqrt(u * (b - a) * (c - a));
	else
		return b - sqrt((1. - u) * (b - a) * (b - c));
}
