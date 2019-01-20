/***************************************************************************
    File                 : nsl_complex.h
    Project              : LabPlot
    Description          : NSL complex data type support
    --------------------------------------------------------------------
    Copyright            : (C) 2019 by Stefan Gerlach (stefan.gerlach@uni.kn)

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
