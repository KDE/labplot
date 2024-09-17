/*
	File                 : nsl_pcm.h
	Project              : LabPlot
	Description          : NSL constants for process monitoring and control
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef NSL_PCM_H
#define NSL_PCM_H

#include <stdlib.h>

/*!
 *
 */
double nsl_pcm_D3(unsigned int n);

/*!
 *
 */
double nsl_pcm_D4(unsigned int n);

/*!
 *
 */
double nsl_pcm_d2(unsigned int n);

/*!
 *
 */
double nsl_pcm_d3(unsigned int n);

/*!
 *
 */
double nsl_pcm_c4(unsigned int n);



// averages

/*!
 *
 */
double nsl_pcm_A2(unsigned int n);

/*!
 *
 */
double nsl_pcm_A3(unsigned int n);


/*!
 *
 */
double nsl_pcm_B3(unsigned int n);

/*!
 *
 */
double nsl_pcm_B4(unsigned int n);

#endif /* NSL_PCM_H */
