//LabPlot : ColumnDialog.h

#ifndef COLUMNDIALOG_H
#define COLUMNDIALOG_H

#include "../ui_columndialog.h"
class MainWin;
class Spreadsheet;

/**
 * @brief Provides a dialog for editing the properties of a spreadsheet column.
 */
class ColumnDialog: public KDialog {
	Q_OBJECT
public:
	ColumnDialog(MainWin *mw, Spreadsheet *s);
private:
	Ui::ColumnDialog ui;
	Spreadsheet *s;
	void setupGUI();
private slots:
	void apply();
};

#endif // COLUMNDIALOG_H
