/***************************************************************************
    File                 : SettingsDialog.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Alexander Semke
    Email (use @ for *)  : alexander.semke*web.de
    Description          : general settings dialog
                           
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
#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QtGui>
#include <kpagedialog.h>

class MainWin;
class SettingsGeneralPage;
class SettingsPrintingPage;

/**
 * @brief Settings dialog for Labplot.
 *
 * Contains the pages for general settings and view settings.
 *
 */
class SettingsDialog : public KPageDialog{
    Q_OBJECT

public:
    explicit SettingsDialog(MainWin* mainWindow);
    virtual ~SettingsDialog();

protected slots:
    virtual void slotButtonClicked(int button);

private:
    void applySettings();
    void restoreDefaults();

private:
    SettingsGeneralPage* generalPage;
    SettingsPrintingPage* printingPage;
};

#endif
