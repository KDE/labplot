/***************************************************************************
    File                 : Axis.cc
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Stefan Gerlach, Alexander Semke
    Email (use @ for *)  : stefan.gerlach*uni-konstanz.de, alexander.semke*web.de
    Description          : axis class
                           
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
#include "Axis.h"
#include <KDebug>

//! general axis class
Axis::Axis() {
	kDebug()<<"Axis()"<<endl;

	//********** General *****************
	m_enabled = true;
	m_scaleType = Axis::SCALETYPE_LINEAR;
	m_position = Axis::POSITION_NORMAL;
	m_positionOffset = 0;

	m_scaleFactor = 1;
	m_offset = 0;

	m_borderEnabled = true;
	m_borderColor = QColor( Qt::black );
	m_borderWidth = 1;


	//*************** Ticks ******************
	m_ticksPosition = Axis::TICKSPOSITION_IN;
	m_ticksType = Axis::TICKSTYPE_INCREMENT;
	m_ticksColor = QColor( Qt::black );

	m_majorTicksEnabled = true;
	m_majorTicksNumberType = Axis::TICKSNUMBERTYPE_AUTO;
	m_majorTicksNumber = 1;
	m_majorTicksWidth = 1;
	m_majorTicksLength = 0.05;

	m_minorTicksEnabled = true;
	m_minorTicksNumberType = Axis::TICKSNUMBERTYPE_CUSTOM;
	m_minorTicksNumber = 3;
	m_minorTicksWidth = 1;
	m_minorTicksLength = 0.05;


	//*********** Tick labels ******************
	m_labelsEnabled = true;
	m_labelsPosition = 15;
	m_labelsRotation = 0;

	m_labelsPrecision = 3;
	m_labelsFormat = Axis::LABELSFORMAT_AUTO;
	m_labelsDateFormat = "auto";//TODO

// 	m_labelsFont = QFont(QString("Adobe Times"),12); //TODO use the system default font?
	m_labelsColor = QColor( Qt::black );
	m_labelsPrefix = "";
	m_labelsSuffix = "";


	//************ Grid ***********************
	m_majorGridEnabled = false;
	m_majorGridStyle = Qt::DashLine;
	m_majorGridColor = QColor( Qt::black );
	m_majorGridWidth = 1;

	m_minorGridEnabled = false;
	m_minorGridStyle = Qt::DotLine;
	m_minorGridColor = QColor( Qt::black );
	m_minorGridWidth = 1;
}

Label* Axis::label(){
	return &m_label;
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
*/

/*
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
