/***************************************************************************
    File                 : nsl_int_test.c
    Project              : LabPlot
    Description          : NSL numerical integration functions
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
#include "nsl_int.h"

int main() {
	double xdata[]={1,2,3,5,7};
	double ydata[]={2,2,2,-2,-2};
	int n=5;

	printf("data:\n");
	size_t i;
	for (i = 0; i < n; i++)
		printf("%g %g\n", xdata[i], ydata[i]);
	puts("");

	printf("integral (rectangle, 1-point):\n");
	int status = nsl_int_rectangle(xdata, ydata, n, 0);

	for (i = 0; i < n; i++)
		printf("%g %g\n", xdata[i], ydata[i]);
	printf("sum = %g (status = %d)\n", ydata[n-1], status);
	puts("");

	printf("area (rectangle, 1-point):\n");
	double ydata4[]={2,2,2,2,2};
	status = nsl_int_rectangle(xdata, ydata4, n, 1);

	for (i = 0; i < n; i++)
		printf("%g %g\n", xdata[i], ydata4[i]);
	printf("sum = %g (status = %d)\n", ydata4[n-1], status);
	puts("");

	printf("integral (trapezoid, 2-point):\n");
	double ydata2[]={1,2,3,-1,-3};
	status = nsl_int_trapezoid(xdata, ydata2, n, 0);

	for (i = 0; i < n; i++)
		printf("%g %g\n", xdata[i], ydata2[i]);
	printf("sum = %g (status = %d)\n", ydata2[n-1], status);
	puts("");

	printf("area (trapezoid, 2-point):\n");
	double ydata3[]={1,2,3,-1,-3};
	status = nsl_int_trapezoid(xdata, ydata3, n, 1);

	for (i = 0; i < n; i++)
		printf("%g %g\n", xdata[i], ydata3[i]);
	printf("sum = %g (status = %d)\n", ydata3[n-1], status);
	puts("");

	printf("integral (Simpson's 1/3, 3-point):\n");
	/*double ydata5[]={2,2,2,2,2};*/
	double ydata5[]={1,2,3,-1,-3};
	size_t np = nsl_int_simpson(xdata, ydata5, n, 0);

	for (i = 0; i < np; i++)
		printf("%g %g\n", xdata[i], ydata5[i]);
	printf("sum = %g (n = %zu, status = %d)\n", ydata5[np-1], np, status);
	puts("");

	printf("integral (Simpson's 3/8, 4-point):\n");
	double xdata2[]={1,2,3,5,7,8,9};
	double ydata6[]={2,2,2,2,2,2,2};
	/*double ydata6[]={1,2,3,4,5,-1,-2};*/
	n=7;
	np = nsl_int_simpson_3_8(xdata2, ydata6, n, 0);

	for (i = 0; i < np; i++)
		printf("%g %g\n", xdata2[i], ydata6[i]);
	printf("sum = %g (n = %zu, status = %d)\n", ydata6[np-1], np, status);
	puts("");

	return 0;
}
