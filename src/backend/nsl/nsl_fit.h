/***************************************************************************
    File                 : nsl_fit.h
    Project              : LabPlot
    Description          : NSL (non)linear fitting functions
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

#ifndef NSL_FIT_H
#define NSL_FIT_H

#define NSL_FIT_MODEL_CATEGORY_COUNT 5
typedef enum {nsl_fit_model_basic, nsl_fit_model_peak, nsl_fit_model_growth, nsl_fit_model_distribution, nsl_fit_model_custom=99} nsl_fit_model_category;

#define NSL_FIT_MODEL_BASIC_COUNT 5
typedef enum {nsl_fit_model_polynomial, nsl_fit_model_power, nsl_fit_model_exponential, nsl_fit_model_inverse_exponential,
	nsl_fit_model_fourier} nsl_fit_model_type_basic;
#define NSL_FIT_MODEL_PEAK_COUNT 4
typedef enum {nsl_fit_model_gaussian, nsl_fit_model_lorentz, nsl_fit_model_sech, nsl_fit_model_logistic} nsl_fit_model_type_peak;
/*TODO: Voigt */
#define NSL_FIT_MODEL_GROWTH_COUNT 8
typedef enum {nsl_fit_model_atan, nsl_fit_model_tanh, nsl_fit_model_algebraic_sigmoid, nsl_fit_model_sigmoid, nsl_fit_model_erf, 
	nsl_fit_model_hill, nsl_fit_model_gompertz, nsl_fit_model_gudermann} nsl_fit_model_type_growth;
#define NSL_FIT_MODEL_DISTRIBUTION_COUNT 12
typedef enum {
	nsl_fit_model_normal, nsl_fit_model_cauchy_lorentz, nsl_fit_model_maxwell, nsl_fit_model_lognormal, nsl_fit_model_gamma, 
	nsl_fit_model_laplace, nsl_fit_model_rayleigh, nsl_fit_model_levy, nsl_fit_model_chi_square, nsl_fit_model_weibull, nsl_fit_model_frechet,
	nsl_fit_model_gumbel} nsl_fit_model_type_distribution;

extern const char* nsl_fit_model_category_name[];
extern const char* nsl_fit_model_basic_name[];
extern const char* nsl_fit_model_peak_name[];
extern const char* nsl_fit_model_growth_name[];
extern const char* nsl_fit_model_distribution_name[];
extern const char* nsl_fit_model_basic_equation[];
extern const char* nsl_fit_model_peak_equation[];
extern const char* nsl_fit_model_growth_equation[];
extern const char* nsl_fit_model_distribution_equation[];

/* convert unbounded variable x to bounded variable where bounds are [min, max] */
double nsl_fit_map_bound(double x, double min, double max);
/* convert bounded variable x to unbounded variable where bounds are [min, max] */
double nsl_fit_map_unbound(double x, double min, double max);

/* model parameter derivatives */
/* basic */
double nsl_fit_model_polynomial_param_deriv(double x, int j, double sigma);
double nsl_fit_model_power1_param_deriv(int param, double x, double a, double b, double sigma);
double nsl_fit_model_power2_param_deriv(int param, double x, double b, double c, double sigma);
double nsl_fit_model_exponential1_param_deriv(int param, double x, double a, double b, double sigma);
double nsl_fit_model_exponential2_param_deriv(int param, double x, double a, double b, double c, double d, double sigma);
double nsl_fit_model_exponential3_param_deriv(int param, double x, double a, double b, double c, double d, double e, double f, double sigma);
double nsl_fit_model_inverse_exponential_param_deriv(int param, double x, double a, double b, double sigma);
double nsl_fit_model_fourier_param_deriv(int param, int degree, double x, double w, double sigma);
/* peak */
double nsl_fit_model_gaussian_param_deriv(int param, double x, double s, double mu, double a, double sigma);
double nsl_fit_model_cauchy_lorentz_param_deriv(int param, double x, double s, double t, double a, double sigma);
double nsl_fit_model_sech_param_deriv(int param, double x, double s, double mu, double a, double sigma);
double nsl_fit_model_logistic_param_deriv(int param, double x, double s, double mu, double a, double sigma);
/* growth */
double nsl_fit_model_atan_param_deriv(int param, double x, double s, double mu, double a, double sigma);
double nsl_fit_model_tanh_param_deriv(int param, double x, double s, double mu, double a, double sigma);
double nsl_fit_model_algebraic_sigmoid_param_deriv(int param, double x, double s, double mu, double a, double sigma);
double nsl_fit_model_sigmoid_param_deriv(int param, double x, double k, double mu, double a, double sigma);
double nsl_fit_model_erf_param_deriv(int param, double x, double s, double mu, double a, double sigma);
double nsl_fit_model_hill_param_deriv(int param, double x, double s, double n, double a, double sigma);
double nsl_fit_model_gompertz_param_deriv(int param, double x, double a, double b, double c, double sigma);
double nsl_fit_model_gudermann_param_deriv(int param, double x, double s, double mu, double a, double sigma);
/* distributions */
double nsl_fit_model_maxwell_param_deriv(int param, double x, double a, double c, double sigma);
double nsl_fit_model_lognormal_param_deriv(int param, double x, double b, double mu, double a, double sigma);
double nsl_fit_model_gamma_param_deriv(int param, double x, double b, double p, double a, double sigma);
double nsl_fit_model_laplace_param_deriv(int param, double x, double s, double mu, double a, double sigma);
double nsl_fit_model_rayleigh_param_deriv(int param, double x, double s, double a, double sigma);
double nsl_fit_model_levy_param_deriv(int param, double x, double g, double mu, double a, double sigma);
double nsl_fit_model_chi_square_param_deriv(int param, double x, double n, double a, double sigma);
double nsl_fit_model_weibull_param_deriv(int param, double x, double k, double l, double mu, double a, double sigma);
double nsl_fit_model_frechet_param_deriv(int param, double x, double a, double mu, double s, double c, double sigma);
double nsl_fit_model_gumbel_param_deriv(int param, double x, double b, double mu, double a, double sigma);

#endif /* NSL_FIT_H */
