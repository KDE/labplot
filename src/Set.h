//LabPlot : Set.h

#ifndef SET_H
#define SET_H

class Function;

#include <QString>
#include <QPainter>
#include "elements/Label.h"
#include "Style.h"
#include "elements/Symbol.h"
#include "definitions.h"

class Set
{
public:
	// type of set (GRASS,VTK obsolete)
	enum SetType {SET2D,SET3D,SETM,SETGRASS,SETVTK,SET4D,SETIMAGE,SETL};
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
	ACCESS(Set::SetType,type,Type);
	Label* label() { return m_label; }
	void setLabel(Label* label) { m_label=label; }
	QList<Point>* data() { return &m_data; }
	void setData(QList<Point> data) { m_data = data;}
	Function* function() { return m_function; }

/*	LSource Source() { return source; }
	void setSource(LSource s) { source=s; }
	int ReadAs() { return readas; }
	void setReadAs(int r) { readas = r; }
	PType Type() { return type; }
*/	Style *style() const { return m_style; }
	void setStyle(Style *s) { m_style = s; }
	Symbol *symbol() const { return m_symbol; }
	void setSymbol(Symbol *s) { m_symbol = s; }

	ACCESS(int, number, Number);
	ACCESSFLAG(m_shown, Shown);
/*	AnnotateValues getAnnotateValues() { return av; }
	void setAnnotateValues(AnnotateValues a) { av=a;}
	QString FitFunction() { return fitfunction; }
	void setFitFunction(QString f) { fitfunction=f; }
*/
	void drawStyle(QPainter *p, int x, int y);	//!< draw style and symbol on QPainter

protected:
	SetType m_type;
	QString m_name;
	int m_number;
	bool m_shown;		//!< shown/hidden
	Function* m_function;
	Label *m_label;
	QList<Point> m_data;
/*	PType type;		//!< plot type of a graph
	LSource source;
	int readas;		//!< selected read as for data
*/	Style* m_style;
	Symbol* m_symbol;
//	AnnotateValues av;
//	QString fitfunction;
};

#endif // Set_H
