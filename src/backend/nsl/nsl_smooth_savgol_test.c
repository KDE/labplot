/***************************************************************************
    File                 : nsl_smooth_savgol_test.c
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
#include "nsl_smooth.h"

int main() {
	/* savgol coefficients */
	int i,j,points=3, order=1;
        gsl_matrix *h;
	
	h = gsl_matrix_alloc(points, points);
	nsl_smooth_savgol_coeff(points, order, h);
	for(j=0;j<points;j++)
		printf(" %g",3*gsl_matrix_get(h,(points-1)/2,j));
	printf("\n");
        gsl_matrix_free(h);

	points=5;
	h = gsl_matrix_alloc(points, points);
	nsl_smooth_savgol_coeff(points, order, h);
	for(j=0;j<points;j++)
		printf(" %g",5*gsl_matrix_get(h,(points-1)/2,j));
	printf("\n");
	order=3;
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

	/* Savitzky Golay with different modes */
	printf("\n");
	int m=5;
	order=2;

	double data[9]={2,2,5,2,1,0,1,4,9};
	printf("mode:interp\n");
	nsl_smooth_savgol(data,9,m,order,nsl_smooth_pad_interp);
	for(i=0;i<9;i++)
		printf(" %7.14f",data[i]);
	printf("\n");

	double data2[9]={2,2,5,2,1,0,1,4,9};
	printf("mode:mirror\n");
	nsl_smooth_savgol(data2,9,m,order,nsl_smooth_pad_mirror);
	for(i=0;i<9;i++)
		printf(" %7.14f",data2[i]);
	printf("\n");

	double data3[9]={2,2,5,2,1,0,1,4,9};
	printf("mode:nearest\n");
	nsl_smooth_savgol(data3,9,m,order,nsl_smooth_pad_nearest);
	for(i=0;i<9;i++)
		printf(" %7.14f",data3[i]);
	printf("\n");

	double data4[9]={2,2,5,2,1,0,1,4,9};
	printf("mode:constant\n");
	nsl_smooth_savgol(data4,9,m,order,nsl_smooth_pad_constant);
	for(i=0;i<9;i++)
		printf(" %7.14f",data4[i]);
	printf("\n");

	double data5[9]={2,2,5,2,1,0,1,4,9};
	printf("mode:wrap\n");
	nsl_smooth_savgol(data5,9,m,order,nsl_smooth_pad_periodic);
	for(i=0;i<9;i++)
		printf(" %7.14f",data5[i]);
	printf("\n");
}
