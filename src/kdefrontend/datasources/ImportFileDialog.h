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

#include "ImportDialog.h"

class AbstractAspect;
class MainWin;
class ImportFileWidget;
class FileDataSource;
class TreeViewComboBox;

class QStatusBar;
class QMenu;

class ImportFileDialog : public ImportDialog {
	Q_OBJECT

public:
	explicit ImportFileDialog(MainWin*, bool fileDataSource = false, const QString& fileName = QString());
	~ImportFileDialog();

	virtual QString selectedObject() const;
	virtual void checkOkButton();

	void importToFileDataSource(FileDataSource*, QStatusBar*) const;
	virtual void importTo(QStatusBar*) const;

private:
	ImportFileWidget* importFileWidget;
	bool m_showOptions;
	QMenu* m_newDataContainerMenu;

private slots:
	void toggleOptions();
	void checkOnFitsTableToMatrix(const bool enable);
};

#endif //IMPORTFILEDIALOG_H
