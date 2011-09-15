#ifndef EXPORTWIDGET_H
#define EXPORTWIDGET_H

#include <QtGui>
#include "ui_exportwidget.h"
class MainWin;
class SpreadsheetView;
//#include "elements/binaryformat.h"

/**
 * @brief Represents the widget where all the export settings can be modified
 * This widget is embedded in \c ExportDialog
 */
class ExportWidget : public QWidget{
    Q_OBJECT

public:
	ExportWidget(QWidget*);
	~ExportWidget();

	void apply();	// used from ExportDialog
private:
	Ui::ExportWidget ui;
//	bool binaryMode;
//	void updateBinaryMode();
//	int startRow() const;
//	int endRow() const;
/*	void importOPJ(MainWin *mainWin, QString filename);
	int importHDF5(MainWin *mainWin, QString filename, SpreadsheetView *s);
	int importNETCDF(QString filename, SpreadsheetView *s);
	int importCDF(QString filename, SpreadsheetView *s);
*/
//	void importASCII(QIODevice *file, SpreadsheetView *s);
	void exportASCII(QTextStream *t);
//	void importBinary(QIODevice *file, SpreadsheetView *s);
//	double getBinaryValue(QDataStream *ds, BinaryFormat type) const;
private slots:
	void save();
	void selectFile();
	void updateSelectedFormat();
//	void fileInfoDialog();
//	void toggleOptions();
};

#endif // EXPORTWIDGET
