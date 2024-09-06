/*
	File                 : ProcessBehaviorChartDock.cpp
	Project              : LabPlot
	Description          : widget for properties of the process behavior chart
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ProcessBehaviorChartDock.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/core/column/Column.h"
#include "backend/nsl/nsl_sf_stats.h"
#include "backend/worksheet/plots/cartesian/ProcessBehaviorChart.h"
#include "backend/worksheet/plots/cartesian/Symbol.h"
#include "commonfrontend/widgets/TreeViewComboBox.h"
#include "kdefrontend/GuiTools.h"
#include "kdefrontend/TemplateHandler.h"
#include "kdefrontend/widgets/LineWidget.h"
#include "kdefrontend/widgets/SymbolWidget.h"
#include <QFrame>

#include <KConfig>
#include <KLocalizedString>

/*!
  \class ProcessBehaviorChartDock
  \brief  Provides a widget for editing the properties of QQ-plots.

  \ingroup kdefrontend
*/
ProcessBehaviorChartDock::ProcessBehaviorChartDock(QWidget* parent)
	: BaseDock(parent)
	, cbXDataColumn(new TreeViewComboBox)
	, cbYDataColumn(new TreeViewComboBox) {
	ui.setupUi(this);
	setPlotRangeCombobox(ui.cbPlotRanges);
	setBaseWidgets(ui.leName, ui.teComment);
	setVisibilityWidgets(ui.chkVisible, ui.chkLegendVisible);

	// Tab "General"
	auto* gridLayout = qobject_cast<QGridLayout*>(ui.tabGeneral->layout());
	gridLayout->addWidget(cbXDataColumn, 3, 2, 1, 1);
	gridLayout->addWidget(cbYDataColumn, 4, 2, 1, 1);

	ui.cbType->addItem(QStringLiteral("XmR"), static_cast<int>(ProcessBehaviorChart::Type::XmR));
	ui.cbType->addItem(QStringLiteral("mR"), static_cast<int>(ProcessBehaviorChart::Type::mR));
	ui.cbType->addItem(QStringLiteral("XbarR"), static_cast<int>(ProcessBehaviorChart::Type::XbarR));
	ui.cbType->addItem(QStringLiteral("R"), static_cast<int>(ProcessBehaviorChart::Type::R));
	ui.cbType->addItem(QStringLiteral("XbarS"), static_cast<int>(ProcessBehaviorChart::Type::XbarS));
	ui.cbType->addItem(QStringLiteral("S"), static_cast<int>(ProcessBehaviorChart::Type::S));
	ui.cbType->addItem(QStringLiteral("P"), static_cast<int>(ProcessBehaviorChart::Type::P));
	ui.cbType->addItem(QStringLiteral("NP"), static_cast<int>(ProcessBehaviorChart::Type::NP));
	ui.cbType->addItem(QStringLiteral("C"), static_cast<int>(ProcessBehaviorChart::Type::C));
	ui.cbType->addItem(QStringLiteral("U"), static_cast<int>(ProcessBehaviorChart::Type::U));

	// Tab "Data Line"
	auto* hBoxLayout = static_cast<QHBoxLayout*>(ui.tabDataLine->layout());
	dataLineWidget = new LineWidget(ui.tabDataLine);
	hBoxLayout->insertWidget(1, dataLineWidget);

	dataSymbolWidget = new SymbolWidget(ui.tabDataLine);
	hBoxLayout->insertWidget(3, dataSymbolWidget);

	// Tab "Contol Limit Lines"
	hBoxLayout = static_cast<QHBoxLayout*>(ui.tabControlLimitLines->layout());
	centerLineWidget = new LineWidget(ui.tabControlLimitLines);
	hBoxLayout->insertWidget(1, centerLineWidget);
	upperLimitLineWidget = new LineWidget(ui.tabControlLimitLines);
	hBoxLayout->insertWidget(5, upperLimitLineWidget);
	lowerLimitLineWidget = new LineWidget(ui.tabControlLimitLines);
	hBoxLayout->insertWidget(9, lowerLimitLineWidget);

	// adjust layouts in the tabs
	for (int i = 0; i < ui.tabWidget->count(); ++i) {
		auto* layout = dynamic_cast<QGridLayout*>(ui.tabWidget->widget(i)->layout());
		if (!layout)
			continue;

		layout->setContentsMargins(2, 2, 2, 2);
		layout->setHorizontalSpacing(2);
		layout->setVerticalSpacing(2);
	}

	// Slots
	// General
	connect(cbXDataColumn, &TreeViewComboBox::currentModelIndexChanged, this, &ProcessBehaviorChartDock::xDataColumnChanged);
	connect(cbYDataColumn, &TreeViewComboBox::currentModelIndexChanged, this, &ProcessBehaviorChartDock::yDataColumnChanged);
	connect(ui.cbType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ProcessBehaviorChartDock::typeChanged);

	// template handler
	auto* frame = new QFrame(this);
	auto* layout = new QHBoxLayout(frame);
	layout->setContentsMargins(0, 11, 0, 11);

	auto* templateHandler = new TemplateHandler(this, QLatin1String("ProcessBehaviorChart"));
	layout->addWidget(templateHandler);
	connect(templateHandler, &TemplateHandler::loadConfigRequested, this, &ProcessBehaviorChartDock::loadConfigFromTemplate);
	connect(templateHandler, &TemplateHandler::saveConfigRequested, this, &ProcessBehaviorChartDock::saveConfigAsTemplate);
	connect(templateHandler, &TemplateHandler::info, this, &ProcessBehaviorChartDock::info);

	ui.verticalLayout->addWidget(frame);

	updateLocale();
	retranslateUi();
}

ProcessBehaviorChartDock::~ProcessBehaviorChartDock() = default;

void ProcessBehaviorChartDock::setModel() {
	auto* model = aspectModel();
	model->enablePlottableColumnsOnly(true);
	model->enableShowPlotDesignation(true);
	model->setSelectableAspects({AspectType::Column});
	cbXDataColumn->setTopLevelClasses(TreeViewComboBox::plotColumnTopLevelClasses());
	cbXDataColumn->setModel(model);
	cbYDataColumn->setTopLevelClasses(TreeViewComboBox::plotColumnTopLevelClasses());
	cbYDataColumn->setModel(model);
}

void ProcessBehaviorChartDock::setPlots(QList<ProcessBehaviorChart*> list) {
	Lock lock(m_initializing);
	m_plots = list;
	m_plot = list.first();
	setAspects(list);
	Q_ASSERT(m_plot);
	setModel();

	// initialize widgets for common properties
	QList<Line*> dataLines;
	QList<Symbol*> dataSymbols;
	QList<Line*> centerLines;
	QList<Line*> upperLimitLines;
	QList<Line*> lowerLimitLines;
	for (auto* plot : m_plots) {
		dataLines << plot->dataLine();
		dataSymbols << plot->dataSymbol();
		centerLines << plot->centerLine();
		upperLimitLines << plot->upperLimitLine();
		lowerLimitLines << plot->lowerLimitLine();
	}
	dataLineWidget->setLines(dataLines);
	dataSymbolWidget->setSymbols(dataSymbols);
	centerLineWidget->setLines(centerLines);
	upperLimitLineWidget->setLines(upperLimitLines);
	lowerLimitLineWidget->setLines(lowerLimitLines);

	// if there are more then one curve in the list, disable the content in the tab "general"
	if (m_plots.size() == 1) {
		cbXDataColumn->setEnabled(true);
		cbXDataColumn->setColumn(m_plot->xDataColumn(), m_plot->xDataColumnPath());
		cbYDataColumn->setEnabled(true);
		cbYDataColumn->setColumn(m_plot->yDataColumn(), m_plot->yDataColumnPath());
	} else {
		cbXDataColumn->setEnabled(false);
		cbXDataColumn->setCurrentModelIndex(QModelIndex());
		cbYDataColumn->setEnabled(false);
		cbYDataColumn->setCurrentModelIndex(QModelIndex());
	}

	ui.chkLegendVisible->setChecked(m_plot->legendVisible());
	ui.chkVisible->setChecked(m_plot->isVisible());

	// load the remaining properties
	load();

	updatePlotRangeList();

	// Slots
	// General-tab
	connect(m_plot, &ProcessBehaviorChart::xDataColumnChanged, this, &ProcessBehaviorChartDock::plotXDataColumnChanged);
	connect(m_plot, &ProcessBehaviorChart::yDataColumnChanged, this, &ProcessBehaviorChartDock::plotYDataColumnChanged);
	connect(m_plot, &ProcessBehaviorChart::typeChanged, this, &ProcessBehaviorChartDock::plotTypeChanged);
}

void ProcessBehaviorChartDock::retranslateUi() {
	// ui.cbType->clear();
}

/*
 * updates the locale in the widgets. called when the application settins are changed.
 */
void ProcessBehaviorChartDock::updateLocale() {
	dataLineWidget->updateLocale();
	dataSymbolWidget->updateLocale();
	centerLineWidget->updateLocale();
	upperLimitLineWidget->updateLocale();
	lowerLimitLineWidget->updateLocale();
}

//*************************************************************
//**** SLOTs for changes triggered in ProcessBehaviorChartDock *****
//*************************************************************

// "General"-tab
void ProcessBehaviorChartDock::xDataColumnChanged(const QModelIndex& index) {
	if (m_initializing)
		return;

	auto aspect = static_cast<AbstractAspect*>(index.internalPointer());
	AbstractColumn* column(nullptr);
	if (aspect) {
		column = dynamic_cast<AbstractColumn*>(aspect);
		Q_ASSERT(column);
	}

	for (auto* plot : m_plots)
		plot->setXDataColumn(column);
}

void ProcessBehaviorChartDock::yDataColumnChanged(const QModelIndex& index) {
	if (m_initializing)
		return;

	auto aspect = static_cast<AbstractAspect*>(index.internalPointer());
	AbstractColumn* column(nullptr);
	if (aspect) {
		column = dynamic_cast<AbstractColumn*>(aspect);
		Q_ASSERT(column);
	}

	for (auto* plot : m_plots)
		plot->setYDataColumn(column);
}

void ProcessBehaviorChartDock::typeChanged(int index) {
	const auto type  = static_cast<ProcessBehaviorChart::Type>(ui.cbType->itemData(index).toInt());

	CONDITIONAL_LOCK_RETURN;

	for (auto* plot : m_plots)
		plot->setType(type);
}

//*************************************************************
//*********** SLOTs for changes triggered in ProcessBehaviorChart *******
//*************************************************************
// General-Tab
void ProcessBehaviorChartDock::plotXDataColumnChanged(const AbstractColumn* column) {
	CONDITIONAL_LOCK_RETURN;
	cbXDataColumn->setColumn(column, m_plot->xDataColumnPath());
}

void ProcessBehaviorChartDock::plotYDataColumnChanged(const AbstractColumn* column) {
	CONDITIONAL_LOCK_RETURN;
	cbYDataColumn->setColumn(column, m_plot->yDataColumnPath());
}

void ProcessBehaviorChartDock::plotTypeChanged(ProcessBehaviorChart::Type type) {
	CONDITIONAL_LOCK_RETURN;
	int index = ui.cbType->findData(static_cast<int>(type));
	ui.cbType->setCurrentIndex(index);
}

//*************************************************************
//************************* Settings **************************
//*************************************************************
void ProcessBehaviorChartDock::load() {
	// distribution
	int index = ui.cbType->findData(static_cast<int>(m_plot->type()));
	ui.cbType->setCurrentIndex(index);
}

void ProcessBehaviorChartDock::loadConfig(KConfig& config) {
	KConfigGroup group = config.group(QStringLiteral("ProcessBehaviorChart"));

	// distribution
	auto dist = group.readEntry(QStringLiteral("distribution"), static_cast<int>(m_plot->type()));
	int index = ui.cbType->findData(static_cast<int>(dist));
	ui.cbType->setCurrentIndex(index);

	// properties of the reference and percentile curves
	dataLineWidget->loadConfig(group);
	dataSymbolWidget->loadConfig(group);
	centerLineWidget->loadConfig(group);
	upperLimitLineWidget->loadConfig(group);
	lowerLimitLineWidget->loadConfig(group);
}

void ProcessBehaviorChartDock::loadConfigFromTemplate(KConfig& config) {
	auto name = TemplateHandler::templateName(config);
	int size = m_plots.size();
	if (size > 1)
		m_plot->beginMacro(i18n("%1 xy-curves: template \"%2\" loaded", size, name));
	else
		m_plot->beginMacro(i18n("%1: template \"%2\" loaded", m_plot->name(), name));

	this->loadConfig(config);

	m_plot->endMacro();
}

void ProcessBehaviorChartDock::saveConfigAsTemplate(KConfig& config) {
	KConfigGroup group = config.group(QStringLiteral("ProcessBehaviorChart"));

	// distribution
	group.writeEntry(QStringLiteral("kernelType"), static_cast<int>(m_plot->type()));

	// properties of the reference and percentile curves
	dataLineWidget->saveConfig(group);
	dataSymbolWidget->saveConfig(group);
	centerLineWidget->saveConfig(group);
	upperLimitLineWidget->saveConfig(group);
	lowerLimitLineWidget->saveConfig(group);
	config.sync();
}