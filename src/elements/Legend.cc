//LabPlot: Legend.cc

#include <cmath>
#include <iostream>
#include <QBrush>
#include <QColor>
#include "Legend.h"

/*
#include <qdir.h>
#include <qimage.h>
#include <kdebug.h>
#include <kdeversion.h>
#if KDE_VERSION > 0x030104
#include <ktempdir.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif
#include <kprocess.h>
#include <kmessagebox.h>

#include "GraphList.h"

using namespace std;
*/

Legend::Legend() {
// 	font = QFont(QString("Adobe Times"),8);
// 	enabled = true;
// 	border = true;
// 	x = 0.7;
// 	y = 0.05;
// 	color = QColor(Qt::white);
// 	transparent = true;
// 	namelength=0;
// 	orientation=0;
// 	ticlabellength=0;
}

/*
int Legend::drawGraphs(QPainter *p, GraphList *gl, PType type, Point size, QFont tmpfont ) {
	kdDebug()<<"Legend::drawGraphs()"<<endl;
	QFontMetrics fm = p->fontMetrics();
	int number=0;	// sed for hidden graphs
	for (unsigned int i = 0 ; i < gl->Number(); i++) {
		Graph *g = gl->getGraph(i);
//		kdDebug()<<"GRAPH "<<i<<" Label : "<<g->getLabel()->simpleTitle()<<endl;
		if(g->isShown() == false)
			continue;

		Label *label = g->getLabel();
		QString title = label->Title();

		Style *style = g->getStyle();

		QPen pen( style->Color(), style->Width(), (Qt::PenStyle) style->PenStyle() );
		p->setPen(pen);	// EPS BUG

		if (type == PSURFACE) {
			if(label->isTeXLabel()) {
#if KDE_VERSION > 0x030104
				KTempDir *tmpdir = new KTempDir();
				QString dirname = tmpdir->name();
#else
				QString dirname("/tmp/LabPlot-texvc");
				mkdir(dirname.latin1(),S_IRWXU);
#endif
				KProcess *proc = new KProcess;
				*proc << "texvc";
				*proc << "/tmp"<<dirname<<title;
				if( proc->start(KProcess::Block) == false) {
					kdDebug()<<"COULD NOT FIND texvc! Gving up."<<endl;
					// TODO : KMessageBox::error((QWidget *)ws,i18n("Could not find texvc! Falling back to normal label."));
					label->setTeXLabel(false);
				}
				else {
					// take resulting image and show it
					QDir d(dirname);
					QString filename = dirname+QString(d[2]);
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
					delete image;
#if KDE_VERSION > 0x030104
					tmpdir->unlink();
#else
					rmdir(dirname.latin1());
#endif
				}
				delete proc;
			}

			if(!label->isTeXLabel()) {
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

// 		// old style
// 			namelength  =  fm.width(name);
// 			if(namelength<8)	// minimum width
// 				namelength=8;
// 			p->setPen(Qt::black);
// 			if(orientation)
// 				p->drawText((int)(x1+70*size.X()),(int)(y1+size.Y()*(20*number+20)), name);
// 			else
// 				p->drawText((int)(x1+10*size.X()),(int)(y1+size.Y()*(20*number+20)), name);

		}
		else {	// simple 2d plot
			QFont tmpfont = font;
			int tmpsize = font.pointSize();
			tmpfont.setPointSize((int)(size.X()*tmpsize));

			// resize font with plot size
			int tmpy = (int)(y1+size.X()*tmpsize*(1.5+1.5*number));

			// draw icon
			g->drawStyle(p,(int)(x1+5*size.X()),tmpy);

			// NOT working : label->draw(0,p,pos,size,size.X(),size.Y(),0);
			if(label->isTeXLabel()) {
#if KDE_VERSION > 0x030104
				KTempDir *tmpdir = new KTempDir();
				QString dirname = tmpdir->name();
#else
				QString dirname("/tmp/LabPlot-texvc");
				mkdir(dirname.latin1(),S_IRWXU);
#endif
				KProcess *proc = new KProcess;
				*proc << "texvc";
				*proc << "/tmp"<<dirname<<title;
				if( proc->start(KProcess::Block) == false) {
					kdDebug()<<"COULD NOT FIND texvc! Gving up."<<endl;
					// TODO : KMessageBox::error((QWidget *)ws,i18n("Could not find texvc! Falling back to normal label."));
					label->setTeXLabel(false);
				}
				else {
					// take resulting image and show it
					QDir d(dirname);
					QString filename = dirname+QString(d[2]);
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
#if KDE_VERSION > 0x030104
					tmpdir->unlink();
#else
					rmdir(dirname.latin1());
#endif
				}
				delete proc;
			}

			if(!label->isTeXLabel()) {
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
		number++;
	}

	return number;
}

void Legend::draw( QPainter *p, PType type, GraphList *graphlist, Point pos, Point size, int w, int h) {
	kdDebug()<<"Legend::draw()"<<endl;

	x1 = x2 = (int) ((x*size.X()+pos.X())*w);
	y1 = y2 = (int) ((y*size.Y()+pos.Y())*h);

	namelength=0;	// reset namelength for legend box

	// set point size
	int pointsize = font.pointSize();
	QFont tmpfont = font;
	//kdDebug()<<"Setting font size to "<<(int) (pointsize*size.X())<<endl;
	tmpfont.setPointSize((int)(pointsize*size.X()));
	p->setFont(tmpfont);
	QFontMetrics fm = p->fontMetrics();

	int number = drawGraphs(p,graphlist,type,size,tmpfont);

	if (type == PSURFACE) {
		if(orientation) {
			x2 = (int) (x1+size.X()*(20+125+10));
			y2 = (int) (y1+40*size.Y()*number+namelength+5);
		}
		else {
			y2 = (int) (y1+size.Y()*(40*number+125+10));
//			kdDebug()<<"	NL "<<namelength<<" | TLL "<<ticlabellength<<endl;
			x2 = (int) (x1+40*size.X()+fmax(namelength-20*size.X(),ticlabellength)+5);
		}
	}
	else {
		x2 = (int) (x1+40*size.X()+namelength+5);
		y2 = (int) (y1+size.X()*pointsize*(1.5+1.5*number));
	}

	if (!transparent) {
		p->setBrush(color);
		p->setPen(Qt::NoPen);
		p->drawRect(x1,y1,x2-x1,y2-y1);
		p->setBrush(QBrush::NoBrush);
		// draw again (since it gets overwritten)
		drawGraphs(p,graphlist,type,size,tmpfont);
	}
	p->setBrush(QBrush::NoBrush);
	if (border)
		p->drawRect(x1,y1,x2-x1,y2-y1);

	// reset font point size
	//kdDebug()<<"Resetting font size to "<<pointsize<<endl;
	tmpfont.setPointSize(pointsize);
	p->setFont(tmpfont);
}

//! calculate if point x,y is inside the legend box (for mouse event)
bool Legend::inside(int X, int Y) {
	kdDebug()<<"x1/x2 y1/y2 "<<x1<<'/'<<x2<<' '<<y1<<'/'<<y2<<endl;
	kdDebug()<<"x/y "<<x<<' '<<y<<endl;
	if (X>x1 && X<x2 && Y>y1 && Y<y2)
		return true;
	else
		return false;
}

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
