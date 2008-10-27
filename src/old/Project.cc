/***************************************************************************
    File                 : Project.cc
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Stefan Gerlach
    Email (use @ for *)  : stefan.gerlach*uni-konstanz.de
    Description          : LabPlot project class
                           
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

#include <KDebug>
#include "Project.h"

Project::Project()
{
	// new project
	m_filename=QString("");
	m_version=0;
	m_labPlot=QString(LVERSION);
	m_title=QString("");
	m_author=QString("");
	m_created=QDateTime::currentDateTime();
	m_modified=QDateTime::currentDateTime();
	m_notes=QString("");
	m_changed=false;
}

QDomElement Project::save(QDomDocument doc) {
	QDomElement ptag = doc.createElement( "Project" );

	QDomElement tag = doc.createElement( "Title" );
	ptag.appendChild( tag );
	QDomText t = doc.createTextNode( m_title );
	tag.appendChild( t );
	tag = doc.createElement( "Author" );
	ptag.appendChild( tag );
	t = doc.createTextNode( m_author );
	tag.appendChild( t );
#if QT_VERSION > 0x030007
	tag = doc.createElement( "Created" );
	ptag.appendChild( tag );
	t = doc.createTextNode( QString::number( m_created.toTime_t() ) );
	tag.appendChild( t );
	tag = doc.createElement( "Date" );
	ptag.appendChild( tag );
	t = doc.createTextNode( QString::number( m_modified.toTime_t() ) );
	tag.appendChild( t );
#else
	QDateTime stoneage(QDate(1970,1,1));
	tag = doc.createElement( "Created" );
	ptag.appendChild( tag );
	t = doc.createTextNode( QString::number( stoneage.secsTo( m_created ) ));
	tag.appendChild( t );
	tag = doc.createElement( "Date" );
	ptag.appendChild( tag );
	t = doc.createTextNode( QString::number(stoneage.secsTo(QDateTime::currentDateTime()) ));
	tag.appendChild( t );
#endif
	m_modified = QDateTime::currentDateTime();
	tag = doc.createElement( "Notes" );
	ptag.appendChild( tag );
	t = doc.createTextNode( m_notes );
	tag.appendChild( t );

	return ptag;
}

void Project::open(QDomNode node) {
	while(!node.isNull()) {
		QDomElement e = node.toElement();
//		kDebug()<<"PROJECT TAG = "<<e.tagName()<<endl;
//		kDebug()<<"PROJECT TEXT = "<<e.text()<<endl;

		if(e.tagName() == "Title")
			m_title = e.text();
		else if(e.tagName() == "Author")
			m_author = e.text();
		else if(e.tagName() == "Comment")	// old name
			m_notes = e.text();
		else if(e.tagName() == "Notes")
			m_notes = e.text();
		else if(e.tagName() == "Created")
			m_created.setTime_t(e.text().toInt());
		else if(e.tagName() == "Date")
			m_modified.setTime_t(e.text().toInt());

		node = node.nextSibling();
	}
}
