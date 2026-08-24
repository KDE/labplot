/***************************************************************************
	File                 : Surface3DSceneDock.h
	Project              : LabPlot
	Description          : widget for Surface3DScene properties (container)
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2026 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
***************************************************************************/

#ifndef SURFACE3DSCENEDOCK_H
#define SURFACE3DSCENEDOCK_H

#include "BaseDock.h"
#include "backend/worksheet/plots/3d/Surface3DScene.h"

#include "ui_surface3dscenedock.h"

class Surface3DScene;

class Surface3DSceneDock : public BaseDock {
	Q_OBJECT

public:
	explicit Surface3DSceneDock(QWidget*);
	void setScenes(const QList<Surface3DScene*>&);

private Q_SLOTS:
	void retranslateUi();

	// SLOTs for changes triggered in Surface3DSceneDock
	void xRotationChanged(int);
	void yRotationChanged(int);
	void zoomLevelChanged(int);
	void themeChanged(int);
	void shadowQualityChanged(int);

	// SLOTs for changes triggered in Surface3DScene
	void sceneXRotationChanged(int);
	void sceneYRotationChanged(int);
	void sceneZoomChanged(int);
	void sceneThemeChanged(Base3DPlot::Theme);
	void sceneShadowsQualityChanged(Base3DPlot::ShadowQuality);

private:
	Ui::Surface3DSceneDock ui;
	QList<Surface3DScene*> m_scenes;
	Surface3DScene* m_scene{nullptr};

	void load();
	void loadConfig(KConfig&);
};

#endif // SURFACE3DSCENEDOCK_H
