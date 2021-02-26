/***************************************************************************
    File                 : nsl_hilbert.h
    Project              : LabPlot
    Description          : NSL Hilbert transform
    --------------------------------------------------------------------
    Copyright            : (C) 2021 by Stefan Gerlach (stefan.gerlach@uni.kn)

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

#ifndef NSL_HILBERT_H
#define NSL_HILBERT_H

#include <stdlib.h>
#include "nsl_dft.h"

/* Hilbert transform result type:
 * imag = imaginary part
 * envelope = abs(hilbert())
 *
 * */
#define NSL_HILBERT_RESULT_TYPE_COUNT 2
typedef enum {nsl_hilbert_result_imag, nsl_hilbert_result_envelope} nsl_hilbert_result_type;
extern const char* nsl_hilbert_result_type_name[];

/* transform data of size n. result in data 
	calculates the Hilbert transform using algorithm described in 
	https://de.wikipedia.org/wiki/Hilbert-Transformation#Berechnung_%C3%BCber_Fouriertransformation
*/
int nsl_hilbert_transform(double data[], size_t stride, size_t n, nsl_hilbert_result_type type);

#endif /* NSL_HILBERT_H */
