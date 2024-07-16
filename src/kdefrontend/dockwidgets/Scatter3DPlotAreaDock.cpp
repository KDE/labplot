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

	connect(m_scatter, &Scatter3DPlotArea::xColumnChanged, this, &Scatter3DPlotAreaDock::scatterXColumnChanged);
	connect(m_scatter, &Scatter3DPlotArea::yColumnChanged, this, &Scatter3DPlotAreaDock::scatterYColumnChanged);
	connect(m_scatter, &Scatter3DPlotArea::zColumnChanged, this, &Scatter3DPlotAreaDock::scatterZColumnChanged);
}

void Scatter3DPlotAreaDock::retranslateUi() {
	// This function should contain translation code if needed
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

//*************************************************************
//************************* Settings **************************
//*************************************************************
void Scatter3DPlotAreaDock::load() {
	// TODO
}

void Scatter3DPlotAreaDock::loadConfig(KConfig& config) {
	// TODO
}
