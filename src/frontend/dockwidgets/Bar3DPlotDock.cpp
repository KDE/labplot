/***************************************************************************
	File                 : Bar3DPlotDock.cpp
	Project              : LabPlot
	Description          : widget for Bar3DPlot properties
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024-2026 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2024 Kuntal Bar <barkuntal6@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
***************************************************************************/

#include "Bar3DPlotDock.h"
#include "TreeViewComboBox.h"
#include "backend/core/AbstractColumn.h"
#include <backend/core/AspectTreeModel.h>

#include <QGridLayout>
#include <QPushButton>

#include <KLocalizedString>

Bar3DPlotDock::Bar3DPlotDock(QWidget* parent)
	: BaseDock(parent) {
	ui.setupUi(this);
	setBaseWidgets(ui.leName, ui.teComment);
	setVisibilityWidgets(ui.chkVisible);
	this->retranslateUi();

	// data-columns
	m_buttonNew = new QPushButton();
	m_buttonNew->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));

	m_gridLayout = new QGridLayout(ui.frameColumns);
	m_gridLayout->setContentsMargins(0, 0, 0, 0);
	m_gridLayout->setHorizontalSpacing(2);
	m_gridLayout->setVerticalSpacing(2);
	ui.frameColumns->setLayout(m_gridLayout);

	// SIGNALs/SLOTs
	connect(m_buttonNew, &QPushButton::clicked, this, &Bar3DPlotDock::addDataColumn);
	connect(ui.kcbColor, &KColorButton::changed, this, &Bar3DPlotDock::colorChanged);
}

void Bar3DPlotDock::setPlots(const QList<Bar3DPlot*>& plots) {
	CONDITIONAL_LOCK_RETURN;
	m_plots = plots;
	m_plot = m_plots.first();
	setAspects(plots);

	// show the properties of the first plot
	loadDataColumns();
	ui.kcbColor->setColor(m_plot->color());

	// Connect to plot signals
	for (auto* plot : m_plots) {
		connect(plot, &Bar3DPlot::dataColumnsChanged, this, &Bar3DPlotDock::plotColumnsChanged);
		connect(plot, &Bar3DPlot::colorChanged, this, &Bar3DPlotDock::plotColorChanged);
	}

	CONDITIONAL_RETURN_NO_LOCK;
	load();
}

void Bar3DPlotDock::retranslateUi() {
	// Translation code if needed
}

void Bar3DPlotDock::addDataColumn() {
	auto* cb = new TreeViewComboBox(ui.frameColumns);
	cb->setTopLevelClasses(TreeViewComboBox::plotColumnTopLevelClasses());
	cb->setModel(aspectModel());
	connect(cb, &TreeViewComboBox::currentModelIndexChanged, this, &Bar3DPlotDock::columnChanged);

	auto* b = new QPushButton(ui.frameColumns);
	b->setIcon(QIcon::fromTheme(QStringLiteral("list-remove")));
	b->setToolTip(i18n("Remove column"));
	connect(b, &QPushButton::clicked, this, &Bar3DPlotDock::removeDataColumn);

	int index = m_dataComboBoxes.size();
	m_gridLayout->addWidget(cb, index, 0);
	m_gridLayout->addWidget(b, index, 1);

	m_dataComboBoxes << cb;
	m_removeButtons << b;

	// Add button at the end
	m_gridLayout->removeWidget(m_buttonNew);
	m_gridLayout->addWidget(m_buttonNew, index + 1, 0);

	setDataColumns();
}

void Bar3DPlotDock::removeDataColumn() {
	auto* sender = static_cast<QPushButton*>(QObject::sender());
	if (!sender)
		return;

	int index = m_removeButtons.indexOf(sender);
	if (index == -1)
		return;

	delete m_dataComboBoxes.takeAt(index);
	delete m_removeButtons.takeAt(index);

	// Rebuild layout
	for (int i = 0; i < m_dataComboBoxes.size(); ++i) {
		m_gridLayout->addWidget(m_dataComboBoxes[i], i, 0);
		m_gridLayout->addWidget(m_removeButtons[i], i, 1);
	}
	m_gridLayout->addWidget(m_buttonNew, m_dataComboBoxes.size(), 0);

	setDataColumns();
}

void Bar3DPlotDock::setDataColumns() {
	CONDITIONAL_LOCK_RETURN;

	QVector<const AbstractColumn*> columns;
	for (auto* cb : m_dataComboBoxes) {
		auto index = cb->currentModelIndex();
		if (index.isValid()) {
			auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
			auto* column = dynamic_cast<AbstractColumn*>(aspect);
			if (column)
				columns << column;
		}
	}

	for (auto* plot : m_plots)
		plot->setDataColumns(columns);
}

void Bar3DPlotDock::loadDataColumns() {
	// Clear existing
	for (auto* cb : m_dataComboBoxes)
		delete cb;
	for (auto* b : m_removeButtons)
		delete b;
	m_dataComboBoxes.clear();
	m_removeButtons.clear();

	auto* model = aspectModel();
	model->enablePlottableColumnsOnly(true);
	model->enableShowPlotDesignation(true);
	model->setSelectableAspects({AspectType::Column});

	// Add existing columns
	auto columns = m_plot->dataColumns();
	for (const auto* column : columns) {
		auto* cb = new TreeViewComboBox(ui.frameColumns);
		cb->setTopLevelClasses(TreeViewComboBox::plotColumnTopLevelClasses());
		cb->setModel(model);
		cb->setAspect(column);
		connect(cb, &TreeViewComboBox::currentModelIndexChanged, this, &Bar3DPlotDock::columnChanged);

		auto* b = new QPushButton(ui.frameColumns);
		b->setIcon(QIcon::fromTheme(QStringLiteral("list-remove")));
		b->setToolTip(i18n("Remove column"));
		connect(b, &QPushButton::clicked, this, &Bar3DPlotDock::removeDataColumn);

		int index = m_dataComboBoxes.size();
		m_gridLayout->addWidget(cb, index, 0);
		m_gridLayout->addWidget(b, index, 1);

		m_dataComboBoxes << cb;
		m_removeButtons << b;
	}

	// Add button at the end
	m_gridLayout->addWidget(m_buttonNew, m_dataComboBoxes.size(), 0);
}

//*************************************************************
//**** SLOTs for changes triggered in Bar3DPlotDock ***
//*************************************************************

void Bar3DPlotDock::columnChanged(const QModelIndex&) {
	setDataColumns();
}

void Bar3DPlotDock::colorChanged(QColor color) {
	CONDITIONAL_LOCK_RETURN;
	for (auto* plot : m_plots)
		plot->setColor(color);
}

//*************************************************************
//******* SLOTs for changes triggered in Bar3DPlot ********
//*************************************************************

void Bar3DPlotDock::plotColumnsChanged(const QVector<const AbstractColumn*>&) {
	CONDITIONAL_LOCK_RETURN;
	loadDataColumns();
}

void Bar3DPlotDock::plotColorChanged(QColor color) {
	CONDITIONAL_LOCK_RETURN;
	ui.kcbColor->setColor(color);
}

//*************************************************************
//************************* Settings **************************
//*************************************************************
void Bar3DPlotDock::load() {
	// Nothing to load from config for now
}

void Bar3DPlotDock::loadConfig(KConfig& config) {
	Q_UNUSED(config)
	// Future: load default plot settings
}
