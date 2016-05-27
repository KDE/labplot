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

/* mode of extension for padding signal 
 *	interp: polynomial interpolation
 *	mirror:   3 2 | 1 2 3 4 5 | 4 3
 *	nearest:  1 1 | 1 2 3 4 5 | 5 5
 *	constant: 0 0 | 1 2 3 4 5 | 0 0
 *	wrap:     4 5 | 1 2 3 4 5 | 1 2
*/
typedef enum {nsl_smooth_savgol_interp=1, nsl_smooth_savgol_mirror, nsl_smooth_savgol_nearest,
	nsl_smooth_savgol_constant, nsl_smooth_savgol_wrap} nsl_smooth_savgol_mode;
/* values used for constant padding */
extern double nsl_smooth_savgol_constant_lvalue, nsl_smooth_savgol_constant_rvalue;

/* Savitzky-Golay coefficents */
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
int nsl_smooth_savgol_coeff(int points, int order, gsl_matrix *h);

/* set values for constant padding */
void nsl_smooth_savgol_constant_set(double lvalue, double rvalue);

/* Savitzky-Golay smoothing */
/**
 * \brief Savitzky-Golay smoothing of (uniformly distributed) data.
 *
 * TODO: When the data is not uniformly distributed, Savitzky-Golay looses its interesting conservation
 * properties. On the other hand, a central point of the algorithm is that for uniform data, the
 * operation can be implemented as a convolution. This is considerably more efficient than a more
 * generic method (see smoothModifiedSavGol()) able to handle non-uniform input data.
 */
int nsl_smooth_savgol(double *data, int n, int points, int order, nsl_smooth_savgol_mode mode);

/* Savitzky-Golay default smooting (interp) */
int nsl_smooth_savgol_default(double *data, int n, int points, int order);

/* TODO SmoothFilter::smoothModifiedSavGol(double *x_in, double *y_inout)
	see SmoothFilter.cpp of libscidavis
*/

#endif /* NSL_SMOOTH_H */


