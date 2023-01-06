/*
	File                 : nsl_baseline.c
	Project              : LabPlot
	Description          : NSL baseline detection and subtraction functions
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2016-2023 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "nsl_baseline.h"
#include "nsl_stats.h"

void nsl_baseline_remove_minimum(double *data, const size_t n) {
	const double min = nsl_stats_minimum(data, n, NULL);

	for (size_t i = 0; i < n; i++)
		data[i] -= min;

}
