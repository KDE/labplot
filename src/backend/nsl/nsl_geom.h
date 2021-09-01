/*
    File                 : nsl_geom.h
    Project              : LabPlot
    Description          : NSL geometry functions
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016 Stefan Gerlach (stefan.gerlach@uni.kn)
    SPDX-License-Identifier: GPL-2.0-or-later
*/


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
