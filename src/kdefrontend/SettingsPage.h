/*
	File                 : SettingsPage.h
	Project              : LabPlot
	Description          : base class for all pages in the Settings-Dialog
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2008-2014 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SETTINGSPAGE_H
#define SETTINGSPAGE_H

#include <QWidget>

class KPageWidgetItem;
class KPageDialog;

class SettingsPage : public QWidget {
	Q_OBJECT

public:
	explicit SettingsPage(QWidget*) {};
	~SettingsPage() override = default;

	virtual bool applySettings() = 0;
	virtual void restoreDefaults() = 0;
	virtual void addSubPages(KPageWidgetItem*, KPageDialog*){

	};
};

#endif
