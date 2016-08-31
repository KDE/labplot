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

#define NSL_GEOM_LINESIM_TYPE_COUNT 7
typedef enum {nsl_geom_linesim_type_douglas_peucker, nsl_geom_linesim_type_nthpoint, nsl_geom_linesim_type_raddist, nsl_geom_linesim_type_perpdist,
	nsl_geom_linesim_type_reumann_witkam, nsl_geom_linesim_type_opheim, nsl_geom_linesim_type_lang} nsl_geom_linesim_type;
extern const char* nsl_geom_linesim_type_name[];

/* calculates positional error (sum of squared perpendicular distance)
	of simplified set (given by index[])
*/
double nsl_geom_linesim_positional_squared_error(const double xdata[], const double ydata[], const size_t n, const size_t index[]);

/* simple n-th point line simplification
	n: number of points
	step: step size
	index: index of reduced points
	-> returns final number of points
*/
size_t nsl_geom_linesim_nthpoint(const size_t n, const size_t step, size_t index[]);

/* radial distance line simplification
	xdata, ydata: data points
	n: number of points
	eps: tolerance (radius)
	index: index of reduced points
	-> returns final number of points
*/
size_t nsl_geom_linesim_raddist(const double xdata[], const double ydata[], const size_t n, const double eps, size_t index[]);

/* perpendicular distance line simplification
	xdata, ydata: data points
	n: number of points
	eps: tolerance (perp. distance)
	index: index of reduced points
	-> returns final number of points
*/
size_t nsl_geom_linesim_perpdist(const double xdata[], const double ydata[], const size_t n, const double eps, size_t index[]);
/* repeat perpendicular distance line simplification
	repeat: number of repeats
 */
size_t nsl_geom_linesim_perpdist_repeat(const double xdata[], const double ydata[], const size_t n, const double eps, const size_t repeat, size_t index[]);

/* Reumann-Witkam line simplification
	xdata, ydata: data points
	n: number of points
	eps: tolerance (perp. distance)
	index: index of reduced points
	-> returns final number of points
*/
size_t nsl_geom_linesim_reumann_witkam(const double xdata[], const double ydata[], const size_t n, const double eps, size_t index[]);

/* Opheim line simplification
	xdata, ydata: data points
	n: number of points
	mineps: minimum tolerance (to define ray)
	maxeps: maxmimum tolerance (to define next key)
	index: index of reduced points
	-> returns final number of points
*/
size_t nsl_geom_linesim_opheim(const double xdata[], const double ydata[], const size_t n, const double mineps, const double maxeps, size_t index[]);

/* Lang line simplification
	xdata, ydata: data points
	n: number of points
	eps: minimum tolerance (perpendicular distance)
	region: search region (number of points)
	index: index of reduced points
	-> returns final number of points
*/
size_t nsl_geom_linesim_lang(const double xdata[], const double ydata[], const size_t n, const double eps, const size_t region, size_t index[]);

/* Douglas-Peucker line simplification
	xdata, ydata: data points
	n: number of points
	eps: minimum tolerance (perpendicular distance)
	region: search region (number of points)
	index: index of reduced points
	-> returns final number of points
*/
size_t nsl_geom_linesim_douglas_peucker(const double xdata[], const double ydata[], const size_t n, const double eps, size_t index[]);

#endif /* NSL_GEOM_LINESIM_H */
