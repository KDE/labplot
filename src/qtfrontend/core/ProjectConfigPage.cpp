/***************************************************************************
    File                 : ProjectConfigPage.cpp
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

#include "core/ProjectConfigPage.h"
#include "core/Project.h"

ProjectConfigPage::ProjectConfigPage() 
{
	ui.setupUi(this);
	ui.default_subwindow_visibility_combobox->setCurrentIndex(Project::global("default_mdi_window_visibility").toInt());
	// TODO: set the ui according to the global settings in Project::Private
}

void ProjectConfigPage::apply() 
{
	int index = ui.default_subwindow_visibility_combobox->currentIndex();
	switch (index)
	{
		case 0:
		case 1:
		case 2: 
			Project::setGlobal("default_mdi_window_visibility", index);
			break;
	}
	// TODO: read settings from ui and change them in Project::Private
}

