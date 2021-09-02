/*
    File                 : SettingsGeneralPage.h
    Project              : LabPlot
    Description          : general settings page
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2011-2016 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef SETTINGSGENERALPAGE_H
#define SETTINGSGENERALPAGE_H

#include "SettingsPage.h"
#include "ui_settingsgeneralpage.h"

class SettingsGeneralPage : public SettingsPage {
	Q_OBJECT

public:
	enum class DecimalSeparator{Dot, Comma, Arabic, Automatic};

	explicit SettingsGeneralPage(QWidget*);

	static DecimalSeparator decimalSeparator(QLocale locale = QLocale());
	QLocale::Language decimalSeparatorLocale() const;

	void applySettings() override;
	void restoreDefaults() override;

private:
	Ui::SettingsGeneralPage ui;
	bool m_changed{false};

	void loadSettings();
	void retranslateUi();

private slots:
	void interfaceChanged(int);
	void autoSaveChanged(int);
	void changed();

signals:
	void settingsChanged();
};

#endif
