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
	Spreadsheet* activeSpreadsheet();
	Worksheet* activeWorksheet();
	Project* getProject() { return project; }
	void setProject(Project *p) { project=p; }
	void addSet(Set *g, int sheet, PlotType ptype);
private:
	QMdiArea *mdi;
	Project *project;
	QMenu *spreadsheetmenu;
	KAction *spreadaction;
	void setupActions();
	bool warnModified();
	void updateGUI();		//!< update GUI of main window
	void updateSheetList();		//!< creates dynamic sheet list menu
	void updateSetList();		//!< creates dynamic set list menu
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
	void settingsDialog();
};

#endif
