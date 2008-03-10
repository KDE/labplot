//LabPlot : Project.h

#ifndef PROJECT_H
#define PROJECT_H

#include <QString>
#include <QDateTime>
#include <QDomDocument>
#include <QDomElement>

#include "defs.h"

class Project
{
public:
	Project();
	QDomElement saveXML(QDomDocument doc);
	void openXML(QDomNode node);
	ACCESS(QString, filename, Filename);
	ACCESS(int, version, Version);
	ACCESS(QString, labplot, LabPlot);
	ACCESS(QString, title, Title);
	ACCESS(QString, author, Author);
	ACCESS(QDateTime, created, Created);
	ACCESS(QDateTime, modified, Modified);
	ACCESS(QString, notes, Notes);
	ACCESS(bool, changed, Changed);
private:
	QString filename;
	int version;
	QString labplot;
	QString title;
	QString author;
	QDateTime created;
	QDateTime modified;
	QString notes;
	bool changed;		//!< set when project was changed
};

#endif //PROJECT_H
