/*
	File                 : Surface3DPlotArea.h
	Project              : LabPlot
	Description          : Surface3DPlotArea
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Kuntal Bar <barkuntal6@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SURFACE3DPLOTAREA_H
#define SURFACE3DPLOTAREA_H

#include "Plot3DArea.h"

#include <Q3DSurface> // TODO: get rid of this include, use forward declaration only
#include <backend/worksheet/WorksheetElementContainer.h>

class AbstractColumn;
class Matrix;
class Surface3DPlotPrivate;

class Q3DSurface;
class QSurface3DSeries;
class QSurfaceDataProxy;

class Surface3DPlot : public Plot3DArea {
	Q_OBJECT
public:
	Surface3DPlot(const QString& name);
	~Surface3DPlot() override;
	enum DataSource { DataSource_Spreadsheet = 0, DataSource_Matrix = 1, DataSource_Empty = 2 };
	enum DrawMode { DrawWireframe = 1, DrawSurface = 2, DrawWireframeSurface = 3 };

	BASIC_D_ACCESSOR_DECL(DataSource, dataSource, DataSource)
	BASIC_D_ACCESSOR_DECL(DrawMode, drawMode, DrawMode)
	BASIC_D_ACCESSOR_DECL(bool, flatShading, FlatShading)
	BASIC_D_ACCESSOR_DECL(bool, smooth, Smooth)
	BASIC_D_ACCESSOR_DECL(QColor, color, Color)

	// Matrix parameters
	POINTER_D_ACCESSOR_DECL(const Matrix, matrix, Matrix)
	const QString& matrixPath() const;

	// Spreadsheet parameters
	POINTER_D_ACCESSOR_DECL(const AbstractColumn, xColumn, XColumn)
	POINTER_D_ACCESSOR_DECL(const AbstractColumn, yColumn, YColumn)
	POINTER_D_ACCESSOR_DECL(const AbstractColumn, zColumn, ZColumn)

	const QString& xColumnPath() const;
	const QString& yColumnPath() const;
	const QString& zColumnPath() const;

	void show(bool visible);

	typedef Surface3DPlotPrivate Private;

private:
	Q_DECLARE_PRIVATE(Surface3DPlot)
private Q_SLOTS:
	// Spreadsheet slots
	void xColumnAboutToBeRemoved(const AbstractAspect*);
	void yColumnAboutToBeRemoved(const AbstractAspect*);
	void zColumnAboutToBeRemoved(const AbstractAspect*);

	// Matrix slots
	void matrixAboutToBeRemoved(const AbstractAspect*);

Q_SIGNALS:
	void sourceTypeChanged(Surface3DPlot::DataSource);
	void drawModeChanged(Surface3DPlot::DrawMode);
	void flatShadingChanged(bool);
	void smoothChanged(bool);
	void colorChanged(QColor);
	void matrixChanged(const Matrix*);
	void xColumnChanged(const AbstractColumn*);
	void yColumnChanged(const AbstractColumn*);
	void zColumnChanged(const AbstractColumn*);
};
#endif // SURFACE3DPLOTAREA_H
