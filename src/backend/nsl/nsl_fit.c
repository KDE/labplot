/***************************************************************************
    File                 : nsl_fit.c
    Project              : LabPlot
    Description          : NSL (non)linear fit functions
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
	* TODO
*/

#include "nsl_common.h"
#include "nsl_fit.h"
#include <gsl/gsl_math.h>

/* 
	see http://www.quantcode.com/modules/smartfaq/faq.php?faqid=96
	and https://lmfit.github.io/lmfit-py/bounds.html
*/
double nsl_fit_map_bound(double x, double min, double max) {
	if (max <= min) {
		printf("given bounds must fulfill max > min! Giving up.\n");
		return DBL_MAX;
	}

	// not bounded
	if (min == -DBL_MAX && max == DBL_MAX)
		return x;

	// open bounds
	if (min == -DBL_MAX)
		return max + 1 - sqrt(x*x + 1);
	if (max == DBL_MAX)
		return min - 1 + sqrt(x*x + 1);

	return min + (max - min)/(1.0 + exp(-x));
}

/* 
	see http://www.quantcode.com/modules/smartfaq/faq.php?faqid=96
	and https://lmfit.github.io/lmfit-py/bounds.html
*/
double nsl_fit_map_unbound(double x, double min, double max) {
	if (max <= min) {
		printf("given bounds must fulfill max > min! Giving up.\n");
		return DBL_MAX;
	}
	if (x < min || x > max) {
		printf("given value must be within bounds! Giving up.\n");
		return -DBL_MAX;
	}

	// not bounded
        if (min == -DBL_MAX && max == DBL_MAX)
                return x;	

	// open bounds
        if (min == -DBL_MAX)
                return sqrt(gsl_pow_2(max - x + 1) -1);
        if (max == DBL_MAX)
                return sqrt(gsl_pow_2(x - min + 1) -1);

	return -log((max - x)/(x - min));
}
