/*
	File                 : ParetoChartDock.cpp
	Project              : LabPlot
	Description          : widget for properties of Pareto chart
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2025-2026 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ParetoChartDock.h"
#include "backend/core/column/Column.h"
#include "backend/worksheet/plots/cartesian/ParetoChart.h"
#include "backend/worksheet/plots/cartesian/Symbol.h"
#include "frontend/widgets/TreeViewComboBox.h"
#include "frontend/TemplateHandler.h"
#include "frontend/widgets/BackgroundWidget.h"
#include "frontend/widgets/LineWidget.h"
#include "frontend/widgets/SymbolWidget.h"
#include "frontend/widgets/ValueWidget.h"

#include <KConfig>
#include <KLocalizedString>

/*!
  \class ParetoChartDock
  \brief  Provides a widget for editing the properties of run charts.

  \ingroup frontend
*/
ParetoChartDock::ParetoChartDock(QWidget* parent)
	: BaseDock(parent)
	, cbDataColumn(new TreeViewComboBox) {
	ui.setupUi(this);
	setPlotRangeCombobox(ui.cbPlotRanges);
	setBaseWidgets(ui.leName, ui.teComment);
	setVisibilityWidgets(ui.chkVisible, ui.chkLegendVisible);

	// Tab "General"
	auto* gridLayout = qobject_cast<QGridLayout*>(ui.tabGeneral->layout());
	gridLayout->addWidget(cbDataColumn, 3, 2, 1, 1);

	// Tab "Bars"
	// filling
	auto* hBoxLayout = static_cast<QHBoxLayout*>(ui.tabBars->layout());
	barBackgroundWidget = new BackgroundWidget(ui.tabBars);
	hBoxLayout->insertWidget(1, barBackgroundWidget);

	// border lines
	barLineWidget = new LineWidget(ui.tabBars);
	hBoxLayout->insertWidget(5, barLineWidget);

	// Tab "Values"
	hBoxLayout = static_cast<QHBoxLayout*>(ui.tabValues->layout());
	valueWidget = new ValueWidget(ui.tabValues);
	hBoxLayout->insertWidget(0, valueWidget);

	// Tab "Line"
	hBoxLayout = static_cast<QHBoxLayout*>(ui.tabLine->layout());
	lineWidget = new LineWidget(ui.tabLine);
	hBoxLayout->insertWidget(1, lineWidget);

	symbolWidget = new SymbolWidget(ui.tabLine);
	hBoxLayout->insertWidget(3, symbolWidget);

	// Slots
	// General
	connect(cbDataColumn, &TreeViewComboBox::currentModelIndexChanged, this, &ParetoChartDock::dataColumnChanged);

	// template handler
	auto* frame = new QFrame(this);
	auto* layout = new QHBoxLayout(frame);
	layout->setContentsMargins(0, 11, 0, 11);

	auto* templateHandler = new TemplateHandler(this, QLatin1String("ParetoChart"));
	layout->addWidget(templateHandler);
	connect(templateHandler, &TemplateHandler::loadConfigRequested, this, &ParetoChartDock::loadConfigFromTemplate);
	connect(templateHandler, &TemplateHandler::saveConfigRequested, this, &ParetoChartDock::saveConfigAsTemplate);
	connect(templateHandler, &TemplateHandler::info, this, &ParetoChartDock::info);

	ui.verticalLayout->addWidget(frame);

	updateLocale();
	retranslateUi();
}

ParetoChartDock::~ParetoChartDock() = default;

void ParetoChartDock::setModel() {
	auto* model = aspectModel();
	model->enablePlottableColumnsOnly(true);
	model->enableShowPlotDesignation(true);
	model->setSelectableAspects({AspectType::Column});
	cbDataColumn->setTopLevelClasses(TreeViewComboBox::plotColumnTopLevelClasses());
	cbDataColumn->setModel(model);
}

void ParetoChartDock::setPlots(QList<ParetoChart*> list) {
	Lock lock(m_initializing);
	m_plots = list;
	m_plot = list.first();
	setAspects(list);
	Q_ASSERT(m_plot);
	setModel();

	// initialize widgets for common properties
	QList<Background*> barBackgrounds;
	QList<Line*> barLines;
	QList<Line*> lines;
	QList<Symbol*> symbols;
	QList<Value*> values;
	for (auto* plot : m_plots) {
		barBackgrounds << plot->barBackground();
		barLines << plot->barLine();
		lines << plot->line();
		symbols << plot->symbol();
		values << plot->value();
	}
	barBackgroundWidget->setBackgrounds(barBackgrounds);
	barLineWidget->setLines(barLines);
	lineWidget->setLines(lines);
	symbolWidget->setSymbols(symbols);
	valueWidget->setValues(values);

	// if there are more then one curve in the list, disable the content in the tab "general"
	if (m_plots.size() == 1) {
		cbDataColumn->setEnabled(true);
		cbDataColumn->setAspect(m_plot->dataColumn(), m_plot->dataColumnPath());
	} else {
		cbDataColumn->setEnabled(false);
		cbDataColumn->setCurrentModelIndex(QModelIndex());
	}

	ui.chkLegendVisible->setChecked(m_plot->legendVisible());
	ui.chkVisible->setChecked(m_plot->isVisible());

	updatePlotRangeList();

	// Slots
	// General-tab
	connect(m_plot, &ParetoChart::dataColumnChanged, this, &ParetoChartDock::plotDataColumnChanged);
}

void ParetoChartDock::retranslateUi() {

}

/*
 * updates the locale in the widgets. called when the application settins are changed.
 */
void ParetoChartDock::updateLocale() {
	// dataLineWidget->updateLocale();
	// dataSymbolWidget->updateLocale();
	// centerLineWidget->updateLocale();
}

//*************************************************************
//** SLOTs for changes triggered in ParetoChartDock **
//*************************************************************

// "General"-tab
void ParetoChartDock::dataColumnChanged(const QModelIndex& index) {
	if (m_initializing)
		return;

	auto aspect = static_cast<AbstractAspect*>(index.internalPointer());
	AbstractColumn* column(nullptr);
	if (aspect) {
		column = dynamic_cast<AbstractColumn*>(aspect);
		Q_ASSERT(column);
	}

	for (auto* plot : m_plots)
		plot->setDataColumn(column);
}

//*************************************************************
//**** SLOTs for changes triggered in ParetoChart ****
//*************************************************************
// General-Tab
void ParetoChartDock::plotDataColumnChanged(const AbstractColumn* column) {
	CONDITIONAL_LOCK_RETURN;
	cbDataColumn->setAspect(column, m_plot->dataColumnPath());
}

//*************************************************************
//************************* Settings **************************
//*************************************************************
void ParetoChartDock::loadConfig(KConfig& config) {
	KConfigGroup group = config.group(QStringLiteral("ParetoChart"));

	// properties of bars and lines
	barLineWidget->loadConfig(group);
	barBackgroundWidget->loadConfig(group);
	lineWidget->loadConfig(group);
	symbolWidget->loadConfig(group);
}

void ParetoChartDock::loadConfigFromTemplate(KConfig& config) {
	auto name = TemplateHandler::templateName(config);
	int size = m_plots.size();
	if (size > 1)
		m_plot->beginMacro(i18n("%1 xy-curves: template \"%2\" loaded", size, name));
	else
		m_plot->beginMacro(i18n("%1: template \"%2\" loaded", m_plot->name(), name));

	this->loadConfig(config);

	m_plot->endMacro();
}

void ParetoChartDock::saveConfigAsTemplate(KConfig& config) {
	KConfigGroup group = config.group(QStringLiteral("ParetoChart"));

	// properties of bars and lines
	barLineWidget->saveConfig(group);
	barBackgroundWidget->saveConfig(group);
	lineWidget->saveConfig(group);
	symbolWidget->saveConfig(group);
	config.sync();
}
