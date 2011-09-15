/***************************************************************************
    File                 : Errorbar.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Stefan Gerlach
    Email (use @ for *)  : stefan.gerlach*uni-konstanz.de
    Description          : error bar class
                           
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

#ifndef ERRORBAR_H
#define ERRORBAR_H

#include <QColor>
#include <QDomElement>
#include "errorbars.h"
#include "defs.h"

class Errorbar {
public:
	Errorbar(QColor c=Qt::blue, int xs=2, int ys=2, Qt::PenStyle st=Qt::SolidLine, int w=1, EType xt=EFLAT, EType yt=EFLAT, 
		QColor bc=Qt::blue, int bw=1, Qt::PenStyle bst=Qt::SolidLine);
	QDomElement saveXML(QDomDocument doc);
	void openXML(QDomNode node);
//	void draw(QPainter *p, QPointArray pa, QPointArray hpa, QPointArray vpa);

	ACCESS(QColor, color, Color);
	ACCESS(QColor, basecolor, BaseColor);
	ACCESS(int, xsize, XSize);
	ACCESS(int, ysize, YSize);
	ACCESS(Qt::PenStyle, style, Style);
	ACCESS(Qt::PenStyle, basestyle, BaseStyle);
	ACCESS(int, width, Width);
	ACCESS(int, basewidth, BaseWidth);
	ACCESS(EType, xtype, XType);
	ACCESS(EType, ytype, YType);
private:
	QColor color;			// errorbar settings
	int xsize, ysize;
	Qt::PenStyle style;
	int width;
	EType xtype, ytype;
	QColor basecolor;		// base settings
	int basewidth;
	Qt::PenStyle basestyle;
};

#endif //ERRORBAR_H
