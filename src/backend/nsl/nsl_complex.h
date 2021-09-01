/*
    File                 : nsl_complex.h
    Project              : LabPlot
    Description          : NSL complex data type support
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2019 Stefan Gerlach <stefan.gerlach@uni.kn>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef NSL_COMPLEX_H
#define NSL_COMPLEX_H

#ifdef _MSC_VER
#include <complex.h>
#define COMPLEX _Dcomplex
#else

/* C++ including this header */
#ifdef __cplusplus
#define COMPLEX double _Complex
#else	/* C */
#include <complex.h>
#define COMPLEX double complex
#endif

#endif	/* _MSC_VER */

#endif /* NSL_COMPLEX_H */
