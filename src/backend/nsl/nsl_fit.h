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

#define NSL_FIT_MODEL_TYPE_COUNT 14
typedef enum {nsl_fit_model_polynomial, nsl_fit_model_power, nsl_fit_model_exponential, nsl_fit_model_inverse_exponential,
	nsl_fit_model_fourier, nsl_fit_model_gaussian, nsl_fit_model_lorentz, nsl_fit_model_maxwell, nsl_fit_model_sigmoid,
	nsl_fit_model_gompertz, nsl_fit_model_weibull, nsl_fit_model_lognormal, nsl_fit_model_gumbel, nsl_fit_model_custom=99} nsl_fit_model_type;
extern const char* nsl_fit_model_name[];
extern const char* nsl_fit_model_equation[];

/* convert unbounded variable x to bounded variable where bounds are [min, max] */
double nsl_fit_map_bound(double x, double min, double max);
/* convert bounded variable x to unbounded variable where bounds are [min, max] */
double nsl_fit_map_unbound(double x, double min, double max);

/* model parameter derivatives */
double nsl_fit_model_polynomial_param_deriv(double x, int j, double sigma);
double nsl_fit_model_power1_param_deriv(int param, double x, double a, double b, double sigma);
double nsl_fit_model_power2_param_deriv(int param, double x, double b, double c, double sigma);
double nsl_fit_model_exponential1_param_deriv(int param, double x, double a, double b, double sigma);
double nsl_fit_model_exponential2_param_deriv(int param, double x, double a, double b, double c, double d, double sigma);
double nsl_fit_model_exponential3_param_deriv(int param, double x, double a, double b, double c, double d, double e, double f, double sigma);
double nsl_fit_model_inverse_exponential_param_deriv(int param, double x, double a, double b, double sigma);
double nsl_fit_model_fourier_param_deriv(int param, int degree, double x, double w, double sigma);
double nsl_fit_model_gaussian_param_deriv(int param, double x, double a, double b, double sigma);
double nsl_fit_model_lorentz_param_deriv(int param, double x, double s, double t, double sigma);
double nsl_fit_model_maxwell_param_deriv(double x, double a, double sigma);
double nsl_fit_model_sigmoid_param_deriv(int param, double x, double a, double b, double c, double sigma);
double nsl_fit_model_gompertz_param_deriv(int param, double x, double a, double b, double c, double sigma);
double nsl_fit_model_weibull_param_deriv(int param, double x, double a, double b, double c, double sigma);
double nsl_fit_model_lognormal_param_deriv(int param, double x, double a, double b, double sigma);
double nsl_fit_model_gumbel_param_deriv(int param, double x, double a, double b, double sigma);

#endif /* NSL_FIT_H */
