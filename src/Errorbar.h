//LabPlot : Errorbar.h

#ifndef ERRORBAR_H
#define ERRORBAR_H

#include <QColor>
#include <QDomElement>
#include "errorbars.h"

class Errorbar {
public:
	Errorbar(QColor c=Qt::blue, int xs=2, int ys=2, Qt::PenStyle st=Qt::SolidLine, int w=1, EType xt=EFLAT, EType yt=EFLAT, 
		QColor bc=Qt::blue, int bw=1, Qt::PenStyle bst=Qt::SolidLine);
	QDomElement saveXML(QDomDocument doc);
	void openXML(QDomNode node);
//	void draw(QPainter *p, QPointArray pa, QPointArray hpa, QPointArray vpa);

	QColor Color() { return color; }
	void setColor(QColor c) { color=c; }
	QColor BaseColor() { return basecolor; }
	void setBaseColor(QColor c) { basecolor=c; }
	int XSize() { return xsize; }
	void setXSize(int s) { xsize=s; }
	int YSize() { return ysize; }
	void setYSize(int s) { ysize=s; }
	Qt::PenStyle Style() { return style; }
	void setStyle(Qt::PenStyle p) { style=p; }
	Qt::PenStyle BaseStyle() { return basestyle; }
	void setBaseStyle(Qt::PenStyle p) { basestyle=p; }
	int Width() { return width; }
	void setWidth(int w) { width=w; }
	int BaseWidth() { return basewidth; }
	void setBaseWidth(int w) { basewidth=w; }
	EType XType() { return xtype; }
	void setXType(EType e) { xtype=e; }
	EType YType() { return ytype; }
	void setYType(EType e) { ytype=e; }
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
