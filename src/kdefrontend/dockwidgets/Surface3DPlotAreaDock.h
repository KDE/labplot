/*
	File                 : Surface3DPlotAreaDock.cpp
	Project              : LabPlot
	Description          : widget for Surface3DPlotArea properties
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Kuntal Bar <barkuntal6@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SURFACE3DPLOTAREADOCK_H
#define SURFACE3DPLOTAREADOCK_H

#include "BaseDock.h"
#include "backend/worksheet/plots/3d/Surface3DPlot.h"

#include "ui_surface3dplotareadock.h"

class Surface3DPlot;
class Matrix;
class AbstractColumn;
class AspectTreeModel;
class ColorMapSelector;
class TreeViewComboBox;

class Surface3DPlotAreaDock : public BaseDock {
	Q_OBJECT

public:
	explicit Surface3DPlotAreaDock(QWidget* parent);
	void setSurfaces(const QList<Surface3DPlot*>& surfaces);

private:
	void showTriangleInfo(bool pred);
	void showItem(QWidget* label, QWidget* comboBox, bool pred);

	void updateUiVisibility();

private Q_SLOTS:
	void retranslateUi();

	// SLOTs for changes triggered in Surface3DPlotAreaDock
	void dataSourceTypeChanged(int);
	void xColumnChanged(const QModelIndex&);
	void yColumnChanged(const QModelIndex&);
	void zColumnChanged(const QModelIndex&);
	void matrixChanged(const QModelIndex&);

	// Appearance properties
	void drawModeChanged(int);
	void shadowQualityChanged(int);
	void flatShadingChanged(bool);
	void zoomLevelChanged(int);
	void xRotationChanged(int);
	void yRotationChanged(int);
	void themeChanged(int);
	void smoothChanged(bool);
	void colorChanged(QColor);

	// SLOTs for changes triggered in Surface3DPlotArea
	void surfaceDrawModeChanged(Surface3DPlot::DrawMode mode);
	void surfaceThemeChanged(Plot3DArea::Theme theme);
	void surfaceFlatShadingChanged(bool);
	void surfaceShadowsQualityChanged(Plot3DArea::ShadowQuality quality);
	void surfaceSourceTypeChanged(Surface3DPlot::DataSource);
	void surfaceXColumnChanged(const AbstractColumn*);
	void surfaceYColumnChanged(const AbstractColumn*);
	void surfaceZColumnChanged(const AbstractColumn*);
	void surfaceMatrixChanged(const Matrix*);
	void surfaceXRotationChanged(int);
	void surfaceYRotationChanged(int);
	void surfaceSmoothChanged(bool);
	void surfaceZoomChanged(int);
	void surfaceColorChanged(QColor);

private:
	Ui::Surface3DPlotAreaDock ui;
	QList<Surface3DPlot*> m_surfaces;
	Surface3DPlot* m_surface{nullptr};

	void load();
	void loadConfig(KConfig&);

Q_SIGNALS:
	void info(const QString&);
	void elementVisibilityChanged();
};

#endif // SURFACE3DPLOTAREADOCK_H
