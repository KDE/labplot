/*
	File                 : nsl_peak.cpp
	Project              : LabPlot
	Description          : NSL peak detection and releated methods
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "nsl_peak.h"
#include "backend/lib/macros.h"

// simple peak detection
template<typename T>
size_t* nsl_peak_detect(T* data, size_t n, size_t& np, T height, size_t distance) {
	DEBUG(Q_FUNC_INFO << ", h = " << height << ", d = " << distance)
	if (n <= 1) // nothing to do
		return nullptr;

	size_t* peaks = (size_t*)malloc(n * sizeof(size_t));
	if (!peaks) {
		WARN("ERROR allocating memory for peak detection")
		return nullptr;
	}

	// find peaks
	np = 0;
	for (size_t i = 0; i < n; i++) {
		bool found = false;
		if (i == 0 && n > 1 && data[0] > data[1]) // start
			found = true;
		else if (i == n - 1 && n > 1 && data[n - 1] > data[n - 2]) // end
			found = true;
		else if (data[i - 1] < data[i] && data[i] > data[i + 1])
			found = true;

		// check minimum height and distance
		if (found && data[i] >= height && (np == 0 || i - peaks[np - 1] >= distance))
			peaks[np++] = i;
	}
	if (np == 0) { // nothing found
		printf("nothing found\n");
		free(peaks);
		return nullptr;
	}

	if (!(peaks = (size_t*)realloc(peaks, np * sizeof(size_t)))) { // should never happen since np <= n
		WARN("ERROR reallocating memory for peak detection")
		free(peaks);
		return nullptr;
	}

	return peaks;
}

// needs explicit instantiation
template size_t* nsl_peak_detect<double>(double* data, size_t n, size_t& np, double height, size_t distance);
template size_t* nsl_peak_detect<int>(int* data, size_t n, size_t& np, int height, size_t distance);
template size_t* nsl_peak_detect<qint64>(qint64* data, size_t n, size_t& np, qint64 height, size_t distance);
