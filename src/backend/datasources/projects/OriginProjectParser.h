/***************************************************************************
    File                 : OriginProjectParser.h
    Project              : LabPlot
    Description          : parser for Origin projects
    --------------------------------------------------------------------
    Copyright            : (C) 2017 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2018 Stefan Gerlach (stefan.gerlach@uni.kn)

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
#ifndef ORIGINPROJECTPARSER_H
#define ORIGINPROJECTPARSER_H

#include "backend/worksheet/plots/PlotArea.h"
#include "backend/datasources/projects/ProjectParser.h"
#include <liborigin/OriginFile.h>

class Axis;
class Column;
class Project;
class Workbook;
class Spreadsheet;
class Matrix;
class Worksheet;
class Note;
class XYCurve;

class OriginProjectParser : public ProjectParser {
	Q_OBJECT

public:
	OriginProjectParser();

	static bool isOriginProject(const QString& fileName);
	static QString supportedExtensions();
	void setImportUnusedObjects(bool);
	bool hasUnusedObjects();

private:
	bool loadFolder(Folder*, tree<Origin::ProjectNode>::iterator, bool preview);
	bool loadWorkbook(Workbook*, bool preview);
	bool loadSpreadsheet(Spreadsheet*, bool preview, const QString& wbName = QString(), int sheetIndex = -1);
	void loadColumnNumericFormat(const Origin::SpreadColumn& originColumn, Column* column) const;
	bool loadMatrixWorkbook(Workbook*, bool preview);
	bool loadMatrix(Matrix*, bool preview, size_t sheetIndex = 0, const QString& mwbName = QString());
	bool loadWorksheet(Worksheet*,  bool preview);
	void loadAxis(const Origin::GraphAxis&, Axis*, int index, const QString& axisTitle = QString()) const;
	void loadCurve(const Origin::GraphCurve&, XYCurve*) const;
	bool loadNote(Note*, bool preview);
	void handleLooseWindows(Folder*, bool preview);

	unsigned int findSpreadByName(const QString&);
	unsigned int findMatrixByName(const QString&);
	unsigned int findExcelByName(const QString&);
	unsigned int findGraphByName(const QString&);
	unsigned int findNoteByName(const QString&);
	QString parseOriginText(const QString&) const;
	QString parseOriginTags(const QString&) const;
	QDateTime creationTime(tree<Origin::ProjectNode>::iterator) const;
	QColor color(Origin::Color) const;
	PlotArea::BackgroundColorStyle backgroundColorStyle(Origin::ColorGradientDirection) const;

	QList<QPair<QString, QString>> charReplacementList() const;
	QString replaceSpecialChars(QString text) const;

	OriginFile* m_originFile{nullptr};
	QStringList m_spreadNameList;
	QStringList m_excelNameList;
	QStringList m_matrixNameList;
	QStringList m_graphNameList;
	QStringList m_noteNameList;
	bool m_importUnusedObjects{false};

protected:
	bool load(Project*, bool) override;
};

#endif // ORIGINPROJECTPARSER_H
