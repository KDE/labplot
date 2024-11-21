/*
	File                 : OriginProjectParser.cpp
	Project              : LabPlot
	Description          : parser for Origin projects
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2017-2024 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2017-2024 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "backend/datasources/projects/OriginProjectParser.h"
#include "backend/core/Project.h"
#include "backend/core/Workbook.h"
#include "backend/core/column/Column.h"
#include "backend/core/datatypes/DateTime2StringFilter.h"
#include "backend/core/datatypes/Double2StringFilter.h"
#include "backend/matrix/Matrix.h"
#include "backend/note/Note.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/worksheet/Background.h"
#include "backend/worksheet/Line.h"
#include "backend/worksheet/TextLabel.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/WorksheetElement.h"
#include "backend/worksheet/plots/PlotArea.h"
#include "backend/worksheet/plots/cartesian/Axis.h"
#include "backend/worksheet/plots/cartesian/BarPlot.h"
#include "backend/worksheet/plots/cartesian/BoxPlot.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/CartesianPlotLegend.h"
#include "backend/worksheet/plots/cartesian/Histogram.h"
#include "backend/worksheet/plots/cartesian/Symbol.h"
#include "backend/worksheet/plots/cartesian/Value.h"
#include "backend/worksheet/plots/cartesian/XYEquationCurve.h"

#include <KLocalizedString>

#include <QDateTime>
#include <QDir>
#include <QFontMetrics>
#include <QGraphicsScene>
#include <QRegularExpression>

#include <gsl/gsl_const_cgs.h>

/*!
\class OriginProjectParser
\brief parser for Origin projects.

\ingroup datasources
*/

OriginProjectParser::OriginProjectParser()
	: ProjectParser() {
	m_topLevelClasses = {AspectType::Folder, AspectType::Workbook, AspectType::Spreadsheet, AspectType::Matrix, AspectType::Worksheet, AspectType::Note};
}

OriginProjectParser::~OriginProjectParser() {
	delete m_originFile;
}

bool OriginProjectParser::isOriginProject(const QString& fileName) {
	// TODO add opju later when liborigin supports it
	return fileName.endsWith(QLatin1String(".opj"), Qt::CaseInsensitive);
}

void OriginProjectParser::setImportUnusedObjects(bool importUnusedObjects) {
	m_importUnusedObjects = importUnusedObjects;
}

void OriginProjectParser::checkContent(bool& hasUnusedObjects, bool& hasMultiLayerGraphs) {
	DEBUG(Q_FUNC_INFO)
	m_originFile = new OriginFile(qPrintable(m_projectFileName));
	if (!m_originFile->parse()) {
		delete m_originFile;
		m_originFile = nullptr;
		hasUnusedObjects = false;
		hasMultiLayerGraphs = false;
		return;
	}

	hasUnusedObjects = this->hasUnusedObjects();
	hasMultiLayerGraphs = this->hasMultiLayerGraphs();

	delete m_originFile;
	m_originFile = nullptr;
}

bool OriginProjectParser::hasUnusedObjects() {
	if (!m_originFile)
		return false;

	for (unsigned int i = 0; i < m_originFile->spreadCount(); i++) {
		const auto& spread = m_originFile->spread(i);
		if (spread.objectID < 0)
			return true;
	}
	for (unsigned int i = 0; i < m_originFile->excelCount(); i++) {
		const auto& excel = m_originFile->excel(i);
		if (excel.objectID < 0)
			return true;
	}
	for (unsigned int i = 0; i < m_originFile->matrixCount(); i++) {
		const auto& matrix = m_originFile->matrix(i);
		if (matrix.objectID < 0)
			return true;
	}

	return false;
}

bool OriginProjectParser::hasMultiLayerGraphs() {
	if (!m_originFile)
		return false;

	for (unsigned int i = 0; i < m_originFile->graphCount(); i++) {
		const auto& graph = m_originFile->graph(i);
		if (graph.layers.size() > 1)
			return true;
	}

	return false;
}

void OriginProjectParser::setGraphLayerAsPlotArea(bool value) {
	m_graphLayerAsPlotArea = value;
}

QString OriginProjectParser::supportedExtensions() {
	// TODO: add opju later when liborigin ever supports it
	static const QString extensions = QStringLiteral("*.opj *.OPJ");
	return extensions;
}

// sets first found spread of given name
unsigned int OriginProjectParser::findSpreadsheetByName(const QString& name) {
	DEBUG(Q_FUNC_INFO << ", name = " << name.toStdString() << ", count = " << m_originFile->spreadCount())
	for (unsigned int i = 0; i < m_originFile->spreadCount(); i++) {
		const auto& spread = m_originFile->spread(i);
		DEBUG(Q_FUNC_INFO << ", spreadsheet name = " << spread.name)
		if (spread.name == name.toStdString()) {
			m_spreadsheetNameList << name;
			m_spreadsheetNameList.removeDuplicates();
			return i;
		}
	}
	return 0;
}
unsigned int OriginProjectParser::findColumnByName(const Origin::SpreadSheet& spread, const QString& name) {
	for (unsigned int i = 0; i < spread.columns.size(); i++) {
		const auto& column = spread.columns[i];
		if (column.name == name.toStdString())
			return i;
	}
	return 0;
}
unsigned int OriginProjectParser::findMatrixByName(const QString& name) {
	for (unsigned int i = 0; i < m_originFile->matrixCount(); i++) {
		const auto& originMatrix = m_originFile->matrix(i);
		if (originMatrix.name == name.toStdString()) {
			m_matrixNameList << name;
			m_matrixNameList.removeDuplicates();
			return i;
		}
	}
	return 0;
}
unsigned int OriginProjectParser::findWorkbookByName(const QString& name) {
	// QDEBUG("WORKBOOK LIST: " << m_workbookNameList << ", name = " << name)
	for (unsigned int i = 0; i < m_originFile->excelCount(); i++) {
		const auto& excel = m_originFile->excel(i);
		if (excel.name == name.toStdString()) {
			m_workbookNameList << name;
			m_workbookNameList.removeDuplicates();
			return i;
		}
	}
	return 0;
}
unsigned int OriginProjectParser::findWorksheetByName(const QString& name) {
	for (unsigned int i = 0; i < m_originFile->graphCount(); i++) {
		const auto& graph = m_originFile->graph(i);
		if (graph.name == name.toStdString()) {
			m_worksheetNameList << name;
			m_worksheetNameList.removeDuplicates();
			return i;
		}
	}
	return 0;
}
unsigned int OriginProjectParser::findNoteByName(const QString& name) {
	for (unsigned int i = 0; i < m_originFile->noteCount(); i++) {
		const auto& originNote = m_originFile->note(i);
		if (originNote.name == name.toStdString()) {
			m_noteNameList << name;
			m_noteNameList.removeDuplicates();
			return i;
		}
	}
	return 0;
}

// get Origin::Spreadsheet from container name (may be a spreadsheet or workbook)
Origin::SpreadSheet OriginProjectParser::getSpreadsheetByName(QString& containerName) {
	DEBUG(Q_FUNC_INFO)
	int sheetIndex = 0; // which sheet? "@X"
	const int atIndex = containerName.indexOf(QLatin1Char('@'));
	if (atIndex != -1) {
		sheetIndex = containerName.mid(atIndex + 1).toInt() - 1;
		containerName.truncate(atIndex);
	}
	// DEBUG("CONTAINER = " << STDSTRING(containerName) << ", SHEET = " << sheetIndex)

	// check if workbook
	int workbookIndex = findWorkbookByName(containerName);
	// if workbook not found, findWorkbookByName() returns 0: check this
	if (workbookIndex == 0 && (m_originFile->excelCount() == 0 || containerName.toStdString() != m_originFile->excel(0).name))
		workbookIndex = -1;
	// DEBUG("WORKBOOK  index = " << workbookIndex)

	// comment of y column is used in legend (if not empty), else the column name
	Origin::SpreadSheet sheet;
	if (workbookIndex != -1) { // container is a workbook
		sheet = m_originFile->excel(workbookIndex).sheets[sheetIndex];
	} else { // container is a spreadsheet?
		int spreadsheetIndex = findSpreadsheetByName(containerName);
		// if spreadsheet not found, findSpreadsheetByName() returns 0: check this
		if (spreadsheetIndex == 0 && (m_originFile->spreadCount() == 0 || containerName.toStdString() != m_originFile->spread(0).name))
			spreadsheetIndex = -1;
		if (spreadsheetIndex != -1)
			sheet = m_originFile->spread(spreadsheetIndex);
	}

	return sheet;
}

// ##############################################################################
// ############## Deserialization from Origin's project tree ####################
// ##############################################################################
bool OriginProjectParser::load(Project* project, bool preview) {
	DEBUG(Q_FUNC_INFO);

	// read and parse the m_originFile-file
	m_originFile = new OriginFile(qPrintable(m_projectFileName));
	if (!m_originFile->parse()) {
		delete m_originFile;
		m_originFile = nullptr;
		return false;
	}

	DEBUG(Q_FUNC_INFO << ", project file name: " << m_projectFileName.toStdString());
	DEBUG(Q_FUNC_INFO << ", Origin version: " << std::setprecision(4) << m_originFile->version());

	// Origin project tree and the iterator pointing to the root node
	const auto* projectTree = m_originFile->project();
	auto projectIt = projectTree->begin(projectTree->begin());

	m_spreadsheetNameList.clear();
	m_workbookNameList.clear();
	m_matrixNameList.clear();
	m_worksheetNameList.clear();
	m_noteNameList.clear();

	// convert the project tree from liborigin's representation to LabPlot's project object
	project->setIsLoading(true);
	if (projectIt.node) { // only opj files from version >= 6.0 have a project tree
		DEBUG(Q_FUNC_INFO << ", project tree found");
		QString name(QString::fromLatin1(projectIt->name.c_str()));
		project->setName(name);
		project->setCreationTime(creationTime(projectIt));
		loadFolder(project, projectIt, preview);
	} else { // for older versions put all windows on rootfolder
		DEBUG(Q_FUNC_INFO << ", no project tree");
		int pos = m_projectFileName.lastIndexOf(QLatin1Char('/')) + 1;
		project->setName(m_projectFileName.mid(pos));
	}
	// imports all loose windows (like prior version 6 which has no project tree)
	handleLooseWindows(project, preview);

	// restore column pointers:
	restorePointers(project);

	if (!preview) {
		const auto& plots = project->children<CartesianPlot>(AbstractAspect::ChildIndexFlag::Recursive);
		for (auto* plot : plots) {
			plot->setIsLoading(false);
			plot->retransform();
		}
	}

	project->setIsLoading(false);

	delete m_originFile;
	m_originFile = nullptr;

	return true;
}

/*!
 * restores the column pointers from the column paths after the project was loaded and all objects instantiated.
 * TODO: why do we need this extra logic here and why Project::restorePointers() is not enough which is already called in ProjectParser::importTo() at the end
 * of the import anyway?
 */
void OriginProjectParser::restorePointers(Project* project) {
	// 1. extend the paths to contain the parent structures first
	// 2. restore the pointers from the paths
	const auto& columns = project->children<Column>(AbstractAspect::ChildIndexFlag::Recursive);
	const auto& spreadsheets = project->children<Spreadsheet>(AbstractAspect::ChildIndexFlag::Recursive);
	DEBUG(Q_FUNC_INFO << ", NUMBER of spreadsheets/columns = " << spreadsheets.count() << "/" << columns.count())

	// xy-curves
	const auto& curves = project->children<XYCurve>(AbstractAspect::ChildIndexFlag::Recursive);
	for (auto* curve : curves) {
		DEBUG(Q_FUNC_INFO << ", RESTORE CURVE with x/y column path " << STDSTRING(curve->xColumnPath()) << " " << STDSTRING(curve->yColumnPath()))
		curve->setSuppressRetransform(true);

		// x-column
		auto spreadsheetName = curve->xColumnPath();
		spreadsheetName.truncate(curve->xColumnPath().lastIndexOf(QLatin1Char('/')));
		// DEBUG(Q_FUNC_INFO << ", SPREADSHEET name from column: " << STDSTRING(spreadsheetName))
		for (const auto* spreadsheet : spreadsheets) {
			QString container, containerPath = spreadsheet->parentAspect()->path();
			if (spreadsheetName.contains(QLatin1Char('/'))) { // part of a workbook
				container = containerPath.mid(containerPath.lastIndexOf(QLatin1Char('/')) + 1) + QLatin1Char('/');
				containerPath = containerPath.left(containerPath.lastIndexOf(QLatin1Char('/')));
			}
			// DEBUG("CONTAINER = " << STDSTRING(container))
			// DEBUG("CONTAINER PATH = " << STDSTRING(containerPath))
			// DEBUG(Q_FUNC_INFO << ", LOOP spreadsheet names = \"" << STDSTRING(container) +
			//	STDSTRING(spreadsheet->name()) << "\", path = " << STDSTRING(spreadsheetName))
			// DEBUG("SPREADSHEET parent path = " << STDSTRING(spreadsheet->parentAspect()->path()))
			if (container + spreadsheet->name() == spreadsheetName) {
				const QString& newPath = containerPath + QLatin1Char('/') + curve->xColumnPath();
				// DEBUG(Q_FUNC_INFO << ", SET COLUMN PATH to \"" << STDSTRING(newPath) << "\"")
				curve->setXColumnPath(newPath);

				RESTORE_COLUMN_POINTER(curve, xColumn, XColumn);
				break;
			}
		}

		// y-column
		spreadsheetName = curve->yColumnPath();
		spreadsheetName.truncate(curve->yColumnPath().lastIndexOf(QLatin1Char('/')));
		for (const auto* spreadsheet : spreadsheets) {
			QString container, containerPath = spreadsheet->parentAspect()->path();
			if (spreadsheetName.contains(QLatin1Char('/'))) { // part of a workbook
				container = containerPath.mid(containerPath.lastIndexOf(QLatin1Char('/')) + 1) + QLatin1Char('/');
				containerPath = containerPath.left(containerPath.lastIndexOf(QLatin1Char('/')));
			}
			if (container + spreadsheet->name() == spreadsheetName) {
				const QString& newPath = containerPath + QLatin1Char('/') + curve->yColumnPath();
				curve->setYColumnPath(newPath);

				RESTORE_COLUMN_POINTER(curve, yColumn, YColumn);
				break;
			}
		}
		DEBUG(Q_FUNC_INFO << ", curve x/y COLUMNS = " << curve->xColumn() << "/" << curve->yColumn())

		// error columns
		// x-error-column
		spreadsheetName = curve->errorBar()->xPlusColumnPath();
		spreadsheetName.truncate(curve->errorBar()->xPlusColumnPath().lastIndexOf(QLatin1Char('/')));
		for (const auto* spreadsheet : spreadsheets) {
			QString container, containerPath = spreadsheet->parentAspect()->path();
			if (spreadsheetName.contains(QLatin1Char('/'))) { // part of a workbook
				container = containerPath.mid(containerPath.lastIndexOf(QLatin1Char('/')) + 1) + QLatin1Char('/');
				containerPath = containerPath.left(containerPath.lastIndexOf(QLatin1Char('/')));
			}
			if (container + spreadsheet->name() == spreadsheetName) {
				const QString& newPath = containerPath + QLatin1Char('/') + curve->errorBar()->xPlusColumnPath();
				DEBUG(Q_FUNC_INFO << ", SET COLUMN PATH to \"" << STDSTRING(newPath) << "\"")
				curve->errorBar()->setXPlusColumnPath(newPath);

				// not needed
				// RESTORE_COLUMN_POINTER(curve, errorBar()->yPlusColumn, errorBar()->YPlusColumn);
				break;
			}
		}
		// y-error-column
		spreadsheetName = curve->errorBar()->yPlusColumnPath();
		spreadsheetName.truncate(curve->errorBar()->yPlusColumnPath().lastIndexOf(QLatin1Char('/')));
		for (const auto* spreadsheet : spreadsheets) {
			QString container, containerPath = spreadsheet->parentAspect()->path();
			if (spreadsheetName.contains(QLatin1Char('/'))) { // part of a workbook
				container = containerPath.mid(containerPath.lastIndexOf(QLatin1Char('/')) + 1) + QLatin1Char('/');
				containerPath = containerPath.left(containerPath.lastIndexOf(QLatin1Char('/')));
			}
			if (container + spreadsheet->name() == spreadsheetName) {
				const QString& newPath = containerPath + QLatin1Char('/') + curve->errorBar()->yPlusColumnPath();
				curve->errorBar()->setYPlusColumnPath(newPath);

				// not needed
				// RESTORE_COLUMN_POINTER(curve, errorBar()->yPlusColumn, errorBar()->YPlusColumn);
				break;
			}
		}
		DEBUG(Q_FUNC_INFO << ", curve x/y error COLUMNS = " << curve->errorBar()->xPlusColumn() << "/" << curve->errorBar()->yPlusColumn())

		curve->setSuppressRetransform(false);
	}

	// histograms
	const auto& hists = project->children<Histogram>(AbstractAspect::ChildIndexFlag::Recursive);
	for (auto* hist : hists) {
		if (!hist)
			continue;
		hist->setSuppressRetransform(true);

		// data column
		auto spreadsheetName = hist->dataColumnPath();
		spreadsheetName.truncate(hist->dataColumnPath().lastIndexOf(QLatin1Char('/')));
		for (const auto* spreadsheet : spreadsheets) {
			QString container, containerPath = spreadsheet->parentAspect()->path();
			if (spreadsheetName.contains(QLatin1Char('/'))) { // part of a workbook
				container = containerPath.mid(containerPath.lastIndexOf(QLatin1Char('/')) + 1) + QLatin1Char('/');
				containerPath = containerPath.left(containerPath.lastIndexOf(QLatin1Char('/')));
			}
			if (container + spreadsheet->name() == spreadsheetName) {
				const QString& newPath = containerPath + QLatin1Char('/') + hist->dataColumnPath();
				hist->setDataColumnPath(newPath);
				RESTORE_COLUMN_POINTER(hist, dataColumn, DataColumn);
				break;
			}
		}

		// unused: auto* value = hist->value();
		// RESTORE_COLUMN_POINTER(value, column, Column);
		// TODO
		RESTORE_COLUMN_POINTER(hist->errorBar(), xPlusColumn, XPlusColumn);
		RESTORE_COLUMN_POINTER(hist->errorBar(), xMinusColumn, XMinusColumn);
		hist->setSuppressRetransform(false);
	}

	// bar plots
	const auto& barPlots = project->children<BarPlot>(AbstractAspect::ChildIndexFlag::Recursive);
	for (auto* barPlot : barPlots) {
		if (!barPlot)
			continue;
		barPlot->setSuppressRetransform(true);

		// x-column
		auto spreadsheetName = barPlot->xColumnPath();
		spreadsheetName.truncate(barPlot->xColumnPath().lastIndexOf(QLatin1Char('/')));
		for (const auto* spreadsheet : spreadsheets) {
			QString container, containerPath = spreadsheet->parentAspect()->path();
			if (spreadsheetName.contains(QLatin1Char('/'))) { // part of a workbook
				container = containerPath.mid(containerPath.lastIndexOf(QLatin1Char('/')) + 1) + QLatin1Char('/');
				containerPath = containerPath.left(containerPath.lastIndexOf(QLatin1Char('/')));
			}
			if (container + spreadsheet->name() == spreadsheetName) {
				const QString& newPath = containerPath + QLatin1Char('/') + barPlot->xColumnPath();
				barPlot->xColumnPath() = newPath;

				RESTORE_COLUMN_POINTER(barPlot, xColumn, XColumn);
				break;
			}
		}

		// data column
		for (auto path : barPlot->dataColumnPaths()) {
			spreadsheetName = path;
			spreadsheetName.truncate(spreadsheetName.lastIndexOf(QLatin1Char('/')));
			for (const auto* spreadsheet : spreadsheets) {
				QString container, containerPath = spreadsheet->parentAspect()->path();
				if (spreadsheetName.contains(QLatin1Char('/'))) { // part of a workbook
					container = containerPath.mid(containerPath.lastIndexOf(QLatin1Char('/')) + 1) + QLatin1Char('/');
					containerPath = containerPath.left(containerPath.lastIndexOf(QLatin1Char('/')));
				}
				if (container + spreadsheet->name() == spreadsheetName) {
					auto paths = barPlot->dataColumnPaths();
					const QString& newPath = containerPath + QLatin1Char('/') + path;

					// replace path
					const auto index = paths.indexOf(path);
					if (index != -1)
						paths[index] = newPath;
					else
						continue;

					barPlot->setDataColumnPaths(paths);

					// RESTORE_COLUMN_POINTER
					if (!path.isEmpty()) {
						for (auto* column : columns) {
							if (!column)
								continue;
							if (column->path() == newPath) {
								auto dataColumns = barPlot->dataColumns();
								if (!dataColumns.contains(column)) {
									dataColumns.append(column);
									barPlot->setDataColumns(std::move(dataColumns));
								}
								break;
							}
						}
					}
					break;
				}
			}
		}
		barPlot->setSuppressRetransform(false);
	}

	// TODO: others
}

bool OriginProjectParser::loadFolder(Folder* folder, tree<Origin::ProjectNode>::iterator baseIt, bool preview) {
	DEBUG(Q_FUNC_INFO)
	const auto* projectTree = m_originFile->project();

	// do not skip anything if pathesToLoad() contains only root folder
	bool containsRootFolder = (folder->pathesToLoad().size() == 1 && folder->pathesToLoad().contains(folder->path()));
	if (containsRootFolder) {
		DEBUG("	pathesToLoad contains only folder path \"" << STDSTRING(folder->path()) << "\". Clearing pathes to load.")
		folder->setPathesToLoad(QStringList());
	}

	// load folder's children: logic for reading the selected objects only is similar to Folder::readChildAspectElement
	for (auto it = projectTree->begin(baseIt); it != projectTree->end(baseIt); ++it) {
		QString name(QString::fromLatin1(it->name.c_str())); // name of the current child
		DEBUG(Q_FUNC_INFO << ", folder item name = " << STDSTRING(name))

		// check whether we need to skip the loading of the current child
		if (!folder->pathesToLoad().isEmpty()) {
			// child's path is not available yet (child not added yet) -> construct the path manually
			const QString childPath = folder->path() + QLatin1Char('/') + name;
			DEBUG("		path = " << STDSTRING(childPath))

			// skip the current child aspect it is not in the list of aspects to be loaded
			if (folder->pathesToLoad().indexOf(childPath) == -1) {
				DEBUG("		skip it!")
				continue;
			}
		}

		// load top-level children.
		// use 'preview' as 'loading'-parameter in the constructors to skip the init() calls in Worksheet, Spreadsheet and Matrix:
		//* when doing the preview of the project we don't want to initialize the objects and skip init()'s
		//* when loading the project, 'preview' is false and we initialize all objects with our default values
		//   and set all possible properties from Origin additionally
		AbstractAspect* aspect = nullptr;
		switch (it->type) {
		case Origin::ProjectNode::Folder: {
			DEBUG(Q_FUNC_INFO << ", top level FOLDER");
			Folder* f = new Folder(name);

			if (!folder->pathesToLoad().isEmpty()) {
				// a child folder to be read -> provide the list of aspects to be loaded to the child folder, too.
				// since the child folder and all its children are not added yet (path() returns empty string),
				// we need to remove the path of the current child folder from the full pathes provided in pathesToLoad.
				// E.g. we want to import the path "Project/Folder/Spreadsheet" in the following project
				//  Project
				//         \Spreadsheet
				//         \Folder
				//                \Spreadsheet
				//
				// Here, we remove the part "Project/Folder/" and proceed for this child folder with "Spreadsheet" only.
				// With this the logic above where it is determined whether to import the child aspect or not works out.

				// manually construct the path of the child folder to be read
				const QString& curFolderPath = folder->path() + QLatin1Char('/') + name;

				// remove the path of the current child folder
				QStringList pathesToLoadNew;
				for (const auto& path : folder->pathesToLoad()) {
					if (path.startsWith(curFolderPath))
						pathesToLoadNew << path.right(path.length() - curFolderPath.length());
				}

				f->setPathesToLoad(pathesToLoadNew);
			}

			loadFolder(f, it, preview);
			aspect = f;
			break;
		}
		case Origin::ProjectNode::SpreadSheet: {
			DEBUG(Q_FUNC_INFO << ", top level SPREADSHEET");
			auto* spreadsheet = new Spreadsheet(name, preview);
			loadSpreadsheet(spreadsheet, preview, name);
			aspect = spreadsheet;
			break;
		}
		case Origin::ProjectNode::Graph: {
			DEBUG(Q_FUNC_INFO << ", top level GRAPH");
			auto* worksheet = new Worksheet(name, preview);
			if (!preview) {
				worksheet->setIsLoading(true);
				worksheet->setTheme(QString());
			}
			loadWorksheet(worksheet, preview);
			aspect = worksheet;
			break;
		}
		case Origin::ProjectNode::Matrix: {
			DEBUG(Q_FUNC_INFO << ", top level MATRIX");
			const auto& originMatrix = m_originFile->matrix(findMatrixByName(name));
			DEBUG("	matrix name = " << originMatrix.name);
			DEBUG("	number of sheets = " << originMatrix.sheets.size());
			if (originMatrix.sheets.size() == 1) {
				// single sheet -> load into a matrix
				Matrix* matrix = new Matrix(name, preview);
				loadMatrix(matrix, preview);
				aspect = matrix;
			} else {
				// multiple sheets -> load into a workbook
				Workbook* workbook = new Workbook(name);
				loadMatrixWorkbook(workbook, preview);
				aspect = workbook;
			}
			break;
		}
		case Origin::ProjectNode::Excel: {
			DEBUG(Q_FUNC_INFO << ", top level WORKBOOK");
			auto* workbook = new Workbook(name);
			loadWorkbook(workbook, preview);
			aspect = workbook;
			break;
		}
		case Origin::ProjectNode::Note: {
			DEBUG(Q_FUNC_INFO << ", top level NOTE");
			Note* note = new Note(name);
			loadNote(note, preview);
			aspect = note;
			break;
		}
		case Origin::ProjectNode::Graph3D:
		default:
			// TODO: add UnsupportedAspect
			break;
		}

		if (aspect) {
			folder->addChildFast(aspect);
			aspect->setCreationTime(creationTime(it));
			aspect->setIsLoading(false);
		}
	}

	// ResultsLog
	QString resultsLog = QString::fromStdString(m_originFile->resultsLogString());
	if (resultsLog.length() > 0) {
		DEBUG("Results log:\t\tyes");
		Note* note = new Note(QStringLiteral("ResultsLog"));

		if (preview)
			folder->addChildFast(note);
		else {
			// only import the log if it is in the list of aspects to be loaded
			const QString childPath = folder->path() + QLatin1Char('/') + note->name();
			if (folder->pathesToLoad().indexOf(childPath) != -1) {
				note->setText(resultsLog);
				folder->addChildFast(note);
			}
		}
	} else
		DEBUG("Results log:\t\tno");

	return folder;
}

void OriginProjectParser::handleLooseWindows(Folder* folder, bool preview) {
	QDEBUG(Q_FUNC_INFO << ", paths to load:" << folder->pathesToLoad());
	QDEBUG("	spreads =" << m_spreadsheetNameList);
	QDEBUG("	workbooks =" << m_workbookNameList);
	QDEBUG("	matrices =" << m_matrixNameList);
	QDEBUG("	worksheets =" << m_worksheetNameList);
	QDEBUG("	notes =" << m_noteNameList);

	DEBUG("Number of spreads loaded:\t" << m_spreadsheetNameList.size() << ", in file: " << m_originFile->spreadCount());
	DEBUG("Number of excels loaded:\t" << m_workbookNameList.size() << ", in file: " << m_originFile->excelCount());
	DEBUG("Number of matrices loaded:\t" << m_matrixNameList.size() << ", in file: " << m_originFile->matrixCount());
	DEBUG("Number of graphs loaded:\t" << m_worksheetNameList.size() << ", in file: " << m_originFile->graphCount());
	DEBUG("Number of notes loaded:\t\t" << m_noteNameList.size() << ", in file: " << m_originFile->noteCount());

	// loop over all spreads to find loose ones
	for (unsigned int i = 0; i < m_originFile->spreadCount(); i++) {
		AbstractAspect* aspect = nullptr;
		const auto& spread = m_originFile->spread(i);
		QString name = QString::fromStdString(spread.name);

		DEBUG("	spread.objectId = " << spread.objectID);
		// skip unused spreads if selected
		if (spread.objectID < 0 && !m_importUnusedObjects) {
			DEBUG("	Dropping unused loose spread: " << STDSTRING(name));
			continue;
		}

		const QString childPath = folder->path() + QLatin1Char('/') + name;
		// we could also use spread.loose
		if (!m_spreadsheetNameList.contains(name) && (preview || folder->pathesToLoad().indexOf(childPath) != -1)) {
			DEBUG("	Adding loose spread: " << STDSTRING(name));

			auto* spreadsheet = new Spreadsheet(name);
			loadSpreadsheet(spreadsheet, preview, name);
			aspect = spreadsheet;
		}
		if (aspect) {
			folder->addChildFast(aspect);
			DEBUG("	creation time as reported by liborigin: " << spread.creationDate);
			aspect->setCreationTime(QDateTime::fromSecsSinceEpoch(spread.creationDate));
		}
	}
	// loop over all workbooks to find loose ones
	for (unsigned int i = 0; i < m_originFile->excelCount(); i++) {
		AbstractAspect* aspect = nullptr;
		const auto& excel = m_originFile->excel(i);
		QString name = QString::fromStdString(excel.name);

		DEBUG("	excel.objectId = " << excel.objectID);
		// skip unused data sets if selected
		if (excel.objectID < 0 && !m_importUnusedObjects) {
			DEBUG("	Dropping unused loose excel: " << STDSTRING(name));
			continue;
		}

		const QString childPath = folder->path() + QLatin1Char('/') + name;
		// we could also use excel.loose
		if (!m_workbookNameList.contains(name) && (preview || folder->pathesToLoad().indexOf(childPath) != -1)) {
			DEBUG("	Adding loose excel: " << STDSTRING(name));
			DEBUG("	 containing number of sheets = " << excel.sheets.size());

			auto* workbook = new Workbook(name);
			loadWorkbook(workbook, preview);
			aspect = workbook;
		}
		if (aspect) {
			folder->addChildFast(aspect);
			DEBUG("	creation time as reported by liborigin: " << excel.creationDate);
			aspect->setCreationTime(QDateTime::fromSecsSinceEpoch(excel.creationDate));
		}
	}
	// loop over all matrices to find loose ones
	for (unsigned int i = 0; i < m_originFile->matrixCount(); i++) {
		AbstractAspect* aspect = nullptr;
		const auto& originMatrix = m_originFile->matrix(i);
		QString name = QString::fromStdString(originMatrix.name);

		DEBUG("	originMatrix.objectId = " << originMatrix.objectID);
		// skip unused data sets if selected
		if (originMatrix.objectID < 0 && !m_importUnusedObjects) {
			DEBUG("	Dropping unused loose matrix: " << STDSTRING(name));
			continue;
		}

		const QString childPath = folder->path() + QLatin1Char('/') + name;
		if (!m_matrixNameList.contains(name) && (preview || folder->pathesToLoad().indexOf(childPath) != -1)) {
			DEBUG("	Adding loose matrix: " << STDSTRING(name));
			DEBUG("	containing number of sheets = " << originMatrix.sheets.size());
			if (originMatrix.sheets.size() == 1) { // single sheet -> load into a matrix
				auto* matrix = new Matrix(name);
				loadMatrix(matrix, preview);
				aspect = matrix;
			} else { // multiple sheets -> load into a workbook
				auto* workbook = new Workbook(name);
				loadMatrixWorkbook(workbook, preview);
				aspect = workbook;
			}
		}
		if (aspect) {
			folder->addChildFast(aspect);
			aspect->setCreationTime(QDateTime::fromSecsSinceEpoch(originMatrix.creationDate));
		}
	}
	// handle loose graphs (is this even possible?)
	for (unsigned int i = 0; i < m_originFile->graphCount(); i++) {
		AbstractAspect* aspect = nullptr;
		const auto& graph = m_originFile->graph(i);
		QString name = QString::fromStdString(graph.name);

		DEBUG("	graph.objectId = " << graph.objectID);
		// skip unused graph if selected
		if (graph.objectID < 0 && !m_importUnusedObjects) {
			DEBUG("	Dropping unused loose graph: " << STDSTRING(name));
			continue;
		}

		const QString childPath = folder->path() + QLatin1Char('/') + name;
		if (!m_worksheetNameList.contains(name) && (preview || folder->pathesToLoad().indexOf(childPath) != -1)) {
			DEBUG("	Adding loose graph: " << STDSTRING(name));
			auto* worksheet = new Worksheet(name);
			loadWorksheet(worksheet, preview);
			aspect = worksheet;
		}
		if (aspect) {
			folder->addChildFast(aspect);
			aspect->setCreationTime(QDateTime::fromSecsSinceEpoch(graph.creationDate));
		}
	}
	// handle loose notes (is this even possible?)
	for (unsigned int i = 0; i < m_originFile->noteCount(); i++) {
		AbstractAspect* aspect = nullptr;
		const auto& originNote = m_originFile->note(i);
		QString name = QString::fromStdString(originNote.name);

		DEBUG("	originNote.objectId = " << originNote.objectID);
		// skip unused notes if selected
		if (originNote.objectID < 0 && !m_importUnusedObjects) {
			DEBUG("	Dropping unused loose note: " << STDSTRING(name));
			continue;
		}

		const QString childPath = folder->path() + QLatin1Char('/') + name;
		if (!m_noteNameList.contains(name) && (preview || folder->pathesToLoad().indexOf(childPath) != -1)) {
			DEBUG("	Adding loose note: " << STDSTRING(name));
			Note* note = new Note(name);
			loadNote(note, preview);
			aspect = note;
		}
		if (aspect) {
			folder->addChildFast(aspect);
			aspect->setCreationTime(QDateTime::fromSecsSinceEpoch(originNote.creationDate));
		}
	}
}

bool OriginProjectParser::loadWorkbook(Workbook* workbook, bool preview) {
	DEBUG(Q_FUNC_INFO);
	// load workbook sheets
	const auto& excel = m_originFile->excel(findWorkbookByName(workbook->name()));
	DEBUG(Q_FUNC_INFO << ", workbook name = " << excel.name);
	DEBUG(Q_FUNC_INFO << ", number of sheets = " << excel.sheets.size());
	for (unsigned int s = 0; s < excel.sheets.size(); ++s) {
		// DEBUG(Q_FUNC_INFO << ", LOADING SHEET " << excel.sheets[s].name)
		auto* spreadsheet = new Spreadsheet(QString::fromStdString(excel.sheets[s].name));
		loadSpreadsheet(spreadsheet, preview, workbook->name(), s);
		workbook->addChildFast(spreadsheet);
	}

	return true;
}

// load spreadsheet from spread (sheetIndex == -1) or from workbook (only sheet sheetIndex)
// name is the spreadsheet name (spread) or the workbook name (if inside a workbook)
bool OriginProjectParser::loadSpreadsheet(Spreadsheet* spreadsheet, bool preview, const QString& name, int sheetIndex) {
	DEBUG(Q_FUNC_INFO << ", own/workbook name = " << STDSTRING(name) << ", sheet index = " << sheetIndex);

	// load spreadsheet data
	Origin::SpreadSheet spread;
	Origin::Excel excel;
	if (sheetIndex == -1) // spread
		spread = m_originFile->spread(findSpreadsheetByName(name));
	else {
		excel = m_originFile->excel(findWorkbookByName(name));
		spread = excel.sheets.at(sheetIndex);
	}

	const size_t cols = spread.columns.size();
	int rows = 0;
	for (size_t j = 0; j < cols; ++j)
		rows = std::max((int)spread.columns.at(j).data.size(), rows);
	// alternative: int rows = excel.maxRows;
	DEBUG(Q_FUNC_INFO << ", cols/maxRows = " << cols << "/" << rows);

	// TODO QLocale locale = mw->locale();

	spreadsheet->setRowCount(rows);
	spreadsheet->setColumnCount((int)cols);
	if (sheetIndex == -1)
		spreadsheet->setComment(QString::fromStdString(spread.label));
	else // TODO: only first spread should get the comments
		spreadsheet->setComment(QString::fromStdString(excel.label));

	// in Origin column width is measured in characters, we need to convert to pixels
	// TODO: determine the font used in Origin in order to get the same column width as in Origin
	QFont font;
	QFontMetrics fm(font);
	const int scaling_factor = fm.maxWidth();

	for (size_t j = 0; j < cols; ++j) {
		auto column = spread.columns[j];
		auto* col = spreadsheet->column((int)j);

		DEBUG(Q_FUNC_INFO << ", column " << j << ", name = " << column.name << ", dataset name = " << column.dataset_name)
		QString name(QString::fromStdString(column.name));
		col->setName(name.remove(QRegularExpression(QStringLiteral(".*_"))));

		if (preview)
			continue;

		// TODO: we don't support any formulas for cells yet.
		DEBUG(Q_FUNC_INFO << ", column " << j << ", command = " << column.command)
		// 		if (column.command.size() > 0)
		// 			col->setFormula(Interval<int>(0, rows), QString::fromStdString(column.command));

		DEBUG(Q_FUNC_INFO << ", column " << j << ", full comment = " << column.comment)
		QString comment;
		if (m_originFile->version() < 9.5) // <= 2017 : pre-Unicode
			comment = QString::fromLatin1(column.comment.c_str());
		else
			comment = QString::fromStdString(column.comment);
		if (comment.contains(QLatin1Char('@'))) // remove @ options
			comment.truncate(comment.indexOf(QLatin1Char('@')));
		col->setComment(comment);
		col->setWidth((int)column.width * scaling_factor);

		// plot designation
		switch (column.type) {
		case Origin::SpreadColumn::X:
			col->setPlotDesignation(AbstractColumn::PlotDesignation::X);
			break;
		case Origin::SpreadColumn::Y:
			col->setPlotDesignation(AbstractColumn::PlotDesignation::Y);
			break;
		case Origin::SpreadColumn::Z:
			col->setPlotDesignation(AbstractColumn::PlotDesignation::Z);
			break;
		case Origin::SpreadColumn::XErr:
			col->setPlotDesignation(AbstractColumn::PlotDesignation::XError);
			break;
		case Origin::SpreadColumn::YErr:
			col->setPlotDesignation(AbstractColumn::PlotDesignation::YError);
			break;
		case Origin::SpreadColumn::Label:
		case Origin::SpreadColumn::NONE:
		default:
			col->setPlotDesignation(AbstractColumn::PlotDesignation::NoDesignation);
		}

		QString format;
		switch (column.valueType) {
		case Origin::Numeric: {
			for (unsigned int i = column.beginRow; i < column.endRow; ++i) {
				const double value = column.data.at(i).as_double();
				if (value != _ONAN)
					col->setValueAt(i, value);
			}

			loadColumnNumericFormat(column, col);
			break;
		}
		case Origin::TextNumeric: {
			// A TextNumeric column can contain numeric and string values, there is no equivalent column mode in LabPlot.
			//  -> Set the column mode as 'Numeric' or 'Text' depending on the type of first non-empty element in column.
			for (unsigned int i = column.beginRow; i < column.endRow; ++i) {
				const Origin::variant value(column.data.at(i));
				if (value.type() == Origin::Variant::V_DOUBLE) {
					if (value.as_double() != _ONAN)
						break;
				} else {
					if (value.as_string() != nullptr) {
						col->setColumnMode(AbstractColumn::ColumnMode::Text);
						break;
					}
				}
			}

			if (col->columnMode() == AbstractColumn::ColumnMode::Double) {
				for (unsigned int i = column.beginRow; i < column.endRow; ++i) {
					const double value = column.data.at(i).as_double();
					if (column.data.at(i).type() == Origin::Variant::V_DOUBLE && value != _ONAN)
						col->setValueAt(i, value);
				}
				loadColumnNumericFormat(column, col);
			} else {
				for (unsigned int i = column.beginRow; i < column.endRow; ++i) {
					const Origin::variant value(column.data.at(i));
					if (value.type() == Origin::Variant::V_STRING) {
						if (value.as_string() != nullptr)
							col->setTextAt(i, QLatin1String(value.as_string()));
					} else {
						if (value.as_double() != _ONAN)
							col->setTextAt(i, QString::number(value.as_double()));
					}
				}
			}
			break;
		}
		case Origin::Text:
			col->setColumnMode(AbstractColumn::ColumnMode::Text);
			for (int i = 0; i < std::min((int)column.data.size(), rows); ++i)
				col->setTextAt(i, QLatin1String(column.data[i].as_string()));
			break;
		case Origin::Time: {
			switch (column.valueTypeSpecification + 128) {
			case Origin::TIME_HH_MM:
				format = QStringLiteral("hh:mm");
				break;
			case Origin::TIME_HH:
				format = QStringLiteral("hh");
				break;
			case Origin::TIME_HH_MM_SS:
				format = QStringLiteral("hh:mm:ss");
				break;
			case Origin::TIME_HH_MM_SS_ZZ:
				format = QStringLiteral("hh:mm:ss.zzz");
				break;
			case Origin::TIME_HH_AP:
				format = QStringLiteral("hh ap");
				break;
			case Origin::TIME_HH_MM_AP:
				format = QStringLiteral("hh:mm ap");
				break;
			case Origin::TIME_MM_SS:
				format = QStringLiteral("mm:ss");
				break;
			case Origin::TIME_MM_SS_ZZ:
				format = QStringLiteral("mm:ss.zzz");
				break;
			case Origin::TIME_HHMM:
				format = QStringLiteral("hhmm");
				break;
			case Origin::TIME_HHMMSS:
				format = QStringLiteral("hhmmss");
				break;
			case Origin::TIME_HH_MM_SS_ZZZ:
				format = QStringLiteral("hh:mm:ss.zzz");
				break;
			}

			for (int i = 0; i < std::min((int)column.data.size(), rows); ++i)
				col->setValueAt(i, column.data[i].as_double());
			col->setColumnMode(AbstractColumn::ColumnMode::DateTime);

			auto* filter = static_cast<DateTime2StringFilter*>(col->outputFilter());
			filter->setFormat(format);
			break;
		}
		case Origin::Date: {
			switch (column.valueTypeSpecification) {
			case Origin::DATE_DD_MM_YYYY:
				format = QStringLiteral("dd/MM/yyyy");
				break;
			case Origin::DATE_DD_MM_YYYY_HH_MM:
				format = QStringLiteral("dd/MM/yyyy HH:mm");
				break;
			case Origin::DATE_DD_MM_YYYY_HH_MM_SS:
				format = QStringLiteral("dd/MM/yyyy HH:mm:ss");
				break;
			case Origin::DATE_DDMMYYYY:
			case Origin::DATE_DDMMYYYY_HH_MM:
			case Origin::DATE_DDMMYYYY_HH_MM_SS:
				format = QStringLiteral("dd.MM.yyyy");
				break;
			case Origin::DATE_MMM_D:
				format = QStringLiteral("MMM d");
				break;
			case Origin::DATE_M_D:
				format = QStringLiteral("M/d");
				break;
			case Origin::DATE_D:
				format = QLatin1Char('d');
				break;
			case Origin::DATE_DDD:
			case Origin::DATE_DAY_LETTER:
				format = QStringLiteral("ddd");
				break;
			case Origin::DATE_YYYY:
				format = QStringLiteral("yyyy");
				break;
			case Origin::DATE_YY:
				format = QStringLiteral("yy");
				break;
			case Origin::DATE_YYMMDD:
			case Origin::DATE_YYMMDD_HH_MM:
			case Origin::DATE_YYMMDD_HH_MM_SS:
			case Origin::DATE_YYMMDD_HHMM:
			case Origin::DATE_YYMMDD_HHMMSS:
				format = QStringLiteral("yyMMdd");
				break;
			case Origin::DATE_MMM:
			case Origin::DATE_MONTH_LETTER:
				format = QStringLiteral("MMM");
				break;
			case Origin::DATE_M_D_YYYY:
				format = QStringLiteral("M-d-yyyy");
				break;
			default:
				format = QStringLiteral("dd.MM.yyyy");
			}

			for (int i = 0; i < std::min((int)column.data.size(), rows); ++i)
				col->setValueAt(i, column.data[i].as_double());
			col->setColumnMode(AbstractColumn::ColumnMode::DateTime);

			auto* filter = static_cast<DateTime2StringFilter*>(col->outputFilter());
			filter->setFormat(format);
			break;
		}
		case Origin::Month: {
			switch (column.valueTypeSpecification) {
			case Origin::MONTH_MMM:
				format = QStringLiteral("MMM");
				break;
			case Origin::MONTH_MMMM:
				format = QStringLiteral("MMMM");
				break;
			case Origin::MONTH_LETTER:
				format = QLatin1Char('M');
				break;
			}

			for (int i = 0; i < std::min((int)column.data.size(), rows); ++i)
				col->setValueAt(i, column.data[i].as_double());
			col->setColumnMode(AbstractColumn::ColumnMode::Month);

			auto* filter = static_cast<DateTime2StringFilter*>(col->outputFilter());
			filter->setFormat(format);
			break;
		}
		case Origin::Day: {
			switch (column.valueTypeSpecification) {
			case Origin::DAY_DDD:
				format = QStringLiteral("ddd");
				break;
			case Origin::DAY_DDDD:
				format = QStringLiteral("dddd");
				break;
			case Origin::DAY_LETTER:
				format = QLatin1Char('d');
				break;
			}

			for (int i = 0; i < std::min((int)column.data.size(), rows); ++i)
				col->setValueAt(i, column.data[i].as_double());
			col->setColumnMode(AbstractColumn::ColumnMode::Day);

			auto* filter = static_cast<DateTime2StringFilter*>(col->outputFilter());
			filter->setFormat(format);
			break;
		}
		case Origin::ColumnHeading:
		case Origin::TickIndexedDataset:
		case Origin::Categorical:
			break;
		}
	}

	// TODO: "hidden" not supported yet
	//	if (spread.hidden || spread.loose)
	//		mw->hideWindow(spreadsheet);

	return true;
}

void OriginProjectParser::loadColumnNumericFormat(const Origin::SpreadColumn& originColumn, Column* column) const {
	if (originColumn.numericDisplayType != 0) {
		int fi = 0;
		switch (originColumn.valueTypeSpecification) {
		case Origin::Decimal:
			fi = 1;
			break;
		case Origin::Scientific:
			fi = 2;
			break;
		case Origin::Engineering:
		case Origin::DecimalWithMarks:
			break;
		}

		auto* filter = static_cast<Double2StringFilter*>(column->outputFilter());
		filter->setNumericFormat(fi);
		filter->setNumDigits(originColumn.decimalPlaces);
	}
}

bool OriginProjectParser::loadMatrixWorkbook(Workbook* workbook, bool preview) {
	DEBUG(Q_FUNC_INFO)
	// load matrix workbook sheets
	const auto& originMatrix = m_originFile->matrix(findMatrixByName(workbook->name()));
	for (size_t s = 0; s < originMatrix.sheets.size(); ++s) {
		auto* matrix = new Matrix(QString::fromStdString(originMatrix.sheets[s].name));
		loadMatrix(matrix, preview, s, workbook->name());
		workbook->addChildFast(matrix);
	}

	return true;
}

bool OriginProjectParser::loadMatrix(Matrix* matrix, bool preview, size_t sheetIndex, const QString& mwbName) {
	DEBUG(Q_FUNC_INFO)
	// import matrix data
	const auto& originMatrix = m_originFile->matrix(findMatrixByName(mwbName));

	if (preview)
		return true;

	// in Origin column width is measured in characters, we need to convert to pixels
	// TODO: determine the font used in Origin in order to get the same column width as in Origin
	QFont font;
	QFontMetrics fm(font);
	const int scaling_factor = fm.maxWidth();

	const auto& layer = originMatrix.sheets.at(sheetIndex);
	const int colCount = layer.columnCount;
	const int rowCount = layer.rowCount;

	matrix->setRowCount(rowCount);
	matrix->setColumnCount(colCount);
	matrix->setFormula(QString::fromStdString(layer.command));

	// TODO: how to handle different widths for different columns?
	for (int j = 0; j < colCount; j++)
		matrix->setColumnWidth(j, layer.width * scaling_factor);

	// TODO: check column major vs. row major to improve the performance here
	for (int i = 0; i < rowCount; i++) {
		for (int j = 0; j < colCount; j++)
			matrix->setCell(i, j, layer.data[j + i * colCount]);
	}

	char format = 'g';
	// TODO: prec not support by Matrix
	// int prec = 6;
	switch (layer.valueTypeSpecification) {
	case 0: // Decimal 1000
		format = 'f';
		//	prec = layer.decimalPlaces;
		break;
	case 1: // Scientific
		format = 'e';
		//	prec = layer.decimalPlaces;
		break;
	case 2: // Engineering
	case 3: // Decimal 1,000
		format = 'g';
		//	prec = layer.significantDigits;
		break;
	}

	matrix->setNumericFormat(format);

	return true;
}

bool OriginProjectParser::loadWorksheet(Worksheet* worksheet, bool preview) {
	DEBUG(Q_FUNC_INFO << ", preview = " << preview)
	if (worksheet->parentAspect())
		DEBUG(Q_FUNC_INFO << ", parent PATH " << STDSTRING(worksheet->parentAspect()->path()))

	// load worksheet data
	const auto& graph = m_originFile->graph(findWorksheetByName(worksheet->name()));
	DEBUG(Q_FUNC_INFO << ", worksheet name = " << graph.name);
	worksheet->setComment(QLatin1String(graph.label.c_str()));

	// TODO: width, height, view mode (print view, page view, window view, draft view)
	// Origin allows to freely resize the window and ajusts the size of the plot (layer) automatically
	// by keeping a certain width-to-height ratio. It's not clear what the actual size of the plot/layer is and how to handle this.
	// For now we simply create a new worksheet here with it's default size and make it using the whole view size.
	// Later we can decide to use one of the following properties:
	//  1) Window.frameRect gives Rect-corner coordinates (in pixels) of the Window object
	//  2) GraphLayer.clientRect gives Rect-corner coordinates (pixels) of the Layer inside the (printer?) page.
	//  3) Graph.width, Graph.height give the (printer?) page size in pixels.
	// 	const QRectF size(0, 0,
	// 			Worksheet::convertToSceneUnits(graph.width/600., Worksheet::Inch),
	// 			Worksheet::convertToSceneUnits(graph.height/600., Worksheet::Inch));
	// 	worksheet->setPageRect(size);
	graphSize.rwidth() = graph.width;
	graphSize.rheight() = graph.height;
	WARN(Q_FUNC_INFO << ", GRAPH width/height (px) = " << graphSize.width() << "/" << graphSize.height())
	// Graphic elements in Origin are scaled relative to the dimensions of the page (Format->Page) with 600 DPI (>=9.6), 300 DPI (<9.6)
	double dpi = 600.;
	if (m_originFile->version() < 9.6)
		dpi = 300.;
	WARN(Q_FUNC_INFO << ", GRAPH width/height (cm) = " << graphSize.width() * GSL_CONST_CGS_INCH / dpi << "/" << graphSize.height() * GSL_CONST_CGS_INCH / dpi)
	// Origin scales text and plots with the size of the layer when no fixed size is used (Layer properties->Size)
	// so we scale all text and plots with a scaling factor to the whole view height used as default
#if defined(HAVE_WINDOWS)
	const double fixedHeight = 14.75; // full height/2 [cm]
#else
	const double fixedHeight = 29.5; // full height [cm]
#endif
	elementScalingFactor = fixedHeight / (graph.height * GSL_CONST_CGS_INCH / dpi);
	// not using the full value for scaling text is better in most cases
	textScalingFactor = 1. + (elementScalingFactor - 1.) / 2.;
	WARN(Q_FUNC_INFO << ", ELEMENT SCALING FACTOR = " << elementScalingFactor)
	WARN(Q_FUNC_INFO << ", TEXT SCALING FACTOR = " << textScalingFactor)
	// default values (1000/1000)
	//	DEBUG(Q_FUNC_INFO << ", WORKSHEET width/height = " << worksheet->pageRect().width() << "/" << worksheet->pageRect().height())

	worksheet->setUseViewSize(true);

	QHash<TextLabel*, QSizeF> textLabelPositions;

	// worksheet background color
	const Origin::ColorGradientDirection bgColorGradient = graph.windowBackgroundColorGradient;
	const Origin::Color bgBaseColor = graph.windowBackgroundColorBase;
	const Origin::Color bgEndColor = graph.windowBackgroundColorEnd;
	worksheet->background()->setColorStyle(backgroundColorStyle(bgColorGradient));
	switch (bgColorGradient) {
	case Origin::ColorGradientDirection::NoGradient:
	case Origin::ColorGradientDirection::TopLeft:
	case Origin::ColorGradientDirection::Left:
	case Origin::ColorGradientDirection::BottomLeft:
	case Origin::ColorGradientDirection::Top:
		worksheet->background()->setFirstColor(color(bgEndColor));
		worksheet->background()->setSecondColor(color(bgBaseColor));
		break;
	case Origin::ColorGradientDirection::Center:
		break;
	case Origin::ColorGradientDirection::Bottom:
	case Origin::ColorGradientDirection::TopRight:
	case Origin::ColorGradientDirection::Right:
	case Origin::ColorGradientDirection::BottomRight:
		worksheet->background()->setFirstColor(color(bgBaseColor));
		worksheet->background()->setSecondColor(color(bgEndColor));
	}

	// TODO: do we need changes on the worksheet layout?

	// process Origin's graph layers - add new plot areas or new coordinate system in the same plot area, depending on the global setting
	// https://www.originlab.com/doc/Origin-Help/MultiLayer-Graph
	int layerIndex = 0; // index of the graph layer
	CartesianPlot* plot = nullptr;
	Origin::Rect layerRect;
	for (const auto& layer : graph.layers) {
		DEBUG(Q_FUNC_INFO << ", Graph Layer " << layerIndex + 1)
		if (layer.is3D()) {
			// TODO: add an "UnsupportedAspect" here since we don't support 3D yet
			return false;
		}

		layerRect = layer.clientRect;
		// DEBUG(Q_FUNC_INFO << ", layer left/right (px) = " << layerRect.left << "/" << layerRect.right)
		// DEBUG(Q_FUNC_INFO << ", layer top/bottom (px) = " << layerRect.top << "/" << layerRect.bottom)
		// DEBUG(Q_FUNC_INFO << ", layer width/height (px) = " << layerRect.width() << "/" << layerRect.height())

		// create a new plot if we're
		// 1. interpreting every layer as a new plot
		// 2. interpreting every layer as a new coordinate system in the same and single plot and no plot was created yet
		DEBUG(Q_FUNC_INFO << ", layer as plot area = " << m_graphLayerAsPlotArea)
		if (m_graphLayerAsPlotArea || (!m_graphLayerAsPlotArea && !plot)) {
			plot = new CartesianPlot(i18n("Plot%1", QString::number(layerIndex + 1)));
			worksheet->addChildFast(plot);
			plot->setIsLoading(true);
		}

		loadGraphLayer(layer, plot, layerIndex, textLabelPositions, preview);
		++layerIndex;
	}

	// padding
	if (plot) {
		plot->setSymmetricPadding(false);
		int numberOfLayer = layerIndex + 1;
		WARN(Q_FUNC_INFO << ", number of layer = " << numberOfLayer)
		if (numberOfLayer == 1 || !m_graphLayerAsPlotArea) { // use layer clientRect for padding
			WARN(Q_FUNC_INFO << ", using layer rect for padding")
			double aspectRatio = (double)graphSize.width() / graphSize.height();

			const double leftPadding = layerRect.left / (double)graphSize.width() * aspectRatio * fixedHeight;
			const double topPadding = layerRect.top / (double)graphSize.height() * fixedHeight;
			const double rightPadding = (graphSize.width() - layerRect.right) / (double)graphSize.width() * aspectRatio * fixedHeight;
			const double bottomPadding = (graphSize.height() - layerRect.bottom) / (double)graphSize.height() * fixedHeight;
			plot->setHorizontalPadding(Worksheet::convertToSceneUnits(leftPadding, Worksheet::Unit::Centimeter));
			plot->setVerticalPadding(Worksheet::convertToSceneUnits(topPadding, Worksheet::Unit::Centimeter));
			plot->setRightPadding(Worksheet::convertToSceneUnits(rightPadding, Worksheet::Unit::Centimeter));
			plot->setBottomPadding(Worksheet::convertToSceneUnits(bottomPadding, Worksheet::Unit::Centimeter));
		} else {
			WARN(Q_FUNC_INFO << ", using fixed padding")
#if defined(HAVE_WINDOWS)
			// TODO: test if min instead of max is relevant
			plot->setHorizontalPadding(100. + 1.5 * plot->horizontalPadding() * std::min(elementScalingFactor, 1.));
			plot->setVerticalPadding(100. + 1.5 * plot->verticalPadding() * std::min(elementScalingFactor, 1.));
			plot->setRightPadding(100. + 1.5 * plot->rightPadding() * std::min(elementScalingFactor, 1.));
			plot->setBottomPadding(100. + 1.5 * plot->bottomPadding() * std::min(elementScalingFactor, 1.));
#else
			plot->setHorizontalPadding(100. + 1.5 * plot->horizontalPadding() * std::max(elementScalingFactor, 1.));
			plot->setVerticalPadding(100. + 1.5 * plot->verticalPadding() * std::max(elementScalingFactor, 1.));
			plot->setRightPadding(100. + 1.5 * plot->rightPadding() * std::max(elementScalingFactor, 1.));
			plot->setBottomPadding(100. + 1.5 * plot->bottomPadding() * std::max(elementScalingFactor, 1.));
#endif
		}
		WARN(Q_FUNC_INFO << ", PADDING (H/V) = " << plot->horizontalPadding() << ", " << plot->verticalPadding())
		WARN(Q_FUNC_INFO << ", PADDING (R/B) = " << plot->rightPadding() << ", " << plot->bottomPadding())
	}

	if (!preview) {
		worksheet->updateLayout();

		// worksheet and plots got their sizes,
		//-> position all text labels inside the plots correctly by converting
		// the relative positions determined above to the absolute values
		auto it = textLabelPositions.constBegin();
		while (it != textLabelPositions.constEnd()) {
			auto* label = it.key();
			const auto& ratios = it.value();

			auto position = label->position();
			position.point.setX(ratios.width());
			position.point.setY(ratios.height());
			position.horizontalPosition = WorksheetElement::HorizontalPosition::Relative;
			position.verticalPosition = WorksheetElement::VerticalPosition::Relative;
			// achor depending on rotation
			auto rotation = label->rotationAngle();
			auto hAlign = WorksheetElement::HorizontalAlignment::Left;
			auto vAlign = WorksheetElement::VerticalAlignment::Top;
			if (rotation > 45 && rotation <= 135) // left/bottom
				vAlign = WorksheetElement::VerticalAlignment::Bottom;
			else if (rotation > 135 && rotation <= 225) { // right/bottom
				hAlign = WorksheetElement::HorizontalAlignment::Right;
				vAlign = WorksheetElement::VerticalAlignment::Bottom;
			} else if (rotation > 225) // right/top
				hAlign = WorksheetElement::HorizontalAlignment::Right;

			label->setHorizontalAlignment(hAlign);
			label->setVerticalAlignment(vAlign);
			label->setPosition(position);

			++it;
		}
	}

	DEBUG(Q_FUNC_INFO << " DONE");
	return true;
}

void OriginProjectParser::loadGraphLayer(const Origin::GraphLayer& layer,
										 CartesianPlot* plot,
										 int layerIndex,
										 QHash<TextLabel*, QSizeF>& textLabelPositions,
										 bool preview) {
	DEBUG(Q_FUNC_INFO << ", NEW GRAPH LAYER")

	// background color
	const auto& regColor = layer.backgroundColor;
	if (regColor.type == Origin::Color::None)
		plot->plotArea()->background()->setOpacity(0);
	else
		plot->plotArea()->background()->setFirstColor(color(regColor));

	// border
	if (layer.borderType == Origin::BorderType::None)
		plot->plotArea()->borderLine()->setStyle(Qt::NoPen);
	else
		plot->plotArea()->borderLine()->setStyle(Qt::SolidLine);

	// ranges: swap axes when exchanged
	const auto& originXAxis = layer.exchangedAxes ? layer.yAxis : layer.xAxis;
	const auto& originYAxis = layer.exchangedAxes ? layer.xAxis : layer.yAxis;

	Range<double> xRange(originXAxis.min, originXAxis.max);
	Range<double> yRange(originYAxis.min, originYAxis.max);
	xRange.setAutoScale(false);
	yRange.setAutoScale(false);

	if (m_graphLayerAsPlotArea) { // graph layer is read as a new plot area
		// set the ranges for default coordinate system
		plot->setRangeDefault(Dimension::X, xRange);
		plot->setRangeDefault(Dimension::Y, yRange);
	} else { // graph layer is read as a new coordinate system in the same plot area
		// create a new coordinate systems and set the ranges for it
		if (layerIndex > 0) {
			// check if identical range already exists
			int selectedXRangeIndex = -1;
			for (int i = 0; i < plot->rangeCount(Dimension::X); i++) {
				const auto& range = plot->range(Dimension::X, i);
				if (range == xRange) {
					selectedXRangeIndex = i;
					break;
				}
			}
			int selectedYRangeIndex = -1;
			for (int i = 0; i < plot->rangeCount(Dimension::Y); i++) {
				const auto& range = plot->range(Dimension::Y, i);
				if (range == yRange) {
					selectedYRangeIndex = i;
					break;
				}
			}

			if (selectedXRangeIndex < 0) {
				plot->addXRange();
				selectedXRangeIndex = plot->rangeCount(Dimension::X) - 1;
			}
			if (selectedYRangeIndex < 0) {
				plot->addYRange();
				selectedYRangeIndex = plot->rangeCount(Dimension::Y) - 1;
			}

			plot->addCoordinateSystem();
			// set ranges for new coordinate system
			plot->setCoordinateSystemRangeIndex(layerIndex, Dimension::X, selectedXRangeIndex);
			plot->setCoordinateSystemRangeIndex(layerIndex, Dimension::Y, selectedYRangeIndex);
		}
		plot->setRange(Dimension::X, layerIndex, xRange);
		plot->setRange(Dimension::Y, layerIndex, yRange);
	}

	// scales
	plot->setXRangeScale(scale(originXAxis.scale));
	plot->setYRangeScale(scale(originYAxis.scale));

	// add legend if available
	const auto& originLegend = layer.legend;
	if (!originLegend.text.empty()) {
		QString legendText;
		// not using UTF8! (9.85, TO)
		legendText = QString::fromLatin1(originLegend.text.c_str());
		DEBUG(Q_FUNC_INFO << ", legend text = \"" << STDSTRING(legendText) << "\"");

		auto* legend = new CartesianPlotLegend(i18n("legend"));

		plot->addLegend(legend);

		// set legend text size
		DEBUG(Q_FUNC_INFO << ", legend text size = " << originLegend.fontSize);
		auto labelFont = legend->labelFont();
		labelFont.setPointSize(Worksheet::convertToSceneUnits(originLegend.fontSize * textScalingFactor, Worksheet::Unit::Point));
		legend->setLabelFont(labelFont);

		// Origin's legend uses "\l(...)" or "\L(...)" string to format the legend symbol
		//  and "%(...)" to format the legend text for each curve
		// s. a. https://www.originlab.com/doc/Origin-Help/Legend-ManualControl
		// the text before these formatting tags, if available, is interpreted as the legend title

		// search for the first occurrence of the legend symbol substring
		int index = legendText.indexOf(QLatin1String("\\l("), 0, Qt::CaseInsensitive);
		QString legendTitle;
		if (index != -1)
			legendTitle = legendText.left(index);
		else {
			// check legend text
			index = legendText.indexOf(QLatin1String("%("));
			if (index != -1)
				legendTitle = legendText.left(index);
		}

		legendTitle = legendTitle.trimmed();
		if (!legendTitle.isEmpty())
			legendTitle = parseOriginText(legendTitle);
		if (!legendTitle.isEmpty()) {
			DEBUG(Q_FUNC_INFO << ", legend title = \"" << STDSTRING(legendTitle) << "\"");
			legend->title()->setText(legendTitle);
		} else {
			DEBUG(Q_FUNC_INFO << ", legend title is empty");
		}

		// TODO: text color
		// const Origin::Color& originColor = originLegend.color;

		// position
		// TODO: In Origin the legend can be placed outside of the plot which is not possible in LabPlot.
		// To achieve this we'll need to increase padding area in the plot to place the legend outside of the plot area.
		// graphSize (% of page), layer.clientRect (% of layer) -> see loadWorksheet()
		auto legendRect = originLegend.clientRect;
		// auto layerRect = layer.clientRect; // for % of layer
		DEBUG(Q_FUNC_INFO << ", LEGEND position (l/t) << " << legendRect.left << "/" << legendRect.top)
		DEBUG(Q_FUNC_INFO << ", page size = " << graphSize.width() << "/" << graphSize.height())
		CartesianPlotLegend::PositionWrapper position;
		QSizeF relativePosition(legendRect.left / (double)graphSize.width(), legendRect.top / (double)graphSize.height());
		// achor depending on rotation
		auto rotation = originLegend.rotation;
		if (rotation > 45 && rotation <= 135) // left/bottom
			relativePosition.setHeight(legendRect.bottom / (double)graphSize.height());
		else if (rotation > 135 && rotation <= 225) { // right/bottom
			relativePosition.setWidth(legendRect.right / (double)graphSize.width());
			relativePosition.setHeight(legendRect.bottom / (double)graphSize.height());
		} else if (rotation > 225) // right/top
			relativePosition.setWidth(legendRect.right / (double)graphSize.width());

		DEBUG(Q_FUNC_INFO << ", relative position to page = " << relativePosition.width() << "/" << relativePosition.height())

		position.point.setX(relativePosition.width());
		position.point.setY(relativePosition.height());
		// achor depending on rotation
		position.horizontalPosition = WorksheetElement::HorizontalPosition::Relative;
		position.verticalPosition = WorksheetElement::VerticalPosition::Relative;
		auto hAlign = WorksheetElement::HorizontalAlignment::Left;
		auto vAlign = WorksheetElement::VerticalAlignment::Top;
		if (rotation > 45 && rotation <= 135) // left/bottom
			vAlign = WorksheetElement::VerticalAlignment::Bottom;
		else if (rotation > 135 && rotation <= 225) { // right/bottom
			hAlign = WorksheetElement::HorizontalAlignment::Right;
			vAlign = WorksheetElement::VerticalAlignment::Bottom;
		} else if (rotation > 225) // right/top
			hAlign = WorksheetElement::HorizontalAlignment::Right;
		legend->setHorizontalAlignment(hAlign);
		legend->setVerticalAlignment(vAlign);

		legend->setPosition(position);

		// rotation
		legend->setRotationAngle(originLegend.rotation);

		// border line
		if (originLegend.borderType == Origin::BorderType::None)
			legend->borderLine()->setStyle(Qt::NoPen);
		else
			legend->borderLine()->setStyle(Qt::SolidLine);

		// background color, determine it with the help of the border type
		if (originLegend.borderType == Origin::BorderType::DarkMarble)
			legend->background()->setFirstColor(Qt::darkGray);
		else if (originLegend.borderType == Origin::BorderType::BlackOut)
			legend->background()->setFirstColor(Qt::black);
		else
			legend->background()->setFirstColor(Qt::white);

		// save current legend text
		m_legendText = std::move(legendText);
	}

	// curves
	loadCurves(layer, plot, layerIndex, preview);

	// reading of other properties is not relevant for the dependency checks in the preview, skip them
	if (preview)
		return;

	// texts
	for (const auto& t : layer.texts) {
		DEBUG(Q_FUNC_INFO << ", EXTRA TEXT = " << t.text);
		auto* label = new TextLabel(QStringLiteral("text label"));
		QString text;
		if (m_originFile->version() < 9.5) // <= 2017 : pre-Unicode
			text = QString::fromLatin1(t.text.c_str());
		else
			text = QString::fromStdString(t.text);
		QTextEdit te(parseOriginText(text));
		te.selectAll();
		DEBUG(Q_FUNC_INFO << ", font size = " << t.fontSize)
		te.setFontPointSize(int(t.fontSize)); // no scaling
		te.setTextColor(OriginProjectParser::color(t.color));
		label->setText(te.toHtml());
		// DEBUG(" TEXT = " << STDSTRING(label->text().text))

		plot->addChild(label);
		label->setParentGraphicsItem(plot->graphicsItem());

		// position
		// determine the relative position to the graph
		QSizeF relativePosition(t.clientRect.left / (double)graphSize.width(), t.clientRect.top / (double)graphSize.height());
		// achor depending on rotation
		if (t.rotation > 45 && t.rotation <= 135) // left/bottom
			relativePosition.setHeight(t.clientRect.bottom / (double)graphSize.height());
		else if (t.rotation > 135 && t.rotation <= 225) { // right/bottom
			relativePosition.setWidth(t.clientRect.right / (double)graphSize.width());
			relativePosition.setHeight(t.clientRect.bottom / (double)graphSize.height());
		} else if (t.rotation > 225) // right/top
			relativePosition.setWidth(t.clientRect.right / (double)graphSize.height());

		DEBUG(Q_FUNC_INFO << ", relative position to page = " << relativePosition.width() << "/" << relativePosition.height())
		textLabelPositions[label] = relativePosition;

		// rotation
		label->setRotationAngle(t.rotation);

		// TODO:
		// int tab;
		// BorderType borderType;
		// Attach attach;
	}

	// axes
	DEBUG(Q_FUNC_INFO << ", number of curves in layer = " << layer.curves.size())
	if (layer.curves.empty()) // no curves, just axes
		loadAxes(layer, plot, layerIndex, QLatin1String("X Axis Title"), QLatin1String("Y Axis Title"));
	else {
		const auto& originCurve = layer.curves.at(0);
		// see loadCurves()
		QString dataName(QString::fromStdString(originCurve.dataName));
		DEBUG(Q_FUNC_INFO << ", curve data name: " << STDSTRING(dataName))
		QString containerName = dataName.right(dataName.length() - 2); // strip "E_" or "T_"
		auto sheet = getSpreadsheetByName(containerName);

		DEBUG("number of columns = " << sheet.columns.size())
		if (sheet.columns.size() == 0) {
			DEBUG(Q_FUNC_INFO << ", WARNING: no columns in sheet")
			return;
		}

		QString xColumnName;
		if (m_originFile->version() < 9.5) // <= 2017 : pre-Unicode
			xColumnName = QString::fromLatin1(originCurve.xColumnName.c_str());
		else
			xColumnName = QString::fromStdString(originCurve.xColumnName);

		const auto& xColumn = sheet.columns[findColumnByName(sheet, xColumnName)];
		QString xColumnInfo = xColumnName;
		if (xColumn.comment.length() > 0) { // long name(, unit(, comment))
			if (m_originFile->version() < 9.5) // <= 2017 : pre-Unicode
				xColumnInfo = QString::fromLatin1(xColumn.comment.c_str());
			else
				xColumnInfo = QString::fromStdString(xColumn.comment);
			if (xColumnInfo.contains(QLatin1Char('@'))) // remove @ options
				xColumnInfo.truncate(xColumnInfo.indexOf(QLatin1Char('@')));
		}
		DEBUG(Q_FUNC_INFO << ", x column name = " << STDSTRING(xColumnName));
		DEBUG(Q_FUNC_INFO << ", x column info = " << STDSTRING(xColumnInfo));

		// same for y
		QString yColumnName;
		if (m_originFile->version() < 9.5) // <= 2017 : pre-Unicode
			yColumnName = QString::fromLatin1(originCurve.yColumnName.c_str());
		else
			yColumnName = QString::fromStdString(originCurve.yColumnName);

		const auto& yColumn = sheet.columns[findColumnByName(sheet, yColumnName)];
		QString yColumnInfo = yColumnName;
		if (yColumn.comment.length() > 0) { // long name(, unit(, comment))
			if (m_originFile->version() < 9.5) // <= 2017 : pre-Unicode
				yColumnInfo = QString::fromLatin1(yColumn.comment.c_str());
			else
				yColumnInfo = QString::fromStdString(yColumn.comment);
			if (yColumnInfo.contains(QLatin1Char('@'))) // remove @ options
				yColumnInfo.truncate(yColumnInfo.indexOf(QLatin1Char('@')));
		}
		DEBUG(Q_FUNC_INFO << ", y column name = " << STDSTRING(yColumnName));
		DEBUG(Q_FUNC_INFO << ", y column info = " << STDSTRING(yColumnInfo));

		// type specific settings
		const auto type = originCurve.type;
		// for histogram use y column info for x column
		if (type == Origin::GraphCurve::Histogram && xColumnInfo.isEmpty())
			xColumnInfo = yColumnInfo;
		// for bar plot reverse x and y column info
		if (type == Origin::GraphCurve::Bar || type == Origin::GraphCurve::BarStack)
			xColumnInfo.swap(yColumnInfo);

		loadAxes(layer, plot, layerIndex, xColumnInfo, yColumnInfo);
	}

	// TODO: range breaks
	DEBUG(Q_FUNC_INFO << ", DONE")
}

void OriginProjectParser::loadCurves(const Origin::GraphLayer& layer, CartesianPlot* plot, int layerIndex, bool preview) {
	DEBUG(Q_FUNC_INFO << "layer " << layerIndex)

	int curveIndex = 1;
	for (const auto& originCurve : layer.curves) {
		QString dataName(QString::fromStdString(originCurve.dataName));
		DEBUG(Q_FUNC_INFO << ", NEW CURVE. data name = " << STDSTRING(dataName))
		DEBUG(Q_FUNC_INFO << ", curve x column name = " << originCurve.xColumnName)
		DEBUG(Q_FUNC_INFO << ", curve y column name = " << originCurve.yColumnName)
		DEBUG(Q_FUNC_INFO << ", curve x data name = " << originCurve.xDataName)

		if (dataName.isEmpty()) // formula may be empty?
			continue;

		Plot* childPlot{nullptr};

		// handle the different data sources for plots (spreadsheet, workbook, matrix and function)
		switch (dataName.at(0).toLatin1()) {
		case 'T': // Spreadsheet
		case 'E': { // Workbook
			// determine the used columns first
			QString containerName = dataName.right(dataName.length() - 2); // strip "E_" or "T_"
			const auto& sheet = getSpreadsheetByName(containerName);
			QString tableName = containerName;
			if (dataName.startsWith(QStringLiteral("E_"))) // container is a workbook
				tableName += QLatin1Char('/') + QString::fromStdString(sheet.name);

			QString xColumnName = QLatin1String(originCurve.xColumnName.c_str());
			QString yColumnName = QLatin1String(originCurve.yColumnName.c_str());
			QString xColumnPath = tableName + QLatin1Char('/') + xColumnName;
			QString yColumnPath = tableName + QLatin1Char('/') + yColumnName;
			DEBUG(Q_FUNC_INFO << ", x/y column path = \"" << STDSTRING(xColumnPath) << "\" \"" << STDSTRING(yColumnPath) << "\"")

			const auto type = originCurve.type;
			DEBUG(Q_FUNC_INFO << ", curve type = " << (int)type)
			switch (type) {
			case Origin::GraphCurve::Line:
			case Origin::GraphCurve::Scatter:
			case Origin::GraphCurve::LineSymbol:
			case Origin::GraphCurve::ErrorBar:
			case Origin::GraphCurve::XErrorBar:
			case Origin::GraphCurve::YErrorBar:
			case Origin::GraphCurve::XYErrorBar: {
				const auto columnName(QString::fromStdString(originCurve.yColumnName));
				const auto& column = sheet.columns[findColumnByName(sheet, columnName)];
				QString shortName = columnName, curveName = columnName;
				QString longName, unit, comments;
				if (column.comment.length() > 0) {
					auto columnInfo = QString::fromStdString(column.comment); // long name(, unit(, comment))
					DEBUG(Q_FUNC_INFO << ", y column full comment = \"" << column.comment << "\"")
					if (columnInfo.contains(QLatin1Char('@'))) // remove @ options
						columnInfo.truncate(columnInfo.indexOf(QLatin1Char('@')));

					parseColumnInfo(columnInfo, longName, unit, comments);
				}
				if (longName.isEmpty())
					longName = shortName;
				if (comments.isEmpty())
					comments = longName;
				curveName = comments;
				DEBUG(Q_FUNC_INFO << ", default curve name = \"" << curveName.toStdString() << "\"")

				// TODO: custom legend not used yet
				// Origin's legend uses "%(...) to format the legend text for each curve
				// s. a. https://www.originlab.com/doc/Origin-Help/Legend-ManualControl

				// parse and use legend text (not used)
				// find substring between %c{curveIndex} and %c{curveIndex+1}
				// int pos1 = legendText.indexOf(QStringLiteral("\\c{%1}").arg(curveIndex)) + 5;
				// int pos2 = legendText.indexOf(QStringLiteral("\\c{%1}").arg(curveIndex + 1));
				// QString curveText = legendText.mid(pos1, pos2 - pos1);
				// replace %(1), %(2), etc. with curve name
				// curveText.replace(QStringLiteral("%(%1)").arg(curveIndex), legendText);

				// curveText = curveText.trimmed();
				// DEBUG(" curve " << curveIndex << " text = \"" << STDSTRING(curveText) << "\"");
				// TODO: curve (legend) does not support HTML text yet.
				// auto* curve = new XYCurve(curveText);

				// check if curve is in actual legendText
				// examples:  \l(1) %(1), \l(2) %(2) text, \l(3) %(3,@LG), ..
				DEBUG(Q_FUNC_INFO << ", LEGEND TEXT = \"" << m_legendText.toStdString() << "\"")
				// DEBUG(Q_FUNC_INFO << ", layer index = " << layerIndex + 1 << ", curve index = " << curveIndex)
				bool enableCurveInLegend = false;
				QString legendCurveString;
				// find \l(C)
				int pos1 = m_legendText.indexOf(QStringLiteral("\\l(%1)").arg(curveIndex));
				if (pos1 == -1) // try \l(L.C)
					pos1 = m_legendText.indexOf(QStringLiteral("\\l(%1.%2)").arg(layerIndex + 1).arg(curveIndex));
				else // remove symbol string
					m_legendText.remove(QStringLiteral("\\l(%1)").arg(curveIndex));

				if (pos1 != -1) { // \l(C) or \l(L.C) found
					// remove symbol string
					m_legendText.remove(QStringLiteral("\\l(%1.%2)").arg(layerIndex + 1).arg(curveIndex));

					// whole line
					int pos2 = m_legendText.indexOf(QRegularExpression(QLatin1String("[\r\n]")), pos1);
					if (pos2 == -1)
						legendCurveString = m_legendText.mid(pos1, pos2);
					else
						legendCurveString = m_legendText.mid(pos1, pos2 - pos1);
					DEBUG(Q_FUNC_INFO << ", legend curve string = \"" << legendCurveString.toStdString() << "\"")
					if (!legendCurveString.isEmpty()) { // don't include empty entries
						// replace %(C) and %(L.C)
						// see https://www.originlab.com/doc/en/LabTalk/ref/Text-Label-Options#Complete_List_of_.40Options
						QString unitString(unit.isEmpty() ? QStringLiteral("") : QStringLiteral(" (") + unit + QStringLiteral(")"));
						// TODO: implement more
						legendCurveString.replace(QStringLiteral("%(%1)").arg(curveIndex), curveName);
						legendCurveString.replace(QStringLiteral("%(%1,@C)").arg(curveIndex), shortName);
						legendCurveString.replace(QStringLiteral("%(%1,@L)").arg(curveIndex), longName);
						legendCurveString.replace(QStringLiteral("%(%1,@LA)").arg(curveIndex), longName);
						legendCurveString.replace(QStringLiteral("%(%1,@LC)").arg(curveIndex), comments);
						legendCurveString.replace(QStringLiteral("%(%1,@LG)").arg(curveIndex), longName + unitString);
						legendCurveString.replace(QStringLiteral("%(%1,@LL)").arg(curveIndex), longName);
						legendCurveString.replace(QStringLiteral("%(%1,@LM)").arg(curveIndex), comments);
						legendCurveString.replace(QStringLiteral("%(%1,@LN)").arg(curveIndex), comments + unitString);
						legendCurveString.replace(QStringLiteral("%(%1,@LS)").arg(curveIndex), shortName);
						legendCurveString.replace(QStringLiteral("%(%1,@LU)").arg(curveIndex), unit);

						// same with %(L.C)
						legendCurveString.replace(QStringLiteral("%(%1.%2)").arg(layerIndex + 1).arg(curveIndex), curveName);
						legendCurveString.replace(QStringLiteral("%(%1.%2,@C)").arg(layerIndex + 1).arg(curveIndex), shortName);
						legendCurveString.replace(QStringLiteral("%(%1.%2,@L)").arg(layerIndex + 1).arg(curveIndex), longName);
						legendCurveString.replace(QStringLiteral("%(%1.%2,@LA)").arg(layerIndex + 1).arg(curveIndex), longName);
						legendCurveString.replace(QStringLiteral("%(%1.%2,@LC)").arg(layerIndex + 1).arg(curveIndex), comments);
						legendCurveString.replace(QStringLiteral("%(%1.%2,@LG)").arg(layerIndex + 1).arg(curveIndex), longName + unitString);
						legendCurveString.replace(QStringLiteral("%(%1.%2,@LL)").arg(layerIndex + 1).arg(curveIndex), longName);
						legendCurveString.replace(QStringLiteral("%(%1.%2,@LM)").arg(layerIndex + 1).arg(curveIndex), comments);
						legendCurveString.replace(QStringLiteral("%(%1.%2,@LN)").arg(layerIndex + 1).arg(curveIndex), comments + unitString);
						legendCurveString.replace(QStringLiteral("%(%1.%2,@LS)").arg(layerIndex + 1).arg(curveIndex), shortName);
						legendCurveString.replace(QStringLiteral("%(%1.%2,@LU)").arg(layerIndex + 1).arg(curveIndex), unit);

						DEBUG(Q_FUNC_INFO << ", legend curve string final = \"" << legendCurveString.toStdString() << "\"")

						legendCurveString = legendCurveString.trimmed(); // remove leading and trailing whitspaces for curve name
						legendCurveString.remove(QRegularExpression(QStringLiteral("\\\\l\\(\\d+\\)"))); // remove left over "\l(X)" (TO)
						if (!legendCurveString.isEmpty())
							curveName = legendCurveString;

						enableCurveInLegend = true;
					}
				}

				DEBUG(Q_FUNC_INFO << ", curve name = \"" << curveName.toStdString() << "\", in legend = " << enableCurveInLegend)

				if (type == Origin::GraphCurve::Line || type == Origin::GraphCurve::Scatter || type == Origin::GraphCurve::LineSymbol) {
					DEBUG(Q_FUNC_INFO << ", line(symbol) or scatter curve")
					auto* curve = new XYCurve(curveName);
					childPlot = curve;
					curve->setXColumnPath(xColumnPath);
					curve->setYColumnPath(yColumnPath);

					curve->setSuppressRetransform(true);
					if (!preview) {
						loadCurve(originCurve, curve);
						curve->setLegendVisible(enableCurveInLegend);
					}
				} else { // error "curves"
					DEBUG(Q_FUNC_INFO << ", ERROR CURVE. curve index = " << curveIndex)
					// find corresponing curve to add error column
					// we use the previous curve if it has the same y column
					if (!preview && plot->childCount<XYCurve>() > 0) { // curves not available in preview
						auto childIndex = plot->childCount<XYCurve>() - 1; // last curve
						auto curve = plot->children<XYCurve>().at(childIndex);
						if (xColumnPath == curve->yColumnPath()) { // TODO: only for reversed plots?
							if (type == Origin::GraphCurve::ErrorBar) {
								curve->errorBar()->setYErrorType(ErrorBar::ErrorType::Symmetric);
								curve->errorBar()->setYPlusColumnPath(yColumnPath);
							} else if (type == Origin::GraphCurve::XErrorBar) {
								curve->errorBar()->setXErrorType(ErrorBar::ErrorType::Symmetric);
								curve->errorBar()->setXPlusColumnPath(yColumnPath);
							}
							// YErrorBar, XYErrorBar not available
						}
					}
				}
			} break;
			case Origin::GraphCurve::Column:
			case Origin::GraphCurve::ColumnStack:
			case Origin::GraphCurve::Bar:
			case Origin::GraphCurve::BarStack: {
				DEBUG(Q_FUNC_INFO << ", BAR/COLUMN PLOT")
				BarPlot* lastPlot = nullptr;
				auto childIndex = plot->childCount<BarPlot>() - 1; // last bar plot
				if (childIndex >= 0)
					lastPlot = plot->children<BarPlot>().at(childIndex);
				if (!lastPlot || xColumnPath != lastPlot->xColumnPath()) { // new bar plot. TODO: column plot: compare yColumnPath?
					auto* barPlot = new BarPlot(yColumnName);
					childPlot = barPlot;

					DEBUG(Q_FUNC_INFO << ", x/y column path = " << xColumnPath.toStdString() << ", " << yColumnPath.toStdString())
					barPlot->xColumnPath() = std::move(xColumnPath);
					barPlot->setDataColumnPaths({yColumnPath});

					// calculate bar width
					const auto& xColumn = sheet.columns[findColumnByName(sheet, xColumnName)];
					const auto& xData = xColumn.data;
					// this fails due to NaNs
					// const auto [xMin, xMax] = minmax_element(xData.begin(), xData.end());
					double xMin = qInf(), xMax = -qInf();
					int numDataRows = 0;
					for (const auto& v : xData) {
						const double value = v.as_double();
						if (v.type() == Origin::Variant::V_DOUBLE && !std::isnan(value)) {
							if (value < xMin)
								xMin = value;
							if (value > xMax)
								xMax = value;
							numDataRows++;
						}
					}
					DEBUG(Q_FUNC_INFO << ", x column data rows: " << numDataRows << ", min/max = " << xMin << "/" << xMax)
					if (numDataRows != 0)
						barPlot->setWidthFactor((xMax - xMin) / numDataRows);

					// TODO: BarPlot::Type::Stacked_100_Percent

					if (!preview) {
						// orientation
						if (type == Origin::GraphCurve::Column || type == Origin::GraphCurve::ColumnStack)
							barPlot->setOrientation(BarPlot::Orientation::Vertical);
						else
							barPlot->setOrientation(BarPlot::Orientation::Horizontal);

						// type - grouped vs. stacked
						if (type == Origin::GraphCurve::ColumnStack || type == Origin::GraphCurve::BarStack)
							barPlot->setType(BarPlot::Type::Stacked);
						else
							barPlot->setType(BarPlot::Type::Grouped);

						loadBackground(originCurve, barPlot->backgroundAt(0));

						// TODO: set error bar (see error plots?)
					}
				} else { // additional columns
					auto dataColumnPaths = lastPlot->dataColumnPaths();
					dataColumnPaths.append(yColumnPath);
					lastPlot->setDataColumnPaths(dataColumnPaths);
				}

				break;
			}
			case Origin::GraphCurve::Box: { // box plot
				DEBUG(Q_FUNC_INFO << ", BOX PLOT")
				auto* boxPlot = new BoxPlot(yColumnName);
				childPlot = boxPlot;

				if (!preview) {
					// TODO
				}
				break;
			}
			case Origin::GraphCurve::Histogram: {
				DEBUG(Q_FUNC_INFO << ", HISTOGRAM")
				auto* hist = new Histogram(yColumnName);
				childPlot = hist;
				hist->setDataColumnPath(yColumnPath);

				if (!preview) {
					hist->setSuppressRetransform(true);
					hist->setBinningMethod(Histogram::BinningMethod::ByWidth);
					hist->setBinWidth(layer.histogramBin);
					hist->setAutoBinRanges(false);
					hist->setBinRangesMin(layer.histogramBegin);
					hist->setBinRangesMax(layer.histogramEnd);

					if (layer.exchangedAxes)
						hist->setOrientation(Histogram::Orientation::Horizontal);
					else
						hist->setOrientation(Histogram::Orientation::Vertical);

					loadBackground(originCurve, hist->background());
				}
				break;
			}
			case Origin::GraphCurve::Scatter3D:
			case Origin::GraphCurve::Surface3D:
			case Origin::GraphCurve::Vector3D:
			case Origin::GraphCurve::ScatterAndErrorBar3D:
			case Origin::GraphCurve::TernaryContour:
			case Origin::GraphCurve::PolarXrYTheta:
			case Origin::GraphCurve::SmithChart:
			case Origin::GraphCurve::Polar:
			case Origin::GraphCurve::BubbleIndexed:
			case Origin::GraphCurve::BubbleColorMapped:
			case Origin::GraphCurve::Area:
			case Origin::GraphCurve::HiLoClose:
			case Origin::GraphCurve::ColumnFloat:
			case Origin::GraphCurve::Vector:
			case Origin::GraphCurve::PlotDot:
			case Origin::GraphCurve::Wall3D:
			case Origin::GraphCurve::Ribbon3D:
			case Origin::GraphCurve::Bar3D:
			case Origin::GraphCurve::AreaStack:
			case Origin::GraphCurve::FlowVector:
			case Origin::GraphCurve::MatrixImage:
			case Origin::GraphCurve::Pie:
			case Origin::GraphCurve::Contour:
			case Origin::GraphCurve::Unknown:
			case Origin::GraphCurve::TextPlot:
			case Origin::GraphCurve::SurfaceColorMap:
			case Origin::GraphCurve::SurfaceColorFill:
			case Origin::GraphCurve::SurfaceWireframe:
			case Origin::GraphCurve::SurfaceBars:
			case Origin::GraphCurve::Line3D:
			case Origin::GraphCurve::Text3D:
			case Origin::GraphCurve::Mesh3D:
			case Origin::GraphCurve::XYZContour:
			case Origin::GraphCurve::XYZTriangular:
			case Origin::GraphCurve::LineSeries:
				break;
			}

			break;
		}
		case 'F': {
			const auto funcIndex = m_originFile->functionIndex(dataName.right(dataName.length() - 2).toStdString());
			if (funcIndex < 0) {
				++curveIndex;
				continue;
			}

			const auto& function = m_originFile->function(funcIndex);

			auto* xyEqCurve = new XYEquationCurve(QString::fromStdString(function.name));
			childPlot = xyEqCurve;
			XYEquationCurve::EquationData eqData;
			eqData.count = function.totalPoints;
			eqData.expression1 = QString::fromStdString(function.formula);
			DEBUG("STRING FUNCTION FORMULA: " << function.formula)

			if (function.type == Origin::Function::Polar) {
				eqData.type = XYEquationCurve::EquationType::Polar;

				// replace 'x' by 'phi'
				eqData.expression1 = eqData.expression1.replace(QLatin1Char('x'), QLatin1String("phi"));

				// convert from degrees to radians
				eqData.min = QString::number(function.begin / 180.) + QLatin1String("*pi");
				eqData.max = QString::number(function.end / 180.) + QLatin1String("*pi");
			} else {
				eqData.expression1 = QLatin1String(function.formula.c_str());
				eqData.min = QString::number(function.begin);
				eqData.max = QString::number(function.end);
			}

			if (!preview) {
				xyEqCurve->setEquationData(eqData);
				xyEqCurve->setSuppressRetransform(true);
				loadCurve(originCurve, xyEqCurve);
			}

			break;
		}
		case 'M': { // Matrix
			// TODO
			break;
		}
		}

		// set the coordinate system index and add to the plot area parent
		if (childPlot && !preview) {
			if (m_graphLayerAsPlotArea)
				childPlot->setCoordinateSystemIndex(plot->defaultCoordinateSystemIndex());
			else
				childPlot->setCoordinateSystemIndex(layerIndex);

			plot->addChildFast(childPlot);
			// DEBUG("ADDED CURVE. child count = " << plot->childCount<XYCurve>())
			childPlot->setSuppressRetransform(false);
		}

		++curveIndex;
	}

	DEBUG(Q_FUNC_INFO << ", DONE")
}

void OriginProjectParser::loadAxes(const Origin::GraphLayer& layer,
								   CartesianPlot* plot,
								   int layerIndex,
								   const QString& xColumnInfo,
								   const QString& yColumnInfo) {
	// swap axes when exchanged
	DEBUG(Q_FUNC_INFO << ", exchanged axes? = " << layer.exchangedAxes)
	const auto& originXAxis = layer.exchangedAxes ? layer.yAxis : layer.xAxis;
	const auto& originYAxis = layer.exchangedAxes ? layer.xAxis : layer.yAxis;

	// x bottom
	if (!originXAxis.formatAxis[0].hidden || originXAxis.tickAxis[0].showMajorLabels) {
		auto* axis = new Axis(QStringLiteral("x"), Axis::Orientation::Horizontal);
		axis->setSuppressRetransform(true);
		axis->setPosition(Axis::Position::Bottom);
		plot->addChildFast(axis);

		// fix padding if label not shown
		const auto& axisFormat = originXAxis.formatAxis[0];
		if (!axisFormat.label.shown)
			plot->setBottomPadding(plot->bottomPadding() / 2.);

		loadAxis(originXAxis, axis, layerIndex, 0, xColumnInfo);
		if (!m_graphLayerAsPlotArea)
			axis->setCoordinateSystemIndex(layerIndex);
		axis->setSuppressRetransform(false);
	}

	// x top
	if (!originXAxis.formatAxis[1].hidden || originXAxis.tickAxis[1].showMajorLabels) {
		auto* axis = new Axis(QStringLiteral("x top"), Axis::Orientation::Horizontal);
		axis->setPosition(Axis::Position::Top);
		axis->setSuppressRetransform(true);
		plot->addChildFast(axis);

		// fix padding if label not shown
		const auto& axisFormat = originXAxis.formatAxis[1];
		if (!axisFormat.label.shown)
			plot->setVerticalPadding(plot->verticalPadding() / 2.);

		loadAxis(originXAxis, axis, layerIndex, 1, xColumnInfo);
		if (!m_graphLayerAsPlotArea)
			axis->setCoordinateSystemIndex(layerIndex);
		axis->setSuppressRetransform(false);
	}

	// y left
	if (!originYAxis.formatAxis[0].hidden || originYAxis.tickAxis[0].showMajorLabels) {
		auto* axis = new Axis(QStringLiteral("y"), Axis::Orientation::Vertical);
		axis->setSuppressRetransform(true);
		axis->setPosition(Axis::Position::Left);
		plot->addChildFast(axis);

		// fix padding if label not shown
		const auto& axisFormat = originYAxis.formatAxis[0];
		if (!axisFormat.label.shown)
			plot->setHorizontalPadding(plot->horizontalPadding() / 2.);

		loadAxis(originYAxis, axis, layerIndex, 0, yColumnInfo);
		if (!m_graphLayerAsPlotArea)
			axis->setCoordinateSystemIndex(layerIndex);
		axis->setSuppressRetransform(false);
	}

	// y right
	if (!originYAxis.formatAxis[1].hidden || originYAxis.tickAxis[1].showMajorLabels) {
		auto* axis = new Axis(QStringLiteral("y right"), Axis::Orientation::Vertical);
		axis->setSuppressRetransform(true);
		axis->setPosition(Axis::Position::Right);
		plot->addChildFast(axis);

		// fix padding if label not shown
		const auto& axisFormat = originYAxis.formatAxis[1];
		if (!axisFormat.label.shown)
			plot->setRightPadding(plot->rightPadding() / 2.);

		loadAxis(originYAxis, axis, layerIndex, 1, yColumnInfo);
		if (!m_graphLayerAsPlotArea)
			axis->setCoordinateSystemIndex(layerIndex);
		axis->setSuppressRetransform(false);
	}
}

/*
 * sets the axis properties (format and ticks) as defined in \c originAxis in \c axis,
 * \c index being 0 or 1 for "bottom" and "top" or "left" and "right" for horizontal or vertical axes, respectively.
 */
void OriginProjectParser::loadAxis(const Origin::GraphAxis& originAxis, Axis* axis, int layerIndex, int index, const QString& columnInfo) const {
	// 	int axisPosition;
	//		possible values:
	//			0: Axis is at default position
	//			1: Axis is at (axisPositionValue)% from standard position
	//			2: Axis is at (axisPositionValue) position of orthogonal axis
	// 		double axisPositionValue;

	// 		bool zeroLine;
	// 		bool oppositeLine;

	DEBUG(Q_FUNC_INFO << ", index = " << index)

	// ranges
	axis->setRange(originAxis.min, originAxis.max);

	// ticks
	axis->setMajorTicksType(Axis::TicksType::Spacing);
	DEBUG(Q_FUNC_INFO << ", step = " << originAxis.step)
	DEBUG(Q_FUNC_INFO << ", position = " << originAxis.position)
	DEBUG(Q_FUNC_INFO << ", anchor = " << originAxis.anchor)
	axis->setMajorTicksSpacing(originAxis.step);
	// set offset from step and min later, when scaling factor is known
	axis->setMinorTicksType(Axis::TicksType::TotalNumber);
	axis->setMinorTicksNumber(originAxis.minorTicks);

	// scale
	switch (originAxis.scale) {
	case Origin::GraphAxis::Linear:
		axis->setScale(RangeT::Scale::Linear);
		break;
	case Origin::GraphAxis::Log10:
		axis->setScale(RangeT::Scale::Log10);
		break;
	case Origin::GraphAxis::Ln:
		axis->setScale(RangeT::Scale::Ln);
		break;
	case Origin::GraphAxis::Log2:
		axis->setScale(RangeT::Scale::Log2);
		break;
	case Origin::GraphAxis::Reciprocal:
		axis->setScale(RangeT::Scale::Inverse);
		break;
	case Origin::GraphAxis::Probability:
	case Origin::GraphAxis::Probit:
	case Origin::GraphAxis::OffsetReciprocal:
	case Origin::GraphAxis::Logit:
		// TODO: set if implemented
		axis->setScale(RangeT::Scale::Linear);
		break;
	}

	// major grid
	const auto& majorGrid = originAxis.majorGrid;
	Qt::PenStyle penStyle(Qt::NoPen);
	if (!majorGrid.hidden) {
		switch (majorGrid.style) {
		case Origin::GraphCurve::Solid:
			penStyle = Qt::SolidLine;
			break;
		case Origin::GraphCurve::Dash:
		case Origin::GraphCurve::ShortDash:
			penStyle = Qt::DashLine;
			break;
		case Origin::GraphCurve::Dot:
		case Origin::GraphCurve::ShortDot:
			penStyle = Qt::DotLine;
			break;
		case Origin::GraphCurve::DashDot:
		case Origin::GraphCurve::ShortDashDot:
			penStyle = Qt::DashDotLine;
			break;
		case Origin::GraphCurve::DashDotDot:
			penStyle = Qt::DashDotDotLine;
			break;
		}
	}
	axis->majorGridLine()->setStyle(penStyle);

	Origin::Color gridColor;
	gridColor.type = Origin::Color::ColorType::Regular;
	gridColor.regular = majorGrid.color;
	axis->majorGridLine()->setColor(OriginProjectParser::color(gridColor));
	axis->majorGridLine()->setWidth(Worksheet::convertToSceneUnits(majorGrid.width, Worksheet::Unit::Point));

	// minor grid
	const auto& minorGrid = originAxis.minorGrid;
	penStyle = Qt::NoPen;
	if (!minorGrid.hidden) {
		switch (minorGrid.style) {
		case Origin::GraphCurve::Solid:
			penStyle = Qt::SolidLine;
			break;
		case Origin::GraphCurve::Dash:
		case Origin::GraphCurve::ShortDash:
			penStyle = Qt::DashLine;
			break;
		case Origin::GraphCurve::Dot:
		case Origin::GraphCurve::ShortDot:
			penStyle = Qt::DotLine;
			break;
		case Origin::GraphCurve::DashDot:
		case Origin::GraphCurve::ShortDashDot:
			penStyle = Qt::DashDotLine;
			break;
		case Origin::GraphCurve::DashDotDot:
			penStyle = Qt::DashDotDotLine;
			break;
		}
	}
	axis->minorGridLine()->setStyle(penStyle);

	gridColor.regular = minorGrid.color;
	axis->minorGridLine()->setColor(OriginProjectParser::color(gridColor));
	axis->minorGridLine()->setWidth(Worksheet::convertToSceneUnits(minorGrid.width, Worksheet::Unit::Point));

	// process Origin::GraphAxisFormat
	const auto& axisFormat = originAxis.formatAxis[index];

	Origin::Color color;
	color.type = Origin::Color::ColorType::Regular;
	color.regular = axisFormat.color;
	axis->line()->setColor(OriginProjectParser::color(color));
	// DEBUG("AXIS LINE THICKNESS = " << axisFormat.thickness)
	double lineThickness = 1.;
	if (layerIndex == 0) // axis line thickness is actually only used for first layer in Origin!
		lineThickness = axisFormat.thickness;
	axis->line()->setWidth(Worksheet::convertToSceneUnits(lineThickness * elementScalingFactor, Worksheet::Unit::Point));

	if (axisFormat.hidden)
		axis->line()->setStyle(Qt::NoPen);
	// TODO: read line style properties? (solid line, dashed line, etc.)

	axis->setMajorTicksLength(Worksheet::convertToSceneUnits(axisFormat.majorTickLength * elementScalingFactor, Worksheet::Unit::Point));
	axis->setMajorTicksDirection((Axis::TicksFlags)axisFormat.majorTicksType);
	axis->majorTicksLine()->setStyle(axis->line()->style());
	axis->majorTicksLine()->setColor(axis->line()->color());
	axis->majorTicksLine()->setWidth(axis->line()->width());
	axis->setMinorTicksLength(axis->majorTicksLength() / 2); // minorTicksLength is half of majorTicksLength
	axis->setMinorTicksDirection((Axis::TicksFlags)axisFormat.minorTicksType);
	axis->minorTicksLine()->setStyle(axis->line()->style());
	axis->minorTicksLine()->setColor(axis->line()->color());
	axis->minorTicksLine()->setWidth(axis->line()->width());

	// axis title
	if (axisFormat.label.shown) {
		/*for (int i=0; i<axisFormat.label.text.size(); i++)
			printf(" %c ", axisFormat.label.text.at(i));
		printf("\n");
		for (int i=0; i<axisFormat.label.text.size(); i++)
			printf(" %02hhx", axisFormat.label.text.at(i));
		printf("\n");
		*/
		QString titleText;
		if (m_originFile->version() < 9.5) // <= 2017 : pre-Unicode
			titleText = parseOriginText(QString::fromLatin1(axisFormat.label.text.c_str()));
		else
			titleText = parseOriginText(QString::fromStdString(axisFormat.label.text));
		DEBUG(Q_FUNC_INFO << ", axis title string = " << STDSTRING(titleText));
		DEBUG(Q_FUNC_INFO << ", column info = " << STDSTRING(columnInfo));

		// if long name not defined: columnInfo contains column name (s.a.)
		QString longName, unit, comments;
		parseColumnInfo(columnInfo, longName, unit, comments);
		if (comments.isEmpty())
			comments = longName;

		QString unitString(unit.isEmpty() ? QStringLiteral("") : QStringLiteral(" (") + unit + QStringLiteral(")"));
		// TODO: more replacements here using column info (see loadCurves())
		titleText.replace(QLatin1String("%(?X)"), longName + unitString);
		titleText.replace(QLatin1String("%(?Y)"), longName + unitString);
		titleText.replace(QLatin1String("%(?X,@L)"), longName);
		titleText.replace(QLatin1String("%(?Y,@L)"), longName);
		titleText.replace(QLatin1String("%(?X,@LC)"), comments);
		titleText.replace(QLatin1String("%(?Y,@LC)"), comments);
		titleText.replace(QLatin1String("%(?X,@LG)"), longName + unitString);
		titleText.replace(QLatin1String("%(?Y,@LG)"), longName + unitString);
		titleText.replace(QLatin1String("%(?X,@LL)"), longName);
		titleText.replace(QLatin1String("%(?Y,@LL)"), longName);
		titleText.replace(QLatin1String("%(?X,@LM)"), comments);
		titleText.replace(QLatin1String("%(?Y,@LM)"), comments);
		titleText.replace(QLatin1String("%(?X,@LN)"), comments + unitString);
		titleText.replace(QLatin1String("%(?Y,@LN)"), comments + unitString);
		// not available
		//		titleText.replace(QLatin1String("%(?X,@LS)"), shortName);
		//		titleText.replace(QLatin1String("%(?Y,@LS)"), shortName);
		titleText.replace(QLatin1String("%(?X,@LU)"), unit);
		titleText.replace(QLatin1String("%(?Y,@LU)"), unit);

		DEBUG(Q_FUNC_INFO << ", axis title final = " << STDSTRING(titleText));

		// use axisFormat.fontSize to override the global font size for the hmtl string
		DEBUG(Q_FUNC_INFO << ", axis font size = " << axisFormat.label.fontSize)
		QTextEdit te(titleText);
		te.selectAll();
		te.setFontPointSize(int(axisFormat.label.fontSize * textScalingFactor));
		// TODO: parseOriginText() returns html formatted string. What is axisFormat.color used for?
		// te.setTextColor(OriginProjectParser::color(t.color));
		axis->title()->setText(te.toHtml());

		// axis->title()->setText(titleText);
		axis->title()->setRotationAngle(axisFormat.label.rotation);
	} else {
		axis->title()->setText({});
	}

	// handle string factor member in GraphAxisFormat
	double scalingFactor = 1.0;
	if (!axisFormat.factor.empty()) {
		scalingFactor = 1. / std::stod(axisFormat.factor);
		DEBUG(Q_FUNC_INFO << ", scaling factor = " << scalingFactor)
		axis->setScalingFactor(scalingFactor);
		axis->setShowScaleOffset(false); // don't show scale factor
	}
	// now set axis ticks start offset
	double startOffset = nsl_math_ceil_multiple(originAxis.min * scalingFactor, originAxis.step * scalingFactor) - originAxis.min;
	DEBUG(Q_FUNC_INFO << ", min = " << originAxis.min << ", step = " << originAxis.step)
	DEBUG(Q_FUNC_INFO << ", start offset = " << startOffset)
	if (axis->range().contains(startOffset))
		axis->setMajorTickStartOffset(startOffset);
	else {
		DEBUG(Q_FUNC_INFO << ", WARNING: start offset = " << startOffset << " outside of axis range!")
		axis->setMajorTickStartOffset(0.);
	}

	axis->setLabelsPrefix(QLatin1String(axisFormat.prefix.c_str()));
	axis->setLabelsSuffix(QLatin1String(axisFormat.suffix.c_str()));

	// process Origin::GraphAxisTick
	const auto& tickAxis = originAxis.tickAxis[index];
	if (tickAxis.showMajorLabels) {
		color.type = Origin::Color::ColorType::Regular;
		color.regular = tickAxis.color;
		axis->setLabelsColor(OriginProjectParser::color(color));
		if (index == 0) // left
			axis->setLabelsPosition(Axis::LabelsPosition::Out);
		else // right
			axis->setLabelsPosition(Axis::LabelsPosition::In);
	} else {
		axis->setLabelsPosition(Axis::LabelsPosition::NoLabels);
	}

	// TODO: handle ValueType valueType member in GraphAxisTick
	// TODO: handle int valueTypeSpecification in GraphAxisTick

	// precision
	if (tickAxis.decimalPlaces == -1) {
		DEBUG(Q_FUNC_INFO << ", SET auto precision")
		axis->setLabelsAutoPrecision(true);
	} else {
		DEBUG(Q_FUNC_INFO << ", DISABLE auto precision. decimalPlaces = " << tickAxis.decimalPlaces)
		axis->setLabelsPrecision(tickAxis.decimalPlaces);
		axis->setLabelsAutoPrecision(false);
	}

	QFont font;
	// TODO: font family?
	DEBUG(Q_FUNC_INFO << ", axis tick label font size = " << tickAxis.fontSize)
	DEBUG(Q_FUNC_INFO << ", axis tick label font size in points = " << Worksheet::convertToSceneUnits(tickAxis.fontSize, Worksheet::Unit::Point))
	font.setPointSize(static_cast<int>(Worksheet::convertToSceneUnits(tickAxis.fontSize * textScalingFactor, Worksheet::Unit::Point)));
	font.setBold(tickAxis.fontBold);
	axis->setLabelsFont(font);
	// TODO: handle string dataName member in GraphAxisTick
	// TODO: handle string columnName member in GraphAxisTick
	axis->setLabelsRotationAngle(tickAxis.rotation);
}

void OriginProjectParser::loadCurve(const Origin::GraphCurve& originCurve, XYCurve* curve) const {
	DEBUG(Q_FUNC_INFO)

	// line properties
	if (originCurve.type == Origin::GraphCurve::Line || originCurve.type == Origin::GraphCurve::LineSymbol) { // TODO: what about *ErrorBar types?
		curve->setLineType(lineType(originCurve.lineConnect));
		curve->line()->setStyle(penStyle(originCurve.lineStyle));
		auto lineWidth = std::max(originCurve.lineWidth * elementScalingFactor, 1.); // minimum 1 px
		curve->line()->setWidth(Worksheet::convertToSceneUnits(lineWidth, Worksheet::Unit::Point));
		curve->line()->setColor(color(originCurve.lineColor));
		curve->line()->setOpacity(1 - originCurve.lineTransparency / 255);
		// TODO: handle unsigned char boxWidth of Origin::GraphCurve
	} else
		curve->line()->setStyle(Qt::NoPen);

	// symbol properties
	loadSymbol(originCurve, curve->symbol(), curve);

	// filling properties
	loadBackground(originCurve, curve->background());
}

void OriginProjectParser::loadBackground(const Origin::GraphCurve& originCurve, Background* background) const {
	DEBUG(Q_FUNC_INFO << ", fill area? " << originCurve.fillArea)
	// fillArea option not used in histogram and bar/column plot
	auto type = originCurve.type;
	if (!originCurve.fillArea && type != Origin::GraphCurve::Histogram && type != Origin::GraphCurve::Bar && type != Origin::GraphCurve::Column
		&& type != Origin::GraphCurve::BarStack && type != Origin::GraphCurve::ColumnStack) {
		background->setPosition(Background::Position::No);
		return;
	}
	// TODO: handle unsigned char fillAreaType;
	// with 'fillAreaType'=0x10 the area between the curve and the x-axis is filled
	// with 'fillAreaType'=0x14 the area included inside the curve is filled. First and last curve points are joined by a line to close the otherwise open
	// area. with 'fillAreaType'=0x12 the area excluded outside the curve is filled. The inverse of fillAreaType=0x14 is filled. At the moment we only
	// support the first type, so set it to XYCurve::FillingBelow
	background->setPosition(Background::Position::Below);

	if (originCurve.fillAreaPattern == 0) {
		background->setType(Background::Type::Color);
	} else {
		background->setType(Background::Type::Pattern);

		// map different patterns in originCurve.fillAreaPattern (has the values of Origin::FillPattern) to Qt::BrushStyle;
		switch (originCurve.fillAreaPattern) {
		case 0:
			background->setBrushStyle(Qt::NoBrush);
			break;
		case 1:
		case 2:
		case 3:
			background->setBrushStyle(Qt::BDiagPattern);
			break;
		case 4:
		case 5:
		case 6:
			background->setBrushStyle(Qt::FDiagPattern);
			break;
		case 7:
		case 8:
		case 9:
			background->setBrushStyle(Qt::DiagCrossPattern);
			break;
		case 10:
		case 11:
		case 12:
			background->setBrushStyle(Qt::HorPattern);
			break;
		case 13:
		case 14:
		case 15:
			background->setBrushStyle(Qt::VerPattern);
			break;
		case 16:
		case 17:
		case 18:
			background->setBrushStyle(Qt::CrossPattern);
			break;
		}
	}

	background->setFirstColor(color(originCurve.fillAreaColor));
	background->setOpacity(1. - originCurve.fillAreaTransparency / 255.);

	// Color fillAreaPatternColor - color for the pattern lines, not supported
	// double fillAreaPatternWidth - width of the pattern lines, not supported
	// bool fillAreaWithLineTransparency - transparency of the pattern lines independent of the area transparency, not supported

	// TODO:
	// unsigned char fillAreaPatternBorderStyle;
	// Color fillAreaPatternBorderColor;
	// double fillAreaPatternBorderWidth;
	// The Border properties are used only in "Column/Bar" (histogram) plots. Those properties are:
	// fillAreaPatternBorderStyle   for the line style (use enum Origin::LineStyle here)
	// fillAreaPatternBorderColor   for the line color
	// fillAreaPatternBorderWidth   for the line width
}

void OriginProjectParser::loadSymbol(const Origin::GraphCurve& originCurve, Symbol* symbol, const XYCurve* curve) const {
	if (originCurve.type != Origin::GraphCurve::Scatter && originCurve.type != Origin::GraphCurve::LineSymbol) {
		symbol->setStyle(Symbol::Style::NoSymbols);
		return;
	}

	// symbol style:
	// try to map the different symbols, mapping is not exact,
	// see https://www.originlab.com/doc/Labtalk/Ref/List-of-Symbol-Shapes
	symbol->setRotationAngle(0);
	switch (originCurve.symbolShape) {
	case 0: // NoSymbol
		symbol->setStyle(Symbol::Style::NoSymbols);
		break;
	case 1: // Square
		switch (originCurve.symbolInterior) {
		case 0: // solid
		case 1: // open
		case 3: // hollow
			symbol->setStyle(Symbol::Style::Square);
			break;
		case 2: // dot
			symbol->setStyle(Symbol::Style::SquareDot);
			break;
		case 4: // plus
			symbol->setStyle(Symbol::Style::SquarePlus);
			break;
		case 5: // X
			symbol->setStyle(Symbol::Style::SquareX);
			break;
		case 6: // minus
		case 10: // down
			symbol->setStyle(Symbol::Style::SquareHalf);
			break;
		case 7: // pipe
			symbol->setStyle(Symbol::Style::SquareHalf);
			symbol->setRotationAngle(90);
			break;
		case 8: // up
			symbol->setStyle(Symbol::Style::SquareHalf);
			symbol->setRotationAngle(180);
			break;
		case 9: // right
			symbol->setStyle(Symbol::Style::SquareHalf);
			symbol->setRotationAngle(-90);
			break;
		case 11: // left
			symbol->setStyle(Symbol::Style::SquareHalf);
			symbol->setRotationAngle(90);
			break;
		}
		break;
	case 2: // Ellipse
	case 20: // Sphere
		switch (originCurve.symbolInterior) {
		case 0: // solid
		case 1: // open
		case 3: // hollow
			symbol->setStyle(Symbol::Style::Circle);
			break;
		case 2: // dot
			symbol->setStyle(Symbol::Style::CircleDot);
			break;
		case 4: // plus
			symbol->setStyle(Symbol::Style::CircleX);
			symbol->setRotationAngle(45);
			break;
		case 5: // X
			symbol->setStyle(Symbol::Style::CircleX);
			break;
		case 6: // minus
			symbol->setStyle(Symbol::Style::CircleHalf);
			symbol->setRotationAngle(90);
			break;
		case 7: // pipe
		case 11: // left
			symbol->setStyle(Symbol::Style::CircleHalf);
			break;
		case 8: // up
			symbol->setStyle(Symbol::Style::CircleHalf);
			symbol->setRotationAngle(90);
			break;
		case 9: // right
			symbol->setStyle(Symbol::Style::CircleHalf);
			symbol->setRotationAngle(180);
			break;
		case 10: // down
			symbol->setStyle(Symbol::Style::CircleHalf);
			symbol->setRotationAngle(-90);
			break;
		}
		break;
	case 3: // UTriangle
		switch (originCurve.symbolInterior) {
		case 0: // solid
		case 1: // open
		case 3: // hollow
		case 4: // plus	TODO
		case 5: // X	TODO
			symbol->setStyle(Symbol::Style::EquilateralTriangle);
			break;
		case 2: // dot
			symbol->setStyle(Symbol::Style::TriangleDot);
			break;
		case 7: // pipe
		case 11: // left
			symbol->setStyle(Symbol::Style::TriangleLine);
			break;
		case 6: // minus
		case 8: // up
			symbol->setStyle(Symbol::Style::TriangleHalf);
			break;
		case 9: // right	TODO
			symbol->setStyle(Symbol::Style::TriangleLine);
			// symbol->setRotationAngle(180);
			break;
		case 10: // down	TODO
			symbol->setStyle(Symbol::Style::TriangleHalf);
			// symbol->setRotationAngle(180);
			break;
		}
		break;
	case 4: // DTriangle
		switch (originCurve.symbolInterior) {
		case 0: // solid
		case 1: // open
		case 3: // hollow
		case 4: // plus	TODO
		case 5: // X	TODO
			symbol->setStyle(Symbol::Style::EquilateralTriangle);
			symbol->setRotationAngle(180);
			break;
		case 2: // dot
			symbol->setStyle(Symbol::Style::TriangleDot);
			symbol->setRotationAngle(180);
			break;
		case 7: // pipe
		case 11: // left
			symbol->setStyle(Symbol::Style::TriangleLine);
			symbol->setRotationAngle(180);
			break;
		case 6: // minus
		case 8: // up
			symbol->setStyle(Symbol::Style::TriangleHalf);
			symbol->setRotationAngle(180);
			break;
		case 9: // right	TODO
			symbol->setStyle(Symbol::Style::TriangleLine);
			symbol->setRotationAngle(180);
			break;
		case 10: // down	TODO
			symbol->setStyle(Symbol::Style::TriangleHalf);
			symbol->setRotationAngle(180);
			break;
		}
		break;
	case 5: // Diamond
		symbol->setStyle(Symbol::Style::Diamond);
		switch (originCurve.symbolInterior) {
		case 0: // solid
		case 1: // open
		case 3: // hollow
			symbol->setStyle(Symbol::Style::Diamond);
			break;
		case 2: // dot
			symbol->setStyle(Symbol::Style::SquareDot);
			symbol->setRotationAngle(45);
			break;
		case 4: // plus
			symbol->setStyle(Symbol::Style::SquareX);
			symbol->setRotationAngle(45);
			break;
		case 5: // X
			symbol->setStyle(Symbol::Style::SquarePlus);
			symbol->setRotationAngle(45);
			break;
		case 6: // minus
		case 10: // down
			symbol->setStyle(Symbol::Style::SquareHalf);
			break;
		case 7: // pipe
			symbol->setStyle(Symbol::Style::SquareHalf);
			symbol->setRotationAngle(90);
			break;
		case 8: // up
			symbol->setStyle(Symbol::Style::SquareHalf);
			symbol->setRotationAngle(180);
			break;
		case 9: // right
			symbol->setStyle(Symbol::Style::SquareHalf);
			symbol->setRotationAngle(-90);
			break;
		case 11: // left
			symbol->setStyle(Symbol::Style::SquareHalf);
			symbol->setRotationAngle(90);
			break;
		}
		break;
	case 6: // Cross +
		symbol->setStyle(Symbol::Style::Cross);
		break;
	case 7: // Cross x
		symbol->setStyle(Symbol::Style::Cross);
		symbol->setRotationAngle(45);
		break;
	case 8: // Snow
		symbol->setStyle(Symbol::Style::XPlus);
		break;
	case 9: // Horizontal -
		symbol->setStyle(Symbol::Style::Line);
		symbol->setRotationAngle(90);
		break;
	case 10: // Vertical |
		symbol->setStyle(Symbol::Style::Line);
		break;
	case 15: // LTriangle
		switch (originCurve.symbolInterior) {
		case 0: // solid
		case 1: // open
		case 3: // hollow
		case 4: // plus	TODO
		case 5: // X	TODO
			symbol->setStyle(Symbol::Style::EquilateralTriangle);
			symbol->setRotationAngle(-90);
			break;
		case 2: // dot
			symbol->setStyle(Symbol::Style::TriangleDot);
			symbol->setRotationAngle(-90);
			break;
		case 7: // pipe
		case 11: // left
			symbol->setStyle(Symbol::Style::TriangleLine);
			symbol->setRotationAngle(-90);
			break;
		case 6: // minus
		case 8: // up
			symbol->setStyle(Symbol::Style::TriangleHalf);
			symbol->setRotationAngle(-90);
			break;
		case 9: // right	TODO
			symbol->setStyle(Symbol::Style::TriangleLine);
			symbol->setRotationAngle(-90);
			break;
		case 10: // down	TODO
			symbol->setStyle(Symbol::Style::TriangleHalf);
			symbol->setRotationAngle(-90);
			break;
		}
		break;
	case 16: // RTriangle
		switch (originCurve.symbolInterior) {
		case 0: // solid
		case 1: // open
		case 3: // hollow
		case 4: // plus	TODO
		case 5: // X	TODO
			symbol->setStyle(Symbol::Style::EquilateralTriangle);
			symbol->setRotationAngle(90);
			break;
		case 2: // dot
			symbol->setStyle(Symbol::Style::TriangleDot);
			symbol->setRotationAngle(90);
			break;
		case 7: // pipe
		case 11: // left
			symbol->setStyle(Symbol::Style::TriangleLine);
			symbol->setRotationAngle(90);
			break;
		case 6: // minus
		case 8: // up
			symbol->setStyle(Symbol::Style::TriangleHalf);
			symbol->setRotationAngle(90);
			break;
		case 9: // right	TODO
			symbol->setStyle(Symbol::Style::TriangleLine);
			symbol->setRotationAngle(90);
			break;
		case 10: // down	TODO
			symbol->setStyle(Symbol::Style::TriangleHalf);
			symbol->setRotationAngle(90);
			break;
		}
		break;
	case 17: // Hexagon
		symbol->setStyle(Symbol::Style::Hexagon);
		break;
	case 18: // Star
		symbol->setStyle(Symbol::Style::Star);
		break;
	case 19: // Pentagon
		symbol->setStyle(Symbol::Style::Pentagon);
		break;
	default:
		symbol->setStyle(Symbol::Style::NoSymbols);
		break;
	}

	// symbol size
	DEBUG(Q_FUNC_INFO << ", symbol size = " << originCurve.symbolSize)
	DEBUG(Q_FUNC_INFO << ", symbol size in points = " << Worksheet::convertToSceneUnits(originCurve.symbolSize, Worksheet::Unit::Point))
	symbol->setSize(Worksheet::convertToSceneUnits(originCurve.symbolSize * elementScalingFactor, Worksheet::Unit::Point));

	// symbol colors
	DEBUG(Q_FUNC_INFO << ", symbol fill color = " << originCurve.symbolFillColor.type << "_"
					  << (Origin::Color::RegularColor)originCurve.symbolFillColor.regular)
	DEBUG(Q_FUNC_INFO << ", symbol color = " << originCurve.symbolColor.type << "_" << (Origin::Color::RegularColor)originCurve.symbolColor.regular)
	// if plot type == line+symbol

	auto brush = symbol->brush();
	auto pen = symbol->pen();
	DEBUG(Q_FUNC_INFO << ", SYMBOL THICKNESS = " << (int)originCurve.symbolThickness)
	// border width (edge thickness in Origin) is given as percentage of the symbol radius
	const double borderScaleFactor = 5.; // match size
	if (curve && originCurve.type == Origin::GraphCurve::LineSymbol) {
		// symbol fill color
		if (originCurve.symbolFillColor.type == Origin::Color::ColorType::Automatic) {
			//"automatic" color -> the color of the line, if available, is used, and black otherwise
			if (curve->lineType() != XYCurve::LineType::NoLine)
				brush.setColor(curve->line()->pen().color());
			else
				brush.setColor(Qt::black);
		} else
			brush.setColor(color(originCurve.symbolFillColor));
		if (originCurve.symbolInterior > 0 && originCurve.symbolInterior < 8) // unfilled styles
			brush.setStyle(Qt::NoBrush);

		// symbol border/edge color and width
		if (originCurve.symbolColor.type == Origin::Color::ColorType::Automatic) {
			//"automatic" color -> the color of the line, if available, has to be used, black otherwise
			if (curve->lineType() != XYCurve::LineType::NoLine)
				pen.setColor(curve->line()->pen().color());
			else
				pen.setColor(Qt::black);
		} else
			pen.setColor(color(originCurve.symbolColor));

		DEBUG(Q_FUNC_INFO << ", BORDER THICKNESS = " << borderScaleFactor * originCurve.symbolThickness / 100. * symbol->size())
		pen.setWidthF(borderScaleFactor * originCurve.symbolThickness / 100. * symbol->size());
	} else if (curve && originCurve.type == Origin::GraphCurve::Scatter) {
		// symbol color (uses originCurve.symbolColor)
		if (originCurve.symbolColor.type == Origin::Color::ColorType::Automatic) {
			//"automatic" color -> the color of the line, if available, is used, and black otherwise
			if (curve->lineType() != XYCurve::LineType::NoLine)
				brush.setColor(curve->line()->pen().color());
			else
				brush.setColor(Qt::black);
		} else
			brush.setColor(color(originCurve.symbolColor));

		if (originCurve.symbolInterior > 0 && originCurve.symbolInterior < 8) { // unfilled styles
			brush.setStyle(Qt::NoBrush);
			DEBUG(Q_FUNC_INFO << ", BORDER THICKNESS = " << borderScaleFactor * originCurve.symbolThickness / 100. * symbol->size())
			pen.setWidthF(borderScaleFactor * originCurve.symbolThickness / 100. * symbol->size());

			// symbol border/edge color and width
			if (originCurve.symbolColor.type == Origin::Color::ColorType::Automatic) {
				//"automatic" color -> the color of the line, if available, has to be used, black otherwise
				if (curve->lineType() != XYCurve::LineType::NoLine)
					pen.setColor(curve->line()->pen().color());
				else
					pen.setColor(Qt::black);
			} else
				pen.setColor(color(originCurve.symbolColor));
		} else
			pen.setStyle(Qt::NoPen); // no border
	}
	symbol->setBrush(brush);
	symbol->setPen(pen);

	// handle unsigned char pointOffset member
	// handle bool connectSymbols member
}

bool OriginProjectParser::loadNote(Note* note, bool preview) {
	DEBUG(Q_FUNC_INFO);
	// load note data
	const auto& originNote = m_originFile->note(findNoteByName(note->name()));

	if (preview)
		return true;

	note->setComment(QString::fromStdString(originNote.label));
	note->setText(QString::fromStdString(originNote.text));

	return true;
}

// ##############################################################################
// ########################### Helper functions  ################################
// ##############################################################################
QDateTime OriginProjectParser::creationTime(tree<Origin::ProjectNode>::iterator it) const {
	// this logic seems to be correct only for the first node (project node). For other nodes the current time is returned.
	char time_str[21];
	strftime(time_str, sizeof(time_str), "%F %T", gmtime(&(*it).creationDate));
	return QDateTime::fromString(QLatin1String(time_str), Qt::ISODate);
}

// parse column info: (long name(, unit(, comment)))
void OriginProjectParser::parseColumnInfo(const QString& info, QString& longName, QString& unit, QString& comments) const {
	if (info.isEmpty())
		return;
	auto infoList = info.split(QRegularExpression(QStringLiteral("[\r\n]")), Qt::SkipEmptyParts);

	switch (infoList.size()) {
	case 2: // long name, unit
		unit = infoList.at(1);
		// fallthrough
	case 1: // long name
		longName = infoList.at(0);
		break;
	default: // long name, unit, comment
		longName = infoList.at(0);
		unit = infoList.at(1);
		comments = infoList.at(2);
	}
}

QString OriginProjectParser::parseOriginText(const QString& str) const {
	DEBUG(Q_FUNC_INFO);
	auto lines = str.split(QLatin1Char('\n'));
	QString text;
	for (int i = 0; i < lines.size(); ++i) {
		if (i > 0)
			text.append(QLatin1String("<br>"));
		text.append(parseOriginTags(lines[i]));
	}

	DEBUG(Q_FUNC_INFO << ", PARSED TEXT = " << STDSTRING(text));

	return text;
}

RangeT::Scale OriginProjectParser::scale(unsigned char scale) const {
	switch (scale) {
	case Origin::GraphAxis::Linear:
		return RangeT::Scale::Linear;
	case Origin::GraphAxis::Log10:
		return RangeT::Scale::Log10;
	case Origin::GraphAxis::Ln:
		return RangeT::Scale::Ln;
	case Origin::GraphAxis::Log2:
		return RangeT::Scale::Log2;
	case Origin::GraphAxis::Reciprocal:
		return RangeT::Scale::Inverse;
	case Origin::GraphAxis::Probability:
	case Origin::GraphAxis::Probit:
	case Origin::GraphAxis::OffsetReciprocal:
	case Origin::GraphAxis::Logit:
		// TODO:
		return RangeT::Scale::Linear;
	}

	return RangeT::Scale::Linear;
}

QColor OriginProjectParser::color(Origin::Color color) const {
	switch (color.type) {
	case Origin::Color::ColorType::Regular:
		switch (color.regular) {
		case Origin::Color::Black:
			return QColor{Qt::black};
		case Origin::Color::Red:
			return QColor{Qt::red};
		case Origin::Color::Green:
			return QColor{Qt::green};
		case Origin::Color::Blue:
			return QColor{Qt::blue};
		case Origin::Color::Cyan:
			return QColor{Qt::cyan};
		case Origin::Color::Magenta:
			return QColor{Qt::magenta};
		case Origin::Color::Yellow:
			return QColor{Qt::yellow};
		case Origin::Color::DarkYellow:
			return QColor{Qt::darkYellow};
		case Origin::Color::Navy:
			return QColor{0, 0, 128};
		case Origin::Color::Purple:
			return QColor{128, 0, 128};
		case Origin::Color::Wine:
			return QColor{128, 0, 0};
		case Origin::Color::Olive:
			return QColor{0, 128, 0};
		case Origin::Color::DarkCyan:
			return QColor{Qt::darkCyan};
		case Origin::Color::Royal:
			return QColor{0, 0, 160};
		case Origin::Color::Orange:
			return QColor{255, 128, 0};
		case Origin::Color::Violet:
			return QColor{128, 0, 255};
		case Origin::Color::Pink:
			return QColor{255, 0, 128};
		case Origin::Color::White:
			return QColor{Qt::white};
		case Origin::Color::LightGray:
			return QColor{Qt::lightGray};
		case Origin::Color::Gray:
			return QColor{Qt::gray};
		case Origin::Color::LTYellow:
			return QColor{255, 0, 128};
		case Origin::Color::LTCyan:
			return QColor{128, 255, 255};
		case Origin::Color::LTMagenta:
			return QColor{255, 128, 255};
		case Origin::Color::DarkGray:
			return QColor{Qt::darkGray};
		case Origin::Color::SpecialV7Axis:
			return QColor{Qt::black};
		}
		break;
	case Origin::Color::ColorType::Custom:
		return QColor{color.custom[0], color.custom[1], color.custom[2]};
	case Origin::Color::ColorType::None:
	case Origin::Color::ColorType::Automatic:
	case Origin::Color::ColorType::Increment:
	case Origin::Color::ColorType::Indexing:
	case Origin::Color::ColorType::RGB:
	case Origin::Color::ColorType::Mapping:
		break;
	}

	return Qt::white;
}

Background::ColorStyle OriginProjectParser::backgroundColorStyle(Origin::ColorGradientDirection colorGradient) const {
	switch (colorGradient) {
	case Origin::ColorGradientDirection::NoGradient:
		return Background::ColorStyle::SingleColor;
	case Origin::ColorGradientDirection::TopLeft:
		return Background::ColorStyle::TopLeftDiagonalLinearGradient;
	case Origin::ColorGradientDirection::Left:
		return Background::ColorStyle::HorizontalLinearGradient;
	case Origin::ColorGradientDirection::BottomLeft:
		return Background::ColorStyle::BottomLeftDiagonalLinearGradient;
	case Origin::ColorGradientDirection::Top:
		return Background::ColorStyle::VerticalLinearGradient;
	case Origin::ColorGradientDirection::Center:
		return Background::ColorStyle::RadialGradient;
	case Origin::ColorGradientDirection::Bottom:
		return Background::ColorStyle::VerticalLinearGradient;
	case Origin::ColorGradientDirection::TopRight:
		return Background::ColorStyle::BottomLeftDiagonalLinearGradient;
	case Origin::ColorGradientDirection::Right:
		return Background::ColorStyle::HorizontalLinearGradient;
	case Origin::ColorGradientDirection::BottomRight:
		return Background::ColorStyle::TopLeftDiagonalLinearGradient;
	}

	return Background::ColorStyle::SingleColor;
}

QString strreverse(const QString& str) { // QString reversing
	auto ba = str.toLocal8Bit();
	std::reverse(ba.begin(), ba.end());

	return QLatin1String(ba);
}

Qt::PenStyle OriginProjectParser::penStyle(unsigned char lineStyle) const {
	switch (lineStyle) {
	case Origin::GraphCurve::Solid:
		return Qt::SolidLine;
	case Origin::GraphCurve::Dash:
	case Origin::GraphCurve::ShortDash:
		return Qt::DashLine;
	case Origin::GraphCurve::Dot:
	case Origin::GraphCurve::ShortDot:
		return Qt::DotLine;
	case Origin::GraphCurve::DashDot:
	case Origin::GraphCurve::ShortDashDot:
		return Qt::DashDotLine;
	case Origin::GraphCurve::DashDotDot:
		return Qt::DashDotDotLine;
	}

	return Qt::SolidLine;
}

XYCurve::LineType OriginProjectParser::lineType(unsigned char lineConnect) const {
	switch (lineConnect) {
	case Origin::GraphCurve::NoLine:
		return XYCurve::LineType::NoLine;
	case Origin::GraphCurve::Straight:
		return XYCurve::LineType::Line;
	case Origin::GraphCurve::TwoPointSegment:
		return XYCurve::LineType::Segments2;
	case Origin::GraphCurve::ThreePointSegment:
		return XYCurve::LineType::Segments3;
	case Origin::GraphCurve::BSpline:
	case Origin::GraphCurve::Bezier:
	case Origin::GraphCurve::Spline:
		return XYCurve::LineType::SplineCubicNatural;
	case Origin::GraphCurve::StepHorizontal:
		return XYCurve::LineType::StartHorizontal;
	case Origin::GraphCurve::StepVertical:
		return XYCurve::LineType::StartVertical;
	case Origin::GraphCurve::StepHCenter:
		return XYCurve::LineType::MidpointHorizontal;
	case Origin::GraphCurve::StepVCenter:
		return XYCurve::LineType::MidpointVertical;
	}

	return XYCurve::LineType::NoLine;
}

QList<QPair<QString, QString>> OriginProjectParser::charReplacementList() const {
	QList<QPair<QString, QString>> replacements;

	// TODO: probably missed some. Is there any generic method?
	replacements << qMakePair(QStringLiteral("ä"), QStringLiteral("&auml;"));
	replacements << qMakePair(QStringLiteral("ö"), QStringLiteral("&ouml;"));
	replacements << qMakePair(QStringLiteral("ü"), QStringLiteral("&uuml;"));
	replacements << qMakePair(QStringLiteral("Ä"), QStringLiteral("&Auml;"));
	replacements << qMakePair(QStringLiteral("Ö"), QStringLiteral("&Ouml;"));
	replacements << qMakePair(QStringLiteral("Ü"), QStringLiteral("&Uuml;"));
	replacements << qMakePair(QStringLiteral("ß"), QStringLiteral("&szlig;"));
	replacements << qMakePair(QStringLiteral("€"), QStringLiteral("&euro;"));
	replacements << qMakePair(QStringLiteral("£"), QStringLiteral("&pound;"));
	replacements << qMakePair(QStringLiteral("¥"), QStringLiteral("&yen;"));
	replacements << qMakePair(QStringLiteral("¤"), QStringLiteral("&curren;")); // krazy:exclude=spelling
	replacements << qMakePair(QStringLiteral("¦"), QStringLiteral("&brvbar;"));
	replacements << qMakePair(QStringLiteral("§"), QStringLiteral("&sect;"));
	replacements << qMakePair(QStringLiteral("µ"), QStringLiteral("&micro;"));
	replacements << qMakePair(QStringLiteral("¹"), QStringLiteral("&sup1;"));
	replacements << qMakePair(QStringLiteral("²"), QStringLiteral("&sup2;"));
	replacements << qMakePair(QStringLiteral("³"), QStringLiteral("&sup3;"));
	replacements << qMakePair(QStringLiteral("¶"), QStringLiteral("&para;"));
	replacements << qMakePair(QStringLiteral("ø"), QStringLiteral("&oslash;"));
	replacements << qMakePair(QStringLiteral("æ"), QStringLiteral("&aelig;"));
	replacements << qMakePair(QStringLiteral("ð"), QStringLiteral("&eth;"));
	replacements << qMakePair(QStringLiteral("ħ"), QStringLiteral("&hbar;"));
	replacements << qMakePair(QStringLiteral("ĸ"), QStringLiteral("&kappa;"));
	replacements << qMakePair(QStringLiteral("¢"), QStringLiteral("&cent;"));
	replacements << qMakePair(QStringLiteral("¼"), QStringLiteral("&frac14;"));
	replacements << qMakePair(QStringLiteral("½"), QStringLiteral("&frac12;"));
	replacements << qMakePair(QStringLiteral("¾"), QStringLiteral("&frac34;"));
	replacements << qMakePair(QStringLiteral("¬"), QStringLiteral("&not;"));
	replacements << qMakePair(QStringLiteral("©"), QStringLiteral("&copy;"));
	replacements << qMakePair(QStringLiteral("®"), QStringLiteral("&reg;"));
	replacements << qMakePair(QStringLiteral("ª"), QStringLiteral("&ordf;"));
	replacements << qMakePair(QStringLiteral("º"), QStringLiteral("&ordm;"));
	replacements << qMakePair(QStringLiteral("±"), QStringLiteral("&plusmn;"));
	replacements << qMakePair(QStringLiteral("¿"), QStringLiteral("&iquest;"));
	replacements << qMakePair(QStringLiteral("×"), QStringLiteral("&times;"));
	replacements << qMakePair(QStringLiteral("°"), QStringLiteral("&deg;"));
	replacements << qMakePair(QStringLiteral("«"), QStringLiteral("&laquo;"));
	replacements << qMakePair(QStringLiteral("»"), QStringLiteral("&raquo;"));
	replacements << qMakePair(QStringLiteral("¯"), QStringLiteral("&macr;"));
	replacements << qMakePair(QStringLiteral("¸"), QStringLiteral("&cedil;"));
	replacements << qMakePair(QStringLiteral("À"), QStringLiteral("&Agrave;"));
	replacements << qMakePair(QStringLiteral("Á"), QStringLiteral("&Aacute;"));
	replacements << qMakePair(QStringLiteral("Â"), QStringLiteral("&Acirc;"));
	replacements << qMakePair(QStringLiteral("Ã"), QStringLiteral("&Atilde;"));
	replacements << qMakePair(QStringLiteral("Å"), QStringLiteral("&Aring;"));
	replacements << qMakePair(QStringLiteral("Æ"), QStringLiteral("&AElig;"));
	replacements << qMakePair(QStringLiteral("Ç"), QStringLiteral("&Ccedil;"));
	replacements << qMakePair(QStringLiteral("È"), QStringLiteral("&Egrave;"));
	replacements << qMakePair(QStringLiteral("É"), QStringLiteral("&Eacute;"));
	replacements << qMakePair(QStringLiteral("Ê"), QStringLiteral("&Ecirc;"));
	replacements << qMakePair(QStringLiteral("Ë"), QStringLiteral("&Euml;"));
	replacements << qMakePair(QStringLiteral("Ì"), QStringLiteral("&Igrave;"));
	replacements << qMakePair(QStringLiteral("Í"), QStringLiteral("&Iacute;"));
	replacements << qMakePair(QStringLiteral("Î"), QStringLiteral("&Icirc;"));
	replacements << qMakePair(QStringLiteral("Ï"), QStringLiteral("&Iuml;"));
	replacements << qMakePair(QStringLiteral("Ð"), QStringLiteral("&ETH;"));
	replacements << qMakePair(QStringLiteral("Ñ"), QStringLiteral("&Ntilde;"));
	replacements << qMakePair(QStringLiteral("Ò"), QStringLiteral("&Ograve;"));
	replacements << qMakePair(QStringLiteral("Ó"), QStringLiteral("&Oacute;"));
	replacements << qMakePair(QStringLiteral("Ô"), QStringLiteral("&Ocirc;"));
	replacements << qMakePair(QStringLiteral("Õ"), QStringLiteral("&Otilde;"));
	replacements << qMakePair(QStringLiteral("Ù"), QStringLiteral("&Ugrave;"));
	replacements << qMakePair(QStringLiteral("Ú"), QStringLiteral("&Uacute;"));
	replacements << qMakePair(QStringLiteral("Û"), QStringLiteral("&Ucirc;"));
	replacements << qMakePair(QStringLiteral("Ý"), QStringLiteral("&Yacute;"));
	replacements << qMakePair(QStringLiteral("Þ"), QStringLiteral("&THORN;"));
	replacements << qMakePair(QStringLiteral("à"), QStringLiteral("&agrave;"));
	replacements << qMakePair(QStringLiteral("á"), QStringLiteral("&aacute;"));
	replacements << qMakePair(QStringLiteral("â"), QStringLiteral("&acirc;"));
	replacements << qMakePair(QStringLiteral("ã"), QStringLiteral("&atilde;"));
	replacements << qMakePair(QStringLiteral("å"), QStringLiteral("&aring;"));
	replacements << qMakePair(QStringLiteral("ç"), QStringLiteral("&ccedil;"));
	replacements << qMakePair(QStringLiteral("è"), QStringLiteral("&egrave;"));
	replacements << qMakePair(QStringLiteral("é"), QStringLiteral("&eacute;"));
	replacements << qMakePair(QStringLiteral("ê"), QStringLiteral("&ecirc;"));
	replacements << qMakePair(QStringLiteral("ë"), QStringLiteral("&euml;"));
	replacements << qMakePair(QStringLiteral("ì"), QStringLiteral("&igrave;"));
	replacements << qMakePair(QStringLiteral("í"), QStringLiteral("&iacute;"));
	replacements << qMakePair(QStringLiteral("î"), QStringLiteral("&icirc;"));
	replacements << qMakePair(QStringLiteral("ï"), QStringLiteral("&iuml;"));
	replacements << qMakePair(QStringLiteral("ñ"), QStringLiteral("&ntilde;"));
	replacements << qMakePair(QStringLiteral("ò"), QStringLiteral("&ograve;"));
	replacements << qMakePair(QStringLiteral("ó"), QStringLiteral("&oacute;"));
	replacements << qMakePair(QStringLiteral("ô"), QStringLiteral("&ocirc;"));
	replacements << qMakePair(QStringLiteral("õ"), QStringLiteral("&otilde;"));
	replacements << qMakePair(QStringLiteral("÷"), QStringLiteral("&divide;"));
	replacements << qMakePair(QStringLiteral("ù"), QStringLiteral("&ugrave;"));
	replacements << qMakePair(QStringLiteral("ú"), QStringLiteral("&uacute;"));
	replacements << qMakePair(QStringLiteral("û"), QStringLiteral("&ucirc;"));
	replacements << qMakePair(QStringLiteral("ý"), QStringLiteral("&yacute;"));
	replacements << qMakePair(QStringLiteral("þ"), QStringLiteral("&thorn;"));
	replacements << qMakePair(QStringLiteral("ÿ"), QStringLiteral("&yuml;"));
	replacements << qMakePair(QStringLiteral("Œ"), QStringLiteral("&#338;"));
	replacements << qMakePair(QStringLiteral("œ"), QStringLiteral("&#339;"));
	replacements << qMakePair(QStringLiteral("Š"), QStringLiteral("&#352;"));
	replacements << qMakePair(QStringLiteral("š"), QStringLiteral("&#353;"));
	replacements << qMakePair(QStringLiteral("Ÿ"), QStringLiteral("&#376;"));
	replacements << qMakePair(QStringLiteral("†"), QStringLiteral("&#8224;"));
	replacements << qMakePair(QStringLiteral("‡"), QStringLiteral("&#8225;"));
	replacements << qMakePair(QStringLiteral("…"), QStringLiteral("&#8230;"));
	replacements << qMakePair(QStringLiteral("‰"), QStringLiteral("&#8240;"));
	replacements << qMakePair(QStringLiteral("™"), QStringLiteral("&#8482;"));

	return replacements;
}

QString OriginProjectParser::replaceSpecialChars(const QString& text) const {
	QString t = text;
	DEBUG(Q_FUNC_INFO << ", got " << t.toStdString())
	for (const auto& r : charReplacementList())
		t.replace(r.first, r.second);
	DEBUG(Q_FUNC_INFO << ", now " << t.toStdString())
	return t;
}

/*!
 * helper function mapping the characters from the Symbol font (outdated and shouldn't be used for html)
 * to Unicode characters, s.a. https://www.alanwood.net/demos/symbol.html
 */
QString greekSymbol(const QString& symbol) {
	// characters in the Symbol-font
	static QStringList symbols{// letters
							   QStringLiteral("A"),
							   QStringLiteral("a"),
							   QStringLiteral("B"),
							   QStringLiteral("b"),
							   QStringLiteral("G"),
							   QStringLiteral("g"),
							   QStringLiteral("D"),
							   QStringLiteral("d"),
							   QStringLiteral("E"),
							   QStringLiteral("e"),
							   QStringLiteral("Z"),
							   QStringLiteral("z"),
							   QStringLiteral("H"),
							   QStringLiteral("h"),
							   QStringLiteral("Q"),
							   QStringLiteral("q"),
							   QStringLiteral("I"),
							   QStringLiteral("i"),
							   QStringLiteral("K"),
							   QStringLiteral("k"),
							   QStringLiteral("L"),
							   QStringLiteral("l"),
							   QStringLiteral("M"),
							   QStringLiteral("m"),
							   QStringLiteral("N"),
							   QStringLiteral("n"),
							   QStringLiteral("X"),
							   QStringLiteral("x"),
							   QStringLiteral("O"),
							   QStringLiteral("o"),
							   QStringLiteral("P"),
							   QStringLiteral("p"),
							   QStringLiteral("R"),
							   QStringLiteral("r"),
							   QStringLiteral("S"),
							   QStringLiteral("s"),
							   QStringLiteral("T"),
							   QStringLiteral("t"),
							   QStringLiteral("U"),
							   QStringLiteral("u"),
							   QStringLiteral("F"),
							   QStringLiteral("f"),
							   QStringLiteral("C"),
							   QStringLiteral("c"),
							   QStringLiteral("Y"),
							   QStringLiteral("y"),
							   QStringLiteral("W"),
							   QStringLiteral("w"),

							   // extra symbols
							   QStringLiteral("V"),
							   QStringLiteral("J"),
							   QStringLiteral("j"),
							   QStringLiteral("v"),
							   QStringLiteral("i")};

	// Unicode friendy codes for greek letters and symbols
	static QStringList unicodeFriendlyCode{// letters
										   QStringLiteral("&Alpha;"),
										   QStringLiteral("&alpha;"),
										   QStringLiteral("&Beta;"),
										   QStringLiteral("&beta;"),
										   QStringLiteral("&Gamma;"),
										   QStringLiteral("&gamma;"),
										   QStringLiteral("&Delta;"),
										   QStringLiteral("&delta;"),
										   QStringLiteral("&Epsilon;"),
										   QStringLiteral("&epsilon;"),
										   QStringLiteral("&Zeta;"),
										   QStringLiteral("&zeta;"),
										   QStringLiteral("&Eta;"),
										   QStringLiteral("&eta;"),
										   QStringLiteral("&Theta;"),
										   QStringLiteral("&theta;"),
										   QStringLiteral("&Iota;"),
										   QStringLiteral("Iota;"),
										   QStringLiteral("&Kappa;"),
										   QStringLiteral("&kappa;"),
										   QStringLiteral("&Lambda;"),
										   QStringLiteral("&lambda;"),
										   QStringLiteral("&Mu;"),
										   QStringLiteral("&mu;"),
										   QStringLiteral("&Nu;"),
										   QStringLiteral("&nu;"),
										   QStringLiteral("&Xi;"),
										   QStringLiteral("&xi;"),
										   QStringLiteral("&Omicron;"),
										   QStringLiteral("&omicron;"),
										   QStringLiteral("&Pi;"),
										   QStringLiteral("&pi;"),
										   QStringLiteral("&Rho;"),
										   QStringLiteral("&rho;"),
										   QStringLiteral("&Sigma;"),
										   QStringLiteral("&sigma;"),
										   QStringLiteral("&Tua;"),
										   QStringLiteral("&tau;"),
										   QStringLiteral("&Upsilon;"),
										   QStringLiteral("&upsilon;"),
										   QStringLiteral("&Phi;"),
										   QStringLiteral("&phi;"),
										   QStringLiteral("&Chi;"),
										   QStringLiteral("&chi;"),
										   QStringLiteral("&Psi;"),
										   QStringLiteral("&psi;"),
										   QStringLiteral("&Omega;"),
										   QStringLiteral("&omega;"),

										   // extra symbols
										   QStringLiteral("&sigmaf;"),
										   QStringLiteral("&thetasym;"),
										   QStringLiteral("&#981;;") /* phi symbol, no friendly code */,
										   QStringLiteral("&piv;"),
										   QStringLiteral("&upsih;")};

	int index = symbols.indexOf(symbol);
	if (index != -1)
		return unicodeFriendlyCode.at(index);
	else
		return QString();
}

/*!
 * converts the string with Origin's syntax for text formatting/highlighting
 * to a string in the richtext/html format supported by Qt.
 * For the supported syntax, see:
 * https://www.originlab.com/doc/LabTalk/ref/Label-cmd
 * https://www.originlab.com/doc/Origin-Help/TextOb-Prop-Text-tab
 * https://doc.qt.io/qt-5/richtext-html-subset.html
 */
QString OriginProjectParser::parseOriginTags(const QString& str) const {
	DEBUG(Q_FUNC_INFO << ", string = " << STDSTRING(str));
	QDEBUG("	UTF8 string: " << str.toUtf8());
	QString line = str;

	// replace %(...) tags
	// 	QRegExp rxcol("\\%\\(\\d+\\)");

	// replace \l(x) (plot legend tags) with \\c{x}, where x is a digit
	line.replace(QRegularExpression(QStringLiteral("\\\\\\s*l\\s*\\(\\s*(\\d+)\\s*\\)")), QStringLiteral("\\c{\\1}"));

	// replace umlauts etc.
	line = replaceSpecialChars(line);

	// replace tabs	(not really supported)
	line.replace(QLatin1Char('\t'), QLatin1String("&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"));

	// In PCRE2 (which is what QRegularExpression uses) variable-length lookbehind is supposed to be
	// exprimental in Perl 5.30; which means it doesn't work at the moment, i.e. using a variable-length
	// negative lookbehind isn't valid syntax from QRegularExpression POV.
	// Ultimately we have to reverse the string and use a negative _lookahead_ instead.
	// The goal is to temporatily replace '(' and ')' that don't denote tags; this is so that we
	// can handle parenthesis that are inside the tag, e.g. '\b(bold (cf))', we want the '(cf)' part
	// to remain as is.
	const QRegularExpression nonTagsRe(QLatin1String(R"(\)([^)(]*)\((?!\s*([buigs\+\-]|\d{1,3}\s*[pc]|[\w ]+\s*:\s*f)\s*\\))"));
	QString linerev = strreverse(line);
	const QString lBracket = strreverse(QStringLiteral("&lbracket;"));
	const QString rBracket = strreverse(QStringLiteral("&rbracket;"));
	linerev.replace(nonTagsRe, rBracket + QStringLiteral("\\1") + lBracket);

	// change the line back to normal
	line = strreverse(linerev);

	// replace \-(...), \+(...), \b(...), \i(...), \u(...), \s(....), \g(...), \f:font(...),
	//  \c'number'(...), \p'size'(...) tags with equivalent supported HTML syntax
	const QRegularExpression tagsRe(QStringLiteral("\\\\\\s*([-+bgisu]|f:(\\w[\\w ]+)|[pc]\\s*(\\d+))\\s*\\(([^()]+?)\\)"));
	QRegularExpressionMatch rmatch;
	while (line.contains(tagsRe, &rmatch)) {
		QString rep;
		const QString tagText = rmatch.captured(4);
		const QString marker = rmatch.captured(1);
		if (marker.startsWith(QLatin1Char('-'))) {
			rep = QStringLiteral("<sub>%1</sub>").arg(tagText);
		} else if (marker.startsWith(QLatin1Char('+'))) {
			rep = QStringLiteral("<sup>%1</sup>").arg(tagText);
		} else if (marker.startsWith(QLatin1Char('b'))) {
			rep = QStringLiteral("<b>%1</b>").arg(tagText);
		} else if (marker.startsWith(QLatin1Char('g'))) { // greek symbols e.g. α φ
			rep = greekSymbol(tagText);
		} else if (marker.startsWith(QLatin1Char('i'))) {
			rep = QStringLiteral("<i>%1</i>").arg(tagText);
		} else if (marker.startsWith(QLatin1Char('s'))) {
			rep = QStringLiteral("<s>%1</s>").arg(tagText);
		} else if (marker.startsWith(QLatin1Char('u'))) {
			rep = QStringLiteral("<u>%1</u>").arg(tagText);
		} else if (marker.startsWith(QLatin1Char('f'))) {
			rep = QStringLiteral("<font face=\"%1\">%2</font>").arg(rmatch.captured(2).trimmed(), tagText);
		} else if (marker.startsWith(QLatin1Char('p'))) { // e.g. \p200(...), means use font-size 200%
			rep = QStringLiteral("<span style=\"font-size: %1%\">%2</span>").arg(rmatch.captured(3), tagText);
		} else if (marker.startsWith(QLatin1Char('c'))) {
			// e.g. \c12(...), set the text color to the corresponding color from
			// the color drop-down list in OriginLab
			const int colorIndex = rmatch.captured(3).toInt();
			Origin::Color c;
			c.type = Origin::Color::ColorType::Regular;
			c.regular = colorIndex <= 23 ? static_cast<Origin::Color::RegularColor>(colorIndex) : Origin::Color::RegularColor::Black;
			QColor color = OriginProjectParser::color(c);
			rep = QStringLiteral("<span style=\"color: %1\">%2</span>").arg(color.name(), tagText);
		}
		line.replace(rmatch.capturedStart(0), rmatch.capturedLength(0), rep);
	}

	// put non-tag '(' and ')' back in their places
	line.replace(QLatin1String("&lbracket;"), QLatin1String("("));
	line.replace(QLatin1String("&rbracket;"), QLatin1String(")"));

	// special characters
	line.replace(QRegularExpression(QStringLiteral("\\\\\\((\\d+)\\)")), QLatin1String("&#\\1;"));

	DEBUG("	result: " << STDSTRING(line));

	return line;
}
