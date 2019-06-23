/***************************************************************************
    File                 : nsl_geom.h
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

#ifndef NSL_GEOM_H
#define NSL_GEOM_H

/* point-point distance
	point (x1,y1) to (x2,y2)
 */
double nsl_geom_point_point_dist(double x1, double y1, double x2, double y2);

/* point-line distance sqrt(dx^2+dy^2)
	point (xp,yp) to line (x1,y1)-(x2,y2)
 */
double nsl_geom_point_line_dist(double x1, double y1, double x2, double y2, double xp, double yp);

/* point-line distance |dy|
	point (xp,yp) to line (x1,y1)-(x2,y2)
 */
double nsl_geom_point_line_dist_y(double x1, double y1, double x2, double y2, double xp, double yp);

/* area of triangle defined by three points */
double nsl_geom_three_point_area(double x1, double y1, double x2, double y2, double x3, double y3);

/* point-point distance in 3d
        point (x1,y1,z1) to (x2,y2,z2)
 */
double nsl_geom_point_point_dist3(double x1, double y1, double z1, double x2, double y2, double z2);

#endif /* NSL_GEOM_H */
