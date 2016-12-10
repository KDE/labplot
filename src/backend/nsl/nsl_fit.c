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

#include "nsl_common.h"
#include "nsl_fit.h"
#include <gsl/gsl_math.h>

const char* nsl_fit_model_name[] = {i18n("Polynomial"), i18n("Power"), i18n("Exponential"), i18n("Inverse Exponential"), i18n("Fourier"),
	i18n("Gaussian"), i18n("Lorentz (Cauchy)"), i18n("Maxwell-Boltzmann"), i18n("Sigmoid"), i18n("Gompertz"), i18n("Weibull"),
	i18n("Log-Normal"), i18n("Gumbel"), i18n("Custom")};

const char* nsl_fit_model_equation[] = {"c0 + c1*x", "a*x^b", "a*exp(b*x)", "a*(1-exp(b*x))+c", "a0 + (a1*cos(w*x) + b1*sin(w*x))",
	"c1/sqrt(2*pi)/a1*exp(-((x-b1)/a1)^2/2)", "a/pi*s/(s^2+(x-t)^2)", "c*sqrt(2/pi)*x^2*exp(-x^2/(2*a^2))/a^3", "a/(1+exp(-b*(x-c)))",
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

/********************** parameter derivatives ******************/

double nsl_fit_model_polynomial_param_deriv(double x, int j, double sigma) {
	return pow(x, j)/sigma;	
}
double nsl_fit_model_power1_param_deriv(int param, double x, double a, double b, double sigma) {
	if (param == 0)
		return pow(x, b)/sigma;
	if (param == 1)
		return a*pow(x, b)*log(x)/sigma;
	return 0;
}
double nsl_fit_model_power2_param_deriv(int param, double x, double b, double c, double sigma) {
	if (param == 0)
		return 1./sigma;
	if (param == 1)
		return pow(x,c)/sigma;
	if (param == 2)
		return b*pow(x,c)*log(x)/sigma;
	return 0;
}
double nsl_fit_model_exponential1_param_deriv(int param, double x, double a, double b, double sigma) {
	if (param == 0)
		return exp(b*x)/sigma;
	if (param == 1)
		return a*x*exp(b*x)/sigma;
	return 0;
}
double nsl_fit_model_exponential2_param_deriv(int param, double x, double a, double b, double c, double d, double sigma) {
	if (param == 0)
		return exp(b*x)/sigma;
	if (param == 1)
		return a*x*exp(b*x)/sigma;
	if (param == 2)
		return exp(d*x)/sigma;
	if (param == 3)
		return c*x*exp(d*x)/sigma;
	return 0;
}
double nsl_fit_model_exponential3_param_deriv(int param, double x, double a, double b, double c, double d, double e, double f, double sigma) {
	if (param == 0)
		return exp(b*x)/sigma;
	if (param == 1)
		return a*x*exp(b*x)/sigma;
	if (param == 2)
		return exp(d*x)/sigma;
	if (param == 3)
		return c*x*exp(d*x)/sigma;
	if (param == 4)
		return exp(f*x)/sigma;
	if (param == 5)
		return e*x*exp(f*x)/sigma;
	return 0;
}
double nsl_fit_model_inverse_exponential_param_deriv(int param, double x, double a, double b, double sigma) {
	if (param == 0)
		return (1. - exp(b*x))/sigma;
	if (param == 1)
		return -a*x*exp(b*x)/sigma;
	if (param == 2)
		return 1./sigma;
	return 0;
}
double nsl_fit_model_fourier_param_deriv(int param, int degree, double x, double w, double sigma) {
	if (param == 0)
		return cos(degree*w*x)/sigma;
	if (param == 1)
		return sin(degree*w*x)/sigma;

	return 0;
}
double nsl_fit_model_gaussian_param_deriv(int param, double x, double a, double b, double c, double sigma) {
	double a2 = a*a, norm = 1./sqrt(2.*M_PI)/a/sigma, efactor = exp(-(x-b)*(x-b)/(2.*a2));

	if (param == 0)
		return c * norm/(a*a2) * ((x-b)*(x-b) - a2) * efactor;
	if (param == 1)
		return c * norm/a2 * (x-b) * efactor;
	if (param == 2)
		return norm * efactor;
		
	return 0;
}
double nsl_fit_model_lorentz_param_deriv(int param, double x, double s, double t, double a, double sigma) {
	double norm = 1./M_PI/sigma, denom = s*s+(x-t)*(x-t);

	if (param == 0)
		return a * norm * ((x-t)*(x-t) - s*s)/(denom*denom);
	if (param == 1)
		return a * norm * 2.*s*(x-t)/(denom*denom);
	if (param == 2)
		return norm * s/denom;

	return 0;
}
double nsl_fit_model_maxwell_param_deriv(int param, double x, double a, double c, double sigma) {
	double a2 = a*a, a3 = a*a2, norm = sqrt(2./M_PI)/a3/sigma, x2 = x*x, efactor = exp(-x2/2./a2);

	if (param == 0)
		return c * norm * x2*(x2-3.*a2)/a3 * efactor;
	if (param == 1)
		return norm * x2 * efactor;

	return 0;
}
double nsl_fit_model_sigmoid_param_deriv(int param, double x, double a, double b, double c, double sigma) {
	if (param == 0)
		return 1./(exp(b*(c-x)) + 1.)/sigma;
	if (param == 1)
		return a*(x-c)*exp((c-x)*b)/pow(exp((c-x)*b) + 1., 2)/sigma;
	if (param == 2)
		return -a*b*exp(b*(c-x))/pow(exp(b*(c-x)) + 1., 2)/sigma;
	return 0;
}
double nsl_fit_model_gompertz_param_deriv(int param, double x, double a, double b, double c, double sigma) {
	if (param == 0)
		return exp(-b*exp(-c*x))/sigma;
	if (param == 1)
		return -a*exp(-c*x-b*exp(-c*x))/sigma;
	if (param == 2)
		return a*b*x*exp(-c*x-b*exp(-c*x))/sigma;
	return 0;
}
double nsl_fit_model_weibull_param_deriv(int param, double x, double a, double b, double c, double sigma) {
	double d = pow((x-c)/b, a);

	if (param == 0)
		return (exp(-d)*d*(a*(d-1.)*log((x-c)/b)-1.))/(c-x)/sigma;
	if (param == 1)
		return (pow(a, 2)*exp(-d)*d*(d-1.))/(b*(x-c))/sigma;
	if (param == 2)
		return (a*exp(-d)*d*(a*(d-1.)+1.))/pow(c-x, 2)/sigma;
	return 0;
}
double nsl_fit_model_lognormal_param_deriv(int param, double x, double a, double b, double sigma) {
	if (param == 0)
		return -(exp(-pow(b-log(x), 2)/(2.*pow(a, 2)))*(a+b-log(x))*(a-b+log(x)))/(sqrt(2.*M_PI)*pow(a, 4)*x)/sigma;
	if (param == 1)
		return ((log(x)-b)*exp(-pow(b-log(x), 2)/(2.*pow(a, 2))))/(sqrt(2.*M_PI)*pow(a, 3)*x)/sigma;
	return 0;
}
double nsl_fit_model_gumbel_param_deriv(int param, double x, double a, double b, double sigma) {
	if (param == 0)
		return (exp((x-2.*b)/a-exp((x-b)/a))*(exp(x/a)*(x-b)-exp(b/a)*(a-b+x)))/pow(a, 3)/sigma;
	if (param == 1)
		return (exp(-exp(x/a-b/a)-(2.*b)/a+x/a)*(exp(x/a)-exp(b/a)))/pow(a, 2)/sigma;
	return 0;
}
