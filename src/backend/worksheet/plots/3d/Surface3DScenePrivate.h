/*
	File                 : Surface3DScenePrivate.h
	Project              : LabPlot
	Description          : private members of 3D Surface Plot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024-2026 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2024 Kuntal Bar <barkuntal6@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SURFACE3DPLOTPRIVATE_H
#define SURFACE3DPLOTPRIVATE_H

#include "Base3DPlotPrivate.h"
#include "Surface3DScene.h"
#include <backend/matrix/Matrix.h>

class Surface3DScene;
class AbstractColumn;

class Surface3DScenePrivate : public Base3DPlotPrivate {
public:
	explicit Surface3DScenePrivate(Surface3DScene* owner);
	Surface3DScene* const q{nullptr};
	Surface3DScene::DataSource sourceType{Surface3DScene::DataSource::DataSource_Spreadsheet};
	Surface3DScene::DrawMode drawMode{Surface3DScene::DrawMode::DrawWireframeSurface};
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
#endif // SURFACE3DPLOTPRIVATE_H
