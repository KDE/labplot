/*
	File                 : SettingsScriptingPage.h
	Project              : LabPlot
	Description          : settings page for Scripting
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2026 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SETTINSSCRIPTINGPAGE_H
#define SETTINSSCRIPTINGPAGE_H

#include "SettingsPage.h"
#include "ui_settingsscriptingpage.h"

class SettingsScriptingPage : public SettingsPage {
	Q_OBJECT

public:
	explicit SettingsScriptingPage(QWidget*);

	QList<Settings::Type> applySettings() override;
	void restoreDefaults() override;

private:
	Ui::SettingsScriptingPage ui;
	void loadSettings();
	void discoverEnvironments();
	void scanForVenvs(const QString& dir);
	void scanForCondaEnvs(const QString& dir);
	void addEnvironment(const QString& envPath);
	void changed();
	bool m_changed{false};
	QString m_pythonMinorVersion; // e.g. "3.11"

Q_SIGNALS:
	void settingsChanged();
};

#endif
