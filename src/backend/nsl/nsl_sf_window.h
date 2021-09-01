/*
    File                 : nsl_sf_window.h
    Project              : LabPlot
    Description          : NSL special window functions
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016 Stefan Gerlach <stefan.gerlach@uni.kn>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef NSL_SF_WINDOW_H
#define NSL_SF_WINDOW_H

#include <stdlib.h>

#define NSL_SF_WINDOW_TYPE_COUNT 15
typedef enum {nsl_sf_window_uniform, nsl_sf_window_triangle, nsl_sf_window_triangleII, nsl_sf_window_triangleIII, 
	nsl_sf_window_welch, nsl_sf_window_hann, nsl_sf_window_hamming, nsl_sf_window_blackman, nsl_sf_window_nuttall,
	nsl_sf_window_blackman_nuttall, nsl_sf_window_blackman_harris, nsl_sf_window_flat_top, nsl_sf_window_cosine,
	nsl_sf_window_bartlett_hann, nsl_sf_window_lanczos} nsl_sf_window_type;
extern const char* nsl_sf_window_type_name[];

/* u range: [0:1] or [0:N-1] ? */

/* uniform */
int nsl_sf_apply_window(double data[], size_t N, nsl_sf_window_type type);

#endif /* NSL_SF_WINDOW_H */
