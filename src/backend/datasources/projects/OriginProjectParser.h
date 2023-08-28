/*
	File                 : OriginProjectParser.h
	Project              : LabPlot
	Description          : parser for Origin projects
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2017 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2018-2021 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ORIGINPROJECTPARSER_H
#define ORIGINPROJECTPARSER_H

#include "backend/datasources/projects/ProjectParser.h"
#include "backend/worksheet/Background.h"
#include <OriginFile.h>

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

protected:
	bool load(Project*, bool) override;

private:
	bool loadFolder(Folder*, tree<Origin::ProjectNode>::iterator, bool preview);
	bool loadWorkbook(Workbook*, bool preview);
	bool loadSpreadsheet(Spreadsheet*, bool preview, const QString& wbName = QString(), int sheetIndex = -1);
	void loadColumnNumericFormat(const Origin::SpreadColumn& originColumn, Column* column) const;
	bool loadMatrixWorkbook(Workbook*, bool preview);
	bool loadMatrix(Matrix*, bool preview, size_t sheetIndex = 0, const QString& mwbName = QString());
	bool loadWorksheet(Worksheet*, bool preview);
	void loadAxis(const Origin::GraphAxis&, Axis*, int index, const QString& axisTitle = QString()) const;
	void loadCurve(const Origin::GraphCurve&, XYCurve*) const;
	bool loadNote(Note*, bool preview);
	void handleLooseWindows(Folder*, bool preview);

	unsigned int findSpreadsheetByName(const QString&);
	unsigned int findMatrixByName(const QString&);
	unsigned int findWorkbookByName(const QString&);
	unsigned int findWorksheetByName(const QString&);
	unsigned int findNoteByName(const QString&);
	QString parseOriginText(const QString&) const;
	QString parseOriginTags(const QString&) const;
	QDateTime creationTime(tree<Origin::ProjectNode>::iterator) const;
	QColor color(Origin::Color) const;
	Background::ColorStyle backgroundColorStyle(Origin::ColorGradientDirection) const;

	QList<QPair<QString, QString>> charReplacementList() const;
	QString replaceSpecialChars(const QString&) const;

	OriginFile* m_originFile{nullptr};
	QStringList m_spreadsheetNameList;
	QStringList m_workbookNameList;
	QStringList m_matrixNameList;
	QStringList m_worksheetNameList;
	QStringList m_noteNameList;
	bool m_importUnusedObjects{false};

	friend class ProjectImportTest;
};

#endif // ORIGINPROJECTPARSER_H
