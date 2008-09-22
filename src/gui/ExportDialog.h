//LabPlot : ExportDialog.h

#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H

#include <KDialog>

class MainWin;
class Spreadsheet;
#include "ExportWidget.h"

/**
 * @brief Provides a dialog for exporting data from a spreadsheet.
 */
class ExportDialog: public KDialog {
	Q_OBJECT
public:
	ExportDialog(MainWin *mw);
private:
	MainWin *mainWin;
	ExportWidget *exportWidget;
	void setupGUI();
private slots:
	void apply() { exportWidget->apply(mainWin); }
};

#endif //EXPORTDIALOG_H
