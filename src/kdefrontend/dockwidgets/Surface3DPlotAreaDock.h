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
#include "backend/worksheet/plots/3d/Surface3DPlotArea.h"

#include "ui_surface3dplotareadock.h"

class Surface3DPlotArea;
class Matrix;
class AbstractColumn;
class AspectTreeModel;
class ColorMapSelector;
class TreeViewComboBox;

class Surface3DPlotAreaDock : public BaseDock {
	Q_OBJECT

public:
    explicit Surface3DPlotAreaDock(QWidget* parent);
    void setSurfaces(const QList<Surface3DPlotArea*>& surfaces);

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

    void xRotationChanged(int);
    void yRotationChanged(int);
    void zoomChanged(int);

    void matrixChanged(const QModelIndex&);

	// SLOTs for changes triggered in Surface3DPlotArea
    void sourceTypeChanged(Surface3DPlotArea::DataSource);
    void surfaceXColumnChanged(const AbstractColumn*);
    void surfaceYColumnChanged(const AbstractColumn*);
    void surfaceZColumnChanged(const AbstractColumn*);
    void surfaceMatrixChanged(const Matrix*);

    // Appearance properties
	void onDrawModeChanged(int);
    void onShadowQualityChanged(int);
    void onFlatShadingChanged(bool);
    void onZoomLevelChanged(int);
    void onXRotationChanged(int);
    void onYRotationChanged(int);
    void onThemeChanged(int);


	// Rendering properties
	void onSmoothChanged(bool);

	void drawModeChanged(Surface3DPlotArea::DrawMode mode);
    void themeChanged(Surface3DPlotArea::Theme theme);
	void flatShadingChanged(bool);
	void shadowsQualityChanged(Surface3DPlotArea::ShadowQuality quality);

private:
	Ui::Surface3DPlotAreaDock ui;
	QList<Surface3DPlotArea*> m_surfaces;
	Surface3DPlotArea* m_surface{nullptr};

	void load();
	void loadConfig(KConfig&);

Q_SIGNALS:
    void info(const QString&);
    void elementVisibilityChanged();
};

#endif // SURFACE3DPLOTAREADOCK_H
