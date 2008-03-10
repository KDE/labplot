//LabPlot : Style.h

#ifndef STYLE_H
#define STYLE_H

#include <QColor>
//#include <qdom.h>

#include "styletype.h"
#include "defs.h"

class Style
{
public:
	Style(StyleType t=LINESTYLE, QColor c="blue", bool f=0, QColor fc="green", int w=1,Qt::PenStyle p=Qt::SolidLine, Qt::BrushStyle b=Qt::SolidPattern );
//	void save(QTextStream *t);
//	int open(QTextStream *t,int version);		// returns graph type
//	QDomElement saveXML(QDomDocument doc);
//	void openXML(QDomNode node);

	ACCESS(StyleType, type, Type);
	ACCESS(QColor, color, Color);
	ACCESS(int, width, Width);
	ACCESS(Qt::PenStyle, penstyle, PenStyle);
	ACCESS(Qt::BrushStyle, brushstyle, BrushStyle);
	ACCESSFLAG(filled, Fill);
	ACCESS(QColor, fillcolor, FillColor);
	ACCESS(int, boxwidth, BoxWidth);
	ACCESSFLAG(autoboxwidth,AutoBoxWidth);
	ACCESSFLAG(sort_points,PointsSorting);
private:
	StyleType type;
	QColor color;		//!< line color
	int width;		//!< line width
	bool filled;		//!< filled to baseline
	QColor fillcolor;
	Qt::PenStyle penstyle;	//!< pen style
	Qt::BrushStyle brushstyle;	//!< fill brush
	int boxwidth;		//!< width for type boxes
	bool autoboxwidth;	//!< automatic box width ?
	bool sort_points;	//!< sort points before plotting ?
};

#endif // STYLE_H
