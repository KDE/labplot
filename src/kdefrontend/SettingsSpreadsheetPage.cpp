/***************************************************************************
    File                 : SettingsSpreadsheetPage.cpp
    Project              : LabPlot
    Description          : settings page for Spreadsheet
    --------------------------------------------------------------------
    Copyright            : (C) 2020 Alexander Semke (alexander.semke@web.de)

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/

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
