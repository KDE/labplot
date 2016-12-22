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
#include <math.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_sf_gamma.h>
#include <gsl/gsl_sf_psi.h>

const char* nsl_fit_model_category_name[] = {i18n("Basic functions"), i18n("Peak Functions"), i18n("Growth (Sigmoidal)"), i18n("Statistics"), i18n("Custom")};

const char* nsl_fit_model_basic_name[] = {i18n("Polynomial"), i18n("Power"), i18n("Exponential"), i18n("Inverse Exponential"), i18n("Fourier")};
const char* nsl_fit_model_peak_name[] = {i18n("Gaussian (normal)"), i18n("Cauchy-Lorentz"), i18n("Hyperbolic secant (sech)"), i18n("Logistic (sech squared)")};
const char* nsl_fit_model_growth_name[] = {i18n("Arctangent"), i18n("Hyperbolic tangent"), i18n("Algebraic sigmoid"), i18n("Logistic function"), 
	i18n("Error function (erf)"), i18n("Hill"), i18n("Gompertz"), i18n("Gudermann (gd)")};
const char* nsl_fit_model_distribution_name[] = {i18n("Normal (Gauss)"), i18n("Cauchy-Lorentz"), i18n("Maxwell-Boltzmann"), i18n("Poisson"), i18n("Logistic"),
	i18n("Log-Normal"), i18n("Gamma"), i18n("Laplace"), i18n("Rayleigh"), i18n("Levy"), i18n("Chi-Square"), i18n("Weibull"), i18n("Frechet (inverse Weibull)"),
	i18n("Gumbel"), i18n("Hyperbolic secant (sech)")};

const char* nsl_fit_model_basic_equation[] = {"c0 + c1*x", "a*x^b", "a*exp(b*x)", "a*(1-exp(b*x)) + c", "a0 + (a1*cos(w*x) + b1*sin(w*x))"};
const char* nsl_fit_model_peak_equation[] = {"a/sqrt(2*pi)/s * exp(-((x-mu)/s)^2/2)", "a/pi * s/(s^2+(x-t)^2)", "a/pi/s * sech((x-mu)/s)",
	"a/4/s * sech((x-mu)/2/s)**2"};
const char* nsl_fit_model_growth_equation[] = {"a * atan((x-mu)/s)", "a * tanh((x-mu)/s)", "a * (x-mu)/s/sqrt(1+((x-mu)/s)^2)", "a/(1+exp(-k*(x-mu)))",
	"a/2 * erf((x-mu)/s/sqrt(2))", "a * x^n/(s^n + x^n)", "a*exp(-b*exp(-c*x))", "a * asin(tanh((x-mu)/s))"};
const char* nsl_fit_model_distribution_equation[] = {"a/sqrt(2*pi)/s * exp(-((x-mu)/s)^2/2)", "a/pi * s/(s^2+(x-t)^2)",
	"c*sqrt(2/pi) * x^2/a^3 * exp(-(x/a)^2/2)", "Poisson", "a/4/s * 1/cosh((x-mu)/2/s)**2",
	"a/(sqrt(2*pi)*x*s) * exp(-( (log(x)-mu)/s )^2/2)", "a * b^p/gamma(p)*x^(p-1)*exp(-b*x)", "a/(2*s) * exp(-fabs(x-mu)/s)",
	"a * x/(s*s) * exp(-x*x/(s*s)/2)", "a * sqrt(g/(2*pi))/pow(x-mu, 1.5) * exp(-g/2./(x-mu))", "a * pow(x,n/2.-1.)/pow(2, n/2.)/gamma(n/2.) * exp(-x/2.)",
	"a * k/l * ((x-mu)/l)^(k-1) * exp(-((x-mu)/l)^k)", "c * a/s*((x-mu)/s)^(-a-1) * exp(-((x-mu)/s)^(-a))", "a/b * exp((x-mu)/b - exp((x-mu)/b))",
	"1./2. * 1./cosh(pi*x/2.)"};

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

/* basic */
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

/* peak */
double nsl_fit_model_gaussian_param_deriv(int param, double x, double s, double mu, double a, double sigma) {
	double s2 = s*s, norm = 1./sqrt(2.*M_PI)/s/sigma, efactor = exp(-(x-mu)*(x-mu)/(2.*s2));

	if (param == 0)
		return a * norm/(s*s2) * ((x-mu)*(x-mu) - s2) * efactor;
	if (param == 1)
		return a * norm/s2 * (x-mu) * efactor;
	if (param == 2)
		return norm * efactor;
	return 0;
}
double nsl_fit_model_cauchy_lorentz_param_deriv(int param, double x, double s, double t, double a, double sigma) {
	double norm = 1./M_PI/sigma, denom = s*s+(x-t)*(x-t);

	if (param == 0)
		return a * norm * ((x-t)*(x-t) - s*s)/(denom*denom);
	if (param == 1)
		return a * norm * 2.*s*(x-t)/(denom*denom);
	if (param == 2)
		return norm * s/denom;
	return 0;
}
double nsl_fit_model_sech_param_deriv(int param, double x, double s, double mu, double a, double sigma) {
	double y = (x-mu)/s, norm = 1./M_PI/s/sigma;

	if (param == 0)
		return a/s * norm * (y*tanh(y)-1.)/cosh(y);
	if (param == 1)
		return a/s * norm * tanh(y)/cosh(y);
	if (param == 2)
		return norm/cosh(y);
	return 0;
}
double nsl_fit_model_logistic_param_deriv(int param, double x, double s, double mu, double a, double sigma) {
	double y = (x-mu)/2/s, norm = 1./4/s/sigma;

	if (param == 0)
		return a/s * norm * (2.*y*tanh(y)-1.)/cosh(y);
	if (param == 1)
		return a/s * norm * tanh(y)/cosh(y)/cosh(y);
	if (param == 2)
		return norm/cosh(y)/cosh(y);
	return 0;
}

/* growth */
double nsl_fit_model_atan_param_deriv(int param, double x, double s, double mu, double a, double sigma) {
	double norm = 1./sigma, y = (x-mu)/s;
	if (param == 0)
		return -a/s * norm * y/(1.+y*y);
	if (param == 1)
		return -a/s * norm * 1./(1+y*y);
	if (param == 2)
		return norm * atan(y);
	return 0;
}
double nsl_fit_model_tanh_param_deriv(int param, double x, double s, double mu, double a, double sigma) {
	double norm = 1./sigma, y = (x-mu)/s;
	if (param == 0)
		return -a/s * norm * y/cosh(y)/cosh(y);
	if (param == 1)
		return -a/s * norm * 1./cosh(y)/cosh(y);
	if (param == 2)
		return norm * tanh(y);
	return 0;
}
double nsl_fit_model_algebraic_sigmoid_param_deriv(int param, double x, double s, double mu, double a, double sigma) {
	double norm = 1./sigma, y = (x-mu)/s, y2 = y*y;
	if (param == 0)
		return -a/s * norm * y/pow(1+y2, 1.5);
	if (param == 1)
		return -a/s * norm * 1./pow(1+y2, 1.5);
	if (param == 2)
		return norm * y/sqrt(1.+y2);
	return 0;
}
double nsl_fit_model_sigmoid_param_deriv(int param, double x, double k, double mu, double a, double sigma) {
	double norm = 1./sigma, y = k*(x-mu);
	if (param == 0)
		return a/k * norm * y*exp(-y)/pow(1. + exp(-y), 2);
	if (param == 1)
		return -a*k * norm * exp(-y)/pow(1. + exp(-y), 2);
	if (param == 2)
		return norm/(1. + exp(-y));
	return 0;
}
double nsl_fit_model_erf_param_deriv(int param, double x, double s, double mu, double a, double sigma) {
	double norm = 1./sigma, y = (x-mu)/(sqrt(2.)*s);
	if (param == 0)
		return -a/sqrt(M_PI)/s * norm * y*exp(-y*y);
	if (param == 1)
		return -a/sqrt(2.*M_PI)/s * norm * exp(-y*y);
	if (param == 2)
		return norm/2. * erf(y);
	return 0;
}
double nsl_fit_model_hill_param_deriv(int param, double x, double s, double n, double a, double sigma) {
	double norm = 1./sigma, y = x/s;
	if (param == 0)
		return -a*n/s * norm * pow(y, n)/pow(1.+pow(y, n), 2.);
	if (param == 1)
		return a * norm * log(y)*pow(y, n)/pow(1.+pow(y, n), 2.);
	if (param == 2)
		return norm * pow(y, n)/(1.+pow(y, n));
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
double nsl_fit_model_gudermann_param_deriv(int param, double x, double s, double mu, double a, double sigma) {
	double norm = 1./sigma, y = (x-mu)/s;
	if (param == 0)
		return -a/s * norm * y/cosh(y);
	if (param == 1)
		return -a/s * norm * 1./cosh(y);
	if (param == 2)
		return -asin(tanh(y));
	return 0;
}

/* distributions */
double nsl_fit_model_maxwell_param_deriv(int param, double x, double a, double c, double sigma) {
	double a2 = a*a, a3 = a*a2, norm = sqrt(2./M_PI)/a3/sigma, x2 = x*x, efactor = exp(-x2/2./a2);

	if (param == 0)
		return c * norm * x2*(x2-3.*a2)/a3 * efactor;
	if (param == 1)
		return norm * x2 * efactor;
	return 0;
}
double nsl_fit_model_lognormal_param_deriv(int param, double x, double s, double mu, double a, double sigma) {
	double norm = 1./sqrt(2.*M_PI)/(x*s)/sigma, y = log(x)-mu, efactor = exp(-(y/s)*(y/s)/2.);

	if (param == 0)
		return a * norm * (y*y - s*s) * efactor;
	if (param == 1)
		return a * norm * y/(s*s) * efactor;
	if (param == 2)
		return norm * efactor;
	return 0;
}
double nsl_fit_model_gamma_param_deriv(int param, double x, double b, double p, double a, double sigma) {
	double factor = pow(b, p)*pow(x, p-1.)/gsl_sf_gamma(p)/sigma, efactor = exp(-b*x);

	if (param == 0)
		return a * factor/b * (p-b*x) * efactor;
	if (param == 1)
		return a * factor * (log(b*x) - gsl_sf_psi(p)) * efactor;
	if (param == 2)
		return factor * efactor;
	return 0;
}
double nsl_fit_model_laplace_param_deriv(int param, double x, double s, double mu, double a, double sigma) {
	double norm = 1./(2.*s)/sigma, y = fabs(x-mu)/s, efactor = exp(-y);

	if (param == 0)
		return a/s*norm * (y-1.) * efactor;
	if (param == 1)
		return a/(s*s)*norm * (x-mu)/y * efactor;
	if (param == 2)
		return norm * efactor;
	return 0;
}
double nsl_fit_model_rayleigh_param_deriv(int param, double x, double s, double a, double sigma) {
	double y=x/s, norm = y/s/sigma, efactor = exp(-y*y/2.);

	if (param == 0)
		return a*y/(s*s) * (y*y-2.)*efactor;
	if (param == 1)
		return norm * efactor;
	return 0;
}
double nsl_fit_model_levy_param_deriv(int param, double x, double g, double mu, double a, double sigma) {
	double y=x-mu, norm = sqrt(g/(2.*M_PI))/pow(y, 1.5)/sigma, efactor = exp(-g/2./y);

	if (param == 0)
		return a/2.*norm/g/y * (y - g) * efactor;
	if (param == 1)
		return a/2.*norm/y/y * (3.*y - g) * efactor;
	if (param == 2)
		return norm * efactor;
	return 0;
}
double nsl_fit_model_chi_square_param_deriv(int param, double x, double n, double a, double sigma) {
	double y=n/2., norm = pow(x, y-1.)/pow(2., y)/gamma(y)/sigma, efactor = exp(-x/2.);

	if (param == 0)
		return a/2. * norm * (log(x/2.) - gsl_sf_psi(y)) * efactor;
	if (param == 1)
		return norm * efactor;
	return 0;
}
double nsl_fit_model_weibull_param_deriv(int param, double x, double k, double l, double mu, double a, double sigma) {
	double y = (x-mu)/l, z = pow(y, k), efactor = exp(-z);

	if (param == 0)
		return a/sigma * z*(k*log(y)*(z-1.) - 1.) * efactor;
	if (param == 1)
		return a/sigma * k*k*z*(z-1.) * efactor;
	if (param == 2)
		return a/sigma * k*z/y*(k-1. - k*z) * efactor;
	if (param == 3)
		return k/l/sigma * z/y * efactor;
	return 0;
}
double nsl_fit_model_frechet_param_deriv(int param, double x, double a, double mu, double s, double c, double sigma) {
	double y = (x-mu)/s, efactor = exp(-pow(y, -a));

	if (param == 0)
		return c/s * pow(y, -2.*a-1.) * (a*log(y)*(1.-pow(y, a))+pow(y, a)) * efactor;
	if (param == 1)
		return c * a/(s*s)*pow(y, -a-2.) * (a+1-a*pow(y, -a)) * efactor;
	if (param == 2)
		return c * pow(a/s, 2.)*pow(y, -2.*a-1.) * (pow(y, a)-1) * efactor;
	if (param == 3)
		return a/sigma/s * pow(y, -a-1) * efactor;
	return 0;
}
double nsl_fit_model_gumbel_param_deriv(int param, double x, double b, double mu, double a, double sigma) {
	double norm = 1./b/sigma, y = (x-mu)/b, efactor = exp(y - exp(y));

	if (param == 0)
		return a * norm/b * (y*exp(y) - y - 1) * efactor;
	if (param == 1)
		return a * norm/b * (exp(y) - 1) * efactor;
	if (param == 2)
		return norm * efactor;
	return 0;
}
