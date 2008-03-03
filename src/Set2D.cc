//LabPlot : Set2D.cc

#include <KDebug>
#include "Set2D.h"

Set2D::Set2D(QString name, Point *data, int number)
	: Set(name, number), data(data)
{
	kdDebug()<<"Set2D()"<<endl;
	type = SET2D;
	if(number>0) resetRanges();
}

// calculate ranges from data
void Set2D::resetRanges() {
	kdDebug()<<"Set2D::resetRanges()"<<endl;
	double xmin=data[0].X(),xmax=xmin;
	double ymin=data[0].Y(),ymax=ymin;
	for(int i=1;i<number;i++) {
		data[i].X()<xmin?xmin=data[i].X():0;
		data[i].X()>xmax?xmax=data[i].X():0;
		data[i].Y()<ymin?ymin=data[i].Y():0;
		data[i].Y()>ymax?ymax=data[i].Y():0;
	}
	range[0].setRange(xmin,xmax);
	range[1].setRange(ymin,ymax);
	kdDebug()<<"	x range : "<<xmin<<xmax<<endl;
	kdDebug()<<"	y range : "<<ymin<<ymax<<endl;
}

/*Graph2D::Graph2D(QString n, QString l, LRange r[2], LSource src, PType t, Style *st,
		Symbol *sy, Point *p, int nr, bool b)
	: Graph(n, l, src, t, st, sy, nr, b)
{
	if (r) {
		for (int i=0;i<2;i++) {
			range[i].setMin(r[i].rMin());
			range[i].setMax(r[i].rMax());
		}
	}

	ptr = new Point[nr];
	for(int i=0;i<nr;i++)
		ptr[i] = p[i];

	delete [] p;
}

Graph2D::~Graph2D() {
	delete [] ptr;

	delete label;
}

Graph2D *Graph2D::Clone() {
	Graph2D *newg = new Graph2D(*this);
	Label *l = new Label();
	*l = *(label);
	newg->setLabel(l);	// set label
	Point *data = new Point[number];
	for(int i=0;i<number;i++)
		data[i] = ptr[i];
	newg->setData(data);	// set data

	LRange nrange[2];
	nrange[0] = range[0];
	nrange[1] = range[1];
	newg->setRange(nrange);

	return newg;
}

QStringList Graph2D::Info()
{
	QStringList s;
	QString t;
	if(type==P2D)
		t=i18n("2D");
	else if (type==PSURFACE)
		t=i18n("Surface");
	else if (type==P3D)
		t=i18n("3D");
	else if (type==PGRASS)
		t=i18n("GRASS");
	else if (type==PPIE)
		t=i18n("Pie");
	else if (type==PPOLAR)
		t=i18n("Polar");

	QString sh=i18n("NO");
	if (shown)
		sh=i18n("YES");

	s << label->simpleTitle() << t << sh;
	s << QString::number(number);
	s << QString(" 1 ");		// only 1 column
	s << QString::number(range[0].rMin()) + " .. " + QString::number(range[0].rMax());
	s << QString::number(range[1].rMin())+ " .. " + QString::number(range[1].rMax());

	return s;
}

void Graph2D::saveXML(QDomDocument doc, QDomElement graphtag) {
	kdDebug()<<"Graph2D::saveXML()"<<endl;
	QDomElement tag = doc.createElement( "Range" );
	tag.setAttribute("xmin",QString::number(range[0].rMin()));
	tag.setAttribute("xmax",QString::number(range[0].rMax()));
	tag.setAttribute("ymin",QString::number(range[1].rMin()));
	tag.setAttribute("ymax",QString::number(range[1].rMax()));
    	graphtag.appendChild( tag );

	for (int i=0;i< number;i++) {
		tag = doc.createElement( "Data" );
		tag.setAttribute("x",QString::number(ptr[i].X()));
		tag.setAttribute("y",QString::number(ptr[i].Y()));
		tag.setAttribute("masked",QString::number(ptr[i].Masked()));
    		graphtag.appendChild( tag );
	}
	kdDebug()<<"Graph2D::saveXML() DONE"<<endl;
}

void Graph2D::openXML(QDomNode node) {
	int i=0;
	while(!node.isNull()) {
		QDomElement e = node.toElement();
//		kdDebug()<<"GRAPH TAG = "<<e.tagName()<<endl;
//		kdDebug()<<"GRAPH TEXT = "<<e.text()<<endl;

		openGraphXML(e);

		if(e.tagName() == "Number")
			ptr = new Point[e.text().toInt()];
		else if(e.tagName() == "Range") {
			range[0].setRange(e.attribute("xmin").toDouble(),e.attribute("xmax").toDouble());
			range[1].setRange(e.attribute("ymin").toDouble(),e.attribute("ymax").toDouble());
		}
		else if(e.tagName() == "Data") {
			// kdDebug()<<"I = "<<i<<" x ="<<e.attribute("x").toDouble()<<endl;
			ptr[i].setPoint(e.attribute("x").toDouble(),e.attribute("y").toDouble());
			ptr[i].setMasked((bool) e.attribute("masked").toInt());
			i++;
		}

		node = node.nextSibling();
	}
}

void Graph2D::save(QTextStream *t, QProgressDialog *progress) {
	saveGraph(t);
	*t<<number<<endl;
       	*t<<range[0].rMin()<<' '<<range[0].rMax()<<' '<<range[1].rMin()<<' '<<range[1].rMax()<<endl;
	*t<<type<<' ';
	style->save(t);
	symbol->save(t);

	//dump data
	progress->setTotalSteps(number);
	for (int i=0;i< number;i++) {
		if(i%1000 == 0) progress->setProgress(i);
		*t<<ptr[i].X()<<' '<<ptr[i].Y()<<' '<<ptr[i].Masked()<<endl;
	}
	progress->cancel();
}

void Graph2D::open(QTextStream *t, int version, QProgressDialog *progress) {
	openGraph(t,version);

	if (version > 2)
		*t>>number;
	else if (version > 1 ) {
		QString title;
		*t>>name>>title>>number;
		label->setTitle(title);
	}
	else	// version == 0
		*t>>name>>number;

	double xmin, xmax, ymin, ymax;
	*t>>xmin>>xmax>>ymin>>ymax;
	range[0].setMin(xmin);
	range[0].setMax(xmax);
	range[1].setMin(ymin);
	range[1].setMax(ymax);

	// this belongs to openGraph() but the ranges are in between :-(
	type = (PType) (style->open(t,version));
	symbol->open(t,version);

	// read data
	double x, y;
	int m;
	ptr = new Point[number];

	progress->setTotalSteps(number);
	for (int i=0;i< number;i++) {
		if(i%1000 == 0) progress->setProgress(i);

		if (version>18) {
			*t>>x>>y>>m;
			ptr[i].setMasked(m);
		}
		else
			*t>>x>>y;

		ptr[i].setPoint(x,y);
		//kdebug()<<x<<' '<<y<<endl;
	}
	progress->cancel();
}
*/
