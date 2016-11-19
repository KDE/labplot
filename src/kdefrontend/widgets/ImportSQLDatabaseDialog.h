/***************************************************************************
    File                 : ImportFileDialog.h
    Project              : LabPlot
    Description          : import data dialog
    --------------------------------------------------------------------
    Copyright            : (C) 2016 by Ankit Wagadre (wagadre.ankit@gmail.com)

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

#include <KDialog>
#include <QVBoxLayout>

class MainWin;
class QVBoxLayout;
class QStatusBar;
class ImportSQLDatabaseWidget;
class Project;


class ImportSQLDatabaseDialog: public KDialog {
	Q_OBJECT

public:
	explicit ImportSQLDatabaseDialog(MainWin* , Project*);
	~ImportSQLDatabaseDialog();

private:
	MainWin* m_mainWin;
	QVBoxLayout* vLayout;
	QStatusBar* m_statusBar;
	ImportSQLDatabaseWidget* importSQLDatabaseWidget;
};

#endif //IMPORTSQLDATABASEDIALOG_H
