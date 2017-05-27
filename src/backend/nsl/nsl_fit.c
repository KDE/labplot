/***************************************************************************
    File                 : nsl_fit.c
    Project              : LabPlot
    Description          : NSL (non)linear fit functions
    --------------------------------------------------------------------
    Copyright            : (C) 2016-2017 by Stefan Gerlach (stefan.gerlach@uni.kn)
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

#include "nsl_fit.h"
#include "nsl_common.h"
#include <gsl/gsl_math.h>
#include <gsl/gsl_sf_erf.h>
#include <gsl/gsl_sf_gamma.h>
#include <gsl/gsl_sf_psi.h>

const char* nsl_fit_model_category_name[] = {i18n("Basic functions"), i18n("Peak functions"), i18n("Growth (sigmoidal)"), i18n("Statistics (distributions)"),
	i18n("Custom")};

const char* nsl_fit_model_basic_name[] = {i18n("Polynomial"), i18n("Power"), i18n("Exponential"), i18n("Inverse exponential"), i18n("Fourier")};
const char* nsl_fit_model_basic_equation[] = {"c0 + c1*x", "a*x^b", "a*exp(b*x)", "a*(1-exp(b*x)) + c", "a0 + (a1*cos(w*x) + b1*sin(w*x))"};
const char* nsl_fit_model_basic_pic_name[] = {"polynom", "power", "exponential", "inv_exponential", "fourier"};

const char* nsl_fit_model_peak_name[] = {i18n("Gaussian (normal)"), i18n("Cauchy-Lorentz"), i18n("Hyperbolic secant (sech)"), i18n("Logistic (sech squared)")};
const char* nsl_fit_model_peak_equation[] = {"a/sqrt(2*pi)/s * exp(-((x-mu)/s)^2/2)", "a/pi * g/(g^2+(x-mu)^2)", "a/pi/s * sech((x-mu)/s)",
	"a/4/s * sech((x-mu)/2/s)**2"};
const char* nsl_fit_model_peak_pic_name[] = {"gaussian", "cauchy_lorentz", "sech", "logistic"};

const char* nsl_fit_model_growth_name[] = {i18n("Inverse tangent"), i18n("Hyperbolic tangent"), i18n("Algebraic sigmoid"), i18n("Logistic function"), 
	i18n("Error function (erf)"), i18n("Hill"), i18n("Gompertz"), i18n("Gudermann (gd)")};
const char* nsl_fit_model_growth_equation[] = {"a * atan((x-mu)/s)", "a * tanh((x-mu)/s)", "a * (x-mu)/s/sqrt(1+((x-mu)/s)^2)", "a/(1+exp(-k*(x-mu)))",
	"a/2 * erf((x-mu)/s/sqrt(2))", "a * x^n/(s^n + x^n)", "a*exp(-b*exp(-c*x))", "a * asin(tanh((x-mu)/s))"};
const char* nsl_fit_model_growth_pic_name[] = {"atan", "tanh", "alg_sigmoid", "logistic_function", "erf", "hill", "gompertz", "gd"};

const char* nsl_fit_weight_type_name[] = {"No", "Instrumental", "Direct", "Inverse", "Statistical", "Statistical (Fit)", "Relative", "Relative (Fit)"};

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
		return max + 1. - sqrt(x*x + 1.);
	if (max == DBL_MAX)
		return min - 1. + sqrt(x*x + 1.);

	return min + (max - min)/(1. + exp(-x));
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
		return sqrt(gsl_pow_2(max - x + 1.) - 1.);
	if (max == DBL_MAX)
		return sqrt(gsl_pow_2(x - min + 1.) - 1.);

	return -log((max - x)/(x - min));
}

/********************** parameter derivatives ******************/

/* basic */
double nsl_fit_model_polynomial_param_deriv(double x, int j, double weight) {
	return weight*pow(x, j);	
}
double nsl_fit_model_power1_param_deriv(int param, double x, double a, double b, double weight) {
	if (param == 0)
		return weight*pow(x, b);
	if (param == 1)
		return weight*a*pow(x, b)*log(x);
	return 0;
}
double nsl_fit_model_power2_param_deriv(int param, double x, double b, double c, double weight) {
	if (param == 0)
		return weight;
	if (param == 1)
		return weight*pow(x, c);
	if (param == 2)
		return weight*b*pow(x, c)*log(x);
	return 0;
}
double nsl_fit_model_exponential1_param_deriv(int param, double x, double a, double b, double weight) {
	if (param == 0)
		return weight*exp(b*x);
	if (param == 1)
		return weight*a*x*exp(b*x);
	return 0;
}
double nsl_fit_model_exponential2_param_deriv(int param, double x, double a, double b, double c, double d, double weight) {
	if (param == 0)
		return weight*exp(b*x);
	if (param == 1)
		return weight*a*x*exp(b*x);
	if (param == 2)
		return weight*exp(d*x);
	if (param == 3)
		return weight*c*x*exp(d*x);
	return 0;
}
double nsl_fit_model_exponential3_param_deriv(int param, double x, double a, double b, double c, double d, double e, double f, double weight) {
	if (param == 0)
		return weight*exp(b*x);
	if (param == 1)
		return weight*a*x*exp(b*x);
	if (param == 2)
		return weight*exp(d*x);
	if (param == 3)
		return weight*c*x*exp(d*x);
	if (param == 4)
		return weight*exp(f*x);
	if (param == 5)
		return weight*e*x*exp(f*x);
	return 0;
}
double nsl_fit_model_inverse_exponential_param_deriv(int param, double x, double a, double b, double weight) {
	if (param == 0)
		return weight*(1. - exp(b*x));
	if (param == 1)
		return -weight*a*x*exp(b*x);
	if (param == 2)
		return weight;
	return 0;
}
double nsl_fit_model_fourier_param_deriv(int param, int degree, double x, double w, double weight) {
	if (param == 0)
		return weight*cos(degree*w*x);
	if (param == 1)
		return weight*sin(degree*w*x);
	return 0;
}

/* peak */
double nsl_fit_model_gaussian_param_deriv(int param, double x, double s, double mu, double a, double weight) {
	double s2 = s*s, norm = weight/sqrt(2.*M_PI)/s, efactor = exp(-(x-mu)*(x-mu)/(2.*s2));

	if (param == 0)
		return a * norm/(s*s2) * ((x-mu)*(x-mu) - s2) * efactor;
	if (param == 1)
		return a * norm/s2 * (x-mu) * efactor;
	if (param == 2)
		return norm * efactor;
	return 0;
}
double nsl_fit_model_lorentz_param_deriv(int param, double x, double s, double t, double a, double weight) {
	double norm = weight/M_PI, denom = s*s+(x-t)*(x-t);

	if (param == 0)
		return a * norm * ((x-t)*(x-t) - s*s)/(denom*denom);
	if (param == 1)
		return a * norm * 2.*s*(x-t)/(denom*denom);
	if (param == 2)
		return norm * s/denom;
	return 0;
}
double nsl_fit_model_sech_param_deriv(int param, double x, double s, double mu, double a, double weight) {
	double y = (x-mu)/s, norm = weight/M_PI/s;

	if (param == 0)
		return a/s * norm * (y*tanh(y)-1.)/cosh(y);
	if (param == 1)
		return a/s * norm * tanh(y)/cosh(y);
	if (param == 2)
		return norm/cosh(y);
	return 0;
}
double nsl_fit_model_logistic_param_deriv(int param, double x, double s, double mu, double a, double weight) {
	double y = (x-mu)/2./s, norm = weight/4./s;

	if (param == 0)
		return a/s * norm * (2.*y*tanh(y)-1.)/cosh(y);
	if (param == 1)
		return a/s * norm * tanh(y)/cosh(y)/cosh(y);
	if (param == 2)
		return norm/cosh(y)/cosh(y);
	return 0;
}

/* growth */
double nsl_fit_model_atan_param_deriv(int param, double x, double s, double mu, double a, double weight) {
	double norm = weight, y = (x-mu)/s;
	if (param == 0)
		return -a/s * norm * y/(1.+y*y);
	if (param == 1)
		return -a/s * norm * 1./(1+y*y);
	if (param == 2)
		return norm * atan(y);
	return 0;
}
double nsl_fit_model_tanh_param_deriv(int param, double x, double s, double mu, double a, double weight) {
	double norm = weight, y = (x-mu)/s;
	if (param == 0)
		return -a/s * norm * y/cosh(y)/cosh(y);
	if (param == 1)
		return -a/s * norm * 1./cosh(y)/cosh(y);
	if (param == 2)
		return norm * tanh(y);
	return 0;
}
double nsl_fit_model_algebraic_sigmoid_param_deriv(int param, double x, double s, double mu, double a, double weight) {
	double norm = weight, y = (x-mu)/s, y2 = y*y;
	if (param == 0)
		return -a/s * norm * y/pow(1.+y2, 1.5);
	if (param == 1)
		return -a/s * norm * 1./pow(1.+y2, 1.5);
	if (param == 2)
		return norm * y/sqrt(1.+y2);
	return 0;
}
double nsl_fit_model_sigmoid_param_deriv(int param, double x, double k, double mu, double a, double weight) {
	double norm = weight, y = k*(x-mu);
	if (param == 0)
		return a/k * norm * y*exp(-y)/gsl_pow_2(1. + exp(-y));
	if (param == 1)
		return -a*k * norm * exp(-y)/gsl_pow_2(1. + exp(-y));
	if (param == 2)
		return norm/(1. + exp(-y));
	return 0;
}
double nsl_fit_model_erf_param_deriv(int param, double x, double s, double mu, double a, double weight) {
	double norm = weight, y = (x-mu)/(sqrt(2.)*s);
	if (param == 0)
		return -a/sqrt(M_PI)/s * norm * y*exp(-y*y);
	if (param == 1)
		return -a/sqrt(2.*M_PI)/s * norm * exp(-y*y);
	if (param == 2)
		return norm/2. * gsl_sf_erf(y);
	return 0;
}
double nsl_fit_model_hill_param_deriv(int param, double x, double s, double n, double a, double weight) {
	double norm = weight, y = x/s;
	if (param == 0)
		return -a*n/s * norm * pow(y, n)/gsl_pow_2(1.+pow(y, n));
	if (param == 1)
		return a * norm * log(y)*pow(y, n)/gsl_pow_2(1.+pow(y, n));
	if (param == 2)
		return norm * pow(y, n)/(1.+pow(y, n));
	return 0;
}
double nsl_fit_model_gompertz_param_deriv(int param, double x, double a, double b, double c, double weight) {
	if (param == 0)
		return weight*exp(-b*exp(-c*x));
	if (param == 1)
		return -weight*a*exp(-c*x-b*exp(-c*x));
	if (param == 2)
		return weight*a*b*x*exp(-c*x-b*exp(-c*x));
	return 0;
}
double nsl_fit_model_gudermann_param_deriv(int param, double x, double s, double mu, double a, double weight) {
	double norm = weight, y = (x-mu)/s;
	if (param == 0)
		return -a/s * norm * y/cosh(y);
	if (param == 1)
		return -a/s * norm * 1./cosh(y);
	if (param == 2)
		return -asin(tanh(y));
	return 0;
}

/* distributions */
double nsl_fit_model_gaussian_tail_param_deriv(int param, double x, double s, double mu, double A, double a, double weight) {
	if (x < a)
		return 0;
	double s2 = s*s, N = erfc(a/s/M_SQRT2)/2., norm = weight/sqrt(2.*M_PI)/s/N, efactor = exp(-(x-mu)*(x-mu)/(2.*s2));

	if (param == 0)
		return A * norm/(s*s2) * ((x-mu)*(x-mu) - s2) * efactor;
	if (param == 1)
		return A * norm/s2 * (x-mu) * efactor;
	if (param == 2)
		return norm * efactor;
	if (param == 3)
		return A/norm/norm * efactor * exp(-a*a/(2.*s2));
	return 0;
}
double nsl_fit_model_exponential_param_deriv(int param, double x, double l, double mu, double a, double weight) {
	if (x < mu)
		return 0;
	double y = l*(x-mu), efactor = exp(-y);

	if (param == 0)
		return weight * a * (1. - y) * efactor;
	if (param == 1)
		return weight * a * gsl_pow_2(l) * efactor;
	if (param == 2)
		return weight * l * efactor;
	return 0;
}
double nsl_fit_model_laplace_param_deriv(int param, double x, double s, double mu, double a, double weight) {
	double norm = weight/(2.*s), y = fabs((x-mu)/s), efactor = exp(-y);

	if (param == 0)
		return a/s*norm * (y-1.) * efactor;
	if (param == 1)
		return a/(s*s)*norm * (x-mu)/y * efactor;
	if (param == 2)
		return norm * efactor;
	return 0;
}
double nsl_fit_model_exp_pow_param_deriv(int param, double x, double s, double mu, double b, double a, double weight) {
	double norm = weight/2./s/gsl_sf_gamma(1.+1./b), y = (x-mu)/s, efactor = exp(-pow(fabs(y), b));

	if (param == 0)
		return norm * a/s * efactor * (b * y * pow(fabs(1./y), 1.-b) * GSL_SIGN(y) - 1.);
	if (param == 1)
		return norm * a*b/s * efactor * pow(fabs(y), b-1.) * GSL_SIGN(y);
	if (param == 2)
		return norm * a/b * gsl_sf_gamma(1.+1./b)/gsl_sf_gamma(1./b) * efactor * (gsl_sf_psi(1.+1./b) - gsl_pow_2(b) * pow(fabs(y), b) * log(fabs(y)));
	if (param == 3)
		return norm * efactor;
	return 0;
}
double nsl_fit_model_maxwell_param_deriv(int param, double x, double a, double c, double weight) {
	double a2 = a*a, a3 = a*a2, norm = weight*sqrt(2./M_PI)/a3, x2 = x*x, efactor = exp(-x2/2./a2);

	if (param == 0)
		return c * norm * x2*(x2-3.*a2)/a3 * efactor;
	if (param == 1)
		return norm * x2 * efactor;
	return 0;
}
double nsl_fit_model_poisson_param_deriv(int param, double x, double l, double a, double weight) {
	double norm = weight*pow(l, x)/gsl_sf_gamma(x+1.);

	if (param == 0)
		return a/l * norm *(x-l)*exp(-l);
	if (param == 1)
		return norm * exp(-l);
	return 0;
}
double nsl_fit_model_lognormal_param_deriv(int param, double x, double s, double mu, double a, double weight) {
	double norm = weight/sqrt(2.*M_PI)/(x*s), y = log(x)-mu, efactor = exp(-(y/s)*(y/s)/2.);

	if (param == 0)
		return a * norm * (y*y - s*s) * efactor;
	if (param == 1)
		return a * norm * y/(s*s) * efactor;
	if (param == 2)
		return norm * efactor;
	return 0;
}
double nsl_fit_model_gamma_param_deriv(int param, double x, double t, double k, double a, double weight) {
	double factor = weight*pow(x, k-1.)/pow(t, k)/gsl_sf_gamma(k), efactor = exp(-x/t);

	if (param == 0)
		return a * factor/t * (x/t-k) * efactor;
	if (param == 1)
		return a * factor * (log(x/t) - gsl_sf_psi(k)) * efactor;
	if (param == 2)
		return factor * efactor;
	return 0;
}
double nsl_fit_model_rayleigh_param_deriv(int param, double x, double s, double a, double weight) {
	double y=x/s, norm = weight*y/s, efactor = exp(-y*y/2.);

	if (param == 0)
		return a*y/(s*s) * (y*y-2.)*efactor;
	if (param == 1)
		return norm * efactor;
	return 0;
}
double nsl_fit_model_levy_param_deriv(int param, double x, double g, double mu, double a, double weight) {
	double y=x-mu, norm = weight*sqrt(g/(2.*M_PI))/pow(y, 1.5), efactor = exp(-g/2./y);

	if (param == 0)
		return a/2.*norm/g/y * (y - g) * efactor;
	if (param == 1)
		return a/2.*norm/y/y * (3.*y - g) * efactor;
	if (param == 2)
		return norm * efactor;
	return 0;
}
double nsl_fit_model_chi_square_param_deriv(int param, double x, double n, double a, double weight) {
	double y=n/2., norm = weight*pow(x, y-1.)/pow(2., y)/gsl_sf_gamma(y), efactor = exp(-x/2.);

	if (param == 0)
		return a/2. * norm * (log(x/2.) - gsl_sf_psi(y)) * efactor;
	if (param == 1)
		return norm * efactor;
	return 0;
}
double nsl_fit_model_weibull_param_deriv(int param, double x, double k, double l, double mu, double a, double weight) {
	double y = (x-mu)/l, z = pow(y, k), efactor = exp(-z);

	if (param == 0)
		return weight*a/l * z/y*(k*log(y)*(1.-z) + 1.) * efactor;
	if (param == 1)
		return weight*a*k*k/l/l * z/y*(z-1.) * efactor;
	if (param == 2)
		return weight*a*k/l/l * z/y/y*(k*z + 1. - k) * efactor;
	if (param == 3)
		return weight*k/l * z/y * efactor;
	return 0;
}
double nsl_fit_model_frechet_param_deriv(int param, double x, double g, double mu, double s, double a, double weight) {
	double y = (x-mu)/s, efactor = exp(-pow(y, -g));

	if (param == 0)
		return weight*a/s * pow(y, -2.*g-1.) * (g*log(y)*(1.-pow(y, g))+pow(y, g)) * efactor;
	if (param == 1)
		return a*weight * g/(s*s)*pow(y, -g-2.) * (g+1.-g*pow(y, -g)) * efactor;
	if (param == 2)
		return a*weight * gsl_pow_2(g/s)*pow(y, -2.*g-1.) * (pow(y, g)-1.) * efactor;
	if (param == 3)
		return g*weight/s * pow(y, -g-1.) * efactor;
	return 0;
}
double nsl_fit_model_gumbel1_param_deriv(int param, double x, double s, double b, double mu, double a, double weight) {
	double norm = weight/s, y = (x-mu)/s, efactor = exp(-y - b*exp(-y));

	if (param == 0)
		return a/s * norm * (y - 1. - b*exp(-y)) * efactor;
	if (param == 1)
		return -a * norm * exp(-y) * efactor;
	if (param == 2)
		return a/s * norm * (1. - b*exp(-y)) * efactor;
	if (param == 3)
		return norm * efactor;
	return 0;
}
double nsl_fit_model_sech_dist_param_deriv(int param, double x, double s, double mu, double a, double weight) {
	double norm = weight/2./s, y = M_PI/2.*(x-mu)/s;

	if (param == 0)
		return -a/s * norm * (y*tanh(y)+1.)/cosh(y);
	if (param == 1)
		return a*M_PI/2./s * norm * tanh(y)/cosh(y);
	if (param == 2)
		return norm * 1./cosh(y);
	return 0;
}
