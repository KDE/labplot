/***************************************************************************
    File                 : ImportFileDialog.h
    Project              : LabPlot
    Description          : import data dialog
    --------------------------------------------------------------------
    Copyright            : (C) 2008-2015 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2008-2015 by Stefan Gerlach (stefan.gerlach@uni.kn)

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

#ifndef IMPORTFILEDIALOG_H
#define IMPORTFILEDIALOG_H

#include <KDialog>

class MainWin;
class ImportFileWidget;
class FileDataSource;
class TreeViewComboBox;

class KMenu;
class QStatusBar;
class QAbstractItemModel;
class QModelIndex;
class QVBoxLayout;
class QLabel;
class QComboBox;
class QGroupBox;
class QToolButton;

class ImportFileDialog: public KDialog {
	Q_OBJECT

public:
	explicit ImportFileDialog(MainWin*, bool fileDataSource = false, const QString& fileName = QString());
	~ImportFileDialog();

	void importToFileDataSource(FileDataSource*, QStatusBar*) const;
	void importTo(QStatusBar*) const;
	void setCurrentIndex(const QModelIndex&);
private:
	void setModel(QAbstractItemModel*);

	MainWin* m_mainWin;
	QVBoxLayout* vLayout;
	ImportFileWidget* importFileWidget;
	QGroupBox* frameAddTo;
	TreeViewComboBox* cbAddTo;
	QLabel* lPosition;
	QComboBox* cbPosition;
	QPushButton* bNewSpreadsheet;
	QPushButton* bNewMatrix;
	QPushButton* bNewWorkbook;
	QToolButton* tbNewDataContainer;
	bool m_showOptions;
	KMenu* m_newDataContainerMenu;

private slots:
	void toggleOptions();
	void newDataContainerMenu();
	void newDataContainer(QAction*);
	void checkOkButton();
	void checkOnFitsTableToMatrix(const bool enable);
};

#endif //IMPORTFILEDIALOG_H
