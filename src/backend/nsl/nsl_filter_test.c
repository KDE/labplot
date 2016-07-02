/***************************************************************************
    File                 : nsl_filter_test.c
    Project              : LabPlot
    Description          : NSL filter test
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
#include "nsl_filter.h"

double main() {
        double data[]={1, 1, 3, 3, 1, -1, 0, 1, 1, 0};
        /*double data[]={1, 1, 3, 3, 1, -1, 0, 1, 1};*/
        /*double data[]={1, 2, 3, 3, 1};*/
        const int N=10;
        /*const int N=9;*/

	int i;
	for(i=0; i < N; i++)
		printf("%g ", data[i]);
	puts("");

	nsl_filter_transform(data, N);

	for(i=0; i < N; i++)
		printf("%g ", data[i]);
	puts("");
	printf("real: %g ",data[0]);
	for(i=0; i < N/2; i++)
		printf("%g ", data[2*i+1]);
	puts("");
	printf("imag: 0 ",data[0]);
	for(i=1; i < N/2; i++)
		printf("%g ", data[2*i]);
	if(N%2)
		printf("%g ", data[N-1]);
	else
		printf("0");
	puts("");

	nsl_filter_backtransform(data, N);

	for(i=0; i < N; i++)
		printf("%g ", data[i]);
	puts("");
}
