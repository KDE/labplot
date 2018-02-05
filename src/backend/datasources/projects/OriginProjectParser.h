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

#include "backend/datasources/projects/ProjectParser.h"
#include <liborigin/OriginFile.h>

class Project;
class Workbook;
class Spreadsheet;
class Matrix;
class Worksheet;
class Note;

class OriginProjectParser : public ProjectParser {
	Q_OBJECT

public:
	OriginProjectParser();

	static bool isOriginProject(const QString& fileName);
	static QString supportedExtensions();

private:
	bool loadFolder(Folder*, const tree<Origin::ProjectNode>::iterator&, bool preview);
	bool loadWorkbook(Workbook*, bool preview);
	bool loadSpreadsheet(Spreadsheet*, bool preview, size_t sheetIndex = 0);
	bool loadMatrixWorkbook(Workbook*, bool preview);
	bool loadMatrix(Matrix*, bool preview, size_t sheetIndex = 0);
	bool loadWorksheet(Worksheet*,  bool preview);
	bool loadNote(Note*, bool preview);

	unsigned int findMatrixByName(QString name);
	unsigned int findExcelByName(QString name);
	QString parseOriginText(const QString &str) const;
	QString parseOriginTags(const QString &str) const;
	QDateTime creationTime(const tree<Origin::ProjectNode>::iterator&) const;
	QColor color(const Origin::Color::RegularColor&) const;

	OriginFile* m_originFile;
	size_t m_excelIndex;
	size_t m_matrixIndex;
	size_t m_graphIndex;
	size_t m_noteIndex;

protected:
	bool load(Project*, bool) override;
};

#endif // ORIGINPROJECTPARSER_H
