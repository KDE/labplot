/***************************************************************************
    File                 : Legend.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Stefan Gerlach, Alexander Semke
    Email (use @ for *)  : stefan.gerlach*uni-konstanz.de, alexander.semke*web.de
    Description          : legend class
                           
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
#ifndef LEGEND_H
#define LEGEND_H

#include <QtGui>

#include "../definitions.h"
#include "../elements/Set.h"

class Legend {
public:
	Legend();

	ACCESSFLAG(m_enabled, Enabled);
	ACCESS(QPoint, position, Position);
	ACCESS(Qt::Orientation, orientation, Orientation);

	ACCESSFLAG(m_fillingEnabled, Filling);
	ACCESS(QColor, fillingColor, FillingColor);
	ACCESSFLAG(m_boxEnabled, Box);
	ACCESSFLAG(m_shadowEnabled, Shadow);

	ACCESS(QFont, textFont, TextFont);
	ACCESS(QColor, textColor, TextColor);
  	void draw(QPainter *p, const QList<Set>*, const Point, const Point, const int w, const int h);
	void drawSetLegends(QPainter *p, const QList<Set>*list_Sets, const Point& size, const QFont& tmpfont );

	/*
	void save(QTextStream *t);
	void open(QTextStream *t, int version);
	QDomElement saveXML(QDomDocument doc);
	void openXML(QDomNode node);
*/
	/*
	void setPosition(double X, double Y) { x = X; y = Y; }
	double X() { return x; }
	double Y() { return y; }
	int drawGraphs(QPainter *p, GraphList *gl, PType type, Point size, QFont tmpfont);
	bool inside(int X, int Y);
	int TicLabelLength() { return ticlabellength; }
	void setTicLabelLength(int l) { ticlabellength = l; }
	*/
private:
	bool m_enabled;
	QPoint m_position;
	Qt::Orientation m_orientation;

	bool m_fillingEnabled;
	QColor m_fillingColor;
	bool m_boxEnabled;
	bool m_shadowEnabled;

	QFont m_textFont;
	QColor m_textColor;

	/*
	int x1, y1, x2, y2;		// legend box
	int namelength;		// legend width
	int ticlabellength;		// max length of tic label. used for border
	*/
};

#endif //LEGEND_H
