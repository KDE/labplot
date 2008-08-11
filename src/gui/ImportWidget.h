#ifndef IMPORTWIDGET_H
#define IMPORTWIDGET_H

#include <QtGui>
#include "../ui_importwidget.h"
class MainWin;
class Spreadsheet;
#include "../binaryformat.h"

/**
 * @brief Represents the widget where all the importsettings can be modified
 * This widget is embedded in \c ImportDialog
 */
class ImportWidget : public QWidget{
    Q_OBJECT

public:
	ImportWidget(QWidget*);
	~ImportWidget();

	QStringList fileNames() { return ui.leFileName->text().split(";"); }
	void apply(MainWin *mainWin);	// used from ImportDialog
private:
	Ui::ImportWidget ui;
	bool binaryMode;
	void updateBinaryMode();
	int startRow() const;
	int endRow() const;
	void importOPJ(MainWin *mainWin, QString filename);
	int importHDF5(MainWin *mainWin, QString filename, Spreadsheet *s);
	int importNETCDF(QString filename, Spreadsheet *s);
	int importCDF(QString filename, Spreadsheet *s);
	void importASCII(QIODevice *file, Spreadsheet *s);
	void importBinary(QIODevice *file, Spreadsheet *s);
	double getBinaryValue(QDataStream *ds, BinaryFormat type) const;
private slots:
	void save();
	void selectFile();
	void updateFileType();
	void fileInfoDialog();
	void toggleOptions();
};

#endif
