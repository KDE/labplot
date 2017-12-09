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
// #include <liborigin/OriginFile.h>
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

OriginProjectParser::OriginProjectParser() : ProjectParser(),
	m_originFile(nullptr),
	m_excelIndex(0),
	m_matrixIndex(0),
	m_worksheetIndex(0),
	m_noteIndex(0) {

	m_topLevelClasses << "Folder" << "Workbook" << "Spreadsheet" << "Matrix" << "Worksheet";
}

QAbstractItemModel* OriginProjectParser::model() {
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

void OriginProjectParser::importTo(Folder* folder, const QStringList& selectedPathes) {
	QDEBUG("Starting the import of " + m_projectFileName);

	//import the selected objects into a temporary project
	Project* project = new Project();
	project->setPathesToLoad(selectedPathes);
	load(project, false);

	//move all children from the temp project to the target folder
	for (auto* child : project->children<AbstractAspect>()) {
		project->removeChild(child);
		folder->addChild(child);
	}
	delete project;

	QDEBUG("Import of " + m_projectFileName + " done.");
}

//##############################################################################
//############## Deserialization from Origin's project tree ####################
//##############################################################################
bool OriginProjectParser::load(Project* project, bool preview) {
	//read and parse the m_originFile-file
	if (m_originFile)
		delete m_originFile;

	m_originFile = new OriginFile((const char*)m_projectFileName.toLocal8Bit());
	if (!m_originFile->parse()) {
		return false;
	}


	//Origin project tree and the iterator pointing to the root node
	const tree<Origin::ProjectNode>* projectTree = m_originFile->project();
	tree<Origin::ProjectNode>::iterator projectIt = projectTree->begin(projectTree->begin());

	//reset the object indices
	m_excelIndex = 0;
	m_matrixIndex = 0;
	m_worksheetIndex = 0;
	m_noteIndex = 0;

	//convert the project tree from liborigin's representation to LabPlot's project object
	QString name(projectIt->name.c_str());
	m_project->setName(name);
	m_project->setCreationTime(creationTime(projectIt));
	loadFolder(project, projectIt, preview);

	return true;
}

bool OriginProjectParser::loadFolder(Folder* folder, const tree<Origin::ProjectNode>::iterator& baseIt, bool preview) {

	//load folder's children
	const tree<Origin::ProjectNode>* projectTree = m_originFile->project();
	for (tree<Origin::ProjectNode>::sibling_iterator it = projectTree->begin(baseIt); it != projectTree->end(baseIt); ++it) {
		QString name(it->name.c_str());

		//load top-level children
		AbstractAspect* aspect = nullptr;
		switch (it->type) {
		case Origin::ProjectNode::Folder: {
			Folder* f = new Folder(name);
			loadFolder(f, it, preview);
			aspect = f;
			break;
		}
		case Origin::ProjectNode::SpreadSheet: {
			Spreadsheet* spreadsheet = new Spreadsheet(0, name);
			loadSpreadsheet(spreadsheet, preview);
			aspect = spreadsheet;
			break;
		}
		case Origin::ProjectNode::Graph: {
			Worksheet* worksheet = new Worksheet(0, name);
			loadWorksheet(worksheet, preview);
			aspect = worksheet;
			++m_worksheetIndex;
			break;
		}
		case Origin::ProjectNode::Matrix: {
			Matrix* matrix = new Matrix(0, name);
			loadMatrix(matrix, preview);
			aspect = matrix;
			++m_matrixIndex;
			break;
		}
		case Origin::ProjectNode::Excel: {
			const Origin::Excel excel = m_originFile->excel(m_excelIndex);
			if (excel.sheets.size() == 1) {
				// single sheet -> load into a spreadsheet
				Spreadsheet* spreadsheet = new Spreadsheet(0, name);
				loadSpreadsheet(spreadsheet, preview);
				aspect = spreadsheet;
			} else {
				// multiple sheets -> load into a workbook
				Workbook* workbook = new Workbook(0, name);
				loadWorkbook(workbook, preview);
				aspect = workbook;
			}
			++m_excelIndex;
			break;
		}
		case Origin::ProjectNode::Note: {
			Note* note = new Note(name);
			loadNote(note, preview);
			aspect = note;
			++m_noteIndex;
			break;
		}
		case Origin::ProjectNode::Graph3D:
		default:
			//TODO: add UnsupportedAspect
			break;
		}

		if (aspect) {
			folder->addChildFast(aspect);
			aspect->setCreationTime(creationTime(it));
		}
	}

	return folder;
}

bool OriginProjectParser::loadWorkbook(Workbook* workbook, bool preview) {
	if (preview)
		return true;

	//load workbook sheets
	Q_UNUSED(workbook)

	return true;
}

bool OriginProjectParser::loadSpreadsheet(Spreadsheet* spreadsheet, bool preview) {
	if (preview)
		return true;

	//load spreadsheet data
	Q_UNUSED(spreadsheet);
// 	const Origin::Excel excel = m_originFile->excel(m_excelIndex);
// 	Origin::SpreadSheet spread = excelwb.sheets[0];
// 	spread.name = excelwb.name;
// 	spread.label = excelwb.label;
// 	importSpreadsheet(0, m_originFile, spread);

	return true;
}

bool OriginProjectParser::loadMatrix(Matrix* matrix, bool preview) {
	if (preview)
		return true;

	//import matrix data
	Q_UNUSED(matrix);
// 	Origin::Matrix matrix = m_originFile->matrix(m);
// 	importMatrix(m_originFile, matrix);

	return true;
}


bool OriginProjectParser::loadWorksheet(Worksheet* worksheet, bool preview) {
	if (preview)
		return true;

	//load worksheet data
	Q_UNUSED(worksheet);

	return true;
}

bool OriginProjectParser::loadNote(Note* note, bool preview) {
	if (preview)
		return true;

	//load note data
	Q_UNUSED(note);

	return true;
}

QDateTime OriginProjectParser::creationTime(const tree<Origin::ProjectNode>::iterator& it) const {
	//this logic seems to be correct only for the first node (project node). For other nodes the current time is returned.
	char time_str[21];
	strftime(time_str, sizeof(time_str), "%F %T", gmtime(&(*it).creationDate));
	return QDateTime::fromString(QString(time_str), Qt::ISODate);
}
