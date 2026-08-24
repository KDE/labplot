/***************************************************************************
	File                 : Surface3DPlotDock.cpp
	Project              : LabPlot
	Description          : widget for Surface3DPlot properties
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

#include <KLocalizedString>

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
	connect(ui.cbDrawType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Surface3DPlotDock::drawModeChanged);
	connect(ui.chkFlatShading, &QCheckBox::toggled, this, &Surface3DPlotDock::flatShadingChanged);
	connect(ui.chkSmooth, &QCheckBox::toggled, this, &Surface3DPlotDock::smoothChanged);
	connect(ui.kcbColor, &KColorButton::changed, this, &Surface3DPlotDock::colorChanged);
}

void Surface3DPlotDock::setPlots(const QList<Surface3DPlot*>& plots) {
	CONDITIONAL_LOCK_RETURN;
	m_plots = plots;
	m_plot = m_plots.first();
	setAspects(plots);
	auto* model = aspectModel();

	model->enablePlottableColumnsOnly(true);
	model->enableShowPlotDesignation(true);
	if (m_plot->dataSource() == Surface3DPlot::DataSource_Spreadsheet)
		model->setSelectableAspects({AspectType::Column});

	ui.cbXColumn->setModel(model);
	ui.cbYColumn->setModel(model);
	ui.cbZColumn->setModel(model);
	if (m_plot->dataSource() == Surface3DPlot::DataSource_Matrix)
		model->setSelectableAspects({AspectType::Matrix});

	ui.cbMatrix->setModel(model);

	// show the properties of the first plot
	// tab "General"
	ui.cbDataSourceType->setCurrentIndex(static_cast<int>(m_plot->dataSource()));
	ui.cbXColumn->setAspect(m_plot->xColumn());
	ui.cbYColumn->setAspect(m_plot->yColumn());
	ui.cbZColumn->setAspect(m_plot->zColumn());
	ui.cbMatrix->setAspect(m_plot->matrix());

	// tab "Mesh"
	ui.cbDrawType->setCurrentIndex(static_cast<int>(m_plot->drawMode()) - 1);
	ui.chkFlatShading->setChecked(m_plot->flatShading());
	ui.chkSmooth->setChecked(m_plot->smooth());
	ui.kcbColor->setColor(m_plot->color());

	dataSourceTypeChanged(m_plot->dataSource());

	// Connect to plot signals
	for (auto* plot : m_plots) {
		connect(plot, &Surface3DPlot::drawModeChanged, this, &Surface3DPlotDock::plotDrawModeChanged);
		connect(plot, &Surface3DPlot::sourceTypeChanged, this, &Surface3DPlotDock::plotSourceTypeChanged);
		connect(plot, &Surface3DPlot::flatShadingChanged, this, &Surface3DPlotDock::plotFlatShadingChanged);
		connect(plot, &Surface3DPlot::smoothChanged, this, &Surface3DPlotDock::plotSmoothChanged);
		connect(plot, &Surface3DPlot::colorChanged, this, &Surface3DPlotDock::plotColorChanged);
		connect(plot, &Surface3DPlot::matrixChanged, this, &Surface3DPlotDock::plotMatrixChanged);
		connect(plot, &Surface3DPlot::xColumnChanged, this, &Surface3DPlotDock::plotXColumnChanged);
		connect(plot, &Surface3DPlot::yColumnChanged, this, &Surface3DPlotDock::plotYColumnChanged);
		connect(plot, &Surface3DPlot::zColumnChanged, this, &Surface3DPlotDock::plotZColumnChanged);
	}

	CONDITIONAL_RETURN_NO_LOCK;
	load();
}

void Surface3DPlotDock::retranslateUi() {
	CONDITIONAL_LOCK_RETURN;

	ui.cbDataSourceType->clear();
	ui.cbDataSourceType->addItem(i18n("Spreadsheet"));
	ui.cbDataSourceType->addItem(i18n("Matrix"));
	ui.cbDataSourceType->addItem(i18n("Demo"));

	ui.cbDrawType->clear();
	ui.cbDrawType->addItem(i18n("Wireframe"));
	ui.cbDrawType->addItem(i18n("Surface"));
	ui.cbDrawType->addItem(i18n("Wireframe & Surface"));
}

//*************************************************************
//**** SLOTs for changes triggered in Surface3DPlotDock ***
//*************************************************************
// Tab "General"

void Surface3DPlotDock::dataSourceTypeChanged(int index) {
	const auto type = static_cast<Surface3DPlot::DataSource>(index);
	const bool spreadsheet = (type == Surface3DPlot::DataSource_Spreadsheet);
	const bool matrix = (type == Surface3DPlot::DataSource_Matrix);

	ui.lXColumn->setVisible(spreadsheet);
	ui.cbXColumn->setVisible(spreadsheet);

	ui.lYColumn->setVisible(spreadsheet);
	ui.cbYColumn->setVisible(spreadsheet);

	ui.lZColumn->setVisible(spreadsheet);
	ui.cbZColumn->setVisible(spreadsheet);

	ui.lMatrix->setVisible(matrix);
	ui.cbMatrix->setVisible(matrix);

	CONDITIONAL_LOCK_RETURN;

	for (auto* plot : m_plots)
		plot->setDataSource(type);
}

void Surface3DPlotDock::xColumnChanged(const QModelIndex& index) {
	CONDITIONAL_LOCK_RETURN;

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	AbstractColumn* column = nullptr;
	if (aspect) {
		column = dynamic_cast<AbstractColumn*>(aspect);
		Q_ASSERT(column);
	}

	for (auto* plot : m_plots)
		plot->setXColumn(column);
}

void Surface3DPlotDock::yColumnChanged(const QModelIndex& index) {
	CONDITIONAL_LOCK_RETURN;

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	AbstractColumn* column = nullptr;
	if (aspect) {
		column = dynamic_cast<AbstractColumn*>(aspect);
		Q_ASSERT(column);
	}

	for (auto* plot : m_plots)
		plot->setYColumn(column);
}

void Surface3DPlotDock::zColumnChanged(const QModelIndex& index) {
	CONDITIONAL_LOCK_RETURN;

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	AbstractColumn* column = nullptr;
	if (aspect) {
		column = dynamic_cast<AbstractColumn*>(aspect);
		Q_ASSERT(column);
	}

	for (auto* plot : m_plots)
		plot->setZColumn(column);
}

void Surface3DPlotDock::matrixChanged(const QModelIndex& index) {
	CONDITIONAL_LOCK_RETURN;

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	Matrix* matrix = nullptr;
	if (aspect) {
		matrix = dynamic_cast<Matrix*>(aspect);
		Q_ASSERT(matrix);
	}

	for (auto* plot : m_plots)
		plot->setMatrix(matrix);
}

void Surface3DPlotDock::updateUiVisibility() {
	const int dataType = ui.cbDataSourceType->currentIndex();
	const bool spreadsheet = (dataType == Surface3DPlot::DataSource_Spreadsheet);
	const bool matrix = (dataType == Surface3DPlot::DataSource_Matrix);

	ui.lXColumn->setVisible(spreadsheet);
	ui.cbXColumn->setVisible(spreadsheet);
	ui.lYColumn->setVisible(spreadsheet);
	ui.cbYColumn->setVisible(spreadsheet);
	ui.lZColumn->setVisible(spreadsheet);
	ui.cbZColumn->setVisible(spreadsheet);

	ui.lMatrix->setVisible(matrix);
	ui.cbMatrix->setVisible(matrix);
}

// Tab "Mesh"
void Surface3DPlotDock::drawModeChanged(int index) {
	CONDITIONAL_LOCK_RETURN;
	for (auto* plot : m_plots)
		plot->setDrawMode(static_cast<Surface3DPlot::DrawMode>(index + 1));
}

void Surface3DPlotDock::flatShadingChanged(bool checked) {
	CONDITIONAL_LOCK_RETURN;
	for (auto* plot : m_plots)
		plot->setFlatShading(checked);
}

void Surface3DPlotDock::smoothChanged(bool checked) {
	CONDITIONAL_LOCK_RETURN;
	for (auto* plot : m_plots)
		plot->setSmooth(checked);
}

void Surface3DPlotDock::colorChanged(QColor color) {
	CONDITIONAL_LOCK_RETURN;
	for (auto* plot : m_plots)
		plot->setColor(color);
}

//*************************************************************
//******* SLOTs for changes triggered in Surface3DPlot ********
//*************************************************************
// Tab "General"
void Surface3DPlotDock::plotMatrixChanged(const Matrix* matrix) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbMatrix->setAspect(matrix);
}

void Surface3DPlotDock::plotXColumnChanged(const AbstractColumn* column) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbXColumn->setAspect(column);
}

void Surface3DPlotDock::plotYColumnChanged(const AbstractColumn* column) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbYColumn->setAspect(column);
}

void Surface3DPlotDock::plotZColumnChanged(const AbstractColumn* column) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbZColumn->setAspect(column);
}

// Tab "Mesh"
void Surface3DPlotDock::plotDrawModeChanged(Surface3DPlot::DrawMode mode) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbDrawType->setCurrentIndex(static_cast<int>(mode) - 1);
}

void Surface3DPlotDock::plotFlatShadingChanged(bool checked) {
	CONDITIONAL_LOCK_RETURN;
	ui.chkFlatShading->setChecked(checked);
}

void Surface3DPlotDock::plotSourceTypeChanged(Surface3DPlot::DataSource type) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbDataSourceType->setCurrentIndex(static_cast<int>(type));
}

void Surface3DPlotDock::plotSmoothChanged(bool checked) {
	CONDITIONAL_LOCK_RETURN;
	ui.chkSmooth->setChecked(checked);
}

void Surface3DPlotDock::plotColorChanged(QColor color) {
	CONDITIONAL_LOCK_RETURN;
	ui.kcbColor->setColor(color);
}

//*************************************************************
//************************* Settings **************************
//*************************************************************
void Surface3DPlotDock::load() {
	// Nothing to load from config for now
}

void Surface3DPlotDock::loadConfig(KConfig& config) {
	Q_UNUSED(config)
	// Future: load default plot settings
}
