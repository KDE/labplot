//LabPlot : Set2D.h

#ifndef SET2D_H
#define SET2D_H

#include "Set.h"
#include "Point.h"
#include "Range.h"
#include <QList>

class Set2D: public Set{

public:
	Set2D(QString name="", int number=0);
/*	Graph2D(QString n="", QString l="",LRange r[2]=0, LSource src=SFUNCTION, PType t=P2D, Style *st=0,
		Symbol *sy=0, Point *p=0, int nr=0, bool b=true);
	~Graph2D();
	Graph2D *Clone();		// clone this graph
*/
	Range getRange(int i) const { return m_range[i]; }
	void setRange(Range range[2]) { m_range[0]=range[0]; m_range[1]=range[1]; }


/*	QStringList Info();
	void save(QTextStream *t, QProgressDialog *progress);
	void open(QTextStream *t, int version, QProgressDialog *progress);
	void saveXML(QDomDocument doc, QDomElement graphtag);
	void openXML(QDomNode node);
*/
private:
	Range m_range[2];
	void resetRanges();
};

#endif
