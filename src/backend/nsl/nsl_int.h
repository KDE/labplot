/***************************************************************************
    File                 : nsl_int.h
    Project              : LabPlot
    Description          : NSL numerical integration functions
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

#ifndef NSL_INT_H
#define NSL_INT_H

#define NSL_INT_NETHOD_COUNT 4
typedef enum {nsl_int_method_rectangular, nsl_int_method_trapezoid, nsl_int_method_Simpson, nsl_int_method_Simpson_3_8} nsl_int_method_type;
extern const char* nsl_int_method_name[];

/*
	numerical integration for non-uniform samples
	rectangular rule (1-point)
	abs - 0:return mathem. area, 1: return absolute area
*/
int nsl_int_rectangle(const double *x, double *y, const size_t n, int abs);
int nsl_int_trapezoid(const double *x, double *y, const size_t n, int abs);

#endif /* NSL_INT_H */
