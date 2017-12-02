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
#include "kdefrontend/datasources/ImportOpj.h"

#include "backend/core/Workbook.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/matrix/Matrix.h"
#include "backend/note/Note.h"
#include "backend/worksheet/Worksheet.h"

#include <QDateTime>

/*!
\class OriginProjectParser
\brief parser for Origin projects.

\ingroup datasources
*/

OriginProjectParser::OriginProjectParser() : ProjectParser() {
	m_topLevelClasses << "Folder" << "Workbook" << "Spreadsheet" << "Matrix" << "Worksheet";
}

QAbstractItemModel* OriginProjectParser::model() {
	WAIT_CURSOR;
	if (m_project == nullptr)
		m_project = new Project();

	AspectTreeModel* model = nullptr;

	//read and parse the opj-file
	OriginFile opj((const char*)m_projectFileName.toLocal8Bit());
	if (!opj.parse()) {
		RESET_CURSOR;
		return model;
	}

	//convert the project tree from liborigin' representation to LabPlot's project object
	const tree<Origin::ProjectNode>* projectTree = opj.project();
	for (tree<Origin::ProjectNode>::iterator it = projectTree->begin(projectTree->begin()); it != projectTree->end(projectTree->begin()); ++it) {
		//name
		QString name(it->name.c_str());

		//creation time
		//this logic seems to be correct only for the first node (project node). For other nodes the current time is returned.
		char time_str[21];
		strftime(time_str, sizeof(time_str), "%F %T", gmtime(&(*it).creationDate));
		std::cout <<  string(projectTree->depth(it) - 1, ' ') <<  (*it).name.c_str() << "\t" << time_str << endl;
		QDateTime creationTime = QDateTime::fromString(QString(time_str), Qt::ISODate);

		if (it == projectTree->begin(projectTree->begin()) && it->type == Origin::ProjectNode::Folder) {
			m_project->setName(name);
			m_project->setCreationTime(creationTime);
			continue;
		}

		//type
		AbstractAspect* aspect = nullptr;
		switch (it->type) {
		case Origin::ProjectNode::Folder:
			aspect = new Folder(name);
			break;
		case Origin::ProjectNode::SpreadSheet:
			aspect = new Workbook(0, name);
			break;
		case Origin::ProjectNode::Graph:
			aspect = new Worksheet(0, name);
			break;
		case Origin::ProjectNode::Matrix:
			aspect = new Matrix(0, name);
			break;
		case Origin::ProjectNode::Note:
			aspect = new Note(name);
			break;
		case Origin::ProjectNode::Excel:
		case Origin::ProjectNode::Graph3D:
		default:
			//TODO: add UnsupportedAspect
			break;
		}

		if (aspect) {
			m_project->addChildFast(aspect);
			aspect->setCreationTime(creationTime);
		}
	}

	model = new AspectTreeModel(m_project);
	model->setReadOnly(true);

	RESET_CURSOR;
	return model;
}

void OriginProjectParser::importTo(Folder* folder, const QStringList& selectedPathes) {
	Q_UNUSED(selectedPathes);
	ImportOpj(folder, m_projectFileName, false);
}
