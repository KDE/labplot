//LabPlot : Symbol.cc

/*#include <qtextstream.h>
#include <qpointarray.h>
#include <qbrush.h>
#include <qpainter.h>
*/
#include <KDebug>
#include "Symbol.h"

Symbol::Symbol(SType type, QColor color, int size, FType fillType, QColor fillColor, int brush)
{
	kDebug()<<"Symbol()"<<endl;
	m_type = type;
	m_color = color;
	m_size = size;
	m_fillType = fillType;
	m_fillColor = fillColor;
	m_brush = brush;
//	errorbar = new Errorbar();
}
/*
void Symbol::save(QTextStream *t) {
	*t<<type<<' '<<color.name()<<endl;
	*t<<size<<' '<<(int)fill<<' '<<fillcolor.name()<<endl;
	*t<<brush<<endl;
}

void Symbol::open(QTextStream *t, int version) {
	QString c;
	int tmp;

	*t>>tmp>>c;
	type = (SType) tmp;
	color = QColor(c);
	*t>>size>>tmp>>c;
	fill = (FType) tmp;
	fillcolor = c;
	if (version>11)
		*t>>brush;
}

QDomElement Symbol::saveXML(QDomDocument doc) {
	QDomElement symboltag = doc.createElement( "Symbol" );

	QDomElement tag = doc.createElement( "Type" );
	symboltag.appendChild( tag );
	QDomText t = doc.createTextNode( QString::number(type) );
	tag.appendChild( t );
	tag = doc.createElement( "Color" );
	symboltag.appendChild( tag );
	t = doc.createTextNode( color.name() );
	tag.appendChild( t );
	tag = doc.createElement( "Size" );
	symboltag.appendChild( tag );
	t = doc.createTextNode( QString::number(size) );
	tag.appendChild( t );
	tag = doc.createElement( "Fill" );
	symboltag.appendChild( tag );
	t = doc.createTextNode( QString::number(fill) );
	tag.appendChild( t );
	tag = doc.createElement( "FillColor" );
	symboltag.appendChild( tag );
	t = doc.createTextNode( fillcolor.name() );
	tag.appendChild( t );
	tag = doc.createElement( "Brush" );
	symboltag.appendChild( tag );
	t = doc.createTextNode( QString::number(brush) );
	tag.appendChild( t );

	if(errorbar) {
		tag = errorbar->saveXML(doc);
		symboltag.appendChild(tag);
	}

	return symboltag;
}

void Symbol::openXML(QDomNode node) {
	while(!node.isNull()) {
		QDomElement e = node.toElement();
//		kDebug()<<"SYMBOL TAG = "<<e.tagName()<<endl;
//		kDebug()<<"SYMBOL TEXT = "<<e.text()<<endl;

		if(e.tagName() == "Type")
			type = (SType) e.text().toInt();
		else if(e.tagName() == "Color")
			color = QColor(e.text());
		else if(e.tagName() == "Size")
			size = e.text().toInt();
		else if(e.tagName() == "Fill")
			fill = (FType) e.text().toInt();
		else if(e.tagName() == "FillColor")
			fillcolor = QColor(e.text());
		else if(e.tagName() == "Brush")
			brush = e.text().toInt();
		else if(e.tagName() == "Errorbar") {
			if(errorbar)
				errorbar->openXML(e.firstChild());
		}

		node = node.nextSibling();
	}
}
*/

//! draw the Symbol to the specified QPainter at position X,Y
void Symbol::draw(QPainter *p, QPoint point) {
	// TODO : QPointArray -> const QPoint *points
	// p->draw(Poly)Line(points)
	int X=point.x(), Y=point.y();
	QBrush qbrush(m_fillColor,(Qt::BrushStyle)m_brush);

	p->setPen(Qt::NoPen);
	p->setBrush(qbrush);
	switch (m_type) {
	case SNONE:
		break;
	case SCROSS:
		// EPS BUG
		p->setPen(m_color);
		p->drawLine(X-m_size,Y-m_size,X+m_size,Y+m_size);
		p->drawLine(X+m_size,Y-m_size,X-m_size,Y+m_size);
		break;
	case SDOT:
		p->setPen(m_color);
		p->drawPoint(X,Y);
		break;
	case SPLUS:
		p->setPen(m_color);
		p->drawLine(X,Y-m_size,X,Y+m_size);
		p->drawLine(X-m_size,Y,X+m_size,Y);
		break;
	case SCIRCLE:
		switch (m_fillType) {
		case FNONE:
			break;
		case FFULL:
			p->drawEllipse(X-m_size,Y-m_size,2*m_size,2*m_size);
			break;
		case FBOTTOM:
			p->drawPie(X-m_size,Y-m_size,2*m_size,2*m_size,16*180,16*180);
			break;
		case FTOP:
			p->drawPie(X-m_size,Y-m_size,2*m_size,2*m_size,0,16*180);
			break;
		case FLEFT:
			p->drawPie(X-m_size,Y-m_size,2*m_size,2*m_size,16*90,16*180);
			break;
		case FRIGHT:
			p->drawPie(X-m_size,Y-m_size,2*m_size,2*m_size,16*270,16*180);
			break;
		case FURIGHT:
			p->drawPie(X-m_size,Y-m_size,2*m_size,2*m_size,-16*45,16*180);
			break;
		case FDLEFT:
			p->drawPie(X-m_size,Y-m_size,2*m_size,2*m_size,16*(180-45),16*180);
			break;
		case FDRIGHT:
			p->drawPie(X-m_size,Y-m_size,2*m_size,2*m_size,16*(180+45),16*180);
			break;
		case FULEFT:
			p->drawPie(X-m_size,Y-m_size,2*m_size,2*m_size,16*45,16*180);
			break;
		}
		p->setPen(m_color);
		p->setBrush(Qt::NoBrush);
		p->drawEllipse(X-m_size,Y-m_size,2*m_size,2*m_size);
		break;
/*	case STRIANGLE:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(3,X-m_size,Y+m_size, X+m_size,Y+m_size,X,Y-m_size);
			break;
		case FBOTTOM:
			a.setPoints(4,X-m_size,Y+m_size, X+m_size,Y+m_size,X+m_size/2,Y, X-m_size/2,Y);
			break;
		case FTOP:
			a.setPoints(3,X,Y-m_size, X-m_size/2,Y,X+m_size/2,Y);
			break;
		case FLEFT:
			a.setPoints(3,X,Y-m_size, X-m_size,Y+m_size,X,Y+m_size);
			break;
		case FRIGHT:
			a.setPoints(3,X,Y-m_size, X+m_size,Y+m_size,X,Y+m_size);
			break;
		case FURIGHT:
			a.setPoints(3,X,Y-m_size, X-m_size/2,Y,X+m_size,Y+m_size);
			break;
		case FDLEFT:
			a.setPoints(3,X-m_size,Y+m_size, X-m_size/2,Y,X+m_size,Y+m_size);
			break;
		case FDRIGHT:
			a.setPoints(3,X-m_size,Y+m_size, X+m_size/2,Y,X+m_size,Y+m_size);
			break;
		case FULEFT:
			a.setPoints(3,X-m_size,Y+m_size, X+m_size/2,Y,X,Y-m_size);
			break;
		}
		p->drawPolygon(a);

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		a.setPoints(4,X-m_size,Y+m_size, X+m_size,Y+m_size, X,Y-m_size, X-m_size,Y+m_size);
		p->drawPolygon(a);
		break;
	case SUTRIANGLE:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(3,X-m_size,Y-m_size, X+m_size,Y-m_size,X,Y+m_size);
			break;
		case FBOTTOM:
			a.setPoints(3,X,Y+m_size, X-m_size/2,Y,X+m_size/2,Y);
			break;
		case FTOP:
			a.setPoints(4,X-m_size,Y-m_size, X+m_size,Y-m_size,X+m_size/2,Y,X-m_size/2,Y);
			break;
		case FLEFT:
			a.setPoints(3,X,Y+m_size, X-m_size,Y-m_size,X,Y-m_size);
			break;
		case FRIGHT:
			a.setPoints(3,X,Y+m_size, X+m_size,Y-m_size,X,Y-m_size);
			break;
		case FURIGHT:
			a.setPoints(3,X-m_size,Y-m_size, X+m_size/2,Y,X+m_size,Y-m_size);
			break;
		case FDLEFT:
			a.setPoints(3,X-m_size,Y-m_size, X+m_size/2,Y,X,Y+m_size);
			break;
		case FDRIGHT:
			a.setPoints(3,X+m_size,Y-m_size, X-m_size/2,Y,X,Y+m_size);
			break;
		case FULEFT:
			a.setPoints(3,X-m_size,Y-m_size, X-m_size/2,Y,X+m_size,Y-m_size);
			break;
		}
		p->drawPolygon(a);

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		a.setPoints(4,X-m_size,Y-m_size,X+m_size,Y-m_size,X,Y+m_size,X-m_size,Y-m_size);
		p->drawPolygon(a);
		break;
	case SRECT:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			p->fillRect(X-m_size,Y-m_size,2*m_size,2*m_size,qbrush);
			break;
		case FBOTTOM:
			p->fillRect(X-m_size,Y,2*m_size,m_size,qbrush);
			break;
		case FTOP:
			p->fillRect(X-m_size,Y-m_size,2*m_size,m_size,qbrush);
			break;
		case FLEFT:
			p->fillRect(X-m_size,Y-m_size,m_size,2*m_size,qbrush);
			break;
		case FRIGHT:
			p->fillRect(X,Y-m_size,m_size,2*m_size,qbrush);
			break;
		case FURIGHT:
			a.setPoints(3,X-m_size,Y-m_size, X+m_size,Y-m_size, X+m_size,Y+m_size);
			p->drawPolygon(a);
			break;
		case FDLEFT:
			a.setPoints(3,X-m_size,Y-m_size, X-m_size,Y+m_size, X+m_size,Y+m_size);
			p->drawPolygon(a);
			break;
		case FDRIGHT:
			a.setPoints(3,X-m_size,Y+m_size, X+m_size,Y+m_size, X+m_size,Y-m_size);
			p->drawPolygon(a);
			break;
		case FULEFT:
			a.setPoints(3,X-m_size,Y-m_size, X-m_size,Y+m_size, X+m_size,Y-m_size);
			p->drawPolygon(a);
			break;
		}
		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		p->drawRect(X-m_size,Y-m_size,2*m_size,2*m_size);
		break;
	case SSTAR:
		p->setPen(color);
		p->drawLine(X-m_size,Y,X+m_size,Y);
		p->drawLine(X-m_size,Y-m_size,X+m_size,Y+m_size);
		p->drawLine(X-m_size,Y+m_size,X+m_size,Y-m_size);
		break;
	case SDIAMOND:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(4,X-m_size,Y, X,Y-m_size, X+m_size,Y, X,Y+m_size);
			break;
		case FBOTTOM:
			a.setPoints(3,X-m_size,Y, X+m_size,Y, X,Y+m_size);
			break;
		case FTOP:
			a.setPoints(3,X-m_size,Y, X+m_size,Y, X,Y-m_size);
			break;
		case FLEFT:
			a.setPoints(3,X-m_size,Y, X,Y+m_size, X,Y-m_size);
			break;
		case FRIGHT:
			a.setPoints(3,X+m_size,Y, X,Y+m_size, X,Y-m_size);
			break;
		case FURIGHT:
			a.setPoints(4,X-m_size/2,Y-m_size/2, X,Y-m_size,X+m_size,Y, X+m_size/2,Y+m_size/2);
			break;
		case FDLEFT:
			a.setPoints(4,X-m_size/2,Y-m_size/2, X-m_size,Y,X,Y+m_size, X+m_size/2,Y+m_size/2);
			break;
		case FDRIGHT:
			a.setPoints(4,X-m_size/2,Y+m_size/2, X,Y+m_size,X+m_size,Y, X+m_size/2,Y-m_size/2);
			break;
		case FULEFT:
			a.setPoints(4,X-m_size/2,Y+m_size/2, X-m_size,Y,X,Y-m_size, X+m_size/2,Y-m_size/2);
			break;
		}
		p->drawPolygon(a);

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		a.setPoints(5,X-m_size,Y, X,Y-m_size, X+m_size,Y, X,Y+m_size, X-m_size,Y);
		p->drawPolygon(a);
		break;
	case SMINUS:
		p->setPen(color);
		p->drawLine(X-2*m_size,Y,X+2*m_size,Y);
		break;
	case SPIPE:
		p->setPen(color);
		p->drawLine(X,Y-2*m_size,X,Y+2*m_size);
		break;
	case SLTRIANGLE:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(3,X-m_size,Y-m_size, X-m_size,Y+m_size, X+m_size,Y);
			break;
		case FBOTTOM:
			a.setPoints(3,X-m_size,Y, X-m_size,Y+m_size, X+m_size,Y);
			break;
		case FTOP:
			a.setPoints(3,X-m_size,Y, X-m_size,Y-m_size, X+m_size,Y);
			break;
		case FLEFT:
			a.setPoints(4,X-m_size,Y-m_size, X,Y-m_size/2,X,Y+m_size/2, X-m_size,Y+m_size);
			break;
		case FRIGHT:
			a.setPoints(3, X,Y-m_size/2,X,Y+m_size/2, X+m_size,Y);
			break;
		case FURIGHT:
			a.setPoints(3, X-m_size,Y-m_size,X,Y+m_size/2, X+m_size,Y);
			break;
		case FDLEFT:
			a.setPoints(3, X-m_size,Y-m_size,X,Y+m_size/2, X-m_size,Y+m_size);
			break;
		case FDRIGHT:
			a.setPoints(3, X,Y-m_size/2,X-m_size,Y+m_size, X+m_size,Y);
			break;
		case FULEFT:
			a.setPoints(3, X,Y-m_size/2,X-m_size,Y+m_size, X-m_size,Y-m_size);
			break;
		}
		p->drawPolygon(a);

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		a.setPoints(3,X-m_size,Y-m_size, X-m_size,Y+m_size, X+m_size,Y);
		p->drawPolygon(a);
		break;
	case SRTRIANGLE:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(3,X+m_size,Y-m_size, X+m_size,Y+m_size, X-m_size,Y);
			break;
		case FBOTTOM:
			a.setPoints(3,X+m_size,Y, X+m_size,Y+m_size, X-m_size,Y);
			break;
		case FTOP:
			a.setPoints(3,X+m_size,Y-m_size, X+m_size,Y, X-m_size,Y);
			break;
		case FLEFT:
			a.setPoints(3,X-m_size,Y, X,Y-m_size/2, X,Y+m_size/2);
			break;
		case FRIGHT:
			a.setPoints(4,X+m_size,Y+m_size,X+m_size,Y-m_size, X,Y-m_size/2, X,Y+m_size/2);
			break;
		case FURIGHT:
			a.setPoints(3,X,Y-m_size/2, X+m_size,Y-m_size, X+m_size,Y+m_size);
			break;
		case FDLEFT:
			a.setPoints(3,X,Y-m_size/2, X-m_size,Y, X+m_size,Y+m_size);
			break;
		case FDRIGHT:
			a.setPoints(3,X,Y+m_size/2, X+m_size,Y-m_size, X+m_size,Y+m_size);
			break;
		case FULEFT:
			a.setPoints(3,X,Y+m_size/2, X+m_size,Y-m_size, X-m_size,Y);
			break;
		}
		p->drawPolygon(a);

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		a.setPoints(3,X+m_size,Y-m_size, X+m_size,Y+m_size, X-m_size,Y);
		p->drawPolygon(a);
		break;
	case STRIANGLE1:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(3,X-m_size,Y-m_size, X+m_size,Y-m_size, X+m_size,Y+m_size);
			break;
		case FBOTTOM:
			a.setPoints(3,X,Y, X+m_size,Y, X+m_size,Y+m_size);
			break;
		case FTOP:
			a.setPoints(4,X,Y, X+m_size,Y, X+m_size,Y-m_size,X-m_size,Y-m_size);
			break;
		case FLEFT:
			a.setPoints(3,X,Y, X,Y-m_size, X-m_size,Y-m_size);
			break;
		case FRIGHT:
			a.setPoints(4,X,Y, X,Y-m_size, X+m_size,Y-m_size,X+m_size,Y+m_size);
			break;
		case FURIGHT:
			a.setPoints(3,X,Y-m_size, X+m_size,Y, X+m_size,Y-m_size);
			break;
		case FDLEFT:
			a.setPoints(4,X,Y-m_size, X+m_size,Y, X+m_size,Y+m_size,X-m_size,Y-m_size);
			break;
		case FDRIGHT:
			a.setPoints(3,X,Y, X+m_size,Y-m_size, X+m_size,Y+m_size);
			break;
		case FULEFT:
			a.setPoints(3,X,Y, X+m_size,Y-m_size, X-m_size,Y-m_size);
			break;
		}
		p->drawPolygon(a);

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		a.setPoints(3,X-m_size,Y-m_size, X+m_size,Y-m_size, X+m_size,Y+m_size);
		p->drawPolygon(a);
		break;
	case STRIANGLE2:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(3,X-m_size,Y+m_size, X+m_size,Y+m_size, X-m_size,Y-m_size);
			break;
		case FBOTTOM:
			a.setPoints(4,X-m_size,Y, X,Y,X+m_size,Y+m_size, X-m_size,Y+m_size);
			break;
		case FTOP:
			a.setPoints(3,X-m_size,Y, X,Y,X-m_size,Y-m_size);
			break;
		case FLEFT:
			a.setPoints(4,X,Y+m_size, X,Y,X-m_size,Y-m_size,X-m_size,Y+m_size);
			break;
		case FRIGHT:
			a.setPoints(3,X,Y+m_size, X,Y,X+m_size,Y+m_size);
			break;
		case FURIGHT:
			a.setPoints(4,X-m_size,Y, X-m_size,Y-m_size,X+m_size,Y+m_size,X,Y+m_size);
			break;
		case FDLEFT:
			a.setPoints(3,X-m_size,Y, X,Y+m_size,X-m_size,Y+m_size);
			break;
		case FDRIGHT:
			a.setPoints(3,X-m_size,Y-m_size, X,Y,X+m_size,Y+m_size);
			break;
		case FULEFT:
			a.setPoints(3,X-m_size,Y-m_size, X,Y,X-m_size,Y-m_size);
			break;
		}
		p->drawPolygon(a);

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		a.setPoints(3,X-m_size,Y+m_size, X+m_size,Y+m_size, X-m_size,Y-m_size);
		p->drawPolygon(a);
		break;
	case STRIANGLE3:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(3,X-m_size,Y+m_size, X-m_size,Y-m_size, X+m_size,Y-m_size);
			break;
		case FBOTTOM:
			a.setPoints(3,X-m_size,Y, X,Y, X-m_size,Y+m_size);
			break;
		case FTOP:
			a.setPoints(4,X-m_size,Y, X,Y, X+m_size,Y-m_size,X-m_size,Y-m_size);
			break;
		case FLEFT:
			a.setPoints(4,X-m_size,Y+m_size,X-m_size,Y-m_size,X,Y-m_size,X,Y);
			break;
		case FRIGHT:
			a.setPoints(3,X+m_size,Y-m_size,X,Y-m_size,X,Y);
			break;
		case FURIGHT:
			a.setPoints(3,X-m_size,Y-m_size,X,Y,X+m_size,Y-m_size);
			break;
		case FDLEFT:
			a.setPoints(3,X-m_size,Y-m_size,X,Y,X-m_size,Y+m_size);
			break;
		case FDRIGHT:
			a.setPoints(4,X-m_size,Y,X,Y-m_size,X+m_size,Y-m_size,X-m_size,Y+m_size);
			break;
		case FULEFT:
			a.setPoints(3,X-m_size,Y-m_size,X-m_size,Y,X,Y-m_size);
			break;
		}
		p->drawPolygon(a);

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		a.setPoints(3,X-m_size,Y+m_size, X-m_size,Y-m_size, X+m_size,Y-m_size);
		p->drawPolygon(a);
		break;
	case STRIANGLE4:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(3,X-m_size,Y+m_size, X+m_size,Y+m_size, X+m_size,Y-m_size);
			break;
		case FBOTTOM:
			a.setPoints(4,X,Y,X+m_size,Y, X+m_size,Y+m_size, X-m_size,Y+m_size);
			break;
		case FTOP:
			a.setPoints(3,X,Y,X+m_size,Y, X+m_size,Y-m_size);
			break;
		case FLEFT:
			a.setPoints(3,X,Y,X,Y+m_size, X-m_size,Y+m_size);
			break;
		case FRIGHT:
			a.setPoints(4,X,Y,X,Y+m_size, X+m_size,Y+m_size,X+m_size,Y-m_size);
			break;
		case FURIGHT:
			a.setPoints(3,X,Y,X+m_size,Y+m_size, X+m_size,Y-m_size);
			break;
		case FDLEFT:
			a.setPoints(3,X,Y,X+m_size,Y+m_size, X-m_size,Y+m_size);
			break;
		case FDRIGHT:
			a.setPoints(3,X,Y+m_size,X+m_size,Y, X+m_size,Y+m_size);
			break;
		case FULEFT:
			a.setPoints(4,X,Y+m_size,X+m_size,Y, X+m_size,Y-m_size,X-m_size,Y+m_size);
			break;
		}
		p->drawPolygon(a);

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		a.setPoints(3,X-m_size,Y+m_size, X+m_size,Y+m_size, X+m_size,Y-m_size);
		p->drawPolygon(a);
		break;
	case SUCIRCLE:
		switch (fill) {
		case FNONE : case FBOTTOM :
			break;
		case FFULL: case FTOP :
			p->drawPie(X-m_size,Y-m_size,2*m_size,2*m_size,0,16*180);
			break;
		case FLEFT:
			p->drawPie(X-m_size,Y-m_size,2*m_size,2*m_size,16*90,16*90);
			break;
		case FRIGHT:
			p->drawPie(X-m_size,Y-m_size,2*m_size,2*m_size,0,16*90);
			break;
		case FURIGHT: case FDLEFT: case FDRIGHT: case FULEFT:
			break;
		}
		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		p->drawPie(X-m_size,Y-m_size,2*m_size,2*m_size,0,16*180);
		break;
	case SDCIRCLE:
		switch (fill) {
		case FNONE: case FTOP:
			break;
		case FFULL: case FBOTTOM :
			p->drawPie(X-m_size,Y-m_size,2*m_size,2*m_size,0,-16*180);
			break;
		case FLEFT:
			p->drawPie(X-m_size,Y-m_size,2*m_size,2*m_size,-16*90,-16*90);
			break;
		case FRIGHT:
			p->drawPie(X-m_size,Y-m_size,2*m_size,2*m_size,-16*90,16*90);
			break;
		case FURIGHT: case FDLEFT: case FDRIGHT : case FULEFT:
			break;
		}
		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		p->drawPie(X-m_size,Y-m_size,2*m_size,2*m_size,0,-16*180);
		break;
	case SSTAR2:
		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		p->drawLine(X,Y-m_size,X,Y+m_size);
		p->drawLine(X-m_size,Y-m_size,X+m_size,Y+m_size);
		p->drawLine(X-m_size,Y+m_size,X+m_size,Y-m_size);
		break;
	case SVBAR:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(4,X-m_size/2,Y+m_size, X+m_size/2,Y+m_size, X+m_size/2,Y-m_size,X-m_size/2,Y-m_size);
			break;
		case FBOTTOM:
			a.setPoints(4,X-m_size/2,Y, X+m_size/2,Y, X+m_size/2,Y-m_size,X-m_size/2,Y-m_size);
			break;
		case FTOP:
			a.setPoints(4,X-m_size/2,Y, X+m_size/2,Y, X+m_size/2,Y+m_size,X-m_size/2,Y+m_size);
			break;
		case FLEFT:
			a.setPoints(4,X-m_size/2,Y-m_size, X,Y-m_size, X,Y+m_size,X-m_size/2,Y+m_size);
			break;
		case FRIGHT:
			a.setPoints(4,X+m_size/2,Y-m_size, X,Y-m_size, X,Y+m_size,X+m_size/2,Y+m_size);
			break;
		case FURIGHT:
			a.setPoints(3,X-m_size/2,Y-m_size, X+m_size/2,Y-m_size, X+m_size/2,Y+m_size);
			break;
		case FDLEFT:
			a.setPoints(3,X-m_size/2,Y-m_size, X-m_size/2,Y+m_size, X+m_size/2,Y+m_size);
			break;
		case FDRIGHT:
			a.setPoints(3,X-m_size/2,Y+m_size, X+m_size/2,Y+m_size, X+m_size/2,Y-m_size);
			break;
		case FULEFT:
			a.setPoints(3,X-m_size/2,Y+m_size, X-m_size/2,Y-m_size, X+m_size/2,Y-m_size);
			break;
		}
		p->drawPolygon(a);

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		a.setPoints(4,X-m_size/2,Y+m_size, X+m_size/2,Y+m_size, X+m_size/2,Y-m_size,X-m_size/2,Y-m_size);
		p->drawPolygon(a);
		break;
	case SHBAR:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(4,X-m_size,Y+m_size/2, X+m_size,Y+m_size/2, X+m_size,Y-m_size/2,X-m_size,Y-m_size/2);
			break;
		case FBOTTOM:
			a.setPoints(4,X-m_size,Y, X+m_size,Y, X+m_size,Y-m_size/2,X-m_size,Y-m_size/2);
			break;
		case FTOP:
			a.setPoints(4,X-m_size,Y, X+m_size,Y, X+m_size,Y+m_size/2,X-m_size,Y+m_size/2);
			break;
		case FLEFT:
			a.setPoints(4,X-m_size,Y-m_size/2, X,Y-m_size/2, X,Y+m_size/2,X-m_size,Y+m_size/2);
			break;
		case FRIGHT:
			a.setPoints(4,X+m_size,Y-m_size/2, X,Y-m_size/2, X,Y+m_size/2,X+m_size,Y+m_size/2);
			break;
		case FURIGHT:
			a.setPoints(3,X-m_size,Y-m_size/2, X+m_size,Y-m_size/2, X+m_size,Y+m_size/2);
			break;
		case FDLEFT:
			a.setPoints(3,X-m_size,Y-m_size/2, X-m_size,Y+m_size/2, X+m_size,Y+m_size/2);
			break;
		case FDRIGHT:
			a.setPoints(3,X-m_size,Y+m_size/2, X+m_size,Y+m_size/2, X+m_size,Y-m_size/2);
			break;
		case FULEFT:
			a.setPoints(3,X-m_size,Y+m_size/2, X-m_size,Y-m_size/2, X+m_size,Y-m_size/2);
			break;
		}
		p->drawPolygon(a);

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		a.setPoints(4,X-m_size,Y+m_size/2, X+m_size,Y+m_size/2, X+m_size,Y-m_size/2,X-m_size,Y-m_size/2);
		p->drawPolygon(a);
		break;
	case SDIAG1:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(4,X-m_size,Y+m_size/2, X-m_size/2,Y+m_size, X+m_size,Y-m_size/2,X+m_size/2,Y-m_size);
			break;
		case FBOTTOM:
			a.setPoints(4,X-m_size,Y+m_size/2, X-m_size/2,Y, X+m_size/2,Y,X-m_size/2,Y+m_size);
			break;
		case FTOP:
			a.setPoints(4,X+m_size/2,Y+m_size, X-m_size/2,Y, X+m_size/2,Y,X+m_size,Y-m_size/2);
			break;
		case FLEFT:
			a.setPoints(4,X-m_size,Y+m_size/2, X,Y-m_size/2, X,Y+m_size/2,X-m_size/2,Y+m_size);
			break;
		case FRIGHT:
			a.setPoints(4,X+m_size/2,Y-m_size, X,Y-m_size/2, X,Y+m_size/2,X+m_size,Y-m_size/2);
			break;
		case FURIGHT:
			a.setPoints(4,X+m_size/2,Y-m_size, X+m_size,Y-m_size/2, X+m_size/4,Y+m_size/4,X-m_size/4,Y-m_size/4);
			break;
		case FDLEFT:
			a.setPoints(4,X-m_size,Y+m_size/2, X-m_size/2,Y+m_size, X+m_size/4,Y+m_size/4,X-m_size/4,Y-m_size/4);
			break;
		case FDRIGHT:
			a.setPoints(4,X-m_size/2,Y+m_size, X+m_size,Y-m_size/2, X+3*m_size/4,Y-3*m_size/4,X-3*m_size/4,Y+3*m_size/4);
			break;
		case FULEFT:
			a.setPoints(4,X-m_size,Y+m_size/2, X+m_size/2,Y-m_size, X+3*m_size/4,Y-3*m_size/4,X-3*m_size/4,Y+3*m_size/4);
			break;
		}
		p->drawPolygon(a);

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		a.setPoints(4,X-m_size,Y+m_size/2, X-m_size/2,Y+m_size, X+m_size,Y-m_size/2,X+m_size/2,Y-m_size);
		p->drawPolygon(a);
		break;
	case SDIAG2:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(4,X-m_size,Y-m_size/2, X+m_size/2,Y+m_size, X+m_size,Y+m_size/2,X-m_size/2,Y-m_size);
			break;
		case FBOTTOM:
			a.setPoints(4,X-m_size/2,Y, X+m_size/2,Y, X+m_size,Y+m_size/2,X+m_size/2,Y+m_size);
			break;
		case FTOP:
			a.setPoints(4,X-m_size/2,Y, X+m_size/2,Y, X-m_size/2,Y-m_size,X-m_size,Y-m_size/2);
			break;
		case FLEFT:
			a.setPoints(4,X-m_size,Y-m_size/2, X-m_size/2,Y-m_size, X,Y-m_size/2,X,Y+m_size/2);
			break;
		case FRIGHT:
			a.setPoints(4,X+m_size,Y+m_size/2, X+m_size/2,Y+m_size, X,Y+m_size/2,X,Y-m_size/2);
			break;
		case FURIGHT:
			a.setPoints(4,X-m_size/2,Y-m_size, X+m_size,Y+m_size/2, X+3*m_size/2,Y+3*m_size/4,X-3*m_size/4,Y-3*m_size/4);
			break;
		case FDLEFT:
			a.setPoints(4,X-m_size,Y-m_size/2, X+m_size/2,Y+m_size, X+3*m_size/4,Y+3*m_size/4,X-3*m_size/4,Y-3*m_size/4);
			break;
		case FDRIGHT:
			a.setPoints(4,X+m_size,Y+m_size/2, X+m_size/2,Y+m_size, X-m_size/4,Y+m_size/4,X+m_size/4,Y-m_size/4);
			break;
		case FULEFT:
			a.setPoints(4,X-m_size,Y-m_size/2, X-m_size/2,Y-m_size, X+m_size/4,Y-m_size/4,X-m_size/4,Y+m_size/4);
			break;
		}
		p->drawPolygon(a);

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		a.setPoints(4,X-m_size,Y-m_size/2, X+m_size/2,Y+m_size, X+m_size,Y+m_size/2,X-m_size/2,Y-m_size);
		p->drawPolygon(a);
		break;
	case SCROSS2:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(12,X-m_size,Y-m_size/2, X-m_size/2,Y-m_size, X,Y-m_size/2,X+m_size/2,Y-m_size,
				X+m_size,Y-m_size/2,X+m_size/2,Y,X+m_size,Y+m_size/2,X+m_size/2,Y+m_size,
				X,Y+m_size/2,X-m_size/2,Y+m_size,X-m_size,Y+m_size/2,X-m_size/2,Y);
			break;
		case FBOTTOM:
			a.setPoints(7,X+m_size/2,Y,X+m_size,Y+m_size/2,X+m_size/2,Y+m_size,
				X,Y+m_size/2,X-m_size/2,Y+m_size,X-m_size,Y+m_size/2,X-m_size/2,Y);
			break;
		case FTOP:
			a.setPoints(7,X-m_size,Y-m_size/2, X-m_size/2,Y-m_size, X,Y-m_size/2,X+m_size/2,Y-m_size,
				X+m_size,Y-m_size/2,X+m_size/2,Y,X-m_size/2,Y);
			break;
		case FLEFT:
			a.setPoints(7,X-m_size,Y-m_size/2, X-m_size/2,Y-m_size, X,Y-m_size/2,
				X,Y+m_size/2,X-m_size/2,Y+m_size,X-m_size,Y+m_size/2,X-m_size/2,Y);
			break;
		case FRIGHT:
			a.setPoints(7,X,Y-m_size/2,X+m_size/2,Y-m_size,X+m_size,Y-m_size/2,
				X+m_size/2,Y,X+m_size,Y+m_size/2,X+m_size/2,Y+m_size,X,Y+m_size/2);
			break;
		case FURIGHT:
			a.setPoints(8,X-3*m_size/4,Y-3*m_size/4, X-m_size/2,Y-m_size, X,Y-m_size/2,X+m_size/2,Y-m_size,
				X+m_size,Y-m_size/2,X+m_size/2,Y,X+m_size,Y+m_size/2,X+3*m_size/4,Y+3*m_size/4);
			break;
		case FDLEFT:
			a.setPoints(8,X-m_size,Y-m_size/2, X-3*m_size/4,Y-3*m_size/4, X+3*m_size/4,Y+3*m_size/4,X+m_size/2,Y+m_size,
				X,Y+m_size/2,X-m_size/2,Y+m_size, X-m_size,Y+m_size/2,X-m_size/2,Y);
			break;
		case FDRIGHT:
			a.setPoints(8,X+3*m_size/4,Y-3*m_size/4,
				X+m_size,Y-m_size/2,X+m_size/2,Y,X+m_size,Y+m_size/2,X+m_size/2,Y+m_size,
				X,Y+m_size/2,X-m_size/2,Y+m_size,X-3*m_size/4,Y+3*m_size/4);
			break;
		case FULEFT:
			a.setPoints(8,X-m_size,Y-m_size/2, X-m_size/2,Y-m_size, X,Y-m_size/2,X+m_size/2,Y-m_size,
				X+3*m_size/4,Y-3*m_size/4,X-3*m_size/4,Y+3*m_size/4,X-m_size,Y+m_size/2,X-m_size/2,Y);
			break;
		}
		p->drawPolygon(a);

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		a.setPoints(12,X-m_size,Y-m_size/2, X-m_size/2,Y-m_size, X,Y-m_size/2,X+m_size/2,Y-m_size,
			X+m_size,Y-m_size/2,X+m_size/2,Y,X+m_size,Y+m_size/2,X+m_size/2,Y+m_size,
			X,Y+m_size/2,X-m_size/2,Y+m_size,X-m_size,Y+m_size/2,X-m_size/2,Y);
		p->drawPolygon(a);
		break;
	case SDIAG3:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(6,X-m_size,Y+m_size/2, X-m_size,Y+m_size,X-m_size/2,Y+m_size, X+m_size,Y-m_size/2,X+m_size,Y-m_size,X+m_size/2,Y-m_size);
			break;
		case FBOTTOM:
			a.setPoints(5,X-m_size,Y+m_size/2, X-m_size,Y+m_size,X-m_size/2,Y+m_size, X+m_size/2,Y,X-m_size/2,Y);
			break;
		case FTOP:
			a.setPoints(5,X-m_size/2,Y,X+m_size/2,Y, X+m_size,Y-m_size/2,X+m_size,Y-m_size,X+m_size/2,Y-m_size);
			break;
		case FLEFT:
			a.setPoints(5,X-m_m_size,Y+m_m_size/2, X-m_size,Y+m_size,X-m_size/2,Y+m_size, X,Y+m_size/2,X,Y-m_size/2);
			break;
		case FRIGHT:
			a.setPoints(5,X,Y-m_size/2,X,Y+m_size/2, X+m_size,Y-m_size/2,X+m_size,Y-m_size,X+m_size/2,Y-m_size);
			break;
		case FURIGHT:
			a.setPoints(5,X-m_size/4,Y-m_size/4,X+m_size/4,Y+m_size/4, X+m_size,Y-m_size/2,X+m_size,Y-m_size,X+m_size/2,Y-m_size);
			break;
		case FDLEFT:
			a.setPoints(5,X-m_size,Y+m_size/2, X-m_size,Y+m_size,X-m_size/2,Y+m_size, X+m_size/4,Y+m_size/4,X-m_size/4,Y-m_size/4);
			break;
		case FDRIGHT:
			a.setPoints(4,X-m_size,Y+m_size, X-m_size/2,Y+m_size,X+m_size,Y-m_size/2, X+m_size,Y-m_size);
			break;
		case FULEFT:
			a.setPoints(4,X-m_size,Y+m_size, X-m_size,Y+m_size/2,X+m_size/2,Y-m_size, X+m_size,Y-m_size);
			break;
		}
		p->drawPolygon(a);

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		a.setPoints(6,X-m_size,Y+m_size/2, X-m_size,Y+m_size,X-m_size/2,Y+m_size, X+m_size,Y-m_size/2,X+m_size,Y-m_size,X+m_size/2,Y-m_size);
		p->drawPolygon(a);
		break;
	case SDIAG4:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(6,X-m_size,Y-m_size/2, X-m_size,Y-m_size,X-m_size/2,Y-m_size, X+m_size,Y+m_size/2,X+m_size,Y+m_size,X+m_size/2,Y+m_size);
			break;
		case FBOTTOM:
			a.setPoints(5,X-m_size/2,Y,X+m_size/2,Y+m_size,X+m_size,Y+m_size,X+m_size,Y+m_size/2,X+m_size/2,Y);
			break;
		case FTOP:
			a.setPoints(5,X-m_size,Y-m_size/2,X-m_size/2,Y,X+m_size/2,Y,X-m_size/2,Y-m_size,X-m_size,Y-m_size);
			break;
		case FLEFT:
			a.setPoints(5,X-m_size,Y-m_size/2,X,Y+m_size/2,X,Y-m_size/2,X-m_size/2,Y-m_size,X-m_size,Y-m_size);
			break;
		case FRIGHT:
			a.setPoints(5,X,Y+m_size/2,X+m_size/2,Y+m_size,X+m_size,Y+m_size,X+m_size,Y+m_size/2,X,Y-m_size/2);
			break;
		case FURIGHT:
			a.setPoints(4,X+m_size,Y+m_size,X+m_size,Y+m_size/2,X-m_size/2,Y-m_size,X-m_size,Y-m_size);
			break;
		case FDLEFT:
			a.setPoints(4,X-m_size,Y-m_size,X-m_size,Y-m_size/2,X+m_size/2,Y+m_size,X+m_size,Y+m_size);
			break;
		case FDRIGHT:
			a.setPoints(5,X-m_size/4,Y+m_size/4,X+m_size/2,Y+m_size,X+m_size,Y+m_size,X+m_size,Y+m_size/2,X+m_size/4,Y-m_size/4);
			break;
		case FULEFT:
			a.setPoints(5,X+m_size/4,Y-m_size/4,X-m_size/2,Y-m_size,X-m_size,Y-m_size,X-m_size,Y-m_size/2,X-m_size/4,Y+m_size/4);
			break;
		}
		p->drawPolygon(a);

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		a.setPoints(6,X-m_size,Y-m_size/2, X-m_size,Y-m_size,X-m_size/2,Y-m_size, X+m_size,Y+m_size/2,X+m_size,Y+m_size,X+m_size/2,Y+m_size);
		p->drawPolygon(a);
		break;
	case SCROSS3:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(16,X-m_size,Y+m_size,X-m_size/2,Y+m_size,X,Y+m_size/2,X+m_size/2,Y+m_size,X+m_size,Y+m_size,
				X+m_size,Y+m_size/2,X+m_size/2,Y,X+m_size,Y-m_size/2,X+m_size,Y-m_size,X+m_size/2,Y-m_size,
				X,Y-m_size/2,X-m_size/2,Y-m_size,X-m_size,Y-m_size,X-m_size,Y-m_size/2,X-m_size/2,Y,X-m_size,Y+m_size/2);
			break;
		case FBOTTOM:
			a.setPoints(9,X-m_size/2,Y,X-m_size,Y+m_size/2,X-m_size,Y+m_size,X-m_size/2,Y+m_size,X,Y+m_size/2,
				X+m_size/2,Y+m_size,X+m_size,Y+m_size,X+m_size,Y+m_size/2,X+m_size/2,Y);
			break;
		case FTOP:
			a.setPoints(9,X+m_size/2,Y,X+m_size,Y-m_size/2,X+m_size,Y-m_size,X+m_size/2,Y-m_size,X,Y-m_size/2,X-m_size/2,Y-m_size,
				X-m_size,Y-m_size,X-m_size,Y-m_size/2,X-m_size/2,Y);
			break;
		case FLEFT:
			a.setPoints(9,X,Y-m_size/2,X-m_size/2,Y-m_size,X-m_size,Y-m_size,X-m_size,Y-m_size/2,X-m_size/2,Y,X-m_size,Y+m_size/2,
				X-m_size,Y+m_size,X-m_size/2,Y+m_size,X,Y+m_size/2);
			break;
		case FRIGHT:
			a.setPoints(9,X,Y+m_size/2,X+m_size/2,Y+m_size,X+m_size,Y+m_size,X+m_size,Y+m_size/2,X+m_size/2,Y,
				X+m_size,Y-m_size/2,X+m_size,Y-m_size,X+m_size/2,Y-m_size,X,Y-m_size/2);
			break;
		case FURIGHT:
			a.setPoints(9,X+m_size,Y+m_size,X+m_size,Y+m_size/2,X+m_size/2,Y,X+m_size,Y-m_size/2,X+m_size,Y-m_size,
				X+m_size/2,Y-m_size,X,Y-m_size/2,X-m_size/2,Y-m_size,X-m_size,Y-m_size);
			break;
		case FDLEFT:
			a.setPoints(9,X-m_size,Y-m_size,X-m_size,Y-m_size/2,X-m_size/2,Y,X-m_size,Y+m_size/2,X-m_size,Y+m_size,X-m_size/2,Y+m_size,
				X,Y+m_size/2,X+m_size/2,Y+m_size,X+m_size,Y+m_size);
			break;
		case FDRIGHT:
			a.setPoints(9,X-m_size,Y+m_size,X-m_size/2,Y+m_size,X,Y+m_size/2,X+m_size/2,Y+m_size,X+m_size,Y+m_size,
				X+m_size,Y+m_size/2,X+m_size/2,Y,X+m_size,Y-m_size/2,X+m_size,Y-m_size);
			break;
		case FULEFT:
			a.setPoints(9,X+m_size,Y-m_size,X+m_size/2,Y-m_size,X,Y-m_size/2,X-m_size/2,Y-m_size,X-m_size,Y-m_size,X-m_size,Y-m_size/2,
				X-m_size/2,Y,X-m_size,Y+m_size/2,X-m_size,Y+m_size);
			break;
		}
		p->drawPolygon(a);

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		a.setPoints(16,X-m_size,Y+m_size,X-m_size/2,Y+m_size,X,Y+m_size/2,X+m_size/2,Y+m_size,X+m_size,Y+m_size,
			X+m_size,Y+m_size/2,X+m_size/2,Y,X+m_size,Y-m_size/2,X+m_size,Y-m_size,X+m_size/2,Y-m_size,X,Y-m_size/2,
			X-m_size/2,Y-m_size,X-m_size,Y-m_size,X-m_size,Y-m_size/2,X-m_size/2,Y,X-m_size,Y+m_size/2);
		p->drawPolygon(a);
		break;
	case SPARRIGHT:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(4,X-m_size,Y+m_size,X,Y-m_size,X+m_size,Y-m_size,X,Y+m_size);
			break;
		case FBOTTOM:
			a.setPoints(4,X-m_size,Y+m_size,X-m_size/2,Y,X+m_size/2,Y,X,Y+m_size);
			break;
		case FTOP:
			a.setPoints(4,X-m_size/2,Y,X,Y-m_size,X+m_size,Y-m_size,X+m_size/2,Y);
			break;
		case FRIGHT:
			a.setPoints(3,X,Y-m_size,X+m_size,Y-m_size,X,Y+m_size);
			break;
		case FLEFT:
			a.setPoints(3,X-m_size,Y+m_size,X,Y-m_size,X,Y+m_size);
			break;
		case FURIGHT:
			a.setPoints(3,X,Y-m_size,X+m_size,Y-m_size,X+m_size/2,Y);
			break;
		case FDRIGHT:
			a.setPoints(4,X-m_size/2,Y+m_size,X+m_size/2,Y-m_size,X+m_size,Y-m_size,X,Y+m_size);
			break;
		case FULEFT:
			a.setPoints(4,X-m_size,Y+m_size,X,Y-m_size,X+m_size/2,Y-m_size,X-m_size/2,Y+m_size);
			break;
		case FDLEFT:
			a.setPoints(3,X-m_size,Y+m_size,X-m_size/2,Y,X,Y+m_size);
			break;
		}
		p->drawPolygon(a);

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		a.setPoints(4,X-m_size,Y+m_size,X,Y-m_size,X+m_size,Y-m_size,X,Y+m_size);
		p->drawPolygon(a);
		break;
	case SPARLEFT:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(4,X,Y+m_size,X-m_size,Y-m_size,X,Y-m_size,X+m_size,Y+m_size);
			break;
		case FBOTTOM:
			a.setPoints(4,X,Y+m_size,X-m_size/2,Y,X+m_size/2,Y,X+m_size,Y+m_size);
			break;
		case FTOP:
			a.setPoints(4,X-m_size/2,Y,X-m_size,Y-m_size,X,Y-m_size,X+m_size/2,Y);
			break;
		case FRIGHT:
			a.setPoints(3,X,Y+m_size,X,Y-m_size,X+m_size,Y+m_size);
			break;
		case FLEFT:
			a.setPoints(3,X,Y-m_size,X-m_size,Y-m_size,X,Y+m_size);
			break;
		case FURIGHT:
			a.setPoints(4,X+m_size/2,Y+m_size,X-m_size/2,Y-m_size,X,Y-m_size,X+m_size,Y+m_size);
			break;
		case FDRIGHT:
			a.setPoints(3,X,Y+m_size,X+m_size/2,Y,X+m_size,Y+m_size);
			break;
		case FULEFT:
			a.setPoints(3,X-m_size/2,Y,X-m_size,Y-m_size,X,Y-m_size);
			break;
		case FDLEFT:
			a.setPoints(4,X,Y+m_size,X-m_size,Y-m_size,X-m_size/2,Y-m_size,X+m_size/2,Y+m_size);
			break;
		}
		p->drawPolygon(a);

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		a.setPoints(4,X,Y+m_size,X-m_size,Y-m_size,X,Y-m_size,X+m_size,Y+m_size);
		p->drawPolygon(a);
		break;
	case SHLEFTCIRCLE:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			p->drawPie(X-m_size,Y-m_size,2*m_size,2*m_size,8*180,16*180);
			break;
		case FBOTTOM:
			p->drawPie(X-m_size,Y-m_size,2*m_size,2*m_size,16*180,8*180);
			break;
		case FTOP:
			p->drawPie(X-m_size,Y-m_size,2*m_size,2*m_size,8*180,8*180);
			break;
		case FRIGHT:
			break;
		case FLEFT:
			p->drawPie(X-m_size,Y-m_size,2*m_size,2*m_size,8*180,16*180);
			break;
		case FURIGHT:
			break;
		case FDRIGHT:
			p->drawPie(X-m_size,Y-m_size,2*m_size,2*m_size,16*180,8*180);
			break;
		case FULEFT:
			p->drawPie(X-m_size,Y-m_size,2*m_size,2*m_size,8*180,8*180);
			break;
		case FDLEFT:
			break;
		}

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		p->drawPie(X-m_size,Y-m_size,2*m_size,2*m_size,8*180,16*180);
		break;
	case SHRIGHTCIRCLE:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			p->drawPie(X-m_size,Y-m_size,2*m_size,2*m_size,24*180,16*180);
			break;
		case FBOTTOM:
			p->drawPie(X-m_size,Y-m_size,2*m_size,2*m_size,24*180,8*180);
			break;
		case FTOP:
			p->drawPie(X-m_size,Y-m_size,2*m_size,2*m_size,32*180,8*180);
			break;
		case FRIGHT:
			p->drawPie(X-m_size,Y-m_size,2*m_size,2*m_size,24*180,16*180);
			break;
		case FLEFT:
			break;
		case FURIGHT:
			p->drawPie(X-m_size,Y-m_size,2*m_size,2*m_size,32*180,8*180);
			break;
		case FDRIGHT:
			p->drawPie(X-m_size,Y-m_size,2*m_size,2*m_size,24*180,8*180);
			break;
		case FULEFT:
			break;
		case FDLEFT:
			break;
		}

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		p->drawPie(X-m_size,Y-m_size,2*m_size,2*m_size,24*180,16*180);
		break;
	case SSMALLDIAMOND:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(4,X,Y-m_size,X-m_size/2,Y,X,Y+m_size,X+m_size/2,Y);
			break;
		case FBOTTOM:
			a.setPoints(3,X-m_size/2,Y,X,Y+m_size,X+m_size/2,Y);
			break;
		case FTOP:
			a.setPoints(3,X,Y-m_size,X-m_size/2,Y,X+m_size/2,Y);
			break;
		case FRIGHT:
			a.setPoints(3,X,Y-m_size,X,Y+m_size,X+m_size/2,Y);
			break;
		case FLEFT:
			a.setPoints(3,X,Y-m_size,X-m_size/2,Y,X,Y+m_size);
			break;
		case FURIGHT:
			break;
		case FDRIGHT:
			break;
		case FULEFT:
			break;
		case FDLEFT:
			break;
		}
		p->drawPolygon(a);

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		a.setPoints(4,X,Y-m_size,X-m_size/2,Y,X,Y+m_size,X+m_size/2,Y);
		p->drawPolygon(a);
		break;
	case SROTDIAMOND:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(4,X-m_size,Y,X,Y+m_size/2,X+m_size,Y,X,Y-m_size/2);
			break;
		case FBOTTOM:
			a.setPoints(3,X-m_size,Y,X,Y+m_size/2,X+m_size,Y);
			break;
		case FTOP:
			a.setPoints(3,X-m_size,Y,X+m_size,Y,X,Y-m_size/2);
			break;
		case FRIGHT:
			a.setPoints(3,X,Y+m_size/2,X+m_size,Y,X,Y-m_size/2);
			break;
		case FLEFT:
			a.setPoints(3,X-m_size,Y,X,Y+m_size/2,X,Y-m_size/2);
			break;
		case FURIGHT:
			break;
		case FDRIGHT:
			break;
		case FULEFT:
			break;
		case FDLEFT:
			break;
		}
		p->drawPolygon(a);

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		a.setPoints(4,X-m_size,Y,X,Y+m_size/2,X+m_size,Y,X,Y-m_size/2);
		p->drawPolygon(a);
		break;
	case SPENTA:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(5,X,Y-m_size,X-m_size,Y,X-m_size/2,Y+m_size,X+m_size/2,Y+m_size,X+m_size,Y);
			break;
		case FBOTTOM:
			a.setPoints(4,X-m_size,Y,X-m_size/2,Y+m_size,X+m_size/2,Y+m_size,X+m_size,Y);
			break;
		case FTOP:
			a.setPoints(3,X,Y-m_size,X-m_size,Y,X+m_size,Y);
			break;
		case FRIGHT:
			a.setPoints(4,X,Y-m_size,X,Y+m_size,X+m_size/2,Y+m_size,X+m_size,Y);
			break;
		case FLEFT:
			a.setPoints(4,X,Y-m_size,X-m_size,Y,X-m_size/2,Y+m_size,X,Y+m_size);
			break;
		case FURIGHT:
			a.setPoints(4,X,Y-m_size,X-m_size,Y,X+m_size/2,Y+m_size,X+m_size,Y);
			break;
		case FDRIGHT:
			a.setPoints(3,X-m_size/2,Y+m_size,X+m_size/2,Y+m_size,X+m_size,Y);
			break;
		case FULEFT:
			a.setPoints(4,X,Y-m_size,X-m_size,Y,X-m_size/2,Y+m_size,X+m_size,Y);
			break;
		case FDLEFT:
			a.setPoints(3,X-m_size,Y,X-m_size/2,Y+m_size,X+m_size/2,Y+m_size);
			break;
		}
		p->drawPolygon(a);

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		a.setPoints(5,X,Y-m_size,X-m_size,Y,X-m_size/2,Y+m_size,X+m_size/2,Y+m_size,X+m_size,Y);
		p->drawPolygon(a);
		break;
	case SPENTALEFT:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(5,X-m_size,Y,X,Y+m_size,X+m_size,Y+m_size/2,X+m_size,Y-m_size/2,X,Y-m_size);
			break;
		case FBOTTOM:
			a.setPoints(4,X-m_size,Y,X,Y+m_size,X+m_size,Y+m_size/2,X+m_size,Y);
			break;
		case FTOP:
			a.setPoints(4,X-m_size,Y,X+m_size,Y,X+m_size,Y-m_size/2,X,Y-m_size);
			break;
		case FRIGHT:
			a.setPoints(4,X,Y+m_size,X+m_size,Y+m_size/2,X+m_size,Y-m_size/2,X,Y-m_size);
			break;
		case FLEFT:
			a.setPoints(3,X-m_size,Y,X,Y+m_size,X,Y-m_size);
			break;
		case FURIGHT:
			a.setPoints(3,X+m_size,Y+m_size/2,X+m_size,Y-m_size/2,X,Y-m_size);
			break;
		case FDRIGHT:
			a.setPoints(3,X,Y+m_size,X+m_size,Y+m_size/2,X+m_size,Y-m_size/2);
			break;
		case FULEFT:
			a.setPoints(3,X-m_size,Y,X+m_size,Y-m_size/2,X,Y-m_size);
			break;
		case FDLEFT:
			a.setPoints(3,X-m_size,Y,X,Y+m_size,X+m_size,Y+m_size/2);
			break;
		}
		p->drawPolygon(a);

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		a.setPoints(5,X-m_size,Y,X,Y+m_size,X+m_size,Y+m_size/2,X+m_size,Y-m_size/2,X,Y-m_size);
		p->drawPolygon(a);
		break;
	case SPENTABOTTOM:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(5,X,Y+m_size,X+m_size,Y,X+m_size/2,Y-m_size,X-m_size/2,Y-m_size,X-m_size,Y);
			break;
		case FBOTTOM:
			a.setPoints(3,X,Y+m_size,X+m_size,Y,X-m_size,Y);
			break;
		case FTOP:
			a.setPoints(4,X+m_size,Y,X+m_size/2,Y-m_size,X-m_size/2,Y-m_size,X-m_size,Y);
			break;
		case FRIGHT:
			a.setPoints(4,X,Y+m_size,X+m_size,Y,X+m_size/2,Y-m_size,X,Y-m_size);
			break;
		case FLEFT:
			a.setPoints(4,X,Y+m_size,X,Y-m_size,X-m_size/2,Y-m_size,X-m_size,Y);
			break;
		case FURIGHT:
			a.setPoints(3,X+m_size,Y,X+m_size/2,Y-m_size,X-m_size/2,Y-m_size);
			break;
		case FDRIGHT:
			a.setPoints(3,X,Y+m_size,X+m_size,Y,X+m_size/2,Y-m_size);
			break;
		case FULEFT:
			a.setPoints(3,X+m_size/2,Y-m_size,X-m_size/2,Y-m_size,X-m_size,Y);
			break;
		case FDLEFT:
			a.setPoints(3,X,Y+m_size,X-m_size/2,Y-m_size,X-m_size,Y);
			break;
		}
		p->drawPolygon(a);

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		a.setPoints(5,X,Y+m_size,X+m_size,Y,X+m_size/2,Y-m_size,X-m_size/2,Y-m_size,X-m_size,Y);
		p->drawPolygon(a);
		break;
	case SPENTARIGHT:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(5,X+m_size,Y,X,Y-m_size,X-m_size,Y-m_size/2,X-m_size,Y+m_size/2,X,Y+m_size);
			break;
		case FBOTTOM:
			a.setPoints(4,X+m_size,Y,X-m_size,Y,X-m_size,Y+m_size/2,X,Y+m_size);
			break;
		case FTOP:
			a.setPoints(4,X+m_size,Y,X,Y-m_size,X-m_size,Y-m_size/2,X-m_size,Y);
			break;
		case FRIGHT:
			a.setPoints(3,X+m_size,Y,X,Y-m_size,X,Y+m_size);
			break;
		case FLEFT:
			a.setPoints(4,X,Y-m_size,X-m_size,Y-m_size/2,X-m_size,Y+m_size/2,X,Y+m_size);
			break;
		case FURIGHT:
			a.setPoints(3,X+m_size,Y,X,Y-m_size,X-m_size,Y-m_size/2);
			break;
		case FDRIGHT:
			a.setPoints(3,X+m_size,Y,X-m_size,Y+m_size/2,X,Y+m_size);
			break;
		case FULEFT:
			a.setPoints(3,X,Y-m_size,X-m_size,Y-m_size/2,X-m_size,Y+m_size/2);
			break;
		case FDLEFT:
			a.setPoints(3,X-m_size,Y-m_size/2,X-m_size,Y+m_size/2,X,Y+m_size);
			break;
		}
		p->drawPolygon(a);

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		a.setPoints(5,X+m_size,Y,X,Y-m_size,X-m_size,Y-m_size/2,X-m_size,Y+m_size/2,X,Y+m_size);
		p->drawPolygon(a);
		break;
	case SHEXAGON:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(6,X-m_size,Y,X-m_size/2,Y-m_size,X+m_size/2,Y-m_size,X+m_size,Y,X+m_size/2,Y+m_size,X-m_size/2,Y+m_size);
			break;
		case FBOTTOM:
			a.setPoints(4,X-m_size,Y,X+m_size,Y,X+m_size/2,Y+m_size,X-m_size/2,Y+m_size);
			break;
		case FTOP:
			a.setPoints(4,X-m_size,Y,X-m_size/2,Y-m_size,X+m_size/2,Y-m_size,X+m_size,Y);
			break;
		case FRIGHT:
			a.setPoints(5,X,Y-m_size,X+m_size/2,Y-m_size,X+m_size,Y,X+m_size/2,Y+m_size,X,Y+m_size);
			break;
		case FLEFT:
			a.setPoints(5,X-m_size,Y,X-m_size/2,Y-m_size,X,Y-m_size,X,Y+m_size,X-m_size/2,Y+m_size);
			break;
		case FURIGHT:
			a.setPoints(4,X-m_size/2,Y-m_size,X+m_size/2,Y-m_size,X+m_size,Y,X+m_size/2,Y+m_size);
			break;
		case FDRIGHT:
			a.setPoints(4,X+m_size/2,Y-m_size,X+m_size,Y,X+m_size/2,Y+m_size,X-m_size/2,Y+m_size);
			break;
		case FULEFT:
			a.setPoints(4,X-m_size,Y,X-m_size/2,Y-m_size,X+m_size/2,Y-m_size,X-m_size/2,Y+m_size);
			break;
		case FDLEFT:
			a.setPoints(4,X-m_size,Y,X-m_size/2,Y-m_size,X+m_size/2,Y+m_size,X-m_size/2,Y+m_size);
			break;
		}
		p->drawPolygon(a);

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		a.setPoints(6,X-m_size,Y,X-m_size/2,Y-m_size,X+m_size/2,Y-m_size,X+m_size,Y,X+m_size/2,Y+m_size,X-m_size/2,Y+m_size);
		p->drawPolygon(a);
		break;
	case SVHEXAGON:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(6,X-m_size,Y-m_size/2,X,Y-m_size,X+m_size,Y-m_size/2,X+m_size,Y+m_size/2,X,Y+m_size,X-m_size,Y+m_size/2);
			break;
		case FBOTTOM:
			a.setPoints(5,X-m_size,Y,X+m_size,Y,X+m_size,Y+m_size/2,X,Y+m_size,X-m_size,Y+m_size/2);
			break;
		case FTOP:
			a.setPoints(5,X-m_size,Y-m_size/2,X,Y-m_size,X+m_size,Y-m_size/2,X+m_size,Y,X-m_size,Y);
			break;
		case FRIGHT:
			a.setPoints(4,X,Y-m_size,X+m_size,Y-m_size/2,X+m_size,Y+m_size/2,X,Y+m_size);
			break;
		case FLEFT:
			a.setPoints(4,X-m_size,Y-m_size/2,X,Y-m_size,X,Y+m_size,X-m_size,Y+m_size/2);
			break;
		case FURIGHT:
			a.setPoints(4,X-m_size,Y-m_size/2,X,Y-m_size,X+m_size,Y-m_size/2,X+m_size,Y+m_size/2);
			break;
		case FDRIGHT:
			a.setPoints(4,X+m_size,Y-m_size/2,X+m_size,Y+m_size/2,X,Y+m_size,X-m_size,Y+m_size/2);
			break;
		case FULEFT:
			a.setPoints(4,X-m_size,Y-m_size/2,X,Y-m_size,X+m_size,Y-m_size/2,X-m_size,Y+m_size/2);
			break;
		case FDLEFT:
			a.setPoints(4,X-m_size,Y-m_size/2,X+m_size,Y+m_size/2,X,Y+m_size,X-m_size,Y+m_size/2);
			break;
		}
		p->drawPolygon(a);

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		a.setPoints(6,X-m_size,Y-m_size/2,X,Y-m_size,X+m_size,Y-m_size/2,X+m_size,Y+m_size/2,X,Y+m_size,X-m_size,Y+m_size/2);
		p->drawPolygon(a);
		break;
	case SSTAR3:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(8,X,Y-m_size,X+m_size/4,Y-m_size/4,X+m_size,Y,X+m_size/4,Y+m_size/4,
				X,Y+m_size,X-m_size/4,Y+m_size/4,X-m_size,Y,X-m_size/4,Y-m_size/4);
			break;
		case FBOTTOM:
			a.setPoints(5,X+m_size,Y,X+m_size/4,Y+m_size/4,X,Y+m_size,X-m_size/4,Y+m_size/4,X-m_size,Y);
			break;
		case FTOP:
			a.setPoints(5,X,Y-m_size,X+m_size/4,Y-m_size/4,X+m_size,Y,X-m_size,Y,X-m_size/4,Y-m_size/4);
			break;
		case FRIGHT:
			a.setPoints(5,X,Y-m_size,X+m_size/4,Y-m_size/4,X+m_size,Y,X+m_size/4,Y+m_size/4,X,Y+m_size);
			break;
		case FLEFT:
			a.setPoints(5,X,Y-m_size,X,Y+m_size,X-m_size/4,Y+m_size/4,X-m_size,Y,X-m_size/4,Y-m_size/4);
			break;
		case FURIGHT:
			a.setPoints(5,X,Y-m_size,X+m_size/4,Y-m_size/4,X+m_size,Y,X+m_size/4,Y+m_size/4,X-m_size/4,Y-m_size/4);
			break;
		case FDRIGHT:
			a.setPoints(5,X+m_size/4,Y-m_size/4,X+m_size,Y,X+m_size/4,Y+m_size/4,X,Y+m_size,X-m_size/4,Y+m_size/4);
			break;
		case FULEFT:
			a.setPoints(5,X,Y-m_size,X+m_size/4,Y-m_size/4,X-m_size/4,Y+m_size/4,X-m_size,Y,X-m_size/4,Y-m_size/4);
			break;
		case FDLEFT:
			a.setPoints(5,X+m_size/4,Y+m_size/4,X,Y+m_size,X-m_size/4,Y+m_size/4,X-m_size,Y,X-m_size/4,Y-m_size/4);
			break;
		}
		p->drawPolygon(a);

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		a.setPoints(8,X,Y-m_size,X+m_size/4,Y-m_size/4,X+m_size,Y,X+m_size/4,Y+m_size/4,
			X,Y+m_size,X-m_size/4,Y+m_size/4,X-m_size,Y,X-m_size/4,Y-m_size/4);
		p->drawPolygon(a);
		break;
	case SUARROW:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(4,X,Y-m_size,X+m_size,Y+m_size,X,Y,X-m_size,Y+m_size);
			break;
		case FBOTTOM:
			a.setPoints(5,X-m_size/2,Y,X+m_size/2,Y,X+m_size,Y+m_size,X,Y,X-m_size,Y+m_size);
			break;
		case FTOP:
			a.setPoints(3,X,Y-m_size,X+m_size/2,Y,X-m_size/2,Y);
			break;
		case FRIGHT:
			a.setPoints(3,X,Y-m_size,X+m_size,Y+m_size,X,Y);
			break;
		case FLEFT:
			a.setPoints(3,X,Y-m_size,X,Y,X-m_size,Y+m_size);
			break;
		case FURIGHT:
			a.setPoints(4,X,Y-m_size,X+m_size,Y+m_size,X,Y,X-m_size,Y+m_size);
			break;
		case FDRIGHT:
			break;
		case FULEFT:
			break;
		case FDLEFT:
			break;
		}
		p->drawPolygon(a);

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		a.setPoints(4,X,Y-m_size,X+m_size,Y+m_size,X,Y,X-m_size,Y+m_size);
		p->drawPolygon(a);
		break;
	case SLARROW:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(4,X-m_size,Y,X+m_size,Y-m_size,X,Y,X+m_size,Y+m_size);
			break;
		case FBOTTOM:
			a.setPoints(3,X-m_size,Y,X,Y,X+m_size,Y+m_size);
			break;
		case FTOP:
			a.setPoints(3,X-m_size,Y,X+m_size,Y-m_size,X,Y);
			break;
		case FRIGHT:
			a.setPoints(5,X,Y+m_size/2,X,Y-m_size/2,X+m_size,Y-m_size,X,Y,X+m_size,Y+m_size);
			break;
		case FLEFT:
			a.setPoints(4,X-m_size,Y,X,Y-m_size/2,X,Y,X,Y+m_size/2);
			break;
		case FURIGHT:
			break;
		case FDRIGHT:
			break;
		case FULEFT:
			break;
		case FDLEFT:
			break;
		}
		p->drawPolygon(a);

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		a.setPoints(4,X-m_size,Y,X+m_size,Y-m_size,X,Y,X+m_size,Y+m_size);
		p->drawPolygon(a);
		break;
	case SDARROW:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(4,X,Y+m_size,X-m_size,Y-m_size,X,Y,X+m_size,Y-m_size);
			break;
		case FBOTTOM:
			a.setPoints(3,X,Y+m_size,X-m_size/2,Y,X+m_size/2,Y);
			break;
		case FTOP:
			a.setPoints(5,X+m_size/2,Y,X-m_size/2,Y,X-m_size,Y-m_size,X,Y,X+m_size,Y-m_size);
			break;
		case FRIGHT:
			a.setPoints(3,X,Y+m_size,X,Y,X+m_size,Y-m_size);
			break;
		case FLEFT:
			a.setPoints(3,X,Y+m_size,X-m_size,Y-m_size,X,Y);
			break;
		case FURIGHT:
			break;
		case FDRIGHT:
			break;
		case FULEFT:
			break;
		case FDLEFT:
			break;
		}
		p->drawPolygon(a);

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		a.setPoints(4,X,Y+m_size,X-m_size,Y-m_size,X,Y,X+m_size,Y-m_size);
		p->drawPolygon(a);
		break;
	case SRARROW:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(4,X+m_size,Y,X-m_size,Y+m_size,X,Y,X-m_size,Y-m_size);
			break;
		case FBOTTOM:
			a.setPoints(3,X+m_size,Y,X-m_size,Y+m_size,X,Y);
			break;
		case FTOP:
			a.setPoints(3,X+m_size,Y,X,Y,X-m_size,Y-m_size);
			break;
		case FRIGHT:
			a.setPoints(3,X+m_size,Y,X,Y+m_size/2,X,Y-m_size/2);
			break;
		case FLEFT:
			a.setPoints(5,X,Y-m_size/2,X,Y+m_size/2,X-m_size,Y+m_size,X,Y,X-m_size,Y-m_size);
			break;
		case FURIGHT:
			break;
		case FDRIGHT:
			break;
		case FULEFT:
			break;
		case FDLEFT:
			break;
		}
		p->drawPolygon(a);

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		a.setPoints(4,X+m_size,Y,X-m_size,Y+m_size,X,Y,X-m_size,Y-m_size);
		p->drawPolygon(a);
		break;
	case SUHOUSE:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(5,X,Y-m_size,X+m_size,Y,X+m_size,Y+m_size,X-m_size,Y+m_size,X-m_size,Y);
			break;
		case FBOTTOM:
			a.setPoints(4,X+m_size,Y,X+m_size,Y+m_size,X-m_size,Y+m_size,X-m_size,Y);
			break;
		case FTOP:
			a.setPoints(3,X,Y-m_size,X+m_size,Y,X-m_size,Y);
			break;
		case FRIGHT:
			a.setPoints(4,X,Y-m_size,X+m_size,Y,X+m_size,Y+m_size,X,Y+m_size);
			break;
		case FLEFT:
			a.setPoints(4,X,Y-m_size,X,Y+m_size,X-m_size,Y+m_size,X-m_size,Y);
			break;
		case FURIGHT:
			a.setPoints(4,X,Y-m_size,X+m_size,Y,X+m_size,Y+m_size,X-m_size,Y);
			break;
		case FDRIGHT:
			a.setPoints(3,X+m_size,Y,X+m_size,Y+m_size,X-m_size,Y+m_size);
			break;
		case FULEFT:
			a.setPoints(4,X,Y-m_size,X+m_size,Y,X-m_size,Y+m_size,X-m_size,Y);
			break;
		case FDLEFT:
			a.setPoints(3,X+m_size,Y+m_size,X-m_size,Y+m_size,X-m_size,Y);
			break;
		}
		p->drawPolygon(a);

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		a.setPoints(5,X,Y-m_size,X+m_size,Y,X+m_size,Y+m_size,X-m_size,Y+m_size,X-m_size,Y);
		p->drawPolygon(a);
		break;
	case SLHOUSE:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(5,X-m_size,Y,X,Y-m_size,X+m_size,Y-m_size,X+m_size,Y+m_size,X,Y+m_size);
			break;
		case FBOTTOM:
			a.setPoints(4,X-m_size,Y,X+m_size,Y,X+m_size,Y+m_size,X,Y+m_size);
			break;
		case FTOP:
			a.setPoints(4,X-m_size,Y,X,Y-m_size,X+m_size,Y-m_size,X+m_size,Y);
			break;
		case FRIGHT:
			a.setPoints(4,X,Y-m_size,X+m_size,Y-m_size,X+m_size,Y+m_size,X,Y+m_size);
			break;
		case FLEFT:
			a.setPoints(3,X-m_size,Y,X,Y-m_size,X,Y+m_size);
			break;
		case FURIGHT:
			a.setPoints(3,X,Y-m_size,X+m_size,Y-m_size,X+m_size,Y+m_size);
			break;
		case FDRIGHT:
			a.setPoints(3,X+m_size,Y-m_size,X+m_size,Y+m_size,X,Y+m_size);
			break;
		case FULEFT:
			a.setPoints(4,X-m_size,Y,X,Y-m_size,X+m_size,Y-m_size,X,Y+m_size);
			break;
		case FDLEFT:
			a.setPoints(4,X-m_size,Y,X,Y-m_size,X+m_size,Y+m_size,X,Y+m_size);
			break;
		}
		p->drawPolygon(a);

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		a.setPoints(5,X-m_size,Y,X,Y-m_size,X+m_size,Y-m_size,X+m_size,Y+m_size,X,Y+m_size);
		p->drawPolygon(a);
		break;
	case SDHOUSE:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(5,X,Y+m_size,X-m_size,Y,X-m_size,Y-m_size,X+m_size,Y-m_size,X+m_size,Y);
			break;
		case FBOTTOM:
			a.setPoints(3,X,Y+m_size,X-m_size,Y,X+m_size,Y);
			break;
		case FTOP:
			a.setPoints(4,X-m_size,Y,X-m_size,Y-m_size,X+m_size,Y-m_size,X+m_size,Y);
			break;
		case FRIGHT:
			a.setPoints(4,X,Y+m_size,X,Y-m_size,X+m_size,Y-m_size,X+m_size,Y);
			break;
		case FLEFT:
			a.setPoints(4,X,Y+m_size,X-m_size,Y,X-m_size,Y-m_size,X,Y-m_size);
			break;
		case FURIGHT:
			a.setPoints(3,X-m_size,Y-m_size,X+m_size,Y-m_size,X+m_size,Y);
			break;
		case FDRIGHT:
			a.setPoints(4,X,Y+m_size,X-m_size,Y,X+m_size,Y-m_size,X+m_size,Y);
			break;
		case FULEFT:
			a.setPoints(3,X-m_size,Y,X-m_size,Y-m_size,X+m_size,Y-m_size);
			break;
		case FDLEFT:
			a.setPoints(4,X,Y+m_size,X-m_size,Y,X-m_size,Y-m_size,X+m_size,Y);
			break;
		}
		p->drawPolygon(a);

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		a.setPoints(5,X,Y+m_size,X-m_size,Y,X-m_size,Y-m_size,X+m_size,Y-m_size,X+m_size,Y);
		p->drawPolygon(a);
		break;
	case SRHOUSE:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(5,X+m_size,Y,X,Y+m_size,X-m_size,Y+m_size,X-m_size,Y-m_size,X,Y-m_size);
			break;
		case FBOTTOM:
			a.setPoints(4,X+m_size,Y,X,Y+m_size,X-m_size,Y+m_size,X-m_size,Y);
			break;
		case FTOP:
			a.setPoints(4,X+m_size,Y,X-m_size,Y,X-m_size,Y-m_size,X,Y-m_size);
			break;
		case FRIGHT:
			a.setPoints(3,X+m_size,Y,X,Y+m_size,X,Y-m_size);
			break;
		case FLEFT:
			a.setPoints(4,X,Y+m_size,X-m_size,Y+m_size,X-m_size,Y-m_size,X,Y-m_size);
			break;
		case FURIGHT:
			a.setPoints(4,X+m_size,Y,X,Y+m_size,X-m_size,Y-m_size,X,Y-m_size);
			break;
		case FDRIGHT:
			a.setPoints(4,X+m_size,Y,X,Y+m_size,X-m_size,Y+m_size,X,Y-m_size);
			break;
		case FULEFT:
			a.setPoints(3,X-m_size,Y+m_size,X-m_size,Y-m_size,X,Y-m_size);
			break;
		case FDLEFT:
			a.setPoints(3,X,Y+m_size,X-m_size,Y+m_size,X-m_size,Y-m_size);
			break;
		}
		p->drawPolygon(a);

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		a.setPoints(5,X+m_size,Y,X,Y+m_size,X-m_size,Y+m_size,X-m_size,Y-m_size,X,Y-m_size);
		p->drawPolygon(a);
		break;
*/
	}

	p->setPen(QColor("black"));
}
