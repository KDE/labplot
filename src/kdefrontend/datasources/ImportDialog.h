/***************************************************************************
    File                 : ImportDialog.h
    Project              : LabPlot
    Description          : import data dialog
    --------------------------------------------------------------------
    Copyright            : (C) 2016-2018 Alexander Semke (alexander.semke@web.de)

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
