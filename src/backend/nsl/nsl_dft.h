/***************************************************************************
    File                 : nsl_dft.h
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

#ifndef NSL_DFT_H
#define NSL_DFT_H

#include <stdlib.h>

/* DFT result type */
#define NSL_DFT_RESULT_TYPE_COUNT 10
typedef enum {nsl_dft_result_magnitude, nsl_dft_result_amplitude, nsl_dft_result_real, nsl_dft_result_imag, nsl_dft_result_power, 
	nsl_dft_result_phase, nsl_dft_result_dB, nsl_dft_result_squaremagnitude, nsl_dft_result_squareamplitude, nsl_dft_result_raw} nsl_dft_result_type;
extern const char* nsl_dft_result_type_name[];

/* transform data of size n. result in data */
int nsl_dft_transform(double data[], size_t stride, size_t n, nsl_dft_result_type type);

#endif /* NSL_DFT_H */
