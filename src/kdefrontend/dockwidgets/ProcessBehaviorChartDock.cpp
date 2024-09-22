/*
	File                 : ProcessBehaviorChartDock.cpp
	Project              : LabPlot
	Description          : widget for properties of the process behavior chart
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ProcessBehaviorChartDock.h"
#include "backend/core/column/Column.h"
#include "backend/worksheet/plots/cartesian/ProcessBehaviorChart.h"
#include "backend/worksheet/plots/cartesian/Symbol.h"
#include "commonfrontend/widgets/TreeViewComboBox.h"
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
	connect(cbDataColumn, &TreeViewComboBox::currentModelIndexChanged, this, &ProcessBehaviorChartDock::dataColumnChanged);
	connect(ui.cbType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ProcessBehaviorChartDock::typeChanged);
	connect(ui.cbLimitsMetric, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ProcessBehaviorChartDock::limitsMetricChanged);
	connect(ui.sbSubgroupSize, &QSpinBox::valueChanged, this, &ProcessBehaviorChartDock::subgroupSizeChanged);

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
	cbDataColumn->setTopLevelClasses(TreeViewComboBox::plotColumnTopLevelClasses());
	cbDataColumn->setModel(model);
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
		cbDataColumn->setEnabled(true);
		cbDataColumn->setColumn(m_plot->dataColumn(), m_plot->dataColumnPath());
	} else {
		cbDataColumn->setEnabled(false);
		cbDataColumn->setCurrentModelIndex(QModelIndex());
	}

	ui.chkLegendVisible->setChecked(m_plot->legendVisible());
	ui.chkVisible->setChecked(m_plot->isVisible());

	// load the remaining properties
	load();

	updatePlotRangeList();

	// Slots
	// General-tab
	connect(m_plot, &ProcessBehaviorChart::dataColumnChanged, this, &ProcessBehaviorChartDock::plotDataColumnChanged);
	connect(m_plot, &ProcessBehaviorChart::typeChanged, this, &ProcessBehaviorChartDock::plotTypeChanged);
	connect(m_plot, &ProcessBehaviorChart::limitsMetricChanged, this, &ProcessBehaviorChartDock::plotLimitsMetricChanged);
	connect(m_plot, &ProcessBehaviorChart::subgroupSizeChanged, this, &ProcessBehaviorChartDock::plotSubgroupSizeChanged);
}

void ProcessBehaviorChartDock::retranslateUi() {
	ui.cbLimitsMetric->clear();
	ui.cbLimitsMetric->addItem(i18n("Average"), static_cast<int>(ProcessBehaviorChart::LimitsMetric::Average));
	ui.cbLimitsMetric->addItem(i18n("Median"), static_cast<int>(ProcessBehaviorChart::LimitsMetric::Median));

	ui.cbType->clear();
	ui.cbType->addItem(QStringLiteral("X"), static_cast<int>(ProcessBehaviorChart::Type::XmR));
	ui.cbType->addItem(QStringLiteral("mR"), static_cast<int>(ProcessBehaviorChart::Type::mR));
	ui.cbType->addItem(QStringLiteral("X̅  (X̅R)"), static_cast<int>(ProcessBehaviorChart::Type::XbarR));
	ui.cbType->addItem(QStringLiteral("R"), static_cast<int>(ProcessBehaviorChart::Type::R));
	ui.cbType->addItem(QStringLiteral("X̅  (X̅S)"), static_cast<int>(ProcessBehaviorChart::Type::XbarS));
	ui.cbType->addItem(QStringLiteral("S"), static_cast<int>(ProcessBehaviorChart::Type::S));

	// tooltips
	QString info = i18n(
		"The supported chart types are grouped according to the plotted statistics and to the metric defining the limits.<br><br>"
		"Individual Values and Moving Ranges, Limits Based on the Average or Median Moving Range:"
		"<ul>"
		"<li>X (XmR) - plot the <b>indidual values</b>.</li>"
		"<li>mR - plot the <b>moving ranges</b>.</li>"
		"</ul>"
		"Averages and Ranges, Limits Based on the Average or Median Range:"
		"<ul>"
		"<li>X̅  (X̅R) - plot the <b>averages for each subgroup</b> .</li>"
		"<li>R (X̅R) - plot the <b>ranges for each subgroup</b>.</li>"
		"</ul>"
		"Averages and Standard Deviations, Limits Based on the Standard Deviations:"
		"<ul>"
		"<li>X̅  (X̅S) - plot the <b>averages for each subgroup</b>.</li>"
		"<li>S (X̅S) - plot the <b>standard deviations for each subgroup</b>.</li>"
		"</ul>");
	ui.lType->setToolTip(info);
	ui.cbType->setToolTip(info);
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
//** SLOTs for changes triggered in ProcessBehaviorChartDock **
//*************************************************************

// "General"-tab
void ProcessBehaviorChartDock::dataColumnChanged(const QModelIndex& index) {
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

void ProcessBehaviorChartDock::typeChanged(int index) {
	const auto type = static_cast<ProcessBehaviorChart::Type>(ui.cbType->itemData(index).toInt());

	// depending on the current type, show/hide the settings for the subgroup type
	bool visible = (type == ProcessBehaviorChart::Type::XbarR || type == ProcessBehaviorChart::Type::R || type == ProcessBehaviorChart::Type::XbarS
					|| type == ProcessBehaviorChart::Type::S);
	ui.lSubgroupSize->setVisible(visible);
	ui.sbSubgroupSize->setVisible(visible);

	// depending on the current type, show/hide the settings for the metric used to define the limits
	visible = (type != ProcessBehaviorChart::Type::XbarS && type != ProcessBehaviorChart::Type::S);
	ui.lLimitsMetric->setVisible(visible);
	ui.cbLimitsMetric->setVisible(visible);

	CONDITIONAL_LOCK_RETURN;
	for (auto* plot : m_plots)
		plot->setType(type);
}

void ProcessBehaviorChartDock::limitsMetricChanged(int index) {
	CONDITIONAL_LOCK_RETURN;
	const auto limitsMetric = static_cast<ProcessBehaviorChart::LimitsMetric>(ui.cbType->itemData(index).toInt());
	for (auto* plot : m_plots)
		plot->setLimitsMetric(limitsMetric);
}

void ProcessBehaviorChartDock::subgroupSizeChanged(int value) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* plot : m_plots)
		plot->setSubgroupSize(value);
}

//*************************************************************
//**** SLOTs for changes triggered in ProcessBehaviorChart ****
//*************************************************************
// General-Tab
void ProcessBehaviorChartDock::plotDataColumnChanged(const AbstractColumn* column) {
	CONDITIONAL_LOCK_RETURN;
	cbDataColumn->setColumn(column, m_plot->dataColumnPath());
}

void ProcessBehaviorChartDock::plotTypeChanged(ProcessBehaviorChart::Type type) {
	CONDITIONAL_LOCK_RETURN;
	const int index = ui.cbType->findData(static_cast<int>(type));
	ui.cbType->setCurrentIndex(index);
}

void ProcessBehaviorChartDock::plotLimitsMetricChanged(ProcessBehaviorChart::LimitsMetric limitsMetric) {
	CONDITIONAL_LOCK_RETURN;
	const int index = ui.cbLimitsMetric->findData(static_cast<int>(limitsMetric));
	ui.cbLimitsMetric->setCurrentIndex(index);
}

void ProcessBehaviorChartDock::plotSubgroupSizeChanged(int value) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbSubgroupSize->setValue(value);
}

//*************************************************************
//************************* Settings **************************
//*************************************************************
void ProcessBehaviorChartDock::load() {
	// type
	int index = ui.cbType->findData(static_cast<int>(m_plot->type()));
	ui.cbType->setCurrentIndex(index);

	// limits metric
	index = ui.cbLimitsMetric->findData(static_cast<int>(m_plot->limitsMetric()));
	ui.cbLimitsMetric->setCurrentIndex(index);

	// subgroup size
	ui.sbSubgroupSize->setValue(static_cast<int>(m_plot->subgroupSize()));
}

void ProcessBehaviorChartDock::loadConfig(KConfig& config) {
	KConfigGroup group = config.group(QStringLiteral("ProcessBehaviorChart"));

	// type
	auto type = group.readEntry(QStringLiteral("type"), static_cast<int>(m_plot->type()));
	int index = ui.cbType->findData(static_cast<int>(type));
	ui.cbType->setCurrentIndex(index);

	// limits metric
	auto limitsMetric = group.readEntry(QStringLiteral("limitsMetric"), static_cast<int>(m_plot->limitsMetric()));
	index = ui.cbLimitsMetric->findData(static_cast<int>(limitsMetric));
	ui.cbLimitsMetric->setCurrentIndex(index);

	// subgroup size
	const int size = group.readEntry(QStringLiteral("subgroupSize"), static_cast<int>(m_plot->subgroupSize()));
	ui.sbSubgroupSize->setValue(size);

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

	// general
	group.writeEntry(QStringLiteral("type"), static_cast<int>(m_plot->type()));
	group.writeEntry(QStringLiteral("limitsMetric"), static_cast<int>(m_plot->limitsMetric()));
	group.writeEntry(QStringLiteral("subgroupSize"), m_plot->subgroupSize());

	// properties of the reference and percentile curves
	dataLineWidget->saveConfig(group);
	dataSymbolWidget->saveConfig(group);
	centerLineWidget->saveConfig(group);
	upperLimitLineWidget->saveConfig(group);
	lowerLimitLineWidget->saveConfig(group);
	config.sync();
}
