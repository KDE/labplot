/*
	File                 : ImportDialog.h
	Project              : LabPlot
	Description          : import data dialog
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2016-2023 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef IMPORTDIALOG_H
#define IMPORTDIALOG_H

#include <QDialog>

class AbstractAspect;
class AspectTreeModel;
class MainWin;
class TreeViewComboBox;

class KMessageWidget;
class QMenu;
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

	virtual bool importTo(QStatusBar*) const = 0;
	void setCurrentIndex(const QModelIndex&);
	virtual QString selectedObject() const = 0;

protected:
	void setModel();

	QVBoxLayout* vLayout{nullptr};
	QPushButton* okButton{nullptr};
	QLabel* lPosition{nullptr};
	QComboBox* cbPosition{nullptr};
	TreeViewComboBox* cbAddTo{nullptr};
	MainWin* m_mainWin;
	QGroupBox* frameAddTo{nullptr};
	QToolButton* tbNewDataContainer{nullptr};
	QMenu* m_newDataContainerMenu{nullptr};
	AspectTreeModel* m_aspectTreeModel;
	KMessageWidget* m_messageWidget{nullptr};
	bool m_liveDataSource{false};

public Q_SLOTS:
	void accept() override;

protected Q_SLOTS:
	virtual void checkOkButton() = 0;
	void showErrorMessage(const QString&);

private Q_SLOTS:
	void newDataContainerMenu();
	void newDataContainer(QAction*);
	void currentModelIndexChanged(const QModelIndex&);

Q_SIGNALS:
	void dataContainerChanged(AbstractAspect*);
};

#endif // IMPORTDIALOG_H
