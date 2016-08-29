/***************************************************************************
    File                 : nsl_geom_linesim.h
    Project              : LabPlot
    Description          : NSL geometry line simplification functions
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

#ifndef NSL_GEOM_LINESIM_H
#define NSL_GEOM_LINESIM_H

#include <stdlib.h>

/* simple n-th point line simplification
	n: number of points
	step: step size
	index: index of reduced points
	-> returns final number of points
*/
size_t nsl_geom_linesim_nthpoint(size_t n, size_t step, size_t index[]);


/* radial distance line simplification
	xdata, ydata: data points
	n: number of points
	eps: tolerance (radius)
	index: index of reduced points
	-> returns final number of points
*/
size_t nsl_geom_linesim_raddist(const double xdata[], const double ydata[], size_t n, double eps, size_t index[]);

/* perpendicular distance line simplification
	xdata, ydata: data points
	n: number of points
	eps: tolerance (perp. distance)
	index: index of reduced points
	-> returns final number of points
*/
size_t nsl_geom_linesim_perpdist(const double xdata[], const double ydata[], size_t n, double eps, size_t index[]);
/* repeat perpendicular distance line simplification
	repeat: number of repeats
 */
size_t nsl_geom_linesim_perpdist_repeat(const double xdata[], const double ydata[], size_t n, double eps, size_t repeat, size_t index[]);

/* Reumann-Witkam line simplification
	xdata, ydata: data points
	n: number of points
	eps: tolerance (perp. distance)
	index: index of reduced points
	-> returns final number of points
*/
size_t nsl_geom_linesim_reumann_witkam(const double xdata[], const double ydata[], size_t n, double eps, size_t index[]);

#endif /* NSL_GEOM_LINESIM_H */
