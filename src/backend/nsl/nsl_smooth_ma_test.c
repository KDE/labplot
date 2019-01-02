/***************************************************************************
    File                 : nsl_smooth_ma_test.c
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
	double data[9]={2,2,5,2,1,0,1,4,9};
	int i,points=5;
	nsl_smooth_weight_type weight = nsl_smooth_weight_uniform;

	int status;
	printf("pad_none\n");
	status = nsl_smooth_moving_average(data, 9, points, weight, nsl_smooth_pad_none);
	for(i=0;i<9;i++)
		printf(" %g",data[i]);
	printf("	status = %d\n", status);
	/*printf("pad_interp\n");
	status = nsl_smooth_moving_average(data, 9, points, weight, nsl_smooth_pad_interp);
	*/
	printf("\npad_mirror\n");
	double data2[9]={2,2,5,2,1,0,1,4,9};
	status = nsl_smooth_moving_average(data2, 9, points, weight, nsl_smooth_pad_mirror);
	for(i=0;i<9;i++)
		printf(" %g",data2[i]);
	printf("	status = %d\n", status);
	printf("\npad_nearest\n");
	double data3[9]={2,2,5,2,1,0,1,4,9};
	status = nsl_smooth_moving_average(data3, 9, points, weight, nsl_smooth_pad_nearest);
	for(i=0;i<9;i++)
		printf(" %g",data3[i]);
	printf("	status = %d\n", status);
	printf("\npad_constant\n");
	double data4[9]={2,2,5,2,1,0,1,4,9};
	status = nsl_smooth_moving_average(data4, 9, points, weight, nsl_smooth_pad_constant);
	for(i=0;i<9;i++)
		printf(" %g",data4[i]);
	printf("	status = %d\n", status);
	printf("\npad_periodic\n");
	double data5[9]={2,2,5,2,1,0,1,4,9};
	status = nsl_smooth_moving_average(data5, 9, points, weight, nsl_smooth_pad_periodic);
	for(i=0;i<9;i++)
		printf(" %g",data5[i]);
	printf("	status = %d\n", status);
	puts("");
}
