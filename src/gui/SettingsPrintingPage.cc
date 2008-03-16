#include "SettingsPrintingPage.h"
#include "../MainWin.h"

SettingsPrintingPage::SettingsPrintingPage(MainWin* main, QWidget* parent)
	:SettingsPage(parent){

	ui.setupUi(this);
//     mainWindow=main;
	loadSettings();
}


SettingsPrintingPage::~SettingsPrintingPage(){
}

void SettingsPrintingPage::applySettings(){
	/*
    const KUrl& url = m_mainWindow->activeViewContainer()->url();
    ViewProperties props(url);  // read current view properties

    const bool useGlobalProps = m_globalProps->isChecked();

    GeneralSettings* settings = DolphinSettings::instance().generalSettings();
    settings->setGlobalViewProps(useGlobalProps);

    if (useGlobalProps) {
        // Remember the global view properties by applying the current view properties.
        // It is important that GeneralSettings::globalViewProps() is set before
        // the class ViewProperties is used, as ViewProperties uses this setting
        // to find the destination folder for storing the view properties.
        ViewProperties globalProps(url);
        globalProps.setDirProperties(props);
    }

    KConfigGroup globalConfig(KGlobal::config(), "PreviewSettings");
    const int byteCount = m_maxPreviewSize->value() * 1024 * 1024; // value() returns size in MB
    globalConfig.writeEntry("MaximumSize",
                            byteCount,
                            KConfigBase::Normal | KConfigBase::Global);
    globalConfig.writeEntry("UseFileThumbnails",
                            m_useFileThumbnails->isChecked(),
                            KConfigBase::Normal | KConfigBase::Global);
    globalConfig.sync();
	*/
}

void SettingsPrintingPage::restoreDefaults(){
//     GeneralSettings* settings = DolphinSettings::instance().generalSettings();
//     settings->setDefaults();
//     loadSettings();
}

void SettingsPrintingPage::loadSettings(){
//     GeneralSettings* settings = DolphinSettings::instance().generalSettings();
//     if (settings->globalViewProps()) {
//         m_globalProps->setChecked(true);
//     } else {
//         m_localProps->setChecked(true);
//     }
//
//     const int min = 1;   // MB
//     const int max = 100; // MB
//     m_maxPreviewSize->setRange(min, max);
//     m_maxPreviewSize->setPageStep(10);
//     m_maxPreviewSize->setSingleStep(1);
//     m_maxPreviewSize->setTickPosition(QSlider::TicksBelow);
//
//     KConfigGroup globalConfig(KGlobal::config(), "PreviewSettings");
//     const int maxByteSize = globalConfig.readEntry("MaximumSize", 5 * 1024 * 1024 /* 5 MB */);
//     int maxMByteSize = maxByteSize / (1024 * 1024);
//     if (maxMByteSize < 1) {
//         maxMByteSize = 1;
//     } else if (maxMByteSize > max) {
//         maxMByteSize = max;
//     }
//
//     m_spinBox->setRange(min, max);
//     m_spinBox->setSingleStep(1);
//     m_spinBox->setSuffix(" MB");
//
//     m_maxPreviewSize->setValue(maxMByteSize);
//     m_spinBox->setValue(m_maxPreviewSize->value());
//
//     const bool useFileThumbnails = globalConfig.readEntry("UseFileThumbnails", true);
//     m_useFileThumbnails->setChecked(useFileThumbnails);
}
