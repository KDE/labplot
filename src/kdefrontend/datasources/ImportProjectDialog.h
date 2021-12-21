/*
    File                 : ImportProjectDialog.h
    Project              : LabPlot
    Description          : import project dialog
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2017-2021 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


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
class KUrlComboBox;

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
	KUrlComboBox* m_cbFileName;
	ProjectParser* m_projectParser{nullptr};
	ProjectType m_projectType;
	AspectTreeModel* m_aspectTreeModel;
	TreeViewComboBox* m_cbAddTo;
	QPushButton* m_bNewFolder;
	QDialogButtonBox* m_buttonBox;

	void showTopLevelOnly(const QModelIndex&);
	bool isTopLevel(const AbstractAspect*) const;

private Q_SLOTS:
	void fileNameChanged(const QString&);
	void refreshPreview();
	void selectionChanged(const QItemSelection&, const QItemSelection&);
	void selectFile();
	void newFolder();
};

#endif //IMPORTPROJECTDIALOG_H
