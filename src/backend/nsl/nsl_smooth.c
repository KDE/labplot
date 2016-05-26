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

void nsl_smooth_savgol_constant_set(double lvalue, double rvalue) {
	nsl_smooth_savgol_constant_lvalue = lvalue;
	nsl_smooth_savgol_constant_rvalue = rvalue;
}

int nsl_smooth_savgol(double *data, int n, int points, int order, nsl_smooth_savgol_mode mode) {
	int i,k,error=0;
        int half=(points-1)/2;	/* n//2 */

        if (points > n) {
		printf("Tried to smooth over more points (points=%d) than given as input (%d).",points, n);
		return -1;
	}
        if (order <1 || order > points-1) {
		printf("The polynomial order must be between 1 and %d (%d given)!",points-1,order);
		return -2;
	}

        /* Savitzky-Golay coefficient matrix, y' = H y */
        gsl_matrix *h = gsl_matrix_alloc(points, points);
        if (error = nsl_smooth_savgol_coeff(points, order, h)) {
		printf("Internal error in Savitzky-Golay algorithm:\n%s",gsl_strerror(error));
                gsl_matrix_free(h);
                return error;
        }

	double *result = (double *)malloc(n*sizeof(double));

        /* left edge */
	for (i=0; i<half; i++) {
		double convolution = 0.0;
		for (k=0; k<points; k++)
			switch(mode) {
			case nsl_smooth_savgol_interp:
				convolution += gsl_matrix_get(h, i, k) * data[k];
				break;
			case nsl_smooth_savgol_mirror:
				convolution += gsl_matrix_get(h, half, k) * data[abs(k+i-half)];
				break;
			case nsl_smooth_savgol_nearest:
				convolution += gsl_matrix_get(h, half, k) * data[GSL_MAX(0,i-half+k)];
				break;
			case nsl_smooth_savgol_constant:
				if (k<half-i)
					convolution += gsl_matrix_get(h, half, k) * nsl_smooth_savgol_constant_lvalue;
				else
					convolution += gsl_matrix_get(h, half, k) * data[i-half+k];
				break;
			case nsl_smooth_savgol_wrap:
				convolution += gsl_matrix_get(h, half, k) * data[k<half-i?n+i+k-half:i-half+k];
        		}
		result[i] = convolution;
	}

        /* central part: convolve with fixed row of h */
	for (i=half; i<n-half; i++) {
		double convolution = 0.0;
		for (k=0; k<points; k++)
			convolution += gsl_matrix_get(h, half, k) * data[i-half+k];
		result[i] = convolution;
	}

        /* right edge */
	for (i=n-half; i<n; i++) {
		double convolution = 0.0;
		for (k=0; k<points; k++)
			switch(mode) {
			case nsl_smooth_savgol_interp:
				convolution += gsl_matrix_get(h, points-n+i, k) * data[n-points+k];
				break;
			case nsl_smooth_savgol_mirror:
				convolution += gsl_matrix_get(h, half, k) * data[n-1-abs(k+1+i-n-half)];
				break;
			case nsl_smooth_savgol_nearest:
				convolution += gsl_matrix_get(h, half, k) * data[GSL_MIN(i-half+k,n-1)];
				break;
			case nsl_smooth_savgol_constant:
				if (k < n-i+half)
					convolution += gsl_matrix_get(h, half, k) * data[i-half+k];
				else
					convolution += gsl_matrix_get(h, half, k) * nsl_smooth_savgol_constant_rvalue;
				break;
			case nsl_smooth_savgol_wrap:
				convolution += gsl_matrix_get(h, half, k) * data[(i-half+k) % n];
			}
		result[i] = convolution;
	}

        gsl_matrix_free(h);

        for (i=0; i<n; i++)
		data[i]=result[i];
	free(result);

	return 0;
}

int nsl_smooth_savgol_default( double *data, int n, int points, int order) {
	return nsl_smooth_savgol(data, n, points, order, nsl_smooth_savgol_constant);
}

