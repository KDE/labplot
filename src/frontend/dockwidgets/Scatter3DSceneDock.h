/***************************************************************************
	File                 : Scatter3DSceneDock.h
	Project              : LabPlot
	Description          : widget for Scatter3DScene properties (container)
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2026 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
***************************************************************************/

#ifndef SCATTER3DSCENEDOCK_H
#define SCATTER3DSCENEDOCK_H

#include "BaseDock.h"
#include "backend/worksheet/plots/3d/Scatter3DScene.h"

#include "ui_scatter3dscenedock.h"

class Scatter3DScene;

class Scatter3DSceneDock : public BaseDock {
	Q_OBJECT

public:
	explicit Scatter3DSceneDock(QWidget*);
	void setScenes(const QList<Scatter3DScene*>&);

private Q_SLOTS:
	void retranslateUi();

	// SLOTs for changes triggered in Scatter3DSceneDock
	void xRotationChanged(int);
	void yRotationChanged(int);
	void zoomLevelChanged(int);
	void themeChanged(int);
	void shadowQualityChanged(int);

	// SLOTs for changes triggered in Scatter3DScene
	void sceneXRotationChanged(int);
	void sceneYRotationChanged(int);
	void sceneZoomChanged(int);
	void sceneThemeChanged(Base3DPlot::Theme);
	void sceneShadowsQualityChanged(Base3DPlot::ShadowQuality);

private:
	Ui::Scatter3DSceneDock ui;
	QList<Scatter3DScene*> m_scenes;
	Scatter3DScene* m_scene{nullptr};

	void load();
	void loadConfig(KConfig&);
};

#endif // SCATTER3DSCENEDOCK_H
