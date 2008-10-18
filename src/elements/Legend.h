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
	ACCESS(QPointF, position, Position);
	ACCESS(Qt::Orientation, orientation, Orientation);

	ACCESSFLAG(m_fillingEnabled, Filling);
	ACCESS(QColor, fillingColor, FillingColor);
	ACCESSFLAG(m_boxEnabled, Box);
	ACCESSFLAG(m_shadowEnabled, Shadow);

	ACCESS(QFont, textFont, TextFont);
	ACCESS(QColor, textColor, TextColor);
  	void draw(QPainter *p, const QList<Set>*, const Point, const Point, const int w, const int h);

	/*
	void save(QTextStream *t);
	void open(QTextStream *t, int version);
	QDomElement saveXML(QDomDocument doc);
	void openXML(QDomNode node);
*/

private:
	bool m_enabled;
	QPointF m_position;
	Qt::Orientation m_orientation;
	int m_lineLength;

	bool m_fillingEnabled;
	QColor m_fillingColor;
	bool m_boxEnabled;
	bool m_shadowEnabled;

	QFont m_textFont;
	QColor m_textColor;

	int m_layoutLeftMargin;
	int m_layoutTopMargin;
	int m_layoutRightMargin;
	int m_layoutBottomMargin;
	int m_layoutHorizontalSpacing;
	int m_layoutVerticalSpacing;
};

#endif //LEGEND_H
