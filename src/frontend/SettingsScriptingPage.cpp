/*
	File                 : SettingsScriptingPage.cpp
	Project              : LabPlot
	Description          : settings page for Scripting
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2026 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "SettingsScriptingPage.h"

/**
 * \brief Page for the 'General' settings of the Labplot settings dialog.
 */
SettingsScriptingPage::SettingsScriptingPage(QWidget* parent)
	: SettingsPage(parent) {
	ui.setupUi(this);
	loadSettings();
}

QList<Settings::Type> SettingsScriptingPage::applySettings() {
	QList<Settings::Type> changes;
	if (!m_changed)
		return changes;

	return changes;
}

void SettingsScriptingPage::restoreDefaults() {
}

void SettingsScriptingPage::loadSettings() {
}
