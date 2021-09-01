/*
    File                 : SettingsPage.h
    Project              : LabPlot
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2008-2014 Alexander Semke (alexander.semke@web.de)
    Description          : base class for all pages in the Settings-Dialog
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SETTINGSPAGE_H
#define SETTINGSPAGE_H

#include <QWidget>

class SettingsPage : public QWidget {
	Q_OBJECT

public:
	explicit SettingsPage(QWidget*) {};
	~SettingsPage() override = default;

	virtual void applySettings() = 0;
	virtual void restoreDefaults() = 0;
};

#endif
