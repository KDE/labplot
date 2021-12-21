/*
    File                 : SettingsSpreadsheetPage.h
    Project              : LabPlot
    Description          : settings page for Spreadsheet
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2020 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef SETTINGSSPREADSHEETPAGE_H
#define SETTINGSSPREADSHEETPAGE_H

#include "SettingsPage.h"
#include "ui_settingsspreadsheetpage.h"

class ThemesComboBox;

class SettingsSpreadsheetPage : public SettingsPage {
	Q_OBJECT

public:
	explicit SettingsSpreadsheetPage(QWidget*);

	void applySettings() override;
	void restoreDefaults() override;

private:
	Ui::SettingsSpreadsheetPage ui;
	bool m_changed{false};

	void loadSettings();

private Q_SLOTS:
	void changed();

Q_SIGNALS:
	void settingsChanged();
};

#endif
