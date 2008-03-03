//LabPlot : FileInfoDialog.h

#ifndef FILEINFODIALOG_H
#define FILEINFODIALOG_H

#include "Dialog.h"

class FileInfoDialog: public Dialog
{
	Q_OBJECT
public:
	FileInfoDialog(MainWin *mw, QString filename);
private slots:
private:
};

#endif //FILEINFODIALOG_H
