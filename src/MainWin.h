#ifndef MAINWIN_H
#define MAINWIN_H
#include <KXmlGuiWindow>
#include <QMdiArea>
#include <KTextEdit>
#include <KAction>

#include "Spreadsheet.h"
#include "Worksheet.h"
#include "Project.h"
#include "Set2D.h"
 
class MainWin : public KXmlGuiWindow
{
	Q_OBJECT
public:
	MainWin(QWidget *parent=0);
	int activeSheetIndex();
	QMdiArea* getMdi() { return mdi; }
	Project* getProject() { return project; } 
	Spreadsheet* activeSpreadsheet();
	Worksheet* activeWorksheet();
	void setProject(Project *p) { project=p; } 
	void addSet(Set *g, int sheet, PlotType ptype);
private:
	QMdiArea *mdi;
	Project *project;
	QMenu *spreadsheetmenu;
	KAction *spreadaction;
	bool modified;		// needs to be saved
	void setupActions();
	bool warnModified();
	void updateSheetList();
public slots:
	Spreadsheet* newSpreadsheet();
	Worksheet* newWorksheet();
private slots:
	void openNew();
	void print();
	void SpreadsheetMenu();
	void importDialog();
	void projectDialog();
	void functionDialog();
	void titleDialog();
	void axesDialog();
	void legendDialog();
	void plotDialog();
	void worksheetDialog();
};
 
#endif
