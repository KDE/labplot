//LabPlot : ColumnDialog.h

#ifndef COLUMNDIALOG_H
#define COLUMNDIALOG_H

#include <KLineEdit>
#include <KComboBox>
#include "Dialog.h"

class ColumnDialog: public Dialog
{
	Q_OBJECT
public:
	ColumnDialog(MainWin *mw, Spreadsheet *s);
private:
	Spreadsheet *s;
	KLineEdit *labelle;
	KComboBox *formatcb, *typecb;
	void setupGUI();
private slots:
	void Apply();
};

#endif // COLUMNDIALOG_H
