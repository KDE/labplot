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

const char* nsl_interp_type_name[] = { i18n("linear"), i18n("polynomial"), i18n("cubic spline (natural)"), i18n("cubic spline (periodic)"),
	i18n("Akima-spline (natural)"), i18n("Akima-spline (periodic)"), i18n("Steffen spline"), i18n("cosine"), i18n("exponential"),
	i18n("piecewise cubic Hermite (PCH)"), i18n("rational functions") };
const char* nsl_interp_pch_variant_name[] = { i18n("finite differences"), i18n("Catmull-Rom"), i18n("cardinal"), i18n("Kochanek-Bartels (TCB)")};
const char* nsl_interp_evaluate_name[] = { i18n("function"), i18n("derivative"), i18n("second derivative"), i18n("integral")};

int nsl_interp_ratint(double *x, double *y, int n, double xn, double *v, double *dv) {
	int i, a = 0, b = n-1;
	while (b-a > 1) {       /* find interval using bisection */
		int j = (int)floor((a+b)/2.);
		if (x[j] > xn)
			b = j;
		else
			a = j;
	}

	int ns=a;	/* nearest index */
	if (fabs(xn-x[a]) > fabs(xn-x[b]))
		ns=b;

	if (xn == x[ns]) {      /* exact point */
		*v = y[ns];
		*dv = 0;
		return 1;
	}

	double *c = (double*)malloc(n*sizeof(double));
	double *d = (double*)malloc(n*sizeof(double));
	for (i=0; i < n; i++)
		c[i] = d[i] = y[i];

	*v = y[ns--];

	double t, dd;
	int m;
	for (m=1; m < n; m++) {
		for (i=0; i < n-m; i++) {
			t = (x[i]-xn)*d[i]/(x[i+m]-xn);
			dd = t-c[i+1];
			if (dd == 0.0) /* pole */
				dd += DBL_MIN;
			dd = (c[i+1]-d[i])/dd;
			d[i] = c[i+1]*dd;
			c[i] = t*dd;
		}

		*dv = (2*(ns+1) < (n-m) ? c[ns+1] : d[ns--]);
		*v += *dv;
	}

	free(c); free(d);

	return 0;
}
