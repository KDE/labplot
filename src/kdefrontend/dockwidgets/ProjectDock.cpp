/***************************************************************************
    File                 : ProjectDock.cpp
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2012 by Stefan Gerlach (stefan.gerlach*uni-konstanz.de)
    							(use @ for *)
    Description          : widget for project properties
                           
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

#include "ProjectDock.h"
#include "../TemplateHandler.h"

/*!
  \class GuiObserver
  \brief  Provides a widget for editing the properties of a project

  \ingroup kdefrontend
*/

ProjectDock::ProjectDock(QWidget *parent): QWidget(parent){
	ui.setupUi(this);

	this->retranslateUi();
}

//************************************************************
//****************** SLOTS ********************************
//************************************************************
void ProjectDock::retranslateUi(){
	m_initializing = true;

	//TODO
	
	m_initializing = false;
}


void ProjectDock::loadConfig(KConfig& config){
//	KConfigGroup group = config.group( "Project" );
	
}

void ProjectDock::saveConfig(KConfig& config){
//	KConfigGroup group = config.group( "Project" );

}
