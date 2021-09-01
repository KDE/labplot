/*
    File                 : ImportDialog.h
    Project              : LabPlot
    Description          : import data dialog
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016-2018 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef IMPORTDIALOG_H
#define IMPORTDIALOG_H

#include <QDialog>

class AbstractAspect;
class AspectTreeModel;
class MainWin;
class TreeViewComboBox;

class QMenu;
class QAbstractItemModel;
class QLabel;
class QModelIndex;
class QVBoxLayout;
class QComboBox;
class QGroupBox;
class QToolButton;
class QStatusBar;

class ImportDialog : public QDialog {
	Q_OBJECT

public:
	explicit ImportDialog(MainWin*);
	~ImportDialog() override;

	virtual void importTo(QStatusBar*) const = 0;
	void setCurrentIndex(const QModelIndex&);
	virtual QString selectedObject() const = 0;

protected:
	void setModel();

	QVBoxLayout* vLayout;
	QPushButton* okButton{nullptr};
	QLabel* lPosition{nullptr};
	QComboBox* cbPosition{nullptr};
	TreeViewComboBox* cbAddTo{nullptr};
	MainWin* m_mainWin;
	QGroupBox* frameAddTo{nullptr};
	QToolButton* tbNewDataContainer{nullptr};
	QMenu* m_newDataContainerMenu{nullptr};
	AspectTreeModel* m_aspectTreeModel;

protected slots:
	virtual void checkOkButton() = 0;

private slots:
	void newDataContainerMenu();
	void newDataContainer(QAction*);
};

#endif //IMPORTDIALOG_H
