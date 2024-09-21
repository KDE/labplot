/*
	File                 : nsl_pcm.h
	Project              : LabPlot
	Description          : Constants for process monitoring and control
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef NSL_PCM_H
#define NSL_PCM_H

#include <stdlib.h>

// Naming conventions for the scaling and correction factors are taken from
// 1. Wheeler "Making Sense of Data", Appendix, page 383
// 2. Dean V. Neubauer, Editor, "Manual on Presentation of Data and Control Chart Analysis", 8th Edition

//*************************************************************
//*********** Scaling factors for charts for Ranges ***********
//*************************************************************

/*!
 * scaling factor D3 used to define the lower limit in the mR and R charts for the subgroup size \c n,
 * obtained by multiplying with this factor the mean moving range and the mean range, respectively.
 */
double nsl_pcm_D3(unsigned int n);

/*!
 * scaling factor D4 used to define the upper limit in the mR and R charts for the subgroup size \c n,
 * obtained by multiplying the mean moving range and the mean range, respectively, with this factor.
 */
double nsl_pcm_D4(unsigned int n);

/*!
 * scaling factor D5 used to define the lower limit in the mR and R charts for the subgroup size \c n,
 * obtained by multiplying with this factor the median moving range and the median range, respectively.
 */
double nsl_pcm_D5(unsigned int n);

/*!
 * scaling factor D6 used to define the upper limit in the mR and R charts for the subgroup size \c n,
 * obtained by multiplying the median moving range and the median range, respectively, with this factor.
 */
double nsl_pcm_D6(unsigned int n);

//*************************************************************
//********* Scaling factors for charts for Averages ***********
//*************************************************************
/*!
 * scaling factor A2 used to define the upper and lower limits in the XBarR chart for the subgroup size \c n,
 * obtained by multiplying the mean range with this factor.
 */
double nsl_pcm_A2(unsigned int n);

/*!
 * scaling factor A3 used to define the upper and lower limits in the XBarS chart for the subgroup size \c n,
 * obtained by multiplying the mean standard deviation with this factor.
 */
double nsl_pcm_A3(unsigned int n);

/*!
 * scaling factor A4 used to define the upper and lower limits in the XBarR chart for the subgroup size \c n,
 * obtained by multiplying the mean range with this factor.
 */
double nsl_pcm_A4(unsigned int n);

//*************************************************************
//**** Scaling factors for charts for Standard Deviations *****
//*************************************************************
/*!
 * scaling factor B3 used to define the lower limit in the S chart for the subgroup size \c n,
 * obtained by multiplying the mean standard deviation with this factor.
 */
double nsl_pcm_B3(unsigned int n);

/*!
 * scaling factor B3 used to define the upper limit in the S chart for the subgroup size \c n,
 * obtained by multiplying the mean standard deviation with this factor.
 */
double nsl_pcm_B4(unsigned int n);

double nsl_pcm_B5(unsigned int n);

double nsl_pcm_B6(unsigned int n);

//*************************************************************
//*********************** Conversion factors ******************
//*************************************************************
// The factors below convert average, median and standard deviations of ranges
// into appropriate measures of dispersion (Sigma(X), Sigma(XBar), Sigma(R)).

/*!
 * a bias correction factor for converting ranges into Sigma(X) for the subgroup size \c n.
 */
double nsl_pcm_d2(unsigned int n);

/*!
 * a bias correction factor for converting ranges into Sigma(R) for the subgroup size \c n.
 */
double nsl_pcm_d3(unsigned int n);

/*!
 * a bias correction factor for converting median ranges into Sigma(X) for the subgroup size \c n.
 */
double nsl_pcm_d4(unsigned int n);

/*!
 * a bias correction factor for converting mean standard deviations into Sigma(X) for the subgroup size \c n.
 */
double nsl_pcm_c4(unsigned int n);

#endif /* NSL_PCM_H */
