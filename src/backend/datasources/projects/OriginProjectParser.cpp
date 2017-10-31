/***************************************************************************
    File                 : OriginProjectParser.h
    Project              : LabPlot
    Description          : parser for Origin projects
    --------------------------------------------------------------------
    Copyright            : (C) 2017 Alexander Semke (alexander.semke@web.de)

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

#include "backend/datasources/projects/OriginProjectParser.h"
#include "backend/core/Project.h"
#include "backend/core/AspectTreeModel.h"
#include <liborigin/OriginFile.h>

/*!
\class OriginProjectParser
\brief parser for Origin projects.

\ingroup datasources
*/

OriginProjectParser::OriginProjectParser() : ProjectParser() {
	m_topLevelClasses<<"Folder"<<"Workbook"<<"Spreadsheet"<<"Matrix"<<"Worksheet";
}

QAbstractItemModel* OriginProjectParser::model() {
	WAIT_CURSOR;
	if (m_project == nullptr)
		m_project = new Project();

	AspectTreeModel* model = nullptr;
	OriginFile opj((const char*)m_projectFileName.toLocal8Bit());
	//parse the OPJ file and create a Project object for the preview
	bool rc = false;
	//m_project->setName();
	//TODO

	RESET_CURSOR;

	if (rc) {
		model = new AspectTreeModel(m_project);
		model->setReadOnly(true);
	}

	return model;
}

void OriginProjectParser::importTo(Folder* folder) {
	Q_UNUSED(folder);
}
