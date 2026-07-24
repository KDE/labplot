/*
	File                 : OriginProjectParser.h
	Project              : LabPlot
	Description          : parser for Origin projects
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2017-2024 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2018-2024 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ORIGINPROJECTPARSER_H
#define ORIGINPROJECTPARSER_H

#include "backend/datasources/projects/ProjectParser.h"
#include "backend/worksheet/Background.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"
#ifdef HAVE_LIBORIGIN
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#endif
#include <OriginFile.h>
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
#endif

class Axis;
class Background;
class CartesianPlot;
class Column;
class Matrix;
class Note;
class Project;
class Spreadsheet;
class Symbol;
class TextLabel;
class XYCurve;
class Workbook;
class Worksheet;

class OriginProjectParser : public ProjectParser {
	Q_OBJECT

public:
	OriginProjectParser();
	~OriginProjectParser() override;

	static bool isOriginProject(const QString& fileName);
	static QString supportedExtensions();

	void checkContent(bool& hasUnusedObjects, bool& hasMultiLayers);
	void setImportUnusedObjects(bool);
	void setGraphLayerAsPlotArea(bool);

protected:
	bool load(Project*, bool preview) override;

private:
	bool loadFolder(Folder*, tree<Origin::ProjectNode>::iterator, bool preview);
	bool loadWorkbook(Workbook*, bool preview);
	bool loadSpreadsheet(Spreadsheet*, bool preview, const QString& wbName = QString(), int sheetIndex = -1);
	void loadColumnNumericFormat(const Origin::SpreadColumn&, Column*) const;
	bool loadMatrixWorkbook(Workbook*, bool preview);
	bool loadMatrix(Matrix*, bool preview, size_t sheetIndex = 0, const QString& mwbName = QString());

	bool loadWorksheet(Worksheet*, bool preview);
	void loadGraphLayer(const Origin::GraphLayer&, CartesianPlot*, int layerIndex, QHash<TextLabel*, QSizeF>& textLabelPositions, bool preview);
	void loadCurves(const Origin::GraphLayer&, CartesianPlot*, int layerIndex, bool preview);
	void loadAxes(const Origin::GraphLayer&, CartesianPlot*, int layerIndex, const QString& xColumnInfo, const QString& yColumnInfo);
	void loadAxis(const Origin::GraphAxis&, Axis*, int layerIndex, int index, const QString& columnInfo = QString()) const;
	void loadCurve(const Origin::GraphCurve&, XYCurve*) const;
	void loadBackground(const Origin::GraphCurve&, Background*) const;
	void loadSymbol(const Origin::GraphCurve&, Symbol*, const XYCurve* = nullptr) const;

	bool loadNote(Note*, bool preview);
	void handleLooseWindows(Folder*, bool preview);

	bool hasUnusedObjects();
	bool hasMultiLayerGraphs();
	void restorePointers(Project*);

	unsigned int findSpreadsheetByName(const QString&);
	unsigned int findColumnByName(const Origin::SpreadSheet&, const QString&);
	unsigned int findMatrixByName(const QString&);
	unsigned int findWorkbookByName(const QString&);
	unsigned int findWorksheetByName(const QString&);
	unsigned int findNoteByName(const QString&);

	Origin::SpreadSheet getSpreadsheetByName(QString&);
	void parseColumnInfo(const QString& info, QString& longName, QString& unit, QString& comments) const;
	QString parseOriginText(const QString&) const;
	QString parseOriginTags(const QString&) const;
	QDateTime creationTime(tree<Origin::ProjectNode>::iterator) const;

	// map between the different enums/datatypes
	RangeT::Scale scale(unsigned char) const;
	QColor color(Origin::Color) const;
	Background::ColorStyle backgroundColorStyle(Origin::ColorGradientDirection) const;
	Qt::PenStyle penStyle(unsigned char lineStyle) const;
	XYCurve::LineType lineType(unsigned char lineConnect) const;

	QList<QPair<QString, QString>> charReplacementList() const;
	QString replaceSpecialChars(const QString&) const;

	OriginFile* m_originFile{nullptr};
	QStringList m_spreadsheetNameList;
	QStringList m_workbookNameList;
	QStringList m_matrixNameList;
	QStringList m_worksheetNameList;
	QStringList m_noteNameList;
	QString m_legendText;
	bool m_importUnusedObjects{false};
	bool m_graphLayerAsPlotArea{true};
	double textScalingFactor{1.};
	double elementScalingFactor{1.};
	QSize graphSize;

	friend class ProjectImportTest;
};

#endif // ORIGINPROJECTPARSER_H
