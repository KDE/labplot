//LabPlot : Symbol.h

#ifndef SYMBOL_H
#define SYMBOL_H

#include <QColor>
#include <QPainter>
//#include <qdom.h>
//#include "Errorbar.h"
#include "symbol.h"
#include "defs.h"

class Symbol {
public:
	Symbol(SType t=SNONE, QColor c="blue", int s=5, FType f=FNONE, QColor fc="red", int b=1);
//	void save(QTextStream *t);
//	void open(QTextStream *t,int version);
//	QDomElement saveXML(QDomDocument doc);
//	void openXML(QDomNode node);
	void draw(QPainter *p, QPoint point);

	ACCESS(SType, type, Type);
	ACCESS(QColor, color, Color);
	ACCESS(int, size, Size);
	ACCESS(FType, filltype, FillType);
	ACCESS(QColor, fillcolor, FillColor);
	ACCESS(int, brush, Brush);
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
	FType filltype;
	QColor fillcolor;
	int brush;	// TODO: use QBrush ?
//	Errorbar *errorbar;
};

#endif //SYMBOL_H
