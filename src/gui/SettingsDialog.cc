#include "SettingsDialog.h"

#include "../MainWin.h"
#include "SettingsGeneralPage.h"
#include "SettingsPrintingPage.h"

#include <klocale.h>
#include <kmessagebox.h>
#include <kicon.h>

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
