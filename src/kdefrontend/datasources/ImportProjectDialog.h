/***************************************************************************
    File                 : ImportProjectDialog.h
    Project              : LabPlot
    Description          : import project dialog
    --------------------------------------------------------------------
    Copyright            : (C) 2017-2019 Alexander Semke (alexander.semke@web.de)

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

#ifndef IMPORTPROJECTDIALOG_H
#define IMPORTPROJECTDIALOG_H

#include <QDialog>
#include "ui_importprojectwidget.h"

class AbstractAspect;
class AspectTreeModel;
class Folder;
class ProjectParser;
class TreeViewComboBox;
class MainWin;
class QDialogButtonBox;
class QStatusBar;

class ImportProjectDialog : public QDialog {
	Q_OBJECT

public:
	enum class ProjectType {LabPlot, Origin};

	explicit ImportProjectDialog(MainWin*, ProjectType);
	~ImportProjectDialog() override;

	void setCurrentFolder(const Folder*);
	void importTo(QStatusBar*) const;

private:
	Ui::ImportProjectWidget ui;
	MainWin* m_mainWin;
	ProjectParser* m_projectParser{nullptr};
	ProjectType m_projectType;
	AspectTreeModel* m_aspectTreeModel;
	TreeViewComboBox* m_cbAddTo;
	QPushButton* m_bNewFolder;
	QDialogButtonBox* m_buttonBox;

	void showTopLevelOnly(const QModelIndex&);
	bool isTopLevel(const AbstractAspect*) const;

private slots:
	void fileNameChanged(const QString&);
	void refreshPreview();
	void selectionChanged(const QItemSelection&, const QItemSelection&);
	void selectFile();
	void newFolder();
};

#endif //IMPORTPROJECTDIALOG_H
