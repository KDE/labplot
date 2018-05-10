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
extern const char* nsl_fit_model_basic_pic_name[];
#define NSL_FIT_MODEL_PEAK_COUNT 4
typedef enum {nsl_fit_model_gaussian, nsl_fit_model_lorentz, nsl_fit_model_sech, nsl_fit_model_logistic} nsl_fit_model_type_peak;
extern const char* nsl_fit_model_peak_pic_name[];
#define NSL_FIT_MODEL_GROWTH_COUNT 8
typedef enum {nsl_fit_model_atan, nsl_fit_model_tanh, nsl_fit_model_algebraic_sigmoid, nsl_fit_model_sigmoid, nsl_fit_model_erf, 
	nsl_fit_model_hill, nsl_fit_model_gompertz, nsl_fit_model_gudermann} nsl_fit_model_type_growth;
extern const char* nsl_fit_model_growth_pic_name[];

extern const char* nsl_fit_model_category_name[];
extern const char* nsl_fit_model_basic_name[];
extern const char* nsl_fit_model_peak_name[];
extern const char* nsl_fit_model_growth_name[];
extern const char* nsl_fit_model_basic_equation[];
extern const char* nsl_fit_model_peak_equation[];
extern const char* nsl_fit_model_growth_equation[];

#define NSL_FIT_WEIGHT_TYPE_COUNT 8
typedef enum {nsl_fit_weight_no,		/* w = 1 */
              nsl_fit_weight_instrumental,	/* w = 1/c^2    (Gaussian, Given errors): default */
              nsl_fit_weight_direct,		/* w = c */
              nsl_fit_weight_inverse,		/* w = 1/c */
              nsl_fit_weight_statistical,	/* w = 1/y      (Poisson) */
              nsl_fit_weight_statistical_fit,	/* w = 1/Y      (Poisson) */
              nsl_fit_weight_relative,		/* w = 1/y^2    (Variance) */
              nsl_fit_weight_relative_fit,	/* w = 1/Y^2    (Variance) */
} nsl_fit_weight_type;
extern const char* nsl_fit_weight_type_name[];

/* convert unbounded variable x to bounded variable where bounds are [min, max] */
double nsl_fit_map_bound(double x, double min, double max);
/* convert bounded variable x to unbounded variable where bounds are [min, max] */
double nsl_fit_map_unbound(double x, double min, double max);

/* model parameter derivatives */
/* basic */
double nsl_fit_model_polynomial_param_deriv(double x, int j, double weight);
double nsl_fit_model_power1_param_deriv(unsigned int param, double x, double a, double b, double weight);
double nsl_fit_model_power2_param_deriv(unsigned int param, double x, double b, double c, double weight);
double nsl_fit_model_exponentialn_param_deriv(unsigned int param, double x, double *p, double weight);
double nsl_fit_model_inverse_exponential_param_deriv(unsigned int param, double x, double a, double b, double weight);
double nsl_fit_model_fourier_param_deriv(unsigned int param, unsigned int degree, double x, double w, double weight);
/* peak */
double nsl_fit_model_gaussian_param_deriv(unsigned int param, double x, double a, double s, double mu, double weight);
double nsl_fit_model_lorentz_param_deriv(unsigned int param, double x, double a, double s, double t, double weight);
double nsl_fit_model_sech_param_deriv(unsigned int param, double x, double a, double s, double mu, double weight);
double nsl_fit_model_logistic_param_deriv(unsigned int param, double x, double a, double s, double mu, double weight);
/* growth */
double nsl_fit_model_atan_param_deriv(unsigned int param, double x, double a, double mu, double s, double weight);
double nsl_fit_model_tanh_param_deriv(unsigned int param, double x, double a, double mu, double s, double weight);
double nsl_fit_model_algebraic_sigmoid_param_deriv(unsigned int param, double x, double a, double mu, double s, double weight);
double nsl_fit_model_sigmoid_param_deriv(unsigned int param, double x, double a, double mu, double k, double weight);
double nsl_fit_model_erf_param_deriv(unsigned int param, double x, double a, double mu, double s, double weight);
double nsl_fit_model_hill_param_deriv(unsigned int param, double x, double a, double n, double s, double weight);
double nsl_fit_model_gompertz_param_deriv(unsigned int param, double x, double a, double b, double c, double weight);
double nsl_fit_model_gudermann_param_deriv(unsigned int param, double x, double a, double mu, double s, double weight);
/* distributions */
double nsl_fit_model_gaussian_tail_param_deriv(unsigned int param, double x, double A, double s, double a, double mu, double weight);
double nsl_fit_model_exponential_param_deriv(unsigned int param, double x, double a, double l, double mu, double weight);
double nsl_fit_model_laplace_param_deriv(unsigned int param, double x, double a, double s, double mu, double weight);
double nsl_fit_model_exp_pow_param_deriv(unsigned int param, double x, double a, double s, double b, double mu, double weight);
double nsl_fit_model_poisson_param_deriv(unsigned int param, double x, double l, double a, double weight);
double nsl_fit_model_lognormal_param_deriv(unsigned int param, double x, double a, double b, double mu, double weight);
double nsl_fit_model_gamma_param_deriv(unsigned int param, double x, double a, double k, double t, double weight);
double nsl_fit_model_flat_param_deriv(unsigned int param, double x, double A, double b, double a, double weight);
double nsl_fit_model_rayleigh_param_deriv(unsigned int param, double x, double a, double s, double weight);
double nsl_fit_model_rayleigh_tail_param_deriv(unsigned int param, double x, double a, double s, double mu, double weight);
double nsl_fit_model_landau_param_deriv(unsigned int param, double x, double weight);
double nsl_fit_model_chi_square_param_deriv(unsigned int param, double x, double a, double n, double weight);
double nsl_fit_model_students_t_param_deriv(unsigned int param, double x, double a, double n, double weight);
double nsl_fit_model_fdist_param_deriv(unsigned int param, double x, double a, double n1, double n2, double weight);
double nsl_fit_model_beta_param_deriv(unsigned int param, double x, double A, double a, double b, double weight);
double nsl_fit_model_pareto_param_deriv(unsigned int param, double x, double A, double a, double b, double weight);
double nsl_fit_model_weibull_param_deriv(unsigned int param, double x, double k, double l, double mu, double a, double weight);
double nsl_fit_model_gumbel1_param_deriv(unsigned int param, double x, double s, double b, double mu, double a, double weight);
double nsl_fit_model_gumbel2_param_deriv(unsigned int param, double x, double a, double b, double mu, double A, double weight);
double nsl_fit_model_binomial_param_deriv(unsigned int param, double x, double p, double n, double A, double weight);
double nsl_fit_model_negative_binomial_param_deriv(unsigned int param, double k, double p, double n, double A, double weight);
double nsl_fit_model_pascal_param_deriv(unsigned int param, double k, double p, double n, double A, double weight);
double nsl_fit_model_geometric_param_deriv(unsigned int param, double k, double p, double A, double weight);
double nsl_fit_model_hypergeometric_param_deriv(unsigned int param, double k, double n1, double n2, double t, double A, double weight);
double nsl_fit_model_logarithmic_param_deriv(unsigned int param, double k, double p, double A, double weight);

double nsl_fit_model_maxwell_param_deriv(unsigned int param, double x, double a, double c, double weight);
double nsl_fit_model_sech_dist_param_deriv(unsigned int param, double x, double a, double s, double mu, double weight);
double nsl_fit_model_levy_param_deriv(unsigned int param, double x, double a, double g, double mu, double weight);
double nsl_fit_model_frechet_param_deriv(unsigned int param, double x, double a, double mu, double s, double c, double weight);

#endif /* NSL_FIT_H */
