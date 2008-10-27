/***************************************************************************
    File                 : ImportWidget.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Stefan Gerlach
    Email (use @ for *)  : stefan.gerlach*uni-konstanz.de
    Description          : import data widget
                           
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/
#ifndef IMPORTWIDGET_H
#define IMPORTWIDGET_H

#include <QtGui>
#include "ui_importwidget.h"
class MainWin;
class Spreadsheet;
class Table;
#include "elements/binaryformat.h"

/**
 * @brief Represents the widget where all the import settings can be modified
 * This widget is embedded in \c ImportDialog
 */
class ImportWidget : public QWidget{
    Q_OBJECT

public:
	ImportWidget(QWidget*);
	~ImportWidget();

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
	QStringList fileNames() { return ui.leFileName->text().split(";"); }
private slots:
	void save();
	void selectFile();
	void updateFileType();
	void fileInfoDialog();
	void toggleOptions();
};

#endif
