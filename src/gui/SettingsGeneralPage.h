/***************************************************************************
    File                 : SettingsGeneralPage.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Alexander Semke
    Email (use @ for *)  : alexander.semke*web.de
    Description          : general settings page
                           
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
#ifndef SETTINGSGENERALPAGE_H
#define SETTINGSGENERALPAGE_H

#include <QtGui>

#include "SettingsPage.h"
#include "../ui_settingsgeneralpage.h"

class MainWin;

/**
 * @brief Page for the 'General' settings of the Labplot settings dialog.
 *
 */
class SettingsGeneralPage : public SettingsPage{
    Q_OBJECT

public:
    SettingsGeneralPage(MainWin* mainWindow, QWidget* parent);
    virtual ~SettingsGeneralPage();

    /** @see SettingsPageBase::applySettings() */
    virtual void applySettings();

    /** @see SettingsPageBase::restoreDefaults() */
    virtual void restoreDefaults();

private:
	Ui::SettingsGeneralPage ui;
    MainWin* mainWindow;

	void loadSettings();

private slots:
    void autoSaveChanged(int);
};

#endif
