/***************************************************************************
    File                 : SettingsWorksheetPage.h
    Project              : LabPlot
    Description          : settings page for Worksheet
    --------------------------------------------------------------------
    --------------------------------------------------------------------
    Copyright            : (C) 2016-2017 by Alexander Semke (alexander.semke@web.de)

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
