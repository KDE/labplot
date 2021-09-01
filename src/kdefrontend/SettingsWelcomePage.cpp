/*
    File                 : SettingsWelcomePage.cpp
    Project              : LabPlot
    Description          : settings page for the welcome screen
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2020 Alexander Semke (alexander.semke@web.de)
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#include "SettingsWelcomePage.h"

/**
 * \brief Page for the 'General' settings of the Labplot settings dialog.
 */
SettingsWelcomePage::SettingsWelcomePage(QWidget* parent) : SettingsPage(parent) {
	ui.setupUi(this);

	connect(ui.bResetLayout, &QPushButton::clicked, this, &SettingsWelcomePage::resetWelcomeScreen);

	loadSettings();
}

void SettingsWelcomePage::applySettings() {

}

void SettingsWelcomePage::restoreDefaults() {
}

void SettingsWelcomePage::loadSettings() {

}
