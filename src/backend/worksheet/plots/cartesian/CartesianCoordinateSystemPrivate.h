
/***************************************************************************
    File                 : CartesianCoordinateSystemPrivate.h
    Project              : LabPlot
    Description          : Cartesian coordinate system for plots.
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2012-2016 Alexander Semke <alexander.semke@web.de>
    SPDX-FileCopyrightText: 2020 Stefan Gerlach <stefan.gerlach@uni.kn>

 **************************************************************************
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CARTESIANCOORDINATESYSTEMPRIVATE_H
#define CARTESIANCOORDINATESYSTEMPRIVATE_H

class CartesianCoordinateSystemPrivate {
public:
	explicit CartesianCoordinateSystemPrivate(CartesianCoordinateSystem *owner);
	~CartesianCoordinateSystemPrivate();

	CartesianCoordinateSystem* const q;
	CartesianPlot* plot{nullptr};
	QVector<CartesianScale*> xScales;
	QVector<CartesianScale*> yScales;
	int xIndex{0}, yIndex{0};	// indices of x/y plot ranges used here
};

#endif
