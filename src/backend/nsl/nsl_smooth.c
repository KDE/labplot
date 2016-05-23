/***************************************************************************
    File                 : nsl_smooth.c
    Project              : LabPlot
    Description          : NSL smooth functions
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Ion Vasilief (ion_vasilief*yahoo.fr)
    Copyright            : (C) 2010 by Knut Franke (knut.franke*gmx.de)
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

#include <gsl/gsl_linalg.h>
#include <gsl/gsl_blas.h>
#include "nsl_smooth.h"

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
int nsl_smooth_savgol_coeff(int points, int order, gsl_matrix *h) {
        int i, j, error = 0;

        /* compute Vandermonde matrix */
        gsl_matrix *vandermonde = gsl_matrix_alloc(points, order+1);
        for (i = 0; i < points; ++i) {
                gsl_matrix_set(vandermonde, i, 0, 1.0);
                for (j = 1; j <= order; ++j)
                        gsl_matrix_set(vandermonde, i, j, gsl_matrix_get(vandermonde,i,j-1) * i);
        }

        /* compute V^TV */
        gsl_matrix *vtv = gsl_matrix_alloc(order+1, order+1);
        error = gsl_blas_dgemm(CblasTrans, CblasNoTrans, 1.0, vandermonde, vandermonde, 0.0, vtv);

        if (!error) {
                /* compute (V^TV)^(-1) using LU decomposition */
                gsl_permutation *p = gsl_permutation_alloc(order+1);
                int signum;
                error = gsl_linalg_LU_decomp(vtv, p, &signum);

                if (!error) {
                        gsl_matrix *vtv_inv = gsl_matrix_alloc(order+1, order+1);
                        error = gsl_linalg_LU_invert(vtv, p, vtv_inv);
                        if (!error) {
                                /* compute (V^TV)^(-1)V^T */
                                gsl_matrix *vtv_inv_vt = gsl_matrix_alloc(order+1, points);
                                error = gsl_blas_dgemm(CblasNoTrans, CblasTrans, 1.0, vtv_inv, vandermonde, 0.0, vtv_inv_vt);

                                if (!error) {
                                        /* finally, compute H = V(V^TV)^(-1)V^T */
                                        error = gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1.0, vandermonde, vtv_inv_vt, 0.0, h);
                                }
                                gsl_matrix_free(vtv_inv_vt);
                        }
                        gsl_matrix_free(vtv_inv);
                }
                gsl_permutation_free(p);
        }
        gsl_matrix_free(vtv);
        gsl_matrix_free(vandermonde);

        return error;
}

/**
 * \brief Savitzky-Golay smoothing of (uniformly distributed) data.
 *
 * When the data is not uniformly distributed, Savitzky-Golay looses its interesting conservation
 * properties. On the other hand, a central point of the algorithm is that for uniform data, the
 * operation can be implemented as a convolution. This is considerably more efficient than a more
 * generic method (see smoothModifiedSavGol()) able to handle non-uniform input data.
 *
 * There are at least three possible approaches for handling edges of the data vector (cutting them
 * off, zero padding and using the left-/rightmost smoothing polynomial for computing smoothed
 * values near the edges). Zero-padding is a particularly bad choice for signals with a distinctly
 * non-zero baseline and cutting off edges makes further computations on the original and smoothed
 * signals more difficult; 
 *
 * SciDAVis: therefore, deviating from the user-specified number of left/right
 * adjacent points (by smoothing over a fixed window at the edges) would be the least annoying
 * alternative; if it were not for the fact that previous versions of SciDAVis had a different
 * behaviour and such a subtle change to the behaviour would be even more annoying, especially
 * between bugfix releases. (would it help to add an "edge behaviour" option to the UI?)
 */
/* void SmoothFilter::smoothSavGol(double *, double *y_inout); */
int nsl_smooth_savgol(double *y_inout) {
	int d_n=100;	//TODO
	int d_right_points = 2;
	int d_left_points = 2;
	int d_polynom_order = 2;
	int i,k,error=0;

        /* total number of points in smoothing window */
        int points = d_left_points + d_right_points + 1;

        if (points < d_polynom_order+1)
		printf("The polynomial order must be lower than the number of points!");
        if (d_n < points)
		printf("Tried to smooth over more points (points=%d) than given as input (%d).",points, d_n);

        /* Savitzky-Golay coefficient matrix, y' = H y */
        gsl_matrix *h = gsl_matrix_alloc(points, points);
        if (error = nsl_smooth_savgol_coeff(points, d_polynom_order, h)) {
		printf("Internal error in Savitzky-Golay algorithm:\n%s",gsl_strerror(error));
                gsl_matrix_free(h);
                return error;
        }

        /* allocate memory for the result (temporary; don't overwrite y_inout while we still read from it) */
	double *result = (double *)malloc(d_n*sizeof(double));

        /* near left edge: use interpolation of (points) left-most input values
         i.e. we deviate from the specified left/right points to use */

/*        for (i=0; i<d_left_points; i++) {
                double convolution = 0.0;
                for (k=0; k<points; k++)
                        convolution += gsl_matrix_get(h, i, k) * y_inout[k];
                result[i] =  convolution;
        }
*/
        /* legacy behaviour: handle left edge by zero padding */
        for (i=0; i<d_left_points; i++) {
                double convolution = 0.0;
                for (k=d_left_points-i; k<points; k++)
                        convolution += gsl_matrix_get(h, d_left_points, k) * y_inout[i-d_left_points+k];
                result[i] = convolution;
        }
        /* central part: convolve with fixed row of h (as given by number of left points to use) */
        for (i=d_left_points; i<d_n-d_right_points; i++) {
                double convolution = 0.0;
                for (k=0; k<points; k++)
                        convolution += gsl_matrix_get(h, d_left_points, k) * y_inout[i-d_left_points+k];
                result[i] = convolution;
        }

        /* near right edge: use interpolation of (points) right-most input values
         i.e. we deviate from the specified left/right points to use */

/*        for (i=d_n-d_right_points; i<d_n; i++) {
                double convolution = 0.0;
                for (k=0; k<points; k++)
                        convolution += gsl_matrix_get(h, points-d_n+i, k) * y_inout[d_n-points+k];
                result[i] = convolution;
        }
*/
        /* legacy behaviour: handle right edge by zero padding */
        for (i=d_n-d_right_points; i<d_n; i++) {
                double convolution = 0.0;
                for (k=0; i-d_left_points+k < d_n; k++)
                        convolution += gsl_matrix_get(h, d_left_points, k) * y_inout[i-d_left_points+k];
                result[i] = convolution;
        }

        gsl_matrix_free(h);

        /* TODO: qCopy(result.begin(), result.end(), y_inout); */
	free(result);

	return 0;
}


/* TODO SmoothFilter::smoothModifiedSavGol(double *x_in, double *y_inout)
	see SmoothFilter.cpp of libscidavis
*/
