//LabPlot : Project.h

#ifndef PROJECT_H
#define PROJECT_H

#include <QString>
#include <QDateTime>
#include <QDomDocument>
#include <QDomElement>

#include "definitions.h"

class Project
{
public:
	Project();
	QDomElement save(QDomDocument doc);
	void open(QDomNode node);
	ACCESS(QString, filename, Filename);
	ACCESS(int, version, Version);
	ACCESS(QString, labPlot, LabPlot);
	ACCESS(QString, title, Title);
	ACCESS(QString, author, Author);
	ACCESS(QDateTime, created, Created);
	ACCESS(QDateTime, modified, Modified);
	ACCESS(QString, notes, Notes);
	ACCESSFLAG(m_changed, Changed);
private:
	QString m_filename;
	int m_version;
	QString m_labPlot;
	QString m_title;
	QString m_author;
	QDateTime m_created;
	QDateTime m_modified;
	QString m_notes;
	bool m_changed;		//!< set when project was changed
};

#endif //PROJECT_H
