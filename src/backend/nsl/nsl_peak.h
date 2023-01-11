/*
	File                 : nsl_peak.h
	Project              : LabPlot
	Description          : NSL peak detection and related methods
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef NSL_PEAK_H
#define NSL_PEAK_H

#include <stdlib.h>

/* simple peak detection by checking values */
/* returns indices of the peaks in data. np is the number of them */
/* caller must free memory */
size_t* nsl_peak_detect(double* data, size_t n, size_t* np);

/* more advanced peak detection (CWT, etc.)*/

#endif
