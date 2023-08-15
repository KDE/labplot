/*
	File                 : nsl_interp.c
	Project              : LabPlot
	Description          : NSL interpolation functions
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2016 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "nsl_interp.h"
#include "nsl_common.h"

const char* nsl_interp_type_name[] = {i18n("Linear"),
									  i18n("Polynomial"),
									  i18n("Cubic Spline (Natural)"),
									  i18n("Cubic Spline (Periodic)"),
									  i18n("Akima-Spline (Natural)"),
									  i18n("Akima-Spline (Periodic)"),
									  i18n("Steffen-Spline"),
									  i18n("Cosine"),
									  i18n("Exponential"),
									  i18n("Piecewise Cubic Hermite (PCH)"),
									  i18n("Rational Functions")};
const char* nsl_interp_pch_variant_name[] = {i18n("Finite Differences"), i18n("Catmull-Rom"), i18n("Cardinal"), i18n("Kochanek-Bartels (TCB)")};
const char* nsl_interp_evaluate_name[] = {i18n("Function"), i18n("Derivative"), i18n("Second Derivative"), i18n("Integral")};

int nsl_interp_ratint(const double* x, const double* y, int n, double xn, double* v, double* dv) {
	int i, a = 0, b = n - 1;
	while (b - a > 1) { /* find interval using bisection */
		int j = (int)floor((a + b) / 2.);
		if (x[j] > xn)
			b = j;
		else
			a = j;
	}

	int ns = a; /* nearest index */
	if (fabs(xn - x[a]) > fabs(xn - x[b]))
		ns = b;

	if (xn == x[ns]) { /* exact point */
		*v = y[ns];
		*dv = 0;
		return 1;
	}

	double* c = (double*)malloc(n * sizeof(double));
	double* d = (double*)malloc(n * sizeof(double));
	for (i = 0; i < n; i++)
		c[i] = d[i] = y[i];

	*v = y[ns--];

	double t, dd;
	int m;
	for (m = 1; m < n; m++) {
		for (i = 0; i < n - m; i++) {
			t = (x[i] - xn) * d[i] / (x[i + m] - xn);
			dd = t - c[i + 1];
			if (dd == 0.0) /* pole */
				dd += DBL_MIN;
			dd = (c[i + 1] - d[i]) / dd;
			d[i] = c[i + 1] * dd;
			c[i] = t * dd;
		}

		*dv = (2 * (ns + 1) < (n - m) ? c[ns + 1] : d[ns--]);
		*v += *dv;
	}

	free(c);
	free(d);

	return 0;
}
