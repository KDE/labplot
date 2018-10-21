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
#include "backend/core/column/Column.h"
#include "backend/core/Project.h"
#include "backend/lib/trace.h"

#include <KLocalizedString>

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
	PERFTRACE("project model for preview created");
	if (!m_project)
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

void ProjectParser::importTo(Folder* targetFolder, const QStringList& selectedPathes) {
	QDEBUG("Starting the import of " + m_projectFileName);

	//import the selected objects into a temporary project
	auto* project = new Project();
	project->setPathesToLoad(selectedPathes);
	load(project, false);

	//determine the first child of the last top level child in the list of the imported objects
	//we want to navigate to in the project explorer after the import
	auto* lastTopLevelChild = project->child<AbstractAspect>(project->childCount<AbstractAspect>()-1);
	AbstractAspect* childToNavigate = nullptr;
	if (lastTopLevelChild->childCount<AbstractAspect>() > 0) {
		childToNavigate = lastTopLevelChild->child<AbstractAspect>(0);

		//we don't want to select columns, select rather their parent spreadsheet
		if (dynamic_cast<const Column*>(childToNavigate))
			childToNavigate = lastTopLevelChild;
	} else {
		childToNavigate = lastTopLevelChild;
	}

	//move all children from the temp project to the target folder
	targetFolder->beginMacro(i18n("%1: Import from %2", targetFolder->name(), m_projectFileName));
	for (auto* child : project->children<AbstractAspect>()) {
		auto* folder = dynamic_cast<Folder*>(child);
		if (folder) {
			moveFolder(targetFolder, folder);
		} else {
			project->removeChild(child);

			//remove the object to be imported in the target folder if it's already existing
			auto* targetChild = targetFolder->child<AbstractAspect>(child->name());
			if (targetChild)
				targetFolder->removeChild(targetChild);

			targetFolder->addChild(child);
		}
	}
	targetFolder->endMacro();

	delete project;

	targetFolder->project()->navigateTo(childToNavigate->path());

	QDEBUG("Import of " + m_projectFileName + " done.");
}

/*
 * moved \c sourceChildFolderToMove from its parten folder to \c targetParentFolder
 * keeping (not overwriting ) the sub-folder structure.
 */
void ProjectParser::moveFolder(Folder* targetParentFolder, Folder* sourceChildFolderToMove) const {
	auto* targetChildFolder = targetParentFolder->child<Folder>(sourceChildFolderToMove->name());
	if (targetChildFolder) {
		//folder exists already in the target parent folder,
		//-> recursively move its children from source into target parent folder
		for (auto* child : sourceChildFolderToMove->children<AbstractAspect>()) {
			auto* folder = dynamic_cast<Folder*>(child);
			if (folder) {
				moveFolder(targetChildFolder, folder);
			} else {
				sourceChildFolderToMove->removeChild(child);

				//remove the object to be imported in the target folder if it's already existing
				auto* targetChild = targetChildFolder->child<AbstractAspect>(child->name());
				if (targetChild)
					targetChildFolder->removeChild(targetChild);

				targetChildFolder->addChild(child);
			}
		}
	} else {
		//folder doesn't exist yet in the target parent folder -> simply move it
		auto* sourceParentFolder = dynamic_cast<Folder*>(sourceChildFolderToMove->parentAspect());
		sourceParentFolder->removeChild(sourceChildFolderToMove);
		targetParentFolder->addChild(sourceChildFolderToMove);
	}
}
