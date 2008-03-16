#include "SettingsGeneralPage.h"
#include "../MainWin.h"

#include <kdialog.h>
#include <kfiledialog.h>
#include <klocale.h>


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
// 	if (state==
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
