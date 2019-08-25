/***************************************************************************
	File                 : ImportDatasetDialog.h
	Project              : LabPlot
	Description          : import dataset data dialog
	--------------------------------------------------------------------
	Copyright            : (C) 2019 Ferencz Koovacs (kferike98@gmail.com)

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
#ifndef IMPORTDATASETDIALOG_H
#define IMPORTDATASETDIALOG_H

#include "ImportDialog.h"

class MainWin;
class ImportDatasetWidget;
class QMenu;
class DatasetHandler;

class ImportDatasetDialog : public ImportDialog {
    Q_OBJECT

public:
    explicit ImportDatasetDialog(MainWin*, const QString& fileName = QString());
	~ImportDatasetDialog() override;
    QString selectedObject() const override;
	void importToDataset(DatasetHandler*, QStatusBar*) const;
    void importTo(QStatusBar*) const override;

   private:
	ImportDatasetWidget* m_importDatasetWidget;

protected  slots:
		void checkOkButton() override;

};

#endif // IMPORTDATASETDIALOG_H
