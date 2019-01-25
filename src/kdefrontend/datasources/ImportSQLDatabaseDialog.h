/***************************************************************************
    File                 : ImportSQLDatabaseDialog.h
    Project              : LabPlot
    Description          : import SQL database dialog
    --------------------------------------------------------------------
    Copyright            : (C) 2016 by Ankit Wagadre (wagadre.ankit@gmail.com)
    Copyright            : (C) 2016-2019 Alexander Semke (alexander.semke@web.de)

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

#ifndef IMPORTSQLDATABASEDIALOG_H
#define IMPORTSQLDATABASEDIALOG_H

#include "kdefrontend/datasources/ImportDialog.h"

class MainWin;
class QStatusBar;
class ImportSQLDatabaseWidget;

class ImportSQLDatabaseDialog : public ImportDialog {
	Q_OBJECT

public:
	explicit ImportSQLDatabaseDialog(MainWin*);
	~ImportSQLDatabaseDialog() override;

	void importTo(QStatusBar*) const override;
	QString selectedObject() const override;

private:
	ImportSQLDatabaseWidget* importSQLDatabaseWidget;

protected  slots:
	void checkOkButton() override;
};

#endif //IMPORTSQLDATABASEDIALOG_H
