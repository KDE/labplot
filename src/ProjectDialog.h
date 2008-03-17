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
