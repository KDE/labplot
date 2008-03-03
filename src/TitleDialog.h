//LabPlot : TitleDialog.h

#ifndef TITLEDIALOG_H
#define TITLEDIALOG_H

#include "Label.h"
#include "Dialog.h"

class TitleDialog: public Dialog
{
	Q_OBJECT
public:
	TitleDialog(MainWin *mw, Label *title=0);
private:
	Label *title;
private slots:
	void saveSettings();
	void Apply();
};

#endif //TITLEDIALOG_H
