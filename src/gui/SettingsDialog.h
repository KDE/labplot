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
