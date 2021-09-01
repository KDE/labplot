/*
    File                 : nsl_smooth.c
    Project              : LabPlot
    Description          : NSL smooth functions
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2010 Knut Franke <knut.franke@gmx.de>
    SPDX-FileCopyrightText: 2016 Stefan Gerlach <stefan.gerlach@uni.kn>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "nsl_smooth.h"
#include "nsl_common.h"
#include "nsl_sf_kernel.h"
#include "nsl_stats.h"
#include <gsl/gsl_math.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_sf_gamma.h>   /* gsl_sf_choose */

const char* nsl_smooth_type_name[] = { i18n("moving average (central)"), i18n("moving average (lagged)"), i18n("percentile"), i18n("Savitzky-Golay") };
const char* nsl_smooth_pad_mode_name[] = { i18n("none"), i18n("interpolating"), i18n("mirror"), i18n("nearest"), i18n("constant"), i18n("periodic") };
const char* nsl_smooth_weight_type_name[] = { i18n("uniform (rectangular)"), i18n("triangular"), i18n("binomial"), i18n("parabolic (Epanechnikov)"),
		i18n("quartic (biweight)"), i18n("triweight"), i18n("tricube"), i18n("cosine")  };
double nsl_smooth_pad_constant_lvalue = 0.0, nsl_smooth_pad_constant_rvalue = 0.0;

int nsl_smooth_moving_average(double *data, size_t n, size_t points, nsl_smooth_weight_type weight, nsl_smooth_pad_mode mode) {
	if (n == 0 || points == 0)
		return -1;

	size_t i, j;
	double *result = (double *)malloc(n*sizeof(double));
	for (i = 0; i < n; i++)
		result[i] = 0;

	for (i = 0; i < n; i++) {
		size_t np = points;
		size_t half = (points-1)/2;
		if (mode == nsl_smooth_pad_none) { /* reduce points */
			half = GSL_MIN(GSL_MIN((points-1)/2, i), n-i-1);
			np = 2 * half + 1;
		}

		/* weight */
		double sum = 0.0;
		double* w = (double*)malloc(np * sizeof(double));
		switch (weight) {
		case nsl_smooth_weight_uniform:
			for (j = 0; j < np; j++)
				w[j] = 1./np;
			break;
		case nsl_smooth_weight_triangular:
			sum = gsl_pow_2((double)(np + 1)/2);
			for (j = 0; j < np; j++)
				w[j] = GSL_MIN(j + 1, np - j)/sum;
			break;
		case nsl_smooth_weight_binomial:
			sum = (np - 1)/2.;
			for (j = 0; j < np; j++)
				w[j] = gsl_sf_choose((unsigned int)(2 * sum), (unsigned int)((sum + fabs(j - sum))/pow(4., sum)));
			break;
		case nsl_smooth_weight_parabolic:
			for (j = 0; j < np; j++) {
				w[j] = nsl_sf_kernel_parabolic(2.*(j-(np-1)/2.)/(np+1));
				sum += w[j];
			}
			for (j = 0; j < np; j++)
				w[j] /= sum;
			break;
		case nsl_smooth_weight_quartic:
			for (j = 0; j < np; j++) {
				w[j]=nsl_sf_kernel_quartic(2.*(j-(np-1)/2.)/(np+1));
				sum += w[j];
			}
			for (j = 0; j < np; j++)
				w[j] /= sum;
			break;
		case nsl_smooth_weight_triweight:
			for (j = 0; j < np; j++) {
				w[j] = nsl_sf_kernel_triweight(2.*(j-(np-1)/2.)/(np+1));
				sum += w[j];
			}
			for (j = 0 ; j < np; j++)
				w[j] /= sum;
			break;
		case nsl_smooth_weight_tricube:
			for (j = 0; j < np; j++) {
				w[j] = nsl_sf_kernel_tricube(2.*(j-(np-1)/2.)/(np+1));
				sum += w[j];
			}
			for (j = 0 ; j < np; j++)
				w[j] /= sum;
			break;
		case nsl_smooth_weight_cosine:
			for (j = 0; j < np; j++) {
				w[j] = nsl_sf_kernel_cosine((j-(np-1)/2.)/((np+1)/2.));
				sum += w[j];
			}
			for (j = 0 ; j < np; j++)
				w[j] /= sum;
			break;
		}

		/*printf("(%d) w:",i);
		for(j=0;j<np;j++)
			printf(" %g",w[j]);
		printf(" (half=%d) index = ",half);*/

		/* calculate weighted average */
		for (j = 0; j < np; j++) {
			int index = (int)(i-half+j);
			switch(mode) {
			case nsl_smooth_pad_none:
				result[i] += w[j]*data[index];
				break;
			case nsl_smooth_pad_interp:
				printf("not implemented yet\n");
				break;
			case nsl_smooth_pad_mirror:
				index = abs((int)(i-half+j));
				/*printf(" %d",GSL_MIN(index,2*(n-1)-index));*/
				result[i] += w[j] * data[GSL_MIN(index, 2*((int)n-1)-index)];
				break;
			case nsl_smooth_pad_nearest:
				/*printf(" %d",GSL_MIN(n-1,GSL_MAX(0,index)));*/
				result[i] += w[j] * data[GSL_MIN((int)n-1, GSL_MAX(0, index))];
				break;
			case nsl_smooth_pad_constant:
				if (index < 0)
					result[i] += w[j]*nsl_smooth_pad_constant_lvalue;
				else if (index > (int)n - 1)
					result[i] += w[j]*nsl_smooth_pad_constant_rvalue;
				else
					result[i] += w[j]*data[index];
				break;
			case nsl_smooth_pad_periodic:
				if (index < 0)
					index = index + (int)n;
				else if (index > (int)n - 1)
					index = index - (int)n;
				result[i] += w[j] * data[index];
				break;
			}
		}
		/*puts("");*/
		free(w);
	}

	for (i = 0; i < n; i++)
		data[i] = result[i];
	free(result);

	return 0;
}

int nsl_smooth_moving_average_lagged(double *data, size_t n, size_t points, nsl_smooth_weight_type weight, nsl_smooth_pad_mode mode) {
	if (n == 0 || points == 0)
		return -1;

	size_t i, j;
	double* result = (double *)malloc(n*sizeof(double));
	for (i = 0; i < n; i++)
		result[i] = 0;

	for (i = 0; i < n; i++) {
		size_t np = points;
		size_t half = (points-1)/2;
		if (mode == nsl_smooth_pad_none) { /* reduce points */
			np = GSL_MIN(points, i+1);
			half = np-1;
		}

		/* weight */
		double sum = 0.0, *w = (double *)malloc(np*sizeof(double));
		switch (weight) {
		case nsl_smooth_weight_uniform:
			for (j = 0; j < np; j++)
				w[j] = 1./np;
			break;
		case nsl_smooth_weight_triangular:
			sum = np*(double)(np+1)/2;
			for (j = 0; j < np; j++)
				w[j] = (j+1)/sum;
			break;
		case nsl_smooth_weight_binomial:
			for (j = 0; j < np; j++) {
				w[j] = gsl_sf_choose((unsigned int)(2*(np-1)), (unsigned int)j);
				sum += w[j];
			}
			for (j = 0 ; j < np; j++)
				w[j] /= sum;
			break;
		case nsl_smooth_weight_parabolic:
			for (j = 0; j < np; j++) {
				w[j] = nsl_sf_kernel_parabolic(1.-(1+j)/(double)np);
				sum += w[j];
			}
			for (j = 0; j < np; j++)
				w[j] /= sum;
			break;
		case nsl_smooth_weight_quartic:
			for( j = 0; j < np; j++) {
				w[j] = nsl_sf_kernel_quartic(1.-(1+j)/(double)np);
				sum += w[j];
			}
			for (j = 0; j < np; j++)
				w[j] /= sum;
			break;
		case nsl_smooth_weight_triweight:
			for (j = 0; j < np; j++) {
				w[j] = nsl_sf_kernel_triweight(1.-(1+j)/(double)np);
				sum += w[j];
			}
			for (j = 0; j < np; j++)
				w[j] /= sum;
			break;
		case nsl_smooth_weight_tricube:
			for (j = 0; j < np; j++) {
				w[j] = nsl_sf_kernel_tricube(1.-(1+j)/(double)np);
				sum += w[j];
			}
			for (j = 0; j < np; j++)
				w[j] /= sum;
			break;
		case nsl_smooth_weight_cosine:
			for (j = 0; j < np; j++) {
				w[j] = nsl_sf_kernel_cosine((np-1-j)/(double)np);
				sum += w[j];
			}
			for (j = 0; j < np; j++)
				w[j] /= sum;
			break;
		}

		/*printf("(%d) w:",i);
		for(j=0;j<np;j++)
			printf(" %g",w[j]);
		printf(" (half=%d) index = ",half);*/

		/* calculate weighted average */
		for (j = 0; j < np; j++) {
			int index = (int)(i-np+1+j);
			switch(mode) {
			case nsl_smooth_pad_none:
				result[i] += w[j]*data[i-half+j];
				/*printf(" %d",index);*/
				break;
			case nsl_smooth_pad_interp:
				printf("not implemented yet\n");
				break;
			case nsl_smooth_pad_mirror:
				index=abs(index);
				/*printf(" %d",index);*/
				result[i] += w[j]*data[index];
				break;
			case nsl_smooth_pad_nearest:
				/*printf(" %d", index);*/
				result[i] += w[j]*data[GSL_MAX(0, index)];
				break;
			case nsl_smooth_pad_constant:
				if (index < 0)
					result[i] += w[j]*nsl_smooth_pad_constant_lvalue;
				else
					result[i] += w[j]*data[index];

				break;
			case nsl_smooth_pad_periodic:
				if (index < 0)
					index += (int)n;
				/*printf(" %d",index);*/
				result[i] += w[j]*data[index];
				break;
			}
		}
		/*puts("");*/
		free(w);
	}

	for (i = 0; i < n; i++)
		data[i] = result[i];
	free(result);

	return 0;
}

int nsl_smooth_percentile(double *data, size_t n, size_t points, double percentile, nsl_smooth_pad_mode mode) {
	if (n == 0 || points == 0)
		return -1;

	size_t i, j;
	double *result = (double *)malloc(n * sizeof(double));

	for (i = 0; i < n; i++) {
		size_t np = points;
		size_t half = (points-1)/2;
		if (mode == nsl_smooth_pad_none) { /* reduce points */
			half = GSL_MIN(GSL_MIN((points-1)/2, i), n-i-1);
			np = 2*half+1;
		}

		double *values = (double *)malloc(np*sizeof(double));
		for (j = 0; j < np; j++) {
			int index = (int)(i-half+j);
			switch(mode) {
			case nsl_smooth_pad_none:
				/*printf(" %d",index);*/
				values[j] = data[index];
				break;
			case nsl_smooth_pad_interp:
				printf("not implemented yet\n");
				break;
			case nsl_smooth_pad_mirror:
				index=abs(index);
				/*printf(" %d",GSL_MIN(index,2*(n-1)-index));*/
				values[j] = data[GSL_MIN(index, 2*((int)n-1)-index)];
				break;
			case nsl_smooth_pad_nearest:
				/*printf(" %d",GSL_MIN(n-1,GSL_MAX(0,index)));*/
				values[j] = data[GSL_MIN((int)n-1, GSL_MAX(0, index))];
				break;
			case nsl_smooth_pad_constant:
				if (index < 0)
					values[j] = nsl_smooth_pad_constant_lvalue;
				else if (index > (int)n-1)
					values[j] = nsl_smooth_pad_constant_rvalue;
				else
					values[j] = data[index];
				break;
			case nsl_smooth_pad_periodic:
				if (index < 0)
					index = index + (int)n;
				else if (index > (int)n-1)
					index = index - (int)n;
				/*printf(" %d",index);*/
				values[j] = data[index];
				break;
			}
		}
		/*puts("");*/

		/*using type 7 as default */
		result[i] = nsl_stats_quantile(values, 1, np, percentile, nsl_stats_quantile_type7);
		free(values);
	}

	for (i = 0; i < n; i++)
		data[i] = result[i];
	free(result);

	return 0;
}

/* taken from SciDAVis */
int nsl_smooth_savgol_coeff(size_t points, int order, gsl_matrix *h) {
	size_t i;
	int j, error = 0;

	/* compute Vandermonde matrix */
	gsl_matrix *vandermonde = gsl_matrix_alloc(points, order+1);
	for (i = 0; i < points; ++i) {
		gsl_matrix_set(vandermonde, i, 0, 1.0);
		for (j = 1; j <= order; ++j)
			gsl_matrix_set(vandermonde, i, j, gsl_matrix_get(vandermonde, i, j-1) * i);
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

void nsl_smooth_pad_constant_set(double lvalue, double rvalue) {
	nsl_smooth_pad_constant_lvalue = lvalue;
	nsl_smooth_pad_constant_rvalue = rvalue;
}

int nsl_smooth_savgol(double *data, size_t n, size_t points, int order, nsl_smooth_pad_mode mode) {
	size_t i, k;
	int error = 0;
	size_t half = (points-1)/2;	/* n//2 */

	if (points > n) {
		printf("Tried to smooth over more points (points=%d) than given as input (%d).", (int)points, (int)n);
		return -1;
	}
	if (order < 1 || (size_t)order > points-1) {
		printf("The polynomial order must be between 1 and %d (%d given).", (int)(points-1), order);
		return -2;
	}

	/* Savitzky-Golay coefficient matrix, y' = H y */
	gsl_matrix *h = gsl_matrix_alloc(points, points);
	error = nsl_smooth_savgol_coeff(points, order, h);
	if (error) {
		printf("Internal error in Savitzky-Golay algorithm:\n%s", gsl_strerror(error));
		gsl_matrix_free(h);
		return error;
	}

	double *result = (double *)malloc(n*sizeof(double));
	for (i = 0; i < n; i++)
		result[i] = 0;

	/* left edge */
	if(mode == nsl_smooth_pad_none) {
		for (i = 0; i < half; i++) {
			/*reduce points and order*/
			size_t rpoints = 2*i+1;
			int rorder = GSL_MIN(order, (int)(rpoints-GSL_MIN(rpoints, 2)));

			gsl_matrix *rh = gsl_matrix_alloc(rpoints, rpoints);
			error = nsl_smooth_savgol_coeff(rpoints, rorder, rh);
			if (error) {
				printf("Internal error in Savitzky-Golay algorithm:\n%s",gsl_strerror(error));
				gsl_matrix_free(rh);
				free(result);
				return error;
			}

			for (k = 0; k < rpoints; k++)
				result[i] += gsl_matrix_get(rh, i, k) * data[k];
		}
	} else {
		for (i = 0; i < half; i++) {
			for (k = 0; k < points; k++)
				switch(mode) {
				case nsl_smooth_pad_interp:
					result[i] += gsl_matrix_get(h, i, k) * data[k];
					break;
				case nsl_smooth_pad_mirror:
					result[i] += gsl_matrix_get(h, half, k) * data[abs((int)(k+i-half))];
					break;
				case nsl_smooth_pad_nearest:
					result[i] += gsl_matrix_get(h, half, k) * data[i+k-GSL_MIN(half,i+k)];
					break;
				case nsl_smooth_pad_constant:
					if (k<half-i)
						result[i] += gsl_matrix_get(h, half, k) * nsl_smooth_pad_constant_lvalue;
					else
						result[i] += gsl_matrix_get(h, half, k) * data[i-half+k];
					break;
				case nsl_smooth_pad_periodic:
					result[i] += gsl_matrix_get(h, half, k) * data[k<half-i?n+i+k-half:i-half+k];
				case nsl_smooth_pad_none:
					break;
				}
		}
	}

	/* central part: convolve with fixed row of h */
	for (i = half; i < n-half; i++)
		for (k = 0; k < points; k++)
			result[i] += gsl_matrix_get(h, half, k) * data[i-half+k];

	/* right edge */
	if(mode == nsl_smooth_pad_none) {
		for (i = n-half; i < n; i++) {
			/*reduce points and order*/
			size_t rpoints = 2*(n-1-i) + 1;
			int rorder = (int)GSL_MIN((size_t)order, rpoints - GSL_MIN(2, rpoints));

			gsl_matrix *rh = gsl_matrix_alloc(rpoints, rpoints);
			error = nsl_smooth_savgol_coeff(rpoints, rorder, rh);
			if (error) {
				printf("Internal error in Savitzky-Golay algorithm:\n%s",gsl_strerror(error));
				gsl_matrix_free(rh);
				free(result);
				return error;
			}

			for (k = 0; k < rpoints; k++)
				result[i] += gsl_matrix_get(rh, n-1-i, k) * data[n-rpoints+k];
		}
	} else {
		for (i = n-half; i < n; i++) {
			for (k = 0; k < points; k++)
				switch(mode) {
				case nsl_smooth_pad_interp:
					result[i] += gsl_matrix_get(h, points-n+i, k) * data[n-points+k];
					break;
				case nsl_smooth_pad_mirror:
					result[i] += gsl_matrix_get(h, half, k) * data[n-1-abs((int)(k+1+i-n-half))];
					break;
				case nsl_smooth_pad_nearest:
					result[i] += gsl_matrix_get(h, half, k) * data[GSL_MIN(i-half+k,n-1)];
					break;
				case nsl_smooth_pad_constant:
					if (k < n-i+half)
						result[i] += gsl_matrix_get(h, half, k) * data[i-half+k];
					else
						result[i] += gsl_matrix_get(h, half, k) * nsl_smooth_pad_constant_rvalue;
					break;
				case nsl_smooth_pad_periodic:
					result[i] += gsl_matrix_get(h, half, k) * data[(i-half+k) % n];
				case nsl_smooth_pad_none:
					break;
				}
		}
	}

	gsl_matrix_free(h);

	for (i = 0; i < n; i++)
		data[i] = result[i];
	free(result);

	return 0;
}

int nsl_smooth_savgol_default( double *data, size_t n, size_t points, int order) {
	return nsl_smooth_savgol(data, n, points, order, nsl_smooth_pad_constant);
}
