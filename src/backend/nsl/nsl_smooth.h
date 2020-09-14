/***************************************************************************
    File                 : nsl_smooth.h
    Project              : LabPlot
    Description          : NSL smooth functions
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

#ifndef NSL_SMOOTH_H
#define NSL_SMOOTH_H

#include <gsl/gsl_matrix.h>

#define NSL_SMOOTH_TYPE_COUNT 4
typedef enum {nsl_smooth_type_moving_average, nsl_smooth_type_moving_average_lagged, nsl_smooth_type_percentile,
	nsl_smooth_type_savitzky_golay} nsl_smooth_type;
extern const char* nsl_smooth_type_name[];
/* TODO: LOWESS/etc., Bezier, B-Spline, (FFT Filter) */

/* mode of extension for padding signal 
 *	none: reduce points at edges
 *	interp: polynomial interpolation
 *	mirror:   3 2 | 1 2 3 4 5 | 4 3	(reflect)
 *	nearest:  1 1 | 1 2 3 4 5 | 5 5	(repeat)
 *	constant: L L | 1 2 3 4 5 | R R
 *	periodic: 4 5 | 1 2 3 4 5 | 1 2 (wrap)
*/
#define NSL_SMOOTH_PAD_MODE_COUNT 6
typedef enum {nsl_smooth_pad_none, nsl_smooth_pad_interp, nsl_smooth_pad_mirror, nsl_smooth_pad_nearest,
	nsl_smooth_pad_constant, nsl_smooth_pad_periodic} nsl_smooth_pad_mode;
extern const char* nsl_smooth_pad_mode_name[];

#define NSL_SMOOTH_WEIGHT_TYPE_COUNT 8
typedef enum {nsl_smooth_weight_uniform, nsl_smooth_weight_triangular, nsl_smooth_weight_binomial, 
	nsl_smooth_weight_parabolic, nsl_smooth_weight_quartic, nsl_smooth_weight_triweight, 
	nsl_smooth_weight_tricube, nsl_smooth_weight_cosine } nsl_smooth_weight_type;
extern const char* nsl_smooth_weight_type_name[];
/*TODO: IIR: exponential, Gaussian, see nsl_sf_kernel */

/* values used for constant padding */
extern double nsl_smooth_pad_constant_lvalue, nsl_smooth_pad_constant_rvalue;

/********* Smoothing algorithms **********/

/* Moving average */
int nsl_smooth_moving_average(double *data, size_t n, size_t points, nsl_smooth_weight_type weight, nsl_smooth_pad_mode mode);

/* Lagged moving average */
int nsl_smooth_moving_average_lagged(double *data, size_t n, size_t points, nsl_smooth_weight_type weight, nsl_smooth_pad_mode mode);

/* Percentile filter */
int nsl_smooth_percentile(double *data, size_t n, size_t points, double percentile, nsl_smooth_pad_mode mode);

/* Savitzky-Golay coefficients */
/**
 * \brief Compute Savitzky-Golay coefficients and store them into #h.
 *
 * This function follows GSL conventions in that it writes its result into a matrix allocated by
 * the caller and returns a non-zero result on error.
 *
 * The coefficient matrix is defined as the matrix H mapping a set of input values to the values
 * of the polynomial of order #polynom_order which minimizes squared deviations from the input
 * values. It is computed using the formula \$H=V(V^TV)^(-1)V^T\$, where \$V\$ is the Vandermonde
 * matrix of the point indices.
 *
 * For a short description of the mathematical background, see
 * http://www.statistics4u.info/fundstat_eng/cc_filter_savgol_math.html
 */
int nsl_smooth_savgol_coeff(size_t points, int order, gsl_matrix *h);

/* set values for constant padding */
void nsl_smooth_pad_constant_set(double lvalue, double rvalue);

/* Savitzky-Golay smoothing */
/**
 * \brief Savitzky-Golay smoothing of (uniformly distributed) data.
 *
 * When the data is not uniformly distributed, Savitzky-Golay looses its interesting conservation
 * properties. On the other hand, a central point of the algorithm is that for uniform data, the
 * operation can be implemented as a convolution. This is considerably more efficient than a more
 * generic method able to handle non-uniform input data.
 */
int nsl_smooth_savgol(double *data, size_t n, size_t points, int order, nsl_smooth_pad_mode mode);

/* Savitzky-Golay default smoothing (interp) */
int nsl_smooth_savgol_default(double *data, size_t n, size_t points, int order);

/* TODO SmoothFilter::smoothModifiedSavGol(double *x_in, double *y_inout)
	see SmoothFilter.cpp of libscidavis
*/

#endif /* NSL_SMOOTH_H */


