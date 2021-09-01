/*
    File                 : SettingsWorksheetPage.h
    Project              : LabPlot
    Description          : settings page for Worksheet
    --------------------------------------------------------------------
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016-2017 Alexander Semke (alexander.semke@web.de)

*/

/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#ifndef SETTINGSWORKSHEETPAGE_H
#define SETTINGSWORKSHEETPAGE_H

#include "SettingsPage.h"
#include "ui_settingsworksheetpage.h"

class ThemesComboBox;

class SettingsWorksheetPage : public SettingsPage {
	Q_OBJECT

public:
	explicit SettingsWorksheetPage(QWidget*);

	void applySettings() override;
	void restoreDefaults() override;

private:
	Ui::SettingsWorksheetPage ui;
	ThemesComboBox* m_cbThemes;
	bool m_changed{false};

	void loadSettings();

private slots:
	void changed();
	void checkTeX(int);

signals:
	void settingsChanged();
};

#endif
