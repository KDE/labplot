//LabPlot : ImportDialog.h

#ifndef IMPORTDIALOG_H
#define IMPORTDIALOG_H

#include <KLineEdit>
#include <QLabel>
#include <QCheckBox>
#include <QIODevice>
#include <QGroupBox>
#include <KComboBox>
#include "Dialog.h"
#include "binaryformat.h"

/*! Main class for Data Import*/
class ImportDialog: public Dialog
{
	Q_OBJECT
public:
	ImportDialog(MainWin *mw);
private:
	KLineEdit *filele;
	QLabel *filel;
	QCheckBox *newspreadcb, *usetitlecb;
	QGroupBox *optionsgb;
	QCheckBox *binarycb;
	KLineEdit *startle, *endle;
	QCheckBox *simplifycb, *emptycb, *headercb, *samexcb;
	QLabel *separatinglabel, *commentlabel;
	KComboBox *separatingcb, *commentcb;
	QLabel *fieldslabel, *formatlabel, *byteorderlabel;	// binary
	KLineEdit *fieldsle;
	KComboBox *formatcb, *byteordercb;
	void setupGUI();
	int startRow();
	int endRow();
	void importBinary(QIODevice *file, Spreadsheet *s);
	double getBinaryValue(QDataStream *ds, BinaryFormat type);
	void importASCII(QIODevice *file, Spreadsheet *s);
	void importOPJ(QString filename);
	int importHDF5(QString filename, Spreadsheet *s);
	int importNETCDF(QString filename, Spreadsheet *s);
	int importCDF(QString filename, Spreadsheet *s);
private slots:
	void selectFile();
	void updateFileInfo();
	void fileInfoDialog();
	void saveSettings();
	void toggleOptions();
	void toggleBinary(bool checked);
	void Apply();
};
#endif //IMPORTDIALOG_H
