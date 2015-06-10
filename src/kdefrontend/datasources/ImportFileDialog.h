/***************************************************************************
    File                 : ImportFileDialog.h
    Project              : LabPlot
    Description          : import data dialog
    --------------------------------------------------------------------
    Copyright            : (C) 2008 Alexander Semke (alexander.semke@web.de)
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
#include <memory>
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
class QProgressBar;

class ImportFileDialog: public KDialog {
  Q_OBJECT

  public:
	explicit ImportFileDialog(QWidget*);
	~ImportFileDialog();

	void importToFileDataSource(FileDataSource*, QStatusBar*) const;
	void importTo(QStatusBar*) const;
	void setModel(std::auto_ptr<QAbstractItemModel>);
	void updateModel(std::auto_ptr<QAbstractItemModel>);
	void setCurrentIndex(const QModelIndex&);

  private:
	QVBoxLayout* vLayout;
	ImportFileWidget *importFileWidget;
	QGroupBox* frameAddTo;
	TreeViewComboBox* cbAddTo;
	QLabel* lPosition;
	QComboBox* cbPosition;
	QWidget* mainWidget;
	QPushButton* bNewSpreadsheet;
	QPushButton* bNewMatrix;
	QPushButton* bNewWorkbook;
	QToolButton* tbNewDataContainer;
	std::auto_ptr<QAbstractItemModel> m_model;
	bool m_optionsShown;
	KMenu* m_newDataContainerMenu;

  private slots:
	void toggleOptions();
	void currentAddToIndexChanged(QModelIndex);
	void newDataContainerMenu();
	void newDataContainer(QAction*);

  signals:
	void newSpreadsheetRequested(const QString&);
	void newMatrixRequested(const QString&);
	void newWorkbookRequested(const QString&);
};

#endif //IMPORTFILEDIALOG_H
