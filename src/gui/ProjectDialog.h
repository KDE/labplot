//LabPlot : ProjectDialog.h

#ifndef PROJECTDIALOG_H
#define PROJECTDIALOG_H

#include <KDialog>
#include <QtGui>
#include "../ui_projectdialog.h"
class MainWin;
class Project;

/**
 * @brief Provides a dialog for editing project settings.
 */
class ProjectDialog: public KDialog {
	Q_OBJECT
public:
	ProjectDialog(MainWin *mw);
private:
	Ui::ProjectDialog ui;
	Project *project;
	void setupGUI();
private slots:
	void apply();
};

#endif //PROJECTDIALOG_H
