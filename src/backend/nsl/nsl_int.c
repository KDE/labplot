/***************************************************************************
    File                 : nsl_int.c
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

/* TODO:
	* implement all rules
	* absolute area
	* sum of all values
*/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <float.h>
#include "nsl_int.h"

const char* nsl_int_method_name[] = {"rectangle (1-point)", "trapezoid (2-point)", "Simpson (3-point)", "Simpson 3/8 (4-point)"};

int nsl_int_rectangle(const double *x, double *y, const size_t n) {
	/* TODO */

	return 0;
}
