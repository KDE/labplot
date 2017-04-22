/***************************************************************************
    File                 : nsl_fit_test.c
    Project              : LabPlot
    Description          : NSL (non)linear functions test
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
#include "nsl_fit.h"

int main() {
	const double data[]={-1000, -100, -10, -1, -.1, 0, .1, 1, 10, 100, 1000};
	const int n=11;
	int i;
	for (i=0; i < n; i++) {
		double x = nsl_fit_map_bound(data[i],-1,2);
		printf("%g -> %g\n", data[i], x);
	}
	puts("");

	const double data2[]={-1, -.99, -.5, 0, .49, .5, .51, 1, 1.5, 1.99, 2};
	for (i=0; i < n; i++) {
		double xb = nsl_fit_map_unbound(data2[i],-1,2);
		printf("%g -> %g\n", data2[i], xb);
	}
}
