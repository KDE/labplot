/***************************************************************************
    File                 : nsl_fit.c
    Project              : LabPlot
    Description          : NSL (non)linear fit functions
    --------------------------------------------------------------------
    Copyright            : (C) 2016-2018 by Stefan Gerlach (stefan.gerlach@uni.kn)
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
#include "nsl_sf_basic.h"
#include <gsl/gsl_math.h>
#include <gsl/gsl_sf_erf.h>
#include <gsl/gsl_sf_gamma.h>
#include <gsl/gsl_sf_psi.h>
#include <gsl/gsl_randist.h>

const char* nsl_fit_model_category_name[] = {i18n("Basic functions"), i18n("Peak functions"), i18n("Growth (sigmoidal)"), i18n("Statistics (distributions)"),
	i18n("Custom")};

const char* nsl_fit_model_basic_name[] = {i18n("Polynomial"), i18n("Power"), i18n("Exponential"), i18n("Inverse exponential"), i18n("Fourier")};
const char* nsl_fit_model_basic_equation[] = {"c0 + c1*x", "a*x^b", "a*exp(b*x)", "a*(1-exp(b*x)) + c", "a0 + (a1*cos(w*x) + b1*sin(w*x))"};
const char* nsl_fit_model_basic_pic_name[] = {"polynom", "power", "exponential", "inv_exponential", "fourier"};

const char* nsl_fit_model_peak_name[] = {i18n("Gaussian (normal)"), i18n("Cauchy-Lorentz"), i18n("Hyperbolic secant (sech)"), i18n("Logistic (sech squared)"),
	i18n("Voigt profile"), i18n("Pseudo-Voigt (same width)")};
const char* nsl_fit_model_peak_equation[] = {"a/sqrt(2*pi)/s * exp(-((x-mu)/s)^2/2)", "a/pi * g/(g^2+(x-mu)^2)", "a/pi/s * sech((x-mu)/s)",
	"a/4/s * sech((x-mu)/2/s)**2", "a*voigt(x - mu, s, g)", "a*pseudovoigt1(x - mu, et, w)"};	// eta is already used as function
const char* nsl_fit_model_peak_pic_name[] = {"gaussian", "cauchy_lorentz", "sech", "logistic", "voigt", "pseudovoigt1"};

const char* nsl_fit_model_growth_name[] = {i18n("Inverse tangent"), i18n("Hyperbolic tangent"), i18n("Algebraic sigmoid"), i18n("Logistic function"),
	i18n("Error function (erf)"), i18n("Hill"), i18n("Gompertz"), i18n("Gudermann (gd)")};
const char* nsl_fit_model_growth_equation[] = {"a * atan((x-mu)/s)", "a * tanh((x-mu)/s)", "a * (x-mu)/s/sqrt(1+((x-mu)/s)^2)", "a/(1+exp(-k*(x-mu)))",
	"a/2 * erf((x-mu)/s/sqrt(2))", "a * x^n/(s^n + x^n)", "a*exp(-b*exp(-c*x))", "a * asin(tanh((x-mu)/s))"};
const char* nsl_fit_model_growth_pic_name[] = {"atan", "tanh", "alg_sigmoid", "logistic_function", "erf", "hill", "gompertz", "gd"};

const char* nsl_fit_weight_type_name[] = {"No", "Instrumental (1/col^2)", "Direct (col)", "Inverse (1/col)", "Statistical (1/data)", "Statistical (Fit)", "Relative (1/data^2)", "Relative (Fit)"};

/*
	see https://seal.web.cern.ch/seal/documents/minuit/mnusersguide.pdf
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

	return min + (sin(x) + 1.) * (max - min)/2.;

	/* alternative transformation for closed bounds
	return min + (max - min)/(1. + exp(-x));
	*/
}

/*
	see https://seal.web.cern.ch/seal/documents/minuit/mnusersguide.pdf
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

	return asin(2. * (x - min)/(max - min) - 1.);

	/* alternative transformation for closed bounds
	return -log((max - x)/(x - min));
	*/
}

/********************** parameter derivatives ******************/

/* basic */
double nsl_fit_model_polynomial_param_deriv(double x, int j, double weight) {
	return sqrt(weight)*pow(x, j);
}
double nsl_fit_model_power1_param_deriv(unsigned int param, double x, double a, double b, double weight) {
	if (param == 0)
		return sqrt(weight)*pow(x, b);
	if (param == 1)
		return sqrt(weight)*a*pow(x, b)*log(x);

	return 0;
}
double nsl_fit_model_power2_param_deriv(unsigned int param, double x, double b, double c, double weight) {
	if (param == 0)
		return sqrt(weight);
	if (param == 1)
		return sqrt(weight)*pow(x, c);
	if (param == 2)
		return sqrt(weight)*b*pow(x, c)*log(x);

	return 0;
}
double nsl_fit_model_exponentialn_param_deriv(unsigned int param, double x, double *p, double weight) {
	if (param % 2 == 0)
		return sqrt(weight)*exp(p[param+1]*x);
	else
		return sqrt(weight)*p[param-1]*x*exp(p[param]*x);
}
double nsl_fit_model_inverse_exponential_param_deriv(unsigned int param, double x, double a, double b, double weight) {
	if (param == 0)
		return sqrt(weight)*(1. - exp(b*x));
	if (param == 1)
		return -sqrt(weight)*a*x*exp(b*x);
	if (param == 2)
		return sqrt(weight);

	return 0;
}
double nsl_fit_model_fourier_param_deriv(unsigned int param, unsigned int degree, double x, double w, double weight) {
	if (param == 0)
		return sqrt(weight)*cos(degree*w*x);
	if (param == 1)
		return sqrt(weight)*sin(degree*w*x);

	return 0;
}

/* peak */
double nsl_fit_model_gaussian_param_deriv(unsigned int param, double x, double A, double s, double mu, double weight) {
	double s2 = s*s, norm = sqrt(weight)/M_SQRT2/M_SQRTPI/s, efactor = exp(-(x-mu)*(x-mu)/(2.*s2));

	if (param == 0)
		return norm * efactor;
	if (param == 1)
		return A * norm/(s*s2) * ((x-mu)*(x-mu) - s2) * efactor;
	if (param == 2)
		return A * norm/s2 * (x-mu) * efactor;

	return 0;
}
double nsl_fit_model_lorentz_param_deriv(unsigned int param, double x, double A, double s, double t, double weight) {
	double norm = sqrt(weight)/M_PI, denom = s*s+(x-t)*(x-t);

	if (param == 0)
		return norm * s/denom;
	if (param == 1)
		return A * norm * ((x-t)*(x-t) - s*s)/(denom*denom);
	if (param == 2)
		return A * norm * 2.*s*(x-t)/(denom*denom);

	return 0;
}
double nsl_fit_model_sech_param_deriv(unsigned int param, double x, double A, double s, double mu, double weight) {
	double y = (x-mu)/s, norm = sqrt(weight)/M_PI/s;

	if (param == 0)
		return norm/cosh(y);
	if (param == 1)
		return A/s * norm * (y*tanh(y)-1.)/cosh(y);
	if (param == 2)
		return A/s * norm * tanh(y)/cosh(y);

	return 0;
}
double nsl_fit_model_logistic_param_deriv(unsigned int param, double x, double A, double s, double mu, double weight) {
	double y = (x-mu)/2./s, norm = sqrt(weight)/4./s;

	if (param == 0)
		return norm/cosh(y)/cosh(y);
	if (param == 1)
		return A/s * norm * (2.*y*tanh(y)-1.)/cosh(y);
	if (param == 2)
		return A/s * norm * tanh(y)/cosh(y)/cosh(y);

	return 0;
}

double nsl_fit_model_voigt_param_deriv(unsigned int param, double x, double a, double mu, double s, double g, double weight) {
#if !defined(_MSC_VER)
	if (s <= 0 || g < 0)
		return 0;

	double y = x - mu, norm = a * sqrt(weight/2./M_PI)/(s*s*s);

	double v = nsl_sf_voigt(y, s, g), im_w = nsl_sf_im_w_of_z(y);
	if (param == 0)
		return sqrt(weight) * v;
	if (param == 1)
		return a*sqrt(weight)*y/(s*s) * v - norm * g * im_w;
	if (param == 2)
//		return a*sqrt(weight)*g/M_PI/(s*s*s) + norm*sqrt(2.*M_PI)*v * (y*y+mu*mu-s*s) - norm/s*im_w*2.*g*y;
		return a/(s*s*s)*sqrt(weight)*(g/M_PI + v*(y*y -g*g -s*s) + im_w*2.*g*y/s);
	if (param == 3)
		return -a*sqrt(weight)/M_PI/(s*s) + norm*M_SQRT2*M_SQRTPI*s*v*g + im_w;
#endif

	return 0;
}

double nsl_fit_model_pseudovoigt1_param_deriv(unsigned int param, double x, double a, double eta, double w, double mu, double weight) {
	double y = x - mu, norm = sqrt(weight);
	double sigma = w/sqrt(2.*M_LN2);

	if (param == 0)
		return norm * nsl_sf_pseudovoigt1(y, eta, w);
	if (param == 1)
		return a * norm * (gsl_ran_cauchy_pdf(y, w) - gsl_ran_gaussian_pdf(y, sigma));
	if (param == 2)
		return a/w * norm * (eta*(1.-2.*w*w)*gsl_ran_cauchy_pdf(y, w) + (eta-1.)*gsl_ran_gaussian_pdf(y, sigma)*(1.-2.*M_LN2*y*y/w/w));
	if (param == 3)
		return 2.*a*y/w/w * norm * (eta*M_PI*w*gsl_pow_2(gsl_ran_cauchy_pdf(y, w)) + (1.-eta)*M_LN2*gsl_ran_gaussian_pdf(y, sigma));

	return 0;
}

/* growth */
double nsl_fit_model_atan_param_deriv(unsigned int param, double x, double A, double mu, double s, double weight) {
	double norm = sqrt(weight), y = (x-mu)/s;
	if (param == 0)
		return norm * atan(y);
	if (param == 1)
		return -A/s * norm * 1./(1+y*y);
	if (param == 2)
		return -A/s * norm * y/(1.+y*y);

	return 0;
}
double nsl_fit_model_tanh_param_deriv(unsigned int param, double x, double A, double mu, double s, double weight) {
	double norm = sqrt(weight), y = (x-mu)/s;
	if (param == 0)
		return norm * tanh(y);
	if (param == 1)
		return -A/s * norm * 1./cosh(y)/cosh(y);
	if (param == 2)
		return -A/s * norm * y/cosh(y)/cosh(y);

	return 0;
}
double nsl_fit_model_algebraic_sigmoid_param_deriv(unsigned int param, double x, double A, double mu, double s, double weight) {
	double norm = sqrt(weight), y = (x-mu)/s, y2 = y*y;
	if (param == 0)
		return norm * y/sqrt(1.+y2);
	if (param == 1)
		return -A/s * norm * 1./pow(1.+y2, 1.5);
	if (param == 2)
		return -A/s * norm * y/pow(1.+y2, 1.5);

	return 0;
}
double nsl_fit_model_sigmoid_param_deriv(unsigned int param, double x, double A, double mu, double k, double weight) {
	double norm = sqrt(weight), y = k*(x-mu);
	if (param == 0)
		return norm/(1. + exp(-y));
	if (param == 1)
		return -A*k * norm * exp(-y)/gsl_pow_2(1. + exp(-y));
	if (param == 2)
		return A/k * norm * y*exp(-y)/gsl_pow_2(1. + exp(-y));

	return 0;
}
double nsl_fit_model_erf_param_deriv(unsigned int param, double x, double A, double mu, double s, double weight) {
	double norm = sqrt(weight), y = (x-mu)/(sqrt(2.)*s);
	if (param == 0)
		return norm/2. * gsl_sf_erf(y);
	if (param == 1)
		return -A/M_SQRT2/M_SQRTPI/s * norm * exp(-y*y);
	if (param == 2)
		return -A/M_SQRTPI/s * norm * y*exp(-y*y);

	return 0;
}
double nsl_fit_model_hill_param_deriv(unsigned int param, double x, double A, double n, double s, double weight) {
	double norm = sqrt(weight), y = x/s;
	if (param == 0)
		return norm * pow(y, n)/(1.+pow(y, n));
	if (param == 1)
		return A * norm * log(y)*pow(y, n)/gsl_pow_2(1.+pow(y, n));
	if (param == 2)
		return -A*n/s * norm * pow(y, n)/gsl_pow_2(1.+pow(y, n));

	return 0;
}
double nsl_fit_model_gompertz_param_deriv(unsigned int param, double x, double a, double b, double c, double weight) {
	if (param == 0)
		return sqrt(weight)*exp(-b*exp(-c*x));
	if (param == 1)
		return -sqrt(weight)*a*exp(-c*x-b*exp(-c*x));
	if (param == 2)
		return sqrt(weight)*a*b*x*exp(-c*x-b*exp(-c*x));

	return 0;
}
double nsl_fit_model_gudermann_param_deriv(unsigned int param, double x, double A, double mu, double s, double weight) {
	double norm = sqrt(weight), y = (x-mu)/s;
	if (param == 0)
		return -asin(tanh(y));
	if (param == 1)
		return -A/s * norm * 1./cosh(y);
	if (param == 2)
		return -A/s * norm * y/cosh(y);

	return 0;
}

/* distributions */
double nsl_fit_model_gaussian_tail_param_deriv(unsigned int param, double x, double A, double s, double a, double mu, double weight) {
	if (x < a)
		return 0;
	double s2 = s*s, N = erfc(a/s/M_SQRT2)/2., norm = sqrt(weight)/M_SQRT2/M_SQRTPI/s/N, efactor = exp(-(x-mu)*(x-mu)/(2.*s2));

	if (param == 0)
		return norm * efactor;
	if (param == 1)
		return A * norm/(s*s2) * ((x-mu)*(x-mu) - s2) * efactor;
	if (param == 2)
		return A/norm/norm * efactor * exp(-a*a/(2.*s2));
	if (param == 3)
		return A * norm/s2 * (x-mu) * efactor;

	return 0;
}
double nsl_fit_model_exponential_param_deriv(unsigned int param, double x, double A, double l, double mu, double weight) {
	if (x < mu)
		return 0;
	double y = l*(x-mu), efactor = exp(-y);

	if (param == 0)
		return sqrt(weight) * l * efactor;
	if (param == 1)
		return sqrt(weight) * A * (1. - y) * efactor;
	if (param == 2)
		return sqrt(weight) * A * gsl_pow_2(l) * efactor;

	return 0;
}
double nsl_fit_model_laplace_param_deriv(unsigned int param, double x, double A, double s, double mu, double weight) {
	double norm = sqrt(weight)/(2.*s), y = fabs((x-mu)/s), efactor = exp(-y);

	if (param == 0)
		return norm * efactor;
	if (param == 1)
		return A/s*norm * (y-1.) * efactor;
	if (param == 2)
		return A/(s*s)*norm * (x-mu)/y * efactor;

	return 0;
}
double nsl_fit_model_exp_pow_param_deriv(unsigned int param, double x, double a, double s, double b, double mu, double weight) {
	double norm = sqrt(weight)/2./s/gsl_sf_gamma(1.+1./b), y = (x-mu)/s, efactor = exp(-pow(fabs(y), b));

	if (param == 0)
		return norm * efactor;
	if (param == 1)
		return norm * a/s * efactor * (b * y * pow(fabs(1./y), 1.-b) * GSL_SIGN(y) - 1.);
	if (param == 2)
		return norm * a/b * gsl_sf_gamma(1.+1./b)/gsl_sf_gamma(1./b) * efactor * (gsl_sf_psi(1.+1./b) - gsl_pow_2(b) * pow(fabs(y), b) * log(fabs(y)));
	if (param == 3)
		return norm * a*b/s * efactor * pow(fabs(y), b-1.) * GSL_SIGN(y);

	return 0;
}
double nsl_fit_model_maxwell_param_deriv(unsigned int param, double x, double a, double s, double weight) {
	double s2 = s*s, s3 = s*s2, norm = sqrt(weight)*M_SQRT2/M_SQRTPI/s3, x2 = x*x, efactor = exp(-x2/2./s2);

	if (param == 0)
		return norm * x2 * efactor;
	if (param == 1)
		return a * norm * x2*(x2-3.*s2)/s3 * efactor;

	return 0;
}
double nsl_fit_model_poisson_param_deriv(unsigned int param, double x, double A, double l, double weight) {
	double norm = sqrt(weight)*pow(l, x)/gsl_sf_gamma(x+1.);

	if (param == 0)
		return norm * exp(-l);
	if (param == 1)
		return A/l * norm *(x-l)*exp(-l);

	return 0;
}
double nsl_fit_model_lognormal_param_deriv(unsigned int param, double x, double A, double s, double mu, double weight) {
	double norm = sqrt(weight)/M_SQRT2/M_SQRTPI/(x*s), y = log(x)-mu, efactor = exp(-(y/s)*(y/s)/2.);

	if (param == 0)
		return norm * efactor;
	if (param == 1)
		return A * norm * (y*y - s*s) * efactor;
	if (param == 2)
		return A * norm * y/(s*s) * efactor;

	return 0;
}
double nsl_fit_model_gamma_param_deriv(unsigned int param, double x, double A, double k, double t, double weight) {
	double factor = sqrt(weight)*pow(x, k-1.)/pow(t, k)/gsl_sf_gamma(k), efactor = exp(-x/t);

	if (param == 0)
		return factor * efactor;
	if (param == 1)
		return A * factor * (log(x/t) - gsl_sf_psi(k)) * efactor;
	if (param == 2)
		return A * factor/t * (x/t-k) * efactor;

	return 0;
}
double nsl_fit_model_flat_param_deriv(unsigned int param, double x, double A, double b, double a, double weight) {
	if (x < a || x > b)
		return 0;

	if (param == 0)
		return sqrt(weight)/(b-a);
	if (param == 1)
		return - sqrt(weight) * A/gsl_pow_2(a-b);
	if (param == 2)
		return sqrt(weight) * A/gsl_pow_2(a-b);

	return 0;
}
double nsl_fit_model_rayleigh_param_deriv(unsigned int param, double x, double A, double s, double weight) {
	double y=x/s, norm = sqrt(weight)*y/s, efactor = exp(-y*y/2.);

	if (param == 0)
		return norm * efactor;
	if (param == 1)
		return A*y/(s*s) * (y*y-2.)*efactor;

	return 0;
}
double nsl_fit_model_rayleigh_tail_param_deriv(unsigned int param, double x, double A, double s, double mu, double weight) {
	double norm = sqrt(weight)*x/(s*s), y = (mu*mu - x*x)/2./(s*s);

	if (param == 0)
		return norm * exp(y);
	if (param == 1)
		return -2. * A * norm/s * (1. + y) * exp(y);
	if (param == 2)
		return A * mu * norm/(s*s) * exp(y);

	return 0;
}
double nsl_fit_model_levy_param_deriv(unsigned int param, double x, double A, double g, double mu, double weight) {
	double y=x-mu, norm = sqrt(weight)*sqrt(g/(2.*M_PI))/pow(y, 1.5), efactor = exp(-g/2./y);

	if (param == 0)
		return norm * efactor;
	if (param == 1)
		return A/2.*norm/g/y * (y - g) * efactor;
	if (param == 2)
		return A/2.*norm/y/y * (3.*y - g) * efactor;

	return 0;
}
double nsl_fit_model_landau_param_deriv(unsigned int param, double x, double weight) {
	if (param == 0)
		return sqrt(weight) * gsl_ran_landau_pdf(x);

	return 0;
}
double nsl_fit_model_chi_square_param_deriv(unsigned int param, double x, double A, double n, double weight) {
	double y=n/2., norm = sqrt(weight)*pow(x, y-1.)/pow(2., y)/gsl_sf_gamma(y), efactor = exp(-x/2.);

	if (param == 0)
		return norm * efactor;
	if (param == 1)
		return A/2. * norm * (log(x/2.) - gsl_sf_psi(y)) * efactor;

	return 0;
}
double nsl_fit_model_students_t_param_deriv(unsigned int param, double x, double A, double n, double weight) {
	if (param == 0)
		return sqrt(weight) * gsl_ran_tdist_pdf(x, n);
	if (param == 1)
		return sqrt(weight) * A * gsl_sf_gamma((n+1.)/2.)/2./pow(n, 1.5)/M_SQRTPI/gsl_sf_gamma(n/2.) * pow(1.+x*x/n, - (n+3.)/2.)
			* (x*x - 1. - (n+x*x)*log1p(x*x/n)  + (n+x*x)*(gsl_sf_psi((n+1.)/2.) - gsl_sf_psi(n/2.)) ) ;

	return 0;
}
double nsl_fit_model_fdist_param_deriv(unsigned int param, double x, double A, double n1, double n2, double weight) {
	double norm = sqrt(weight) * gsl_sf_gamma((n1+n2)/2.)/gsl_sf_gamma(n1/2.)/gsl_sf_gamma(n2/2.) * pow(n1, n1/2.) * pow(n2, n2/2.) * pow(x, n1/2.-1.);
	double y = n2+n1*x;

	if (param == 0)
		return sqrt(weight) * gsl_ran_fdist_pdf(x, n1, n2);
	if (param == 1)
		return A/2. * norm * pow(y, -(n1+n2+2.)/2.) * (n2*(1.-x) + y*(log(n1) + log(x) - log(y) + gsl_sf_psi((n1+n2)/2.) - gsl_sf_psi(n1/2.)));
	if (param == 2)
		return A/2. * norm * pow(y, -(n1+n2+2.)/2.) * (n1*(x-1.) + y*(log(n2) - log(y) + gsl_sf_psi((n1+n2)/2.) - gsl_sf_psi(n2/2.)));

	return 0;
}
double nsl_fit_model_beta_param_deriv(unsigned int param, double x, double A, double a, double b, double weight) {
	double norm = sqrt(weight) * A * gsl_sf_gamma(a+b)/gsl_sf_gamma(a)/gsl_sf_gamma(b) * pow(x, a-1.) * pow(1.-x, b-1.);

	if (param == 0)
		return sqrt(weight) * gsl_ran_beta_pdf(x, a, b);
	if (param == 1)
		return norm * (log(x) - gsl_sf_psi(a) + gsl_sf_psi(a+b));
	if (param == 2)
		return norm * (log(1.-x) - gsl_sf_psi(b) + gsl_sf_psi(a+b));

	return 0;
}
double nsl_fit_model_pareto_param_deriv(unsigned int param, double x, double A, double a, double b, double weight) {
	if (x < b)
		return 0;

	double norm = sqrt(weight) * A;
	if (param == 0)
		return sqrt(weight) * gsl_ran_pareto_pdf(x, a, b);
	if (param == 1)
		return norm * pow(b/x, a) * (1. + a * log(b/x))/x;
	if (param == 2)
		return norm * a*a * pow(b/x, a-1.)/x/x;

	return 0;
}
double nsl_fit_model_weibull_param_deriv(unsigned int param, double x, double A, double k, double l, double mu, double weight) {
	double y = (x-mu)/l, z = pow(y, k), efactor = exp(-z);

	if (param == 0)
		return sqrt(weight) * k/l * z/y * efactor;
	if (param == 1)
		return sqrt(weight) * A/l * z/y*(k*log(y)*(1.-z) + 1.) * efactor;
	if (param == 2)
		return sqrt(weight) * A*k*k/l/l * z/y*(z-1.) * efactor;
	if (param == 3)
		return sqrt(weight) * A*k/l/l * z/y/y*(k*z + 1. - k) * efactor;

	return 0;
}
double nsl_fit_model_frechet_param_deriv(unsigned int param, double x, double A, double g, double s, double mu, double weight) {
	double y = (x-mu)/s, efactor = exp(-pow(y, -g));

	if (param == 0)
		return g * sqrt(weight)/s * pow(y, -g-1.) * efactor;
	if (param == 1)
		return sqrt(weight) * A/s * pow(y, -2.*g-1.) * (g*log(y)*(1.-pow(y, g))+pow(y, g)) * efactor;
	if (param == 2)
		return A * sqrt(weight) * gsl_pow_2(g/s)*pow(y, -2.*g-1.) * (pow(y, g)-1.) * efactor;
	if (param == 3)
		return A * sqrt(weight) * g/(s*s)*pow(y, -g-2.) * (g+1.-g*pow(y, -g)) * efactor;

	return 0;
}
double nsl_fit_model_gumbel1_param_deriv(unsigned int param, double x, double A, double s, double mu, double b, double weight) {
	double norm = sqrt(weight)/s, y = (x-mu)/s, efactor = exp(-y - b*exp(-y));

	if (param == 0)
		return norm * efactor;
	if (param == 1)
		return A/s * norm * (y - 1. - b*exp(-y)) * efactor;
	if (param == 2)
		return A/s * norm * (1. - b*exp(-y)) * efactor;
	if (param == 3)
		return -A * norm * exp(-y) * efactor;

	return 0;
}
double nsl_fit_model_gumbel2_param_deriv(unsigned int param, double x, double A, double a, double b, double mu, double weight) {
	double y = x - mu, norm = A * sqrt(weight) * exp(-b * pow(y, -a));

	if (param == 0)
		return sqrt(weight) * gsl_ran_gumbel2_pdf(y, a, b);
	if (param == 1)
		return norm * b * pow(y, -1. -2.*a) * (pow(y, a) -a*(pow(y, a)-b)*log(y));
	if (param == 2)
		return norm * a * pow(y, -1. -2.*a) * (pow(y, a) - b);
	if (param == 3)
		return norm * a * b * pow(y, -2.*(a + 1.)) * ((1. + a)*pow(y, a) - a*b);

	return 0;
}
double nsl_fit_model_binomial_param_deriv(unsigned int param, double k, double A, double p, double n, double weight) {
	if (k < 0 || k > n || n < 0 || p < 0 || p > 1.)
		return 0;
	k = round(k);
	n = round(n);

	double norm = sqrt(weight) * gsl_sf_fact((unsigned int)n)/gsl_sf_fact((unsigned int)(n-k))/gsl_sf_fact((unsigned int)(k));
	if (param == 0)
		return sqrt(weight) * gsl_ran_binomial_pdf((unsigned int)k, p, (unsigned int)n);
	if (param == 1)
		return A * norm * pow(p, k-1.) * pow(1.-p, n-k-1.) * (k-n*p);
	if (param == 2)
		return A * norm * pow(p, k) * pow(1.-p, n-k) * (log(1.-p) + gsl_sf_psi(n+1.) - gsl_sf_psi(n-k+1.));

	return 0;
}
double nsl_fit_model_negative_binomial_param_deriv(unsigned int param, double k, double A, double p, double n, double weight) {
	if (k < 0 || k > n || n < 0 || p < 0 || p > 1.)
		return 0;

	double norm = A * sqrt(weight) * gsl_sf_gamma(n+k)/gsl_sf_gamma(k+1.)/gsl_sf_gamma(n);
	if (param == 0)
		return sqrt(weight) * gsl_ran_negative_binomial_pdf((unsigned int)k, p, n);
	if (param == 1)
		return - norm * pow(p, n-1.) * pow(1.-p, k-1.) * (n*(p-1.) + k*p);
	if (param == 2)
		return norm * pow(p, n) * pow(1.-p, k) * (log(p) - gsl_sf_psi(n) + gsl_sf_psi(n+k));

	return 0;
}
double nsl_fit_model_pascal_param_deriv(unsigned int param, double k, double A, double p, double n, double weight) {
	return nsl_fit_model_negative_binomial_param_deriv(param, k, A, p, round(n), weight);
}
double nsl_fit_model_geometric_param_deriv(unsigned int param, double k, double A, double p, double weight) {
	if (param == 0)
		return sqrt(weight) * gsl_ran_geometric_pdf((unsigned int)k, p);
	if (param == 1)
		return A * sqrt(weight) * pow(1.-p, k-2.) * (1.-k*p);

	return 0;
}
double nsl_fit_model_hypergeometric_param_deriv(unsigned int param, double k, double A, double n1, double n2, double t, double weight) {
	if (t > n1 + n2)
		return 0;

	double norm = sqrt(weight) * gsl_ran_hypergeometric_pdf((unsigned int)k, (unsigned int)n1, (unsigned int)n2, (unsigned int)t);
	if (param == 0)
		return norm;
	if (param == 1)
		return A * norm * (gsl_sf_psi(n1+1.) - gsl_sf_psi(n1-k+1.) - gsl_sf_psi(n1+n2+1.) + gsl_sf_psi(n1+n2-t+1.));
	if (param == 2)
		return A * norm * (gsl_sf_psi(n2+1.) - gsl_sf_psi(n2+k-t+1.) - gsl_sf_psi(n1+n2+1.) + gsl_sf_psi(n1+n2-t+1.));
	if (param == 3)
		return A * norm * (gsl_sf_psi(n2+k-t+1.) - gsl_sf_psi(n1+n2-t+1.) - gsl_sf_psi(t-k+1.) + gsl_sf_psi(t+1.));

	return 0;
}
double nsl_fit_model_logarithmic_param_deriv(unsigned int param, double k, double A, double p, double weight) {
	if (param == 0)
		return sqrt(weight) * gsl_ran_logarithmic_pdf((unsigned int)k, p);
	if (param == 1)
		return A * sqrt(weight) * pow(1.-p, k-2.) * (1.-k*p);

	return 0;
}
double nsl_fit_model_sech_dist_param_deriv(unsigned int param, double x, double A, double s, double mu, double weight) {
	double norm = sqrt(weight)/2./s, y = M_PI_2*(x-mu)/s;

	if (param == 0)
		return norm * 1./cosh(y);
	if (param == 1)
		return -A/s * norm * (y*tanh(y)+1.)/cosh(y);
	if (param == 2)
		return A*M_PI_2/s * norm * tanh(y)/cosh(y);

	return 0;
}
