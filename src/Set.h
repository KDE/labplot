//LabPlot : Set.h

#ifndef SET_H
#define SET_H

#include <QString>
#include <QPainter>
#include "elements/Label.h"
#include "Style.h"
#include "Symbol.h"
#include "settype.h"
#include "defs.h"

class Set
{
public:
	Set(QString name="", int number=100);
/*	Set(QString n="", QString label="", LSource src=SFUNCTION, PType t=P2D, Style *st=0,
			Symbol *sy=0,int nr=0, bool s=true);
	virtual ~Set() {}
	void openSet(QTextStream *t,int version);
	void saveSet(QTextStream *t);
	virtual void saveXML(QDomDocument doc, QDomElement graphtag) = 0;
	QDomElement saveGraphXML(QDomDocument doc, int gtype);
	virtual void openXML(QDomNode node) = 0;
	void openGraphXML(QDomElement e);
	QString Name() { return name; }
*/
	SetType Type() const { return type; }
	Label* getLabel() const { return label; }
//	void setLabel(Label* l) { label=l; }
/*	LSource Source() { return source; }
	void setSource(LSource s) { source=s; }
	int ReadAs() { return readas; }
	void setReadAs(int r) { readas = r; }
	PType Type() { return type; }
*/	Style *getStyle() const { return style; }
	void setStyle(Style *s) { style = s; }
	Symbol *getSymbol() const { return symbol; }
	void setSymbol(Symbol *s) { symbol = s; }

	ACCESS(int, number, Number);
	ACCESSFLAG(shown, Shown);
/*	AnnotateValues getAnnotateValues() { return av; }
	void setAnnotateValues(AnnotateValues a) { av=a;}
	QString FitFunction() { return fitfunction; }
	void setFitFunction(QString f) { fitfunction=f; }
*/
	void drawStyle(QPainter *p, int x, int y);	//!< draw style and symbol on QPainter

protected:
	SetType type;
	QString name;
	int number;
	bool shown;		//!< shown/hidden
	Label *label;
/*	PType type;		//!< plot type of a graph
	LSource source;
	int readas;		//!< selected read as for data
*/	Style* style;
	Symbol* symbol;
//	AnnotateValues av;
//	QString fitfunction;
};

#endif // Set_H
