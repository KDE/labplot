/***************************************************************************
    File                 : SettingsGeneral.cc
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
#include "SettingsGeneralPage.h"
#include "MainWin.h"

#include <KDialog>
#include <KLocale>
#include <kfiledialog.h>

SettingsGeneralPage::SettingsGeneralPage(MainWin* main, QWidget* parent) :
    SettingsPage(parent){

	ui.setupUi(this);
    mainWindow=main;
	loadSettings();

	connect(ui.checkBoxAutoSave, SIGNAL(stateChanged(int)), this, SLOT( autoSaveChanged(int)) );
}

SettingsGeneralPage::~SettingsGeneralPage(){
}

void SettingsGeneralPage::autoSaveChanged(int state){
	Q_UNUSED(state);
}

void SettingsGeneralPage::applySettings(){
//     GeneralSettings* settings = DolphinSettings::instance().generalSettings();
//
//     const KUrl url(m_homeUrl->text());
//     KFileItem fileItem(S_IFDIR, KFileItem::Unknown, url);
//     if (url.isValid() && fileItem.isDir()) {
//         settings->setHomeUrl(url.prettyUrl());
//     }
//
//     settings->setSplitView(m_splitView->isChecked());
//     settings->setEditableUrl(m_editableUrl->isChecked());
//     settings->setFilterBar(m_filterBar->isChecked());
//
//     KSharedConfig::Ptr konqConfig = KSharedConfig::openConfig("konquerorrc", KConfig::IncludeGlobals);
//     KConfigGroup trashConfig(konqConfig, "Trash");
//     trashConfig.writeEntry("ConfirmTrash", m_confirmMoveToTrash->isChecked());
//     trashConfig.writeEntry("ConfirmDelete", m_confirmDelete->isChecked());
//     trashConfig.sync();
//
//     KConfigGroup kdeConfig(KGlobal::config(), "KDE");
//     kdeConfig.writeEntry("ShowDeleteCommand", m_showDeleteCommand->isChecked());
//     kdeConfig.sync();
}

void SettingsGeneralPage::restoreDefaults(){
//     GeneralSettings* settings = DolphinSettings::instance().generalSettings();
//     settings->setDefaults();
//
//     // TODO: reset default settings for trash and show delete command...
//
//     loadSettings();
}

void SettingsGeneralPage::loadSettings(){
//     GeneralSettings* settings = DolphinSettings::instance().generalSettings();
//     m_homeUrl->setText(settings->homeUrl());
//     m_splitView->setChecked(settings->splitView());
//     m_editableUrl->setChecked(settings->editableUrl());
//     m_filterBar->setChecked(settings->filterBar());
//
//     KSharedConfig::Ptr konqConfig = KSharedConfig::openConfig("konquerorrc", KConfig::IncludeGlobals);
//     const KConfigGroup trashConfig(konqConfig, "Trash");
//     m_confirmMoveToTrash->setChecked(trashConfig.readEntry("ConfirmTrash", false));
//     m_confirmDelete->setChecked(trashConfig.readEntry("ConfirmDelete", true));
//
//     const KConfigGroup kdeConfig(KGlobal::config(), "KDE");
//     m_showDeleteCommand->setChecked(kdeConfig.readEntry("ShowDeleteCommand", false));
}
