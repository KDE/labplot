/***************************************************************************
    File                 : nsl_geom.c
    Project              : LabPlot
    Description          : NSL geometry functions
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

#include "nsl_geom.h"
#include <gsl/gsl_math.h>

double nsl_geom_point_point_dist(double x1, double y1, double x2, double y2) {
	return gsl_hypot(x2-x1, y2-y1);
}

double nsl_geom_point_line_dist(double x1, double y1, double x2, double y2, double xp, double yp) {
	return fabs( (xp-x1)*(y2-y1) - (x2-x1)*(yp-y1) ) / nsl_geom_point_point_dist(x1, y1, x2, y2);
}

double nsl_geom_point_line_dist_y(double x1, double y1, double x2, double y2, double xp, double yp) {
	return fabs( yp - y1 - (y2-y1)*(xp-x1)/(x2-x1) );
}

double nsl_geom_three_point_area(double x1, double y1, double x2, double y2, double x3, double y3) {
	return fabs( x1*(y2-y3) + x2*(y3-y1) + x3*(y1-y2) ) / 2.;
}
