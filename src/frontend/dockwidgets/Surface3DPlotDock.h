/***************************************************************************
	File                 : Surface3DPlotDock.h
	Project              : LabPlot
	Description          : widget for Surface3DScene properties
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024-2026 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2024 Kuntal Bar <barkuntal6@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
***************************************************************************/

#ifndef SURFACE3DPLOTDOCK_H
#define SURFACE3DPLOTDOCK_H

#include "BaseDock.h"
#include "backend/worksheet/plots/3d/Surface3DScene.h"

#include "ui_surface3dplotdock.h"

class Surface3DScene;
class Matrix;
class AbstractColumn;
class AspectTreeModel;
class ColorMapSelector;
class TreeViewComboBox;

class Surface3DPlotDock : public BaseDock {
	Q_OBJECT

public:
	explicit Surface3DPlotDock(QWidget*);
	void setSurfaces(const QList<Surface3DScene*>&);

private:
	void showTriangleInfo(bool pred);
	void showItem(QWidget* label, QWidget* comboBox, bool pred);

	void updateUiVisibility();

private Q_SLOTS:
	void retranslateUi();

	// SLOTs for changes triggered in Surface3DPlotDock
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

	// SLOTs for changes triggered in Surface3DPlotDock
	void surfaceDrawModeChanged(Surface3DScene::DrawMode);
	void surfaceThemeChanged(Base3DPlot::Theme);
	void surfaceFlatShadingChanged(bool);
	void surfaceShadowsQualityChanged(Base3DPlot::ShadowQuality);
	void surfaceSourceTypeChanged(Surface3DScene::DataSource);
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
	Ui::Surface3DPlotDock ui;
	QList<Surface3DScene*> m_surfaces;
	Surface3DScene* m_surface{nullptr};

	void load();
	void loadConfig(KConfig&);

Q_SIGNALS:
	void info(const QString&);
	void elementVisibilityChanged();
};

#endif // SURFACE3DPLOTDOCK_H
