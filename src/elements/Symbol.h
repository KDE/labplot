/***************************************************************************
    File                 : Symbol.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Stefan Gerlach, Alexander Semke
    Email (use @ for *)  : stefan.gerlach*uni-konstanz.de, alexander.semke*web.de
    Description          : plot symbol class
                           
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
#ifndef SYMBOL_H
#define SYMBOL_H

#include <QPainter>
#include "../definitions.h"

/**
 * @brief Contains all the information needed to define/describe a symbol.
*	Provides the static function \c draw() for drawing a symbol.
*/
class Symbol {
public:

	enum SymbolType {SNONE,SCROSS,SDOT,SPLUS,SCIRCLE,STRIANGLE,SUTRIANGLE,SRECT,SSTAR,SDIAMOND,SMINUS,SPIPE,
		SLTRIANGLE,SRTRIANGLE,STRIANGLE1,STRIANGLE2,STRIANGLE3,STRIANGLE4,SUCIRCLE,SDCIRCLE,SSTAR2,
		SVBAR,SHBAR,SDIAG1,SDIAG2,SCROSS2,SDIAG3,SDIAG4,SCROSS3,
		SPARRIGHT,SPARLEFT,SHLEFTCIRCLE,SHRIGHTCIRCLE,SSMALLDIAMOND,SROTDIAMOND,SPENTA,SPENTALEFT,
		SPENTABOTTOM,SPENTARIGHT,SHEXAGON,SVHEXAGON,SSTAR3,SUARROW,SLARROW,SDARROW,SRARROW,
		SUHOUSE,SLHOUSE,SDHOUSE,SRHOUSE};

	// symbol fill type
	enum SymbolFillType {FNONE, FFULL,FBOTTOM,FTOP,FLEFT,FRIGHT,FURIGHT,FDLEFT,FULEFT,FDRIGHT};

// 	Symbol (bool s=true, SymbolType t=SNONE, QColor c=Qt::blue, int s=5, SymbolFillType f=FNONE, QColor fc="red", int b=1);
	Symbol();
//	void save(QTextStream *t);
//	void open(QTextStream *t,int version);
//	QDomElement saveXML(QDomDocument doc);
//	void openXML(QDomNode node);
	void draw(QPainter *p, const QPointF& point);
	static void draw( QPainter *p, const QPoint* point, const SymbolType t=SNONE, const QColor color=Qt::blue,
					   const int size=5, const SymbolFillType fillType=FNONE, const QColor fillColor=Qt::transparent, const Qt::BrushStyle fillBrushStyle=Qt::NoBrush);

	static int styleCount();
	static int fillingTypeCount();

	ACCESS(SymbolType, type, Type);
	ACCESS(QColor, color, Color);
	ACCESS(int, size, Size);
	ACCESS(SymbolFillType, fillType, FillType);
	ACCESS(QColor, fillColor, FillColor);
	ACCESS(Qt::BrushStyle, fillBrushStyle, FillBrushStyle);
//	void setErrorbar(Errorbar *e) { errorbar=e; }
//	Errorbar *errorBar() { return errorbar; }
//	EType errorbarType() { return etype; }
//	void setErrorbarType(EType e) { etype=e; }
//	int errorbarSize() { return esize; }
//	void setErrorbarSize(int s) { esize=s; }

private:
	SymbolType m_type;
	QColor m_color;
	int m_size;
	SymbolFillType m_fillType;
	QColor m_fillColor;
	Qt::BrushStyle m_fillBrushStyle;
//	Errorbar *errorbar;
};

#endif //SYMBOL_H
