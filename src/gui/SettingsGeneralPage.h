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
