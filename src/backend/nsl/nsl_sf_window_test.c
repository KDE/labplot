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
	const int N=10;

	int i;
	for(i=0; i < N; i++)
		printf("%g ", nsl_sf_window(i,N, nsl_sf_window_uniform));
	puts("\n");
	for(i=0; i < N; i++)
		printf("%g ", nsl_sf_window(i,N, nsl_sf_window_triangle));
	puts("\n");
	for(i=0; i < N; i++)
		printf("%g ", nsl_sf_window(i,N, nsl_sf_window_triangleII));
	puts("\n");
	for(i=0; i < N; i++)
		printf("%g ", nsl_sf_window(i,N, nsl_sf_window_triangleIII));
	puts("\n");
	for(i=0; i < N; i++)
		printf("%g ", nsl_sf_window(i,N, nsl_sf_window_welch));
	puts("\n");
	for(i=0; i < N; i++)
		printf("%g ", nsl_sf_window(i,N, nsl_sf_window_hann));
	puts("\n");
	for(i=0; i < N; i++)
		printf("%g ", nsl_sf_window(i,N, nsl_sf_window_hamming));
	puts("\n");
	for(i=0; i < N; i++)
		printf("%g ", nsl_sf_window(i,N, nsl_sf_window_blackman));
	puts("\n");
	for(i=0; i < N; i++)
		printf("%g ", nsl_sf_window(i,N, nsl_sf_window_nuttall));
	puts("\n");
	for(i=0; i < N; i++)
		printf("%g ", nsl_sf_window(i,N, nsl_sf_window_blackman_nuttall));
	puts("\n");
	for(i=0; i < N; i++)
		printf("%g ", nsl_sf_window(i,N, nsl_sf_window_blackman_harris));
	puts("\n");
	for(i=0; i < N; i++)
		printf("%g ", nsl_sf_window(i,N, nsl_sf_window_flat_top));
	puts("\n");
	for(i=0; i < N; i++)
		printf("%g ", nsl_sf_window(i,N, nsl_sf_window_cosine));
	puts("\n");
	for(i=0; i < N; i++)
		printf("%g ", nsl_sf_window(i,N, nsl_sf_window_bartlett_hann));
	puts("\n");
	for(i=0; i < N; i++)
		printf("%g ", nsl_sf_window(i,N, nsl_sf_window_lanczos));
	puts("\n");

}
