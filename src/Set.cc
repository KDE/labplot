//LabPlot : Graph.cc

#include "Set.h"
#include "settype.h"

//Graph::Graph(QString n, QString l, LSource src, PType t, Style *st, Symbol *sy, int nr, bool s)
Set::Set(QString name, int number)
	: name(name), number(number)
{
	label = new Label();
//	source = src;
//	type = t;
//	style = st;
//	if(style == 0)
		style = new Style();
//	symbol = sy;
//	if(symbol == 0)
		symbol = new Symbol();

	shown = true;
//	readas = 0;
//	fitfunction = QString("a*x+b");
}

/*void Graph::saveGraph(QTextStream *t) {
	*t<<name<<endl;
	// OLD : *t<<label<<endl;
	label->save(t);
	*t<<shown<<endl;
	*t<<(int)source<<endl;
	*t<<readas<<endl;
	av.save(t);
	*t<<fitfunction<<endl;
}

// open Graph specfic things
void Graph::openGraph(QTextStream *t,int version) {
	if (version > 2) {
		// name + label
		t->readLine();
		name = t->readLine();
		if(version > 21)
			label->open(t,version,false);
		else {
			QString title = t->readLine();
			label->setTitle(title);
		}

		if(version>14) {
			int tmp;
			*t>>tmp;
			shown = (bool) tmp;
			if(version>17) {
				*t>>tmp;
				source = (LSource) tmp;
				if(version>18)
					*t>>readas;
			}
			av.open(t, version);
			if(version>18) {
				t->readLine();
				fitfunction = t->readLine();
				//kDebug()<<"FIT FUNCTION : "<<fitfunction<<endl;
			}
		}
	}
}

QDomElement Graph::saveGraphXML(QDomDocument doc, int gtype) {
	kDebug()<<"Graph::saveGraphXML()"<<endl;
	QDomElement graphtag = doc.createElement( "Graph" );
	graphtag.setAttribute("type",QString::number(gtype));

	QDomElement tag = doc.createElement( "Name" );
    	graphtag.appendChild( tag );
  	QDomText t = doc.createTextNode( name );
    	tag.appendChild( t );
	tag = doc.createElement( "Number" );
    	graphtag.appendChild( tag );
  	t = doc.createTextNode( QString::number(number) );
    	tag.appendChild( t );
	tag = doc.createElement( "PlotType" );
    	graphtag.appendChild( tag );
  	t = doc.createTextNode( QString::number(type) );
    	tag.appendChild( t );

	tag = label->saveXML(doc);
    	graphtag.appendChild( tag );

	tag = doc.createElement( "Shown" );
    	graphtag.appendChild( tag );
  	t = doc.createTextNode( QString::number(shown) );
    	tag.appendChild( t );
	tag = doc.createElement( "Source" );
    	graphtag.appendChild( tag );
  	t = doc.createTextNode( QString::number(source) );
    	tag.appendChild( t );
	tag = doc.createElement( "ReadAs" );
    	graphtag.appendChild( tag );
  	t = doc.createTextNode( QString::number(readas) );
    	tag.appendChild( t );
	tag = doc.createElement( "FitFunction" );
    	graphtag.appendChild( tag );
  	t = doc.createTextNode( fitfunction );
    	tag.appendChild( t );

	tag = av.saveXML(doc);
    	graphtag.appendChild( tag );
	tag = style->saveXML(doc);
    	graphtag.appendChild( tag );
	tag = symbol->saveXML(doc);
    	graphtag.appendChild( tag );

	kDebug()<<"	calling saveXML()"<<endl;
	saveXML(doc,graphtag);
	kDebug()<<"	done"<<endl;

	return graphtag;
}

void Graph::openGraphXML(QDomElement e) {
	if(e.tagName() == "Name" )
		name = e.text();
	if(e.tagName() == "Label" )
		label->openXML(e.firstChild());
	else if(e.tagName() == "Number" )
		number = e.text().toInt();
	else if(e.tagName() == "PlotType" )
		type = (PType) e.text().toInt();
	else if(e.tagName() == "Shown" )
		shown = (bool) e.text().toInt();
	else if(e.tagName() == "Source" )
		source = (LSource) e.text().toInt();
	else if(e.tagName() == "ReadAs" )
		readas = e.text().toInt();
	else if(e.tagName() == "FitFunction" )
		fitfunction = e.text();
	else if(e.tagName() == "Annotate" )
		av.openXML(e.firstChild());
	else if(e.tagName() == "Style" )
		style->openXML(e.firstChild());
	else if(e.tagName() == "Symbol" )
		symbol->openXML(e.firstChild());
}
*/
void Set::drawStyle(QPainter *p, int x, int y) {
/*	if(style->Type() != NOLINESTYLE)
		p->drawLine(x,y,x+30,y);
	symbol->draw(p,x+15,y);
*/
}
