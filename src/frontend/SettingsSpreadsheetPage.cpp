/*
	File                 : SettingsSpreadsheetPage.cpp
	Project              : LabPlot
	Description          : settings page for Spreadsheet
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2020-2025 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "SettingsSpreadsheetPage.h"
#include "backend/core/Settings.h"

#include <KConfigGroup>
#include <KLocalizedString>

/**
 * \brief Page for Spreadsheet settings of the Labplot settings dialog.
 */
SettingsSpreadsheetPage::SettingsSpreadsheetPage(QWidget* parent)
	: SettingsPage(parent) {
	ui.setupUi(this);

	loadSettings();

	connect(ui.chkShowColumnType, &QCheckBox::toggled, this, &SettingsSpreadsheetPage::changed);
	connect(ui.chkShowPlotDesignation, &QCheckBox::toggled, this, &SettingsSpreadsheetPage::changed);
}

QList<Settings::Type> SettingsSpreadsheetPage::applySettings() {
	QList<Settings::Type> changes;
	if (!m_changed)
		return changes;

	KConfigGroup group = Settings::group(QStringLiteral("Settings_Spreadsheet"));
	group.writeEntry(QLatin1String("ShowColumnType"), ui.chkShowColumnType->isChecked());
	group.writeEntry(QLatin1String("ShowPlotDesignation"), ui.chkShowPlotDesignation->isChecked());

	changes << Settings::Type::Spreadsheet;
	return changes;
}

void SettingsSpreadsheetPage::restoreDefaults() {
	ui.chkShowColumnType->setChecked(true);
	ui.chkShowPlotDesignation->setChecked(true);
}

void SettingsSpreadsheetPage::loadSettings() {
	const KConfigGroup group = Settings::group(QStringLiteral("Settings_Spreadsheet"));
	ui.chkShowColumnType->setChecked(group.readEntry(QLatin1String("ShowColumnType"), true));
	ui.chkShowPlotDesignation->setChecked(group.readEntry(QLatin1String("ShowPlotDesignation"), true));
}

void SettingsSpreadsheetPage::changed() {
	m_changed = true;
	Q_EMIT settingsChanged();
}
