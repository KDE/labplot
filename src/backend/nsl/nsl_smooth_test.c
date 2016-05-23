/***************************************************************************
    File                 : nsl_smooth_test.c
    Project              : LabPlot
    Description          : NSL smooth functions
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

#include <stdio.h>
//#include <gsl/gsl_statistics_double.h>
#include "nsl_smooth.h"

double main() {
	int i,j,points=5, order=2;
        gsl_matrix *h;
	
	h = gsl_matrix_alloc(points, points);
	nsl_smooth_savgol_coeff(points, order, h);
	for(j=0;j<points;j++)
		printf(" %g",35*gsl_matrix_get(h,(points-1)/2,j));
	printf("\n");
        gsl_matrix_free(h);

	points=7;
        h = gsl_matrix_alloc(points, points);
	nsl_smooth_savgol_coeff(points, order, h);
	for(j=0;j<points;j++)
		printf(" %g",21*gsl_matrix_get(h,(points-1)/2,j));
	printf("\n");
	order=4;
	nsl_smooth_savgol_coeff(points, order, h);
	for(j=0;j<points;j++)
		printf(" %g",231*gsl_matrix_get(h,(points-1)/2,j));
	printf("\n");
        gsl_matrix_free(h);

	order=2;
	points=9;
        h = gsl_matrix_alloc(points, points);
	nsl_smooth_savgol_coeff(points, order, h);
	for(j=0;j<points;j++)
		printf(" %g",231*gsl_matrix_get(h,(points-1)/2,j));
	printf("\n");
	order=4;
	nsl_smooth_savgol_coeff(points, order, h);
	for(j=0;j<points;j++)
		printf(" %g",429*gsl_matrix_get(h,(points-1)/2,j));
	printf("\n");
        gsl_matrix_free(h);
}
