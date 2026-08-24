/***************************************************************************
	File                 : Scatter3DPlotDock.cpp
	Project              : LabPlot
	Description          : widget for Scatter3DScene properties
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024-2026 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2024 Kuntal Bar <barkuntal6@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
***************************************************************************/

#include "Scatter3DPlotDock.h"
#include "TreeViewComboBox.h"
#include "backend/core/AbstractColumn.h"
#include "backend/matrix/Matrix.h"
#include <backend/core/AspectTreeModel.h>

Scatter3DPlotDock::Scatter3DPlotDock(QWidget* parent)
	: BaseDock(parent) {
	ui.setupUi(this);
	setBaseWidgets(ui.leName, ui.teComment);
	setVisibilityWidgets(ui.chkVisible);
	this->retranslateUi();

	const QVector<TreeViewComboBox*> treeViews(QVector<TreeViewComboBox*>() << ui.cbXCoordinate << ui.cbYCoordinate << ui.cbZCoordinate);

	for (auto* view : treeViews)
		view->setTopLevelClasses(TreeViewComboBox::plotColumnTopLevelClasses());

	// SIGNALs/SLOTs
	// General
	connect(ui.cbXCoordinate, &TreeViewComboBox::currentModelIndexChanged, this, &Scatter3DPlotDock::xColumnChanged);
	connect(ui.cbYCoordinate, &TreeViewComboBox::currentModelIndexChanged, this, &Scatter3DPlotDock::yColumnChanged);
	connect(ui.cbZCoordinate, &TreeViewComboBox::currentModelIndexChanged, this, &Scatter3DPlotDock::zColumnChanged);
	connect(ui.cbPointStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Scatter3DPlotDock::pointStyleChanged);
	connect(ui.cbTheme, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Scatter3DPlotDock::themeChanged);
	connect(ui.cbShadowQuality, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Scatter3DPlotDock::shadowQualityChanged);
	connect(ui.slXRot, &QSlider::sliderMoved, this, &Scatter3DPlotDock::xRotationChanged);
	connect(ui.slYRot, &QSlider::sliderMoved, this, &Scatter3DPlotDock::yRotationChanged);
	connect(ui.slZoom, &QSlider::sliderMoved, this, &Scatter3DPlotDock::zoomLevelChanged);
	connect(ui.kcbColor, &KColorButton::changed, this, &Scatter3DPlotDock::colorChanged);
}

void Scatter3DPlotDock::setScatters(const QList<Scatter3DScene*>& scatters) {
	CONDITIONAL_LOCK_RETURN;
	m_scatters = scatters;
	m_scatter = m_scatters.first();
	setAspects(scatters);
	auto* model = aspectModel();

	model->enablePlottableColumnsOnly(true);
	model->enableShowPlotDesignation(true);
	model->setSelectableAspects({AspectType::Column});

	ui.cbXCoordinate->setModel(model);
	ui.cbYCoordinate->setModel(model);
	ui.cbZCoordinate->setModel(model);

	// show the properties of the first scatter
	// tab "General"
	ui.cbXCoordinate->setAspect(m_scatter->xColumn(), m_scatter->xColumnPath());
	ui.cbYCoordinate->setAspect(m_scatter->yColumn(), m_scatter->yColumnPath());
	ui.cbZCoordinate->setAspect(m_scatter->zColumn(), m_scatter->zColumnPath());
	ui.cbTheme->setCurrentIndex(m_scatter->theme());
	ui.cbShadowQuality->setCurrentIndex(m_scatter->shadowQuality());
	ui.cbPointStyle->setCurrentIndex(m_scatter->pointStyle());
	ui.slZoom->setRange(100, 400);
	ui.slXRot->setRange(0, 90);
	ui.slYRot->setRange(0, 90);
	ui.slZoom->setValue(m_scatter->zoomLevel());
	ui.slXRot->setValue(m_scatter->xRotation());
	ui.slYRot->setValue(m_scatter->yRotation());
	ui.kcbColor->setColor(m_scatter->color());
	connect(m_scatter, &Scatter3DScene::xColumnChanged, this, &Scatter3DPlotDock::scatterXColumnChanged);
	connect(m_scatter, &Scatter3DScene::yColumnChanged, this, &Scatter3DPlotDock::scatterYColumnChanged);
	connect(m_scatter, &Scatter3DScene::zColumnChanged, this, &Scatter3DPlotDock::scatterZColumnChanged);
	connect(m_scatter, &Scatter3DScene::themeChanged, this, &Scatter3DPlotDock::scatterThemeChanged);
	connect(m_scatter, &Scatter3DScene::shadowQualityChanged, this, &Scatter3DPlotDock::scatterShadowQualityChanged);
	connect(m_scatter, &Scatter3DScene::pointStyleChanged, this, &Scatter3DPlotDock::scatterPointStyleChanged);
	connect(m_scatter, &Scatter3DScene::zoomLevelChanged, this, &Scatter3DPlotDock::scatterZoomLevelChanged);
	connect(m_scatter, &Scatter3DScene::xRotationChanged, this, &Scatter3DPlotDock::scatterXRotationChanged);
	connect(m_scatter, &Scatter3DScene::yRotationChanged, this, &Scatter3DPlotDock::scatterYRotationChanged);
	connect(m_scatter, &Scatter3DScene::colorChanged, this, &Scatter3DPlotDock::scatterColorChanged);
}

void Scatter3DPlotDock::retranslateUi() {
	// This function should contain translation code if needed
	ui.cbShadowQuality->insertItem(Scatter3DScene::None, i18n("None"));
	ui.cbShadowQuality->insertItem(Scatter3DScene::Low, i18n("Low"));
	ui.cbShadowQuality->insertItem(Scatter3DScene::Medium, i18n("Medium"));
	ui.cbShadowQuality->insertItem(Scatter3DScene::High, i18n("High"));
	ui.cbShadowQuality->insertItem(Scatter3DScene::SoftLow, i18n("Soft Low"));
	ui.cbShadowQuality->insertItem(Scatter3DScene::SoftMedium, i18n("Soft Medium"));
	ui.cbShadowQuality->insertItem(Scatter3DScene::SoftHigh, i18n("Soft High"));

	ui.cbTheme->insertItem(Scatter3DScene::Theme::Qt, i18n("Qt"));
	ui.cbTheme->insertItem(Scatter3DScene::Theme::PrimaryColors, i18n("Primary Colors"));
	ui.cbTheme->insertItem(Scatter3DScene::Theme::StoneMoss, i18n("Stone Moss"));
	ui.cbTheme->insertItem(Scatter3DScene::Theme::ArmyBlue, i18n("Army Blue"));
	ui.cbTheme->insertItem(Scatter3DScene::Theme::Retro, i18n("Retro"));
	ui.cbTheme->insertItem(Scatter3DScene::Theme::Ebony, i18n("Ebony"));
	ui.cbTheme->insertItem(Scatter3DScene::Theme::Isabelle, i18n("Isabelle"));

	ui.cbPointStyle->insertItem(Scatter3DScene::Sphere, i18n("Sphere"));
	ui.cbPointStyle->insertItem(Scatter3DScene::Cube, i18n("Cube"));
	ui.cbPointStyle->insertItem(Scatter3DScene::Cone, i18n("Cone"));
	ui.cbPointStyle->insertItem(Scatter3DScene::Pyramid, i18n("Pyramid"));
}

//*************************************************************
//**** SLOTs for changes triggered in Scatter3DPlotDock ***
//*************************************************************
// Tab "General"

void Scatter3DPlotDock::xColumnChanged(const QModelIndex& index) {
	CONDITIONAL_LOCK_RETURN;

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	AbstractColumn* column = nullptr;
	if (aspect) {
		column = dynamic_cast<AbstractColumn*>(aspect);
		Q_ASSERT(column);
	}

	for (auto* scatter : m_scatters)
		scatter->setXColumn(column);
}

void Scatter3DPlotDock::yColumnChanged(const QModelIndex& index) {
	CONDITIONAL_LOCK_RETURN;

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	AbstractColumn* column = nullptr;
	if (aspect) {
		column = dynamic_cast<AbstractColumn*>(aspect);
		Q_ASSERT(column);
	}

	for (auto* scatter : m_scatters)
		scatter->setYColumn(column);
}

void Scatter3DPlotDock::zColumnChanged(const QModelIndex& index) {
	CONDITIONAL_LOCK_RETURN;

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	AbstractColumn* column = nullptr;
	if (aspect) {
		column = dynamic_cast<AbstractColumn*>(aspect);
		Q_ASSERT(column);
	}

	for (auto* scatter : m_scatters)
		scatter->setZColumn(column);
}

//*************************************************************
//***** SLOTs for changes triggered in Scatter3DPlotArea ******
//*************************************************************
// Tab "General"
void Scatter3DPlotDock::scatterXColumnChanged(const AbstractColumn* column) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbXCoordinate->setAspect(column, m_scatter->xColumnPath());
}

void Scatter3DPlotDock::scatterYColumnChanged(const AbstractColumn* column) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbYCoordinate->setAspect(column, m_scatter->yColumnPath());
}

void Scatter3DPlotDock::scatterZColumnChanged(const AbstractColumn* column) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbZCoordinate->setAspect(column, m_scatter->zColumnPath());
}

void Scatter3DPlotDock::scatterXRotationChanged(int xRot) {
	CONDITIONAL_LOCK_RETURN;
	ui.slXRot->setValue(xRot);
}

void Scatter3DPlotDock::scatterYRotationChanged(int yRot) {
	CONDITIONAL_LOCK_RETURN;
	ui.slYRot->setValue(yRot);
}
void Scatter3DPlotDock::scatterZoomLevelChanged(int val) {
	CONDITIONAL_LOCK_RETURN;
	ui.slZoom->setValue(val);
}
void Scatter3DPlotDock::scatterShadowQualityChanged(Base3DPlot::ShadowQuality shadowQuality) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbShadowQuality->setCurrentIndex(shadowQuality);
}
void Scatter3DPlotDock::scatterPointStyleChanged(Scatter3DScene::PointStyle pointStyle) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbPointStyle->setCurrentIndex(pointStyle);
}

void Scatter3DPlotDock::scatterThemeChanged(Base3DPlot::Theme theme) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbTheme->setCurrentIndex(theme);
}

void Scatter3DPlotDock::scatterColorChanged(QColor color) {
	CONDITIONAL_LOCK_RETURN;
	ui.kcbColor->setColor(color);
}
void Scatter3DPlotDock::xRotationChanged(int xRot) {
	CONDITIONAL_LOCK_RETURN;
	m_scatter->setXRotation(xRot);
}

void Scatter3DPlotDock::yRotationChanged(int yRot) {
	CONDITIONAL_LOCK_RETURN;
	m_scatter->setYRotation(yRot);
}
void Scatter3DPlotDock::zoomLevelChanged(int zoomLevel) {
	CONDITIONAL_LOCK_RETURN;
	m_scatter->setZoomLevel(zoomLevel);
}
void Scatter3DPlotDock::shadowQualityChanged(int shadowQuality) {
	CONDITIONAL_LOCK_RETURN;
	for (Scatter3DScene* surface : m_scatters)
		surface->setShadowQuality(static_cast<Base3DPlot::ShadowQuality>(shadowQuality));
}
void Scatter3DPlotDock::pointStyleChanged(int pointStyle) {
	CONDITIONAL_LOCK_RETURN;
	for (Scatter3DScene* surface : m_scatters)
		surface->setPointStyle(static_cast<Scatter3DScene::PointStyle>(pointStyle));
}

void Scatter3DPlotDock::themeChanged(int theme) {
	CONDITIONAL_LOCK_RETURN;
	for (Scatter3DScene* surface : m_scatters)
		surface->setTheme(static_cast<Base3DPlot::Theme>(theme));
}

void Scatter3DPlotDock::colorChanged(QColor color) {
	CONDITIONAL_LOCK_RETURN;
	for (Scatter3DScene* surface : m_scatters)
		surface->setColor(color);
}

//*************************************************************
//************************* Settings **************************
//*************************************************************
void Scatter3DPlotDock::load() {
	// TODO
}

void Scatter3DPlotDock::loadConfig(KConfig& config) {
	// TODO
}
