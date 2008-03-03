//LabPlot : Symbol.h

#ifndef SYMBOL_H
#define SYMBOL_H

#include <QColor>
#include <QPainter>
//#include <qdom.h>
//#include "Errorbar.h"
#include "symbol.h"

class Symbol {
public:
	Symbol(SType t=SNONE, QColor c="blue", int s=5, FType f=FNONE, QColor fc="red", int b=1);
/*	void save(QTextStream *t);
	void open(QTextStream *t,int version);
	QDomElement saveXML(QDomDocument doc);
	void openXML(QDomNode node);
*/
	void draw(QPainter *p, QPoint point);
	SType Type() { return type; }
	void setType(SType t) { type = t; }
	QColor Color() { return color; }
	void setColor(QString c) {color = QColor(c); }
	void setColor(QColor c) {color = c; }
	int Size() { return size; }
	void setSize(int s) { size = s; }
	FType Fill() { return fill; }
	void setFill(FType f) { fill = f; }
	QColor FillColor() { return fillcolor; }
	void setFillColor(QString fc) { fillcolor = QColor(fc); }
	void setFillColor(QColor fc) { fillcolor = fc; }
	int Brush() { return brush;}
	void setBrush(int b) { brush = b; }
//	void setErrorbar(Errorbar *e) { errorbar=e; }
//	Errorbar *errorBar() { return errorbar; }
//	EType errorbarType() { return etype; }
//	void setErrorbarType(EType e) { etype=e; }
//	int errorbarSize() { return esize; }
//	void setErrorbarSize(int s) { esize=s; }
private:
	SType type;
	QColor color;
	int size;
	FType fill;
	QColor fillcolor;
	int brush;
//	Errorbar *errorbar;
};

#endif //SYMBOL_H
