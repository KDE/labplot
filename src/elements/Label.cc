//LabPlot: Label.cc

#include <KDebug>
#include <QPainter>
#include <QTextDocument>
#include "Label.h"

Label::Label(QString text){
	m_text=text;

	//initialize default values
	m_textColor=Qt::black;
// 	font = f;
	m_positionType=LEFT;
	m_position.setX(0);
	m_position.setY(0);
	m_boxEnabled = false;
	m_shadowEnabled=false;
	m_fillingEnabled = false;
	m_fillingColor = Qt::white;
	m_rotation = 0.0;
	m_texEnabled = false;
}

/*int Label::Length() {
	QSimpleRichText *rt = new QSimpleRichText(title,font);

	return rt->width();
}

QString Label::simpleTitle() {
	QString t(title);

	t.replace(QRegExp(QString("\n")),"");	// replace newlines

	QRegExp rx("<.*>");		// remove all html tags
	rx.setMinimal(true);
	t.replace(rx,"");

	return t;
}

QStringList Label::Info() {
	QStringList s;

	// no label
	if(title.isEmpty())
		return s;

	s << QString::number(x) + "/" + QString::number(y);

	return s;
}

*/

//TODO use QPoints instead of Point?
void Label::draw(QPainter *p, const Point pos, const Point size, const int w, const int h, const double phi) const{
	kDebug()<<"Start drawing"<<endl;

	//TODO
/*	// resize font with plot size
	QFont tmpfont(font);
	int tmpsize = font.pointSize();
	tmpfont.setPointSize((int)(size.X()*tmpsize));
*/

	QTextDocument *t = new QTextDocument();
	t->setDefaultFont(m_textFont);
	t->setHtml(m_text);
	QRect box( 0, 0, t->size().width(), t->size().height() );
	float xOffset, yOffset;

	//TODO: adjust the offsets depending on whether
	//the bounding box and shadow are enabled or not
	switch (m_positionType){
		case LEFT:
			xOffset = 0;
			yOffset = 0;
			break;
		case CENTER:
			xOffset = (size.x()-t->size().width()/w)/2;
			yOffset = 0;
			break;
	case RIGHT:
			xOffset = size.x()-t->size().width()/w;
			yOffset = 0;
			break;
	case CUSTOM:
			xOffset = m_position.x();
			yOffset = m_position.y();
			break;
	}
	p->save();
	p->translate((pos.x()+xOffset)*w, (pos.y()+yOffset)*h);
	p->rotate(phi+m_rotation); // phi : normal angle, m_rotation : additional rotation

	//show shadow, if enabled
	//TODO make the size of the shadow depend on w and h.
	if (m_shadowEnabled){
// 		Q_UNUSED(box);
		QRectF rightShadow(box.right()+1, box.top() + 5, 5, box.height());
		QRectF bottomShadow(box.left() + 5, box.bottom()+1, box.width(), 5);
// 		if (rightShadow.intersects(box) || rightShadow.contains(box))
			p->fillRect(rightShadow, Qt::darkGray);

// 		if (bottomShadow.intersects(box) || bottomShadow.contains(box))
			p->fillRect(bottomShadow, Qt::darkGray);
	}

	//show bounding box, if enabled
	if (m_boxEnabled) {
		p->setPen( Qt::black );
		p->drawRect(box);
	}

	//fill the label box, if enabled.
	if( m_fillingEnabled )
		p->fillRect( box, QBrush(m_fillingColor) );


		//TODO
	/*	if(is_texlabel) {
		KTempDir *tmpdir = new KTempDir();
		QString dirname = tmpdir->name();
		KProcess *proc = new KProcess;
		*proc << "texvc";
		*proc << "/tmp"<<dirname<<title;
		if( proc->start(KProcess::Block) == false) {
			kDebug()<<"COULD NOT FIND texvc! Gving up."<<endl;
			KMessageBox::error((QWidget *)ws,i18n("Could not find texvc! Falling back to normal label."));
			is_texlabel = false;
		}
		else {
			// take resulting image and show it
			QDir d(dirname);
			QString filename = dirname+QString(d[2]);
			QImage *image = new QImage(filename);
 			if(!image->isNull()) {
//				kDebug()<<"\n	drawing TeX image file "<<filename<<endl;
				p->save();
				p->translate((pos.X()+x*size.X())*w,(pos.Y()+y*size.Y())*h);
				// phi : normal angle, rotation : additional rotation
				p->rotate(phi+rotation);
				if (boxed) {
					p->setPen(QColor("black"));
					p->drawRect(-1,-1,image->width()+2,image->height()+2);
				}
				p->drawImage(0,0,*image);
				p->restore();
			}
#if KDE_VERSION > 0x030104
			tmpdir->unlink();
#else
		rmdir(dirname.latin1());
#endif
			return;
		}
	}
	*/
	t->drawContents(p);
	p->restore();
	kDebug()<<"Label drawn"<<endl;
}

/*
//! return true if mouse inside (x0,y0)
bool Label::inside(int x0, int y0, Point pos, Point size, int w, int h) {
	int x1 = (int)(w*(pos.X()+size.X()*x));		// left
	int x2 = (int)(x1+richtext->widthUsed());	// right
	int y1 = (int)(h*(pos.Y()+size.Y()*y));		// top
	int y2 = (int)(y1+richtext->height());		// botton

	if (x0>x1 && x0<x2 && y0>y1 && y0<y2 )
		return true;
	return false;
}

bool Label::insideY(int x0, int y0, Point pos, Point size, int w, int h) {
	int x1 = (int)(w*(pos.X()+size.X()*x));		// left
	int x2 = (int)(x1+richtext->widthUsed());	// right
	int y2 = (int)(h*(pos.Y()+size.Y()*y));		// top
	int y1 = (int)(y2-richtext->widthUsed());		// botton

	//kDebug()<<"Label::insideY()  X/Y    X1/X2 | Y1/Y2"<<endl;
	//kDebug()<<"       "<<x0<<' '<<y0<<"   "<<x1<<'/'<<x2<<" | "<<y1<<'/'<<y2<<endl;

	if (x0>x1 && x0<x2 && y0>y1 && y0<y2 )
		return true;
	return false;
}

bool Label::insideZ(int x0, int y0, Point pos, Point size, int w, int h) {
	int x1 = (int)(w*(pos.X()+size.X()*x)-richtext->height());
	int x2 = (int)(w*(pos.X()+size.X()*x)+richtext->width());
	int y1 = (int)(h*(pos.Y()+size.Y()*y)-richtext->height());
	int y2 = (int)(h*(pos.Y()+size.Y()*y)+richtext->width());

	if (x0>x1 && x0<x2 && y0>y1 && y0<y2 )
		return true;
	return false;
}
*/

/*
void Label::save(QTextStream *ts) {
	title.replace( QRegExp(QString("\n")), QString("") ); // strip all newlines from t
	*ts<<title<<endl;

	*ts<<font.family()<<endl;
	*ts<<font.pointSize()<<' ';
	*ts<<font.weight()<<' '<<font.italic()<<endl;

	*ts<<color.name()<<endl;

	*ts<<x<<' '<<y<<endl;
	*ts<<(int)boxed<<endl;
	*ts<<rotation<<endl;
	*ts<<(int)is_texlabel<<endl;

	*ts<<bgcolor.name()<<endl;
	*ts<<transparent<<endl;
}

//!< ALSO change in Plot.cc !!!
void Label::open(QTextStream *ts,int version, bool skip) {
//	kDebug()<<"Label::open()"<<endl;
	QString t, family, c;
	int pointsize, weight, italic, tmp;
	double tmpx,tmpy;

	if(skip)
		ts->readLine();

	title = ts->readLine();

	if(version>3) {
		family=ts->readLine();
		*ts>>pointsize>>weight>>italic;
	}
	else {
		*ts>>family>>pointsize>>weight>>italic;
	}

	font = QFont(family,pointsize,weight,italic);

//	kDebug()<<"Label : "<<title<<endl;
//	kDebug()<<"(family/pointsize/weight/italic) : "<<family<<' '<<pointsize<<' '<<weight<<' '<<italic<<endl;

	if (version > 3) {
		*ts>>c;
		color = QColor(c);
		*ts>>tmpx>>tmpy;
		x = tmpx;
		y = tmpy;
	}
	if (version > 8) {
		*ts>>tmp;
		boxed = (bool)tmp;
	}
	if (version > 16)
		*ts>>rotation;
	if (version > 20) {
		*ts>>tmp;
		is_texlabel = (bool)tmp;
	}
	if (version > 21) {
		*ts>>c;
		bgcolor = QColor(c);
		*ts>>tmp;
		transparent=(bool)tmp;
//		kDebug()<<"	BGCOLOR : "<<bgcolor.name()<<endl;
	}
}

QDomElement Label::saveXML(QDomDocument doc) {
	QDomElement labeltag = doc.createElement( "Label" );

	QDomElement tag = doc.createElement( "Title" );
    	labeltag.appendChild( tag );
  	QDomText t = doc.createTextNode( title );
    	tag.appendChild( t );

	tag = doc.createElement( "Font" );
	tag.setAttribute("family",font.family());
	tag.setAttribute("pointsize",font.pointSize());
	tag.setAttribute("weight",font.weight());
	tag.setAttribute("italic",font.italic());
    	labeltag.appendChild( tag );

	tag = doc.createElement( "Color" );
    	labeltag.appendChild( tag );
  	t = doc.createTextNode( color.name() );
    	tag.appendChild( t );
	tag = doc.createElement( "Position" );
	tag.setAttribute("x",x);
	tag.setAttribute("y",y);
    	labeltag.appendChild( tag );
	tag = doc.createElement( "Boxed" );
    	labeltag.appendChild( tag );
  	t = doc.createTextNode( QString::number(boxed) );
    	tag.appendChild( t );
	tag = doc.createElement( "Rotation" );
    	labeltag.appendChild( tag );
  	t = doc.createTextNode( QString::number(rotation) );
    	tag.appendChild( t );
	tag = doc.createElement( "TeXLabel" );
    	labeltag.appendChild( tag );
  	t = doc.createTextNode( QString::number(is_texlabel) );
    	tag.appendChild( t );
	tag = doc.createElement( "BackgroundColor" );
    	labeltag.appendChild( tag );
  	t = doc.createTextNode( bgcolor.name() );
    	tag.appendChild( t );
	tag = doc.createElement( "Transparent" );
    	labeltag.appendChild( tag );
  	t = doc.createTextNode( QString::number(transparent) );
    	tag.appendChild( t );

	return labeltag;
}

void Label::openXML(QDomNode node) {
	while(!node.isNull()) {
		QDomElement e = node.toElement();
//		kDebug()<<"LABEL TAG = "<<e.tagName()<<endl;
//		kDebug()<<"LABEL TEXT = "<<e.text()<<endl;

		if(e.tagName() == "Title")
			title = e.text();
		else if(e.tagName() == "Font")
			font = QFont(e.attribute("family"),e.attribute("pointsize").toInt(),
				e.attribute("weight").toInt(),(bool) e.attribute("italic").toInt());
		else if(e.tagName() == "Color")
			color = QColor(e.text());
		else if(e.tagName() == "Position") {
			x = e.attribute("x").toDouble();
			y = e.attribute("y").toDouble();
		}
		else if(e.tagName() == "Boxed")
			boxed = (bool) e.text().toInt();
		else if(e.tagName() == "Rotation")
			rotation = e.text().toInt();
		else if(e.tagName() == "TeXLabel")
			is_texlabel = (bool) e.text().toInt();
		else if(e.tagName() == "BackgroundColor")
			bgcolor = QColor(e.text());
		else if(e.tagName() == "Transparent")
			transparent = (bool) e.text().toInt();

		node = node.nextSibling();
	}
}

void Label::saveSettings(KConfig *config, QString entry) {
	config->writeEntry(entry+"LabelTitle",title);
	config->writeEntry(entry+"LabelFont",font);
	config->writeEntry(entry+"LabelColor",color);
	config->writeEntry(entry+"LabelXPosition",x);
	config->writeEntry(entry+"LabelYPosition",y);
	config->writeEntry(entry+"LabelBoxed",boxed);
	config->writeEntry(entry+"LabelRotation",rotation);
	config->writeEntry(entry+"LabelTeXLabel",is_texlabel);
	config->writeEntry(entry+"LabelBackgroundColor",bgcolor);
	config->writeEntry(entry+"LabelTransparent",transparent);
}

void Label::readSettings(KConfig *config, QString entry) {
	title = config->readEntry(entry+"LabelTitle",title);
	font = config->readFontEntry(entry+"LabelFont",&font);
	color = config->readColorEntry(entry+"LabelColor",&color);
	x = config->readDoubleNumEntry(entry+"LabelXPosition",x);
	y = config->readDoubleNumEntry(entry+"LabelYPosition",y);
	boxed = config->readBoolEntry(entry+"LabelBoxed",boxed);
	rotation = config->readDoubleNumEntry(entry+"LabelRotation",rotation);
	is_texlabel = config->readBoolEntry(entry+"LabelTeXLabel",is_texlabel);
	bgcolor = config->readColorEntry(entry+"LabelBackgroundColor",&bgcolor);
	transparent = config->readBoolEntry(entry+"LabelTranparent",transparent);
}
*/