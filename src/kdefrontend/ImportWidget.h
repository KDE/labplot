/***************************************************************************
    File                 : ImportWidget.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Stefan Gerlach
    Email (use @ for *)  : stefan.gerlach*uni-konstanz.de
    Description          : import file widget

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
#include "elements/binaryformat.h"
#include "ui_importwidget.h"

class MainWin;
class Spreadsheet;


class ImportWidget : public QWidget{
    Q_OBJECT

public:
	ImportWidget(QWidget*);
	~ImportWidget();

	void apply(MainWin *mainWin);
	void showOptions() const;

private:
	Ui::ImportWidget ui;

	bool binaryMode;
	void updateBinaryMode();
	int startRow() const;
	int endRow() const;
	void importOPJ(MainWin *mainWin, QString filename);
	int importHDF5(MainWin *mainWin, QString filename, Spreadsheet *spreadsheet);
	int importNETCDF(QString filename, Spreadsheet *spreadsheet);
	int importCDF(QString filename, Spreadsheet *spreadsheet);
	void importASCII(QIODevice *file, Spreadsheet *spreadsheet);
	void importBinary(QIODevice *file, Spreadsheet *spreadsheet);
	double getBinaryValue(QDataStream *ds, BinaryFormat type) const;
	QStringList fileNames()const;

private slots:
	void save();
	void selectFile();
	void updateFileType();
	void fileInfoDialog();
	void toggleOptions();
};

#endif
