/*
	File                 : Surface3DPlotAreaDock.cpp
	Project              : LabPlot
	Description          : widget for Surface3DPlotArea properties
	--------------------------------------------------------------------
    SPDX-FileCopyrightText: 2024 Kuntal Bar <barkuntal6@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "Surface3DPlotAreaDock.h"
#include "TreeViewComboBox.h"
#include "backend/core/AbstractColumn.h"
#include "backend/matrix/Matrix.h"
#include <backend/core/AspectTreeModel.h>
// #include <kdefrontend/TemplateHandler.h>

Surface3DPlotAreaDock::Surface3DPlotAreaDock(QWidget* parent)
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
    // TODO  ui.cbMatrix->setSelectableAspects({AspectType::Matrix});

    // SIGNALs/SLOTs
    // General
    connect(ui.cbDataSourceType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Surface3DPlotAreaDock::dataSourceTypeChanged);
    connect(ui.cbXColumn, &TreeViewComboBox::currentModelIndexChanged, this, &Surface3DPlotAreaDock::xColumnChanged);
    connect(ui.cbYColumn, &TreeViewComboBox::currentModelIndexChanged, this, &Surface3DPlotAreaDock::yColumnChanged);
    connect(ui.cbZColumn, &TreeViewComboBox::currentModelIndexChanged, this, &Surface3DPlotAreaDock::zColumnChanged);
    connect(ui.cbMatrix, &TreeViewComboBox::currentModelIndexChanged, this, &Surface3DPlotAreaDock::matrixChanged);

    // Mesh
    connect(ui.cbDrawType, SIGNAL(currentIndexChanged(int)), SLOT(onDrawModeChanged(int)));
    connect(ui.chkFlatShading, SIGNAL(toggled(bool)), SLOT(onFlatShadingChanged(bool)));
    connect(ui.cbShadowQuality, SIGNAL(currentIndexChanged(int)), SLOT(onShadowQualityChanged(int)));
    connect(ui.chkSmooth, SIGNAL(toggled(bool)), SLOT(onSmoothChanged(bool)));

    connect(ui.slXRot, &QSlider::sliderMoved, this, &Surface3DPlotAreaDock::onXRotationChanged);
    connect(ui.slYRot, &QSlider::sliderMoved, this, &Surface3DPlotAreaDock::onYRotationChanged);
    connect(ui.slZoom, &QSlider::sliderMoved, this, &Surface3DPlotAreaDock::onZoomLevelChanged);
    connect(ui.cbTheme, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Surface3DPlotAreaDock::onThemeChanged);
}

void Surface3DPlotAreaDock::setSurfaces(const QList<Surface3DPlotArea*>& surfaces) {
    CONDITIONAL_LOCK_RETURN;
    m_surfaces = surfaces;
    m_surface = m_surfaces.first();
    setAspects(surfaces);
    auto* model = aspectModel();

    model->enablePlottableColumnsOnly(true);
    model->enableShowPlotDesignation(true);
    if (m_surface->dataSource() == Surface3DPlotArea::DataSource::DataSource_Spreadsheet)
        model->setSelectableAspects({AspectType::Column});

    ui.cbXColumn->setModel(model);
    ui.cbYColumn->setModel(model);
    ui.cbZColumn->setModel(model);
    if (m_surface->dataSource() == Surface3DPlotArea::DataSource::DataSource_Matrix)
        model->setSelectableAspects({AspectType::Matrix});

    ui.cbMatrix->setModel(model);

    // show the properties of the first surface
    // tab "General"
    ui.cbDataSourceType->setCurrentIndex(static_cast<int>(m_surface->dataSource()));
    dataSourceTypeChanged(ui.cbDataSourceType->currentIndex());
    surfaceXColumnChanged(m_surface->xColumn());
    surfaceXColumnChanged(m_surface->yColumn());
    surfaceXColumnChanged(m_surface->zColumn());

    // // TODO: matrix
    // ui.cbMatrix->setMatrix(m_surface->matrix(), m_surface->matrixPath());
    surfaceMatrixChanged(m_surface->matrix());
    // tab "Mesh"

    ui.cbShadowQuality->setCurrentIndex(static_cast<int>(m_surface->shadowQuality()));
    drawModeChanged(m_surface->drawMode());
    ui.cbDrawType->setCurrentIndex(static_cast<int>(m_surface->drawMode() - 1));

    sourceTypeChanged(m_surface->dataSource());

    ui.slXRot->setRange(0, 360);
    ui.slXRot->setValue(m_surface->xRotation());
    ui.slYRot->setRange(0, 360);
    ui.slYRot->setValue(m_surface->yRotation());
    ui.slZoom->setRange(0, 5);
    ui.slZoom->setValue(m_surface->zoomLevel());

    ui.cbTheme->setCurrentIndex(static_cast<int>(m_surface->theme()));

    connect(m_surface, &Surface3DPlotArea::drawModeChanged, this, &Surface3DPlotAreaDock::drawModeChanged);
    connect(m_surface, &Surface3DPlotArea::sourceTypeChanged, this, &Surface3DPlotAreaDock::sourceTypeChanged);
    connect(m_surface, &Surface3DPlotArea::flatShadingChanged, this, &Surface3DPlotAreaDock::flatShadingChanged);
    connect(m_surface, &Surface3DPlotArea::shadowQualityChanged, this, &Surface3DPlotAreaDock::shadowsQualityChanged);
    connect(m_surface, &Surface3DPlotArea::smoothChanged, this, &Surface3DPlotAreaDock::onSmoothChanged);
    connect(m_surface, &Surface3DPlotArea::zoomChanged, this, &Surface3DPlotAreaDock::zoomChanged);
    connect(m_surface, &Surface3DPlotArea::xRotationChanged, this, &Surface3DPlotAreaDock::xRotationChanged);
    connect(m_surface, &Surface3DPlotArea::yRotationChanged, this, &Surface3DPlotAreaDock::yRotationChanged);
    connect(m_surface, &Surface3DPlotArea::themeChanged, this, &Surface3DPlotAreaDock::themeChanged);

    connect(m_surface, &Surface3DPlotArea::matrixChanged, this, &Surface3DPlotAreaDock::surfaceMatrixChanged);
    connect(m_surface, &Surface3DPlotArea::xColumnChanged, this, &Surface3DPlotAreaDock::surfaceXColumnChanged);
    connect(m_surface, &Surface3DPlotArea::yColumnChanged, this, &Surface3DPlotAreaDock::surfaceYColumnChanged);
    connect(m_surface, &Surface3DPlotArea::zColumnChanged, this, &Surface3DPlotAreaDock::surfaceZColumnChanged);
}

void Surface3DPlotAreaDock::showItem(QWidget* label, QWidget* comboBox, bool pred) {
	label->setVisible(pred);
	comboBox->setVisible(pred);
}

void Surface3DPlotAreaDock::showTriangleInfo(bool pred) {
	if (m_initializing)
		return;

	showItem(ui.lXColumn, ui.cbXColumn, pred);
	showItem(ui.lYColumn, ui.cbYColumn, pred);
	showItem(ui.lZColumn, ui.cbZColumn, pred);

	Q_EMIT elementVisibilityChanged();
}

void Surface3DPlotAreaDock::retranslateUi() {
    ui.cbDataSourceType->insertItem(Surface3DPlotArea::DataSource_Spreadsheet, i18n("Spreadsheet"));
    ui.cbDataSourceType->insertItem(Surface3DPlotArea::DataSource_Matrix, i18n("Matrix"));
    ui.cbDataSourceType->insertItem(Surface3DPlotArea::DataSource_Empty, i18n("Demo"));

    ui.cbDrawType->insertItem(Surface3DPlotArea::DrawWireframe, i18n("Wireframe"));
    ui.cbDrawType->insertItem(Surface3DPlotArea::DrawSurface, i18n("Surface"));
    ui.cbDrawType->insertItem(Surface3DPlotArea::DrawWireframeSurface, i18n("Wireframe & Surface"));

    ui.cbShadowQuality->insertItem(Surface3DPlotArea::None, i18n("None"));
    ui.cbShadowQuality->insertItem(Surface3DPlotArea::Low, i18n("Low"));
    ui.cbShadowQuality->insertItem(Surface3DPlotArea::Medium, i18n("Medium"));
    ui.cbShadowQuality->insertItem(Surface3DPlotArea::High, i18n("High"));
    ui.cbShadowQuality->insertItem(Surface3DPlotArea::SoftLow, i18n("Soft Low"));
    ui.cbShadowQuality->insertItem(Surface3DPlotArea::SoftMedium, i18n("Soft Medium"));
    ui.cbShadowQuality->insertItem(Surface3DPlotArea::SoftHigh, i18n("Soft High"));

    ui.cbTheme->insertItem(Surface3DPlotArea::Theme::Qt, i18n("Qt"));
    ui.cbTheme->insertItem(Surface3DPlotArea::Theme::PrimaryColors, i18n("Primary Colors"));
    ui.cbTheme->insertItem(Surface3DPlotArea::Theme::StoneMoss, i18n("Stone Moss"));
    ui.cbTheme->insertItem(Surface3DPlotArea::Theme::ArmyBlue, i18n("Army Blue"));
    ui.cbTheme->insertItem(Surface3DPlotArea::Theme::Retro, i18n("Retro"));
    ui.cbTheme->insertItem(Surface3DPlotArea::Theme::Ebony, i18n("Ebony"));
    ui.cbTheme->insertItem(Surface3DPlotArea::Theme::Isabelle, i18n("Isabelle"));
}

//*************************************************************
//**** SLOTs for changes triggered in Surface3DPlotAreaDock ***
//*************************************************************
// Tab "General"

void Surface3DPlotAreaDock::dataSourceTypeChanged(int index) {
    const auto type = static_cast<Surface3DPlotArea::DataSource>(index);
    const bool spreadsheet = (type == Surface3DPlotArea::DataSource::DataSource_Spreadsheet);
    const bool matrix = (type == Surface3DPlotArea::DataSource::DataSource_Matrix);

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

void Surface3DPlotAreaDock::xColumnChanged(const QModelIndex& index) {
    CONDITIONAL_LOCK_RETURN;

    auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
    AbstractColumn* column = nullptr;
    if (aspect) {
        column = dynamic_cast<AbstractColumn*>(aspect);
        Q_ASSERT(column);
    }

    for (auto* curve : m_surfaces)
        curve->setXColumn(column);
}

void Surface3DPlotAreaDock::yColumnChanged(const QModelIndex& index) {
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

void Surface3DPlotAreaDock::zColumnChanged(const QModelIndex& index) {
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

void Surface3DPlotAreaDock::onXRotationChanged(int value) {
    CONDITIONAL_LOCK_RETURN;
    m_surface->setXRotation(value);
}

void Surface3DPlotAreaDock::onYRotationChanged(int value) {
    CONDITIONAL_LOCK_RETURN;
    m_surface->setYRotation(value);
}

void Surface3DPlotAreaDock::onThemeChanged(int value) {
    CONDITIONAL_LOCK_RETURN;
    m_surface->setTheme(static_cast<Surface3DPlotArea::Theme>(value));
}

void Surface3DPlotAreaDock::onZoomLevelChanged(int value) {
    CONDITIONAL_LOCK_RETURN;
    m_surface->setZoomLevel(value);
}

void Surface3DPlotAreaDock::matrixChanged(const QModelIndex& index) {
    CONDITIONAL_LOCK_RETURN;

    auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
    Matrix* matrix = nullptr;
    if (aspect) {
        matrix = dynamic_cast<Matrix*>(aspect);
        Q_ASSERT(matrix);
    }

    for (auto* curve : m_surfaces)
        curve->setMatrix(matrix);
}

void Surface3DPlotAreaDock::updateUiVisibility() {
    const int type = ui.cbDrawType->currentIndex();
    const int dataType = ui.cbDataSourceType->currentIndex();
    if (type == Surface3DPlotArea::DrawMode::DrawSurface || type == Surface3DPlotArea::DrawMode::DrawWireframe) {
        showItem(ui.lDataSourceType, ui.cbDataSourceType, true);
        showItem(ui.lMatrix, ui.cbMatrix, false);
        showTriangleInfo(dataType == Surface3DPlotArea::DataSource_Spreadsheet);
    } else {
        showItem(ui.lDataSourceType, ui.cbDataSourceType, false);
		showTriangleInfo(false);
	}

	Q_EMIT elementVisibilityChanged();
}

// Tab "Mesh"
void Surface3DPlotAreaDock::onDrawModeChanged(int index) {
    CONDITIONAL_LOCK_RETURN;
    for (Surface3DPlotArea* surface : m_surfaces)
        surface->setDrawMode(static_cast<Surface3DPlotArea::DrawMode>(index + 1));
}

void Surface3DPlotAreaDock::onFlatShadingChanged(bool vis) {
    CONDITIONAL_LOCK_RETURN;
    for (Surface3DPlotArea* surface : m_surfaces)
        surface->setFlatShading(vis);
}

void Surface3DPlotAreaDock::zoomChanged(int val) {
    CONDITIONAL_LOCK_RETURN;
    ui.slZoom->setValue(val);
}

void Surface3DPlotAreaDock::xRotationChanged(int val) {
    CONDITIONAL_LOCK_RETURN;
    ui.slXRot->setValue(val);
}
void Surface3DPlotAreaDock::yRotationChanged(int val) {
    CONDITIONAL_LOCK_RETURN;
    ui.slYRot->setValue(val);
}

void Surface3DPlotAreaDock::onShadowQualityChanged(int index) {
    CONDITIONAL_LOCK_RETURN;
    for (Surface3DPlotArea* surface : m_surfaces)
        surface->setShadowQuality(static_cast<Surface3DPlotArea::ShadowQuality>(index));
}

void Surface3DPlotAreaDock::onSmoothChanged(bool vis) {
    CONDITIONAL_LOCK_RETURN;
    for (Surface3DPlotArea* surface : m_surfaces)
        surface->setSmooth(vis);
}

//*************************************************************
//***** SLOTs for changes triggered in Surface3DPlotArea ******
//*************************************************************
// Tab "General"
void Surface3DPlotAreaDock::surfaceMatrixChanged(const Matrix* matrix) {
    CONDITIONAL_LOCK_RETURN;
    ui.cbMatrix->setMatrix(matrix, m_surface->matrixPath());
}

void Surface3DPlotAreaDock::surfaceXColumnChanged(const AbstractColumn* column) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbXColumn->setColumn(column, m_surface->xColumnPath());
}

void Surface3DPlotAreaDock::surfaceYColumnChanged(const AbstractColumn* column) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbYColumn->setColumn(column, m_surface->yColumnPath());
}

void Surface3DPlotAreaDock::surfaceZColumnChanged(const AbstractColumn* column) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbZColumn->setColumn(column, m_surface->zColumnPath());
}

// Tab "Mesh"
void Surface3DPlotAreaDock::drawModeChanged(Surface3DPlotArea::DrawMode mode) {
    CONDITIONAL_LOCK_RETURN;

    ui.cbDrawType->setCurrentIndex(mode - 1);
}

void Surface3DPlotAreaDock::themeChanged(Surface3DPlotArea::Theme theme) {
    CONDITIONAL_LOCK_RETURN;

    ui.cbTheme->setCurrentIndex(theme);
}

void Surface3DPlotAreaDock::flatShadingChanged(bool vis) {
    CONDITIONAL_LOCK_RETURN;

    ui.chkFlatShading->setEnabled(vis);
}

void Surface3DPlotAreaDock::shadowsQualityChanged(Surface3DPlotArea::ShadowQuality quality) {
    CONDITIONAL_LOCK_RETURN;

    ui.cbShadowQuality->setCurrentIndex(quality);
}

void Surface3DPlotAreaDock::sourceTypeChanged(Surface3DPlotArea::DataSource type) {
    CONDITIONAL_LOCK_RETURN;

    ui.cbDataSourceType->setCurrentIndex(type);
}

//*************************************************************
//************************* Settings **************************
//*************************************************************
void Surface3DPlotAreaDock::load() {
	// TODO
}

void Surface3DPlotAreaDock::loadConfig(KConfig& config) {
	// TODO
}
