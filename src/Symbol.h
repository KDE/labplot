//LabPlot : Symbol.h

#ifndef SYMBOL_H
#define SYMBOL_H

#include <QColor>
#include <QPainter>
//#include <qdom.h>
//#include "Errorbar.h"
#include "symbol.h"
#include "definitions.h"

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
	ACCESS(FType, fillType, FillType);
	ACCESS(QColor, fillColor, FillColor);
	ACCESS(int, brush, Brush);
//	void setErrorbar(Errorbar *e) { errorbar=e; }
//	Errorbar *errorBar() { return errorbar; }
//	EType errorbarType() { return etype; }
//	void setErrorbarType(EType e) { etype=e; }
//	int errorbarSize() { return esize; }
//	void setErrorbarSize(int s) { esize=s; }
private:
	SType m_type;
	QColor m_color;
	int m_size;
	FType m_fillType;
	QColor m_fillColor;
	int m_brush;	// TODO: use QBrush ?
//	Errorbar *errorbar;
};

#endif //SYMBOL_H
