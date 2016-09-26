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
	double ydata[]={1,4,16,64,256,1024,4096};
	const int n=7;

	printf("function x^2:\n");
	size_t i;
	for (i=0; i < n; i++)
		printf("%g %g\n", xdata[i], ydata[i]);
	puts("");

	printf("expecting 2*x as derivative:\n");
	/*int status = nsl_diff_deriv_first_equal(xdata, ydata, n);*/
	int status = nsl_diff_first_deriv(xdata, ydata, n, 2);

	for (i=0; i < n; i++)
		printf("%g %g\n", xdata[i], ydata[i]);
	puts("");

	printf("avg derivative:\n");
	double ydata2[]={1,4,16,64,256,1024,4096};
	status = nsl_diff_first_deriv_avg(xdata, ydata2, n);

	for (i=0; i < n; i++)
		printf("%g %g\n", xdata[i], ydata2[i]);
	puts("");

	printf("expecting 2 as second derivative:\n");
	double ydata3[]={1,4,16,64,256,1024,4096};
	status = nsl_diff_second_deriv(xdata, ydata3, n, 2);

	for (i=0; i < n; i++)
		printf("%g %g\n", xdata[i], ydata3[i]);

	return 0;
}
