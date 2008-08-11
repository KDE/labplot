//LabPlot : ImportDialog.h

#ifndef IMPORTDIALOG_H
#define IMPORTDIALOG_H

#include <KDialog>

class MainWin;
class Spreadsheet;
#include "ImportWidget.h"

/**
 * @brief Provides a dialog for importing data into a spreadsheet.
 */
class ImportDialog: public KDialog {
	Q_OBJECT
public:
	ImportDialog(MainWin *mw);
private:
	MainWin *mainWin;
	ImportWidget *importWidget;
	void setupGUI();
private slots:
	void apply() { importWidget->apply(mainWin); }
};

#endif //IMPORTDIALOG_H
