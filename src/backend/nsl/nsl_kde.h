/***************************************************************************
    File                 : nsl_sort.h
    Project              : LabPlot
    Description          : NSL functions for the kernel density estimation
    --------------------------------------------------------------------
    Copyright            : (C) 2021 by Alexander Semke (alexander.semke@web.de)

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

#ifndef NSL_KDE_H
#define NSL_KDE_H

/* calculates the density at point x for the sample data with the bandwith h */
double nsl_kde(const double* data, double x, double h, size_t n);

/* calculates the "normal distribution approximation" bandwidth */
double nsl_kde_normal_dist_bandwith(double* data, int n);

#endif /* NSL_KDE_H */
