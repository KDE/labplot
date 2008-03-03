//LabPlot : Project.h

#ifndef PROJECT_H
#define PROJECT_H

#include <QString>
#include <QDateTime>
#include <QDomDocument>
#include <QDomElement>

class Project
{
public:
	Project();
	QDomElement saveXML(QDomDocument doc);
	void openXML(QDomNode node);
	QString Filename() { return filename; }
	void setFilename(QString f) {filename=f;}
	int Version() {return version; }
	void setVersion (int v) { version = v;}
	QString LabPlot() { return labplot; }
	void setLabPlot(QString l) {labplot=l;}
	QString Title() { return title; }
	void setTitle(QString t) {title=t;}
	QString Author() { return author; }
	void setAuthor(QString a) {author=a;}
	QDateTime Created() { return created; }
	void setCreated(QDateTime c) {created=c;}
	QDateTime Modified() { return modified; }
	void setModified(QDateTime m) {modified=m;}
	QString Notes() { return notes; }
	void setNotes(QString c) {notes=c;}
private:
	QString filename;
	int version;
	QString labplot;
	QString title;
	QString author;
	QDateTime created;
	QDateTime modified;
	QString notes;
};

#endif //PROJECT_H
