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

#include <Q3DSurface> // TODO: get rid of this include, use forward declaration only
#include <backend/worksheet/WorksheetElementContainer.h>

class AbstractColumn;
class Matrix;
class Surface3DPlotAreaPrivate;

class Q3DSurface;
class QSurface3DSeries;
class QSurfaceDataProxy;

class Surface3DPlotArea : public WorksheetElementContainer {
	Q_OBJECT
public:
	Surface3DPlotArea(const QString& name);
	~Surface3DPlotArea() override;
	void handleResize(double horizontalRatio, double verticalRatio, bool pageResize = false) override;
	void retransform() override;
	enum DataSource { DataSource_Spreadsheet = 0, DataSource_Matrix = 1, DataSource_Empty = 2 };
	enum DrawMode { DrawWireframe = 1, DrawSurface = 2, DrawWireframeSurface = 3 };
	enum Theme {
		Qt = 0,
		PrimaryColors = 1,
		StoneMoss = 2,
		ArmyBlue = 3,
		Retro = 4,
		Ebony = 5,
		Isabelle = 6,
	};

	enum ShadowQuality { None = 0, Low = 1, Medium = 2, High = 3, SoftLow = 4, SoftMedium = 5, SoftHigh = 6 };

	void setRect(const QRectF&) override;
	void setPrevRect(const QRectF&) override;

	BASIC_D_ACCESSOR_DECL(DataSource, dataSource, DataSource)
	BASIC_D_ACCESSOR_DECL(DrawMode, drawMode, DrawMode)
	BASIC_D_ACCESSOR_DECL(QColor, color, Color)
	BASIC_D_ACCESSOR_DECL(double, opacity, Opacity)
	BASIC_D_ACCESSOR_DECL(bool, flatShading, FlatShading)
	BASIC_D_ACCESSOR_DECL(ShadowQuality, shadowQuality, ShadowQuality)
	BASIC_D_ACCESSOR_DECL(bool, smooth, Smooth)
	BASIC_D_ACCESSOR_DECL(int, zoomLevel, ZoomLevel)
	BASIC_D_ACCESSOR_DECL(int, xRotation, XRotation)
	BASIC_D_ACCESSOR_DECL(int, yRotation, YRotation)
	BASIC_D_ACCESSOR_DECL(Theme, theme, Theme)

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

	Q3DSurface* m_surface;
	typedef Surface3DPlotAreaPrivate Private;

private:
	Q_DECLARE_PRIVATE(Surface3DPlotArea)
private Q_SLOTS:
	// Spreadsheet slots
	void xColumnAboutToBeRemoved(const AbstractAspect*);
	void yColumnAboutToBeRemoved(const AbstractAspect*);
	void zColumnAboutToBeRemoved(const AbstractAspect*);

	// Matrix slots
	void matrixAboutToBeRemoved(const AbstractAspect*);

Q_SIGNALS:
	void sourceTypeChanged(Surface3DPlotArea::DataSource);
	void drawModeChanged(Surface3DPlotArea::DrawMode);
	void colorChanged(QColor);
	void opacityChanged(double);
	void flatShadingChanged(bool);
	void lightningChanged(bool);
	void shadowQualityChanged(Surface3DPlotArea::ShadowQuality);
	void smoothChanged(bool);
	void matrixChanged(const Matrix*);
	void xColumnChanged(const AbstractColumn*);
	void yColumnChanged(const AbstractColumn*);
	void zColumnChanged(const AbstractColumn*);
	void zoomLevelChanged(int);
	void xRotationChanged(int);
	void yRotationChanged(int);
	void rectChanged(QRectF&);
	void themeChanged(Surface3DPlotArea::Theme);
};
#endif // SURFACE3DPLOTAREA_H
