/*
	File                 : Surface3DPlotAreaPrivate.h
	Project              : LabPlot
	Description          : Surface3DPlotAreaPrivate
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Kuntal Bar <barkuntal6@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef SURFACE3DPLOTAREAPRIVATE_H
#define SURFACE3DPLOTAREAPRIVATE_H

#include "Surface3DPlotArea.h"
#include <backend/matrix/Matrix.h>
#include <backend/worksheet/WorksheetElementContainerPrivate.h>
#include <backend/worksheet/WorksheetElementPrivate.h>

class Surface3DPlotArea;
class WorksheetElementContainerPrivate;

class Surface3DPlotAreaPrivate : public WorksheetElementContainerPrivate {
public:
	explicit Surface3DPlotAreaPrivate(Surface3DPlotArea* owner);
	Surface3DPlotArea* const q{nullptr};
	Surface3DPlotArea::DataSource sourceType{Surface3DPlotArea::DataSource::DataSource_Spreadsheet};
	Surface3DPlotArea::DrawMode drawMode{Surface3DPlotArea::DrawMode::DrawWireframeSurface};
	bool flatShading;
	Surface3DPlotArea::ShadowQuality shadowQuality;
	bool smooth;
	int xRotation;
	int yRotation;
	int zoomLevel;
	Surface3DPlotArea::Theme theme;

	QColor color;
	double opacity;

	// Spreadsheet properties
	const AbstractColumn* xColumn{nullptr};
	const AbstractColumn* yColumn{nullptr};
	const AbstractColumn* zColumn{nullptr};

	// Matrix properties
	const Matrix* matrix{nullptr};
	QString matrixPath;

	QString xColumnPath;
	QString yColumnPath;
	QString zColumnPath;

	void retransform() override;
	void recalcShapeAndBoundingRect() override;

	void generateData() const;
	// Data generation
	void generateDemoData() const;
	void generateMatrixData() const;
	void generateSpreadsheetData() const;

	// Export
	void saveSpreadsheetConfig(QXmlStreamWriter*) const;
	void saveMatrixConfig(QXmlStreamWriter*) const;

	// Import
	bool loadSpreadsheetConfig(XmlStreamReader*);
	bool loadMatrixConfig(XmlStreamReader*);
	void recalc();
	void updateDrawMode();
	void updateColor();
	void updateOpacity();
	void updateFlatShading();
	void updateShadowQuality();
	void updateSmoothMesh();
};
#endif // SURFACE3DPLOTAREAPRIVATE_H
