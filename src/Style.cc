//LabPlot : Style.cc

#include <KDebug>
#include "Style.h"

Style::Style(StyleType t, QColor c, bool f, QColor fc, int w, Qt::PenStyle p, Qt::BrushStyle b)
{
	kDebug()<<"Style()"<<endl;
	type = t;
	color = c;
	filled = f;
	fillcolor = fc;
	width=w;
	penstyle = p;
	brushstyle = b;
	boxwidth=10;
	autoboxwidth=false;
	sort_points=true;
}
/*
void Style::save(QTextStream *t) {
	*t<<type<<' '<<color.name()<<endl;
	*t<<fill<<' '<<fillcolor.name()<<endl;
	*t<<width<<' '<<penStyle<<' '<<brush<<endl;
	*t<<boxwidth<<' '<<autoboxwidth<<endl;
	*t<<sort_points<<endl;
}

int Style::open(QTextStream *t,int version) {
	kDebug()<<"Style::open()"<<endl;
	QString c;
	int graphtype, f, tmptype;

	*t>>graphtype>>tmptype>>c;
	type = (StylesType)tmptype;
	color=QColor(c);
	*t>>f>>c;
	fill= (bool)f;
	fillcolor=QColor(c);
	if (version>11)
		*t>>width>>penStyle>>brush;

	int e;
	if(version>18) {
		*t>>boxwidth>>e;
		autoboxwidth=(bool)e;
	}
	if(version>22) {
		*t>>e;
		sort_points = (bool)e;
	}

	return graphtype;
}

QDomElement Style::saveXML(QDomDocument doc) {
	QDomElement styletag = doc.createElement( "Style" );

	QDomElement tag = doc.createElement( "Type" );
	styletag.appendChild( tag );
	QDomText t = doc.createTextNode( QString::number(type) );
	tag.appendChild( t );
	tag = doc.createElement( "Color" );
	styletag.appendChild( tag );
	t = doc.createTextNode( color.name() );
	tag.appendChild( t );
	tag = doc.createElement( "Width" );
	styletag.appendChild( tag );
	t = doc.createTextNode( QString::number(width) );
	tag.appendChild( t );
	tag = doc.createElement( "Fill" );
	styletag.appendChild( tag );
	t = doc.createTextNode( QString::number(fill) );
	tag.appendChild( t );
	tag = doc.createElement( "FillColor" );
	styletag.appendChild( tag );
	t = doc.createTextNode( fillcolor.name() );
	tag.appendChild( t );
	tag = doc.createElement( "PenStyle" );
	styletag.appendChild( tag );
	t = doc.createTextNode( QString::number(penStyle) );
	tag.appendChild( t );
	tag = doc.createElement( "Brush" );
	styletag.appendChild( tag );
	t = doc.createTextNode( QString::number(brush) );
	tag.appendChild( t );
	tag = doc.createElement( "BoxWidth" );
	styletag.appendChild( tag );
	t = doc.createTextNode( QString::number(boxwidth) );
	tag.appendChild( t );
	tag = doc.createElement( "AutoBoxWidth" );
	styletag.appendChild( tag );
	t = doc.createTextNode( QString::number(autoboxwidth) );
	tag.appendChild( t );
	tag = doc.createElement( "SortPoints" );
	styletag.appendChild( tag );
	t = doc.createTextNode( QString::number(sort_points) );
	tag.appendChild( t );

	return styletag;
}

void Style::openXML(QDomNode node) {
	while(!node.isNull()) {
		QDomElement e = node.toElement();
//		kDebug()<<"STYLE TAG = "<<e.tagName()<<endl;
//		kDebug()<<"STYLE TEXT = "<<e.text()<<endl;

		if(e.tagName() == "Type")
			type = (StylesType) e.text().toInt();
		else if(e.tagName() == "Color")
			color = QColor(e.text());
		else if(e.tagName() == "Width")
			width = e.text().toInt();
		else if(e.tagName() == "Fill")
			fill = (bool) e.text().toInt();
		else if(e.tagName() == "FillColor")
			fillcolor = QColor(e.text());
		else if(e.tagName() == "PenStyle")
			penStyle = e.text().toInt();
		else if(e.tagName() == "Brush")
			brush = e.text().toInt();
		else if(e.tagName() == "BoxWidth")
			boxwidth = e.text().toInt();
		else if(e.tagName() == "AutoBoxWidth")
			autoboxwidth = (bool) e.text().toInt();
		else if(e.tagName() == "SortPoints")
			sort_points = (bool) e.text().toInt();

		node = node.nextSibling();
	}
}
*/
