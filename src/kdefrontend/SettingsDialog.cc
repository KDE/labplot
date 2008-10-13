/***************************************************************************
    File                 : SettingsDialog.cc
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
#include "SettingsDialog.h"

#include "../MainWin.h"
#include "SettingsGeneralPage.h"
#include "SettingsPrintingPage.h"

#include <KLocale>
#include <kmessagebox.h>
#include <KIcon>

SettingsDialog::SettingsDialog(MainWin* mainWindow) :
    KPageDialog(mainWindow){

    const QSize minSize = minimumSize();
    setMinimumSize(QSize(512, minSize.height()));

    setFaceType(List);
    setCaption(i18nc("@title:window", "Dolphin Preferences"));
    setButtons(Ok | Apply | Cancel | Default);
    setDefaultButton(Ok);

    generalPage = new SettingsGeneralPage(mainWindow, this);
    KPageWidgetItem* generalFrame = addPage(generalPage,
                                                    i18nc("@title:group", "General"));
    generalFrame->setIcon(KIcon("system-run"));

    printingPage = new SettingsPrintingPage(mainWindow, this);
    KPageWidgetItem* printingFrame = addPage(printingPage,
                                                 i18nc("@title:group", "View Modes"));
    printingFrame->setIcon(KIcon("document-print"));

    const KConfigGroup dialogConfig(KSharedConfig::openConfig("labplotrc"), "SettingsDialog");
    restoreDialogSize(dialogConfig);
}

SettingsDialog::~SettingsDialog(){
    KConfigGroup dialogConfig(KSharedConfig::openConfig("labplotrc"), "SettingsDialog");
    saveDialogSize(dialogConfig);
}

void SettingsDialog::slotButtonClicked(int button){
    if ((button == Ok) || (button == Apply)) {
        applySettings();
    } else if (button == Default) {
        const QString text(i18nc("@info", "All settings will be reset to default values. Do you want to continue?"));
        if (KMessageBox::questionYesNo(this, text) == KMessageBox::Yes) {
            restoreDefaults();
        }
    }

    KPageDialog::slotButtonClicked(button);
}

void SettingsDialog::applySettings(){
//     generalPage->applySettings();
//     printingPage->applySettings();
	//TODO
//     LabplotApplication::app()->refreshMainWindows();
}

void SettingsDialog::restoreDefaults(){
//     generalPage->restoreDefaults();
//     printingPage->restoreDefaults();
	//TODO
//     LabplotApplication::app()->refreshMainWindows();
}

// #include "moc_SettingsDialog.cpp"
