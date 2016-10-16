/***************************************************************************
    File                 : nsl_fit.c
    Project              : LabPlot
    Description          : NSL (non)linear fit functions
    --------------------------------------------------------------------
    Copyright            : (C) 2016 by Stefan Gerlach (stefan.gerlach@uni.kn)
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

/* TODO:
	* nsl_sf_model_deriv(int param_index, ... )
*/

#include "nsl_common.h"
#include "nsl_fit.h"
#include <gsl/gsl_math.h>

const char* nsl_fit_model_name[] = {i18n("Polynomial"), i18n("Power"), i18n("Exponential"), i18n("Inverse Exponential"), i18n("Fourier"),
	i18n("Gaussian"), i18n("Lorentz (Cauchy)"), i18n("Maxwell-Boltzmann"), i18n("Sigmoid"), i18n("Gompertz"), i18n("Weibull"),
	i18n("Log-Normal"), i18n("Gumbel"), i18n("Custom")};

const char* nsl_fit_model_equation[] = {"c0 + c1*x", "a*x^b", "a*exp(b*x)", "a*(1-exp(b*x))+c", "a0 + (a1*cos(w*x) + b1*sin(w*x))",
	"1/sqrt(2*pi)/a1*exp(-((x-b1)/a1)^2/2)", "1/pi*s/(s^2+(x-t)^2)", "sqrt(2/pi)*x^2*exp(-x^2/(2*a^2))/a^3", "a/(1+exp(-b*(x-c)))",
	"a*exp(-b*exp(-c*x))", "a/b*((x-c)/b)^(a-1)*exp(-((x-c)/b)^a)", "1/(sqrt(2*pi)*x*a)*exp(-(log(x)-b)^2/(2*a^2))", 
	"1/a*exp((x-b)/a-exp((x-b)/a))"};

/* 
	see http://www.quantcode.com/modules/smartfaq/faq.php?faqid=96
	and https://lmfit.github.io/lmfit-py/bounds.html
*/
double nsl_fit_map_bound(double x, double min, double max) {
	if (max <= min) {
		printf("given bounds must fulfill max > min (min = %g, max = %g)! Giving up.\n", min, max);
		return DBL_MAX;
	}

	/* not bounded */
	if (min == -DBL_MAX && max == DBL_MAX)
		return x;

	/* open bounds */
	if (min == -DBL_MAX)
		return max + 1 - sqrt(x*x + 1);
	if (max == DBL_MAX)
		return min - 1 + sqrt(x*x + 1);

	return min + (max - min)/(1.0 + exp(-x));
}

/* 
	see http://www.quantcode.com/modules/smartfaq/faq.php?faqid=96
	and https://lmfit.github.io/lmfit-py/bounds.html
*/
double nsl_fit_map_unbound(double x, double min, double max) {
	if (max <= min) {
		printf("given bounds must fulfill max > min (min = %g, max = %g)! Giving up.\n", min, max);
		return DBL_MAX;
	}
	if (x < min || x > max) {
		printf("given value must be within bounds! Giving up.\n");
		return -DBL_MAX;
	}

	/* not bounded */
	if (min == -DBL_MAX && max == DBL_MAX)
		return x;

	/* open bounds */
	if (min == -DBL_MAX)
		return sqrt(gsl_pow_2(max - x + 1) - 1);
	if (max == DBL_MAX)
		return sqrt(gsl_pow_2(x - min + 1) - 1);

	return -log((max - x)/(x - min));
}
