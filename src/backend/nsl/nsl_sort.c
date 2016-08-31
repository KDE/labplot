/***************************************************************************
    File                 : nsl_sort.c
    Project              : LabPlot
    Description          : NSL sorting functions
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

#include <stdlib.h>
#include "nsl_sort.h"

int nsl_sort_compare_size_t(const void* a, const void* b) {
	size_t _a = * ( (size_t*) a );
	size_t _b = * ( (size_t*) b );

	/* an easy expression for comparing */
	return (_a > _b) - (_a < _b);
}

void nsl_sort_size_t(size_t array[], const size_t n) {
	qsort(array, n, sizeof(size_t), nsl_sort_compare_size_t);
}

