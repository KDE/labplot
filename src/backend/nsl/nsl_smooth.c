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
#include <gsl/gsl_blas.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_sf_gamma.h> /* gsl_sf_choose */
#include <gsl/gsl_sort.h>

const char* nsl_smooth_type_name[] = {i18n("Moving Average (Central)"),
									  i18n("Moving Average (Lagged)"),
									  i18n("Percentile"),
									  i18n("Savitzky-Golay"),
									  i18n("LOWESS")};
const char* nsl_smooth_pad_mode_name[] = {i18n("None"), i18n("Interpolating"), i18n("Mirror"), i18n("Nearest"), i18n("Constant"), i18n("Periodic")};
const char* nsl_smooth_weight_type_name[] = {i18n("Uniform (Rectangular)"),
											 i18n("Triangular"),
											 i18n("Binomial"),
											 i18n("Parabolic (Epanechnikov)"),
											 i18n("Quartic (Biweight)"),
											 i18n("Triweight"),
											 i18n("Tricube"),
											 i18n("Cosine")};
double nsl_smooth_pad_constant_lvalue = 0.0, nsl_smooth_pad_constant_rvalue = 0.0;

int nsl_smooth_moving_average(double* data, size_t n, size_t points, nsl_smooth_weight_type weight, nsl_smooth_pad_mode mode) {
	if (n == 0 || points == 0)
		return -1;

	size_t i, j;
	double* result = (double*)malloc(n * sizeof(double));
	for (i = 0; i < n; i++)
		result[i] = 0;

	for (i = 0; i < n; i++) {
		size_t np = points;
		size_t half = (points - 1) / 2;
		if (mode == nsl_smooth_pad_none) { /* reduce points */
			half = GSL_MIN(GSL_MIN((points - 1) / 2, i), n - i - 1);
			np = 2 * half + 1;
		}

		/* weight */
		double sum = 0.0;
		double* w = (double*)malloc(np * sizeof(double));
		switch (weight) {
		case nsl_smooth_weight_uniform:
			for (j = 0; j < np; j++)
				w[j] = 1. / np;
			break;
		case nsl_smooth_weight_triangular:
			sum = gsl_pow_2((double)(np + 1) / 2);
			for (j = 0; j < np; j++)
				w[j] = GSL_MIN(j + 1, np - j) / sum;
			break;
		case nsl_smooth_weight_binomial:
			sum = (np - 1) / 2.;
			for (j = 0; j < np; j++)
				w[j] = gsl_sf_choose((unsigned int)(2 * sum), (unsigned int)((sum + fabs(j - sum)) / pow(4., sum)));
			break;
		case nsl_smooth_weight_parabolic:
			for (j = 0; j < np; j++) {
				w[j] = nsl_sf_kernel_parabolic(2. * (j - (np - 1) / 2.) / (np + 1));
				sum += w[j];
			}
			for (j = 0; j < np; j++)
				w[j] /= sum;
			break;
		case nsl_smooth_weight_quartic:
			for (j = 0; j < np; j++) {
				w[j] = nsl_sf_kernel_quartic(2. * (j - (np - 1) / 2.) / (np + 1));
				sum += w[j];
			}
			for (j = 0; j < np; j++)
				w[j] /= sum;
			break;
		case nsl_smooth_weight_triweight:
			for (j = 0; j < np; j++) {
				w[j] = nsl_sf_kernel_triweight(2. * (j - (np - 1) / 2.) / (np + 1));
				sum += w[j];
			}
			for (j = 0; j < np; j++)
				w[j] /= sum;
			break;
		case nsl_smooth_weight_tricube:
			for (j = 0; j < np; j++) {
				w[j] = nsl_sf_kernel_tricube(2. * (j - (np - 1) / 2.) / (np + 1));
				sum += w[j];
			}
			for (j = 0; j < np; j++)
				w[j] /= sum;
			break;
		case nsl_smooth_weight_cosine:
			for (j = 0; j < np; j++) {
				w[j] = nsl_sf_kernel_cosine((j - (np - 1) / 2.) / ((np + 1) / 2.));
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
			int index = (int)(i - half + j);
			switch (mode) {
			case nsl_smooth_pad_none:
				result[i] += w[j] * data[index];
				break;
			case nsl_smooth_pad_interp:
				printf("not implemented yet\n");
				break;
			case nsl_smooth_pad_mirror:
				index = abs((int)(i - half + j));
				/*printf(" %d",GSL_MIN(index,2*(n-1)-index));*/
				result[i] += w[j] * data[GSL_MIN(index, 2 * ((int)n - 1) - index)];
				break;
			case nsl_smooth_pad_nearest:
				/*printf(" %d",GSL_MIN(n-1,GSL_MAX(0,index)));*/
				result[i] += w[j] * data[GSL_MIN((int)n - 1, GSL_MAX(0, index))];
				break;
			case nsl_smooth_pad_constant:
				if (index < 0)
					result[i] += w[j] * nsl_smooth_pad_constant_lvalue;
				else if (index > (int)n - 1)
					result[i] += w[j] * nsl_smooth_pad_constant_rvalue;
				else
					result[i] += w[j] * data[index];
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

int nsl_smooth_moving_average_lagged(double* data, size_t n, size_t points, nsl_smooth_weight_type weight, nsl_smooth_pad_mode mode) {
	if (n == 0 || points == 0)
		return -1;

	size_t i, j;
	double* result = (double*)malloc(n * sizeof(double));
	for (i = 0; i < n; i++)
		result[i] = 0;

	for (i = 0; i < n; i++) {
		size_t np = points;
		size_t half = (points - 1) / 2;
		if (mode == nsl_smooth_pad_none) { /* reduce points */
			np = GSL_MIN(points, i + 1);
			half = np - 1;
		}

		/* weight */
		double sum = 0.0, *w = (double*)malloc(np * sizeof(double));
		switch (weight) {
		case nsl_smooth_weight_uniform:
			for (j = 0; j < np; j++)
				w[j] = 1. / np;
			break;
		case nsl_smooth_weight_triangular:
			sum = np * (double)(np + 1) / 2;
			for (j = 0; j < np; j++)
				w[j] = (j + 1) / sum;
			break;
		case nsl_smooth_weight_binomial:
			for (j = 0; j < np; j++) {
				w[j] = gsl_sf_choose((unsigned int)(2 * (np - 1)), (unsigned int)j);
				sum += w[j];
			}
			for (j = 0; j < np; j++)
				w[j] /= sum;
			break;
		case nsl_smooth_weight_parabolic:
			for (j = 0; j < np; j++) {
				w[j] = nsl_sf_kernel_parabolic(1. - (1 + j) / (double)np);
				sum += w[j];
			}
			for (j = 0; j < np; j++)
				w[j] /= sum;
			break;
		case nsl_smooth_weight_quartic:
			for (j = 0; j < np; j++) {
				w[j] = nsl_sf_kernel_quartic(1. - (1 + j) / (double)np);
				sum += w[j];
			}
			for (j = 0; j < np; j++)
				w[j] /= sum;
			break;
		case nsl_smooth_weight_triweight:
			for (j = 0; j < np; j++) {
				w[j] = nsl_sf_kernel_triweight(1. - (1 + j) / (double)np);
				sum += w[j];
			}
			for (j = 0; j < np; j++)
				w[j] /= sum;
			break;
		case nsl_smooth_weight_tricube:
			for (j = 0; j < np; j++) {
				w[j] = nsl_sf_kernel_tricube(1. - (1 + j) / (double)np);
				sum += w[j];
			}
			for (j = 0; j < np; j++)
				w[j] /= sum;
			break;
		case nsl_smooth_weight_cosine:
			for (j = 0; j < np; j++) {
				w[j] = nsl_sf_kernel_cosine((np - 1 - j) / (double)np);
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
			int index = (int)(i - np + 1 + j);
			switch (mode) {
			case nsl_smooth_pad_none:
				result[i] += w[j] * data[i - half + j];
				/*printf(" %d",index);*/
				break;
			case nsl_smooth_pad_interp:
				printf("not implemented yet\n");
				break;
			case nsl_smooth_pad_mirror:
				index = abs(index);
				/*printf(" %d",index);*/
				result[i] += w[j] * data[index];
				break;
			case nsl_smooth_pad_nearest:
				/*printf(" %d", index);*/
				result[i] += w[j] * data[GSL_MAX(0, index)];
				break;
			case nsl_smooth_pad_constant:
				if (index < 0)
					result[i] += w[j] * nsl_smooth_pad_constant_lvalue;
				else
					result[i] += w[j] * data[index];

				break;
			case nsl_smooth_pad_periodic:
				if (index < 0)
					index += (int)n;
				/*printf(" %d",index);*/
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

int nsl_smooth_percentile(double* data, size_t n, size_t points, double percentile, nsl_smooth_pad_mode mode) {
	if (n == 0 || points == 0)
		return -1;

	size_t i, j;
	double* result = (double*)malloc(n * sizeof(double));

	for (i = 0; i < n; i++) {
		size_t np = points;
		size_t half = (points - 1) / 2;
		if (mode == nsl_smooth_pad_none) { /* reduce points */
			half = GSL_MIN(GSL_MIN((points - 1) / 2, i), n - i - 1);
			np = 2 * half + 1;
		}

		double* values = (double*)malloc(np * sizeof(double));
		for (j = 0; j < np; j++) {
			int index = (int)(i - half + j);
			switch (mode) {
			case nsl_smooth_pad_none:
				/*printf(" %d",index);*/
				values[j] = data[index];
				break;
			case nsl_smooth_pad_interp:
				printf("not implemented yet\n");
				break;
			case nsl_smooth_pad_mirror:
				index = abs(index);
				/*printf(" %d",GSL_MIN(index,2*(n-1)-index));*/
				values[j] = data[GSL_MIN(index, 2 * ((int)n - 1) - index)];
				break;
			case nsl_smooth_pad_nearest:
				/*printf(" %d",GSL_MIN(n-1,GSL_MAX(0,index)));*/
				values[j] = data[GSL_MIN((int)n - 1, GSL_MAX(0, index))];
				break;
			case nsl_smooth_pad_constant:
				if (index < 0)
					values[j] = nsl_smooth_pad_constant_lvalue;
				else if (index > (int)n - 1)
					values[j] = nsl_smooth_pad_constant_rvalue;
				else
					values[j] = data[index];
				break;
			case nsl_smooth_pad_periodic:
				if (index < 0)
					index = index + (int)n;
				else if (index > (int)n - 1)
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
int nsl_smooth_savgol_coeff(size_t points, int order, gsl_matrix* h) {
	size_t i;
	int j, error = 0;

	/* compute Vandermonde matrix */
	gsl_matrix* vandermonde = gsl_matrix_alloc(points, order + 1);
	for (i = 0; i < points; ++i) {
		gsl_matrix_set(vandermonde, i, 0, 1.0);
		for (j = 1; j <= order; ++j)
			gsl_matrix_set(vandermonde, i, j, gsl_matrix_get(vandermonde, i, j - 1) * i);
	}

	/* compute V^TV */
	gsl_matrix* vtv = gsl_matrix_alloc(order + 1, order + 1);
	error = gsl_blas_dgemm(CblasTrans, CblasNoTrans, 1.0, vandermonde, vandermonde, 0.0, vtv);

	if (!error) {
		/* compute (V^TV)^(-1) using LU decomposition */
		gsl_permutation* p = gsl_permutation_alloc(order + 1);
		int signum;
		error = gsl_linalg_LU_decomp(vtv, p, &signum);

		if (!error) {
			gsl_matrix* vtv_inv = gsl_matrix_alloc(order + 1, order + 1);
			error = gsl_linalg_LU_invert(vtv, p, vtv_inv);
			if (!error) {
				/* compute (V^TV)^(-1)V^T */
				gsl_matrix* vtv_inv_vt = gsl_matrix_alloc(order + 1, points);
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

int nsl_smooth_savgol(double* data, size_t n, size_t points, int order, nsl_smooth_pad_mode mode) {
	size_t i, k;
	int error = 0;
	size_t half = (points - 1) / 2; /* n//2 */

	if (points > n) {
		printf("Tried to smooth over more points (points=%d) than given as input (%d).", (int)points, (int)n);
		return -1;
	}
	if (order < 1 || (size_t)order > points - 1) {
		printf("The polynomial order must be between 1 and %d (%d given).", (int)(points - 1), order);
		return -2;
	}

	/* Savitzky-Golay coefficient matrix, y' = H y */
	gsl_matrix* h = gsl_matrix_alloc(points, points);
	error = nsl_smooth_savgol_coeff(points, order, h);
	if (error) {
		printf("Internal error in Savitzky-Golay algorithm:\n%s", gsl_strerror(error));
		gsl_matrix_free(h);
		return error;
	}

	double* result = (double*)malloc(n * sizeof(double));
	for (i = 0; i < n; i++)
		result[i] = 0;

	/* left edge */
	if (mode == nsl_smooth_pad_none) {
		for (i = 0; i < half; i++) {
			/*reduce points and order*/
			size_t rpoints = 2 * i + 1;
			int rorder = GSL_MIN(order, (int)(rpoints - GSL_MIN(rpoints, 2)));

			gsl_matrix* rh = gsl_matrix_alloc(rpoints, rpoints);
			error = nsl_smooth_savgol_coeff(rpoints, rorder, rh);
			if (error) {
				printf("Internal error in Savitzky-Golay algorithm:\n%s", gsl_strerror(error));
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
				switch (mode) {
				case nsl_smooth_pad_interp:
					result[i] += gsl_matrix_get(h, i, k) * data[k];
					break;
				case nsl_smooth_pad_mirror:
					result[i] += gsl_matrix_get(h, half, k) * data[abs((int)(k + i - half))];
					break;
				case nsl_smooth_pad_nearest:
					result[i] += gsl_matrix_get(h, half, k) * data[i + k - GSL_MIN(half, i + k)];
					break;
				case nsl_smooth_pad_constant:
					if (k < half - i)
						result[i] += gsl_matrix_get(h, half, k) * nsl_smooth_pad_constant_lvalue;
					else
						result[i] += gsl_matrix_get(h, half, k) * data[i - half + k];
					break;
				case nsl_smooth_pad_periodic:
					result[i] += gsl_matrix_get(h, half, k) * data[k < half - i ? n + i + k - half : i - half + k];
				case nsl_smooth_pad_none:
					break;
				}
		}
	}

	/* central part: convolve with fixed row of h */
	for (i = half; i < n - half; i++)
		for (k = 0; k < points; k++)
			result[i] += gsl_matrix_get(h, half, k) * data[i - half + k];

	/* right edge */
	if (mode == nsl_smooth_pad_none) {
		for (i = n - half; i < n; i++) {
			/*reduce points and order*/
			size_t rpoints = 2 * (n - 1 - i) + 1;
			int rorder = (int)GSL_MIN((size_t)order, rpoints - GSL_MIN(2, rpoints));

			gsl_matrix* rh = gsl_matrix_alloc(rpoints, rpoints);
			error = nsl_smooth_savgol_coeff(rpoints, rorder, rh);
			if (error) {
				printf("Internal error in Savitzky-Golay algorithm:\n%s", gsl_strerror(error));
				gsl_matrix_free(rh);
				free(result);
				return error;
			}

			for (k = 0; k < rpoints; k++)
				result[i] += gsl_matrix_get(rh, n - 1 - i, k) * data[n - rpoints + k];
		}
	} else {
		for (i = n - half; i < n; i++) {
			for (k = 0; k < points; k++)
				switch (mode) {
				case nsl_smooth_pad_interp:
					result[i] += gsl_matrix_get(h, points - n + i, k) * data[n - points + k];
					break;
				case nsl_smooth_pad_mirror:
					result[i] += gsl_matrix_get(h, half, k) * data[n - 1 - abs((int)(k + 1 + i - n - half))];
					break;
				case nsl_smooth_pad_nearest:
					result[i] += gsl_matrix_get(h, half, k) * data[GSL_MIN(i - half + k, n - 1)];
					break;
				case nsl_smooth_pad_constant:
					if (k < n - i + half)
						result[i] += gsl_matrix_get(h, half, k) * data[i - half + k];
					else
						result[i] += gsl_matrix_get(h, half, k) * nsl_smooth_pad_constant_rvalue;
					break;
				case nsl_smooth_pad_periodic:
					result[i] += gsl_matrix_get(h, half, k) * data[(i - half + k) % n];
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

int nsl_smooth_savgol_default(double* data, size_t n, size_t points, int order) {
	return nsl_smooth_savgol(data, n, points, order, nsl_smooth_pad_constant);
}

/* Helper function for LOWESS: tricube weight function */
static double nsl_smooth_lowess_tricube(double x) {
	if (fabs(x) >= 1.0)
		return 0.0;
	double tmp = 1.0 - x * x * x;
	return tmp * tmp * tmp;
}

/* Helper function for LOWESS: weighted least squares fit */
static int nsl_smooth_lowess_fit(const double* x, const double* y, const double* w, size_t n, int degree, double x0, double* y0) {
	/* Fit polynomial of given degree using weighted least squares
	 * Returns fitted value at x0 */

	if (n == 0 || degree < 0 || degree > 2)
		return -1;

	/* Build design matrix and solve normal equations */
	double sumw = 0.0, sumwx = 0.0, sumwx2 = 0.0, sumwx3 = 0.0, sumwx4 = 0.0;
	double sumwy = 0.0, sumwxy = 0.0, sumwx2y = 0.0;

	for (size_t i = 0; i < n; i++) {
		double dx = x[i] - x0;
		double dx2 = dx * dx;
		double ww = w[i];

		sumw += ww;
		sumwx += ww * dx;
		sumwx2 += ww * dx2;
		sumwy += ww * y[i];
		sumwxy += ww * dx * y[i];

		if (degree >= 2) {
			double dx3 = dx2 * dx;
			double dx4 = dx2 * dx2;
			sumwx3 += ww * dx3;
			sumwx4 += ww * dx4;
			sumwx2y += ww * dx2 * y[i];
		}
	}

	if (sumw == 0.0)
		return -1;

	/* Solve for coefficients */
	if (degree == 0) {
		/* Constant fit: y = a0 */
		*y0 = sumwy / sumw;
	} else if (degree == 1) {
		/* Linear fit: y = a0 + a1*(x-x0) */
		double det = sumw * sumwx2 - sumwx * sumwx;
		if (fabs(det) < 1e-12)
			return -1;
		*y0 = (sumwy * sumwx2 - sumwxy * sumwx) / det;
	} else {
		/* Quadratic fit: y = a0 + a1*(x-x0) + a2*(x-x0)^2 */
		/* Use GSL for 3x3 system */
		gsl_matrix* A = gsl_matrix_alloc(3, 3);
		gsl_vector* b = gsl_vector_alloc(3);
		gsl_vector* coef = gsl_vector_alloc(3);

		gsl_matrix_set(A, 0, 0, sumw);
		gsl_matrix_set(A, 0, 1, sumwx);
		gsl_matrix_set(A, 0, 2, sumwx2);
		gsl_matrix_set(A, 1, 0, sumwx);
		gsl_matrix_set(A, 1, 1, sumwx2);
		gsl_matrix_set(A, 1, 2, sumwx3);
		gsl_matrix_set(A, 2, 0, sumwx2);
		gsl_matrix_set(A, 2, 1, sumwx3);
		gsl_matrix_set(A, 2, 2, sumwx4);

		gsl_vector_set(b, 0, sumwy);
		gsl_vector_set(b, 1, sumwxy);
		gsl_vector_set(b, 2, sumwx2y);

		gsl_permutation* p = gsl_permutation_alloc(3);
		int signum;
		int status = gsl_linalg_LU_decomp(A, p, &signum);
		if (status == 0)
			status = gsl_linalg_LU_solve(A, p, b, coef);

		if (status == 0)
			*y0 = gsl_vector_get(coef, 0);
		else
			*y0 = sumwy / sumw; /* Fall back to mean */

		gsl_permutation_free(p);
		gsl_vector_free(coef);
		gsl_vector_free(b);
		gsl_matrix_free(A);

		if (status != 0)
			return -1;
	}

	return 0;
}

int nsl_smooth_lowess(const double* xdata, double* ydata, size_t n, double span, double delta, int iterations) {
	if (n == 0 || span <= 0.0 || span > 1.0 || iterations < 0)
		return -1;

	/* Calculate number of points in local neighborhood */
	size_t nlocal = (size_t)(span * n);
	if (nlocal < 2)
		nlocal = 2;
	if (nlocal > n)
		nlocal = n;

	/* Allocate working arrays */
	double* ys = (double*)malloc(n * sizeof(double)); /* smoothed values */
	double* rw = (double*)malloc(n * sizeof(double)); /* robustness weights */
	double* res = (double*)malloc(n * sizeof(double)); /* residuals */
	double* weights = (double*)malloc(nlocal * sizeof(double));

	if (!ys || !rw || !res || !weights) {
		free(ys);
		free(rw);
		free(res);
		free(weights);
		return -1;
	}

	/* Initialize robustness weights to 1 */
	for (size_t i = 0; i < n; i++)
		rw[i] = 1.0;

	/* Main LOWESS iterations */
	for (int iter = 0; iter <= iterations; iter++) {
		size_t last = 0; /* Last point where full regression was computed */

		for (size_t i = 0; i < n; i++) {
			double x0 = xdata[i];

			/* Check if we can use interpolation (delta optimization) */
			if (delta > 0.0 && i > 0 && fabs(x0 - xdata[last]) < delta) {
				/* Interpolate between last computed point and next point to be computed */
				size_t next = i;
				while (next < n - 1 && fabs(xdata[next + 1] - xdata[last]) < delta)
					next++;

				if (next >= n - 1) {
					/* Use last computed value */
					ys[i] = ys[last];
				} else {
					/* Linear interpolation */
					double alpha = (x0 - xdata[last]) / (xdata[next] - xdata[last]);
					ys[i] = (1.0 - alpha) * ys[last] + alpha * ys[next];
				}
				continue;
			}

			/* Find nlocal nearest neighbors */
			/* For simplicity, use symmetric neighborhood around i */
			size_t left = 0, right = n - 1;

			if (nlocal < n) {
				/* Find the nlocal closest points */
				if (i < nlocal / 2) {
					left = 0;
					right = nlocal - 1;
				} else if (i >= n - nlocal / 2) {
					left = n - nlocal;
					right = n - 1;
				} else {
					left = i - nlocal / 2;
					right = left + nlocal - 1;
				}
			}

			/* Calculate distances and find maximum distance */
			double maxdist = 0.0;
			for (size_t j = left; j <= right; j++) {
				double dist = fabs(xdata[j] - x0);
				if (dist > maxdist)
					maxdist = dist;
			}

			/* Calculate tricube weights */
			if (maxdist > 0.0) {
				for (size_t j = 0; j < right - left + 1; j++) {
					double dist = fabs(xdata[left + j] - x0);
					weights[j] = nsl_smooth_lowess_tricube(dist / maxdist) * rw[left + j];
				}
			} else {
				/* All points at same x location */
				for (size_t j = 0; j < right - left + 1; j++)
					weights[j] = rw[left + j];
			}

			/* Fit polynomial (degree 1: linear regression) */
			double fitted;
			int status = nsl_smooth_lowess_fit(&xdata[left], &ydata[left], weights, right - left + 1, 1, x0, &fitted);

			if (status == 0)
				ys[i] = fitted;
			else
				ys[i] = ydata[i]; /* Fall back to original value on error */

			last = i;
		}

		/* Update robustness weights for next iteration */
		if (iter < iterations) {
			/* Calculate residuals */
			for (size_t i = 0; i < n; i++)
				res[i] = fabs(ydata[i] - ys[i]);

			/* Find median absolute residual */
			double* sorted_res = (double*)malloc(n * sizeof(double));
			if (!sorted_res) {
				free(ys);
				free(rw);
				free(res);
				free(weights);
				return -1;
			}

			for (size_t i = 0; i < n; i++)
				sorted_res[i] = res[i];
			gsl_sort(sorted_res, 1, n);
			double cmad = 6.0 * nsl_stats_median_sorted(sorted_res, 1, n, nsl_stats_quantile_type7);
			free(sorted_res);

			/* Update robustness weights using bisquare function */
			if (cmad > 0.0) {
				for (size_t i = 0; i < n; i++) {
					double r = res[i] / cmad;
					if (r < 1.0)
						rw[i] = (1.0 - r * r) * (1.0 - r * r);
					else
						rw[i] = 0.0;
				}
			}
		}
	}

	/* Copy smoothed values to output */
	for (size_t i = 0; i < n; i++)
		ydata[i] = ys[i];

	free(ys);
	free(rw);
	free(res);
	free(weights);

	return 0;
}
