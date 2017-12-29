/***************************************************************************
    File                 : ProjectParser.h
    Project              : LabPlot
    Description          : base class for project parsers
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
#include "ProjectParser.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/core/Project.h"

#include <KLocale>

/*!
\class ProjectParser
\brief  base class for project parsers

\ingroup datasources
*/
ProjectParser::ProjectParser() : QObject(), m_project(nullptr) {

}

ProjectParser::~ProjectParser() {
	if (m_project != nullptr)
		delete m_project;
}

void ProjectParser::setProjectFileName(const QString& name) {
	m_projectFileName = name;

	//delete the previous project object
	if (m_project) {
		delete m_project;
		m_project = nullptr;
	}
}

const QString& ProjectParser::projectFileName() const {
	return m_projectFileName;
}

QList<const char*> ProjectParser::topLevelClasses() const {
	return m_topLevelClasses;
}

QAbstractItemModel* ProjectParser::model() {
	WAIT_CURSOR;
	if (m_project == nullptr)
		m_project = new Project();

	AspectTreeModel* model = nullptr;
	bool rc = load(m_project, true);
	if (rc) {
		model = new AspectTreeModel(m_project);
		model->setReadOnly(true);
	}

	RESET_CURSOR;
	return model;
}

void ProjectParser::importTo(Folder* folder, const QStringList& selectedPathes) {
	QDEBUG("Starting the import of " + m_projectFileName);

	//import the selected objects into a temporary project
	Project* project = new Project();
	project->setPathesToLoad(selectedPathes);
	load(project, false);

	//move all children from the temp project to the target folder
	folder->beginMacro(i18n("%1: Import from %2").arg(folder->name()).arg(m_projectFileName));
	for (auto* child : project->children<AbstractAspect>()) {
		project->removeChild(child);
		folder->addChild(child);
	}
	folder->endMacro();
	delete project;

	QDEBUG("Import of " + m_projectFileName + " done.");
}
