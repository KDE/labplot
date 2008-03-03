//LabPlot : Style.h

#ifndef STYLE_H
#define STYLE_H

#include <QColor>
//#include <qdom.h>

#include "styletype.h"

class Style
{
public:
	Style(StyleType t=LINESTYLE, QColor c="blue", bool f=0, QColor fc="green", int w=1,int p=1, int b=1 );
/*	void save(QTextStream *t);
	int open(QTextStream *t,int version);		// returns graph type
	QDomElement saveXML(QDomDocument doc);
	void openXML(QDomNode node);
*/
	StyleType Type() { return type; }
	void setType(StyleType t) { type = t; }
	QColor Color() { return color; }
	void setColor(QString c) { color = QColor(c); }
	void setColor(QColor c) { color = c; }
	int Width() { return width; }
	void setWidth(int w) { width = w; }
	int PenStyle() { return penStyle; }
	void setPenStyle(int p) { penStyle = p; }
	int Brush() { return brush; }
	void setBrush (int b) { brush = b; }
	bool isFilled() { return fill; }
	void setFilled(int f) { fill = f; }
	QColor FillColor() { return fillcolor; }
	void setFillColor(QString c) { fillcolor = QColor(c); }
	void setFillColor(QColor c) { fillcolor = c; }
	int BoxWidth() { return boxwidth; }
	void setBoxWidth(int b) { boxwidth = b; }
	bool AutoBoxWidth() { return autoboxwidth; }
	void setAutoBoxWidth(bool b=true) { autoboxwidth = b; }
	bool PointsSortingEnabled() { return sort_points; }	
	void setPointsSorting(bool e=true) { sort_points = e; }
private:
	StyleType type;
	QColor color;		// line color
	int width;		// line width
	bool fill;		// filled to baseline
	QColor fillcolor;	// fill color
	int penStyle;		// pen style :NoPen, SolidLine, DashLine, DotLine, DashDotLine, DashDotDotLine
	int brush;		// fill brush : NoBrush, Solid, Dense1, Dense2, Dense3, Dense4, Dense5,
				//	 Dense6, Dense7, Horizontal, Vertical, Cross, BDiag, FDiag, DiagCross
	int boxwidth;		// width for type boxes
	bool autoboxwidth;	// automatic box width
	bool sort_points;	// sort points before plotting ?
};

#endif // STYLE_H
