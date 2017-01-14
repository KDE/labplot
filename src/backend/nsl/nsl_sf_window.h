/***************************************************************************
    File                 : nsl_sf_window.h
    Project              : LabPlot
    Description          : NSL special window functions
    --------------------------------------------------------------------
    Copyright            : (C) 2016 by Stefan Gerlach (stefan.gerlach@uni.kn)

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/

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
