/*
	File                 : RunChartDock.cpp
	Project              : LabPlot
	Description          : widget for properties of the run chart
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "RunChartDock.h"
#include "backend/core/column/Column.h"
#include "backend/worksheet/plots/cartesian/RunChart.h"
#include "backend/worksheet/plots/cartesian/Symbol.h"
#include "commonfrontend/widgets/TreeViewComboBox.h"
#include "kdefrontend/TemplateHandler.h"
#include "kdefrontend/widgets/LineWidget.h"
#include "kdefrontend/widgets/SymbolWidget.h"

#include <QFrame>

#include <KConfig>
#include <KLocalizedString>

/*!
  \class RunChartDock
  \brief  Provides a widget for editing the properties of run charts.

  \ingroup kdefrontend
*/
RunChartDock::RunChartDock(QWidget* parent)
	: BaseDock(parent)
	, cbDataColumn(new TreeViewComboBox) {
	ui.setupUi(this);
	setPlotRangeCombobox(ui.cbPlotRanges);
	setBaseWidgets(ui.leName, ui.teComment);
	setVisibilityWidgets(ui.chkVisible, ui.chkLegendVisible);

	// Tab "General"
	auto* gridLayout = qobject_cast<QGridLayout*>(ui.tabGeneral->layout());
	gridLayout->addWidget(cbDataColumn, 3, 2, 1, 1);

	// Tab "Data Line"
	auto* hBoxLayout = static_cast<QHBoxLayout*>(ui.tabDataLine->layout());
	dataLineWidget = new LineWidget(ui.tabDataLine);
	hBoxLayout->insertWidget(1, dataLineWidget);

	dataSymbolWidget = new SymbolWidget(ui.tabDataLine);
	hBoxLayout->insertWidget(3, dataSymbolWidget);

	// Tab "Center Line"
	hBoxLayout = static_cast<QHBoxLayout*>(ui.tabCenterLine->layout());
	centerLineWidget = new LineWidget(ui.tabCenterLine);
	hBoxLayout->insertWidget(1, centerLineWidget);

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
	connect(cbDataColumn, &TreeViewComboBox::currentModelIndexChanged, this, &RunChartDock::dataColumnChanged);
	connect(ui.cbCenterMetric, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &RunChartDock::centerMetricChanged);

	// template handler
	auto* frame = new QFrame(this);
	auto* layout = new QHBoxLayout(frame);
	layout->setContentsMargins(0, 11, 0, 11);

	auto* templateHandler = new TemplateHandler(this, QLatin1String("RunChart"));
	layout->addWidget(templateHandler);
	connect(templateHandler, &TemplateHandler::loadConfigRequested, this, &RunChartDock::loadConfigFromTemplate);
	connect(templateHandler, &TemplateHandler::saveConfigRequested, this, &RunChartDock::saveConfigAsTemplate);
	connect(templateHandler, &TemplateHandler::info, this, &RunChartDock::info);

	ui.verticalLayout->addWidget(frame);

	updateLocale();
	retranslateUi();
}

RunChartDock::~RunChartDock() = default;

void RunChartDock::setModel() {
	auto* model = aspectModel();
	model->enablePlottableColumnsOnly(true);
	model->enableShowPlotDesignation(true);
	model->setSelectableAspects({AspectType::Column});
	cbDataColumn->setTopLevelClasses(TreeViewComboBox::plotColumnTopLevelClasses());
	cbDataColumn->setModel(model);
}

void RunChartDock::setPlots(QList<RunChart*> list) {
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
	for (auto* plot : m_plots) {
		dataLines << plot->dataLine();
		dataSymbols << plot->dataSymbol();
		centerLines << plot->centerLine();
	}
	dataLineWidget->setLines(dataLines);
	dataSymbolWidget->setSymbols(dataSymbols);
	centerLineWidget->setLines(centerLines);

	// if there are more then one curve in the list, disable the content in the tab "general"
	if (m_plots.size() == 1) {
		cbDataColumn->setEnabled(true);
		cbDataColumn->setColumn(m_plot->dataColumn(), m_plot->dataColumnPath());
	} else {
		cbDataColumn->setEnabled(false);
		cbDataColumn->setCurrentModelIndex(QModelIndex());
	}

	ui.chkLegendVisible->setChecked(m_plot->legendVisible());
	ui.chkVisible->setChecked(m_plot->isVisible());

	// center metric
	const int index = ui.cbCenterMetric->findData(static_cast<int>(m_plot->centerMetric()));
	ui.cbCenterMetric->setCurrentIndex(index);

	updatePlotRangeList();

	// Slots
	// General-tab
	connect(m_plot, &RunChart::dataColumnChanged, this, &RunChartDock::plotDataColumnChanged);
	connect(m_plot, &RunChart::centerMetricChanged, this, &RunChartDock::plotCenterMetricChanged);
}

void RunChartDock::retranslateUi() {
	ui.cbCenterMetric->clear();
	ui.cbCenterMetric->addItem(i18n("Average"), static_cast<int>(RunChart::CenterMetric::Average));
	ui.cbCenterMetric->addItem(i18n("Median"), static_cast<int>(RunChart::CenterMetric::Median));
}

/*
 * updates the locale in the widgets. called when the application settins are changed.
 */
void RunChartDock::updateLocale() {
	dataLineWidget->updateLocale();
	dataSymbolWidget->updateLocale();
	centerLineWidget->updateLocale();
}

//*************************************************************
//** SLOTs for changes triggered in RunChartDock **
//*************************************************************

// "General"-tab
void RunChartDock::dataColumnChanged(const QModelIndex& index) {
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

void RunChartDock::centerMetricChanged(int index) {
	CONDITIONAL_LOCK_RETURN;
	const auto metric = static_cast<RunChart::CenterMetric>(ui.cbCenterMetric->itemData(index).toInt());
	for (auto* plot : m_plots)
		plot->setCenterMetric(metric);
}

//*************************************************************
//**** SLOTs for changes triggered in RunChart ****
//*************************************************************
// General-Tab
void RunChartDock::plotDataColumnChanged(const AbstractColumn* column) {
	CONDITIONAL_LOCK_RETURN;
	cbDataColumn->setColumn(column, m_plot->dataColumnPath());
}

void RunChartDock::plotCenterMetricChanged(RunChart::CenterMetric metric) {
	CONDITIONAL_LOCK_RETURN;
	const int index = ui.cbCenterMetric->findData(static_cast<int>(metric));
	ui.cbCenterMetric->setCurrentIndex(index);
}

//*************************************************************
//************************* Settings **************************
//*************************************************************
void RunChartDock::loadConfig(KConfig& config) {
	KConfigGroup group = config.group(QStringLiteral("RunChart"));

	// center metric
	const auto metric = group.readEntry(QStringLiteral("CenterMetric"), static_cast<int>(m_plot->centerMetric()));
	const int index = ui.cbCenterMetric->findData(static_cast<int>(metric));
	ui.cbCenterMetric->setCurrentIndex(index);

	// properties of the reference and percentile curves
	dataLineWidget->loadConfig(group);
	dataSymbolWidget->loadConfig(group);
	centerLineWidget->loadConfig(group);
}

void RunChartDock::loadConfigFromTemplate(KConfig& config) {
	auto name = TemplateHandler::templateName(config);
	int size = m_plots.size();
	if (size > 1)
		m_plot->beginMacro(i18n("%1 xy-curves: template \"%2\" loaded", size, name));
	else
		m_plot->beginMacro(i18n("%1: template \"%2\" loaded", m_plot->name(), name));

	this->loadConfig(config);

	m_plot->endMacro();
}

void RunChartDock::saveConfigAsTemplate(KConfig& config) {
	KConfigGroup group = config.group(QStringLiteral("RunChart"));

	// general
	group.writeEntry(QStringLiteral("CenterMetric"), static_cast<int>(m_plot->centerMetric()));

	// properties of the data and center curves
	dataLineWidget->saveConfig(group);
	dataSymbolWidget->saveConfig(group);
	centerLineWidget->saveConfig(group);
	config.sync();
}
