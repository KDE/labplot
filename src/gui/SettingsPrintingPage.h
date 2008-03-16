#ifndef SETTINGSPRINTINGPAGE_H
#define SETTINGSPRINTINGPAGE_H

#include <QtGui>

#include "SettingsPage.h"
#include "../ui_settingsprintingpage.h"

class MainWin;

/**
 * @brief Represents the page from the Labplot Settings which allows
 * to modify general settings for the things concerning printing.
 */
class SettingsPrintingPage : public SettingsPage{
    Q_OBJECT

public:
    SettingsPrintingPage(MainWin* mainWindow, QWidget* parent);
    virtual ~SettingsPrintingPage();

	void applySettings();
    void restoreDefaults();

private:
    void loadSettings();
	Ui::SettingsPrintingPage ui;
};

#endif
