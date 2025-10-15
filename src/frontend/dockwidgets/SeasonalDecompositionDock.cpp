/*
	File                 : SeasonalDecompositionDock.cpp
	Project              : LabPlot
	Description          : widget for properties of a time series seasonal decomposition
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2025 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "SeasonalDecompositionDock.h"
#include "backend/core/column/Column.h"
#include "frontend/TemplateHandler.h"
#include "frontend/widgets/TreeViewComboBox.h"

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KMessageWidget>

/*!
  \class SeasonalDecompositionDock
  \brief

  \ingroup frontend
*/
SeasonalDecompositionDock::SeasonalDecompositionDock(QWidget* parent)
	: BaseDock(parent) {
	ui.setupUi(this);
	setBaseWidgets(ui.leName, ui.teComment);

	// Tab "General"
	cbXColumn = new TreeViewComboBox();
	ui.gridLayout->addWidget(cbXColumn, 4, 2, 1, 1);

	cbYColumn = new TreeViewComboBox();
	ui.gridLayout->addWidget(cbYColumn, 5, 2, 1, 1);

	retranslateUi();

	// Slots
	// General
	connect(cbXColumn, &TreeViewComboBox::currentModelIndexChanged, this, &SeasonalDecompositionDock::xColumnChanged);
	connect(cbYColumn, &TreeViewComboBox::currentModelIndexChanged, this, &SeasonalDecompositionDock::yColumnChanged);
	connect(ui.cbMethod, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SeasonalDecompositionDock::methodChanged);

	// template handler
	auto* frame = new QFrame(this);
	auto* layout = new QHBoxLayout(frame);
	layout->setContentsMargins(0, 0, 0, 0);

	auto* templateHandler = new TemplateHandler(this, QLatin1String("SeasonalDecomposition"));
	layout->addWidget(templateHandler);
	connect(templateHandler, &TemplateHandler::loadConfigRequested, this, &SeasonalDecompositionDock::loadConfigFromTemplate);
	connect(templateHandler, &TemplateHandler::saveConfigRequested, this, &SeasonalDecompositionDock::saveConfigAsTemplate);
	connect(templateHandler, &TemplateHandler::info, this, &SeasonalDecompositionDock::info);

	ui.gridLayout->addWidget(frame, 12, 0, 1, 3);
}

SeasonalDecompositionDock::~SeasonalDecompositionDock() = default;

void SeasonalDecompositionDock::setModel() {
	auto* model = aspectModel();
	model->enablePlottableColumnsOnly(true);
	model->enableShowPlotDesignation(true);
	model->setSelectableAspects({AspectType::Column});
	cbXColumn->setTopLevelClasses(TreeViewComboBox::plotColumnTopLevelClasses());
	cbXColumn->setModel(model);
	cbYColumn->setTopLevelClasses(TreeViewComboBox::plotColumnTopLevelClasses());
	cbYColumn->setModel(model);
}

void SeasonalDecompositionDock::setDecompositions(QList<SeasonalDecomposition*> list) {
	CONDITIONAL_LOCK_RETURN;
	m_decompositions = list;
	m_decomposition = list.first();
	setAspects(list);
	Q_ASSERT(m_decomposition);
	setModel();

	// show the properties of the first curve
	// if there are more than one curve in the list, disable the content in the tab "general"
	if (m_decompositions.size() == 1) {
		cbXColumn->setEnabled(true);
		cbXColumn->setAspect(m_decomposition->xColumn(), m_decomposition->xColumnPath());
		cbYColumn->setEnabled(true);
		cbYColumn->setAspect(m_decomposition->yColumn(), m_decomposition->yColumnPath());
	} else {
		cbXColumn->setEnabled(false);
		cbXColumn->setCurrentModelIndex(QModelIndex());
		cbYColumn->setEnabled(false);
		cbYColumn->setCurrentModelIndex(QModelIndex());
	}

	load(); // load the remaining properties

	// Slots
	// General-tab
	connect(m_decomposition, &SeasonalDecomposition::xColumnChanged, this, &SeasonalDecompositionDock::decompositionXColumnChanged);
	connect(m_decomposition, &SeasonalDecomposition::yColumnChanged, this, &SeasonalDecompositionDock::decompositionYColumnChanged);
	connect(m_decomposition, &SeasonalDecomposition::methodChanged, this, &SeasonalDecompositionDock::decompositionMethodChanged);
 }

void SeasonalDecompositionDock::retranslateUi() {
	CONDITIONAL_LOCK_RETURN;

	// method
	ui.cbMethod->clear();
	ui.cbMethod->addItem(i18n("Loess"), static_cast<int>(SeasonalDecomposition::Method::LOESS));

	// tooltip texts
	QString info = i18n(
		"Method used to perform the seasonal-trend decomposition::"
		"<ul>"
		"<li>LOESS - Locally Estimated Scatterplot Smoothing</li>"
		"</ul>");
	ui.lMethod->setToolTip(info);
	ui.cbMethod->setToolTip(info);
}

//*************************************************************
//* SLOTs for changes triggered in SeasonalDecompositionDock **
//*************************************************************

// "General"-tab
void SeasonalDecompositionDock::methodChanged(int) {
	CONDITIONAL_LOCK_RETURN;

	const auto method = static_cast<SeasonalDecomposition::Method>(ui.cbMethod->currentData().toInt());
	for (auto* decomposition : m_decompositions)
		decomposition->setMethod(method);
}

void SeasonalDecompositionDock::xColumnChanged(const QModelIndex& index) {
	CONDITIONAL_LOCK_RETURN;

	auto aspect = static_cast<AbstractAspect*>(index.internalPointer());
	AbstractColumn* column(nullptr);
	if (aspect) {
		column = dynamic_cast<AbstractColumn*>(aspect);
		Q_ASSERT(column);
	}

	for (auto* decomposition : m_decompositions)
		decomposition->setXColumn(column);
}

void SeasonalDecompositionDock::yColumnChanged(const QModelIndex& index) {
	CONDITIONAL_LOCK_RETURN;

	auto aspect = static_cast<AbstractAspect*>(index.internalPointer());
	AbstractColumn* column(nullptr);
	if (aspect) {
		column = dynamic_cast<AbstractColumn*>(aspect);
		Q_ASSERT(column);
	}

	for (auto* decomposition : m_decompositions)
		decomposition->setYColumn(column);
}

//*************************************************************
//*********** SLOTs for changes triggered in Histogram *******
//*************************************************************
// General-Tab
void SeasonalDecompositionDock::decompositionXColumnChanged(const AbstractColumn* column) {
	CONDITIONAL_LOCK_RETURN;
	cbXColumn->setAspect(column, m_decomposition->xColumnPath());
}

void SeasonalDecompositionDock::decompositionYColumnChanged(const AbstractColumn* column) {
	CONDITIONAL_LOCK_RETURN;
	cbYColumn->setAspect(column, m_decomposition->yColumnPath());
}

void SeasonalDecompositionDock::decompositionMethodChanged(SeasonalDecomposition::Method method) {
	CONDITIONAL_LOCK_RETURN;
	const int index = ui.cbMethod->findData(static_cast<int>(method));
	ui.cbMethod->setCurrentIndex(index);
}

//*************************************************************
//************************* Settings **************************
//*************************************************************
void SeasonalDecompositionDock::load() {
	// General

	// method
	const int index = ui.cbMethod->findData(static_cast<int>(m_decomposition->method()));
	ui.cbMethod->setCurrentIndex(index);
	methodChanged(index);
}

void SeasonalDecompositionDock::loadConfig(KConfig& config) {
	KConfigGroup group = config.group(QStringLiteral("SeasonalDecomposition"));

		// method
	const auto method = group.readEntry(QStringLiteral("Method"), static_cast<int>(m_decomposition->method()));
	const int index = ui.cbMethod->findData(static_cast<int>(method));
	ui.cbMethod->setCurrentIndex(index);
	methodChanged(index);

}

void SeasonalDecompositionDock::loadConfigFromTemplate(KConfig& config) {
	auto name = TemplateHandler::templateName(config);
	int size = m_decompositions.size();
	if (size > 1)
		m_decomposition->beginMacro(i18n("%1 seasonal decompositions: template \"%2\" loaded", size, name));
	else
		m_decomposition->beginMacro(i18n("%1: template \"%2\" loaded", m_decomposition->name(), name));

	this->loadConfig(config);

	m_decomposition->endMacro();
}

void SeasonalDecompositionDock::saveConfigAsTemplate(KConfig& config) {
	KConfigGroup group = config.group(QStringLiteral("SeasonalDecomposition"));

	config.sync();
}
