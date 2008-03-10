//LabPlot : Project.cc

#include <KDebug>
#include "Project.h"

Project::Project()
{
	// new project
	filename=QString("");
	version=0;
	labplot=QString(LVERSION);
	title=QString("");
	author=QString("");
	created=QDateTime::currentDateTime();
	modified=QDateTime::currentDateTime();
	notes=QString("");
	changed=false;
}

QDomElement Project::saveXML(QDomDocument doc) {
	QDomElement ptag = doc.createElement( "Project" );

	QDomElement tag = doc.createElement( "Title" );
	ptag.appendChild( tag );
	QDomText t = doc.createTextNode( title );
	tag.appendChild( t );
	tag = doc.createElement( "Author" );
	ptag.appendChild( tag );
	t = doc.createTextNode( author );
	tag.appendChild( t );
#if QT_VERSION > 0x030007
	tag = doc.createElement( "Created" );
	ptag.appendChild( tag );
	t = doc.createTextNode( QString::number( created.toTime_t() ) );
	tag.appendChild( t );
	tag = doc.createElement( "Date" );
	ptag.appendChild( tag );
	t = doc.createTextNode( QString::number( modified.toTime_t() ) );
	tag.appendChild( t );
#else
	QDateTime stoneage(QDate(1970,1,1));
	tag = doc.createElement( "Created" );
	ptag.appendChild( tag );
	t = doc.createTextNode( QString::number( stoneage.secsTo( project->Created() ) ));
	tag.appendChild( t );
	tag = doc.createElement( "Date" );
	ptag.appendChild( tag );
	t = doc.createTextNode( QString::number(stoneage.secsTo(QDateTime::currentDateTime()) ));
	tag.appendChild( t );
#endif
	modified = QDateTime::currentDateTime();
	tag = doc.createElement( "Notes" );
	ptag.appendChild( tag );
	t = doc.createTextNode(notes);
	tag.appendChild( t );

	return ptag;
}

void Project::openXML(QDomNode node) {
	while(!node.isNull()) {
		QDomElement e = node.toElement();
//		kdDebug()<<"PROJECT TAG = "<<e.tagName()<<endl;
//		kdDebug()<<"PROJECT TEXT = "<<e.text()<<endl;

		if(e.tagName() == "Title")
			title = e.text();
		else if(e.tagName() == "Author")
			author = e.text();
		else if(e.tagName() == "Comment")	// old name
			notes = e.text();
		else if(e.tagName() == "Notes")
			notes = e.text();
		else if(e.tagName() == "Created")
			created.setTime_t(e.text().toInt());
		else if(e.tagName() == "Date")
			modified.setTime_t(e.text().toInt());

		node = node.nextSibling();
	}
}
