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
#include <gsl/gsl_sf_gamma.h>

double nsl_pcm_d2(unsigned int n) {
	// TODO
	return 1.;
}

double nsl_pcm_d3(unsigned int n) {
	// TODO
	return 1.;
}

double nsl_pcm_c4(unsigned int n) {
	return sqrt(2. / (n - 1)) * gsl_sf_gamma((n - 2.) / 2. + 1) / gsl_sf_gamma((n - 3.) / 2. + 1);
}
