/*
    File                 : nsl_hilbert.h
    Project              : LabPlot
    Description          : NSL Hilbert transform
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2021 Stefan Gerlach (stefan.gerlach@uni.kn)

*/

/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
