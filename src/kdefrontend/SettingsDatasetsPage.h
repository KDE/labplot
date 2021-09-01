/*
    File                 : SettingsDatasetsPage.h
    Project              : LabPlot
    Description          : settings page for Datasets
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2019 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef SETTINGDATASETSPAGE_H
#define SETTINGDATASETSPAGE_H

#include "SettingsPage.h"
#include "ui_settingsdatasetspage.h"

class SettingsDatasetsPage : public SettingsPage {
	Q_OBJECT

public:
	explicit SettingsDatasetsPage(QWidget*);

	void applySettings() override;
	void restoreDefaults() override;

private:
	Ui::SettingsDatasetsPage ui;
	void loadSettings();

private slots:
	void clearCache();
};

#endif
