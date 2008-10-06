/***************************************************************************
    File                 : Style.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Stefan Gerlach, Alexander Semke
    Email (use @ for *)  : stefan.gerlach*uni-konstanz.de, alexander.semke*web.de
    Description          : plot style class
                           
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
#ifndef STYLE_H
#define STYLE_H

#include "Symbol.h"
#include "../definitions.h"

class Style{
public:
	enum StyleType {LINESTYLE,STEPSSTYLE,BOXESSTYLE,IMPULSESSTYLE,YBOXESSTYLE};

	Style(bool l=true, Style::StyleType t=LINESTYLE, QColor c="blue", bool f=false, QColor fc="green", int w=1,Qt::PenStyle p=Qt::SolidLine, Qt::BrushStyle b=Qt::SolidPattern );
//	void save(QTextStream *t);
//	int open(QTextStream *t,int version);		// returns graph type
//	QDomElement saveXML(QDomDocument doc);
//	void openXML(QDomNode node);

	//Line style
	ACCESSFLAG(m_lineEnabled, LineEnabled);
	ACCESS(Style::StyleType, type, Type);
	ACCESS(Qt::PenStyle, lineStyle, LineStyle);
	ACCESS(QColor, lineColor, LineColor);
	ACCESS(int, lineWidth, LineWidth);

	//Area filling
	ACCESSFLAG(m_filled, Filled);
	ACCESS(Qt::BrushStyle, fillBrushStyle, FillBrushStyle);
	ACCESS(QColor, fillColor, FillColor);

	//Symbol
	ACCESSFLAG(m_symbolEnabled, SymbolEnabled);
	Symbol* symbol();

	//Misc
	ACCESS(int, boxWidth, BoxWidth);
	ACCESSFLAG(m_autoBoxWidth,AutoBoxWidth);
	ACCESSFLAG(m_sortPoints,PointsSorting);

private:
	//Line style
	bool m_lineEnabled;
	Style::StyleType m_type;
	QColor m_lineColor;
	int m_lineWidth;
	Qt::PenStyle m_lineStyle;

	//Area filling
	bool m_filled;		//!< filled to baseline
	QColor m_fillColor;
	Qt::BrushStyle m_fillBrushStyle;

	//Symbol
	bool m_symbolEnabled;
	Symbol m_symbol;

	//Misc
	int m_boxWidth;		//!< width for type boxes
	bool m_autoBoxWidth;	//!< automatic box width ?
	bool m_sortPoints;	//!< sort points before plotting ?
};

#endif // STYLE_H
