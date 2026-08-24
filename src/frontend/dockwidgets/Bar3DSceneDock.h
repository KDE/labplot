/***************************************************************************
	File                 : Bar3DSceneDock.h
	Project              : LabPlot
	Description          : widget for Bar3DScene properties (container)
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2026 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
***************************************************************************/

#ifndef BAR3DSCENEDOCK_H
#define BAR3DSCENEDOCK_H

#include "BaseDock.h"
#include "backend/worksheet/plots/3d/Bar3DScene.h"

#include "ui_bar3dscenedock.h"

class Bar3DScene;

class Bar3DSceneDock : public BaseDock {
	Q_OBJECT

public:
	explicit Bar3DSceneDock(QWidget*);
	void setScenes(const QList<Bar3DScene*>&);

private Q_SLOTS:
	void retranslateUi();

	// SLOTs for changes triggered in Bar3DSceneDock
	void xRotationChanged(int);
	void yRotationChanged(int);
	void zoomLevelChanged(int);
	void themeChanged(int);
	void shadowQualityChanged(int);

	// SLOTs for changes triggered in Bar3DScene
	void sceneXRotationChanged(int);
	void sceneYRotationChanged(int);
	void sceneZoomChanged(int);
	void sceneThemeChanged(Base3DPlot::Theme);
	void sceneShadowsQualityChanged(Base3DPlot::ShadowQuality);

private:
	Ui::Bar3DSceneDock ui;
	QList<Bar3DScene*> m_scenes;
	Bar3DScene* m_scene{nullptr};

	void load();
	void loadConfig(KConfig&);
};

#endif // BAR3DSCENEDOCK_H
