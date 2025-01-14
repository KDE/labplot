/*
	File                 : SettingsGeneralPage.h
	Project              : LabPlot
	Description          : general settings page
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2011-2025 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SETTINGSGENERALPAGE_H
#define SETTINGSGENERALPAGE_H

#include "SettingsPage.h"
#include "ui_settingsgeneralpage.h"

#include <QLocale>

class SettingsGeneralPage : public SettingsPage {
	Q_OBJECT

public:
	explicit SettingsGeneralPage(QWidget*, const QLocale&);

	bool applySettings() override;
	void restoreDefaults() override;

	QLocale::Language numberFormat() const;

private:
	Ui::SettingsGeneralPage ui;
	bool m_changed{false};
	QLocale m_defaultSystemLocale;

	void loadSettings();
	void retranslateUi();

private Q_SLOTS:
	void loadOnStartChanged();
	void newProjectChanged();
	void autoSaveChanged(bool);
	void changed();

Q_SIGNALS:
	void settingsChanged();
};

#endif
