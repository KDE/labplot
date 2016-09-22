/***************************************************************************
    File                 : nsl_diff_test.c
    Project              : LabPlot
    Description          : NSL numerical differentiation functions
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
#include "nsl_diff.h"

int main() {
	const double xdata[]={1,2,4,8,16,32,64};
	const int n=7;

	/*printf("function x^2/x^3:\n");
	for (i=0; i < n; i++)
		printf("%g %g\n", xdata[i], ydata[i]);
	puts("");*/

	printf("expecting 2*x as derivative (second order):\n");
	double ydata[]={1,4,16,64,256,1024,4096};
	/*int status = nsl_diff_deriv_first_equal(xdata, ydata, n);*/
	int status = nsl_diff_first_deriv(xdata, ydata, n, 2);

	size_t i;
	for (i=0; i < n; i++)
		printf("%g %g (%g)\n", xdata[i], ydata[i], 2.*xdata[i]);
	puts("");

	printf("expecting 3*x^2 as derivative (fourth order):\n");
	double ydata4[]={1,8,64,512,4096,32768,262144};
	status = nsl_diff_first_deriv(xdata, ydata4, n, 4);

	for (i=0; i < n; i++)
		printf("%g %g (%g)\n", xdata[i], ydata4[i], 3.*xdata[i]*xdata[i]);
	puts("");

	printf("avg derivative:\n");
	double ydata2[]={1,4,16,64,256,1024,4096};
	status = nsl_diff_first_deriv_avg(xdata, ydata2, n);

	for (i=0; i < n; i++)
		printf("%g %g\n", xdata[i], ydata2[i]);
	puts("");

	printf("expecting 2 as second derivative (first order):\n");
	double ydata3[]={1,4,16,64,256,1024,4096};
	status = nsl_diff_second_deriv(xdata, ydata3, n, 1);

	for (i=0; i < n; i++)
		printf("%g %g\n", xdata[i], ydata3[i]);
	puts("");

	printf("expecting 2 as second derivative (second order):\n");
	double ydata5[]={1,4,16,64,256,1024,4096};
	status = nsl_diff_second_deriv(xdata, ydata5, n, 2);

	for (i=0; i < n; i++)
		printf("%g %g\n", xdata[i], ydata5[i]);
	puts("");

	printf("expecting 6*x as second derivative (third order):\n");
	double ydata6[]={1,8,64,512,4096,32768,262144};
	status = nsl_diff_second_deriv(xdata, ydata6, n, 3);

	for (i=0; i < n; i++)
		printf("%g %g (%g)\n", xdata[i], ydata6[i], 6*xdata[i]);
	puts("");

	printf("expecting 6 as third derivative (second order):\n");
	double ydata7[]={1,8,64,512,4096,32768,262144};
	status = nsl_diff_third_deriv(xdata, ydata7, n, 2);

	for (i=0; i < n; i++)
		printf("%g %g\n", xdata[i], ydata7[i]);
	puts("");

	printf("expecting 0 as fourth derivative (first order):\n");
	double ydata8[]={1,8,64,512,4096,32768,262144};
	status = nsl_diff_fourth_deriv(xdata, ydata8, n, 1);

	for (i=0; i < n; i++)
		printf("%g %g\n", xdata[i], ydata8[i]);
	puts("");

	printf("expecting 0 as fourth derivative (third order):\n");
	double ydata9[]={1,8,64,512,4096,32768,262144};
	status = nsl_diff_fourth_deriv(xdata, ydata9, n, 3);

	for (i=0; i < n; i++)
		printf("%g %g\n", xdata[i], ydata9[i]);
	puts("");

	printf("expecting 0 as fifth derivative (second order):\n");
	double ydata10[]={1,8,64,512,4096,32768,262144};
	status = nsl_diff_fifth_deriv(xdata, ydata10, n, 2);

	for (i=0; i < n; i++)
		printf("%g %g\n", xdata[i], ydata10[i]);
	puts("");

	printf("expecting 0 as sixth derivative (first order):\n");
	double ydata11[]={1,8,64,512,4096,32768,262144};
	status = nsl_diff_six_deriv(xdata, ydata11, n, 1);

	for (i=0; i < n; i++)
		printf("%g %g\n", xdata[i], ydata11[i]);
	puts("");

	return 0;
}
