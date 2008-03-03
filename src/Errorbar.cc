//LabPlot : Errorbar.cc

#include <qtextstream.h>
#include <qpointarray.h>
#include <qbrush.h>
#include <qpainter.h>
#include  <kdebug.h>
#include "Errorbar.h"

Errorbar::Errorbar(QColor c, int xs, int ys, Qt::PenStyle st, int w, EType xt, EType yt, QColor bc, int bw, Qt::PenStyle bst)
{
	color = c;
	xsize = xs;
	ysize = ys;
	style = st;
	width = w;
	xtype = xt;
	ytype = yt;
	basecolor = bc;
	basewidth = bw;
	basestyle = bst;
}

QDomElement Errorbar::saveXML(QDomDocument doc) {
	QDomElement errorbartag = doc.createElement( "Errorbar" );

	QDomElement tag = doc.createElement( "Color" );
	errorbartag.appendChild( tag );
	QDomText t = doc.createTextNode( color.name() );
	tag.appendChild( t );
	tag = doc.createElement( "XSize" );
	errorbartag.appendChild( tag );
	t = doc.createTextNode( QString::number(xsize) );
	tag.appendChild( t );
	tag = doc.createElement( "YSize" );
	errorbartag.appendChild( tag );
	t = doc.createTextNode( QString::number(ysize) );
	tag.appendChild( t );
	tag = doc.createElement( "Style" );
	errorbartag.appendChild( tag );
	t = doc.createTextNode( QString::number(style) );
	tag.appendChild( t );
	tag = doc.createElement( "Width" );
	errorbartag.appendChild( tag );
	t = doc.createTextNode( QString::number(width) );
	tag.appendChild( t );
	tag = doc.createElement( "XType" );
	errorbartag.appendChild( tag );
	t = doc.createTextNode( QString::number(xtype) );
	tag.appendChild( t );
	tag = doc.createElement( "YType" );
	errorbartag.appendChild( tag );
	t = doc.createTextNode( QString::number(ytype) );
	tag.appendChild( t );

	tag = doc.createElement( "BaseColor" );
	errorbartag.appendChild( tag );
	t = doc.createTextNode( basecolor.name() );
	tag.appendChild( t );
	tag = doc.createElement( "BaseWidth" );
	errorbartag.appendChild( tag );
	t = doc.createTextNode( QString::number(basewidth) );
	tag.appendChild( t );
	tag = doc.createElement( "BaseStyle" );
	errorbartag.appendChild( tag );
	t = doc.createTextNode( QString::number(basestyle) );
	tag.appendChild( t );

	return errorbartag;
}

void Errorbar::openXML(QDomNode node) {
	while(!node.isNull()) {
		QDomElement e = node.toElement();
		kdDebug()<<"ERRORBAR TAG = "<<e.tagName()<<endl;
		kdDebug()<<"ERRORBAR TEXT = "<<e.text()<<endl;

		if(e.tagName() == "Color")
			color = QColor(e.text());
		else if(e.tagName() == "XSize")
			xsize = e.text().toInt();
		else if(e.tagName() == "YSize")
			ysize = e.text().toInt();
		else if(e.tagName() == "Style")
			style = (Qt::PenStyle) e.text().toInt();
		else if(e.tagName() == "Width")
			width = e.text().toInt();
		else if(e.tagName() == "XType")
			xtype = (EType) e.text().toInt();
		else if(e.tagName() == "YType")
			ytype = (EType) e.text().toInt();
		else if(e.tagName() == "BaseColor")
			basecolor = QColor(e.text());
		else if(e.tagName() == "BaseWidth")
			basewidth = e.text().toInt();
		else if(e.tagName() == "BaseStyle")
			basestyle = (Qt::PenStyle) e.text().toInt();

		node = node.nextSibling();
	}
}

//! draw errorbars for x-y-dy, x-y-dx-dy and x-y-dy1-dy2
// at (x,y) from xleft to xright and ybottom to ytop
void Errorbar::draw(QPainter *p, QPointArray pa, QPointArray hpa, QPointArray vpa) {
	p->setPen(QPen(color, width, style));

	for(unsigned int i=0;i<pa.size();i++) {
		int x=pa[i].x(), y=pa[i].y();
		int xleft=hpa[i].x(), xright=hpa[i].y();
		int ytop=vpa[i].x(),ybottom=vpa[i].y();

		// baseline
		p->setPen(QPen(basecolor, basewidth, basestyle));
		if (xleft != x)
			p->drawLine(xleft,y,x,y);
		if (xright != x)
			p->drawLine(x,y,xright,y);
		if (ytop != y)
			p->drawLine(x,y,x,ytop);
		if (ybottom != y)
			p->drawLine(x,y,x,ybottom);

		// errorbars
		p->setPen(QPen(color, width, style));
		if (xleft != x) {
			switch (xtype) {
			case EFLAT:
				p->drawLine(xleft,y-xsize,xleft,y+xsize);
				break;
			case EARROW:
				p->drawLine(xleft+xsize,y-xsize,xleft,y);
				p->drawLine(xleft,y,xleft+xsize,y+xsize);
				break;
			case EY:
				p->drawLine(xleft-xsize,y-xsize,xleft,y);
				p->drawLine(xleft,y,xleft-xsize,y+xsize);
				break;
			}
		}
		if (xright != x) {
			switch (xtype) {
			case EFLAT:
				p->drawLine(xright,y-xsize,xright,y+xsize);
				break;
			case EARROW:
				p->drawLine(xright-xsize,y-xsize,xright,y);
				p->drawLine(xright,y,xright-xsize,y+xsize);
				break;
			case EY:
				p->drawLine(xright+xsize,y-xsize,xright,y);
				p->drawLine(xright,y,xright+xsize,y+xsize);
				break;
			}
		}
		if (ytop != y) {
			switch (ytype) {
			case EFLAT:
				p->drawLine(x-ysize,ytop,x+ysize,ytop);
				break;
			case EARROW:
				p->drawLine(x-ysize,ytop-ysize,x,ytop);
				p->drawLine(x,ytop,x+ysize,ytop-ysize);
				break;
			case EY:
				p->drawLine(x-ysize,ytop+ysize,x,ytop);
				p->drawLine(x,ytop,x+ysize,ytop+ysize);
				break;
			}
		}
		if (ybottom != y) {
			switch (ytype) {
			case EFLAT:
				p->drawLine(x-ysize,ybottom,x+ysize,ybottom);
				break;
			case EARROW:
				p->drawLine(x-ysize,ybottom+ysize,x,ybottom);
				p->drawLine(x,ybottom,x+ysize,ybottom+ysize);
				break;
			case EY:
				p->drawLine(x-ysize,ybottom-ysize,x,ybottom);
				p->drawLine(x,ybottom,x+ysize,ybottom-ysize);
				break;
			}
		}
	}
}

