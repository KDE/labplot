/***************************************************************************
    File                 : OriginProjectParser.h
    Project              : LabPlot
    Description          : parser for Origin projects
    --------------------------------------------------------------------
    Copyright            : (C) 2017-2018 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2017-2019 Stefan Gerlach (stefan.gerlach@uni.kn)

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
#include "backend/core/column/Column.h"
#include "backend/core/datatypes/Double2StringFilter.h"
#include "backend/core/datatypes/DateTime2StringFilter.h"
#include "backend/core/Project.h"
#include "backend/core/Workbook.h"
#include "backend/matrix/Matrix.h"
#include "backend/note/Note.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/Axis.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/CartesianPlotLegend.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"
#include "backend/worksheet/plots/cartesian/XYEquationCurve.h"
#include "backend/worksheet/TextLabel.h"
#include "3rdparty/liborigin/OriginFile.h"

#include <KLocalizedString>

#include <QDir>
#include <QDateTime>
#include <QFontMetrics>
#include <QRegularExpression>

/*!
\class OriginProjectParser
\brief parser for Origin projects.

\ingroup datasources
*/

OriginProjectParser::OriginProjectParser() : ProjectParser() {
	m_topLevelClasses = {AspectType::Folder, AspectType::Workbook,
	                     AspectType::Spreadsheet, AspectType::Matrix,
	                     AspectType::Worksheet, AspectType::Note};
}

bool OriginProjectParser::isOriginProject(const QString& fileName) {
	//TODO add opju later when liborigin supports it
	return fileName.endsWith(QLatin1String(".opj"), Qt::CaseInsensitive);
}

void OriginProjectParser::setImportUnusedObjects(bool importUnusedObjects) {
	m_importUnusedObjects = importUnusedObjects;
}

bool OriginProjectParser::hasUnusedObjects() {
	m_originFile = new OriginFile((const char*)m_projectFileName.toLocal8Bit());
	if (!m_originFile->parse()) {
		delete m_originFile;
		m_originFile = nullptr;
		return false;
	}

	for (unsigned int i = 0; i < m_originFile->spreadCount(); i++) {
		const Origin::SpreadSheet& spread = m_originFile->spread(i);
		if (spread.objectID < 0)
			return true;
	}
	for (unsigned int i = 0; i < m_originFile->excelCount(); i++) {
		const Origin::Excel& excel = m_originFile->excel(i);
		if (excel.objectID < 0)
			return true;
	}
	for (unsigned int i = 0; i < m_originFile->matrixCount(); i++) {
		const Origin::Matrix& originMatrix = m_originFile->matrix(i);
		if (originMatrix.objectID < 0)
			return true;
	}

	delete m_originFile;
	m_originFile = nullptr;
	return false;
}

QString OriginProjectParser::supportedExtensions() {
	//TODO add opju later when liborigin supports it
	static const QString extensions = "*.opj *.OPJ";
	return extensions;
}

unsigned int OriginProjectParser::findSpreadByName(const QString& name) {
	for (unsigned int i = 0; i < m_originFile->spreadCount(); i++) {
		const Origin::SpreadSheet& spread = m_originFile->spread(i);
		if (spread.name == name.toStdString()) {
			m_spreadNameList << name;
			return i;
		}
	}
	return 0;
}
unsigned int OriginProjectParser::findMatrixByName(const QString& name) {
	for (unsigned int i = 0; i < m_originFile->matrixCount(); i++) {
		const Origin::Matrix& originMatrix = m_originFile->matrix(i);
		if (originMatrix.name == name.toStdString()) {
			m_matrixNameList << name;
			return i;
		}
	}
	return 0;
}
unsigned int OriginProjectParser::findExcelByName(const QString& name) {
	for (unsigned int i = 0; i < m_originFile->excelCount(); i++) {
		const Origin::Excel& excel = m_originFile->excel(i);
		if (excel.name == name.toStdString()) {
			m_excelNameList << name;
			return i;
		}
	}
	return 0;
}
unsigned int OriginProjectParser::findGraphByName(const QString& name) {
	for (unsigned int i = 0; i < m_originFile->graphCount(); i++) {
		const Origin::Graph& graph = m_originFile->graph(i);
		if (graph.name == name.toStdString()) {
			m_graphNameList << name;
			return i;
		}
	}
	return 0;
}
unsigned int OriginProjectParser::findNoteByName(const QString& name) {
	for (unsigned int i = 0; i < m_originFile->noteCount(); i++) {
		const Origin::Note& originNote = m_originFile->note(i);
		if (originNote.name == name.toStdString()) {
			m_noteNameList << name;
			return i;
		}
	}
	return 0;
}

//##############################################################################
//############## Deserialization from Origin's project tree ####################
//##############################################################################
bool OriginProjectParser::load(Project* project, bool preview) {
	DEBUG("OriginProjectParser::load()");

	//read and parse the m_originFile-file
	m_originFile = new OriginFile((const char*)m_projectFileName.toLocal8Bit());
	if (!m_originFile->parse()) {
		delete m_originFile;
		m_originFile = nullptr;
		return false;
	}

	//Origin project tree and the iterator pointing to the root node
	const tree<Origin::ProjectNode>* projectTree = m_originFile->project();
	tree<Origin::ProjectNode>::iterator projectIt = projectTree->begin(projectTree->begin());

	m_spreadNameList.clear();
	m_excelNameList.clear();
	m_matrixNameList.clear();
	m_graphNameList.clear();
	m_noteNameList.clear();

	//convert the project tree from liborigin's representation to LabPlot's project object
	project->setIsLoading(true);
	if (projectIt.node) { // only opj files from version >= 6.0 do have project tree
		DEBUG("	have a project tree");
		QString name(QString::fromLatin1(projectIt->name.c_str()));
		project->setName(name);
		project->setCreationTime(creationTime(projectIt));
		loadFolder(project, projectIt, preview);
	} else { // for lower versions put all windows on rootfolder
		DEBUG("	have no project tree");
		int pos = m_projectFileName.lastIndexOf(QLatin1String("/")) + 1;
		project->setName((const char*)m_projectFileName.mid(pos).toLocal8Bit());
	}
	// imports all loose windows (like prior version 6 which has no project tree)
	handleLooseWindows(project, preview);

	//restore column pointers:
	//1. extend the pathes to contain the parent structures first
	//2. restore the pointers from the pathes
	const QVector<Column*> columns = project->children<Column>(AbstractAspect::ChildIndexFlag::Recursive);
	const QVector<Spreadsheet*> spreadsheets = project->children<Spreadsheet>(AbstractAspect::ChildIndexFlag::Recursive);
	for (auto* curve : project->children<XYCurve>(AbstractAspect::ChildIndexFlag::Recursive)) {
		curve->suppressRetransform(true);

		//x-column
		QString spreadsheetName = curve->xColumnPath().left(curve->xColumnPath().indexOf(QLatin1Char('/')));
		for (const auto* spreadsheet : spreadsheets) {
			if (spreadsheet->name() == spreadsheetName) {
				const QString& newPath = spreadsheet->parentAspect()->path() + '/' + curve->xColumnPath();
				curve->setXColumnPath(newPath);

				for (auto* column : columns) {
					if (!column)
						continue;
					if (column->path() == newPath) {
						curve->setXColumn(column);
						break;
					}
				}
				break;
			}
		}

		//x-column
		spreadsheetName = curve->yColumnPath().left(curve->yColumnPath().indexOf(QLatin1Char('/')));
		for (const auto* spreadsheet : spreadsheets) {
			if (spreadsheet->name() == spreadsheetName) {
				const QString& newPath = spreadsheet->parentAspect()->path() + '/' + curve->yColumnPath();
				curve->setYColumnPath(newPath);

				for (auto* column : columns) {
					if (!column)
						continue;
					if (column->path() == newPath) {
						curve->setYColumn(column);
						break;
					}
				}
				break;
			}
		}

		//TODO: error columns


		curve->suppressRetransform(false);
	}

	if (!preview) {
		for (auto* plot : project->children<CartesianPlot>(AbstractAspect::ChildIndexFlag::Recursive)) {
			plot->setIsLoading(false);
			plot->retransform();
		}
	}

	emit project->loaded();
	project->setIsLoading(false);

	delete m_originFile;
	m_originFile = nullptr;

	return true;
}

bool OriginProjectParser::loadFolder(Folder* folder, tree<Origin::ProjectNode>::iterator baseIt, bool preview) {
	DEBUG("OriginProjectParser::loadFolder()")
	const tree<Origin::ProjectNode>* projectTree = m_originFile->project();

	// do not skip anything if pathesToLoad() contains only root folder
	bool containsRootFolder = (folder->pathesToLoad().size() == 1 && folder->pathesToLoad().contains(folder->path()));
	if (containsRootFolder) {
		DEBUG("	pathesToLoad contains only folder path \""  << STDSTRING(folder->path()) << "\". Clearing pathes to load.")
		folder->setPathesToLoad(QStringList());
	}

	//load folder's children: logic for reading the selected objects only is similar to Folder::readChildAspectElement
	for (tree<Origin::ProjectNode>::sibling_iterator it = projectTree->begin(baseIt); it != projectTree->end(baseIt); ++it) {
		QString name(QString::fromLatin1(it->name.c_str())); //name of the current child
		DEBUG("	* folder item name = " << STDSTRING(name))

		//check whether we need to skip the loading of the current child
		if (!folder->pathesToLoad().isEmpty()) {
			//child's path is not available yet (child not added yet) -> construct the path manually
			const QString childPath = folder->path() + '/' + name;
			DEBUG("		path = " << STDSTRING(childPath))

			//skip the current child aspect it is not in the list of aspects to be loaded
			if (folder->pathesToLoad().indexOf(childPath) == -1) {
				DEBUG("		skip it!")
				continue;
			}
		}

		//load top-level children
		AbstractAspect* aspect = nullptr;
		switch (it->type) {
		case Origin::ProjectNode::Folder: {
			DEBUG("	top level folder");
			Folder* f = new Folder(name);

			if (!folder->pathesToLoad().isEmpty()) {
				//a child folder to be read -> provide the list of aspects to be loaded to the child folder, too.
				//since the child folder and all its children are not added yet (path() returns empty string),
				//we need to remove the path of the current child folder from the full pathes provided in pathesToLoad.
				//E.g. we want to import the path "Project/Folder/Spreadsheet" in the following project
				// Project
				//        \Spreadsheet
				//        \Folder
				//               \Spreadsheet
				//
				//Here, we remove the part "Project/Folder/" and proceed for this child folder with "Spreadsheet" only.
				//With this the logic above where it is determined whether to import the child aspect or not works out.

				//manually construct the path of the child folder to be read
				const QString& curFolderPath = folder->path()  + '/' + name;

				//remove the path of the current child folder
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
			DEBUG("	top level spreadsheet");
			Spreadsheet* spreadsheet = new Spreadsheet(name);
			loadSpreadsheet(spreadsheet, preview, name);
			aspect = spreadsheet;
			break;
		}
		case Origin::ProjectNode::Graph: {
			DEBUG("	top level graph");
			Worksheet* worksheet = new Worksheet(name);
			worksheet->setIsLoading(true);
			worksheet->setTheme(QString());
			loadWorksheet(worksheet, preview);
			aspect = worksheet;
			break;
		}
		case Origin::ProjectNode::Matrix: {
			DEBUG("	top level matrix");
			const Origin::Matrix& originMatrix = m_originFile->matrix(findMatrixByName(name));
			DEBUG("	matrix name = " << originMatrix.name);
			DEBUG("	number of sheets = " << originMatrix.sheets.size());
			if (originMatrix.sheets.size() == 1) {
				// single sheet -> load into a matrix
				Matrix* matrix = new Matrix(name);
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
			DEBUG("	top level excel");
			Workbook* workbook = new Workbook(name);
			loadWorkbook(workbook, preview);
			aspect = workbook;
			break;
		}
		case Origin::ProjectNode::Note: {
			DEBUG("top level note");
			Note* note = new Note(name);
			loadNote(note, preview);
			aspect = note;
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
			aspect->setIsLoading(false);
		}
	}

	// ResultsLog
	QString resultsLog = QString::fromLatin1(m_originFile->resultsLogString().c_str());
	if (resultsLog.length() > 0) {
		DEBUG("Results log:\t\tyes");
		Note* note = new Note("ResultsLog");

		if (preview)
			folder->addChildFast(note);
		else {
			//only import the log if it is in the list of aspects to be loaded
			const QString childPath = folder->path() + '/' + note->name();
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
	DEBUG("OriginProjectParser::handleLooseWindows()");
	QDEBUG("pathes to load:" << folder->pathesToLoad());
	m_spreadNameList.removeDuplicates();
	m_excelNameList.removeDuplicates();
	m_matrixNameList.removeDuplicates();
	m_graphNameList.removeDuplicates();
	m_noteNameList.removeDuplicates();
	QDEBUG("	spreads =" << m_spreadNameList);
	QDEBUG("	excels =" << m_excelNameList);
	QDEBUG("	matrices =" << m_matrixNameList);
	QDEBUG("	graphs =" << m_graphNameList);
	QDEBUG("	notes =" << m_noteNameList);

	DEBUG("Number of spreads loaded:\t" << m_spreadNameList.size() << ", in file: " << m_originFile->spreadCount());
	DEBUG("Number of excels loaded:\t" << m_excelNameList.size() << ", in file: " << m_originFile->excelCount());
	DEBUG("Number of matrices loaded:\t" << m_matrixNameList.size() << ", in file: " << m_originFile->matrixCount());
	DEBUG("Number of graphs loaded:\t" << m_graphNameList.size() << ", in file: " << m_originFile->graphCount());
	DEBUG("Number of notes loaded:\t\t" << m_noteNameList.size() << ", in file: " << m_originFile->noteCount());

	// loop over all spreads to find loose ones
	for (unsigned int i = 0; i < m_originFile->spreadCount(); i++) {
		AbstractAspect* aspect = nullptr;
		const Origin::SpreadSheet& spread = m_originFile->spread(i);
		QString name = QString::fromStdString(spread.name);

		DEBUG("	spread.objectId = " << spread.objectID);
		// skip unused spreads if selected
		if (spread.objectID < 0 && !m_importUnusedObjects) {
			DEBUG("	Dropping unused loose spread: " << STDSTRING(name));
			continue;
		}

		const QString childPath = folder->path() + '/' + name;
		// we could also use spread.loose
		if (!m_spreadNameList.contains(name) && (preview || folder->pathesToLoad().indexOf(childPath) != -1)) {
			DEBUG("	Adding loose spread: " << STDSTRING(name));

			Spreadsheet* spreadsheet = new Spreadsheet(name);
			loadSpreadsheet(spreadsheet, preview, name);
			aspect = spreadsheet;
		}
		if (aspect) {
			folder->addChildFast(aspect);
			DEBUG("	creation time as reported by liborigin: " << spread.creationDate);
			aspect->setCreationTime(QDateTime::fromTime_t(spread.creationDate));
		}
	}
	// loop over all excels to find loose ones
	for (unsigned int i = 0; i < m_originFile->excelCount(); i++) {
		AbstractAspect* aspect = nullptr;
		const Origin::Excel& excel = m_originFile->excel(i);
		QString name = QString::fromStdString(excel.name);

		DEBUG("	excel.objectId = " << excel.objectID);
		// skip unused data sets if selected
		if (excel.objectID < 0 && !m_importUnusedObjects) {
			DEBUG("	Dropping unused loose excel: " << STDSTRING(name));
			continue;
		}

		const QString childPath = folder->path() + '/' + name;
		// we could also use excel.loose
		if (!m_excelNameList.contains(name) && (preview || folder->pathesToLoad().indexOf(childPath) != -1)) {
			DEBUG("	Adding loose excel: " << STDSTRING(name));
			DEBUG("	 containing number of sheets = " << excel.sheets.size());

			Workbook* workbook = new Workbook(name);
			loadWorkbook(workbook, preview);
			aspect = workbook;
		}
		if (aspect) {
			folder->addChildFast(aspect);
			DEBUG("	creation time as reported by liborigin: " << excel.creationDate);
			aspect->setCreationTime(QDateTime::fromTime_t(excel.creationDate));
		}
	}
	// loop over all matrices to find loose ones
	for (unsigned int i = 0; i < m_originFile->matrixCount(); i++) {
		AbstractAspect* aspect = nullptr;
		const Origin::Matrix& originMatrix = m_originFile->matrix(i);
		QString name = QString::fromStdString(originMatrix.name);

		DEBUG("	originMatrix.objectId = " << originMatrix.objectID);
		// skip unused data sets if selected
		if (originMatrix.objectID < 0 && !m_importUnusedObjects) {
			DEBUG("	Dropping unused loose matrix: " << STDSTRING(name));
			continue;
		}

		const QString childPath = folder->path() + '/' + name;
		if (!m_matrixNameList.contains(name) && (preview || folder->pathesToLoad().indexOf(childPath) != -1)) {
			DEBUG("	Adding loose matrix: " << STDSTRING(name));
			DEBUG("	containing number of sheets = " << originMatrix.sheets.size());
			if (originMatrix.sheets.size() == 1) { // single sheet -> load into a matrix
				Matrix* matrix = new Matrix(name);
				loadMatrix(matrix, preview);
				aspect = matrix;
			} else { // multiple sheets -> load into a workbook
				Workbook* workbook = new Workbook(name);
				loadMatrixWorkbook(workbook, preview);
				aspect = workbook;
			}
		}
		if (aspect) {
			folder->addChildFast(aspect);
			aspect->setCreationTime(QDateTime::fromTime_t(originMatrix.creationDate));
		}
	}
	// handle loose graphs (is this even possible?)
	for (unsigned int i = 0; i < m_originFile->graphCount(); i++) {
		AbstractAspect* aspect = nullptr;
		const Origin::Graph& graph = m_originFile->graph(i);
		QString name = QString::fromStdString(graph.name);

		DEBUG("	graph.objectId = " << graph.objectID);
		// skip unused graph if selected
		if (graph.objectID < 0 && !m_importUnusedObjects) {
			DEBUG("	Dropping unused loose graph: " << STDSTRING(name));
			continue;
		}

		const QString childPath = folder->path() + '/' + name;
		if (!m_graphNameList.contains(name) && (preview || folder->pathesToLoad().indexOf(childPath) != -1)) {
			DEBUG("	Adding loose graph: " << STDSTRING(name));
			Worksheet* worksheet = new Worksheet(name);
			loadWorksheet(worksheet, preview);
			aspect = worksheet;
		}
		if (aspect) {
			folder->addChildFast(aspect);
			aspect->setCreationTime(QDateTime::fromTime_t(graph.creationDate));
		}
	}
	// handle loose notes (is this even possible?)
	for (unsigned int i = 0; i < m_originFile->noteCount(); i++) {
		AbstractAspect* aspect = nullptr;
		const Origin::Note& originNote = m_originFile->note(i);
		QString name = QString::fromStdString(originNote.name);

		DEBUG("	originNote.objectId = " << originNote.objectID);
		// skip unused notes if selected
		if (originNote.objectID < 0 && !m_importUnusedObjects) {
			DEBUG("	Dropping unused loose note: " << STDSTRING(name));
			continue;
		}

		const QString childPath = folder->path() + '/' + name;
		if (!m_noteNameList.contains(name) && (preview || folder->pathesToLoad().indexOf(childPath) != -1)) {
			DEBUG("	Adding loose note: " << STDSTRING(name));
			Note* note = new Note(name);
			loadNote(note, preview);
			aspect = note;
		}
		if (aspect) {
			folder->addChildFast(aspect);
			aspect->setCreationTime(QDateTime::fromTime_t(originNote.creationDate));
		}
	}
}

bool OriginProjectParser::loadWorkbook(Workbook* workbook, bool preview) {
	DEBUG(Q_FUNC_INFO);
	//load workbook sheets
	const Origin::Excel& excel = m_originFile->excel(findExcelByName(workbook->name()));
	DEBUG(Q_FUNC_INFO << " excel name = " << excel.name);
	DEBUG(Q_FUNC_INFO << " number of sheets = " << excel.sheets.size());
	for (unsigned int s = 0; s < excel.sheets.size(); ++s) {
		Spreadsheet* spreadsheet = new Spreadsheet(QString::fromLatin1(excel.sheets[s].name.c_str()));
		loadSpreadsheet(spreadsheet, preview, workbook->name(), s);
		workbook->addChildFast(spreadsheet);
	}

	return true;
}

// load spreadsheet from spread (sheetIndex == -1) or from excel (only sheet sheetIndex)
bool OriginProjectParser::loadSpreadsheet(Spreadsheet* spreadsheet, bool preview, const QString& name, int sheetIndex) {
	DEBUG("loadSpreadsheet() sheetIndex = " << sheetIndex);

	//load spreadsheet data
	Origin::SpreadSheet spread;
	Origin::Excel excel;
	if (sheetIndex == -1)	// spread
		spread = m_originFile->spread(findSpreadByName(name));
	else {
		excel = m_originFile->excel(findExcelByName(name));
		spread = excel.sheets[sheetIndex];
	}

	const size_t cols = spread.columns.size();
	int rows = 0;
	for (size_t j = 0; j < cols; ++j)
		rows = std::max((int)spread.columns[j].data.size(), rows);
	// alternative: int rows = excel.maxRows;
	DEBUG("loadSpreadsheet() cols/maxRows = " << cols << "/" << rows);

	//TODO QLocale locale = mw->locale();

	spreadsheet->setRowCount(rows);
	spreadsheet->setColumnCount((int)cols);
	if (sheetIndex == -1)
		spreadsheet->setComment(QString::fromLatin1(spread.label.c_str()));
	else
		spreadsheet->setComment(QString::fromLatin1(excel.label.c_str()));

	//in Origin column width is measured in characters, we need to convert to pixels
	//TODO: determine the font used in Origin in order to get the same column width as in Origin
	QFont font;
	QFontMetrics fm(font);
	const int scaling_factor = fm.maxWidth();

	for (size_t j = 0; j < cols; ++j) {
		Origin::SpreadColumn column = spread.columns[j];
		Column* col = spreadsheet->column((int)j);

		QString name(column.name.c_str());
		col->setName(name.replace(QRegExp(".*_"), QString()));

		if (preview)
			continue;

		//TODO: we don't support any formulas for cells yet.
// 		if (column.command.size() > 0)
// 			col->setFormula(Interval<int>(0, rows), QString(column.command.c_str()));

		col->setComment(QString::fromLatin1(column.comment.c_str()));
		col->setWidth((int)column.width * scaling_factor);

		//plot designation
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
			//A TextNumeric column can contain numeric and string values, there is no equivalent column mode in LabPlot.
			// -> Set the column mode as 'Numeric' or 'Text' depending on the type of first non-empty element in column.
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

			if (col->columnMode() == AbstractColumn::ColumnMode::Numeric) {
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
							col->setTextAt(i, value.as_string());
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
			for (int i = 0; i < min((int)column.data.size(), rows); ++i)
				col->setTextAt(i, column.data[i].as_string());
			break;
		case Origin::Time: {
			switch (column.valueTypeSpecification + 128) {
			case Origin::TIME_HH_MM:
				format = "hh:mm";
				break;
			case Origin::TIME_HH:
				format = "hh";
				break;
			case Origin::TIME_HH_MM_SS:
				format = "hh:mm:ss";
				break;
			case Origin::TIME_HH_MM_SS_ZZ:
				format = "hh:mm:ss.zzz";
				break;
			case Origin::TIME_HH_AP:
				format = "hh ap";
				break;
			case Origin::TIME_HH_MM_AP:
				format = "hh:mm ap";
				break;
			case Origin::TIME_MM_SS:
				format = "mm:ss";
				break;
			case Origin::TIME_MM_SS_ZZ:
				format = "mm:ss.zzz";
				break;
			case Origin::TIME_HHMM:
				format = "hhmm";
				break;
			case Origin::TIME_HHMMSS:
				format = "hhmmss";
				break;
			case Origin::TIME_HH_MM_SS_ZZZ:
				format = "hh:mm:ss.zzz";
				break;
			}

			for (int i = 0; i < min((int)column.data.size(), rows); ++i)
				col->setValueAt(i, column.data[i].as_double());
			col->setColumnMode(AbstractColumn::ColumnMode::DateTime);

			auto* filter = static_cast<DateTime2StringFilter*>(col->outputFilter());
			filter->setFormat(format);
			break;
		}
		case Origin::Date: {
			switch (column.valueTypeSpecification) {
			case Origin::DATE_DD_MM_YYYY:
				format = "dd/MM/yyyy";
				break;
			case Origin::DATE_DD_MM_YYYY_HH_MM:
				format = "dd/MM/yyyy HH:mm";
				break;
			case Origin::DATE_DD_MM_YYYY_HH_MM_SS:
				format = "dd/MM/yyyy HH:mm:ss";
				break;
			case Origin::DATE_DDMMYYYY:
			case Origin::DATE_DDMMYYYY_HH_MM:
			case Origin::DATE_DDMMYYYY_HH_MM_SS:
				format = "dd.MM.yyyy";
					break;
			case Origin::DATE_MMM_D:
				format = "MMM d";
				break;
			case Origin::DATE_M_D:
				format = "M/d";
				break;
			case Origin::DATE_D:
				format = 'd';
				break;
			case Origin::DATE_DDD:
			case Origin::DATE_DAY_LETTER:
				format = "ddd";
				break;
			case Origin::DATE_YYYY:
				format = "yyyy";
				break;
			case Origin::DATE_YY:
				format = "yy";
				break;
			case Origin::DATE_YYMMDD:
			case Origin::DATE_YYMMDD_HH_MM:
			case Origin::DATE_YYMMDD_HH_MM_SS:
			case Origin::DATE_YYMMDD_HHMM:
			case Origin::DATE_YYMMDD_HHMMSS:
				format = "yyMMdd";
				break;
			case Origin::DATE_MMM:
			case Origin::DATE_MONTH_LETTER:
				format = "MMM";
				break;
			case Origin::DATE_M_D_YYYY:
				format = "M-d-yyyy";
				break;
			default:
				format = "dd.MM.yyyy";
			}

			for (int i = 0; i < min((int)column.data.size(), rows); ++i)
				col->setValueAt(i, column.data[i].as_double());
			col->setColumnMode(AbstractColumn::ColumnMode::DateTime);

			auto* filter = static_cast<DateTime2StringFilter*>(col->outputFilter());
			filter->setFormat(format);
			break;
		}
		case Origin::Month: {
			switch (column.valueTypeSpecification) {
			case Origin::MONTH_MMM:
				format = "MMM";
				break;
			case Origin::MONTH_MMMM:
				format = "MMMM";
				break;
			case Origin::MONTH_LETTER:
				format = 'M';
				break;
			}

			for (int i = 0; i < min((int)column.data.size(), rows); ++i)
				col->setValueAt(i, column.data[i].as_double());
			col->setColumnMode(AbstractColumn::ColumnMode::Month);

			auto* filter = static_cast<DateTime2StringFilter*>(col->outputFilter());
			filter->setFormat(format);
			break;
		}
		case Origin::Day: {
			switch (column.valueTypeSpecification) {
			case Origin::DAY_DDD:
				format = "ddd";
				break;
			case Origin::DAY_DDDD:
				format = "dddd";
				break;
			case Origin::DAY_LETTER:
				format = 'd';
				break;
			}

			for (int i = 0; i < min((int)column.data.size(), rows); ++i)
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

	//TODO: "hidden" not supported yet
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
	DEBUG("loadMatrixWorkbook()");
	//load matrix workbook sheets
	const Origin::Matrix& originMatrix = m_originFile->matrix(findMatrixByName(workbook->name()));
	for (size_t s = 0; s < originMatrix.sheets.size(); ++s) {
		Matrix* matrix = new Matrix(QString::fromLatin1(originMatrix.sheets[s].name.c_str()));
		loadMatrix(matrix, preview, s, workbook->name());
		workbook->addChildFast(matrix);
	}

	return true;
}

bool OriginProjectParser::loadMatrix(Matrix* matrix, bool preview, size_t sheetIndex, const QString& mwbName) {
	DEBUG("loadMatrix()");
	//import matrix data
	const Origin::Matrix& originMatrix = m_originFile->matrix(findMatrixByName(mwbName));

	if (preview)
		return true;


	//in Origin column width is measured in characters, we need to convert to pixels
	//TODO: determine the font used in Origin in order to get the same column width as in Origin
	QFont font;
	QFontMetrics fm(font);
	const int scaling_factor = fm.maxWidth();

	const Origin::MatrixSheet& layer = originMatrix.sheets[sheetIndex];
	const int colCount = layer.columnCount;
	const int rowCount = layer.rowCount;

	matrix->setRowCount(rowCount);
	matrix->setColumnCount(colCount);
	matrix->setFormula(layer.command.c_str());

	//TODO: how to handle different widths for different columns?
	for (int j = 0; j < colCount; j++)
		matrix->setColumnWidth(j, layer.width * scaling_factor);

	//TODO: check column major vs. row major to improve the performance here
	for (int i = 0; i < rowCount; i++) {
		for (int j = 0; j < colCount; j++)
			matrix->setCell(i, j, layer.data[j + i*colCount]);
	}

	char format = 'g';
	//TODO: prec not support by Matrix
	//int prec = 6;
	switch (layer.valueTypeSpecification) {
	case 0: //Decimal 1000
		format = 'f';
	//	prec = layer.decimalPlaces;
		break;
	case 1: //Scientific
		format = 'e';
	//	prec = layer.decimalPlaces;
		break;
	case 2: //Engineering
	case 3: //Decimal 1,000
		format = 'g';
	//	prec = layer.significantDigits;
		break;
	}

	matrix->setNumericFormat(format);

	return true;
}

bool OriginProjectParser::loadWorksheet(Worksheet* worksheet, bool preview) {
	DEBUG(Q_FUNC_INFO << ", preview = " << preview);

	//load worksheet data
	const Origin::Graph& graph = m_originFile->graph(findGraphByName(worksheet->name()));
	DEBUG(Q_FUNC_INFO << ", graph name = " << graph.name);
	worksheet->setComment(graph.label.c_str());

	//TODO: width, height, view mode (print view, page view, window view, draft view)
	//Origin allows to freely resize the window and ajusts the size of the plot (layer) automatically
	//by keeping a certain width-to-height ratio. It's not clear what the actual size of the plot/layer is and how to handle this.
	//For now we simply create a new wokrsheet here with it's default size and make it using the whole view size.
	//Later we can decide to use one of the following properties:
	// 1) Window.frameRect gives Rect-corner coordinates (in pixels) of the Window object
	// 2) GraphLayer.clientRect gives Rect-corner coordinates (pixels) of the Layer inside the (printer?) page.
	// 3) Graph.width, Graph.height give the (printer?) page size in pixels.
// 	const QRectF size(0, 0,
// 					  Worksheet::convertToSceneUnits(graph.width/600., Worksheet::Inch),
// 					  Worksheet::convertToSceneUnits(graph.height/600., Worksheet::Inch));
// 	worksheet->setPageRect(size);
	worksheet->setUseViewSize(true);

	QHash<TextLabel*, QSizeF> textLabelPositions;

	// worksheet background color
	const Origin::ColorGradientDirection bckgColorGradient = graph.windowBackgroundColorGradient;
	const Origin::Color bckgBaseColor = graph.windowBackgroundColorBase;
	const Origin::Color bckgEndColor =  graph.windowBackgroundColorEnd;
	worksheet->setBackgroundColorStyle(backgroundColorStyle(bckgColorGradient));
	switch (bckgColorGradient) {
		case Origin::ColorGradientDirection::NoGradient:
		case Origin::ColorGradientDirection::TopLeft:
		case Origin::ColorGradientDirection::Left:
		case Origin::ColorGradientDirection::BottomLeft:
		case Origin::ColorGradientDirection::Top:
			worksheet->setBackgroundFirstColor(color(bckgEndColor));
			worksheet->setBackgroundSecondColor(color(bckgBaseColor));
			break;
		case Origin::ColorGradientDirection::Center:
			break;
		case Origin::ColorGradientDirection::Bottom:
		case Origin::ColorGradientDirection::TopRight:
		case Origin::ColorGradientDirection::Right:
		case Origin::ColorGradientDirection::BottomRight:
			worksheet->setBackgroundFirstColor(color(bckgBaseColor));
			worksheet->setBackgroundSecondColor(color(bckgEndColor));
	}

	//TODO: do we need changes on the worksheet layout?

	//add plots
	int index = 1;
	for (const auto& layer : graph.layers) {
		if (!layer.is3D()) {
			CartesianPlot* plot = new CartesianPlot(i18n("Plot%1", QString::number(index)));
			worksheet->addChildFast(plot);

			if (preview)
				continue;

			plot->setIsLoading(true);
			//TODO: width, height

			//background color
			const Origin::Color& regColor = layer.backgroundColor;
			if (regColor.type == Origin::Color::None)
				plot->plotArea()->setBackgroundOpacity(0);
			else
				plot->plotArea()->setBackgroundFirstColor(color(regColor));

			//border
			if (layer.borderType == Origin::BorderType::None)
				plot->plotArea()->setBorderPen(QPen(Qt::NoPen));
			else
				plot->plotArea()->setBorderPen(QPen(Qt::SolidLine));

			//ranges
			plot->setAutoScaleX(-1, false);
			plot->setAutoScaleY(-1, false);
			const Origin::GraphAxis& originXAxis = layer.xAxis;
			const Origin::GraphAxis& originYAxis = layer.yAxis;

			const Range<double> xRange(originXAxis.min, originXAxis.max);
			const Range<double> yRange(originYAxis.min, originYAxis.max);
			plot->setXRange(xRange);
			plot->setYRange(yRange);

			//scales
			switch (originXAxis.scale) {
			case Origin::GraphAxis::Linear:
				plot->setXRangeScale(RangeT::Scale::Linear);
				break;
			case Origin::GraphAxis::Log10:
				plot->setXRangeScale(RangeT::Scale::Log10);
				break;
			case Origin::GraphAxis::Ln:
				plot->setXRangeScale(RangeT::Scale::Ln);
				break;
			case Origin::GraphAxis::Log2:
				plot->setXRangeScale(RangeT::Scale::Log2);
				break;
			case Origin::GraphAxis::Probability:
			case Origin::GraphAxis::Probit:
			case Origin::GraphAxis::Reciprocal:
			case Origin::GraphAxis::OffsetReciprocal:
			case Origin::GraphAxis::Logit:
				//TODO:
				plot->setXRangeScale(RangeT::Scale::Linear);
				break;
			}

			switch (originYAxis.scale) {
			case Origin::GraphAxis::Linear:
				plot->setYRangeScale(RangeT::Scale::Linear);
				break;
			case Origin::GraphAxis::Log10:
				plot->setYRangeScale(RangeT::Scale::Log10);
				break;
			case Origin::GraphAxis::Ln:
				plot->setYRangeScale(RangeT::Scale::Ln);
				break;
			case Origin::GraphAxis::Log2:
				plot->setYRangeScale(RangeT::Scale::Log2);
				break;
			case Origin::GraphAxis::Probability:
			case Origin::GraphAxis::Probit:
			case Origin::GraphAxis::Reciprocal:
			case Origin::GraphAxis::OffsetReciprocal:
			case Origin::GraphAxis::Logit:
				//TODO:
				plot->setYRangeScale(RangeT::Scale::Linear);
				break;
			}

			//axes
			//x bottom
			if (layer.curves.size()) {
				Origin::GraphCurve originCurve = layer.curves[0];
				QString xColumnName = QString::fromLatin1(originCurve.xColumnName.c_str());
				//TODO: "Partikelgrö"
				DEBUG("	xColumnName = " << STDSTRING(xColumnName));
				QDEBUG("	UTF8 xColumnName = " << xColumnName.toUtf8());
				QString yColumnName = QString::fromLatin1(originCurve.yColumnName.c_str());

				if (!originXAxis.formatAxis[0].hidden) {
					Axis* axis = new Axis("x", Axis::Orientation::Horizontal);
					axis->setSuppressRetransform(true);
					axis->setPosition(Axis::Position::Bottom);
					plot->addChildFast(axis);
					loadAxis(originXAxis, axis, 0, xColumnName);
					axis->setSuppressRetransform(false);
				}

				//x top
				if (!originXAxis.formatAxis[1].hidden) {
					Axis* axis = new Axis("x top", Axis::Orientation::Horizontal);
					axis->setPosition(Axis::Position::Top);
					axis->setSuppressRetransform(true);
					plot->addChildFast(axis);
					loadAxis(originXAxis, axis, 1, xColumnName);
					axis->setSuppressRetransform(false);
				}

				//y left
				if (!originYAxis.formatAxis[0].hidden) {
					Axis* axis = new Axis("y", Axis::Orientation::Vertical);
					axis->setSuppressRetransform(true);
					axis->setPosition(Axis::Position::Left);
					plot->addChildFast(axis);
					loadAxis(originYAxis, axis, 0, yColumnName);
					axis->setSuppressRetransform(false);
				}

				//y right
				if (!originYAxis.formatAxis[1].hidden) {
					Axis* axis = new Axis("y right", Axis::Orientation::Vertical);
					axis->setSuppressRetransform(true);
					axis->setPosition(Axis::Position::Right);
					plot->addChildFast(axis);
					loadAxis(originYAxis, axis, 1, yColumnName);
					axis->setSuppressRetransform(false);
				}
			} else {
				//TODO: ?
			}

			//range breaks
			//TODO

			//add legend if available
			const Origin::TextBox& originLegend = layer.legend;
			const QString& legendText = QString::fromLatin1(originLegend.text.c_str());
			DEBUG(" legend text = " << STDSTRING(legendText));
			if (!originLegend.text.empty()) {
				auto* legend = new CartesianPlotLegend(i18n("legend"));

				//Origin's legend uses "\l(...)" or "\L(...)" string to format the legend symbol
				// and "%(...) to format the legend text for each curve
				//s. a. https://www.originlab.com/doc/Origin-Help/Legend-ManualControl
				//the text before these formatting tags, if available, is interpreted as the legend title
				QString legendTitle;

				//search for the first occurrence of the legend symbol substring
				int index = legendText.indexOf(QLatin1String("\\l("), 0, Qt::CaseInsensitive);
				if (index != -1)
					legendTitle = legendText.left(index);
				else {
					//check legend text
					index = legendText.indexOf(QLatin1String("%("));
					if (index != -1)
						legendTitle = legendText.left(index);
				}

				legendTitle = legendTitle.trimmed();
				if (!legendTitle.isEmpty())
					legendTitle = parseOriginText(legendTitle);

				DEBUG(" legend title = " << STDSTRING(legendTitle));
				legend->title()->setText(legendTitle);

				//TODO: text color
				//const Origin::Color& originColor = originLegend.color;

				//position
				//TODO: for the first release with OPJ support we put the legend to the bottom left corner,
				//in the next release we'll evaluate originLegend.clientRect giving the position inside of the whole page in Origin.
				//In Origin the legend can be placed outside of the plot which is not possible in LabPlot.
				//To achieve this we'll need to increase padding area in the plot and to place the legend outside of the plot area.
				CartesianPlotLegend::PositionWrapper position;
				position.horizontalPosition = WorksheetElement::HorizontalPosition::Right;
				position.verticalPosition = WorksheetElement::VerticalPosition::Bottom;
				legend->setPosition(position);

				//rotation
				legend->setRotationAngle(originLegend.rotation);

				//border line
				if (originLegend.borderType == Origin::BorderType::None)
					legend->setBorderPen(QPen(Qt::NoPen));
				else
					legend->setBorderPen(QPen(Qt::SolidLine));

				//background color, determine it with the help of the border type
				if (originLegend.borderType == Origin::BorderType::DarkMarble)
					legend->setBackgroundFirstColor(Qt::darkGray);
				else if (originLegend.borderType == Origin::BorderType::BlackOut)
					legend->setBackgroundFirstColor(Qt::black);
				else
					legend->setBackgroundFirstColor(Qt::white);

				plot->addLegend(legend);
			}

			//texts
			for (const auto& s : layer.texts) {
				DEBUG("EXTRA TEXT = " << s.text.c_str());
				TextLabel* label = new TextLabel("text label");
				label->setText(parseOriginText(QString::fromLatin1(s.text.c_str())));
				plot->addChild(label);
				label->setParentGraphicsItem(plot->graphicsItem());

				//position
				//determine the relative position inside of the layer rect
				const float horRatio = (float)(s.clientRect.left-layer.clientRect.left)/(layer.clientRect.right-layer.clientRect.left);
				const float vertRatio = (float)(s.clientRect.top-layer.clientRect.top)/(layer.clientRect.bottom-layer.clientRect.top);
				textLabelPositions[label] = QSizeF(horRatio, vertRatio);
				DEBUG("horizontal/vertical ratio = " << horRatio << ", " << vertRatio);

				//rotation
				label->setRotationAngle(s.rotation);

				//TODO:
// 				Color color;
// 				unsigned short fontSize;
// 				int tab;
// 				BorderType borderType;
// 				Attach attach;
			}

			//curves
			int curveIndex = 1;
			for (const auto& originCurve : layer.curves) {

				QString data(originCurve.dataName.c_str());
				switch (data[0].toLatin1()) {
				case 'T':
				case 'E': {
					if (originCurve.type == Origin::GraphCurve::Line || originCurve.type == Origin::GraphCurve::Scatter || originCurve.type == Origin::GraphCurve::LineSymbol
						|| originCurve.type == Origin::GraphCurve::ErrorBar || originCurve.type == Origin::GraphCurve::XErrorBar) {

						// parse and use legend text
						// find substring between %c{curveIndex} and %c{curveIndex+1}
						int pos1 = legendText.indexOf(QString("\\c{%1}").arg(curveIndex)) + 5;
						int pos2 = legendText.indexOf(QString("\\c{%1}").arg(curveIndex+1));
						QString curveText = legendText.mid(pos1, pos2 - pos1);
						// replace %(1), %(2), etc. with curve name
						curveText.replace(QString("%(%1)").arg(curveIndex), QString::fromLatin1(originCurve.yColumnName.c_str()));
						curveText = curveText.trimmed();
						DEBUG(" curve " << curveIndex << " text = " << STDSTRING(curveText));

						//XYCurve* xyCurve = new XYCurve(i18n("Curve%1", QString::number(curveIndex)));
						//TODO: curve (legend) does not support HTML text yet.
						//XYCurve* xyCurve = new XYCurve(curveText);
						XYCurve* curve = new XYCurve(QString::fromLatin1(originCurve.yColumnName.c_str()));
						const QString& tableName = data.right(data.length() - 2);
						curve->setXColumnPath(tableName + '/' + originCurve.xColumnName.c_str());
						curve->setYColumnPath(tableName + '/' + originCurve.yColumnName.c_str());

						curve->suppressRetransform(true);
						if (!preview)
							loadCurve(originCurve, curve);
						plot->addChildFast(curve);
						curve->suppressRetransform(false);
					} else if (originCurve.type == Origin::GraphCurve::Column) {
						//vertical bars

					} else if (originCurve.type == Origin::GraphCurve::Bar) {
						//horizontal bars

					} else if (originCurve.type == Origin::GraphCurve::Histogram) {

					}
				}
				break;
				case 'F': {
					Origin::Function function;
					const vector<Origin::Function>::difference_type funcIndex = m_originFile->functionIndex(data.right(data.length()-2).toStdString().c_str());
					if (funcIndex < 0) {
						++curveIndex;
						continue;
					}

					function = m_originFile->function(funcIndex);

					XYEquationCurve* xyEqCurve = new XYEquationCurve(function.name.c_str());
					XYEquationCurve::EquationData eqData;

					eqData.count = function.totalPoints;
					eqData.expression1 = QString(function.formula.c_str());

					if (function.type == Origin::Function::Polar) {
						eqData.type = XYEquationCurve::EquationType::Polar;

						//replace 'x' by 'phi'
						eqData.expression1 = eqData.expression1.replace('x', "phi");

						//convert from degrees to radians
						eqData.min = QString::number(function.begin/180) + QLatin1String("*pi");
						eqData.max = QString::number(function.end/180) + QLatin1String("*pi");
					} else {
						eqData.expression1 = QString(function.formula.c_str());
						eqData.min = QString::number(function.begin);
						eqData.max = QString::number(function.end);
					}

					xyEqCurve->suppressRetransform(true);
					xyEqCurve->setEquationData(eqData);
					if (!preview)
						loadCurve(originCurve, xyEqCurve);
					plot->addChildFast(xyEqCurve);
					xyEqCurve->suppressRetransform(false);
				}
				}

				++curveIndex;
			}
		} else {
			//no support for 3D plots yet
			//TODO: add an "UnsupportedAspect" here
		}

		++index;
	}

	if (!preview) {
		worksheet->updateLayout();

		//worksheet and plots got their sizes,
		//-> position all text labels inside the plots correctly by converting
		//the relative positions determined above to the absolute values
		QHash<TextLabel*, QSizeF>::const_iterator it = textLabelPositions.constBegin();
		while (it != textLabelPositions.constEnd()) {
			TextLabel* label = it.key();
			const QSizeF& ratios = it.value();
			const CartesianPlot* plot = static_cast<const CartesianPlot*>(label->parentAspect());

			TextLabel::PositionWrapper position = label->position();
			position.point.setX(plot->dataRect().width()*(ratios.width()-0.5));
			position.point.setY(plot->dataRect().height()*(ratios.height()-0.5));
			label->setPosition(position);

			++it;
		}
	}

	return true;
}

/*
 * sets the axis properties (format and ticks) as defined in \c originAxis in \c axis,
 * \c index being 0 or 1 for "top" and "bottom" or "left" and "right" for horizontal or vertical axes, respectively.
 */
void OriginProjectParser::loadAxis(const Origin::GraphAxis& originAxis, Axis* axis, int index, const QString& axisTitle) const {
// 	int axisPosition;
//		possible values:
//			0: Axis is at default position
//			1: Axis is at (axisPositionValue)% from standard position
//			2: Axis is at (axisPositionValue) position of orthogonal axis
// 		double axisPositionValue;

// 		bool zeroLine;
// 		bool oppositeLine;

	//ranges
	axis->setRange(originAxis.min, originAxis.max);

	//ticks
	axis->setMajorTicksType(Axis::TicksType::Spacing);
	axis->setMajorTicksSpacing(originAxis.step);
	axis->setMinorTicksType(Axis::TicksType::TotalNumber);
	axis->setMinorTicksNumber(originAxis.minorTicks);

	//scale
	switch (originAxis.scale) {
	case Origin::GraphAxis::Linear:
		axis->setScale(Axis::Scale::Linear);
		break;
	case Origin::GraphAxis::Log10:
		axis->setScale(Axis::Scale::Log10);
		break;
	case Origin::GraphAxis::Ln:
		axis->setScale(Axis::Scale::Ln);
		break;
	case Origin::GraphAxis::Log2:
		axis->setScale(Axis::Scale::Log2);
		break;
	case Origin::GraphAxis::Probability:
	case Origin::GraphAxis::Probit:
	case Origin::GraphAxis::Reciprocal:
	case Origin::GraphAxis::OffsetReciprocal:
	case Origin::GraphAxis::Logit:
		//TODO:
		axis->setScale(Axis::Scale::Linear);
		break;
	}

	//major grid
	const Origin::GraphGrid& majorGrid = originAxis.majorGrid;
	QPen gridPen = axis->majorGridPen();
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
	gridPen.setStyle(penStyle);

	Origin::Color gridColor;
	gridColor.type = Origin::Color::ColorType::Regular;
	gridColor.regular = majorGrid.color;
	gridPen.setColor(OriginProjectParser::color(gridColor));
	gridPen.setWidthF(Worksheet::convertToSceneUnits(majorGrid.width, Worksheet::Unit::Point));
	axis->setMajorGridPen(gridPen);

	//minor grid
	const Origin::GraphGrid& minorGrid = originAxis.minorGrid;
	gridPen = axis->minorGridPen();
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
	gridPen.setStyle(penStyle);

	gridColor.regular = minorGrid.color;
	gridPen.setColor(OriginProjectParser::color(gridColor));
	gridPen.setWidthF(Worksheet::convertToSceneUnits(minorGrid.width, Worksheet::Unit::Point));
	axis->setMinorGridPen(gridPen);

	//process Origin::GraphAxisFormat
	const Origin::GraphAxisFormat& axisFormat = originAxis.formatAxis[index];

	QPen pen;
	Origin::Color color;
	color.type = Origin::Color::ColorType::Regular;
	color.regular = axisFormat.color;
	pen.setColor(OriginProjectParser::color(color));
	pen.setWidthF(Worksheet::convertToSceneUnits(axisFormat.thickness, Worksheet::Unit::Point));
	axis->setLinePen(pen);

	axis->setMajorTicksLength( Worksheet::convertToSceneUnits(axisFormat.majorTickLength, Worksheet::Unit::Point) );
	axis->setMajorTicksDirection( (Axis::TicksFlags) axisFormat.majorTicksType);
	axis->setMajorTicksPen(pen);
	axis->setMinorTicksLength( axis->majorTicksLength()/2); // minorTicksLength is half of majorTicksLength
	axis->setMinorTicksDirection( (Axis::TicksFlags) axisFormat.minorTicksType);
	axis->setMinorTicksPen(pen);

	QString titleText = parseOriginText(QString::fromLatin1(axisFormat.label.text.c_str()));
	DEBUG("	axis title text = " << STDSTRING(titleText));
	//TODO: parseOriginText() returns html formatted string. What is axisFormat.color used for?
	//TODO: use axisFormat.fontSize to override the global font size for the hmtl string?
	//TODO: convert special character here too
	DEBUG("	curve name = " << STDSTRING(axisTitle));
	titleText.replace("%(?X)", axisTitle);
	titleText.replace("%(?Y)", axisTitle);
	DEBUG(" axis title = " << STDSTRING(titleText));
	axis->title()->setText(titleText);
	axis->title()->setRotationAngle(axisFormat.label.rotation);

	axis->setLabelsPrefix(axisFormat.prefix.c_str());
	axis->setLabelsSuffix(axisFormat.suffix.c_str());
	//TODO: handle string factor member in GraphAxisFormat

	//process Origin::GraphAxisTick
	const Origin::GraphAxisTick& tickAxis = originAxis.tickAxis[index];
	if (tickAxis.showMajorLabels) {
		color.type = Origin::Color::ColorType::Regular;
		color.regular = tickAxis.color;
		axis->setLabelsColor(OriginProjectParser::color(color));
		//TODO: how to set labels position (top vs. bottom)?
	} else {
		axis->setLabelsPosition(Axis::LabelsPosition::NoLabels);
	}

	//TODO: handle ValueType valueType member in GraphAxisTick
	//TODO: handle int valueTypeSpecification in GraphAxisTick

	//precision
	if (tickAxis.decimalPlaces == -1)
		axis->setLabelsAutoPrecision(true);
	else {
		axis->setLabelsPrecision(tickAxis.decimalPlaces);
		axis->setLabelsAutoPrecision(false);
	}

	QFont font;
	//TODO: font family?
	font.setPixelSize( Worksheet::convertToSceneUnits(tickAxis.fontSize, Worksheet::Unit::Point) );
	font.setBold(tickAxis.fontBold);
	axis->setLabelsFont(font);
	//TODO: handle string dataName member in GraphAxisTick
	//TODO: handle string columnName member in GraphAxisTick
	axis->setLabelsRotationAngle(tickAxis.rotation);
}

void OriginProjectParser::loadCurve(const Origin::GraphCurve& originCurve, XYCurve* curve) const {
	//line properties
	QPen pen = curve->linePen();
	Qt::PenStyle penStyle(Qt::NoPen);
	if (originCurve.type == Origin::GraphCurve::Line || originCurve.type == Origin::GraphCurve::LineSymbol) {
		switch (originCurve.lineConnect) {
		case Origin::GraphCurve::NoLine:
			curve->setLineType(XYCurve::LineType::NoLine);
			break;
		case Origin::GraphCurve::Straight:
			curve->setLineType(XYCurve::LineType::Line);
			break;
		case Origin::GraphCurve::TwoPointSegment:
			curve->setLineType(XYCurve::LineType::Segments2);
			break;
		case Origin::GraphCurve::ThreePointSegment:
			curve->setLineType(XYCurve::LineType::Segments3);
			break;
		case Origin::GraphCurve::BSpline:
		case Origin::GraphCurve::Bezier:
		case Origin::GraphCurve::Spline:
			curve->setLineType(XYCurve::LineType::SplineCubicNatural);
			break;
		case Origin::GraphCurve::StepHorizontal:
			curve->setLineType(XYCurve::LineType::StartHorizontal);
			break;
		case Origin::GraphCurve::StepVertical:
			curve->setLineType(XYCurve::LineType::StartVertical);
			break;
		case Origin::GraphCurve::StepHCenter:
			curve->setLineType(XYCurve::LineType::MidpointHorizontal);
			break;
		case Origin::GraphCurve::StepVCenter:
			curve->setLineType(XYCurve::LineType::MidpointVertical);
			break;
		}

		switch (originCurve.lineStyle) {
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

		pen.setStyle(penStyle);
		pen.setWidthF( Worksheet::convertToSceneUnits(originCurve.lineWidth, Worksheet::Unit::Point) );
		pen.setColor(color(originCurve.lineColor));
		curve->setLineOpacity(1 - originCurve.lineTransparency/255);

		//TODO: handle unsigned char boxWidth of Origin::GraphCurve
	}
	pen.setStyle(penStyle);
	curve->setLinePen(pen);


	//symbol properties
	if (originCurve.type == Origin::GraphCurve::Scatter || originCurve.type == Origin::GraphCurve::LineSymbol) {
		//try to map the different symbols, mapping is not exact
		curve->setSymbolsRotationAngle(0);
		switch (originCurve.symbolShape) {
		case 0: //NoSymbol
			curve->setSymbolsStyle(Symbol::Style::NoSymbols);
			break;
		case 1: //Rect
			curve->setSymbolsStyle(Symbol::Style::Square);
			break;
		case 2: //Ellipse
		case 20://Sphere
			curve->setSymbolsStyle(Symbol::Style::Circle);
			break;
		case 3: //UTriangle
			curve->setSymbolsStyle(Symbol::Style::EquilateralTriangle);
			break;
		case 4: //DTriangle
			curve->setSymbolsStyle(Symbol::Style::EquilateralTriangle);
			break;
		case 5: //Diamond
			curve->setSymbolsStyle(Symbol::Style::Diamond);
			break;
		case 6: //Cross +
			curve->setSymbolsStyle(Symbol::Style::Cross);
			break;
		case 7: //Cross x
			curve->setSymbolsStyle(Symbol::Style::Cross);
			break;
		case 8: //Snow
			curve->setSymbolsStyle(Symbol::Style::Star4);
			break;
		case 9: //Horizontal -
			curve->setSymbolsStyle(Symbol::Style::Line);
			curve->setSymbolsRotationAngle(90);
			break;
		case 10: //Vertical |
			curve->setSymbolsStyle(Symbol::Style::Line);
			break;
		case 15: //LTriangle
			curve->setSymbolsStyle(Symbol::Style::EquilateralTriangle);
			break;
		case 16: //RTriangle
			curve->setSymbolsStyle(Symbol::Style::EquilateralTriangle);
			break;
		case 17: //Hexagon
		case 19: //Pentagon
			curve->setSymbolsStyle(Symbol::Style::Square);
			break;
		case 18: //Star
			curve->setSymbolsStyle(Symbol::Style::Star5);
			break;
		default:
			curve->setSymbolsStyle(Symbol::Style::NoSymbols);
		}

		//symbol size
		curve->setSymbolsSize(Worksheet::convertToSceneUnits(originCurve.symbolSize, Worksheet::Unit::Point));

		//symbol fill color
		QBrush brush = curve->symbolsBrush();
		if (originCurve.symbolFillColor.type == Origin::Color::ColorType::Automatic) {
			//"automatic" color -> the color of the line, if available, has to be used, black otherwise
			if (curve->lineType() != XYCurve::LineType::NoLine)
				brush.setColor(curve->linePen().color());
			else
				brush.setColor(Qt::black);
		} else
			brush.setColor(color(originCurve.symbolFillColor));

		curve->setSymbolsBrush(brush);

		//symbol border/edge color and width
		QPen pen = curve->symbolsPen();
		if (originCurve.symbolColor.type == Origin::Color::ColorType::Automatic) {
			//"automatic" color -> the color of the line, if available, has to be used, black otherwise
			if (curve->lineType() != XYCurve::LineType::NoLine)
				pen.setColor(curve->linePen().color());
			else
				pen.setColor(Qt::black);
		} else
			pen.setColor(color(originCurve.symbolColor));

		//border width (edge thickness in Origin) is given by percentage of the symbol radius
		pen.setWidthF(originCurve.symbolThickness/100.*curve->symbolsSize()/2.);

		curve->setSymbolsPen(pen);

		//handle unsigned char pointOffset member
		//handle bool connectSymbols member
	} else {
		curve->setSymbolsStyle(Symbol::Style::NoSymbols);
	}

	//filling properties
	if (originCurve.fillArea) {
		//TODO: handle unsigned char fillAreaType;
		//with 'fillAreaType'=0x10 the area between the curve and the x-axis is filled
		//with 'fillAreaType'=0x14 the area included inside the curve is filled. First and last curve points are joined by a line to close the otherwise open area.
		//with 'fillAreaType'=0x12 the area excluded outside the curve is filled. The inverse of fillAreaType=0x14 is filled.
		//At the moment we only support the first type, so set it to XYCurve::FillingBelow
		curve->setFillingPosition(XYCurve::FillingPosition::Below);

		if (originCurve.fillAreaPattern == 0) {
			curve->setFillingType(PlotArea::BackgroundType::Color);
		} else {
			curve->setFillingType(PlotArea::BackgroundType::Pattern);

			//map different patterns in originCurve.fillAreaPattern (has the values of Origin::FillPattern) to Qt::BrushStyle;
			switch (originCurve.fillAreaPattern) {
			case 0:
				curve->setFillingBrushStyle(Qt::NoBrush);
				break;
			case 1:
			case 2:
			case 3:
				curve->setFillingBrushStyle(Qt::BDiagPattern);
				break;
			case 4:
			case 5:
			case 6:
				curve->setFillingBrushStyle(Qt::FDiagPattern);
				break;
			case 7:
			case 8:
			case 9:
				curve->setFillingBrushStyle(Qt::DiagCrossPattern);
				break;
			case 10:
			case 11:
			case 12:
				curve->setFillingBrushStyle(Qt::HorPattern);
				break;
			case 13:
			case 14:
			case 15:
				curve->setFillingBrushStyle(Qt::VerPattern);
				break;
			case 16:
			case 17:
			case 18:
				curve->setFillingBrushStyle(Qt::CrossPattern);
				break;
			}
		}

		curve->setFillingFirstColor(color(originCurve.fillAreaColor));
		curve->setFillingOpacity(1 - originCurve.fillAreaTransparency/255);

		//Color fillAreaPatternColor - color for the pattern lines, not supported
		//double fillAreaPatternWidth - width of the pattern lines, not supported
		//bool fillAreaWithLineTransparency - transparency of the pattern lines independent of the area transparency, not supported

		//TODO:
		//unsigned char fillAreaPatternBorderStyle;
		//Color fillAreaPatternBorderColor;
		//double fillAreaPatternBorderWidth;
		//The Border properties are used only in "Column/Bar" (histogram) plots. Those properties are:
		//fillAreaPatternBorderStyle   for the line style (use enum Origin::LineStyle here)
		//fillAreaPatternBorderColor   for the line color
		//fillAreaPatternBorderWidth   for the line width
	} else
		curve->setFillingPosition(XYCurve::FillingPosition::NoFilling);
}

bool OriginProjectParser::loadNote(Note* note, bool preview) {
	DEBUG("OriginProjectParser::loadNote()");
	//load note data
	const Origin::Note& originNote = m_originFile->note(findNoteByName(note->name()));

	if (preview)
		return true;

	note->setComment(originNote.label.c_str());
	note->setNote(QString::fromLatin1(originNote.text.c_str()));

	return true;
}

//##############################################################################
//########################### Helper functions  ################################
//##############################################################################
QDateTime OriginProjectParser::creationTime(tree<Origin::ProjectNode>::iterator it) const {
	//this logic seems to be correct only for the first node (project node). For other nodes the current time is returned.
	char time_str[21];
	strftime(time_str, sizeof(time_str), "%F %T", gmtime(&(*it).creationDate));
	return QDateTime::fromString(QString(time_str), Qt::ISODate);
}

QString OriginProjectParser::parseOriginText(const QString &str) const {
	DEBUG("parseOriginText()");
	QStringList lines = str.split('\n');
	QString text;
	for (int i = 0; i < lines.size(); ++i) {
		if (i > 0)
			text.append("<br>");
		text.append(parseOriginTags(lines[i]));
	}

	DEBUG(" PARSED TEXT = " << STDSTRING(text));

	return text;
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

	return QColor(Qt::white);
}

PlotArea::BackgroundColorStyle OriginProjectParser::backgroundColorStyle(Origin::ColorGradientDirection colorGradient) const {
	switch (colorGradient) {
		case Origin::ColorGradientDirection::NoGradient:
			return PlotArea::BackgroundColorStyle::SingleColor;
		case Origin::ColorGradientDirection::TopLeft:
			return PlotArea::BackgroundColorStyle::TopLeftDiagonalLinearGradient;
		case Origin::ColorGradientDirection::Left:
			return PlotArea::BackgroundColorStyle::HorizontalLinearGradient;
		case Origin::ColorGradientDirection::BottomLeft:
			return PlotArea::BackgroundColorStyle::BottomLeftDiagonalLinearGradient;
		case Origin::ColorGradientDirection::Top:
			return PlotArea::BackgroundColorStyle::VerticalLinearGradient;
		case Origin::ColorGradientDirection::Center:
			return PlotArea::BackgroundColorStyle::RadialGradient;
		case Origin::ColorGradientDirection::Bottom:
			return PlotArea::BackgroundColorStyle::VerticalLinearGradient;
		case Origin::ColorGradientDirection::TopRight:
			return PlotArea::BackgroundColorStyle::BottomLeftDiagonalLinearGradient;
		case Origin::ColorGradientDirection::Right:
			return PlotArea::BackgroundColorStyle::HorizontalLinearGradient;
		case Origin::ColorGradientDirection::BottomRight:
			return PlotArea::BackgroundColorStyle::TopLeftDiagonalLinearGradient;
	}

	return PlotArea::BackgroundColorStyle::SingleColor;
}

QString strreverse(const QString &str) {	//QString reversing
	QByteArray ba = str.toLocal8Bit();
	std::reverse(ba.begin(), ba.end());

	return QString(ba);
}

QList<QPair<QString, QString>> OriginProjectParser::charReplacementList() const {
	QList<QPair<QString, QString>> replacements;

	// TODO: probably missed some. Is there any generic method?
	replacements << qMakePair(QString("ä"), QString("&auml;"));
	replacements << qMakePair(QString("ö"), QString("&ouml;"));
	replacements << qMakePair(QString("ü"), QString("&uuml;"));
	replacements << qMakePair(QString("Ä"), QString("&Auml;"));
	replacements << qMakePair(QString("Ö"), QString("&Ouml;"));
	replacements << qMakePair(QString("Ü"), QString("&Uuml;"));
	replacements << qMakePair(QString("ß"), QString("&szlig;"));
	replacements << qMakePair(QString("€"), QString("&euro;"));
	replacements << qMakePair(QString("£"), QString("&pound;"));
	replacements << qMakePair(QString("¥"), QString("&yen;"));
	replacements << qMakePair(QString("¤"), QString("&curren;")); // krazy:exclude=spelling
	replacements << qMakePair(QString("¦"), QString("&brvbar;"));
	replacements << qMakePair(QString("§"), QString("&sect;"));
	replacements << qMakePair(QString("µ"), QString("&micro;"));
	replacements << qMakePair(QString("¹"), QString("&sup1;"));
	replacements << qMakePair(QString("²"), QString("&sup2;"));
	replacements << qMakePair(QString("³"), QString("&sup3;"));
	replacements << qMakePair(QString("¶"), QString("&para;"));
	replacements << qMakePair(QString("ø"), QString("&oslash;"));
	replacements << qMakePair(QString("æ"), QString("&aelig;"));
	replacements << qMakePair(QString("ð"), QString("&eth;"));
	replacements << qMakePair(QString("ħ"), QString("&hbar;"));
	replacements << qMakePair(QString("ĸ"), QString("&kappa;"));
	replacements << qMakePair(QString("¢"), QString("&cent;"));
	replacements << qMakePair(QString("¼"), QString("&frac14;"));
	replacements << qMakePair(QString("½"), QString("&frac12;"));
	replacements << qMakePair(QString("¾"), QString("&frac34;"));
	replacements << qMakePair(QString("¬"), QString("&not;"));
	replacements << qMakePair(QString("©"), QString("&copy;"));
	replacements << qMakePair(QString("®"), QString("&reg;"));
	replacements << qMakePair(QString("ª"), QString("&ordf;"));
	replacements << qMakePair(QString("º"), QString("&ordm;"));
	replacements << qMakePair(QString("±"), QString("&plusmn;"));
	replacements << qMakePair(QString("¿"), QString("&iquest;"));
	replacements << qMakePair(QString("×"), QString("&times;"));
	replacements << qMakePair(QString("°"), QString("&deg;"));
	replacements << qMakePair(QString("«"), QString("&laquo;"));
	replacements << qMakePair(QString("»"), QString("&raquo;"));
	replacements << qMakePair(QString("¯"), QString("&macr;"));
	replacements << qMakePair(QString("¸"), QString("&cedil;"));
	replacements << qMakePair(QString("À"), QString("&Agrave;"));
	replacements << qMakePair(QString("Á"), QString("&Aacute;"));
	replacements << qMakePair(QString("Â"), QString("&Acirc;"));
	replacements << qMakePair(QString("Ã"), QString("&Atilde;"));
	replacements << qMakePair(QString("Å"), QString("&Aring;"));
	replacements << qMakePair(QString("Æ"), QString("&AElig;"));
	replacements << qMakePair(QString("Ç"), QString("&Ccedil;"));
	replacements << qMakePair(QString("È"), QString("&Egrave;"));
	replacements << qMakePair(QString("É"), QString("&Eacute;"));
	replacements << qMakePair(QString("Ê"), QString("&Ecirc;"));
	replacements << qMakePair(QString("Ë"), QString("&Euml;"));
	replacements << qMakePair(QString("Ì"), QString("&Igrave;"));
	replacements << qMakePair(QString("Í"), QString("&Iacute;"));
	replacements << qMakePair(QString("Î"), QString("&Icirc;"));
	replacements << qMakePair(QString("Ï"), QString("&Iuml;"));
	replacements << qMakePair(QString("Ð"), QString("&ETH;"));
	replacements << qMakePair(QString("Ñ"), QString("&Ntilde;"));
	replacements << qMakePair(QString("Ò"), QString("&Ograve;"));
	replacements << qMakePair(QString("Ó"), QString("&Oacute;"));
	replacements << qMakePair(QString("Ô"), QString("&Ocirc;"));
	replacements << qMakePair(QString("Õ"), QString("&Otilde;"));
	replacements << qMakePair(QString("Ù"), QString("&Ugrave;"));
	replacements << qMakePair(QString("Ú"), QString("&Uacute;"));
	replacements << qMakePair(QString("Û"), QString("&Ucirc;"));
	replacements << qMakePair(QString("Ý"), QString("&Yacute;"));
	replacements << qMakePair(QString("Þ"), QString("&THORN;"));
	replacements << qMakePair(QString("à"), QString("&agrave;"));
	replacements << qMakePair(QString("á"), QString("&aacute;"));
	replacements << qMakePair(QString("â"), QString("&acirc;"));
	replacements << qMakePair(QString("ã"), QString("&atilde;"));
	replacements << qMakePair(QString("å"), QString("&aring;"));
	replacements << qMakePair(QString("ç"), QString("&ccedil;"));
	replacements << qMakePair(QString("è"), QString("&egrave;"));
	replacements << qMakePair(QString("é"), QString("&eacute;"));
	replacements << qMakePair(QString("ê"), QString("&ecirc;"));
	replacements << qMakePair(QString("ë"), QString("&euml;"));
	replacements << qMakePair(QString("ì"), QString("&igrave;"));
	replacements << qMakePair(QString("í"), QString("&iacute;"));
	replacements << qMakePair(QString("î"), QString("&icirc;"));
	replacements << qMakePair(QString("ï"), QString("&iuml;"));
	replacements << qMakePair(QString("ñ"), QString("&ntilde;"));
	replacements << qMakePair(QString("ò"), QString("&ograve;"));
	replacements << qMakePair(QString("ó"), QString("&oacute;"));
	replacements << qMakePair(QString("ô"), QString("&ocirc;"));
	replacements << qMakePair(QString("õ"), QString("&otilde;"));
	replacements << qMakePair(QString("÷"), QString("&divide;"));
	replacements << qMakePair(QString("ù"), QString("&ugrave;"));
	replacements << qMakePair(QString("ú"), QString("&uacute;"));
	replacements << qMakePair(QString("û"), QString("&ucirc;"));
	replacements << qMakePair(QString("ý"), QString("&yacute;"));
	replacements << qMakePair(QString("þ"), QString("&thorn;"));
	replacements << qMakePair(QString("ÿ"), QString("&yuml;"));
	replacements << qMakePair(QString("Œ"), QString("&#338;"));
	replacements << qMakePair(QString("œ"), QString("&#339;"));
	replacements << qMakePair(QString("Š"), QString("&#352;"));
	replacements << qMakePair(QString("š"), QString("&#353;"));
	replacements << qMakePair(QString("Ÿ"), QString("&#376;"));
	replacements << qMakePair(QString("†"), QString("&#8224;"));
	replacements << qMakePair(QString("‡"), QString("&#8225;"));
	replacements << qMakePair(QString("…"), QString("&#8230;"));
	replacements << qMakePair(QString("‰"), QString("&#8240;"));
	replacements << qMakePair(QString("™"), QString("&#8482;"));

	return replacements;
}

QString OriginProjectParser::replaceSpecialChars(const QString& text) const {
	QString t = text;
	for (const auto& r : charReplacementList())
		t.replace(r.first, r.second);
	return t;
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
	DEBUG("parseOriginTags()");
	DEBUG("	string: " << STDSTRING(str));
	QDEBUG("	UTF8 string: " << str.toUtf8());
	QString line = str;

	//replace %(...) tags
// 	QRegExp rxcol("\\%\\(\\d+\\)");

	// replace \l(x) (plot legend tags) with \\c{x}, where x is a digit
	line.replace(QRegularExpression(QStringLiteral("\\\\\\s*l\\s*\\(\\s*(\\d+)\\s*\\)")), QStringLiteral("\\c{\\1}"));

	// replace umlauts etc.
	line = replaceSpecialChars(line);

	// replace tabs	(not really supported)
	line.replace('\t', "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;");

	// In PCRE2 (which is what QRegularExpression uses) variable-length lookbehind is supposed to be
	// exprimental in Perl 5.30; which means it doesn't work at the moment, i.e. using a variable-length
	// negative lookbehind isn't valid syntax from QRegularExpression POV.
	// Ultimately we have to reverse the string and use a negative _lookahead_ instead.
	// The goal is to temporatily replace '(' and ')' that don't denote tags; this is so that we
	// can handle parenthesis that are inside the tag, e.g. '\b(bold (cf))', we want the '(cf)' part
	// to remain as is.
	const QRegularExpression nonTagsRe(R"(\)([^)(]*)\((?!\s*([buigs\+\-]|\d{1,3}\s*[pc]|[\w ]+\s*:\s*f)\s*\\))");
	QString linerev = strreverse(line);
	const QString lBracket = strreverse("&lbracket;");
	const QString rBracket = strreverse("&rbracket;");
	linerev.replace(nonTagsRe, rBracket + QStringLiteral("\\1") + lBracket);

	// change the line back to normal
	line = strreverse(linerev);

	//replace \-(...), \+(...), \b(...), \i(...), \u(...), \s(....), \g(...), \f:font(...),
	// \c'number'(...), \p'size'(...) tags with equivalent supported HTML syntax
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
				rep = QStringLiteral("<font face=Symbol>%1</font>").arg(tagText);
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
				c.regular = colorIndex <= 23 ? static_cast<Origin::Color::RegularColor>(colorIndex)
											   : Origin::Color::RegularColor::Black;
				QColor color = OriginProjectParser::color(c);
				rep = QStringLiteral("<span style=\"color: %1\">%2</span>").arg(color.name(), tagText);
		}
		line.replace(rmatch.capturedStart(0), rmatch.capturedLength(0), rep);
	}

	// put non-tag '(' and ')' back in their places
	line.replace("&lbracket;", "(");
	line.replace("&rbracket;", ")");

	// special characters
	line.replace(QRegularExpression(QStringLiteral("\\\\\\((\\d+)\\)")), "&#\\1;");

	DEBUG("	result: " << STDSTRING(line));

	return line;
}
