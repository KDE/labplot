/***************************************************************************
    File                 : nsl_dft_test.c
    Project              : LabPlot
    Description          : NSL discrete Fourier transform functions
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
#include "nsl_dft.h"

#define NN 10
#define TWOSIDED 0	/* 0,1 */

int main() {
	double data[]={1, 1, 3, 3, 1, -1, 0, 1, 1, 0};
	/*double data[]={1, 2, 3, 3, 1};*/
	const int N=10;

#if TWOSIDED == 1
	int size=N;
#else
	int size=N/2;
#endif

	/* input */
	int i;
	for(i=0; i < N; i++)
		printf("%g ", data[i]);
	puts("\nraw:");

	// 'raw' result
	nsl_dft_transform(data, 1, N, TWOSIDED, nsl_dft_result_raw);

	/* output */
	for(i=0; i < size; i++)
		printf("%g ", data[i]);
	puts("\nreal:");

	double data2[]={1, 1, 3, 3, 1, -1, 0, 1, 1, 0};
	/*double data2[]={1, 2, 3, 3, 1};*/
	nsl_dft_transform(data2, 1, N, TWOSIDED, nsl_dft_result_real);

	/* output */
	for(i=0; i < size; i++)
		printf("%g ", data2[i]);
	puts("\nimag:");

	double data3[]={1, 1, 3, 3, 1, -1, 0, 1, 1, 0};
	/*double data3[]={1, 2, 3, 3, 1};*/
	nsl_dft_transform(data3, 1, N, TWOSIDED, nsl_dft_result_imag);

	/* output */
	for(i=0; i < size; i++)
		printf("%g ", data3[i]);
	puts("\nmagnitude:");

	double data4[]={1, 1, 3, 3, 1, -1, 0, 1, 1, 0};
	/*double data4[]={1, 2, 3, 3, 1};*/
	nsl_dft_transform(data4, 1, N, TWOSIDED, nsl_dft_result_magnitude);

	/* output */
	for(i=0; i < size; i++)
		printf("%g ", data4[i]);
	puts("\namplitude:");

	double data5[]={1, 1, 3, 3, 1, -1, 0, 1, 1, 0};
	/*double data5[]={1, 2, 3, 3, 1};*/
	nsl_dft_transform(data5, 1, N, TWOSIDED, nsl_dft_result_amplitude);

	/* output */
	for(i=0; i < size; i++)
		printf("%g ", data5[i]);
	puts("\npower:");

	double data6[]={1, 1, 3, 3, 1, -1, 0, 1, 1, 0};
	/*double data6[]={1, 2, 3, 3, 1};*/
	nsl_dft_transform(data6, 1, N, TWOSIDED, nsl_dft_result_power);

	/* output */
	for(i=0; i < size; i++)
		printf("%g ", data6[i]);
	puts("\nphase:");

	double data7[]={1, 1, 3, 3, 1, -1, 0, 1, 1, 0};
	/*double data7[]={1, 2, 3, 3, 1};*/
	nsl_dft_transform(data7, 1, N, TWOSIDED, nsl_dft_result_phase);

	/* output */
	for(i=0; i < size; i++)
		printf("%g ", data7[i]);
	puts("\ndB:");

	double data8[]={1, 1, 3, 3, 1, -1, 0, 1, 1, 0};
	/*double data8[]={1, 2, 3, 3, 1};*/
	nsl_dft_transform(data8, 1, N, TWOSIDED, nsl_dft_result_dB);

	/* output */
	for(i=0; i < size; i++)
		printf("%g ", data8[i]);
	puts("\nsquare magnitude:");

	double data9[]={1, 1, 3, 3, 1, -1, 0, 1, 1, 0};
	nsl_dft_transform(data9, 1, N, TWOSIDED, nsl_dft_result_squaremagnitude);

	/* output */
	for(i=0; i < size; i++)
		printf("%g ", data9[i]);
	puts("\nsquare amplitude:");

	double data10[]={1, 1, 3, 3, 1, -1, 0, 1, 1, 0};
	nsl_dft_transform(data10, 1, N, TWOSIDED, nsl_dft_result_squareamplitude);

	/* output */
	for(i=0; i < size; i++)
		printf("%g ", data10[i]);
	puts("\nnormdB:");

	double data11[]={1, 1, 3, 3, 1, -1, 0, 1, 1, 0};
	nsl_dft_transform(data11, 1, N, TWOSIDED, nsl_dft_result_normdB);

	/* output */
	for(i=0; i < size; i++)
		printf("%g ", data11[i]);
	puts("");
}
