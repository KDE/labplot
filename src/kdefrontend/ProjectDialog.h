/***************************************************************************
    File                 : ProjectDialog.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Stefan Gerlach
    Email (use @ for *)  : stefan.gerlach*uni-konstanz.de
    Description          : dialog for project settings
                           
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

#ifndef PROJECTDIALOG_H
#define PROJECTDIALOG_H

#include <KDialog>
#include <QtGui>
#include "ui_projectdialog.h"
class MainWin;
class Project;

/**
 * @brief Provides a dialog for editing project settings.
 */
class ProjectDialog: public KDialog {
	Q_OBJECT
public:
	ProjectDialog(MainWin *mw);
private:
	Ui::ProjectDialog ui;
	Project *project;
	void setupGUI();
private slots:
	void apply();
};

#endif //PROJECTDIALOG_H
