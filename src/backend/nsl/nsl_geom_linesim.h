/*
    File                 : nsl_geom_linesim.h
    Project              : LabPlot
    Description          : NSL geometry line simplification functions
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016 Stefan Gerlach <stefan.gerlach@uni.kn>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


/*
	TODO:
	* accelerate Visvalingam-Whyatt
	* calculate error statistics
	* more algorithms: Jenks, Zhao-Saalfeld
	* non-parametric version of Visvalingam-Whyatt, Opheim and Lang
*/

#ifndef NSL_GEOM_LINESIM_H
#define NSL_GEOM_LINESIM_H

#include <stdlib.h>

#define NSL_GEOM_LINESIM_TYPE_COUNT 10
typedef enum {nsl_geom_linesim_type_douglas_peucker_variant, nsl_geom_linesim_type_douglas_peucker, nsl_geom_linesim_type_visvalingam_whyatt,
	nsl_geom_linesim_type_reumann_witkam, nsl_geom_linesim_type_perpdist, nsl_geom_linesim_type_nthpoint, nsl_geom_linesim_type_raddist,
	nsl_geom_linesim_type_interp, nsl_geom_linesim_type_opheim, nsl_geom_linesim_type_lang} nsl_geom_linesim_type;
extern const char* nsl_geom_linesim_type_name[];

/*********** error calculation functions *********/

/* calculates positional error (sum of perpendicular distance per point)
	of simplified set (given by index[])
*/
double nsl_geom_linesim_positional_error(const double xdata[], const double ydata[], const size_t n, const size_t index[]);
/* calculates positional error (sum of squared perpendicular distance per point)
	of simplified set (given by index[])
*/
double nsl_geom_linesim_positional_squared_error(const double xdata[], const double ydata[], const size_t n, const size_t index[]);

/* calculates area error (area between original and simplified data per point)
	of simplified set (given by index[])
*/
double nsl_geom_linesim_area_error(const double xdata[], const double ydata[], const size_t n, const size_t index[]);

/* calculates tolerance by using diagonal distance of all data point clip area
	divided by n */
double nsl_geom_linesim_clip_diag_perpoint(const double xdata[], const double ydata[], const size_t n);

/* calculates tolerance from clip area
	divided by n */
double nsl_geom_linesim_clip_area_perpoint(const double xdata[], const double ydata[], const size_t n);

/* calculates tolerance from average distance of following point
	divided by n */
double nsl_geom_linesim_avg_dist_perpoint(const double xdata[], const double ydata[], const size_t n);

/*********** simplification algorithms *********/

/* Douglas-Peucker line simplification
	xdata, ydata: data points
	n: number of points
	tol: minimum tolerance (perpendicular distance)
	index: index of reduced points
	-> returns final number of points
*/
size_t nsl_geom_linesim_douglas_peucker(const double xdata[], const double ydata[], const size_t n, const double tol, size_t index[]);
size_t nsl_geom_linesim_douglas_peucker_auto(const double xdata[], const double ydata[], const size_t n, size_t index[]);
/* Douglas-Peucker variant resulting in a given number of points
	xdata, ydata: data points
	n: number of points
	nout: number of output points
	index: index of reduced points
	-> returns perpendicular distance of last added point (upper limit for all remaining points)
*/
double nsl_geom_linesim_douglas_peucker_variant(const double xdata[], const double ydata[], const size_t n, const size_t nout, size_t index[]);

/* simple n-th point line simplification
	n: number of points
	step: step size
	index: index of reduced points
	-> returns final number of points
*/
size_t nsl_geom_linesim_nthpoint(const size_t n, const int step, size_t index[]);

/* radial distance line simplification
	xdata, ydata: data points
	n: number of points
	tol: tolerance (radius)
	index: index of reduced points
	-> returns final number of points
*/
size_t nsl_geom_linesim_raddist(const double xdata[], const double ydata[], const size_t n, const double tol, size_t index[]);
size_t nsl_geom_linesim_raddist_auto(const double xdata[], const double ydata[], const size_t n, size_t index[]);

/* perpendicular distance line simplification
	xdata, ydata: data points
	n: number of points
	tol: tolerance (perp. distance)
	index: index of reduced points
	-> returns final number of points
*/
size_t nsl_geom_linesim_perpdist(const double xdata[], const double ydata[], const size_t n, const double tol, size_t index[]);
size_t nsl_geom_linesim_perpdist_auto(const double xdata[], const double ydata[], const size_t n, size_t index[]);
/* repeat perpendicular distance line simplification
	repeat: number of repeats
 */
size_t nsl_geom_linesim_perpdist_repeat(const double xdata[], const double ydata[], const size_t n, const double tol, const size_t repeat, size_t index[]);

/* line simplification by nearest neigbor interpolation	(idea from xmgrace)
	xdata, ydata: data points
	n: number of points
	tol: tolerance (perp. distance)
	index: index of reduced points
	-> returns final number of points
*/
size_t nsl_geom_linesim_interp(const double xdata[], const double ydata[], const size_t n, const double tol, size_t index[]);
size_t nsl_geom_linesim_interp_auto(const double xdata[], const double ydata[], const size_t n, size_t index[]);

/* Visvalingam-Whyatt line simplification
	xdata, ydata: data points
	n: number of points
	tol: tolerance (area)
	index: index of reduced points
	-> returns final number of points
*/
size_t nsl_geom_linesim_visvalingam_whyatt(const double xdata[], const double ydata[], const size_t n, const double tol, size_t index[]);
size_t nsl_geom_linesim_visvalingam_whyatt_auto(const double xdata[], const double ydata[], const size_t n, size_t index[]);

/* Reumann-Witkam line simplification
	xdata, ydata: data points
	n: number of points
	tol: tolerance (perp. distance)
	index: index of reduced points
	-> returns final number of points
*/
size_t nsl_geom_linesim_reumann_witkam(const double xdata[], const double ydata[], const size_t n, const double tol, size_t index[]);
size_t nsl_geom_linesim_reumann_witkam_auto(const double xdata[], const double ydata[], const size_t n, size_t index[]);

/* Opheim line simplification
	xdata, ydata: data points
	n: number of points
	mintol: minimum tolerance (to define ray)
	maxtol: maximum tolerance (to define next key)
	index: index of reduced points
	-> returns final number of points
*/
size_t nsl_geom_linesim_opheim(const double xdata[], const double ydata[], const size_t n, const double mintol, const double maxtol, size_t index[]);
size_t nsl_geom_linesim_opheim_auto(const double xdata[], const double ydata[], const size_t n, size_t index[]);

/* Lang line simplification
	xdata, ydata: data points
	n: number of points
	tol: minimum tolerance (perpendicular distance)
	region: search region (number of points)
	index: index of reduced points
	-> returns final number of points
*/
size_t nsl_geom_linesim_lang(const double xdata[], const double ydata[], const size_t n, const double tol, const size_t region, size_t index[]);
size_t nsl_geom_linesim_lang_auto(const double xdata[], const double ydata[], const size_t n, size_t index[]);

#endif /* NSL_GEOM_LINESIM_H */
