/***************************************************************************
    File                 : SettingsGeneralPage.cpp
    Project              : LabPlot
    Description          : general settings page
    --------------------------------------------------------------------
    Copyright            : (C) 2008-2018 Alexander Semke (alexander.semke@web.de)

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

#include "SettingsGeneralPage.h"

#include <KI18n/KLocalizedString>
#include <KConfigGroup>
#include <KSharedConfig>

/**
 * \brief Page for the 'General' settings of the Labplot settings dialog.
 */
SettingsGeneralPage::SettingsGeneralPage(QWidget* parent) : SettingsPage(parent) {
	ui.setupUi(this);
	ui.sbAutoSaveInterval->setSuffix(i18n("min."));
	retranslateUi();

	connect(ui.cbLoadOnStart, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			this, &SettingsGeneralPage::changed);
	connect(ui.cbInterface, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			this, &SettingsGeneralPage::interfaceChanged);
	connect(ui.cbMdiVisibility, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			this, &SettingsGeneralPage::changed);
	connect(ui.cbTabPosition, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			this, &SettingsGeneralPage::changed);
	connect(ui.chkAutoSave, &QCheckBox::stateChanged, this, &SettingsGeneralPage::autoSaveChanged);
	connect(ui.chkMemoryInfo, &QCheckBox::stateChanged, this, &SettingsGeneralPage::changed);
	connect(ui.chkWelcomeScreen, &QCheckBox::stateChanged, this, &SettingsGeneralPage::changed);

	loadSettings();
	interfaceChanged(ui.cbInterface->currentIndex());
	autoSaveChanged(ui.chkAutoSave->checkState());
}

void SettingsGeneralPage::applySettings() {
	KConfigGroup group = KSharedConfig::openConfig()->group(QLatin1String("Settings_General"));
	group.writeEntry(QLatin1String("LoadOnStart"), ui.cbLoadOnStart->currentIndex());
	group.writeEntry(QLatin1String("ViewMode"), ui.cbInterface->currentIndex());
	group.writeEntry(QLatin1String("TabPosition"), ui.cbTabPosition->currentIndex());
	group.writeEntry(QLatin1String("MdiWindowVisibility"), ui.cbMdiVisibility->currentIndex());
	group.writeEntry(QLatin1String("AutoSave"), ui.chkAutoSave->isChecked());
	group.writeEntry(QLatin1String("AutoSaveInterval"), ui.sbAutoSaveInterval->value());
	group.writeEntry(QLatin1String("ShowMemoryInfo"), ui.chkMemoryInfo->isChecked());
	group.writeEntry(QLatin1String("ShowWelcomeScreen"), ui.chkWelcomeScreen->isChecked());
}

void SettingsGeneralPage::restoreDefaults() {
	loadSettings();
}

void SettingsGeneralPage::loadSettings() {
	const KConfigGroup group = KSharedConfig::openConfig()->group(QLatin1String("Settings_General"));
	ui.cbLoadOnStart->setCurrentIndex(group.readEntry(QLatin1String("LoadOnStart"), 0));
	ui.cbInterface->setCurrentIndex(group.readEntry(QLatin1String("ViewMode"), 0));
	ui.cbTabPosition->setCurrentIndex(group.readEntry(QLatin1String("TabPosition"), 0));
	ui.cbMdiVisibility->setCurrentIndex(group.readEntry(QLatin1String("MdiWindowVisibility"), 0));
	ui.chkAutoSave->setChecked(group.readEntry<bool>(QLatin1String("AutoSave"), false));
	ui.sbAutoSaveInterval->setValue(group.readEntry(QLatin1String("AutoSaveInterval"), 0));
	ui.chkMemoryInfo->setChecked(group.readEntry<bool>(QLatin1String("ShowMemoryInfo"), true));
	ui.chkWelcomeScreen->setChecked(group.readEntry<bool>(QLatin1String("ShowWelcomeScreen"), true));
}

void SettingsGeneralPage::retranslateUi() {
	ui.cbLoadOnStart->clear();
	ui.cbLoadOnStart->addItem(i18n("Do nothing"));
	ui.cbLoadOnStart->addItem(i18n("Create new empty project"));
	ui.cbLoadOnStart->addItem(i18n("Create new project with worksheet"));
	ui.cbLoadOnStart->addItem(i18n("Load last used project"));

	ui.cbInterface->clear();
	ui.cbInterface->addItem(i18n("Sub-window view"));
	ui.cbInterface->addItem(i18n("Tabbed view"));

	ui.cbMdiVisibility->clear();
	ui.cbMdiVisibility->addItem(i18n("Show windows of the current folder only"));
	ui.cbMdiVisibility->addItem(i18n("Show windows of the current folder and its subfolders only"));
	ui.cbMdiVisibility->addItem(i18n("Show all windows"));

	ui.cbTabPosition->clear();
	ui.cbTabPosition->addItem(i18n("Top"));
	ui.cbTabPosition->addItem(i18n("Bottom"));
	ui.cbTabPosition->addItem(i18n("Left"));
	ui.cbTabPosition->addItem(i18n("Right"));
}

void SettingsGeneralPage::changed() {
	m_changed = true;
	emit settingsChanged();
}

void SettingsGeneralPage::interfaceChanged(int index) {
	bool tabbedView = (index == 1);
	ui.lTabPosition->setVisible(tabbedView);
	ui.cbTabPosition->setVisible(tabbedView);
	ui.lMdiVisibility->setVisible(!tabbedView);
	ui.cbMdiVisibility->setVisible(!tabbedView);
	changed();
}

void SettingsGeneralPage::autoSaveChanged(int state) {
	const bool visible = (state == Qt::Checked);
	ui.lAutoSaveInterval->setVisible(visible);
	ui.sbAutoSaveInterval->setVisible(visible);
	changed();
}
