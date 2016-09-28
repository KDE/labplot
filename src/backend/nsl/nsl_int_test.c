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
	const double xdata[]={1,2,3,5,7};
	double ydata[]={1,1,1,1,1};
	const int n=5;

	printf("data:\n");
	size_t i;
	for (i=0; i < n; i++)
		printf("%g %g\n", xdata[i], ydata[i]);
	puts("");

	printf("expecting ??? as integral (rectangle, 1-point):\n");
	double sum = nsl_int_rectangle(xdata, ydata, n);

	for (i=0; i < n; i++)
		printf("%g %g (%g)\n", xdata[i], ydata[i], xdata[i]);
	printf("sum = %g\n", sum);

	return 0;
}
