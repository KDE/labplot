/***************************************************************************
    File                 : nsl_sf_window_test.c
    Project              : LabPlot
    Description          : NSL window functions test
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
#include "nsl_sf_window.h"

int main() {
	const int N = 10;
	double data[] = {1,1,1,1,1,1,1,1,1,1};

	int i;
	nsl_sf_window_type t;

	for (t = nsl_sf_window_uniform; t <= nsl_sf_window_lanczos; t++) {
		nsl_sf_apply_window(data, N, t);
		for(i = 0; i < N; i++)
			printf("%g ", data[i]);
		puts("\n");
	}
}
