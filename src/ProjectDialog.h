//LabPlot : ProjectDialog.h

#ifndef PROJECTDIALOG_H
#define PROJECTDIALOG_H

/*#include <qfont.h>
#include <qtextedit.h>
#include <qcheckbox.h>
#include <kdeversion.h>
#include "Worksheet.h"*/
#include <KLineEdit>
#include <KDateTimeWidget>
#include <QTextEdit>
#include "Dialog.h"

class ProjectDialog: public Dialog
{
	Q_OBJECT
public:
	ProjectDialog(MainWin *mw);
/*public slots:
	QString Title() { return titlele->text(); }
	void setTitle(QString t) { titlele->setText(t); }
	QString Author() { return authorle->text(); }
	void setAuthor(QString t) { authorle->setText(t); }
	QDateTime Created() { 
		return created->dateTime(); 
	}
	void setCreated(QDateTime dt) { 
		created->setDateTime(dt); 
	}	
	QDateTime Modified() { 
		return modified->dateTime(); 
	}
	void setModified(QDateTime dt) { 
		modified->setDateTime(dt); 
	}	
	QString Notes() { return noteste->text(); }
	void setNotes(QString n) { noteste->setText(n); }
*/
private:
	Project *project;
	KLineEdit *titlele, *authorle;
	void setupGUI();
	QTextEdit *noteste;
	KDateTimeWidget *created, *modified;
private slots:
	void Apply();
};

#endif //PROJECTDIALOG_H
