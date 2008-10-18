/***************************************************************************
    File                 : Legend.cc
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Stefan Gerlach, Alexander Semke
    Email (use @ for *)  : stefan.gerlach*uni-konstanz.de, alexander.semke*web.de
    Description          : legend class

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
#include "Legend.h"

#include <cmath>
#include <iostream>

#include <KDebug>

Legend::Legend(){
	//General
	m_enabled = true;
	m_position.setX( 0.7 );
	m_position.setY( 0.05 );
	m_orientation=Qt::Vertical;
	m_lineLength=50;

	//Background
	m_fillingEnabled=false;
	m_fillingColor = QColor(Qt::white);
	m_boxEnabled = true;
	m_shadowEnabled = true;

	//Text font (default system font is used)
	m_textFont.setPointSize(8);
	m_textColor = QColor(Qt::black);

	//Layout
	m_layoutLeftMargin=4;
	m_layoutTopMargin=4;
	m_layoutRightMargin=4;
	m_layoutBottomMargin=4;
	m_layoutHorizontalSpacing=4;
	m_layoutVerticalSpacing=4;
}

/*!
	draws the legend for the data sets in \c list_Sets.
*/
 void Legend::draw( QPainter* p, const QList<Set>* list_Sets, const Point pos, const Point size, const int w, const int h){
	QFont tmpFont=p->font();
	p->setFont(m_textFont);

	int x1 = (int) ((m_position.x()*size.x()+pos.x())*w);
	int y1 = (int) ((m_position.y()*size.y()+pos.y())*h);
	int maxLabelWidth=0;
	int totalLabelWidth=0;
	int labelHeight;//=m_textFont.pointSize();
	int setsNumber=0;

// 	// set point size
// 	int pointsize = m_textFont.pointSize();
// 	QFont tmpfont = m_textFont;
// 	tmpfont.setPointSize((int)(pointsize*size.x()));
// 	p->setFont(tmpfont);
// 	QFontMetrics fm = p->fontMetrics();

	//determine the number of all visible/shown sets
	// and the maximal length of the set's labels.
	const Set* set;
	const Label* label;
	QTextDocument textDocument;
	textDocument.setDefaultFont( m_textFont );//TODO doesn't work!!!
	for (int i=0; i<list_Sets->size(); i++){
		set = &list_Sets->at(i);
		if (set->isShown()==false)
			continue;

		setsNumber++;
		label=const_cast<Set*>(set)->label();
		if ( label->isTex() ){

		}else{
			textDocument.setHtml( label->text() );
			totalLabelWidth += textDocument.size().width();
			if ( textDocument.size().width()>maxLabelWidth )
				maxLabelWidth=textDocument.size().width();
		}
	}
	labelHeight=textDocument.size().height();

	int boxWidth;
	int boxHeight;

	// 		TODO
// 	if (type == PSURFACE) {


	//TODO consider also the case wenn the thickness of the line is greater as the fond size
	if(m_orientation==Qt::Vertical){
		boxWidth = m_layoutLeftMargin+m_lineLength+m_layoutHorizontalSpacing+maxLabelWidth+m_layoutRightMargin;
 		boxHeight=m_layoutTopMargin+setsNumber*labelHeight
						+(setsNumber-1)*m_layoutVerticalSpacing+m_layoutBottomMargin;
 	} else {
	 	boxWidth = m_layoutLeftMargin+setsNumber*(m_lineLength+m_layoutHorizontalSpacing)+totalLabelWidth
				+(setsNumber-1)*3*m_layoutHorizontalSpacing+m_layoutRightMargin;
 		boxHeight=m_layoutTopMargin+labelHeight+m_layoutBottomMargin;
 	}

	QRect box(x1, y1, boxWidth, boxHeight);

	//show shadow, if enabled
	//TODO make the size of the shadow depend on w and h.
	if (m_shadowEnabled){
		QRectF rightShadow(box.right()+1, box.top() + 5, 5, box.height());
		QRectF bottomShadow(box.left() + 5, box.bottom()+1, box.width(), 5);
		p->fillRect(rightShadow, Qt::darkGray);
		p->fillRect(bottomShadow, Qt::darkGray);
	}

	//show bounding box, if enabled
	if (m_boxEnabled) {
		p->setBrush( Qt::NoBrush ); // do not fill
 		p->setPen( Qt::black );
		p->drawRect(box);
	}

	//fill the legend box, if enabled.
	if( m_fillingEnabled )
		p->fillRect( box, QBrush(m_fillingColor) );


	//draw the lines and the labels of the shown sets
	Style* style;
	Symbol* symbol;
	int currentLineX=0;
	int currentLineY=0;
	int currentLabelX=0;
	int currentLabelY=0;
	int offsetX=x1+m_layoutLeftMargin;
	int offsetY=y1+m_layoutTopMargin;
	for (int i = 0 ; i <list_Sets->size(); i++){
		set = &list_Sets->at(i);
		if (set->isShown()==false)
			continue;

		textDocument.setHtml( const_cast<Set*>(set)->label()->text() );
		style = const_cast<Set*>(set)->style();
		QPen pen( style->lineColor(), style->lineWidth(), style->lineStyle() );
		p->setPen(pen);

		//determine the coordinates for drawing the line and the label
		currentLineX = offsetX;
		currentLabelX = offsetX+m_lineLength+m_layoutHorizontalSpacing;
		currentLineY=offsetY+labelHeight/2;
		currentLabelY=offsetY;

		if (m_orientation==Qt::Vertical){
			offsetY += labelHeight+m_layoutVerticalSpacing;
		}else{
			offsetX += m_lineLength+m_layoutHorizontalSpacing+textDocument.size().width()+3*m_layoutHorizontalSpacing;
		}

		//draw the line
		p->drawLine(currentLineX, currentLineY, currentLineX+m_lineLength, currentLineY);

		//draw the symbol, if enabled,  in the middle of the line
		if (style->isSymbolEnabled() ){
			symbol=const_cast<Style*>(style)->symbol();
			symbol->draw( p, QPoint(currentLineX+m_lineLength/2, currentLineY) );
		}

		//draw the label of the current set
		p->save();
		p->translate(currentLabelX, currentLabelY);
		textDocument.drawContents(p);
		p->restore();
	}

	//TODO
	// reset font point size
// 	tmpfont.setPointSize(pointsize);
	p->setFont(tmpFont);

// 	p->restore();
	kDebug()<<"Legend drawn"<<endl;
}



//TODO old code -> port/remove

// void Legend::drawSetLegends(QPainter *p, const QList<Set>*list_Sets, const Point& size, const QFont& tmpfont ) {
/*
	kdDebug()<<"Legend::drawGraphs()"<<endl;
	QFontMetrics fm = p->fontMetrics();

	Set* set;
	for (unsigned int i = 0 ; i < gl->Number(); i++) {
		set = list_Sets.at(i);
//		kdDebug()<<"GRAPH "<<i<<" Label : "<<g->getLabel()->simpleTitle()<<endl;
		if( !set->isShown() )
			continue;

		Label *label = set->getLabel();
// 		QString title = label->Title();
		Style *style = set->getStyle();
		QPen pen( style->Color(), style->Width(), style->PenStyle() );
		p->setPen(pen);	// EPS BUG

		if (type == PSURFACE) {
			if(label->isTeXLabel()) {
				QImage *image = new QImage(filename);
					if(!image->isNull()) {
						namelength < image->width() ? namelength = image->width() :0;

						//				kdDebug()<<"\n	drawing TeX image file "<<filename<<endl;
						p->save();

						if(orientation)
							p->translate((int)(x1+70*size.X()),(int)(y1+size.Y()*(20*number+5)));
						else
							p->translate((int)(x1+10*size.X()),(int)(y1+size.Y()*(20*number+5)));
						// phi : normal angle, rotation : additional rotation
						p->rotate(label->Rotation());
						if (label->Boxed()) {
							p->setPen(QColor("black"));
							p->drawRect(-1,-1,image->width()+2,image->height()+2);
						}
						p->drawImage(0,0,*image);
						p->restore();
					}
			}else{
				QSimpleRichText *richtext = new QSimpleRichText(title,tmpfont);
				richtext->setWidth(p,500);
				namelength = richtext->widthUsed();

				p->save();
				if(orientation)
					p->translate((int)(x1+70*size.X()),(int)(y1+size.Y()*(20*number+5)));
				else
					p->translate((int)(x1+10*size.X()),(int)(y1+size.Y()*(20*number+5)));
				richtext->draw(p,0,0,QRect(),QColorGroup());
				p->restore();

				delete richtext;
			}
		}else{	// simple 2d plot
			QFont tmpfont = font;
			int tmpsize = font.pointSize();
			tmpfont.setPointSize((int)(size.X()*tmpsize));

			// resize font with plot size
			int tmpy = (int)(y1+size.X()*tmpsize*(1.5+1.5*number));

			// draw icon
			g->drawStyle(p,(int)(x1+5*size.X()),tmpy);

			// NOT working : label->draw(0,p,pos,size,size.X(),size.Y(),0);
			if(label->isTeXLabel()) {
					QImage *image = new QImage(filename);
					if(!image->isNull()) {
						namelength < image->width() ? namelength = image->width() :0;

						//				kdDebug()<<"\n	drawing TeX image file "<<filename<<endl;
						p->save();
						p->translate((int)(x1+40*size.X()),tmpy - image->height()/2);
						// phi : normal angle, rotation : additional rotation
						p->rotate(label->Rotation());
						if (label->Boxed()) {
							p->setPen(QColor("black"));
							p->drawRect(-1,-1,image->width()+2,image->height()+2);
						}
						p->drawImage(0,0,*image);
						p->restore();
					}
					delete image;
			}else{
				QSimpleRichText *richtext = new QSimpleRichText(title,tmpfont);
				richtext->setWidth(p,500);
				namelength < richtext->widthUsed() ? namelength = richtext->widthUsed() :0;

				p->save();
				p->translate((int)(x1+40*size.X()),tmpy - richtext->height()/2);
				richtext->draw(p,0,0,QRect(),QColorGroup());
				p->restore();

				delete richtext;
			}
		}
	}
*/
// }


/*
void Legend::save(QTextStream *t) {
	*t<<x<<' '<<y<<endl;
	*t<<font.family()<<endl;
	*t<<font.pointSize()<<' '<<font.weight()<<' '<<font.italic()<<endl;
	*t<<enabled<<' '<<border<<endl;
	*t<<orientation<<endl;
	*t<<color.name()<<endl;
	*t<<transparent<<endl;
}

void Legend::open(QTextStream *t, int version) {
	kdDebug()<<"Legend::open()"<<endl;
	QString family, col;
	int pointsize, weight, italic, tmp;

	*t>>x>>y;

	if(version > 3) {
		t->readLine();
		family=t->readLine();
		*t>>pointsize>>weight>>italic;
	}
	else {
		*t>>family>>pointsize>>weight>>italic;
	}
	font = QFont(family,pointsize,weight,italic);

	if (version > 4) {
		int en, be;
		*t>>en>>be;
		enabled = en;
		border = be;
	}
	if(version > 20) {
		*t>>tmp;
		orientation = (bool)tmp;
	}
	if(version > 21) {
		*t>>col;
		color = QColor(col);
		*t>>tmp;
		transparent = (bool) tmp;
	}

	kdDebug()<<"Legend : "<<x<<' '<<y<<endl;
	kdDebug()<<"	 "<<family<<' '<<pointsize<<endl;
	kdDebug()<<"	COLOR "<<color.name()<<' '<<(int)transparent<<endl;
}

QDomElement Legend::saveXML(QDomDocument doc) {
	QDomElement legendtag = doc.createElement( "Legend" );

	QDomElement tag = doc.createElement( "Enabled" );
   	legendtag.appendChild( tag );
  	QDomText t = doc.createTextNode( QString::number(enabled) );
    	tag.appendChild( t );
	tag = doc.createElement( "Border" );
   	legendtag.appendChild( tag );
  	t = doc.createTextNode( QString::number(border) );
    	tag.appendChild( t );
	tag = doc.createElement( "Orientation" );
   	legendtag.appendChild( tag );
  	t = doc.createTextNode( QString::number(orientation) );
    	tag.appendChild( t );
	tag = doc.createElement( "Position" );
	tag.setAttribute("x",x);
	tag.setAttribute("y",y);
    	legendtag.appendChild( tag );

	tag = doc.createElement( "Font" );
	tag.setAttribute("family",font.family());
	tag.setAttribute("pointsize",font.pointSize());
	tag.setAttribute("weight",font.weight());
	tag.setAttribute("italic",font.italic());
    	legendtag.appendChild( tag );

	tag = doc.createElement( "Color" );
   	legendtag.appendChild( tag );
  	t = doc.createTextNode( color.name() );
    	tag.appendChild( t );
	tag = doc.createElement( "Transparent" );
   	legendtag.appendChild( tag );
  	t = doc.createTextNode( QString::number(transparent) );
    	tag.appendChild( t );

	return legendtag;
}

void Legend::openXML(QDomNode node) {
	while(!node.isNull()) {
		QDomElement e = node.toElement();
//		kdDebug()<<"LEGEND TAG = "<<e.tagName()<<endl;
//		kdDebug()<<"LEGEND TEXT = "<<e.text()<<endl;

		if(e.tagName() == "Enabled")
			enabled = (bool) e.text().toInt();
		else if(e.tagName() == "Border")
			border = (bool) e.text().toInt();
		else if(e.tagName() == "Orientation")
			orientation = (bool) e.text().toInt();
		else if(e.tagName() == "Position") {
			x = e.attribute("x").toDouble();
			y = e.attribute("y").toDouble();
		}
		else if(e.tagName() == "Font")
			font = QFont(e.attribute("family"),e.attribute("pointsize").toInt(),
				e.attribute("weight").toInt(),(bool) e.attribute("italic").toInt());
		else if(e.tagName() == "Color")
			color = QString(e.text());
		else if(e.tagName() == "Transparent")
			transparent = (bool) e.text().toInt();

		node = node.nextSibling();
	}
}*/
