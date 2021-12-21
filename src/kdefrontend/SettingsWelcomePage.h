/*
    File                 : SettingsWelcomePage.h
    Project              : LabPlot
    Description          : settings page for the welcome screen
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2020 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef SETTINGWELCOMEPAGE_H
#define SETTINGWELCOMEPAGE_H

#include "SettingsPage.h"
#include "ui_settingswelcomepage.h"

class SettingsWelcomePage : public SettingsPage {
	Q_OBJECT

public:
	explicit SettingsWelcomePage(QWidget*);

	void applySettings() override;
	void restoreDefaults() override;

private:
	Ui::SettingsWelcomePage ui;
	void loadSettings();

Q_SIGNALS:
	void settingsChanged();
	void resetWelcomeScreen();
};

#endif
