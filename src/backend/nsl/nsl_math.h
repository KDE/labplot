/***************************************************************************
    File                 : nsl_math.h
    Project              : LabPlot
    Description          : NSL math functions
    --------------------------------------------------------------------
    Copyright            : (C) 2018 by Stefan Gerlach (stefan.gerlach@uni.kn)

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

#ifndef NSL_MATH_H
#define NSL_MATH_H

/* get number of decimals */
int nsl_math_decimals(double value);

/* round double value to n decimal places */
double nsl_math_round_places(double value, unsigned int n); 

/* round double value to precision p */
double nsl_math_round_precision(double value, unsigned int p);

#endif /* NSL_MATH_H */
