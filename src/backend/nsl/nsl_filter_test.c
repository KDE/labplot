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

print_data(double data[], int n) {
	int i;
	for(i=0; i < n; i++)
		printf("%g ", data[i]);
	puts("");
}

int main() {
        double data[]={1, 2, 3, 3, 1, -1, 0, 1, 1, 0};
        /*double data[]={1, 1, 3, 3, 1, -1, 0, 1, 1};*/
        /*double data[]={1, 2, 3, 3, 1};*/
        const int N=10;
        /*const int N=9;*/

	print_data(data, N);
	nsl_filter_fourier(data, N, nsl_filter_type_high_pass, nsl_filter_form_ideal, 0, 0, 2);
	/*nsl_filter_fourier(data, N, nsl_filter_type_low_pass, nsl_filter_form_ideal, 0, 3, 2);*/
	/*nsl_filter_fourier(data, N, nsl_filter_type_high_pass, nsl_filter_form_ideal, 0, 3, 2);*/
	/*nsl_filter_fourier(data, N, nsl_filter_type_band_pass, nsl_filter_form_ideal, 0, 2, 2);*/
	/*nsl_filter_fourier(data, N, nsl_filter_type_band_reject, nsl_filter_form_ideal, 0, 2, 2);*/
	/*nsl_filter_fourier(data, N, nsl_filter_type_low_pass, nsl_filter_form_butterworth, 1, 2, 2);*/
	/*nsl_filter_fourier(data, N, nsl_filter_type_high_pass, nsl_filter_form_butterworth, 1, 2, 2);*/
	/*nsl_filter_fourier(data, N, nsl_filter_type_band_pass, nsl_filter_form_butterworth, 2, 2, 2);*/
	print_data(data, N);

	return 0;
}
