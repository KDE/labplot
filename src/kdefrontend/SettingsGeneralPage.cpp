/***************************************************************************
    File                 : SettingsGeneral.cc
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008-2013 by Alexander Semke
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

/**
 * \brief Page for the 'General' settings of the Labplot settings dialog.
 *
 */
SettingsGeneralPage::SettingsGeneralPage(QWidget* parent) :
    SettingsPage(parent), m_changed(false) {

	ui.setupUi(this);
	retranslateUi();

	loadSettings();

	connect(ui.cbLoadOnStart, SIGNAL(currentIndexChanged(int)), this, SLOT( changed()) );
	connect(ui.cbInterface, SIGNAL(currentIndexChanged(int)), this, SLOT( interfaceChanged(int)) );
	connect(ui.chkAutoSave, SIGNAL(stateChanged(int)), this, SLOT( changed()) );
	connect(ui.sbAutoSaveInterval, SIGNAL(valueChanged(int)), this, SLOT( changed()) );
	
	interfaceChanged(0);
}


/** @see SettingsPageBase::applySettings() */
void SettingsGeneralPage::applySettings(){
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

/** @see SettingsPageBase::restoreDefaults() */
void SettingsGeneralPage::restoreDefaults(){
//     GeneralSettings* settings = DolphinSettings::instance().generalSettings();
//     settings->setDefaults();
//
//     // TODO: reset default settings for trash and show delete command...
//
//     loadSettings();
}

void SettingsGeneralPage::loadSettings(){
//     KSharedConfig::Ptr konqConfig = KSharedConfig::openConfig("konquerorrc", KConfig::IncludeGlobals);
//     const KConfigGroup trashConfig(konqConfig, "Trash");
//     m_confirmMoveToTrash->setChecked(trashConfig.readEntry("ConfirmTrash", false));
//     m_confirmDelete->setChecked(trashConfig.readEntry("ConfirmDelete", true));
// 
//     const KConfigGroup kdeConfig(KGlobal::config(), "KDE");
//     m_showDeleteCommand->setChecked(kdeConfig.readEntry("ShowDeleteCommand", false));
}

void SettingsGeneralPage::retranslateUi() {
	ui.cbLoadOnStart->clear();
	ui.cbLoadOnStart->addItem(i18n("Do nothing"));
	ui.cbLoadOnStart->addItem(i18n("Create new empty project"));
	ui.cbLoadOnStart->addItem(i18n("Create new project with worksheet"));
	ui.cbLoadOnStart->addItem(i18n("Load last used project"));
	
	ui.cbInterface->clear();
	ui.cbInterface->addItem(i18n("Sub-window view"));
	ui.cbInterface->addItem(i18n("Tabbed view"));
	
	ui.cbMdiVisibility->clear();
	ui.cbMdiVisibility->addItem(i18n("Show windows of the current folder only"));
	ui.cbMdiVisibility->addItem(i18n("Show windows of the current folder and its subfolders only"));
	ui.cbMdiVisibility->addItem(i18n("Show all windows"));

	ui.cbTabPosition->clear();
	ui.cbTabPosition->addItem(i18n("Top"));
	ui.cbTabPosition->addItem(i18n("Bottom"));
	ui.cbTabPosition->addItem(i18n("Left"));
	ui.cbTabPosition->addItem(i18n("Right"));
}

void SettingsGeneralPage::changed() {
	m_changed = true;
}

void SettingsGeneralPage::interfaceChanged(int index) {
	bool tabbedView = (index==1);
	ui.lTabPosition->setVisible(tabbedView);
	ui.cbTabPosition->setVisible(tabbedView);
	ui.lMdiVisibility->setVisible(!tabbedView);
	ui.cbMdiVisibility->setVisible(!tabbedView);
	
	changed();
}
