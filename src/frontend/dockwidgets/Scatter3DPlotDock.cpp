/***************************************************************************
	File                 : Scatter3DPlotDock.cpp
	Project              : LabPlot
	Description          : widget for Scatter3DPlot properties
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024-2026 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2024 Kuntal Bar <barkuntal6@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
***************************************************************************/

#include "Scatter3DPlotDock.h"
#include "TreeViewComboBox.h"
#include "backend/core/AbstractColumn.h"
#include <backend/core/AspectTreeModel.h>

#include <KLocalizedString>

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
	connect(ui.cbXCoordinate, &TreeViewComboBox::currentModelIndexChanged, this, &Scatter3DPlotDock::xColumnChanged);
	connect(ui.cbYCoordinate, &TreeViewComboBox::currentModelIndexChanged, this, &Scatter3DPlotDock::yColumnChanged);
	connect(ui.cbZCoordinate, &TreeViewComboBox::currentModelIndexChanged, this, &Scatter3DPlotDock::zColumnChanged);
	connect(ui.cbPointStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Scatter3DPlotDock::pointStyleChanged);
	connect(ui.kcbColor, &KColorButton::changed, this, &Scatter3DPlotDock::colorChanged);
}

void Scatter3DPlotDock::setPlots(const QList<Scatter3DPlot*>& plots) {
	CONDITIONAL_LOCK_RETURN;
	m_plots = plots;
	m_plot = m_plots.first();
	setAspects(plots);
	auto* model = aspectModel();

	model->enablePlottableColumnsOnly(true);
	model->enableShowPlotDesignation(true);
	model->setSelectableAspects({AspectType::Column});

	ui.cbXCoordinate->setModel(model);
	ui.cbYCoordinate->setModel(model);
	ui.cbZCoordinate->setModel(model);

	// show the properties of the first plot
	ui.cbXCoordinate->setAspect(m_plot->xColumn());
	ui.cbYCoordinate->setAspect(m_plot->yColumn());
	ui.cbZCoordinate->setAspect(m_plot->zColumn());
	ui.cbPointStyle->setCurrentIndex(static_cast<int>(m_plot->pointStyle()));
	ui.kcbColor->setColor(m_plot->color());

	// Connect to plot signals
	for (auto* plot : m_plots) {
		connect(plot, &Scatter3DPlot::xColumnChanged, this, &Scatter3DPlotDock::plotXColumnChanged);
		connect(plot, &Scatter3DPlot::yColumnChanged, this, &Scatter3DPlotDock::plotYColumnChanged);
		connect(plot, &Scatter3DPlot::zColumnChanged, this, &Scatter3DPlotDock::plotZColumnChanged);
		connect(plot, &Scatter3DPlot::pointStyleChanged, this, &Scatter3DPlotDock::plotPointStyleChanged);
		connect(plot, &Scatter3DPlot::colorChanged, this, &Scatter3DPlotDock::plotColorChanged);
	}

	CONDITIONAL_RETURN_NO_LOCK;
	load();
}

void Scatter3DPlotDock::retranslateUi() {
	CONDITIONAL_LOCK_RETURN;

	ui.cbPointStyle->clear();
	ui.cbPointStyle->addItem(i18n("Sphere"));
	ui.cbPointStyle->addItem(i18n("Cube"));
	ui.cbPointStyle->addItem(i18n("Cone"));
	ui.cbPointStyle->addItem(i18n("Pyramid"));
}

//*************************************************************
//**** SLOTs for changes triggered in Scatter3DPlotDock ***
//*************************************************************

void Scatter3DPlotDock::xColumnChanged(const QModelIndex& index) {
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

void Scatter3DPlotDock::yColumnChanged(const QModelIndex& index) {
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

void Scatter3DPlotDock::zColumnChanged(const QModelIndex& index) {
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

void Scatter3DPlotDock::pointStyleChanged(int index) {
	CONDITIONAL_LOCK_RETURN;
	for (auto* plot : m_plots)
		plot->setPointStyle(static_cast<Scatter3DPlot::PointStyle>(index));
}

void Scatter3DPlotDock::colorChanged(QColor color) {
	CONDITIONAL_LOCK_RETURN;
	for (auto* plot : m_plots)
		plot->setColor(color);
}

//*************************************************************
//******* SLOTs for changes triggered in Scatter3DPlot ********
//*************************************************************

void Scatter3DPlotDock::plotXColumnChanged(const AbstractColumn* column) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbXCoordinate->setAspect(column);
}

void Scatter3DPlotDock::plotYColumnChanged(const AbstractColumn* column) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbYCoordinate->setAspect(column);
}

void Scatter3DPlotDock::plotZColumnChanged(const AbstractColumn* column) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbZCoordinate->setAspect(column);
}

void Scatter3DPlotDock::plotPointStyleChanged(Scatter3DPlot::PointStyle style) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbPointStyle->setCurrentIndex(static_cast<int>(style));
}

void Scatter3DPlotDock::plotColorChanged(QColor color) {
	CONDITIONAL_LOCK_RETURN;
	ui.kcbColor->setColor(color);
}

//*************************************************************
//************************* Settings **************************
//*************************************************************
void Scatter3DPlotDock::load() {
	// Nothing to load from config for now
}

void Scatter3DPlotDock::loadConfig(KConfig& config) {
	Q_UNUSED(config)
	// Future: load default plot settings
}
