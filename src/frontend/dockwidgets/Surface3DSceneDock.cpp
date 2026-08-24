/***************************************************************************
	File                 : Surface3DSceneDock.cpp
	Project              : LabPlot
	Description          : widget for Surface3DScene properties (container)
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2026 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
***************************************************************************/

#include "Surface3DSceneDock.h"

Surface3DSceneDock::Surface3DSceneDock(QWidget* parent)
	: BaseDock(parent) {
	ui.setupUi(this);
	setBaseWidgets(ui.leName, ui.teComment);
	setVisibilityWidgets(ui.chkVisible);
	this->retranslateUi();

	// SIGNALs/SLOTs - Scene properties
	connect(ui.slXRot, &QSlider::sliderMoved, this, &Surface3DSceneDock::xRotationChanged);
	connect(ui.slYRot, &QSlider::sliderMoved, this, &Surface3DSceneDock::yRotationChanged);
	connect(ui.slZoom, &QSlider::sliderMoved, this, &Surface3DSceneDock::zoomLevelChanged);
	connect(ui.cbTheme, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Surface3DSceneDock::themeChanged);
	connect(ui.cbShadowQuality, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Surface3DSceneDock::shadowQualityChanged);
}

void Surface3DSceneDock::setScenes(const QList<Surface3DScene*>& scenes) {
	CONDITIONAL_LOCK_RETURN;
	m_scenes = scenes;
	m_scene = m_scenes.first();
	setAspects(scenes);

	// Show properties of the first scene
	ui.slXRot->setValue(m_scene->xRotation());
	ui.slYRot->setValue(m_scene->yRotation());
	ui.slZoom->setValue(m_scene->zoomLevel());
	ui.cbTheme->setCurrentIndex(static_cast<int>(m_scene->theme()));
	ui.cbShadowQuality->setCurrentIndex(static_cast<int>(m_scene->shadowQuality()));

	// SIGNALs/SLOTs - connect to scene changes
	for (auto* scene : m_scenes) {
		connect(scene, &Surface3DScene::xRotationChanged, this, &Surface3DSceneDock::sceneXRotationChanged);
		connect(scene, &Surface3DScene::yRotationChanged, this, &Surface3DSceneDock::sceneYRotationChanged);
		connect(scene, &Surface3DScene::zoomLevelChanged, this, &Surface3DSceneDock::sceneZoomChanged);
		connect(scene, &Surface3DScene::themeChanged, this, &Surface3DSceneDock::sceneThemeChanged);
		connect(scene, &Surface3DScene::shadowQualityChanged, this, &Surface3DSceneDock::sceneShadowsQualityChanged);
	}

	CONDITIONAL_RETURN_NO_LOCK;
	load();
}

void Surface3DSceneDock::retranslateUi() {
	ui.cbTheme->clear();
	ui.cbTheme->addItem(i18n("Qt"));
	ui.cbTheme->addItem(i18n("Primary Colors"));
	ui.cbTheme->addItem(i18n("Stone Moss"));
	ui.cbTheme->addItem(i18n("Army Blue"));
	ui.cbTheme->addItem(i18n("Retro"));
	ui.cbTheme->addItem(i18n("Ebony"));
	ui.cbTheme->addItem(i18n("Isabelle"));

	ui.cbShadowQuality->clear();
	ui.cbShadowQuality->addItem(i18n("None"));
	ui.cbShadowQuality->addItem(i18n("Low"));
	ui.cbShadowQuality->addItem(i18n("Medium"));
	ui.cbShadowQuality->addItem(i18n("High"));
	ui.cbShadowQuality->addItem(i18n("Soft Low"));
	ui.cbShadowQuality->addItem(i18n("Soft Medium"));
	ui.cbShadowQuality->addItem(i18n("Soft High"));
}

//*************************************************************
//******* SLOTs for changes triggered in Surface3DSceneDock **
//*************************************************************
void Surface3DSceneDock::xRotationChanged(int value) {
	CONDITIONAL_LOCK_RETURN;
	for (auto* scene : m_scenes)
		scene->setXRotation(value);
}

void Surface3DSceneDock::yRotationChanged(int value) {
	CONDITIONAL_LOCK_RETURN;
	for (auto* scene : m_scenes)
		scene->setYRotation(value);
}

void Surface3DSceneDock::zoomLevelChanged(int value) {
	CONDITIONAL_LOCK_RETURN;
	for (auto* scene : m_scenes)
		scene->setZoomLevel(value);
}

void Surface3DSceneDock::themeChanged(int index) {
	CONDITIONAL_LOCK_RETURN;
	auto theme = static_cast<Base3DPlot::Theme>(index);
	for (auto* scene : m_scenes)
		scene->setTheme(theme);
}

void Surface3DSceneDock::shadowQualityChanged(int index) {
	CONDITIONAL_LOCK_RETURN;
	auto quality = static_cast<Base3DPlot::ShadowQuality>(index);
	for (auto* scene : m_scenes)
		scene->setShadowQuality(quality);
}

//*************************************************************
//******** SLOTs for changes triggered in Surface3DScene *****
//*************************************************************
void Surface3DSceneDock::sceneXRotationChanged(int value) {
	CONDITIONAL_LOCK_RETURN;
	ui.slXRot->setValue(value);
}

void Surface3DSceneDock::sceneYRotationChanged(int value) {
	CONDITIONAL_LOCK_RETURN;
	ui.slYRot->setValue(value);
}

void Surface3DSceneDock::sceneZoomChanged(int value) {
	CONDITIONAL_LOCK_RETURN;
	ui.slZoom->setValue(value);
}

void Surface3DSceneDock::sceneThemeChanged(Base3DPlot::Theme theme) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbTheme->setCurrentIndex(static_cast<int>(theme));
}

void Surface3DSceneDock::sceneShadowsQualityChanged(Base3DPlot::ShadowQuality quality) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbShadowQuality->setCurrentIndex(static_cast<int>(quality));
}

//*************************************************************
//************************* Settings **************************
//*************************************************************
void Surface3DSceneDock::load() {
	// Nothing to load from config for now
}

void Surface3DSceneDock::loadConfig(KConfig& config) {
	Q_UNUSED(config)
	// Future: load default scene settings
}
