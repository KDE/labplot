/***************************************************************************
    File                 : ImportFileWidget.h
    Project              : LabPlot
    Description          : import file data widget
    --------------------------------------------------------------------
    Copyright            : (C) 2009 by Stefan Gerlach (stefan.gerlach@uni-konstanz.de)
    Copyright            : (C) 2009-2015 Alexander Semke (alexander.semke@web.de)

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
#include "HDFOptionsWidget.h"
#include "ImageOptionsWidget.h"
#include "NetCDFOptionsWidget.h"
#include "FITSOptionsWidget.h"
#include "backend/datasources/FileDataSource.h"

class AbstractFileFilter;
class QTableWidget;

class ImportFileWidget : public QWidget {
	Q_OBJECT

public:
	explicit ImportFileWidget(QWidget*, const QString& fileName = QString());
	~ImportFileWidget();

	void showOptions(bool);
	void saveSettings(FileDataSource*) const;
	FileDataSource::FileType currentFileType() const;
	AbstractFileFilter* currentFileFilter() const;
	QString fileName() const;
	const QStringList selectedHDFNames() const;
	const QStringList selectedNetCDFNames() const;
	const QStringList selectedFITSExtensions() const;
    void hideDataSource();
	void showAsciiHeaderOptions(bool);

    void initializeAndFillPortsAndBaudRates();

private:
    enum SourceType {
        FileOrPipe = 0,
        NetworkSocket,
        LocalSocket,
        SerialPort
    };

	Ui::ImportFileWidget ui;
	Ui::AsciiOptionsWidget asciiOptionsWidget;
	Ui::BinaryOptionsWidget binaryOptionsWidget;
	Ui::HDFOptionsWidget hdfOptionsWidget;
	Ui::ImageOptionsWidget imageOptionsWidget;
	Ui::NetCDFOptionsWidget netcdfOptionsWidget;
	Ui::FITSOptionsWidget fitsOptionsWidget;
	QTableWidget* twPreview;
	const QString& m_fileName;
    bool m_fileDataSource;

private slots:
	void fileNameChanged(const QString&);
	void fileTypeChanged(int);
	void hdfTreeWidgetSelectionChanged();
	void netcdfTreeWidgetSelectionChanged();
	void fitsTreeWidgetSelectionChanged();

    void sourceTypeChanged(int);

	void saveFilter();
	void manageFilters();
	void filterChanged(int);
	void headerChanged(int);
	void selectFile();
	void fileInfoDialog();
	void refreshPreview();
	void loadSettings();

signals:
	void fileNameChanged();
	void checkedFitsTableToMatrix(const bool enable);
};

#endif
