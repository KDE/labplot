/*
    File                 : SettingsSpreadsheetPage.cpp
    Project              : LabPlot
    Description          : settings page for Spreadsheet
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2020 Alexander Semke (alexander.semke@web.de)
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#include "SettingsSpreadsheetPage.h"

#include <KLocalizedString>
#include <KSharedConfig>
#include <KConfigGroup>

/**
 * \brief Page for Spreadsheet settings of the Labplot settings dialog.
 */
SettingsSpreadsheetPage::SettingsSpreadsheetPage(QWidget* parent) : SettingsPage(parent) {
	ui.setupUi(this);
	connect(ui.chkShowColumnType, &QCheckBox::stateChanged, this, &SettingsSpreadsheetPage::changed);
	connect(ui.chkShowPlotDesignation, &QCheckBox::stateChanged, this, &SettingsSpreadsheetPage::changed);
	loadSettings();
}

void SettingsSpreadsheetPage::applySettings() {
	if (!m_changed)
		return;

	KConfigGroup group = KSharedConfig::openConfig()->group(QLatin1String("Settings_Spreadsheet"));
	group.writeEntry(QLatin1String("ShowColumnType"), ui.chkShowColumnType->isChecked());
	group.writeEntry(QLatin1String("ShowPlotDesignation"), ui.chkShowPlotDesignation->isChecked());
}

void SettingsSpreadsheetPage::restoreDefaults() {
	ui.chkShowColumnType->setChecked(true);
	ui.chkShowPlotDesignation->setChecked(true);
}

void SettingsSpreadsheetPage::loadSettings() {
	const KConfigGroup group = KSharedConfig::openConfig()->group(QLatin1String("Settings_Spreadsheet"));
	ui.chkShowColumnType->setChecked(group.readEntry(QLatin1String("ShowColumnType"), true));
	ui.chkShowPlotDesignation->setChecked(group.readEntry(QLatin1String("ShowPlotDesignation"), true));
}

void SettingsSpreadsheetPage::changed() {
	m_changed = true;
	emit settingsChanged();
}
