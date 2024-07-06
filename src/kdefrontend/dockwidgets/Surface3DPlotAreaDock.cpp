/*
	File                 : Surface3DPlotAreaDock.cpp
	Project              : LabPlot
	Description          : widget for Surface3DPlotArea properties
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "Surface3DPlotAreaDock.h"
#include "TreeViewComboBox.h"
#include <backend/core/AspectTreeModel.h>
// #include <kdefrontend/TemplateHandler.h>

Surface3DPlotAreaDock::Surface3DPlotAreaDock(QWidget* parent)
	: BaseDock(parent) {
	ui.setupUi(this);
	setBaseWidgets(ui.leName, ui.teComment);
	setVisibilityWidgets(ui.chkVisible);
	this->retranslateUi();

	const QVector<TreeViewComboBox*> treeViews(QVector<TreeViewComboBox*>()
												<< ui.cbXColumn << ui.cbYColumn << ui.cbZColumn << ui.cbNode1 << ui.cbNode2 << ui.cbNode3);

	for (auto* view : treeViews)
		view->setTopLevelClasses(TreeViewComboBox::plotColumnTopLevelClasses());

	// Matrix data source
	ui.cbMatrix->setTopLevelClasses({AspectType::Folder, AspectType::Workbook, AspectType::Matrix});
	//TODO  ui.cbMatrix->setSelectableAspects({AspectType::Matrix});

	// SIGNALs/SLOTs
	// General
	connect(ui.cbDataSourceType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Surface3DPlotAreaDock::dataSourceTypeChanged);
	connect(ui.cbXColumn, &TreeViewComboBox::currentModelIndexChanged, this, &Surface3DPlotAreaDock::xColumnChanged);
	connect(ui.cbYColumn, &TreeViewComboBox::currentModelIndexChanged, this, &Surface3DPlotAreaDock::yColumnChanged);
	connect(ui.cbZColumn, &TreeViewComboBox::currentModelIndexChanged, this, &Surface3DPlotAreaDock::zColumnChanged);
	connect(ui.cbMatrix, &TreeViewComboBox::currentModelIndexChanged, this, &Surface3DPlotAreaDock::matrixChanged);

	// Mesh
	connect(ui.cbMeshType, SIGNAL(currentIndexChanged(int)), SLOT(onMeshTypeChanged(int)));
	connect(ui.cbDrawType, SIGNAL(currentIndexChanged(int)), SLOT(onDrawModeChanged(int)));
	connect(ui.chkFlatShading, SIGNAL(toggled(bool)), SLOT(onFlatShadingChanged(bool)));
	connect(ui.chkGridVisible, SIGNAL(toggled(bool)), SLOT(onGridVisibleChanged(bool)));
	connect(ui.cbShadowQuality, SIGNAL(currentIndexChanged(int)), SLOT(onShadowQualityChanged(int)));
}

void Surface3DPlotAreaDock::setSurfaces(const QList<Surface3DPlotArea*>& surfaces) {
	m_surfaces = surfaces;
	m_surface = m_surfaces.first();
	setAspects(surfaces);
	auto* model = aspectModel();

	model->enablePlottableColumnsOnly(true);
	model->enableShowPlotDesignation(true);
	model->setSelectableAspects({AspectType::Column});

	ui.cbXColumn->setModel(model);
	ui.cbYColumn->setModel(model);
	ui.cbZColumn->setModel(model);
	ui.cbMatrix->setModel(model);

	ui.cbNode1->setModel(model);
	ui.cbNode2->setModel(model);
	ui.cbNode3->setModel(model);

	// show the properties of the first surface
	// tab "General"
	ui.cbDataSourceType->setCurrentIndex(static_cast<int>(m_surface->dataSource()));
	dataSourceTypeChanged(ui.cbDataSourceType->currentIndex());
	ui.cbXColumn->setColumn(m_surface->xColumn(), m_surface->xColumnPath());
	ui.cbYColumn->setColumn(m_surface->yColumn(), m_surface->yColumnPath());
	ui.cbYColumn->setColumn(m_surface->zColumn(), m_surface->zColumnPath());
	// TODO: matrix

	// tab "Mesh"
	meshTypeChanged(m_surface->meshType());
	drawModeChanged(m_surface->drawMode());
	sourceTypeChanged(m_surface->dataSource());

	firstNodeChanged(m_surface->firstNode());
	secondNodeChanged(m_surface->secondNode());
	thirdNodeChanged(m_surface->thirdNode());

	connect(m_surface, &Surface3DPlotArea::drawModeChanged, this, &Surface3DPlotAreaDock::drawModeChanged);
	connect(m_surface, &Surface3DPlotArea::sourceTypeChanged, this, &Surface3DPlotAreaDock::sourceTypeChanged);
	connect(m_surface, &Surface3DPlotArea::meshTypeChanged, this, &Surface3DPlotAreaDock::meshTypeChanged);
	connect(m_surface, &Surface3DPlotArea::flatShadingChanged, this, &Surface3DPlotAreaDock::flatShadingChanged);
	connect(m_surface, &Surface3DPlotArea::gridVisibilityChanged, this, &Surface3DPlotAreaDock::gridVisibleChanged);
	connect(m_surface, &Surface3DPlotArea::shadowQualityChanged, this, &Surface3DPlotAreaDock::shadowsQualityChanged);
	connect(m_surface, &Surface3DPlotArea::smoothChanged, this, &Surface3DPlotAreaDock::onSmoothChanged);

	connect(m_surface, &Surface3DPlotArea::matrixChanged, this, &Surface3DPlotAreaDock::surfaceMatrixChanged);
	connect(m_surface, &Surface3DPlotArea::xColumnChanged, this, &Surface3DPlotAreaDock::surfaceXColumnChanged);
	connect(m_surface, &Surface3DPlotArea::yColumnChanged, this, &Surface3DPlotAreaDock::surfaceYColumnChanged);
	connect(m_surface, &Surface3DPlotArea::zColumnChanged, this, &Surface3DPlotAreaDock::surfaceZColumnChanged);
	connect(m_surface, &Surface3DPlotArea::firstNodeChanged, this, &Surface3DPlotAreaDock::firstNodeChanged);
	connect(m_surface, &Surface3DPlotArea::secondNodeChanged, this, &Surface3DPlotAreaDock::secondNodeChanged);
	connect(m_surface, &Surface3DPlotArea::thirdNodeChanged, this, &Surface3DPlotAreaDock::thirdNodeChanged);

	updateUiVisibility();
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
	showItem(ui.labelNode1, ui.cbNode1, pred);
	showItem(ui.labelNode2, ui.cbNode2, pred);
	showItem(ui.labelNode3, ui.cbNode3, pred);
	ui.labelNodeHeader->setVisible(pred);

	Q_EMIT elementVisibilityChanged();
}

void Surface3DPlotAreaDock::retranslateUi() {
	ui.cbDataSourceType->insertItem(Surface3DPlotArea::DataSource_Spreadsheet, i18n("Spreadsheet"));
	ui.cbDataSourceType->insertItem(Surface3DPlotArea::DataSource_Matrix, i18n("Matrix"));
	ui.cbDataSourceType->insertItem(Surface3DPlotArea::DataSource_Empty, i18n("Demo"));

	ui.cbDrawType->insertItem(Surface3DPlotArea::DrawSurface, i18n("Surface"));
	ui.cbDrawType->insertItem(Surface3DPlotArea::DrawWireframe, i18n("Wireframe"));

	// Mesh Type options
	ui.cbMeshType->insertItem(Surface3DPlotArea::UserDefined, i18n("User Defined"));
	ui.cbMeshType->insertItem(Surface3DPlotArea::Bar, i18n("Bar"));
	ui.cbMeshType->insertItem(Surface3DPlotArea::Cube, i18n("Cube"));
	ui.cbMeshType->insertItem(Surface3DPlotArea::Pyramid, i18n("Pyramid"));
	ui.cbMeshType->insertItem(Surface3DPlotArea::Cone, i18n("Cone"));
	ui.cbMeshType->insertItem(Surface3DPlotArea::Cylinder, i18n("Cylinder"));
	ui.cbMeshType->insertItem(Surface3DPlotArea::BevelBar, i18n("Bevel Bar"));
	ui.cbMeshType->insertItem(Surface3DPlotArea::BevelCube, i18n("Bevel Cube"));
	ui.cbMeshType->insertItem(Surface3DPlotArea::Sphere, i18n("Sphere"));
	ui.cbMeshType->insertItem(Surface3DPlotArea::Minimal, i18n("Minimal"));
	ui.cbMeshType->insertItem(Surface3DPlotArea::Arrow, i18n("Arrow"));
	ui.cbMeshType->insertItem(Surface3DPlotArea::Point, i18n("Point"));

	ui.cbShadowQuality->insertItem(Surface3DPlotArea::None, i18n("None"));
	ui.cbShadowQuality->insertItem(Surface3DPlotArea::Low, i18n("Low"));
	ui.cbShadowQuality->insertItem(Surface3DPlotArea::Medium, i18n("Medium"));
	ui.cbShadowQuality->insertItem(Surface3DPlotArea::High, i18n("High"));
	ui.cbShadowQuality->insertItem(Surface3DPlotArea::SoftLow, i18n("Soft Low"));
	ui.cbShadowQuality->insertItem(Surface3DPlotArea::SoftMedium, i18n("Soft Medium"));
	ui.cbShadowQuality->insertItem(Surface3DPlotArea::SoftHigh, i18n("Soft High"));
}

const AbstractColumn* Surface3DPlotAreaDock::getColumn(const QModelIndex& index) const {
	if (!index.isValid())
		return nullptr;

	// Assuming the model is an AspectTreeModel or compatible
	auto* aspect = static_cast<AbstractColumn*>(index.internalPointer());
	return dynamic_cast<const AbstractColumn*>(aspect);
}

const Matrix* Surface3DPlotAreaDock::getMatrix(const QModelIndex& index) const {
	if (!index.isValid())
		return nullptr;

	// Assuming the model is an AspectTreeModel or compatible
	auto* aspect = static_cast<Matrix*>(index.internalPointer());
	return dynamic_cast<const Matrix*>(aspect);
}

//*************************************************************
//**** SLOTs for changes triggered in Surface3DPlotAreaDock ***
//*************************************************************
// Tab "General"
void Surface3DPlotAreaDock::onTreeViewIndexChanged(const QModelIndex& index) {
	const AbstractColumn* column = getColumn(index);

	QObject* senderW = sender();
	const Lock lock(m_initializing);
	for (Surface3DPlotArea* surface : m_surfaces) {
		if (senderW == ui.cbXColumn)
			surface->setXColumn(column);
		else if (senderW == ui.cbYColumn)
			surface->setYColumn(column);
		else if (senderW == ui.cbZColumn)
			surface->setZColumn(column);
		else if (senderW == ui.cbNode1)
			surface->setFirstNode(column);
		else if (senderW == ui.cbNode2)
			surface->setSecondNode(column);
		else if (senderW == ui.cbNode3)
			surface->setThirdNode(column);
		else if (senderW == ui.cbMatrix)
			surface->setMatrix(getMatrix(index));
	}
}

void Surface3DPlotAreaDock::dataSourceTypeChanged(int index) {
	const auto type = static_cast<Surface3DPlotArea::DataSource>(index);
	const bool spreadsheet = (type == Surface3DPlotArea::DataSource::DataSource_Spreadsheet);

	ui.lXColumn->setVisible(spreadsheet);
	ui.cbXColumn->setVisible(spreadsheet);

	ui.lYColumn->setVisible(spreadsheet);
	ui.cbYColumn->setVisible(spreadsheet);

	ui.lZColumn->setVisible(spreadsheet);
	ui.cbZColumn->setVisible(spreadsheet);

	ui.lMatrix->setVisible(!spreadsheet);
	ui.cbMatrix->setVisible(!spreadsheet);

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

void Surface3DPlotAreaDock::matrixChanged(const QModelIndex& index) {

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
	{
		const Lock lock(m_initializing);
		for (Surface3DPlotArea* surface : m_surfaces)
			surface->setDrawMode(static_cast<Surface3DPlotArea::DrawMode>(index));
	}

    updateUiVisibility();
}

void Surface3DPlotAreaDock::onFlatShadingChanged(bool vis) {
    qDebug() << Q_FUNC_INFO;
	{
		const Lock lock(m_initializing);
		for (Surface3DPlotArea* surface : m_surfaces)
			surface->setFlatShading(vis);
	}
}

void Surface3DPlotAreaDock::onGridVisibleChanged(bool vis) {
	{
		const Lock lock(m_initializing);
		for (Surface3DPlotArea* surface : m_surfaces)
			surface->setGridVisibility(vis);
	}
}

void Surface3DPlotAreaDock::onShadowQualityChanged(int index) {
	{
		const Lock lock(m_initializing);
		for (Surface3DPlotArea* surface : m_surfaces)
			surface->setShadowQuality(static_cast<Surface3DPlotArea::ShadowQuality>(index));
	}
}

void Surface3DPlotAreaDock::onSmoothChanged(bool vis) {
	{
		const Lock lock(m_initializing);
		for (Surface3DPlotArea* surface : m_surfaces)
			surface->setSmooth(vis);
	}
}

//*************************************************************
//***** SLOTs for changes triggered in Surface3DPlotArea ******
//*************************************************************
// Tab "General"
void Surface3DPlotAreaDock::surfaceMatrixChanged(const Matrix* matrix) {
	setModelFromAspect(ui.cbMatrix, matrix);
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
	if (m_initializing)
		return;
	ui.cbDrawType->setCurrentIndex(mode);
    updateUiVisibility();
}

void Surface3DPlotAreaDock::meshTypeChanged(Surface3DPlotArea::MeshType type) {
	if (m_initializing)
		return;
	ui.cbMeshType->setCurrentIndex(type);
}

void Surface3DPlotAreaDock::flatShadingChanged(bool vis) {
	if (m_initializing)
		return;
    ui.chkFlatShading->setEnabled(vis);
}

void Surface3DPlotAreaDock::gridVisibleChanged(bool vis) {
	if (m_initializing)
		return;
	ui.chkGridVisible->setEnabled(vis);
}

void Surface3DPlotAreaDock::shadowsQualityChanged(Surface3DPlotArea::ShadowQuality quality) {
	if (m_initializing)
		return;
	ui.cbShadowQuality->setCurrentIndex(quality);
}

void Surface3DPlotAreaDock::sourceTypeChanged(Surface3DPlotArea::DataSource type) {
    qDebug() << Q_FUNC_INFO;
	if (m_initializing)
		return;
	ui.cbDataSourceType->setCurrentIndex(type);
}

QModelIndex Surface3DPlotAreaDock::modelIndexOfAspect(AspectTreeModel* model, const AbstractAspect* aspect) const {
	if (!model || !aspect)
		return QModelIndex();

	const QList<QModelIndex> indexes = model->match(model->index(0, 0), Qt::DisplayRole, QVariant::fromValue(aspect), 1, Qt::MatchExactly | Qt::MatchRecursive);

	return indexes.isEmpty() ? QModelIndex() : indexes.first();
}

void Surface3DPlotAreaDock::setModelFromAspect(TreeViewComboBox* cb, const AbstractAspect* aspect) {
	if (m_initializing)
		return;
	// cb->setCurrentModelIndex(modelIndexOfAspect(aspectTreeModel, aspect));
}

void Surface3DPlotAreaDock::firstNodeChanged(const AbstractColumn* column) {
	setModelFromAspect(ui.cbNode1, column);
}

void Surface3DPlotAreaDock::secondNodeChanged(const AbstractColumn* column) {
	setModelFromAspect(ui.cbNode2, column);
}

void Surface3DPlotAreaDock::thirdNodeChanged(const AbstractColumn* column) {
	setModelFromAspect(ui.cbNode3, column);
}

void Surface3DPlotAreaDock::onMeshTypeChanged(int index) {
    {
        const Lock lock(m_initializing);
        for (Surface3DPlotArea* surface : m_surfaces)
            surface->setMeshType(static_cast<Surface3DPlotArea::MeshType>(index));
    }
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
