/*
	File                 : nsl_pcm.c
	Project              : LabPlot
	Description          : NSL constants for process monitoring and control
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/


#include "nsl_pcm.h"
#include "nsl_common.h"
#include <gsl/gsl_math.h>
#include <gsl/gsl_sf_gamma.h>

// Precomputed d2 values for subgroup sizes from 2 to 100
static const double d2_values[] = {
	0.0,    // Placeholder for index 0 (not used)
	0.0,    // Placeholder for index 1 (not used)
	1.128,  // d2 for n = 2
	1.693,  // d2 for n = 3
	2.059,  // d2 for n = 4
	2.326,  // d2 for n = 5
	2.534,  // d2 for n = 6
	2.704,  // d2 for n = 7
	2.847,  // d2 for n = 8
	2.970,  // d2 for n = 9
	3.078,  // d2 for n = 10
	3.173,  // d2 for n = 11
	3.258,  // d2 for n = 12
	3.336,  // d2 for n = 13
	3.407,  // d2 for n = 14
	3.472,  // d2 for n = 15
	3.532,  // d2 for n = 16
	3.588,  // d2 for n = 17
	3.640,  // d2 for n = 18
	3.689,  // d2 for n = 19
	3.735,  // d2 for n = 20
	3.778,  // d2 for n = 21
	3.819,  // d2 for n = 22
	3.858,  // d2 for n = 23
	3.895,  // d2 for n = 24
	3.931,  // d2 for n = 25
	3.966,  // d2 for n = 26
	3.999,  // d2 for n = 27
	4.032,  // d2 for n = 28
	4.064,  // d2 for n = 29
	4.094,  // d2 for n = 30
	4.124,  // d2 for n = 31
	4.153,  // d2 for n = 32
	4.181,  // d2 for n = 33
	4.209,  // d2 for n = 34
	4.236,  // d2 for n = 35
	4.262,  // d2 for n = 36
	4.288,  // d2 for n = 37
	4.313,  // d2 for n = 38
	4.338,  // d2 for n = 39
	4.362,  // d2 for n = 40
	4.386,  // d2 for n = 41
	4.410,  // d2 for n = 42
	4.433,  // d2 for n = 43
	4.456,  // d2 for n = 44
	4.478,  // d2 for n = 45
	4.500,  // d2 for n = 46
	4.522,  // d2 for n = 47
	4.543,  // d2 for n = 48
	4.564,  // d2 for n = 49
	4.585,  // d2 for n = 50
	4.605,  // d2 for n = 51
	4.625,  // d2 for n = 52
	4.645,  // d2 for n = 53
	4.664,  // d2 for n = 54
	4.683,  // d2 for n = 55
	4.702,  // d2 for n = 56
	4.720,  // d2 for n = 57
	4.738,  // d2 for n = 58
	4.756,  // d2 for n = 59
	4.774,  // d2 for n = 60
	4.791,  // d2 for n = 61
	4.808,  // d2 for n = 62
	4.825,  // d2 for n = 63
	4.842,  // d2 for n = 64
	4.858,  // d2 for n = 65
	4.874,  // d2 for n = 66
	4.890,  // d2 for n = 67
	4.906,  // d2 for n = 68
	4.921,  // d2 for n = 69
	4.936,  // d2 for n = 70
	4.951,  // d2 for n = 71
	4.966,  // d2 for n = 72
	4.980,  // d2 for n = 73
	4.995,  // d2 for n = 74
	5.009,  // d2 for n = 75
	5.023,  // d2 for n = 76
	5.037,  // d2 for n = 77
	5.051,  // d2 for n = 78
	5.064,  // d2 for n = 79
	5.078,  // d2 for n = 80
	5.091,  // d2 for n = 81
	5.104,  // d2 for n = 82
	5.117,  // d2 for n = 83
	5.130,  // d2 for n = 84
	5.143,  // d2 for n = 85
	5.155,  // d2 for n = 86
	5.168,  // d2 for n = 87
	5.180,  // d2 for n = 88
	5.192,  // d2 for n = 89
	5.204,  // d2 for n = 90
	5.216,  // d2 for n = 91
	5.228,  // d2 for n = 92
	5.240,  // d2 for n = 93
	5.251,  // d2 for n = 94
	5.263,  // d2 for n = 95
	5.274,  // d2 for n = 96
	5.285,  // d2 for n = 97
	5.296,  // d2 for n = 98
	5.307,  // d2 for n = 99
	5.318   // d2 for n = 100
};


// Precomputed d3 values for subgroup sizes from 2 to 100
static const double d3_values[] = {
	0.0,    // Placeholder for index 0 (not used)
	0.0,    // Placeholder for index 1 (not used)
	0.853,  // d3 for n = 2
	0.888,  // d3 for n = 3
	0.880,  // d3 for n = 4
	0.864,  // d3 for n = 5
	0.848,  // d3 for n = 6
	0.833,  // d3 for n = 7
	0.820,  // d3 for n = 8
	0.808,  // d3 for n = 9
	0.797,  // d3 for n = 10
	0.787,  // d3 for n = 11
	0.778,  // d3 for n = 12
	0.770,  // d3 for n = 13
	0.763,  // d3 for n = 14
	0.756,  // d3 for n = 15
	0.750,  // d3 for n = 16
	0.744,  // d3 for n = 17
	0.739,  // d3 for n = 18
	0.734,  // d3 for n = 19
	0.729,  // d3 for n = 20
	0.724,  // d3 for n = 21
	0.720,  // d3 for n = 22
	0.716,  // d3 for n = 23
	0.712,  // d3 for n = 24
	0.708,  // d3 for n = 25
	0.704,  // d3 for n = 26
	0.701,  // d3 for n = 27
	0.698,  // d3 for n = 28
	0.695,  // d3 for n = 29
	0.692,  // d3 for n = 30
	0.689,  // d3 for n = 31
	0.686,  // d3 for n = 32
	0.683,  // d3 for n = 33
	0.681,  // d3 for n = 34
	0.679,  // d3 for n = 35
	0.676,  // d3 for n = 36
	0.674,  // d3 for n = 37
	0.672,  // d3 for n = 38
	0.670,  // d3 for n = 39
	0.668,  // d3 for n = 40
	0.666,  // d3 for n = 41
	0.664,  // d3 for n = 42
	0.662,  // d3 for n = 43
	0.660,  // d3 for n = 44
	0.659,  // d3 for n = 45
	0.657,  // d3 for n = 46
	0.655,  // d3 for n = 47
	0.654,  // d3 for n = 48
	0.652,  // d3 for n = 49
	0.651,  // d3 for n = 50
	0.649,  // d3 for n = 51
	0.648,  // d3 for n = 52
	0.646,  // d3 for n = 53
	0.645,  // d3 for n = 54
	0.644,  // d3 for n = 55
	0.642,  // d3 for n = 56
	0.641,  // d3 for n = 57
	0.640,  // d3 for n = 58
	0.639,  // d3 for n = 59
	0.637,  // d3 for n = 60
	0.636,  // d3 for n = 61
	0.635,  // d3 for n = 62
	0.634,  // d3 for n = 63
	0.633,  // d3 for n = 64
	0.632,  // d3 for n = 65
	0.631,  // d3 for n = 66
	0.630,  // d3 for n = 67
	0.629,  // d3 for n = 68
	0.628,  // d3 for n = 69
	0.627,  // d3 for n = 70
	0.626,  // d3 for n = 71
	0.625,  // d3 for n = 72
	0.624,  // d3 for n = 73
	0.623,  // d3 for n = 74
	0.622,  // d3 for n = 75
	0.621,  // d3 for n = 76
	0.620,  // d3 for n = 77
	0.619,  // d3 for n = 78
	0.618,  // d3 for n = 79
	0.617,  // d3 for n = 80
	0.616,  // d3 for n = 81
	0.615,  // d3 for n = 82
	0.614,  // d3 for n = 83
	0.613,  // d3 for n = 84
	0.612,  // d3 for n = 85
	0.611,  // d3 for n = 86
	0.610,  // d3 for n = 87
	0.609,  // d3 for n = 88
	0.608,  // d3 for n = 89
	0.607,  // d3 for n = 90
	0.606,  // d3 for n = 91
	0.605,  // d3 for n = 92
	0.604,  // d3 for n = 93
	0.603,  // d3 for n = 94
	0.602,  // d3 for n = 95
	0.601,  // d3 for n = 96
	0.600,  // d3 for n = 97
	0.599,  // d3 for n = 98
	0.598,  // d3 for n = 99
	0.597   // d3 for n = 100
};

double nsl_pcm_D3(unsigned int n) {
	const double d2 = nsl_pcm_d2(n);
	const double d3 = nsl_pcm_d3(n);
	return GSL_MAX_DBL(0., 1 - 3 * d3 / d2);
}

double nsl_pcm_D4(unsigned int n) {
	const double d2 = nsl_pcm_d2(n);
	const double d3 = nsl_pcm_d3(n);
	return 1 + 3 * d3 / d2;
}

double nsl_pcm_d2(unsigned int n) {
	if (n < 2 || n > 100) {
		// For n outside the precomputed range, calculate using the formula
		return gsl_sf_gamma(n / 2.0) * sqrt(2.0) / gsl_sf_gamma((n - 1) / 2.0);
	}

	return d2_values[n];
}

double nsl_pcm_d3(unsigned int n) {
	if (n < 2 || n > 100) {
		// For n outside the precomputed range, calculate using the formula
		return gsl_sf_gamma(n / 2.0) * sqrt(2.0) / gsl_sf_gamma((n - 1) / 2.0);
	}

	return d3_values[n];
}

double nsl_pcm_c4(unsigned int n) {
	return sqrt(2. / (n - 1)) * gsl_sf_gamma((n - 2.) / 2. + 1) / gsl_sf_gamma((n - 3.) / 2. + 1);
}


// averages
double nsl_pcm_A2(unsigned int n) {
	const double d2 = nsl_pcm_d2(n);
	return 3 / d2 / sqrt(n);
}

double nsl_pcm_A3(unsigned int n) {
	const double c4 = nsl_pcm_c4(n);
	return 3. / c4 / sqrt(n);
}

// lower limit
double nsl_pcm_B3(unsigned int n) {
	const double c4 = nsl_pcm_c4(n);
	return GSL_MAX_DBL(0., 1 -  3 / c4 * sqrt(1 - pow(c4, 2)));
}

double nsl_pcm_B4(unsigned int n) {
	const double c4 = nsl_pcm_c4(n);
	return 1 +  3 / c4 * sqrt(1 - pow(c4, 2));
}
