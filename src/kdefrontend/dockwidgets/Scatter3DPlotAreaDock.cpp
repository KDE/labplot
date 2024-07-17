/*
	File                 : Scatter3DPlotAreaDock.cpp
	Project              : LabPlot
	Description          : widget for Scatter3DPlotArea properties
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Kuntal Bar <barkuntal6@gmail.com>

 SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "Scatter3DPlotAreaDock.h"
#include "TreeViewComboBox.h"
#include "backend/core/AbstractColumn.h"
#include "backend/matrix/Matrix.h"
#include <backend/core/AspectTreeModel.h>
// #include <kdefrontend/TemplateHandler.h>

Scatter3DPlotAreaDock::Scatter3DPlotAreaDock(QWidget* parent)
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
	connect(ui.cbXCoordinate, &TreeViewComboBox::currentModelIndexChanged, this, &Scatter3DPlotAreaDock::xColumnChanged);
	connect(ui.cbYCoordinate, &TreeViewComboBox::currentModelIndexChanged, this, &Scatter3DPlotAreaDock::yColumnChanged);
	connect(ui.cbZCoordinate, &TreeViewComboBox::currentModelIndexChanged, this, &Scatter3DPlotAreaDock::zColumnChanged);
	connect(ui.cbPointStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Scatter3DPlotAreaDock::pointStyleChanged);
	connect(ui.cbTheme, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Scatter3DPlotAreaDock::themeChanged);
	connect(ui.cbShadowQuality, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Scatter3DPlotAreaDock::shadowQualityChanged);
	connect(ui.dsbOpacity, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Scatter3DPlotAreaDock::opacityChanged);
	connect(ui.slXRot, &QSlider::sliderMoved, this, &Scatter3DPlotAreaDock::xRotationChanged);
	connect(ui.slYRot, &QSlider::sliderMoved, this, &Scatter3DPlotAreaDock::yRotationChanged);
	connect(ui.slZoom, &QSlider::sliderMoved, this, &Scatter3DPlotAreaDock::zoomLevelChanged);
}

void Scatter3DPlotAreaDock::setScatters(const QList<Scatter3DPlotArea*>& scatters) {
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
	scatterXColumnChanged(m_scatter->xColumn());
	scatterYColumnChanged(m_scatter->yColumn());
	scatterZColumnChanged(m_scatter->zColumn());
	scatterThemeChanged(m_scatter->theme());
	scatterShadowQualityChanged(m_scatter->shadowQuality());
	scatterPointStyleChanged(m_scatter->pointStyle());
	scatterOpacityChanged(m_scatter->opacity());
	scatterZoomLevelChanged(m_scatter->zoomLevel());
	scatterXRotationChanged(m_scatter->xRotation());
	scatterYRotationChanged(m_scatter->yRotation());

	connect(m_scatter, &Scatter3DPlotArea::xColumnChanged, this, &Scatter3DPlotAreaDock::scatterXColumnChanged);
	connect(m_scatter, &Scatter3DPlotArea::yColumnChanged, this, &Scatter3DPlotAreaDock::scatterYColumnChanged);
	connect(m_scatter, &Scatter3DPlotArea::zColumnChanged, this, &Scatter3DPlotAreaDock::scatterZColumnChanged);
	connect(m_scatter, &Scatter3DPlotArea::themeChanged, this, &Scatter3DPlotAreaDock::scatterThemeChanged);
	connect(m_scatter, &Scatter3DPlotArea::shadowQualityChanged, this, &Scatter3DPlotAreaDock::scatterShadowQualityChanged);
	connect(m_scatter, &Scatter3DPlotArea::pointStyleChanged, this, &Scatter3DPlotAreaDock::scatterPointStyleChanged);
	connect(m_scatter, &Scatter3DPlotArea::opacityChanged, this, &Scatter3DPlotAreaDock::scatterOpacityChanged);
	connect(m_scatter, &Scatter3DPlotArea::zoomLevelChanged, this, &Scatter3DPlotAreaDock::scatterZoomLevelChanged);
	connect(m_scatter, &Scatter3DPlotArea::xRotationChanged, this, &Scatter3DPlotAreaDock::scatterXRotationChanged);
	connect(m_scatter, &Scatter3DPlotArea::yRotationChanged, this, &Scatter3DPlotAreaDock::scatterYRotationChanged);
}

void Scatter3DPlotAreaDock::retranslateUi() {
	// This function should contain translation code if needed
	ui.cbShadowQuality->insertItem(Scatter3DPlotArea::None, i18n("None"));
	ui.cbShadowQuality->insertItem(Scatter3DPlotArea::Low, i18n("Low"));
	ui.cbShadowQuality->insertItem(Scatter3DPlotArea::Medium, i18n("Medium"));
	ui.cbShadowQuality->insertItem(Scatter3DPlotArea::High, i18n("High"));
	ui.cbShadowQuality->insertItem(Scatter3DPlotArea::SoftLow, i18n("Soft Low"));
	ui.cbShadowQuality->insertItem(Scatter3DPlotArea::SoftMedium, i18n("Soft Medium"));
	ui.cbShadowQuality->insertItem(Scatter3DPlotArea::SoftHigh, i18n("Soft High"));

	ui.cbTheme->insertItem(Scatter3DPlotArea::Theme::Qt, i18n("Qt"));
	ui.cbTheme->insertItem(Scatter3DPlotArea::Theme::PrimaryColors, i18n("Primary Colors"));
	ui.cbTheme->insertItem(Scatter3DPlotArea::Theme::StoneMoss, i18n("Stone Moss"));
	ui.cbTheme->insertItem(Scatter3DPlotArea::Theme::ArmyBlue, i18n("Army Blue"));
	ui.cbTheme->insertItem(Scatter3DPlotArea::Theme::Retro, i18n("Retro"));
	ui.cbTheme->insertItem(Scatter3DPlotArea::Theme::Ebony, i18n("Ebony"));
	ui.cbTheme->insertItem(Scatter3DPlotArea::Theme::Isabelle, i18n("Isabelle"));

	ui.cbPointStyle->insertItem(Scatter3DPlotArea::Sphere, i18n("Sphere"));
	ui.cbPointStyle->insertItem(Scatter3DPlotArea::Cube, i18n("Cube"));
	ui.cbPointStyle->insertItem(Scatter3DPlotArea::Cone, i18n("Cone"));
	ui.cbPointStyle->insertItem(Scatter3DPlotArea::Pyramid, i18n("Pyramid"));
}

//*************************************************************
//**** SLOTs for changes triggered in Scatter3DPlotAreaDock ***
//*************************************************************
// Tab "General"

void Scatter3DPlotAreaDock::xColumnChanged(const QModelIndex& index) {
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

void Scatter3DPlotAreaDock::yColumnChanged(const QModelIndex& index) {
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

void Scatter3DPlotAreaDock::zColumnChanged(const QModelIndex& index) {
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
void Scatter3DPlotAreaDock::scatterXColumnChanged(const AbstractColumn* column) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbXCoordinate->setColumn(column, m_scatter->xColumnPath());
}

void Scatter3DPlotAreaDock::scatterYColumnChanged(const AbstractColumn* column) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbYCoordinate->setColumn(column, m_scatter->yColumnPath());
}

void Scatter3DPlotAreaDock::scatterZColumnChanged(const AbstractColumn* column) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbZCoordinate->setColumn(column, m_scatter->zColumnPath());
}

void Scatter3DPlotAreaDock::scatterXRotationChanged(int xRot) {
	CONDITIONAL_LOCK_RETURN;
	ui.slXRot->setValue(xRot);
}

void Scatter3DPlotAreaDock::scatterYRotationChanged(int yRot) {
	CONDITIONAL_LOCK_RETURN;
	ui.slYRot->setValue(yRot);
}
void Scatter3DPlotAreaDock::scatterZoomLevelChanged(int val) {
	CONDITIONAL_LOCK_RETURN;
	ui.slZoom->setValue(val);
}
void Scatter3DPlotAreaDock::scatterShadowQualityChanged(Scatter3DPlotArea::ShadowQuality shadowQuality) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbShadowQuality->setCurrentIndex(shadowQuality);
}
void Scatter3DPlotAreaDock::scatterPointStyleChanged(Scatter3DPlotArea::PointStyle pointStyle) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbPointStyle->setCurrentIndex(pointStyle);
}
void Scatter3DPlotAreaDock::scatterOpacityChanged(double opacity) {
	CONDITIONAL_LOCK_RETURN;
	ui.dsbOpacity->setValue(opacity);

}
void Scatter3DPlotAreaDock::scatterThemeChanged(Scatter3DPlotArea::Theme theme) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbTheme->setCurrentIndex(theme);
}
void Scatter3DPlotAreaDock::xRotationChanged(int xRot) {
	CONDITIONAL_LOCK_RETURN;
	m_scatter->setXRotation(xRot);
}

void Scatter3DPlotAreaDock::yRotationChanged(int yRot) {
	CONDITIONAL_LOCK_RETURN;
	m_scatter->setYRotation(yRot);
}
void Scatter3DPlotAreaDock::zoomLevelChanged(int zoomLevel) {
	CONDITIONAL_LOCK_RETURN;
	m_scatter->setZoomLevel(zoomLevel);
}
void Scatter3DPlotAreaDock::shadowQualityChanged(int shadowQuality) {
	CONDITIONAL_LOCK_RETURN;
	for (Scatter3DPlotArea* surface : m_scatters)
		surface->setShadowQuality(static_cast<Scatter3DPlotArea::ShadowQuality>(shadowQuality));
}
void Scatter3DPlotAreaDock::pointStyleChanged(int pointStyle) {
	CONDITIONAL_LOCK_RETURN;
	for (Scatter3DPlotArea* surface : m_scatters)
		surface->setPointStyle(static_cast<Scatter3DPlotArea::PointStyle>(pointStyle));
}
void Scatter3DPlotAreaDock::opacityChanged(double opacity) {
	CONDITIONAL_LOCK_RETURN;
	for (Scatter3DPlotArea* surface : m_scatters)
		surface->setOpacity(opacity);
}
void Scatter3DPlotAreaDock::themeChanged(int theme) {
	CONDITIONAL_LOCK_RETURN;
	for (Scatter3DPlotArea* surface : m_scatters)
		surface->setTheme(static_cast<Scatter3DPlotArea::Theme>(theme));
}

//*************************************************************
//************************* Settings **************************
//*************************************************************
void Scatter3DPlotAreaDock::load() {
	// TODO
}

void Scatter3DPlotAreaDock::loadConfig(KConfig& config) {
	// TODO
}
