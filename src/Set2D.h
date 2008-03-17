//LabPlot : Set2D.h

#ifndef SET2D_H
#define SET2D_H

#include "Set.h"
#include "Point.h"
#include "Range.h"

class Set2D: public Set
{
public:
	Set2D(QString name="", Point *data=0, int number=0);
/*	Graph2D(QString n="", QString l="",LRange r[2]=0, LSource src=SFUNCTION, PType t=P2D, Style *st=0, 
		Symbol *sy=0, Point *p=0, int nr=0, bool b=true);
	~Graph2D();
	Graph2D *Clone();		// clone this graph
*/
	Range getRange(int i) const { return range[i]; }
	void setRange(Range r[2]) { range[0]=r[0]; range[1]=r[1]; }

	Point *Data() const { return data; }
//	void setData(Point *d) { data = d; }
/*	QStringList Info();	
	void save(QTextStream *t, QProgressDialog *progress);
	void open(QTextStream *t, int version, QProgressDialog *progress);
	void saveXML(QDomDocument doc, QDomElement graphtag);
	void openXML(QDomNode node);
*/
private:
	Point *data;
	Range range[2];
	void resetRanges();
};

#endif // SET2D_H
