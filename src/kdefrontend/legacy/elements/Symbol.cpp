/***************************************************************************
    File                 : Symbol.cc
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Stefan Gerlach, Alexander Semke
    Email (use @ for *)  : stefan.gerlach*uni-konstanz.de, alexander.semke*web.de
    Description          : plot symbol class

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/
#include "Symbol.h"
#include <KDebug>
/*
Symbol::Symbol(SymbolType type, QColor color, int size, SymbolFillType fillType, QColor fillColor, int fillBrush){
	kDebug()<<"Symbol()"<<endl;
	m_type = type;
	m_color = color;
	m_size = size;
	m_fillType = fillType;
	m_fillColor = fillColor;
	m_brush = fillBrush;
//	errorbar = new Errorbar();
}*/

Symbol::Symbol(){

}
/*!
	 return the total number in symbol types.
	The number is 50. At the moment only 5 types are implemented.
*/
int Symbol::styleCount(){
	 return 6;
}

int Symbol::fillingTypeCount(){
	 return 10;
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

QDomElement Symbol::savexML(QDomDocument doc) {
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
		tag = errorbar->savexML(doc);
		symboltag.appendChild(tag);
	}

	return symboltag;
}

void Symbol::openxML(QDomNode node) {
	while(!node.isNull()) {
		QDomElement e = node.toElement();
//		kDebug()<<"SYMBOL TAG = "<<e.tagName()<<endl;
//		kDebug()<<"SYMBOL TExT = "<<e.text()<<endl;

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
				errorbar->openxML(e.firstChild());
		}

		node = node.nextSibling();
	}
}
*/

void Symbol::draw(QPainter *p, const QPoint* point, const SymbolType symbolType, const QColor color,
					   const int size, const SymbolFillType fillType, const QColor fillColor, const Qt::BrushStyle fillBrushStyle){

	int x=point->x();
	int y=point->y();
	QPolygonF polygon;
	QBrush brush(fillColor, fillBrushStyle);
	p->setPen(color);
	p->setBrush(brush);

	switch (symbolType) {
	case SNONE:
		break;
	case SCROSS:
		// EPS BUG
		p->drawLine(x-size,y-size,x+size,y+size);
		p->drawLine(x+size,y-size,x-size,y+size);
		break;
	case SDOT:
		p->drawPoint(x,y);
		break;
	case SPLUS:
		p->drawLine(x,y-size,x,y+size);
		p->drawLine(x-size,y,x+size,y);
		break;
	case SCIRCLE:
		switch (fillType) {
		case FNONE:
			p->drawEllipse(x-size,y-size,2*size,2*size);
			break;
		case FFULL:
			p->drawEllipse(x-size,y-size,2*size,2*size);
			break;
		case FBOTTOM:
			p->drawPie(x-size,y-size,2*size,2*size,16*180,16*180);
			break;
		case FTOP:
			p->drawPie(x-size,y-size,2*size,2*size,0,16*180);
			break;
		case FLEFT:
			p->drawPie(x-size,y-size,2*size,2*size,16*90,16*180);
			break;
		case FRIGHT:
			p->drawPie(x-size,y-size,2*size,2*size,16*270,16*180);
			break;
		case FURIGHT:
			p->drawPie(x-size,y-size,2*size,2*size,-16*45,16*180);
			break;
		case FDLEFT:
			p->drawPie(x-size,y-size,2*size,2*size,16*(180-45),16*180);
			break;
		case FDRIGHT:
			p->drawPie(x-size,y-size,2*size,2*size,16*(180+45),16*180);
			break;
		case FULEFT:
			p->drawPie(x-size,y-size,2*size,2*size,16*45,16*180);
			break;
		}
		p->setBrush(Qt::NoBrush);
		p->drawEllipse(x-size,y-size,2*size,2*size);
		break;
	case STRIANGLE:
		switch (fillType) {
		case FNONE:
			break;
		case FFULL:
			polygon<<QPoint(x-size, y+size)<<QPoint(x+size, y+size)<<QPoint(x, y-size);
			break;
		case FBOTTOM:
			polygon<<QPoint(x-size, y+size)<<QPoint(x+size, y+size)<<QPoint(x+size/2, y)<<QPoint(x-size/2, y);
			break;
		case FTOP:
			polygon<<QPoint(x, y-size)<<QPoint(x-size/2, y)<<QPoint(x+size/2, y);
			break;
		case FLEFT:
			polygon<<QPoint(x, y-size)<<QPoint(x-size, y+size)<<QPoint(x, y+size);
			break;
		case FRIGHT:
			polygon<<QPoint(x, y-size)<<QPoint(x+size, y+size)<<QPoint(x, y+size);
			break;
		case FURIGHT:
			polygon<<QPoint(x, y-size)<<QPoint(x-size/2, y)<<QPoint(x+size, y+size);
			break;
		case FDLEFT:
			polygon<<QPoint(x-size, y+size)<<QPoint(x-size/2, y)<<QPoint(x+size, y+size);
			break;
		case FDRIGHT:
			polygon<<QPoint(x-size, y+size)<<QPoint(x+size/2, y)<<QPoint(x+size, y+size);
			break;
		case FULEFT:
			polygon<<QPoint(x-size, y+size)<<QPoint(x+size/2, y)<<QPoint(x, y-size);
			break;
		}
		p->drawPolygon(polygon);

		p->setBrush(Qt::NoBrush);
		polygon.clear();
		polygon<<QPoint(x-size, y+size)<<QPoint( x+size, y+size)<<QPoint(x,y-size)<<QPoint(x-size, y+size);
		p->drawPolygon(polygon);
		break;
	}
	//TODO

}

//! draw the Symbol to the specified QPainter at position x,y
//TODO Check, whether the static function above can be used everywhere and remove this function.
void Symbol::draw(QPainter* p, const QPointF& point) {
	int x=point.x();
	 int y=point.y();

	 QPolygonF polygon;
	QBrush qbrush(m_fillColor, m_fillBrushStyle);
	p->setPen(Qt::NoPen);
	p->setBrush(qbrush);

	switch (m_type) {
	case SNONE:
		break;
	case SCROSS:
		// EPS BUG
		p->setPen(m_color);
		p->drawLine(x-m_size,y-m_size,x+m_size,y+m_size);
		p->drawLine(x+m_size,y-m_size,x-m_size,y+m_size);
		break;
	case SDOT:
		p->setPen(m_color);
		p->drawPoint(x,y);
		break;
	case SPLUS:
		p->setPen(m_color);
		p->drawLine(x,y-m_size,x,y+m_size);
		p->drawLine(x-m_size,y,x+m_size,y);
		break;
	case SCIRCLE:
		switch (m_fillType) {
		case FNONE:
			break;
		case FFULL:
			p->drawEllipse(x-m_size,y-m_size,2*m_size,2*m_size);
			break;
		case FBOTTOM:
			p->drawPie(x-m_size,y-m_size,2*m_size,2*m_size,16*180,16*180);
			break;
		case FTOP:
			p->drawPie(x-m_size,y-m_size,2*m_size,2*m_size,0,16*180);
			break;
		case FLEFT:
			p->drawPie(x-m_size,y-m_size,2*m_size,2*m_size,16*90,16*180);
			break;
		case FRIGHT:
			p->drawPie(x-m_size,y-m_size,2*m_size,2*m_size,16*270,16*180);
			break;
		case FURIGHT:
			p->drawPie(x-m_size,y-m_size,2*m_size,2*m_size,-16*45,16*180);
			break;
		case FDLEFT:
			p->drawPie(x-m_size,y-m_size,2*m_size,2*m_size,16*(180-45),16*180);
			break;
		case FDRIGHT:
			p->drawPie(x-m_size,y-m_size,2*m_size,2*m_size,16*(180+45),16*180);
			break;
		case FULEFT:
			p->drawPie(x-m_size,y-m_size,2*m_size,2*m_size,16*45,16*180);
			break;
		}
		p->setPen(m_color);
		p->setBrush(Qt::NoBrush);
		p->drawEllipse(x-m_size,y-m_size,2*m_size,2*m_size);
		break;
// 	case STRIANGLE:
// 		switch (fill) {
// 		case FNONE:
// 			break;
// 		case FFULL:
// 			a.setPoints(3,x-m_size,y+m_size, x+m_size,y+m_size,x,y-m_size);
// 			break;
// 		case FBOTTOM:
// 			a.setPoints(4,x-m_size,y+m_size, x+m_size,y+m_size,x+m_size/2,y, x-m_size/2,y);
// 			break;
// 		case FTOP:
// 			a.setPoints(3,x,y-m_size, x-m_size/2,y,x+m_size/2,y);
// 			break;
// 		case FLEFT:
// 			a.setPoints(3,x,y-m_size, x-m_size,y+m_size,x,y+m_size);
// 			break;
// 		case FRIGHT:
// 			a.setPoints(3,x,y-m_size, x+m_size,y+m_size,x,y+m_size);
// 			break;
// 		case FURIGHT:
// 			a.setPoints(3,x,y-m_size, x-m_size/2,y,x+m_size,y+m_size);
// 			break;
// 		case FDLEFT:
// 			a.setPoints(3,x-m_size,y+m_size, x-m_size/2,y,x+m_size,y+m_size);
// 			break;
// 		case FDRIGHT:
// 			a.setPoints(3,x-m_size,y+m_size, x+m_size/2,y,x+m_size,y+m_size);
// 			break;
// 		case FULEFT:
// 			a.setPoints(3,x-m_size,y+m_size, x+m_size/2,y,x,y-m_size);
// 			break;
// 		}
// 		p->drawPolygon(a);
//
// 		p->setPen(color);
// 		p->setBrush(Qt::NoBrush);
// 		a.setPoints(4,x-m_size,y+m_size, x+m_size,y+m_size, x,y-m_size, x-m_size,y+m_size);
// 		p->drawPolygon(a);
// 		break;
	case STRIANGLE:
		switch (m_fillType) {
		case FNONE:
			break;
		case FFULL:
			polygon<<QPoint(x-m_size, y+m_size)<<QPoint(x+m_size, y+m_size)<<QPoint(x, y-m_size);
			break;
		case FBOTTOM:
			polygon<<QPoint(x-m_size, y+m_size)<<QPoint(x+m_size, y+m_size)<<QPoint(x+m_size/2, y)<<QPoint(x-m_size/2, y);
			break;
		case FTOP:
			polygon<<QPoint(x, y-m_size)<<QPoint(x-m_size/2, y)<<QPoint(x+m_size/2, y);
			break;
		case FLEFT:
			polygon<<QPoint(x, y-m_size)<<QPoint(x-m_size, y+m_size)<<QPoint(x, y+m_size);
			break;
		case FRIGHT:
			polygon<<QPoint(x, y-m_size)<<QPoint(x+m_size, y+m_size)<<QPoint(x, y+m_size);
			break;
		case FURIGHT:
			polygon<<QPoint(x, y-m_size)<<QPoint(x-m_size/2, y)<<QPoint(x+m_size, y+m_size);
			break;
		case FDLEFT:
			polygon<<QPoint(x-m_size, y+m_size)<<QPoint(x-m_size/2, y)<<QPoint(x+m_size, y+m_size);
			break;
		case FDRIGHT:
			polygon<<QPoint(x-m_size, y+m_size)<<QPoint(x+m_size/2, y)<<QPoint(x+m_size, y+m_size);
			break;
		case FULEFT:
			polygon<<QPoint(x-m_size, y+m_size)<<QPoint(x+m_size/2, y)<<QPoint(x, y-m_size);
			break;
		}
		p->drawPolygon(polygon);

		p->setBrush(Qt::NoBrush);
		polygon.clear();
		polygon<<QPoint(x-m_size, y+m_size)<<QPoint( x+m_size, y+m_size)<<QPoint(x,y-m_size)<<QPoint(x-m_size, y+m_size);
		p->drawPolygon(polygon);
		break;

	/*case SUTRIANGLE:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(3,x-m_size,y-m_size, x+m_size,y-m_size,x,y+m_size);
			break;
		case FBOTTOM:
			a.setPoints(3,x,y+m_size, x-m_size/2,y,x+m_size/2,y);
			break;
		case FTOP:
			a.setPoints(4,x-m_size,y-m_size, x+m_size,y-m_size,x+m_size/2,y,x-m_size/2,y);
			break;
		case FLEFT:
			a.setPoints(3,x,y+m_size, x-m_size,y-m_size,x,y-m_size);
			break;
		case FRIGHT:
			a.setPoints(3,x,y+m_size, x+m_size,y-m_size,x,y-m_size);
			break;
		case FURIGHT:
			a.setPoints(3,x-m_size,y-m_size, x+m_size/2,y,x+m_size,y-m_size);
			break;
		case FDLEFT:
			a.setPoints(3,x-m_size,y-m_size, x+m_size/2,y,x,y+m_size);
			break;
		case FDRIGHT:
			a.setPoints(3,x+m_size,y-m_size, x-m_size/2,y,x,y+m_size);
			break;
		case FULEFT:
			a.setPoints(3,x-m_size,y-m_size, x-m_size/2,y,x+m_size,y-m_size);
			break;
		}
		p->drawPolygon(a);

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		a.setPoints(4,x-m_size,y-m_size,x+m_size,y-m_size,x,y+m_size,x-m_size,y-m_size);
		p->drawPolygon(a);
		break;
	case SRECT:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			p->fillRect(x-m_size,y-m_size,2*m_size,2*m_size,qbrush);
			break;
		case FBOTTOM:
			p->fillRect(x-m_size,y,2*m_size,m_size,qbrush);
			break;
		case FTOP:
			p->fillRect(x-m_size,y-m_size,2*m_size,m_size,qbrush);
			break;
		case FLEFT:
			p->fillRect(x-m_size,y-m_size,m_size,2*m_size,qbrush);
			break;
		case FRIGHT:
			p->fillRect(x,y-m_size,m_size,2*m_size,qbrush);
			break;
		case FURIGHT:
			a.setPoints(3,x-m_size,y-m_size, x+m_size,y-m_size, x+m_size,y+m_size);
			p->drawPolygon(a);
			break;
		case FDLEFT:
			a.setPoints(3,x-m_size,y-m_size, x-m_size,y+m_size, x+m_size,y+m_size);
			p->drawPolygon(a);
			break;
		case FDRIGHT:
			a.setPoints(3,x-m_size,y+m_size, x+m_size,y+m_size, x+m_size,y-m_size);
			p->drawPolygon(a);
			break;
		case FULEFT:
			a.setPoints(3,x-m_size,y-m_size, x-m_size,y+m_size, x+m_size,y-m_size);
			p->drawPolygon(a);
			break;
		}
		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		p->drawRect(x-m_size,y-m_size,2*m_size,2*m_size);
		break;
	case SSTAR:
		p->setPen(color);
		p->drawLine(x-m_size,y,x+m_size,y);
		p->drawLine(x-m_size,y-m_size,x+m_size,y+m_size);
		p->drawLine(x-m_size,y+m_size,x+m_size,y-m_size);
		break;
	case SDIAMOND:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(4,x-m_size,y, x,y-m_size, x+m_size,y, x,y+m_size);
			break;
		case FBOTTOM:
			a.setPoints(3,x-m_size,y, x+m_size,y, x,y+m_size);
			break;
		case FTOP:
			a.setPoints(3,x-m_size,y, x+m_size,y, x,y-m_size);
			break;
		case FLEFT:
			a.setPoints(3,x-m_size,y, x,y+m_size, x,y-m_size);
			break;
		case FRIGHT:
			a.setPoints(3,x+m_size,y, x,y+m_size, x,y-m_size);
			break;
		case FURIGHT:
			a.setPoints(4,x-m_size/2,y-m_size/2, x,y-m_size,x+m_size,y, x+m_size/2,y+m_size/2);
			break;
		case FDLEFT:
			a.setPoints(4,x-m_size/2,y-m_size/2, x-m_size,y,x,y+m_size, x+m_size/2,y+m_size/2);
			break;
		case FDRIGHT:
			a.setPoints(4,x-m_size/2,y+m_size/2, x,y+m_size,x+m_size,y, x+m_size/2,y-m_size/2);
			break;
		case FULEFT:
			a.setPoints(4,x-m_size/2,y+m_size/2, x-m_size,y,x,y-m_size, x+m_size/2,y-m_size/2);
			break;
		}
		p->drawPolygon(a);

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		a.setPoints(5,x-m_size,y, x,y-m_size, x+m_size,y, x,y+m_size, x-m_size,y);
		p->drawPolygon(a);
		break;
	case SMINUS:
		p->setPen(color);
		p->drawLine(x-2*m_size,y,x+2*m_size,y);
		break;
	case SPIPE:
		p->setPen(color);
		p->drawLine(x,y-2*m_size,x,y+2*m_size);
		break;
	case SLTRIANGLE:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(3,x-m_size,y-m_size, x-m_size,y+m_size, x+m_size,y);
			break;
		case FBOTTOM:
			a.setPoints(3,x-m_size,y, x-m_size,y+m_size, x+m_size,y);
			break;
		case FTOP:
			a.setPoints(3,x-m_size,y, x-m_size,y-m_size, x+m_size,y);
			break;
		case FLEFT:
			a.setPoints(4,x-m_size,y-m_size, x,y-m_size/2,x,y+m_size/2, x-m_size,y+m_size);
			break;
		case FRIGHT:
			a.setPoints(3, x,y-m_size/2,x,y+m_size/2, x+m_size,y);
			break;
		case FURIGHT:
			a.setPoints(3, x-m_size,y-m_size,x,y+m_size/2, x+m_size,y);
			break;
		case FDLEFT:
			a.setPoints(3, x-m_size,y-m_size,x,y+m_size/2, x-m_size,y+m_size);
			break;
		case FDRIGHT:
			a.setPoints(3, x,y-m_size/2,x-m_size,y+m_size, x+m_size,y);
			break;
		case FULEFT:
			a.setPoints(3, x,y-m_size/2,x-m_size,y+m_size, x-m_size,y-m_size);
			break;
		}
		p->drawPolygon(a);

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		a.setPoints(3,x-m_size,y-m_size, x-m_size,y+m_size, x+m_size,y);
		p->drawPolygon(a);
		break;
	case SRTRIANGLE:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(3,x+m_size,y-m_size, x+m_size,y+m_size, x-m_size,y);
			break;
		case FBOTTOM:
			a.setPoints(3,x+m_size,y, x+m_size,y+m_size, x-m_size,y);
			break;
		case FTOP:
			a.setPoints(3,x+m_size,y-m_size, x+m_size,y, x-m_size,y);
			break;
		case FLEFT:
			a.setPoints(3,x-m_size,y, x,y-m_size/2, x,y+m_size/2);
			break;
		case FRIGHT:
			a.setPoints(4,x+m_size,y+m_size,x+m_size,y-m_size, x,y-m_size/2, x,y+m_size/2);
			break;
		case FURIGHT:
			a.setPoints(3,x,y-m_size/2, x+m_size,y-m_size, x+m_size,y+m_size);
			break;
		case FDLEFT:
			a.setPoints(3,x,y-m_size/2, x-m_size,y, x+m_size,y+m_size);
			break;
		case FDRIGHT:
			a.setPoints(3,x,y+m_size/2, x+m_size,y-m_size, x+m_size,y+m_size);
			break;
		case FULEFT:
			a.setPoints(3,x,y+m_size/2, x+m_size,y-m_size, x-m_size,y);
			break;
		}
		p->drawPolygon(a);

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		a.setPoints(3,x+m_size,y-m_size, x+m_size,y+m_size, x-m_size,y);
		p->drawPolygon(a);
		break;
	case STRIANGLE1:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(3,x-m_size,y-m_size, x+m_size,y-m_size, x+m_size,y+m_size);
			break;
		case FBOTTOM:
			a.setPoints(3,x,y, x+m_size,y, x+m_size,y+m_size);
			break;
		case FTOP:
			a.setPoints(4,x,y, x+m_size,y, x+m_size,y-m_size,x-m_size,y-m_size);
			break;
		case FLEFT:
			a.setPoints(3,x,y, x,y-m_size, x-m_size,y-m_size);
			break;
		case FRIGHT:
			a.setPoints(4,x,y, x,y-m_size, x+m_size,y-m_size,x+m_size,y+m_size);
			break;
		case FURIGHT:
			a.setPoints(3,x,y-m_size, x+m_size,y, x+m_size,y-m_size);
			break;
		case FDLEFT:
			a.setPoints(4,x,y-m_size, x+m_size,y, x+m_size,y+m_size,x-m_size,y-m_size);
			break;
		case FDRIGHT:
			a.setPoints(3,x,y, x+m_size,y-m_size, x+m_size,y+m_size);
			break;
		case FULEFT:
			a.setPoints(3,x,y, x+m_size,y-m_size, x-m_size,y-m_size);
			break;
		}
		p->drawPolygon(a);

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		a.setPoints(3,x-m_size,y-m_size, x+m_size,y-m_size, x+m_size,y+m_size);
		p->drawPolygon(a);
		break;
	case STRIANGLE2:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(3,x-m_size,y+m_size, x+m_size,y+m_size, x-m_size,y-m_size);
			break;
		case FBOTTOM:
			a.setPoints(4,x-m_size,y, x,y,x+m_size,y+m_size, x-m_size,y+m_size);
			break;
		case FTOP:
			a.setPoints(3,x-m_size,y, x,y,x-m_size,y-m_size);
			break;
		case FLEFT:
			a.setPoints(4,x,y+m_size, x,y,x-m_size,y-m_size,x-m_size,y+m_size);
			break;
		case FRIGHT:
			a.setPoints(3,x,y+m_size, x,y,x+m_size,y+m_size);
			break;
		case FURIGHT:
			a.setPoints(4,x-m_size,y, x-m_size,y-m_size,x+m_size,y+m_size,x,y+m_size);
			break;
		case FDLEFT:
			a.setPoints(3,x-m_size,y, x,y+m_size,x-m_size,y+m_size);
			break;
		case FDRIGHT:
			a.setPoints(3,x-m_size,y-m_size, x,y,x+m_size,y+m_size);
			break;
		case FULEFT:
			a.setPoints(3,x-m_size,y-m_size, x,y,x-m_size,y-m_size);
			break;
		}
		p->drawPolygon(a);

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		a.setPoints(3,x-m_size,y+m_size, x+m_size,y+m_size, x-m_size,y-m_size);
		p->drawPolygon(a);
		break;
	case STRIANGLE3:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(3,x-m_size,y+m_size, x-m_size,y-m_size, x+m_size,y-m_size);
			break;
		case FBOTTOM:
			a.setPoints(3,x-m_size,y, x,y, x-m_size,y+m_size);
			break;
		case FTOP:
			a.setPoints(4,x-m_size,y, x,y, x+m_size,y-m_size,x-m_size,y-m_size);
			break;
		case FLEFT:
			a.setPoints(4,x-m_size,y+m_size,x-m_size,y-m_size,x,y-m_size,x,y);
			break;
		case FRIGHT:
			a.setPoints(3,x+m_size,y-m_size,x,y-m_size,x,y);
			break;
		case FURIGHT:
			a.setPoints(3,x-m_size,y-m_size,x,y,x+m_size,y-m_size);
			break;
		case FDLEFT:
			a.setPoints(3,x-m_size,y-m_size,x,y,x-m_size,y+m_size);
			break;
		case FDRIGHT:
			a.setPoints(4,x-m_size,y,x,y-m_size,x+m_size,y-m_size,x-m_size,y+m_size);
			break;
		case FULEFT:
			a.setPoints(3,x-m_size,y-m_size,x-m_size,y,x,y-m_size);
			break;
		}
		p->drawPolygon(a);

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		a.setPoints(3,x-m_size,y+m_size, x-m_size,y-m_size, x+m_size,y-m_size);
		p->drawPolygon(a);
		break;
	case STRIANGLE4:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(3,x-m_size,y+m_size, x+m_size,y+m_size, x+m_size,y-m_size);
			break;
		case FBOTTOM:
			a.setPoints(4,x,y,x+m_size,y, x+m_size,y+m_size, x-m_size,y+m_size);
			break;
		case FTOP:
			a.setPoints(3,x,y,x+m_size,y, x+m_size,y-m_size);
			break;
		case FLEFT:
			a.setPoints(3,x,y,x,y+m_size, x-m_size,y+m_size);
			break;
		case FRIGHT:
			a.setPoints(4,x,y,x,y+m_size, x+m_size,y+m_size,x+m_size,y-m_size);
			break;
		case FURIGHT:
			a.setPoints(3,x,y,x+m_size,y+m_size, x+m_size,y-m_size);
			break;
		case FDLEFT:
			a.setPoints(3,x,y,x+m_size,y+m_size, x-m_size,y+m_size);
			break;
		case FDRIGHT:
			a.setPoints(3,x,y+m_size,x+m_size,y, x+m_size,y+m_size);
			break;
		case FULEFT:
			a.setPoints(4,x,y+m_size,x+m_size,y, x+m_size,y-m_size,x-m_size,y+m_size);
			break;
		}
		p->drawPolygon(a);

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		a.setPoints(3,x-m_size,y+m_size, x+m_size,y+m_size, x+m_size,y-m_size);
		p->drawPolygon(a);
		break;
	case SUCIRCLE:
		switch (fill) {
		case FNONE : case FBOTTOM :
			break;
		case FFULL: case FTOP :
			p->drawPie(x-m_size,y-m_size,2*m_size,2*m_size,0,16*180);
			break;
		case FLEFT:
			p->drawPie(x-m_size,y-m_size,2*m_size,2*m_size,16*90,16*90);
			break;
		case FRIGHT:
			p->drawPie(x-m_size,y-m_size,2*m_size,2*m_size,0,16*90);
			break;
		case FURIGHT: case FDLEFT: case FDRIGHT: case FULEFT:
			break;
		}
		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		p->drawPie(x-m_size,y-m_size,2*m_size,2*m_size,0,16*180);
		break;
	case SDCIRCLE:
		switch (fill) {
		case FNONE: case FTOP:
			break;
		case FFULL: case FBOTTOM :
			p->drawPie(x-m_size,y-m_size,2*m_size,2*m_size,0,-16*180);
			break;
		case FLEFT:
			p->drawPie(x-m_size,y-m_size,2*m_size,2*m_size,-16*90,-16*90);
			break;
		case FRIGHT:
			p->drawPie(x-m_size,y-m_size,2*m_size,2*m_size,-16*90,16*90);
			break;
		case FURIGHT: case FDLEFT: case FDRIGHT : case FULEFT:
			break;
		}
		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		p->drawPie(x-m_size,y-m_size,2*m_size,2*m_size,0,-16*180);
		break;
	case SSTAR2:
		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		p->drawLine(x,y-m_size,x,y+m_size);
		p->drawLine(x-m_size,y-m_size,x+m_size,y+m_size);
		p->drawLine(x-m_size,y+m_size,x+m_size,y-m_size);
		break;
	case SVBAR:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(4,x-m_size/2,y+m_size, x+m_size/2,y+m_size, x+m_size/2,y-m_size,x-m_size/2,y-m_size);
			break;
		case FBOTTOM:
			a.setPoints(4,x-m_size/2,y, x+m_size/2,y, x+m_size/2,y-m_size,x-m_size/2,y-m_size);
			break;
		case FTOP:
			a.setPoints(4,x-m_size/2,y, x+m_size/2,y, x+m_size/2,y+m_size,x-m_size/2,y+m_size);
			break;
		case FLEFT:
			a.setPoints(4,x-m_size/2,y-m_size, x,y-m_size, x,y+m_size,x-m_size/2,y+m_size);
			break;
		case FRIGHT:
			a.setPoints(4,x+m_size/2,y-m_size, x,y-m_size, x,y+m_size,x+m_size/2,y+m_size);
			break;
		case FURIGHT:
			a.setPoints(3,x-m_size/2,y-m_size, x+m_size/2,y-m_size, x+m_size/2,y+m_size);
			break;
		case FDLEFT:
			a.setPoints(3,x-m_size/2,y-m_size, x-m_size/2,y+m_size, x+m_size/2,y+m_size);
			break;
		case FDRIGHT:
			a.setPoints(3,x-m_size/2,y+m_size, x+m_size/2,y+m_size, x+m_size/2,y-m_size);
			break;
		case FULEFT:
			a.setPoints(3,x-m_size/2,y+m_size, x-m_size/2,y-m_size, x+m_size/2,y-m_size);
			break;
		}
		p->drawPolygon(a);

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		a.setPoints(4,x-m_size/2,y+m_size, x+m_size/2,y+m_size, x+m_size/2,y-m_size,x-m_size/2,y-m_size);
		p->drawPolygon(a);
		break;
	case SHBAR:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(4,x-m_size,y+m_size/2, x+m_size,y+m_size/2, x+m_size,y-m_size/2,x-m_size,y-m_size/2);
			break;
		case FBOTTOM:
			a.setPoints(4,x-m_size,y, x+m_size,y, x+m_size,y-m_size/2,x-m_size,y-m_size/2);
			break;
		case FTOP:
			a.setPoints(4,x-m_size,y, x+m_size,y, x+m_size,y+m_size/2,x-m_size,y+m_size/2);
			break;
		case FLEFT:
			a.setPoints(4,x-m_size,y-m_size/2, x,y-m_size/2, x,y+m_size/2,x-m_size,y+m_size/2);
			break;
		case FRIGHT:
			a.setPoints(4,x+m_size,y-m_size/2, x,y-m_size/2, x,y+m_size/2,x+m_size,y+m_size/2);
			break;
		case FURIGHT:
			a.setPoints(3,x-m_size,y-m_size/2, x+m_size,y-m_size/2, x+m_size,y+m_size/2);
			break;
		case FDLEFT:
			a.setPoints(3,x-m_size,y-m_size/2, x-m_size,y+m_size/2, x+m_size,y+m_size/2);
			break;
		case FDRIGHT:
			a.setPoints(3,x-m_size,y+m_size/2, x+m_size,y+m_size/2, x+m_size,y-m_size/2);
			break;
		case FULEFT:
			a.setPoints(3,x-m_size,y+m_size/2, x-m_size,y-m_size/2, x+m_size,y-m_size/2);
			break;
		}
		p->drawPolygon(a);

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		a.setPoints(4,x-m_size,y+m_size/2, x+m_size,y+m_size/2, x+m_size,y-m_size/2,x-m_size,y-m_size/2);
		p->drawPolygon(a);
		break;
	case SDIAG1:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(4,x-m_size,y+m_size/2, x-m_size/2,y+m_size, x+m_size,y-m_size/2,x+m_size/2,y-m_size);
			break;
		case FBOTTOM:
			a.setPoints(4,x-m_size,y+m_size/2, x-m_size/2,y, x+m_size/2,y,x-m_size/2,y+m_size);
			break;
		case FTOP:
			a.setPoints(4,x+m_size/2,y+m_size, x-m_size/2,y, x+m_size/2,y,x+m_size,y-m_size/2);
			break;
		case FLEFT:
			a.setPoints(4,x-m_size,y+m_size/2, x,y-m_size/2, x,y+m_size/2,x-m_size/2,y+m_size);
			break;
		case FRIGHT:
			a.setPoints(4,x+m_size/2,y-m_size, x,y-m_size/2, x,y+m_size/2,x+m_size,y-m_size/2);
			break;
		case FURIGHT:
			a.setPoints(4,x+m_size/2,y-m_size, x+m_size,y-m_size/2, x+m_size/4,y+m_size/4,x-m_size/4,y-m_size/4);
			break;
		case FDLEFT:
			a.setPoints(4,x-m_size,y+m_size/2, x-m_size/2,y+m_size, x+m_size/4,y+m_size/4,x-m_size/4,y-m_size/4);
			break;
		case FDRIGHT:
			a.setPoints(4,x-m_size/2,y+m_size, x+m_size,y-m_size/2, x+3*m_size/4,y-3*m_size/4,x-3*m_size/4,y+3*m_size/4);
			break;
		case FULEFT:
			a.setPoints(4,x-m_size,y+m_size/2, x+m_size/2,y-m_size, x+3*m_size/4,y-3*m_size/4,x-3*m_size/4,y+3*m_size/4);
			break;
		}
		p->drawPolygon(a);

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		a.setPoints(4,x-m_size,y+m_size/2, x-m_size/2,y+m_size, x+m_size,y-m_size/2,x+m_size/2,y-m_size);
		p->drawPolygon(a);
		break;
	case SDIAG2:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(4,x-m_size,y-m_size/2, x+m_size/2,y+m_size, x+m_size,y+m_size/2,x-m_size/2,y-m_size);
			break;
		case FBOTTOM:
			a.setPoints(4,x-m_size/2,y, x+m_size/2,y, x+m_size,y+m_size/2,x+m_size/2,y+m_size);
			break;
		case FTOP:
			a.setPoints(4,x-m_size/2,y, x+m_size/2,y, x-m_size/2,y-m_size,x-m_size,y-m_size/2);
			break;
		case FLEFT:
			a.setPoints(4,x-m_size,y-m_size/2, x-m_size/2,y-m_size, x,y-m_size/2,x,y+m_size/2);
			break;
		case FRIGHT:
			a.setPoints(4,x+m_size,y+m_size/2, x+m_size/2,y+m_size, x,y+m_size/2,x,y-m_size/2);
			break;
		case FURIGHT:
			a.setPoints(4,x-m_size/2,y-m_size, x+m_size,y+m_size/2, x+3*m_size/2,y+3*m_size/4,x-3*m_size/4,y-3*m_size/4);
			break;
		case FDLEFT:
			a.setPoints(4,x-m_size,y-m_size/2, x+m_size/2,y+m_size, x+3*m_size/4,y+3*m_size/4,x-3*m_size/4,y-3*m_size/4);
			break;
		case FDRIGHT:
			a.setPoints(4,x+m_size,y+m_size/2, x+m_size/2,y+m_size, x-m_size/4,y+m_size/4,x+m_size/4,y-m_size/4);
			break;
		case FULEFT:
			a.setPoints(4,x-m_size,y-m_size/2, x-m_size/2,y-m_size, x+m_size/4,y-m_size/4,x-m_size/4,y+m_size/4);
			break;
		}
		p->drawPolygon(a);

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		a.setPoints(4,x-m_size,y-m_size/2, x+m_size/2,y+m_size, x+m_size,y+m_size/2,x-m_size/2,y-m_size);
		p->drawPolygon(a);
		break;
	case SCROSS2:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(12,x-m_size,y-m_size/2, x-m_size/2,y-m_size, x,y-m_size/2,x+m_size/2,y-m_size,
				x+m_size,y-m_size/2,x+m_size/2,y,x+m_size,y+m_size/2,x+m_size/2,y+m_size,
				x,y+m_size/2,x-m_size/2,y+m_size,x-m_size,y+m_size/2,x-m_size/2,y);
			break;
		case FBOTTOM:
			a.setPoints(7,x+m_size/2,y,x+m_size,y+m_size/2,x+m_size/2,y+m_size,
				x,y+m_size/2,x-m_size/2,y+m_size,x-m_size,y+m_size/2,x-m_size/2,y);
			break;
		case FTOP:
			a.setPoints(7,x-m_size,y-m_size/2, x-m_size/2,y-m_size, x,y-m_size/2,x+m_size/2,y-m_size,
				x+m_size,y-m_size/2,x+m_size/2,y,x-m_size/2,y);
			break;
		case FLEFT:
			a.setPoints(7,x-m_size,y-m_size/2, x-m_size/2,y-m_size, x,y-m_size/2,
				x,y+m_size/2,x-m_size/2,y+m_size,x-m_size,y+m_size/2,x-m_size/2,y);
			break;
		case FRIGHT:
			a.setPoints(7,x,y-m_size/2,x+m_size/2,y-m_size,x+m_size,y-m_size/2,
				x+m_size/2,y,x+m_size,y+m_size/2,x+m_size/2,y+m_size,x,y+m_size/2);
			break;
		case FURIGHT:
			a.setPoints(8,x-3*m_size/4,y-3*m_size/4, x-m_size/2,y-m_size, x,y-m_size/2,x+m_size/2,y-m_size,
				x+m_size,y-m_size/2,x+m_size/2,y,x+m_size,y+m_size/2,x+3*m_size/4,y+3*m_size/4);
			break;
		case FDLEFT:
			a.setPoints(8,x-m_size,y-m_size/2, x-3*m_size/4,y-3*m_size/4, x+3*m_size/4,y+3*m_size/4,x+m_size/2,y+m_size,
				x,y+m_size/2,x-m_size/2,y+m_size, x-m_size,y+m_size/2,x-m_size/2,y);
			break;
		case FDRIGHT:
			a.setPoints(8,x+3*m_size/4,y-3*m_size/4,
				x+m_size,y-m_size/2,x+m_size/2,y,x+m_size,y+m_size/2,x+m_size/2,y+m_size,
				x,y+m_size/2,x-m_size/2,y+m_size,x-3*m_size/4,y+3*m_size/4);
			break;
		case FULEFT:
			a.setPoints(8,x-m_size,y-m_size/2, x-m_size/2,y-m_size, x,y-m_size/2,x+m_size/2,y-m_size,
				x+3*m_size/4,y-3*m_size/4,x-3*m_size/4,y+3*m_size/4,x-m_size,y+m_size/2,x-m_size/2,y);
			break;
		}
		p->drawPolygon(a);

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		a.setPoints(12,x-m_size,y-m_size/2, x-m_size/2,y-m_size, x,y-m_size/2,x+m_size/2,y-m_size,
			x+m_size,y-m_size/2,x+m_size/2,y,x+m_size,y+m_size/2,x+m_size/2,y+m_size,
			x,y+m_size/2,x-m_size/2,y+m_size,x-m_size,y+m_size/2,x-m_size/2,y);
		p->drawPolygon(a);
		break;
	case SDIAG3:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(6,x-m_size,y+m_size/2, x-m_size,y+m_size,x-m_size/2,y+m_size, x+m_size,y-m_size/2,x+m_size,y-m_size,x+m_size/2,y-m_size);
			break;
		case FBOTTOM:
			a.setPoints(5,x-m_size,y+m_size/2, x-m_size,y+m_size,x-m_size/2,y+m_size, x+m_size/2,y,x-m_size/2,y);
			break;
		case FTOP:
			a.setPoints(5,x-m_size/2,y,x+m_size/2,y, x+m_size,y-m_size/2,x+m_size,y-m_size,x+m_size/2,y-m_size);
			break;
		case FLEFT:
			a.setPoints(5,x-m_m_size,y+m_m_size/2, x-m_size,y+m_size,x-m_size/2,y+m_size, x,y+m_size/2,x,y-m_size/2);
			break;
		case FRIGHT:
			a.setPoints(5,x,y-m_size/2,x,y+m_size/2, x+m_size,y-m_size/2,x+m_size,y-m_size,x+m_size/2,y-m_size);
			break;
		case FURIGHT:
			a.setPoints(5,x-m_size/4,y-m_size/4,x+m_size/4,y+m_size/4, x+m_size,y-m_size/2,x+m_size,y-m_size,x+m_size/2,y-m_size);
			break;
		case FDLEFT:
			a.setPoints(5,x-m_size,y+m_size/2, x-m_size,y+m_size,x-m_size/2,y+m_size, x+m_size/4,y+m_size/4,x-m_size/4,y-m_size/4);
			break;
		case FDRIGHT:
			a.setPoints(4,x-m_size,y+m_size, x-m_size/2,y+m_size,x+m_size,y-m_size/2, x+m_size,y-m_size);
			break;
		case FULEFT:
			a.setPoints(4,x-m_size,y+m_size, x-m_size,y+m_size/2,x+m_size/2,y-m_size, x+m_size,y-m_size);
			break;
		}
		p->drawPolygon(a);

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		a.setPoints(6,x-m_size,y+m_size/2, x-m_size,y+m_size,x-m_size/2,y+m_size, x+m_size,y-m_size/2,x+m_size,y-m_size,x+m_size/2,y-m_size);
		p->drawPolygon(a);
		break;
	case SDIAG4:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(6,x-m_size,y-m_size/2, x-m_size,y-m_size,x-m_size/2,y-m_size, x+m_size,y+m_size/2,x+m_size,y+m_size,x+m_size/2,y+m_size);
			break;
		case FBOTTOM:
			a.setPoints(5,x-m_size/2,y,x+m_size/2,y+m_size,x+m_size,y+m_size,x+m_size,y+m_size/2,x+m_size/2,y);
			break;
		case FTOP:
			a.setPoints(5,x-m_size,y-m_size/2,x-m_size/2,y,x+m_size/2,y,x-m_size/2,y-m_size,x-m_size,y-m_size);
			break;
		case FLEFT:
			a.setPoints(5,x-m_size,y-m_size/2,x,y+m_size/2,x,y-m_size/2,x-m_size/2,y-m_size,x-m_size,y-m_size);
			break;
		case FRIGHT:
			a.setPoints(5,x,y+m_size/2,x+m_size/2,y+m_size,x+m_size,y+m_size,x+m_size,y+m_size/2,x,y-m_size/2);
			break;
		case FURIGHT:
			a.setPoints(4,x+m_size,y+m_size,x+m_size,y+m_size/2,x-m_size/2,y-m_size,x-m_size,y-m_size);
			break;
		case FDLEFT:
			a.setPoints(4,x-m_size,y-m_size,x-m_size,y-m_size/2,x+m_size/2,y+m_size,x+m_size,y+m_size);
			break;
		case FDRIGHT:
			a.setPoints(5,x-m_size/4,y+m_size/4,x+m_size/2,y+m_size,x+m_size,y+m_size,x+m_size,y+m_size/2,x+m_size/4,y-m_size/4);
			break;
		case FULEFT:
			a.setPoints(5,x+m_size/4,y-m_size/4,x-m_size/2,y-m_size,x-m_size,y-m_size,x-m_size,y-m_size/2,x-m_size/4,y+m_size/4);
			break;
		}
		p->drawPolygon(a);

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		a.setPoints(6,x-m_size,y-m_size/2, x-m_size,y-m_size,x-m_size/2,y-m_size, x+m_size,y+m_size/2,x+m_size,y+m_size,x+m_size/2,y+m_size);
		p->drawPolygon(a);
		break;
	case SCROSS3:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(16,x-m_size,y+m_size,x-m_size/2,y+m_size,x,y+m_size/2,x+m_size/2,y+m_size,x+m_size,y+m_size,
				x+m_size,y+m_size/2,x+m_size/2,y,x+m_size,y-m_size/2,x+m_size,y-m_size,x+m_size/2,y-m_size,
				x,y-m_size/2,x-m_size/2,y-m_size,x-m_size,y-m_size,x-m_size,y-m_size/2,x-m_size/2,y,x-m_size,y+m_size/2);
			break;
		case FBOTTOM:
			a.setPoints(9,x-m_size/2,y,x-m_size,y+m_size/2,x-m_size,y+m_size,x-m_size/2,y+m_size,x,y+m_size/2,
				x+m_size/2,y+m_size,x+m_size,y+m_size,x+m_size,y+m_size/2,x+m_size/2,y);
			break;
		case FTOP:
			a.setPoints(9,x+m_size/2,y,x+m_size,y-m_size/2,x+m_size,y-m_size,x+m_size/2,y-m_size,x,y-m_size/2,x-m_size/2,y-m_size,
				x-m_size,y-m_size,x-m_size,y-m_size/2,x-m_size/2,y);
			break;
		case FLEFT:
			a.setPoints(9,x,y-m_size/2,x-m_size/2,y-m_size,x-m_size,y-m_size,x-m_size,y-m_size/2,x-m_size/2,y,x-m_size,y+m_size/2,
				x-m_size,y+m_size,x-m_size/2,y+m_size,x,y+m_size/2);
			break;
		case FRIGHT:
			a.setPoints(9,x,y+m_size/2,x+m_size/2,y+m_size,x+m_size,y+m_size,x+m_size,y+m_size/2,x+m_size/2,y,
				x+m_size,y-m_size/2,x+m_size,y-m_size,x+m_size/2,y-m_size,x,y-m_size/2);
			break;
		case FURIGHT:
			a.setPoints(9,x+m_size,y+m_size,x+m_size,y+m_size/2,x+m_size/2,y,x+m_size,y-m_size/2,x+m_size,y-m_size,
				x+m_size/2,y-m_size,x,y-m_size/2,x-m_size/2,y-m_size,x-m_size,y-m_size);
			break;
		case FDLEFT:
			a.setPoints(9,x-m_size,y-m_size,x-m_size,y-m_size/2,x-m_size/2,y,x-m_size,y+m_size/2,x-m_size,y+m_size,x-m_size/2,y+m_size,
				x,y+m_size/2,x+m_size/2,y+m_size,x+m_size,y+m_size);
			break;
		case FDRIGHT:
			a.setPoints(9,x-m_size,y+m_size,x-m_size/2,y+m_size,x,y+m_size/2,x+m_size/2,y+m_size,x+m_size,y+m_size,
				x+m_size,y+m_size/2,x+m_size/2,y,x+m_size,y-m_size/2,x+m_size,y-m_size);
			break;
		case FULEFT:
			a.setPoints(9,x+m_size,y-m_size,x+m_size/2,y-m_size,x,y-m_size/2,x-m_size/2,y-m_size,x-m_size,y-m_size,x-m_size,y-m_size/2,
				x-m_size/2,y,x-m_size,y+m_size/2,x-m_size,y+m_size);
			break;
		}
		p->drawPolygon(a);

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		a.setPoints(16,x-m_size,y+m_size,x-m_size/2,y+m_size,x,y+m_size/2,x+m_size/2,y+m_size,x+m_size,y+m_size,
			x+m_size,y+m_size/2,x+m_size/2,y,x+m_size,y-m_size/2,x+m_size,y-m_size,x+m_size/2,y-m_size,x,y-m_size/2,
			x-m_size/2,y-m_size,x-m_size,y-m_size,x-m_size,y-m_size/2,x-m_size/2,y,x-m_size,y+m_size/2);
		p->drawPolygon(a);
		break;
	case SPARRIGHT:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(4,x-m_size,y+m_size,x,y-m_size,x+m_size,y-m_size,x,y+m_size);
			break;
		case FBOTTOM:
			a.setPoints(4,x-m_size,y+m_size,x-m_size/2,y,x+m_size/2,y,x,y+m_size);
			break;
		case FTOP:
			a.setPoints(4,x-m_size/2,y,x,y-m_size,x+m_size,y-m_size,x+m_size/2,y);
			break;
		case FRIGHT:
			a.setPoints(3,x,y-m_size,x+m_size,y-m_size,x,y+m_size);
			break;
		case FLEFT:
			a.setPoints(3,x-m_size,y+m_size,x,y-m_size,x,y+m_size);
			break;
		case FURIGHT:
			a.setPoints(3,x,y-m_size,x+m_size,y-m_size,x+m_size/2,y);
			break;
		case FDRIGHT:
			a.setPoints(4,x-m_size/2,y+m_size,x+m_size/2,y-m_size,x+m_size,y-m_size,x,y+m_size);
			break;
		case FULEFT:
			a.setPoints(4,x-m_size,y+m_size,x,y-m_size,x+m_size/2,y-m_size,x-m_size/2,y+m_size);
			break;
		case FDLEFT:
			a.setPoints(3,x-m_size,y+m_size,x-m_size/2,y,x,y+m_size);
			break;
		}
		p->drawPolygon(a);

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		a.setPoints(4,x-m_size,y+m_size,x,y-m_size,x+m_size,y-m_size,x,y+m_size);
		p->drawPolygon(a);
		break;
	case SPARLEFT:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(4,x,y+m_size,x-m_size,y-m_size,x,y-m_size,x+m_size,y+m_size);
			break;
		case FBOTTOM:
			a.setPoints(4,x,y+m_size,x-m_size/2,y,x+m_size/2,y,x+m_size,y+m_size);
			break;
		case FTOP:
			a.setPoints(4,x-m_size/2,y,x-m_size,y-m_size,x,y-m_size,x+m_size/2,y);
			break;
		case FRIGHT:
			a.setPoints(3,x,y+m_size,x,y-m_size,x+m_size,y+m_size);
			break;
		case FLEFT:
			a.setPoints(3,x,y-m_size,x-m_size,y-m_size,x,y+m_size);
			break;
		case FURIGHT:
			a.setPoints(4,x+m_size/2,y+m_size,x-m_size/2,y-m_size,x,y-m_size,x+m_size,y+m_size);
			break;
		case FDRIGHT:
			a.setPoints(3,x,y+m_size,x+m_size/2,y,x+m_size,y+m_size);
			break;
		case FULEFT:
			a.setPoints(3,x-m_size/2,y,x-m_size,y-m_size,x,y-m_size);
			break;
		case FDLEFT:
			a.setPoints(4,x,y+m_size,x-m_size,y-m_size,x-m_size/2,y-m_size,x+m_size/2,y+m_size);
			break;
		}
		p->drawPolygon(a);

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		a.setPoints(4,x,y+m_size,x-m_size,y-m_size,x,y-m_size,x+m_size,y+m_size);
		p->drawPolygon(a);
		break;
	case SHLEFTCIRCLE:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			p->drawPie(x-m_size,y-m_size,2*m_size,2*m_size,8*180,16*180);
			break;
		case FBOTTOM:
			p->drawPie(x-m_size,y-m_size,2*m_size,2*m_size,16*180,8*180);
			break;
		case FTOP:
			p->drawPie(x-m_size,y-m_size,2*m_size,2*m_size,8*180,8*180);
			break;
		case FRIGHT:
			break;
		case FLEFT:
			p->drawPie(x-m_size,y-m_size,2*m_size,2*m_size,8*180,16*180);
			break;
		case FURIGHT:
			break;
		case FDRIGHT:
			p->drawPie(x-m_size,y-m_size,2*m_size,2*m_size,16*180,8*180);
			break;
		case FULEFT:
			p->drawPie(x-m_size,y-m_size,2*m_size,2*m_size,8*180,8*180);
			break;
		case FDLEFT:
			break;
		}

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		p->drawPie(x-m_size,y-m_size,2*m_size,2*m_size,8*180,16*180);
		break;
	case SHRIGHTCIRCLE:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			p->drawPie(x-m_size,y-m_size,2*m_size,2*m_size,24*180,16*180);
			break;
		case FBOTTOM:
			p->drawPie(x-m_size,y-m_size,2*m_size,2*m_size,24*180,8*180);
			break;
		case FTOP:
			p->drawPie(x-m_size,y-m_size,2*m_size,2*m_size,32*180,8*180);
			break;
		case FRIGHT:
			p->drawPie(x-m_size,y-m_size,2*m_size,2*m_size,24*180,16*180);
			break;
		case FLEFT:
			break;
		case FURIGHT:
			p->drawPie(x-m_size,y-m_size,2*m_size,2*m_size,32*180,8*180);
			break;
		case FDRIGHT:
			p->drawPie(x-m_size,y-m_size,2*m_size,2*m_size,24*180,8*180);
			break;
		case FULEFT:
			break;
		case FDLEFT:
			break;
		}

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		p->drawPie(x-m_size,y-m_size,2*m_size,2*m_size,24*180,16*180);
		break;
	case SSMALLDIAMOND:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(4,x,y-m_size,x-m_size/2,y,x,y+m_size,x+m_size/2,y);
			break;
		case FBOTTOM:
			a.setPoints(3,x-m_size/2,y,x,y+m_size,x+m_size/2,y);
			break;
		case FTOP:
			a.setPoints(3,x,y-m_size,x-m_size/2,y,x+m_size/2,y);
			break;
		case FRIGHT:
			a.setPoints(3,x,y-m_size,x,y+m_size,x+m_size/2,y);
			break;
		case FLEFT:
			a.setPoints(3,x,y-m_size,x-m_size/2,y,x,y+m_size);
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
		a.setPoints(4,x,y-m_size,x-m_size/2,y,x,y+m_size,x+m_size/2,y);
		p->drawPolygon(a);
		break;
	case SROTDIAMOND:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(4,x-m_size,y,x,y+m_size/2,x+m_size,y,x,y-m_size/2);
			break;
		case FBOTTOM:
			a.setPoints(3,x-m_size,y,x,y+m_size/2,x+m_size,y);
			break;
		case FTOP:
			a.setPoints(3,x-m_size,y,x+m_size,y,x,y-m_size/2);
			break;
		case FRIGHT:
			a.setPoints(3,x,y+m_size/2,x+m_size,y,x,y-m_size/2);
			break;
		case FLEFT:
			a.setPoints(3,x-m_size,y,x,y+m_size/2,x,y-m_size/2);
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
		a.setPoints(4,x-m_size,y,x,y+m_size/2,x+m_size,y,x,y-m_size/2);
		p->drawPolygon(a);
		break;
	case SPENTA:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(5,x,y-m_size,x-m_size,y,x-m_size/2,y+m_size,x+m_size/2,y+m_size,x+m_size,y);
			break;
		case FBOTTOM:
			a.setPoints(4,x-m_size,y,x-m_size/2,y+m_size,x+m_size/2,y+m_size,x+m_size,y);
			break;
		case FTOP:
			a.setPoints(3,x,y-m_size,x-m_size,y,x+m_size,y);
			break;
		case FRIGHT:
			a.setPoints(4,x,y-m_size,x,y+m_size,x+m_size/2,y+m_size,x+m_size,y);
			break;
		case FLEFT:
			a.setPoints(4,x,y-m_size,x-m_size,y,x-m_size/2,y+m_size,x,y+m_size);
			break;
		case FURIGHT:
			a.setPoints(4,x,y-m_size,x-m_size,y,x+m_size/2,y+m_size,x+m_size,y);
			break;
		case FDRIGHT:
			a.setPoints(3,x-m_size/2,y+m_size,x+m_size/2,y+m_size,x+m_size,y);
			break;
		case FULEFT:
			a.setPoints(4,x,y-m_size,x-m_size,y,x-m_size/2,y+m_size,x+m_size,y);
			break;
		case FDLEFT:
			a.setPoints(3,x-m_size,y,x-m_size/2,y+m_size,x+m_size/2,y+m_size);
			break;
		}
		p->drawPolygon(a);

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		a.setPoints(5,x,y-m_size,x-m_size,y,x-m_size/2,y+m_size,x+m_size/2,y+m_size,x+m_size,y);
		p->drawPolygon(a);
		break;
	case SPENTALEFT:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(5,x-m_size,y,x,y+m_size,x+m_size,y+m_size/2,x+m_size,y-m_size/2,x,y-m_size);
			break;
		case FBOTTOM:
			a.setPoints(4,x-m_size,y,x,y+m_size,x+m_size,y+m_size/2,x+m_size,y);
			break;
		case FTOP:
			a.setPoints(4,x-m_size,y,x+m_size,y,x+m_size,y-m_size/2,x,y-m_size);
			break;
		case FRIGHT:
			a.setPoints(4,x,y+m_size,x+m_size,y+m_size/2,x+m_size,y-m_size/2,x,y-m_size);
			break;
		case FLEFT:
			a.setPoints(3,x-m_size,y,x,y+m_size,x,y-m_size);
			break;
		case FURIGHT:
			a.setPoints(3,x+m_size,y+m_size/2,x+m_size,y-m_size/2,x,y-m_size);
			break;
		case FDRIGHT:
			a.setPoints(3,x,y+m_size,x+m_size,y+m_size/2,x+m_size,y-m_size/2);
			break;
		case FULEFT:
			a.setPoints(3,x-m_size,y,x+m_size,y-m_size/2,x,y-m_size);
			break;
		case FDLEFT:
			a.setPoints(3,x-m_size,y,x,y+m_size,x+m_size,y+m_size/2);
			break;
		}
		p->drawPolygon(a);

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		a.setPoints(5,x-m_size,y,x,y+m_size,x+m_size,y+m_size/2,x+m_size,y-m_size/2,x,y-m_size);
		p->drawPolygon(a);
		break;
	case SPENTABOTTOM:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(5,x,y+m_size,x+m_size,y,x+m_size/2,y-m_size,x-m_size/2,y-m_size,x-m_size,y);
			break;
		case FBOTTOM:
			a.setPoints(3,x,y+m_size,x+m_size,y,x-m_size,y);
			break;
		case FTOP:
			a.setPoints(4,x+m_size,y,x+m_size/2,y-m_size,x-m_size/2,y-m_size,x-m_size,y);
			break;
		case FRIGHT:
			a.setPoints(4,x,y+m_size,x+m_size,y,x+m_size/2,y-m_size,x,y-m_size);
			break;
		case FLEFT:
			a.setPoints(4,x,y+m_size,x,y-m_size,x-m_size/2,y-m_size,x-m_size,y);
			break;
		case FURIGHT:
			a.setPoints(3,x+m_size,y,x+m_size/2,y-m_size,x-m_size/2,y-m_size);
			break;
		case FDRIGHT:
			a.setPoints(3,x,y+m_size,x+m_size,y,x+m_size/2,y-m_size);
			break;
		case FULEFT:
			a.setPoints(3,x+m_size/2,y-m_size,x-m_size/2,y-m_size,x-m_size,y);
			break;
		case FDLEFT:
			a.setPoints(3,x,y+m_size,x-m_size/2,y-m_size,x-m_size,y);
			break;
		}
		p->drawPolygon(a);

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		a.setPoints(5,x,y+m_size,x+m_size,y,x+m_size/2,y-m_size,x-m_size/2,y-m_size,x-m_size,y);
		p->drawPolygon(a);
		break;
	case SPENTARIGHT:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(5,x+m_size,y,x,y-m_size,x-m_size,y-m_size/2,x-m_size,y+m_size/2,x,y+m_size);
			break;
		case FBOTTOM:
			a.setPoints(4,x+m_size,y,x-m_size,y,x-m_size,y+m_size/2,x,y+m_size);
			break;
		case FTOP:
			a.setPoints(4,x+m_size,y,x,y-m_size,x-m_size,y-m_size/2,x-m_size,y);
			break;
		case FRIGHT:
			a.setPoints(3,x+m_size,y,x,y-m_size,x,y+m_size);
			break;
		case FLEFT:
			a.setPoints(4,x,y-m_size,x-m_size,y-m_size/2,x-m_size,y+m_size/2,x,y+m_size);
			break;
		case FURIGHT:
			a.setPoints(3,x+m_size,y,x,y-m_size,x-m_size,y-m_size/2);
			break;
		case FDRIGHT:
			a.setPoints(3,x+m_size,y,x-m_size,y+m_size/2,x,y+m_size);
			break;
		case FULEFT:
			a.setPoints(3,x,y-m_size,x-m_size,y-m_size/2,x-m_size,y+m_size/2);
			break;
		case FDLEFT:
			a.setPoints(3,x-m_size,y-m_size/2,x-m_size,y+m_size/2,x,y+m_size);
			break;
		}
		p->drawPolygon(a);

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		a.setPoints(5,x+m_size,y,x,y-m_size,x-m_size,y-m_size/2,x-m_size,y+m_size/2,x,y+m_size);
		p->drawPolygon(a);
		break;
	case SHExAGON:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(6,x-m_size,y,x-m_size/2,y-m_size,x+m_size/2,y-m_size,x+m_size,y,x+m_size/2,y+m_size,x-m_size/2,y+m_size);
			break;
		case FBOTTOM:
			a.setPoints(4,x-m_size,y,x+m_size,y,x+m_size/2,y+m_size,x-m_size/2,y+m_size);
			break;
		case FTOP:
			a.setPoints(4,x-m_size,y,x-m_size/2,y-m_size,x+m_size/2,y-m_size,x+m_size,y);
			break;
		case FRIGHT:
			a.setPoints(5,x,y-m_size,x+m_size/2,y-m_size,x+m_size,y,x+m_size/2,y+m_size,x,y+m_size);
			break;
		case FLEFT:
			a.setPoints(5,x-m_size,y,x-m_size/2,y-m_size,x,y-m_size,x,y+m_size,x-m_size/2,y+m_size);
			break;
		case FURIGHT:
			a.setPoints(4,x-m_size/2,y-m_size,x+m_size/2,y-m_size,x+m_size,y,x+m_size/2,y+m_size);
			break;
		case FDRIGHT:
			a.setPoints(4,x+m_size/2,y-m_size,x+m_size,y,x+m_size/2,y+m_size,x-m_size/2,y+m_size);
			break;
		case FULEFT:
			a.setPoints(4,x-m_size,y,x-m_size/2,y-m_size,x+m_size/2,y-m_size,x-m_size/2,y+m_size);
			break;
		case FDLEFT:
			a.setPoints(4,x-m_size,y,x-m_size/2,y-m_size,x+m_size/2,y+m_size,x-m_size/2,y+m_size);
			break;
		}
		p->drawPolygon(a);

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		a.setPoints(6,x-m_size,y,x-m_size/2,y-m_size,x+m_size/2,y-m_size,x+m_size,y,x+m_size/2,y+m_size,x-m_size/2,y+m_size);
		p->drawPolygon(a);
		break;
	case SVHExAGON:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(6,x-m_size,y-m_size/2,x,y-m_size,x+m_size,y-m_size/2,x+m_size,y+m_size/2,x,y+m_size,x-m_size,y+m_size/2);
			break;
		case FBOTTOM:
			a.setPoints(5,x-m_size,y,x+m_size,y,x+m_size,y+m_size/2,x,y+m_size,x-m_size,y+m_size/2);
			break;
		case FTOP:
			a.setPoints(5,x-m_size,y-m_size/2,x,y-m_size,x+m_size,y-m_size/2,x+m_size,y,x-m_size,y);
			break;
		case FRIGHT:
			a.setPoints(4,x,y-m_size,x+m_size,y-m_size/2,x+m_size,y+m_size/2,x,y+m_size);
			break;
		case FLEFT:
			a.setPoints(4,x-m_size,y-m_size/2,x,y-m_size,x,y+m_size,x-m_size,y+m_size/2);
			break;
		case FURIGHT:
			a.setPoints(4,x-m_size,y-m_size/2,x,y-m_size,x+m_size,y-m_size/2,x+m_size,y+m_size/2);
			break;
		case FDRIGHT:
			a.setPoints(4,x+m_size,y-m_size/2,x+m_size,y+m_size/2,x,y+m_size,x-m_size,y+m_size/2);
			break;
		case FULEFT:
			a.setPoints(4,x-m_size,y-m_size/2,x,y-m_size,x+m_size,y-m_size/2,x-m_size,y+m_size/2);
			break;
		case FDLEFT:
			a.setPoints(4,x-m_size,y-m_size/2,x+m_size,y+m_size/2,x,y+m_size,x-m_size,y+m_size/2);
			break;
		}
		p->drawPolygon(a);

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		a.setPoints(6,x-m_size,y-m_size/2,x,y-m_size,x+m_size,y-m_size/2,x+m_size,y+m_size/2,x,y+m_size,x-m_size,y+m_size/2);
		p->drawPolygon(a);
		break;
	case SSTAR3:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(8,x,y-m_size,x+m_size/4,y-m_size/4,x+m_size,y,x+m_size/4,y+m_size/4,
				x,y+m_size,x-m_size/4,y+m_size/4,x-m_size,y,x-m_size/4,y-m_size/4);
			break;
		case FBOTTOM:
			a.setPoints(5,x+m_size,y,x+m_size/4,y+m_size/4,x,y+m_size,x-m_size/4,y+m_size/4,x-m_size,y);
			break;
		case FTOP:
			a.setPoints(5,x,y-m_size,x+m_size/4,y-m_size/4,x+m_size,y,x-m_size,y,x-m_size/4,y-m_size/4);
			break;
		case FRIGHT:
			a.setPoints(5,x,y-m_size,x+m_size/4,y-m_size/4,x+m_size,y,x+m_size/4,y+m_size/4,x,y+m_size);
			break;
		case FLEFT:
			a.setPoints(5,x,y-m_size,x,y+m_size,x-m_size/4,y+m_size/4,x-m_size,y,x-m_size/4,y-m_size/4);
			break;
		case FURIGHT:
			a.setPoints(5,x,y-m_size,x+m_size/4,y-m_size/4,x+m_size,y,x+m_size/4,y+m_size/4,x-m_size/4,y-m_size/4);
			break;
		case FDRIGHT:
			a.setPoints(5,x+m_size/4,y-m_size/4,x+m_size,y,x+m_size/4,y+m_size/4,x,y+m_size,x-m_size/4,y+m_size/4);
			break;
		case FULEFT:
			a.setPoints(5,x,y-m_size,x+m_size/4,y-m_size/4,x-m_size/4,y+m_size/4,x-m_size,y,x-m_size/4,y-m_size/4);
			break;
		case FDLEFT:
			a.setPoints(5,x+m_size/4,y+m_size/4,x,y+m_size,x-m_size/4,y+m_size/4,x-m_size,y,x-m_size/4,y-m_size/4);
			break;
		}
		p->drawPolygon(a);

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		a.setPoints(8,x,y-m_size,x+m_size/4,y-m_size/4,x+m_size,y,x+m_size/4,y+m_size/4,
			x,y+m_size,x-m_size/4,y+m_size/4,x-m_size,y,x-m_size/4,y-m_size/4);
		p->drawPolygon(a);
		break;
	case SUARROW:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(4,x,y-m_size,x+m_size,y+m_size,x,y,x-m_size,y+m_size);
			break;
		case FBOTTOM:
			a.setPoints(5,x-m_size/2,y,x+m_size/2,y,x+m_size,y+m_size,x,y,x-m_size,y+m_size);
			break;
		case FTOP:
			a.setPoints(3,x,y-m_size,x+m_size/2,y,x-m_size/2,y);
			break;
		case FRIGHT:
			a.setPoints(3,x,y-m_size,x+m_size,y+m_size,x,y);
			break;
		case FLEFT:
			a.setPoints(3,x,y-m_size,x,y,x-m_size,y+m_size);
			break;
		case FURIGHT:
			a.setPoints(4,x,y-m_size,x+m_size,y+m_size,x,y,x-m_size,y+m_size);
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
		a.setPoints(4,x,y-m_size,x+m_size,y+m_size,x,y,x-m_size,y+m_size);
		p->drawPolygon(a);
		break;
	case SLARROW:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(4,x-m_size,y,x+m_size,y-m_size,x,y,x+m_size,y+m_size);
			break;
		case FBOTTOM:
			a.setPoints(3,x-m_size,y,x,y,x+m_size,y+m_size);
			break;
		case FTOP:
			a.setPoints(3,x-m_size,y,x+m_size,y-m_size,x,y);
			break;
		case FRIGHT:
			a.setPoints(5,x,y+m_size/2,x,y-m_size/2,x+m_size,y-m_size,x,y,x+m_size,y+m_size);
			break;
		case FLEFT:
			a.setPoints(4,x-m_size,y,x,y-m_size/2,x,y,x,y+m_size/2);
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
		a.setPoints(4,x-m_size,y,x+m_size,y-m_size,x,y,x+m_size,y+m_size);
		p->drawPolygon(a);
		break;
	case SDARROW:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(4,x,y+m_size,x-m_size,y-m_size,x,y,x+m_size,y-m_size);
			break;
		case FBOTTOM:
			a.setPoints(3,x,y+m_size,x-m_size/2,y,x+m_size/2,y);
			break;
		case FTOP:
			a.setPoints(5,x+m_size/2,y,x-m_size/2,y,x-m_size,y-m_size,x,y,x+m_size,y-m_size);
			break;
		case FRIGHT:
			a.setPoints(3,x,y+m_size,x,y,x+m_size,y-m_size);
			break;
		case FLEFT:
			a.setPoints(3,x,y+m_size,x-m_size,y-m_size,x,y);
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
		a.setPoints(4,x,y+m_size,x-m_size,y-m_size,x,y,x+m_size,y-m_size);
		p->drawPolygon(a);
		break;
	case SRARROW:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(4,x+m_size,y,x-m_size,y+m_size,x,y,x-m_size,y-m_size);
			break;
		case FBOTTOM:
			a.setPoints(3,x+m_size,y,x-m_size,y+m_size,x,y);
			break;
		case FTOP:
			a.setPoints(3,x+m_size,y,x,y,x-m_size,y-m_size);
			break;
		case FRIGHT:
			a.setPoints(3,x+m_size,y,x,y+m_size/2,x,y-m_size/2);
			break;
		case FLEFT:
			a.setPoints(5,x,y-m_size/2,x,y+m_size/2,x-m_size,y+m_size,x,y,x-m_size,y-m_size);
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
		a.setPoints(4,x+m_size,y,x-m_size,y+m_size,x,y,x-m_size,y-m_size);
		p->drawPolygon(a);
		break;
	case SUHOUSE:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(5,x,y-m_size,x+m_size,y,x+m_size,y+m_size,x-m_size,y+m_size,x-m_size,y);
			break;
		case FBOTTOM:
			a.setPoints(4,x+m_size,y,x+m_size,y+m_size,x-m_size,y+m_size,x-m_size,y);
			break;
		case FTOP:
			a.setPoints(3,x,y-m_size,x+m_size,y,x-m_size,y);
			break;
		case FRIGHT:
			a.setPoints(4,x,y-m_size,x+m_size,y,x+m_size,y+m_size,x,y+m_size);
			break;
		case FLEFT:
			a.setPoints(4,x,y-m_size,x,y+m_size,x-m_size,y+m_size,x-m_size,y);
			break;
		case FURIGHT:
			a.setPoints(4,x,y-m_size,x+m_size,y,x+m_size,y+m_size,x-m_size,y);
			break;
		case FDRIGHT:
			a.setPoints(3,x+m_size,y,x+m_size,y+m_size,x-m_size,y+m_size);
			break;
		case FULEFT:
			a.setPoints(4,x,y-m_size,x+m_size,y,x-m_size,y+m_size,x-m_size,y);
			break;
		case FDLEFT:
			a.setPoints(3,x+m_size,y+m_size,x-m_size,y+m_size,x-m_size,y);
			break;
		}
		p->drawPolygon(a);

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		a.setPoints(5,x,y-m_size,x+m_size,y,x+m_size,y+m_size,x-m_size,y+m_size,x-m_size,y);
		p->drawPolygon(a);
		break;
	case SLHOUSE:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(5,x-m_size,y,x,y-m_size,x+m_size,y-m_size,x+m_size,y+m_size,x,y+m_size);
			break;
		case FBOTTOM:
			a.setPoints(4,x-m_size,y,x+m_size,y,x+m_size,y+m_size,x,y+m_size);
			break;
		case FTOP:
			a.setPoints(4,x-m_size,y,x,y-m_size,x+m_size,y-m_size,x+m_size,y);
			break;
		case FRIGHT:
			a.setPoints(4,x,y-m_size,x+m_size,y-m_size,x+m_size,y+m_size,x,y+m_size);
			break;
		case FLEFT:
			a.setPoints(3,x-m_size,y,x,y-m_size,x,y+m_size);
			break;
		case FURIGHT:
			a.setPoints(3,x,y-m_size,x+m_size,y-m_size,x+m_size,y+m_size);
			break;
		case FDRIGHT:
			a.setPoints(3,x+m_size,y-m_size,x+m_size,y+m_size,x,y+m_size);
			break;
		case FULEFT:
			a.setPoints(4,x-m_size,y,x,y-m_size,x+m_size,y-m_size,x,y+m_size);
			break;
		case FDLEFT:
			a.setPoints(4,x-m_size,y,x,y-m_size,x+m_size,y+m_size,x,y+m_size);
			break;
		}
		p->drawPolygon(a);

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		a.setPoints(5,x-m_size,y,x,y-m_size,x+m_size,y-m_size,x+m_size,y+m_size,x,y+m_size);
		p->drawPolygon(a);
		break;
	case SDHOUSE:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(5,x,y+m_size,x-m_size,y,x-m_size,y-m_size,x+m_size,y-m_size,x+m_size,y);
			break;
		case FBOTTOM:
			a.setPoints(3,x,y+m_size,x-m_size,y,x+m_size,y);
			break;
		case FTOP:
			a.setPoints(4,x-m_size,y,x-m_size,y-m_size,x+m_size,y-m_size,x+m_size,y);
			break;
		case FRIGHT:
			a.setPoints(4,x,y+m_size,x,y-m_size,x+m_size,y-m_size,x+m_size,y);
			break;
		case FLEFT:
			a.setPoints(4,x,y+m_size,x-m_size,y,x-m_size,y-m_size,x,y-m_size);
			break;
		case FURIGHT:
			a.setPoints(3,x-m_size,y-m_size,x+m_size,y-m_size,x+m_size,y);
			break;
		case FDRIGHT:
			a.setPoints(4,x,y+m_size,x-m_size,y,x+m_size,y-m_size,x+m_size,y);
			break;
		case FULEFT:
			a.setPoints(3,x-m_size,y,x-m_size,y-m_size,x+m_size,y-m_size);
			break;
		case FDLEFT:
			a.setPoints(4,x,y+m_size,x-m_size,y,x-m_size,y-m_size,x+m_size,y);
			break;
		}
		p->drawPolygon(a);

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		a.setPoints(5,x,y+m_size,x-m_size,y,x-m_size,y-m_size,x+m_size,y-m_size,x+m_size,y);
		p->drawPolygon(a);
		break;
	case SRHOUSE:
		switch (fill) {
		case FNONE:
			break;
		case FFULL:
			a.setPoints(5,x+m_size,y,x,y+m_size,x-m_size,y+m_size,x-m_size,y-m_size,x,y-m_size);
			break;
		case FBOTTOM:
			a.setPoints(4,x+m_size,y,x,y+m_size,x-m_size,y+m_size,x-m_size,y);
			break;
		case FTOP:
			a.setPoints(4,x+m_size,y,x-m_size,y,x-m_size,y-m_size,x,y-m_size);
			break;
		case FRIGHT:
			a.setPoints(3,x+m_size,y,x,y+m_size,x,y-m_size);
			break;
		case FLEFT:
			a.setPoints(4,x,y+m_size,x-m_size,y+m_size,x-m_size,y-m_size,x,y-m_size);
			break;
		case FURIGHT:
			a.setPoints(4,x+m_size,y,x,y+m_size,x-m_size,y-m_size,x,y-m_size);
			break;
		case FDRIGHT:
			a.setPoints(4,x+m_size,y,x,y+m_size,x-m_size,y+m_size,x,y-m_size);
			break;
		case FULEFT:
			a.setPoints(3,x-m_size,y+m_size,x-m_size,y-m_size,x,y-m_size);
			break;
		case FDLEFT:
			a.setPoints(3,x,y+m_size,x-m_size,y+m_size,x-m_size,y-m_size);
			break;
		}
		p->drawPolygon(a);

		p->setPen(color);
		p->setBrush(Qt::NoBrush);
		a.setPoints(5,x+m_size,y,x,y+m_size,x-m_size,y+m_size,x-m_size,y-m_size,x,y-m_size);
		p->drawPolygon(a);
		break;
*/
	}

	p->setPen(QColor("black"));
}
