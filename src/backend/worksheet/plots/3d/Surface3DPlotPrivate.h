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

#include "Base3DAreaPrivate.h"
#include "Surface3DPlot.h"
#include <backend/matrix/Matrix.h>
#include <backend/worksheet/WorksheetElementContainerPrivate.h>
#include <backend/worksheet/WorksheetElementPrivate.h>

class Surface3DPlot;
class WorksheetElementContainerPrivate;

class Surface3DPlotPrivate : public Base3DAreaPrivate {
public:
	explicit Surface3DPlotPrivate(Surface3DPlot* owner);
	Surface3DPlot* const q{nullptr};
	Surface3DPlot::DataSource sourceType{Surface3DPlot::DataSource::DataSource_Spreadsheet};
	Surface3DPlot::DrawMode drawMode{Surface3DPlot::DrawMode::DrawWireframeSurface};
	bool flatShading;
	bool smooth;
	QColor color;

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
	void updateFlatShading();
	void updateSmoothMesh();
};
#endif // SURFACE3DPLOTAREAPRIVATE_H
