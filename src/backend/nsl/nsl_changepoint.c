/*
	File                 : nsl_changepoint.c
	Project              : LabPlot
	Description          : NSL change point detection functions
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2026 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "nsl_changepoint.h"
#include <float.h>
#include <math.h>
#include <string.h>

const char* nsl_changepoint_method_name[] = {
	"Binary Segmentation",
	"PELT"
};

/* linear regression cost for segment [start, end) */
static double segment_cost(const double x[], const double y[], size_t start, size_t end) {
	size_t n = end - start;
	if (n < 2)
		return 0.0;

	double sum_x = 0., sum_y = 0., sum_xx = 0., sum_xy = 0.;
	for (size_t i = start; i < end; i++) {
		sum_x += x[i];
		sum_y += y[i];
		sum_xx += x[i] * x[i];
		sum_xy += x[i] * y[i];
	}

	double mean_x = sum_x / n;
	double mean_y = sum_y / n;
	double var_x = sum_xx / n - mean_x * mean_x;

	if (fabs(var_x) < 1e-15)
		return 0.0; /* vertical line */

	double slope = (sum_xy / n - mean_x * mean_y) / var_x;
	double intercept = mean_y - slope * mean_x;

	double sse = 0.;
	for (size_t i = start; i < end; i++) {
		double residual = y[i] - (slope * x[i] + intercept);
		sse += residual * residual;
	}

	return sse;
}

/* find best split point in [start, end) */
static size_t find_best_split(const double x[], const double y[], size_t start, size_t end, size_t min_size, double* best_cost) {
	size_t best_split = start + min_size;
	*best_cost = DBL_MAX;

	for (size_t s = start + min_size; s <= end - min_size; s++) {
		double cost = segment_cost(x, y, start, s) + segment_cost(x, y, s, end);
		if (cost < *best_cost) {
			*best_cost = cost;
			best_split = s;
		}
	}

	return best_split;
}

/* recursive binary segmentation helper */
static size_t binary_seg_recursive(const double x[], const double y[], size_t start, size_t end,
	double penalty, size_t min_size, size_t changepoints[], size_t max_cp, size_t current_cp) {

	if (end - start < 2 * min_size || current_cp >= max_cp)
		return current_cp;

	double base_cost = segment_cost(x, y, start, end);
	double split_cost;
	size_t split = find_best_split(x, y, start, end, min_size, &split_cost);

	if (base_cost - split_cost < penalty)
		return current_cp; /* no improvement */

	changepoints[current_cp++] = split;

	current_cp = binary_seg_recursive(x, y, start, split, penalty, min_size, changepoints, max_cp, current_cp);
	current_cp = binary_seg_recursive(x, y, split, end, penalty, min_size, changepoints, max_cp, current_cp);

	return current_cp;
}

/* comparison for qsort */
static int compare_size_t(const void* a, const void* b) {
	size_t ia = *(const size_t*)a;
	size_t ib = *(const size_t*)b;
	return (ia > ib) - (ia < ib);
}

size_t nsl_changepoint_binary_segmentation(const double x[], const double y[], size_t n,
	double penalty, size_t min_segment_size, size_t changepoints[], size_t max_changepoints) {

	if (!x || !y || n < 2 * min_segment_size || !changepoints || max_changepoints == 0)
		return 0;

	size_t count = binary_seg_recursive(x, y, 0, n, penalty, min_segment_size, changepoints, max_changepoints, 0);

	if (count > 0)
		qsort(changepoints, count, sizeof(size_t), compare_size_t);

	return count;
}

size_t nsl_changepoint_pelt(const double x[], const double y[], size_t n,
	double penalty, size_t min_segment_size, size_t changepoints[], size_t max_changepoints) {

	if (!x || !y || n < 2 * min_segment_size || !changepoints || max_changepoints == 0)
		return 0;

	/* ponytail: simplified PELT - full dynamic programming without pruning optimization.
	   Add pruning when datasets > 10k points show perf issues. */

	double* F = (double*)malloc((n + 1) * sizeof(double)); /* optimal cost to point i */
	size_t* cp = (size_t*)malloc((n + 1) * sizeof(size_t)); /* last changepoint before i */

	if (!F || !cp) {
		free(F);
		free(cp);
		return 0;
	}

	F[0] = -penalty;
	cp[0] = 0;

	for (size_t t = min_segment_size; t <= n; t++) {
		F[t] = DBL_MAX;
		cp[t] = 0;

		size_t start = (t > min_segment_size) ? min_segment_size : 0;
		for (size_t s = start; s <= t - min_segment_size; s++) {
			double cost = F[s] + segment_cost(x, y, s, t) + penalty;
			if (cost < F[t]) {
				F[t] = cost;
				cp[t] = s;
			}
		}
	}

	/* backtrack to find changepoints */
	size_t count = 0;
	size_t current = n;
	size_t* temp = (size_t*)malloc(max_changepoints * sizeof(size_t));

	if (temp) {
		while (current > 0 && count < max_changepoints) {
			size_t prev = cp[current];
			if (prev > 0 && prev < n)
				temp[count++] = prev;
			current = prev;
		}

		/* reverse order */
		for (size_t i = 0; i < count; i++)
			changepoints[i] = temp[count - 1 - i];

		free(temp);
	}

	free(F);
	free(cp);

	return count;
}
