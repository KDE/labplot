/*
	File                 : nsl_peak.c
	Project              : LabPlot
	Description          : NSL peak detection and releated methods
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "nsl_peak.h"

#include <stdio.h>

size_t* nsl_peak_detect(double* data, size_t n, size_t* np) {
	if (n <= 1) // nothing to do
		return NULL;

	size_t* peaks = (size_t*)malloc(n * sizeof(size_t));
	if (!peaks) {
		fprintf(stderr, "ERROR allocating memory for peak detection\n");
		return NULL;
	}

	*np = 0;
	// find peaks
	for (size_t i = 0; i < n; i++) {
		if (i == 0 && n > 1 && data[0] > data[1]) { // start
			peaks[(*np)++] = i;
			continue;
		}
		if (i == n - 1 && n > 1 && data[n - 1] > data[n - 2]) { // end
			peaks[(*np)++] = i;
			continue;
		}

		if (data[i - 1] < data[i] && data[i] > data[i + 1])
			peaks[(*np)++] = i;
	}

	if (!realloc(peaks, *np * sizeof(size_t))) { // should never happen since *np <= n
		fprintf(stderr, "ERROR reallocating memory for peak detection\n");
		free(peaks);
		return NULL;
	}

	return peaks;
}
