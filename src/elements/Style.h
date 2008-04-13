//LabPlot : Style.h

#ifndef STYLE_H
#define STYLE_H

#include <QColor>
//#include <qdom.h>

#include "../styletype.h"
#include "../definitions.h"

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
	ACCESS(Qt::PenStyle, penStyle, PenStyle);
	ACCESS(Qt::BrushStyle, brushStyle, BrushStyle);
	ACCESSFLAG(m_filled, Fill);
	ACCESS(QColor, fillColor, FillColor);
	ACCESS(int, boxWidth, BoxWidth);
	ACCESSFLAG(m_autoBoxWidth,AutoBoxWidth);
	ACCESSFLAG(m_sortPoints,PointsSorting);
private:
	StyleType m_type;
	QColor m_color;		//!< line color
	int m_width;		//!< line width
	bool m_filled;		//!< filled to baseline
	QColor m_fillColor;
	Qt::PenStyle m_penStyle;	//!< pen style
	Qt::BrushStyle m_brushStyle;	//!< fill brush
	int m_boxWidth;		//!< width for type boxes
	bool m_autoBoxWidth;	//!< automatic box width ?
	bool m_sortPoints;	//!< sort points before plotting ?
};

#endif // STYLE_H
