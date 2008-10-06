/***************************************************************************
    File                 : ProjectConfigPage.h
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2008 Tilman Benkert
    Email (use @ for *)  : thzs*gmx.net
    Description          : Project settings page for preferences dialog.

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
#ifndef PROJECT_CONFIG_PAGE_H
#define PROJECT_CONFIG_PAGE_H

#include "ui_ProjectConfigPage.h"
#include "lib/ConfigPageWidget.h"

//! Helper class for Project
class ProjectConfigPage : public ConfigPageWidget
{
	Q_OBJECT

	public:
		ProjectConfigPage();

	public slots:
		virtual void apply();
	private:
		Ui::ProjectConfigPage ui;
};

#endif // ifndef PROJECT_CONFIG_PAGE_H
