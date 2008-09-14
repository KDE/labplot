#ifndef MAINWIN_H
#define MAINWIN_H
#include <KXmlGuiWindow>
#include <QMdiArea>
#include <KTextEdit>
#include <KAction>

#include "Spreadsheet.h"
#include "Worksheet.h"
#include "Project.h"
#include "elements/Set.h"
#include "plots/Plot.h"

class MainWin : public KXmlGuiWindow
{
	Q_OBJECT
public:
	MainWin(QWidget *parent=0,QString filename=0);
	int activeSheetIndex() const;
	QMdiArea* getMdi() const { return mdi; }
	Spreadsheet* activeSpreadsheet() const;			//!< get active spreadsheet
	Spreadsheet* getSpreadsheet(QString title) const;	//!< get Spreadsheet of name title
	Worksheet* activeWorksheet() const;			//!< get active worksheet
	Worksheet* getWorksheet(QString name) const;		//!< get Worksheet of name title
	Project* getProject() const { return project; }
	void setProject(Project *p) { project=p; }
	void updateGUI();		//!< update GUI of main window
	void updateSheetList();		//!< update dynamic sheet list menu
	void updateSetList();		//!< update dynamic set list menu
	void openXML(QIODevice *file);	//!< do the actual opening
	void saveXML(QIODevice *file);	//!< do the actual saving

private:
	QMdiArea *mdi;
	Project *project;
	QMenu *spreadsheetmenu;
	KAction *spreadaction;
	void setupActions();
	bool warnModified();
	void addSet(Set s, const int sheet, const Plot::PlotType ptype);

public slots:
	Spreadsheet* newSpreadsheet();
	Worksheet* newWorksheet();
	void save(QString filename=0);	//!< save project (.lml format)
	void open(QString filename);	//!< open project (.lml format)
	void saveAs();	//!< save as different file name (.lml format)

private slots:
	void openNew();
	void print();
	void SpreadsheetMenu();
	void importDialog();
	void projectDialog();
	void functionActionTriggered(QAction*);
	void titleDialog();
	void axesDialog();
	void legendDialog();
	void settingsDialog();
};

#endif
