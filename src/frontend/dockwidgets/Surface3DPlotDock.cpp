/***************************************************************************
	File                 : Surface3DPlotDock.cpp
	Project              : LabPlot
	Description          : widget for Surface3DScene properties
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024-2026 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2024 Kuntal Bar <barkuntal6@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
***************************************************************************/

#include "Surface3DPlotDock.h"
#include "TreeViewComboBox.h"
#include "backend/core/AbstractColumn.h"
#include "backend/matrix/Matrix.h"
#include <backend/core/AspectTreeModel.h>
// #include <kdefrontend/TemplateHandler.h>

Surface3DPlotDock::Surface3DPlotDock(QWidget* parent)
	: BaseDock(parent) {
	ui.setupUi(this);
	setBaseWidgets(ui.leName, ui.teComment);
	setVisibilityWidgets(ui.chkVisible);
	this->retranslateUi();

	const QVector<TreeViewComboBox*> treeViews(QVector<TreeViewComboBox*>() << ui.cbXColumn << ui.cbYColumn << ui.cbZColumn);

	for (auto* view : treeViews)
		view->setTopLevelClasses(TreeViewComboBox::plotColumnTopLevelClasses());

	// Matrix data source
	ui.cbMatrix->setTopLevelClasses({AspectType::Folder, AspectType::Workbook, AspectType::Matrix});

	// SIGNALs/SLOTs
	// General
	connect(ui.cbDataSourceType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Surface3DPlotDock::dataSourceTypeChanged);
	connect(ui.cbXColumn, &TreeViewComboBox::currentModelIndexChanged, this, &Surface3DPlotDock::xColumnChanged);
	connect(ui.cbYColumn, &TreeViewComboBox::currentModelIndexChanged, this, &Surface3DPlotDock::yColumnChanged);
	connect(ui.cbZColumn, &TreeViewComboBox::currentModelIndexChanged, this, &Surface3DPlotDock::zColumnChanged);
	connect(ui.cbMatrix, &TreeViewComboBox::currentModelIndexChanged, this, &Surface3DPlotDock::matrixChanged);

	// Mesh
	connect(ui.cbDrawType, SIGNAL(currentIndexChanged(int)), SLOT(drawModeChanged(int)));
	connect(ui.chkFlatShading, SIGNAL(toggled(bool)), SLOT(flatShadingChanged(bool)));
	connect(ui.cbShadowQuality, SIGNAL(currentIndexChanged(int)), SLOT(shadowQualityChanged(int)));
	connect(ui.chkSmooth, SIGNAL(toggled(bool)), SLOT(smoothChanged(bool)));
	connect(ui.slXRot, &QSlider::sliderMoved, this, &Surface3DPlotDock::xRotationChanged);
	connect(ui.slYRot, &QSlider::sliderMoved, this, &Surface3DPlotDock::yRotationChanged);
	connect(ui.slZoom, &QSlider::sliderMoved, this, &Surface3DPlotDock::zoomLevelChanged);
	connect(ui.kcbColor, &KColorButton::changed, this, &Surface3DPlotDock::colorChanged);
	connect(ui.cbTheme, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Surface3DPlotDock::themeChanged);
}

void Surface3DPlotDock::setSurfaces(const QList<Surface3DScene*>& surfaces) {
	CONDITIONAL_LOCK_RETURN;
	m_surfaces = surfaces;
	m_surface = m_surfaces.first();
	setAspects(surfaces);
	auto* model = aspectModel();

	model->enablePlottableColumnsOnly(true);
	model->enableShowPlotDesignation(true);
	if (m_surface->dataSource() == Surface3DScene::DataSource::DataSource_Spreadsheet)
		model->setSelectableAspects({AspectType::Column});

	ui.cbXColumn->setModel(model);
	ui.cbYColumn->setModel(model);
	ui.cbZColumn->setModel(model);
	if (m_surface->dataSource() == Surface3DScene::DataSource::DataSource_Matrix)
		model->setSelectableAspects({AspectType::Matrix});

	ui.cbMatrix->setModel(model);

	// show the properties of the first surface
	// tab "General"
	ui.cbDataSourceType->setCurrentIndex(static_cast<int>(m_surface->dataSource()));
	ui.cbXColumn->setAspect(m_surface->xColumn(), m_surface->xColumnPath());
	ui.cbYColumn->setAspect(m_surface->yColumn(), m_surface->yColumnPath());
	ui.cbZColumn->setAspect(m_surface->zColumn(), m_surface->zColumnPath());
	ui.cbMatrix->setMatrix(m_surface->matrix(), m_surface->matrixPath());
	// tab "Mesh"

	ui.cbShadowQuality->setCurrentIndex(static_cast<int>(m_surface->shadowQuality()));
	ui.cbDrawType->setCurrentIndex(static_cast<int>(m_surface->drawMode() - 1));

	dataSourceTypeChanged(m_surface->dataSource());

	ui.slXRot->setRange(0, 90);
	ui.slXRot->setValue(m_surface->xRotation());
	ui.slYRot->setRange(0, 90);
	ui.slYRot->setValue(m_surface->yRotation());
	ui.slZoom->setRange(100, 400);
	ui.slZoom->setValue(m_surface->zoomLevel());

	ui.cbTheme->setCurrentIndex(static_cast<int>(m_surface->theme()));
	ui.kcbColor->setColor(Qt::green);

	connect(m_surface, &Surface3DScene::drawModeChanged, this, &Surface3DPlotDock::surfaceDrawModeChanged);
	connect(m_surface, &Surface3DScene::sourceTypeChanged, this, &Surface3DPlotDock::surfaceSourceTypeChanged);
	connect(m_surface, &Surface3DScene::flatShadingChanged, this, &Surface3DPlotDock::surfaceFlatShadingChanged);
	connect(m_surface, &Surface3DScene::shadowQualityChanged, this, &Surface3DPlotDock::surfaceShadowsQualityChanged);
	connect(m_surface, &Surface3DScene::smoothChanged, this, &Surface3DPlotDock::surfaceSmoothChanged);
	connect(m_surface, &Surface3DScene::zoomLevelChanged, this, &Surface3DPlotDock::surfaceZoomChanged);
	connect(m_surface, &Surface3DScene::xRotationChanged, this, &Surface3DPlotDock::surfaceXRotationChanged);
	connect(m_surface, &Surface3DScene::yRotationChanged, this, &Surface3DPlotDock::surfaceYRotationChanged);
	connect(m_surface, &Surface3DScene::themeChanged, this, &Surface3DPlotDock::surfaceThemeChanged);
	connect(m_surface, &Surface3DScene::colorChanged, this, &Surface3DPlotDock::colorChanged);
	connect(m_surface, &Surface3DScene::matrixChanged, this, &Surface3DPlotDock::surfaceMatrixChanged);
	connect(m_surface, &Surface3DScene::xColumnChanged, this, &Surface3DPlotDock::surfaceXColumnChanged);
	connect(m_surface, &Surface3DScene::yColumnChanged, this, &Surface3DPlotDock::surfaceYColumnChanged);
	connect(m_surface, &Surface3DScene::zColumnChanged, this, &Surface3DPlotDock::surfaceZColumnChanged);
}

void Surface3DPlotDock::showItem(QWidget* label, QWidget* comboBox, bool pred) {
	label->setVisible(pred);
	comboBox->setVisible(pred);
}

void Surface3DPlotDock::showTriangleInfo(bool pred) {
	if (m_initializing)
		return;

	showItem(ui.lXColumn, ui.cbXColumn, pred);
	showItem(ui.lYColumn, ui.cbYColumn, pred);
	showItem(ui.lZColumn, ui.cbZColumn, pred);

	Q_EMIT elementVisibilityChanged();
}

void Surface3DPlotDock::retranslateUi() {
	ui.cbDataSourceType->insertItem(Surface3DScene::DataSource_Spreadsheet, i18n("Spreadsheet"));
	ui.cbDataSourceType->insertItem(Surface3DScene::DataSource_Matrix, i18n("Matrix"));
	ui.cbDataSourceType->insertItem(Surface3DScene::DataSource_Empty, i18n("Demo"));

	ui.cbDrawType->insertItem(Surface3DScene::DrawWireframe, i18n("Wireframe"));
	ui.cbDrawType->insertItem(Surface3DScene::DrawSurface, i18n("Surface"));
	ui.cbDrawType->insertItem(Surface3DScene::DrawWireframeSurface, i18n("Wireframe & Surface"));

	ui.cbShadowQuality->insertItem(Surface3DScene::None, i18n("None"));
	ui.cbShadowQuality->insertItem(Surface3DScene::Low, i18n("Low"));
	ui.cbShadowQuality->insertItem(Surface3DScene::Medium, i18n("Medium"));
	ui.cbShadowQuality->insertItem(Surface3DScene::High, i18n("High"));
	ui.cbShadowQuality->insertItem(Surface3DScene::SoftLow, i18n("Soft Low"));
	ui.cbShadowQuality->insertItem(Surface3DScene::SoftMedium, i18n("Soft Medium"));
	ui.cbShadowQuality->insertItem(Surface3DScene::SoftHigh, i18n("Soft High"));

	ui.cbTheme->insertItem(Surface3DScene::Theme::Qt, i18n("Qt"));
	ui.cbTheme->insertItem(Surface3DScene::Theme::PrimaryColors, i18n("Primary Colors"));
	ui.cbTheme->insertItem(Surface3DScene::Theme::StoneMoss, i18n("Stone Moss"));
	ui.cbTheme->insertItem(Surface3DScene::Theme::ArmyBlue, i18n("Army Blue"));
	ui.cbTheme->insertItem(Surface3DScene::Theme::Retro, i18n("Retro"));
	ui.cbTheme->insertItem(Surface3DScene::Theme::Ebony, i18n("Ebony"));
	ui.cbTheme->insertItem(Surface3DScene::Theme::Isabelle, i18n("Isabelle"));
}

//*************************************************************
//**** SLOTs for changes triggered in Surface3DPlotDock ***
//*************************************************************
// Tab "General"

void Surface3DPlotDock::dataSourceTypeChanged(int index) {
	const auto type = static_cast<Surface3DScene::DataSource>(index);
	const bool spreadsheet = (type == Surface3DScene::DataSource::DataSource_Spreadsheet);
	const bool matrix = (type == Surface3DScene::DataSource::DataSource_Matrix);

	ui.lXColumn->setVisible(spreadsheet);
	ui.cbXColumn->setVisible(spreadsheet);

	ui.lYColumn->setVisible(spreadsheet);
	ui.cbYColumn->setVisible(spreadsheet);

	ui.lZColumn->setVisible(spreadsheet);
	ui.cbZColumn->setVisible(spreadsheet);

	ui.lMatrix->setVisible(matrix);
	ui.cbMatrix->setVisible(matrix);

	CONDITIONAL_LOCK_RETURN;

	for (auto* surface : m_surfaces)
		surface->setDataSource(type);
}

void Surface3DPlotDock::xColumnChanged(const QModelIndex& index) {
	CONDITIONAL_LOCK_RETURN;

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	AbstractColumn* column = nullptr;
	if (aspect) {
		column = dynamic_cast<AbstractColumn*>(aspect);
		Q_ASSERT(column);
	}

	for (auto* surface : m_surfaces)
		surface->setXColumn(column);
}

void Surface3DPlotDock::yColumnChanged(const QModelIndex& index) {
	CONDITIONAL_LOCK_RETURN;

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	AbstractColumn* column = nullptr;
	if (aspect) {
		column = dynamic_cast<AbstractColumn*>(aspect);
		Q_ASSERT(column);
	}

	for (auto* curve : m_surfaces)
		curve->setYColumn(column);
}

void Surface3DPlotDock::zColumnChanged(const QModelIndex& index) {
	CONDITIONAL_LOCK_RETURN;

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	AbstractColumn* column = nullptr;
	if (aspect) {
		column = dynamic_cast<AbstractColumn*>(aspect);
		Q_ASSERT(column);
	}

	for (auto* curve : m_surfaces)
		curve->setZColumn(column);
}

void Surface3DPlotDock::xRotationChanged(int value) {
	CONDITIONAL_LOCK_RETURN;
	m_surface->setXRotation(value);
}

void Surface3DPlotDock::yRotationChanged(int value) {
	CONDITIONAL_LOCK_RETURN;
	m_surface->setYRotation(value);
}

void Surface3DPlotDock::themeChanged(int value) {
	CONDITIONAL_LOCK_RETURN;
	m_surface->setTheme(static_cast<Base3DPlot::Theme>(value));
}

void Surface3DPlotDock::zoomLevelChanged(int value) {
	CONDITIONAL_LOCK_RETURN;
	m_surface->setZoomLevel(value);
}

void Surface3DPlotDock::matrixChanged(const QModelIndex& index) {
	CONDITIONAL_LOCK_RETURN;

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	Matrix* matrix = nullptr;
	if (aspect) {
		matrix = dynamic_cast<Matrix*>(aspect);
		Q_ASSERT(matrix);
	}

	for (auto* surface : m_surfaces)
		surface->setMatrix(matrix);
}

void Surface3DPlotDock::updateUiVisibility() {
	const int type = ui.cbDrawType->currentIndex();
	const int dataType = ui.cbDataSourceType->currentIndex();
	if (type == Surface3DScene::DrawMode::DrawSurface || type == Surface3DScene::DrawMode::DrawWireframe) {
		showItem(ui.lDataSourceType, ui.cbDataSourceType, true);
		showItem(ui.lMatrix, ui.cbMatrix, false);
		showTriangleInfo(dataType == Surface3DScene::DataSource_Spreadsheet);
	} else {
		showItem(ui.lDataSourceType, ui.cbDataSourceType, false);
		showTriangleInfo(false);
	}

	Q_EMIT elementVisibilityChanged();
}

// Tab "Mesh"
void Surface3DPlotDock::drawModeChanged(int index) {
	CONDITIONAL_LOCK_RETURN;
	for (auto* surface : m_surfaces)
		surface->setDrawMode(static_cast<Surface3DScene::DrawMode>(index + 1));
}

void Surface3DPlotDock::flatShadingChanged(bool vis) {
	CONDITIONAL_LOCK_RETURN;
	for (auto* surface : m_surfaces)
		surface->setFlatShading(vis);
}

void Surface3DPlotDock::shadowQualityChanged(int index) {
	CONDITIONAL_LOCK_RETURN;
	for (auto* surface : m_surfaces)
		surface->setShadowQuality(static_cast<Surface3DScene::ShadowQuality>(index));
}

void Surface3DPlotDock::smoothChanged(bool vis) {
	CONDITIONAL_LOCK_RETURN;
	for (auto* surface : m_surfaces)
		surface->setSmooth(vis);
}
void Surface3DPlotDock::colorChanged(QColor color) {
	CONDITIONAL_LOCK_RETURN;
	for (auto* surface : m_surfaces)
		surface->setColor(color);
}

//*************************************************************
//******* SLOTs for changes triggered in Surface3DScene ********
//*************************************************************
// Tab "General"
void Surface3DPlotDock::surfaceMatrixChanged(const Matrix* matrix) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbMatrix->setMatrix(matrix, m_surface->matrixPath());
}

void Surface3DPlotDock::surfaceXColumnChanged(const AbstractColumn* column) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbXColumn->setAspect(column, m_surface->xColumnPath());
}

void Surface3DPlotDock::surfaceYColumnChanged(const AbstractColumn* column) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbYColumn->setAspect(column, m_surface->yColumnPath());
}

void Surface3DPlotDock::surfaceZColumnChanged(const AbstractColumn* column) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbZColumn->setAspect(column, m_surface->zColumnPath());
}

// Tab "Mesh"
void Surface3DPlotDock::surfaceDrawModeChanged(Surface3DScene::DrawMode mode) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbDrawType->setCurrentIndex(mode - 1);
}

void Surface3DPlotDock::surfaceThemeChanged(Base3DPlot::Theme theme) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbTheme->setCurrentIndex(theme);
}

void Surface3DPlotDock::surfaceFlatShadingChanged(bool vis) {
	CONDITIONAL_LOCK_RETURN;
	ui.chkFlatShading->setEnabled(vis);
}

void Surface3DPlotDock::surfaceShadowsQualityChanged(Base3DPlot::ShadowQuality quality) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbShadowQuality->setCurrentIndex(quality);
}

void Surface3DPlotDock::surfaceSourceTypeChanged(Surface3DScene::DataSource type) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbDataSourceType->setCurrentIndex(type);
}
void Surface3DPlotDock::surfaceZoomChanged(int val) {
	CONDITIONAL_LOCK_RETURN;
	ui.slZoom->setValue(val);
}

void Surface3DPlotDock::surfaceXRotationChanged(int val) {
	CONDITIONAL_LOCK_RETURN;
	ui.slXRot->setValue(val);
}
void Surface3DPlotDock::surfaceYRotationChanged(int val) {
	CONDITIONAL_LOCK_RETURN;
	ui.slYRot->setValue(val);
}
void Surface3DPlotDock::surfaceSmoothChanged(bool vis) {
	CONDITIONAL_LOCK_RETURN;
	ui.chkSmooth->setEnabled(vis);
}
void Surface3DPlotDock::surfaceColorChanged(QColor color) {
	CONDITIONAL_LOCK_RETURN;
	ui.kcbColor->setColor(color);
}

//*************************************************************
//************************* Settings **************************
//*************************************************************
void Surface3DPlotDock::load() {
	// TODO
}

void Surface3DPlotDock::loadConfig(KConfig& config) {
	// TODO
}
