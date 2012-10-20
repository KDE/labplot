/***************************************************************************
    File                 : ImportFileWidget.h
    Project              : LabPlot
    Description          : import file data widget
    --------------------------------------------------------------------
    Copyright            : (C) 2009 by Stefan Gerlach (stefan.gerlach*uni-konstanz.de)
    Copyright            : (C) 2009-2012 Alexander Semke (alexander.semke*web.de)
                           (replace * with @ in the email addresses)

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
#ifndef IMPORTFILEWIDGET_H
#define IMPORTFILEWIDGET_H

#include "ui_importfilewidget.h"
#include "AsciiOptionsWidget.h"
#include "BinaryOptionsWidget.h"

class FileDataSource;
class AbstractFileFilter;

class ImportFileWidget : public QWidget{
    Q_OBJECT

public:
	ImportFileWidget(QWidget*);
	~ImportFileWidget();

 	bool toggleOptions();
	void saveSettings(FileDataSource*) const;
	AbstractFileFilter* currentFileFilter() const;
	QString fileName() const;
	void hideDataSource() const;
	
private:
	Ui::ImportFileWidget ui;
	Ui::AsciiOptionsWidget asciiOptionsWidget;
	Ui::BinaryOptionsWidget binaryOptionsWidget;
	
private slots:
	void fileNameChanged(const QString&);
	void fileTypeChanged(int);
	void saveFilter();
	void manageFilters();
	void filterChanged(int);
	void headerChanged(int);
	void selectFile();
 	void fileInfoDialog();
	void refreshPreview();
};

#endif
