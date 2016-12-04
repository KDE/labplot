/***************************************************************************
    File                 : ImportDialog.h
    Project              : LabPlot
    Description          : import data dialog
    --------------------------------------------------------------------
    Copyright            : (C) 2016 Alexander Semke (alexander.semke@web.de)

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

#include <KDialog>

class AbstractAspect;
class MainWin;
class TreeViewComboBox;

class KMenu;
class QAbstractItemModel;
class QLabel;
class QModelIndex;
class QVBoxLayout;
class QComboBox;
class QGroupBox;
class QToolButton;
class QStatusBar;

class ImportDialog : public KDialog {
	Q_OBJECT

public:
	explicit ImportDialog(MainWin*);
	~ImportDialog();

	virtual void import(QStatusBar*) const = 0;
	void setCurrentIndex(const QModelIndex&);
	virtual QString selectedObject() const = 0;
	virtual void checkOkButton() = 0;

protected:
	void setModel(QAbstractItemModel*, AbstractAspect*);

	QVBoxLayout* vLayout;
	TreeViewComboBox* cbAddTo;
	QLabel* lPosition;
	QComboBox* cbPosition;

private:
	MainWin* m_mainWin;
	QGroupBox* frameAddTo;
	QToolButton* tbNewDataContainer;
	KMenu* m_newDataContainerMenu;

private slots:
	void newDataContainerMenu();
	void newDataContainer(QAction*);
	void modelIndexChanged();
};

#endif //IMPORTDIALOG_H
