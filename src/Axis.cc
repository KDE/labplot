//LabPlot: Axis.cc

#include <KDebug>
#include "Axis.h"

//! general axis class
Axis::Axis() {
	kDebug()<<"Axis()"<<endl;
	label = new Label();
	scale = LINEAR;
	position = 0;
	scaling=1;
	shift=0;
	enabled = true;
	ticktype = 1;
	ticklabelrotation = 0;
	ticklabelprefix = QString("");
	ticklabelsuffix = QString("");
	tickfont = QFont(QString("Adobe Times"),12);
	tickcolor = QColor("black");
	ticklabelcolor = QColor("black");
	bordercolor = QColor("black");
	minorgridcolor = QColor("black");
	majorgridcolor = QColor("black");
	majorticks = -1;		// default ("auto")
	minorticks = 3;
	majorticks_enabled = true;
	minorticks_enabled = true;
	ticklabel_enabled = true;
	ticklabelformat = AUTO;
	datetimeformat = "auto";
	ticklabelprecision = 3;
	tickposition = 0;
	gap = 15;
	majorgridtype = Qt::DashLine;
	minorgridtype = Qt::DotLine;
	majortickwidth = minortickwidth = 1;
	borderwidth = 1;
	majorgridwidth = minorgridwidth = 1;
	border_enabled = true;
	majorgrid_enabled = false;
	minorgrid_enabled = false;
}

/*void Axis::setLabel(Label *l) {
	if(label != 0 && l != 0)
		delete label;

	label=l;
}

void Axis::centerX(int plotsize, double center) {
	int length = label->Length();
//	kDebug()<<"LENGTH = "<<length<<endl;
//	kDebug()<<"PLOTSIZE = "<<plotsize<<endl;
//	kDebug()<<"CENTER = "<<center<<endl;

	double nx = center - length/(2.0*plotsize);
	kDebug()<<"NX="<<nx<<endl;
	label->setX(nx);
}

void Axis::centerY(int plotsize, double center) {
	int length = label->Length();
//	kDebug()<<"LENGTH = "<<length<<endl;
//	kDebug()<<"PLOTSIZE = "<<plotsize<<endl;
//	kDebug()<<"CENTER = "<<center<<endl;

	double ny = center + length/(2.0*plotsize);
	kDebug()<<"NY="<<ny<<endl;
	label->setY(ny);
}

QDomElement Axis::saveXML(QDomDocument doc, int id) {
	QDomElement axistag = doc.createElement( "Axis" );
	axistag.setAttribute("id",QString::number(id));

	QDomElement tag = doc.createElement( "Enabled" );
   	axistag.appendChild( tag );
  	QDomText t = doc.createTextNode( QString::number(enabled) );
    	tag.appendChild( t );
	tag = doc.createElement( "Scale" );
   	axistag.appendChild( tag );
  	t = doc.createTextNode( QString::number(scale) );
    	tag.appendChild( t );
	tag = doc.createElement( "Position" );
   	axistag.appendChild( tag );
  	t = doc.createTextNode( QString::number(position) );
    	tag.appendChild( t );
	tag = doc.createElement( "Scaling" );
   	axistag.appendChild( tag );
  	t = doc.createTextNode( QString::number(scaling) );
    	tag.appendChild( t );
	tag = doc.createElement( "Shift" );
   	axistag.appendChild( tag );
  	t = doc.createTextNode( QString::number(shift) );
    	tag.appendChild( t );

	tag = doc.createElement( "Border" );
	tag.setAttribute("enabled",QString::number(border_enabled));
	tag.setAttribute("width",QString::number(borderwidth));
	tag.setAttribute("color",bordercolor.name());
	axistag.appendChild( tag );

	tag = doc.createElement( "MajorGrid" );
	tag.setAttribute("enabled",QString::number(majorgrid_enabled));
	tag.setAttribute("width",QString::number(majorgridwidth));
	tag.setAttribute("color",majorgridcolor.name());
	tag.setAttribute("style",QString::number(majorgridtype));
	axistag.appendChild( tag );
	tag = doc.createElement( "MinorGrid" );
	tag.setAttribute("enabled",QString::number(minorgrid_enabled));
	tag.setAttribute("width",QString::number(minorgridwidth));
	tag.setAttribute("color",minorgridcolor.name());
	tag.setAttribute("style",QString::number(minorgridtype));
	axistag.appendChild( tag );

	tag = doc.createElement( "MajorTicks" );
	tag.setAttribute("enabled",QString::number(majorticks_enabled));
	tag.setAttribute("position",QString::number(tickpos));
	tag.setAttribute("type",QString::number(ticktype));
	tag.setAttribute("nr",QString::number(majorticks));
	tag.setAttribute("width",QString::number(majortickwidth));
	tag.setAttribute("color",tickcolor.name());
	tag.setAttribute("length",QString::number(majorticklength));
	axistag.appendChild( tag );
	tag = doc.createElement( "MinorTicks" );
	tag.setAttribute("enabled",QString::number(minorticks_enabled));
	tag.setAttribute("nr",QString::number(minorticks));
	tag.setAttribute("width",QString::number(minorTickWidth()));
	tag.setAttribute("length",QString::number(minorticklength));
	axistag.appendChild( tag );

	QDomElement ticktag = doc.createElement( "TickLabel" );
	ticktag.setAttribute("enabled",QString::number(ticklabel_enabled));
	axistag.appendChild( ticktag );
	tag = doc.createElement( "Prefix" );
   	ticktag.appendChild( tag );
  	t = doc.createTextNode( ticklabelprefix );
    	tag.appendChild( t );
	tag = doc.createElement( "Suffix" );
   	ticktag.appendChild( tag );
  	t = doc.createTextNode( ticklabelsuffix );
    	tag.appendChild( t );
	tag = doc.createElement( "Font" );
	tag.setAttribute("family",tickfont.family());
	tag.setAttribute("pointsize",tickfont.pointSize());
	tag.setAttribute("weight",tickfont.weight());
	tag.setAttribute("italic",tickfont.italic());
    	ticktag.appendChild( tag );
	tag = doc.createElement( "Rotation" );
   	ticktag.appendChild( tag );
  	t = doc.createTextNode( QString::number(ticklabelrotation) );
    	tag.appendChild( t );
	tag = doc.createElement( "Position" );
   	ticktag.appendChild( tag );
  	t = doc.createTextNode( QString::number(gap) );
    	tag.appendChild( t );
	tag = doc.createElement( "Color" );
   	ticktag.appendChild( tag );
  	t = doc.createTextNode( ticklabelcolor.name() );
    	tag.appendChild( t );
	tag = doc.createElement( "Format" );
   	ticktag.appendChild( tag );
  	t = doc.createTextNode( QString::number(ticklabelformat) );
    	tag.appendChild( t );
	tag = doc.createElement( "Precision" );
   	ticktag.appendChild( tag );
  	t = doc.createTextNode( QString::number(ticklabelprecision) );
    	tag.appendChild( t );
	tag = doc.createElement( "DatetimeFormat" );
   	ticktag.appendChild( tag );
  	t = doc.createTextNode( datetimeformat );
    	tag.appendChild( t );

	tag = label->saveXML(doc);
	axistag.appendChild( tag );

	return axistag;
}

void Axis::openXML(QDomNode node) {
	kDebug()<<"Axis::openXML()"<<endl;
	while(!node.isNull()) {
		QDomElement e = node.toElement();
//		kDebug()<<"AXIS TAG : "<<e.text()<<endl;

		if(e.tagName() == "Enabled")
			enabled = (bool) e.text().toInt();
		else if(e.tagName() == "Scale")
			scale = (TScale) e.text().toInt();
		else if(e.tagName() == "Position")
			position = e.text().toInt();
		else if(e.tagName() == "Scaling")
			scaling = e.text().toDouble();
		else if(e.tagName() == "Shift")
			shift = e.text().toDouble();

		else if(e.tagName() == "Border") {
			border_enabled = (bool) e.attribute("enabled").toInt();
			borderwidth = e.attribute("width").toInt();
			bordercolor = QColor(e.attribute("color"));
		}
		else if(e.tagName() == "MajorGrid") {
			majorgrid_enabled = (bool) e.attribute("enabled").toInt();
			majorgridwidth = e.attribute("width").toInt();
			majorgridcolor = QColor(e.attribute("color"));
			majorgridtype = (Qt::PenStyle) e.attribute("style").toInt();
		}
		else if(e.tagName() == "MinorGrid") {
			minorgrid_enabled = (bool) e.attribute("enabled").toInt();
			minorgridwidth = e.attribute("width").toInt();
			minorgridcolor = QColor(e.attribute("color"));
			minorgridtype = (Qt::PenStyle) e.attribute("style").toInt();
		}

		else if(e.tagName() == "MajorTicks") {
			majorticks_enabled = (bool) e.attribute("enabled").toInt();
			tickpos = e.attribute("position").toInt();
			ticktype = e.attribute("type").toInt();
			majorticks = e.attribute("nr").toDouble();
			majortickwidth = e.attribute("width").toInt();
			tickcolor = QColor(e.attribute("color"));
			majorticklength = e.attribute("length").toDouble();
		}
		else if(e.tagName() == "MinorTicks") {
			minorticks_enabled = (bool) e.attribute("enabled").toInt();
			minorticks = e.attribute("nr").toInt();
			minortickwidth = e.attribute("width").toInt();
			minorticklength = e.attribute("length").toDouble();
		}
		else if(e.tagName() == "TickLabel") {
			ticklabel_enabled = (bool) e.attribute("enabled").toInt();
			QDomNode ticknode = e.firstChild();
			while(!ticknode.isNull()) {
				QDomElement te = ticknode.toElement();
//				kDebug()<<"TICK TAG : "<<te.text()<<endl;
				if(te.tagName() == "Prefix")
					ticklabelprefix = te.text();
				else if(te.tagName() == "Suffix")
					ticklabelsuffix = te.text();
				else if(te.tagName() == "Font")
					tickfont = QFont(te.attribute("family"),te.attribute("pointsize").toInt(),
						te.attribute("weight").toInt(),(bool) te.attribute("italic").toInt());
				else if(te.tagName() == "Rotation")
					ticklabelrotation = te.text().toDouble();
				else if(te.tagName() == "Position")
					gap = te.text().toInt();
				else if(te.tagName() == "Color")
					ticklabelcolor = QColor(te.text());
				else if(te.tagName() == "Format")
					ticklabelformat = (TFormat) te.text().toInt();
				else if(te.tagName() == "Precision")
					ticklabelprecision = te.text().toInt();
				else if(te.tagName() == "DateTimeFormat")
					datetimeformat = te.text();

				ticknode = ticknode.nextSibling();
			}
		}

		label->openXML(e.firstChild());

		node = node.nextSibling();
	}
}
*/
