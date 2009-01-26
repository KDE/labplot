/***************************************************************************
    File                 : Plot2DSimple.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Stefan Gerlach, Alexander Semke
    Email (use @ for *)  : stefan.gerlach*uni-konstanz.de, alexander.semke*web.de
    Description          : simple 2d plot

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
#ifndef PLOT2DSIMPLE_H
#define PLOT2DSIMPLE_H

#include "Plot2D.h"

//! simple 2D Plot	(line, error)
class Plot2DSimple:public Plot2D {
public:
	Plot2DSimple(AbstractScriptingEngine*, const QString &name);
	//	QStringList Info();
	QWidget *view();
	void calculateXY(Point d,double *x, double *y, int w, int h);
	void drawFill(QPainter *p, int w, int h);
	void drawCurves(QPainter *p, const int w, const int h);

};

#endif // PLOT2DSIMPLE_H
