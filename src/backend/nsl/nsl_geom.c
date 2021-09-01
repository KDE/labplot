/*
    File                 : nsl_geom.c
    Project              : LabPlot
    Description          : NSL geometry functions
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016 Stefan Gerlach <stefan.gerlach@uni.kn>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


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

double nsl_geom_point_point_dist3(double x1, double y1, double z1, double x2, double y2, double z2) {
	return gsl_hypot3(x2-x1, y2-y1, z2-z1);
}
