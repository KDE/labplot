#ifndef EXPORTWIDGET_H
#define EXPORTWIDGET_H

#include <QtGui>
#include "../ui_exportwidget.h"
class MainWin;
class Spreadsheet;
//#include "../binaryformat.h"

/**
 * @brief Represents the widget where all the export settings can be modified
 * This widget is embedded in \c ExportDialog
 */
class ExportWidget : public QWidget{
    Q_OBJECT

public:
	ExportWidget(QWidget*);
	~ExportWidget();

	void apply(MainWin *mainWin);	// used from ExportDialog
private:
	Ui::ExportWidget ui;
//	bool binaryMode;
//	void updateBinaryMode();
//	int startRow() const;
//	int endRow() const;
/*	void importOPJ(MainWin *mainWin, QString filename);
	int importHDF5(MainWin *mainWin, QString filename, Spreadsheet *s);
	int importNETCDF(QString filename, Spreadsheet *s);
	int importCDF(QString filename, Spreadsheet *s);
*/
//	void importASCII(QIODevice *file, Spreadsheet *s);
	void exportASCII(QTextStream *t);
//	void importBinary(QIODevice *file, Spreadsheet *s);
//	double getBinaryValue(QDataStream *ds, BinaryFormat type) const;
private slots:
	void save();
	void selectFile();
	void updateSelectedFormat();
//	void fileInfoDialog();
//	void toggleOptions();
};

#endif // EXPORTWIDGET
