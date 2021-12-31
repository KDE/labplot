/*
    File                 : SettingsNotebookPage.h
    Project              : LabPlot
    Description          : Settings page for Notebook
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2021 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef SETTINGSNOTEBOOKPAGE_H
#define SETTINGSNOTEBOOKPAGE_H

#include "SettingsPage.h"
#include "ui_settingsnotebookpage.h"

class SettingsNotebookPage : public SettingsPage {
	Q_OBJECT

public:
	explicit SettingsNotebookPage(QWidget*);

	void applySettings() override;
	void restoreDefaults() override;

private:
	Ui::SettingsNotebookPage ui;
	bool m_changed{false};

	void loadSettings();

private Q_SLOTS:
	void changed();

Q_SIGNALS:
	void settingsChanged();
};

#endif
