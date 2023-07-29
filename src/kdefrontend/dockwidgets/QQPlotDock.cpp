/*
	File                 : QQPlotDock.cpp
	Project              : LabPlot
	Description          : widget for QQ-plot properties
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "QQPlotDock.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/core/Project.h"
#include "backend/core/column/Column.h"
#include "backend/worksheet/plots/cartesian/QQPlot.h"
#include "backend/worksheet/plots/cartesian/Symbol.h"
#include "commonfrontend/widgets/TreeViewComboBox.h"
#include "kdefrontend/TemplateHandler.h"
#include "kdefrontend/widgets/LineWidget.h"
#include "kdefrontend/widgets/SymbolWidget.h"

#include <QFrame>

#include <KConfig>
#include <KLocalizedString>

/*!
  \class QQPlotDock
  \brief  Provides a widget for editing the properties of QQ-plots.

  \ingroup kdefrontend
*/
QQPlotDock::QQPlotDock(QWidget* parent)
	: BaseDock(parent)
	, cbDataColumn(new TreeViewComboBox) {
	ui.setupUi(this);
	m_leName = ui.leName;
	m_teComment = ui.teComment;
	m_teComment->setFixedHeight(2 * m_leName->height());

	// Tab "General"
	auto* gridLayout = qobject_cast<QGridLayout*>(ui.tabGeneral->layout());
	gridLayout->addWidget(cbDataColumn, 3, 2, 1, 1);

	// Tab "Reference Line"
	auto* hBoxLayout = static_cast<QHBoxLayout*>(ui.tabReferenceLine->layout());
	lineWidget = new LineWidget(ui.tabReferenceLine);
	hBoxLayout->insertWidget(0, lineWidget);

	// Tab "Percentiles"
	// hBoxLayout = new QHBoxLayout(ui.tabPercentiles);
	hBoxLayout = static_cast<QHBoxLayout*>(ui.tabPercentiles->layout());
	symbolWidget = new SymbolWidget(ui.tabPercentiles);
	hBoxLayout->insertWidget(0, symbolWidget);

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
	connect(ui.leName, &QLineEdit::textChanged, this, &QQPlotDock::nameChanged);
	connect(ui.teComment, &QTextEdit::textChanged, this, &QQPlotDock::commentChanged);
	connect(ui.chkVisible, &QCheckBox::clicked, this, &QQPlotDock::visibilityChanged);
	connect(cbDataColumn, &TreeViewComboBox::currentModelIndexChanged, this, &QQPlotDock::dataColumnChanged);
	connect(ui.cbPlotRanges, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &QQPlotDock::plotRangeChanged);

	// template handler
	auto* frame = new QFrame(this);
	auto* layout = new QHBoxLayout(frame);
	layout->setContentsMargins(0, 11, 0, 11);

	auto* templateHandler = new TemplateHandler(this, TemplateHandler::ClassName::Worksheet);
	layout->addWidget(templateHandler);
	connect(templateHandler, &TemplateHandler::loadConfigRequested, this, &QQPlotDock::loadConfigFromTemplate);
	connect(templateHandler, &TemplateHandler::saveConfigRequested, this, &QQPlotDock::saveConfigAsTemplate);
	connect(templateHandler, &TemplateHandler::info, this, &QQPlotDock::info);

	ui.verticalLayout->addWidget(frame);

	updateLocale();
	retranslateUi();
}

QQPlotDock::~QQPlotDock() {
	if (m_aspectTreeModel)
		delete m_aspectTreeModel;
}

void QQPlotDock::setModel() {
	m_aspectTreeModel->enablePlottableColumnsOnly(true);
	m_aspectTreeModel->enableShowPlotDesignation(true);

	QList<AspectType> list{AspectType::Folder,
						   AspectType::Workbook,
						   AspectType::Datapicker,
						   AspectType::DatapickerCurve,
						   AspectType::Spreadsheet,
						   AspectType::LiveDataSource,
						   AspectType::Column,
						   AspectType::Worksheet,
						   AspectType::CartesianPlot,
						   AspectType::XYFitCurve,
						   AspectType::XYSmoothCurve,
						   AspectType::CantorWorksheet};

	cbDataColumn->setTopLevelClasses(list);

	list = {AspectType::Column};
	m_aspectTreeModel->setSelectableAspects(list);

	cbDataColumn->setModel(m_aspectTreeModel);
}

void QQPlotDock::setPlots(QList<QQPlot*> list) {
	Lock lock(m_initializing);
	m_plots = list;
	m_plot = list.first();
	setAspects(list);
	Q_ASSERT(m_plot);
	m_aspectTreeModel = new AspectTreeModel(m_plot->project());
	setModel();

	// initialize widgets for common properties
	QList<Line*> lines;
	QList<Symbol*> symbols;
	for (auto* plot : m_plots) {
		lines << plot->line();
		symbols << plot->symbol();
	}
	lineWidget->setLines(lines);
	symbolWidget->setSymbols(symbols);

	// if there are more then one curve in the list, disable the content in the tab "general"
	if (m_plots.size() == 1) {
		ui.lName->setEnabled(true);
		ui.leName->setEnabled(true);
		ui.lComment->setEnabled(true);
		ui.teComment->setEnabled(true);

		ui.lDataColumn->setEnabled(true);
		cbDataColumn->setEnabled(true);

		cbDataColumn->setColumn(m_plot->dataColumn(), m_plot->dataColumnPath());
		ui.leName->setText(m_plot->name());
		ui.teComment->setText(m_plot->comment());
	} else {
		ui.lName->setEnabled(false);
		ui.leName->setEnabled(false);
		ui.lComment->setEnabled(false);
		ui.teComment->setEnabled(false);

		ui.lDataColumn->setEnabled(false);
		cbDataColumn->setEnabled(false);
		cbDataColumn->setCurrentModelIndex(QModelIndex());

		ui.leName->setText(QString());
		ui.teComment->setText(QString());
	}

	ui.leName->setStyleSheet(QString());
	ui.leName->setToolTip(QString());
	ui.chkVisible->setChecked(m_plot->isVisible());

	// load the remaining properties
	KConfig config(QString(), KConfig::SimpleConfig);
	loadConfig(config);

	updatePlotRanges();

	// Slots
	// General-tab
	connect(m_plot, &QQPlot::aspectDescriptionChanged, this, &QQPlotDock::aspectDescriptionChanged);
	connect(m_plot, &QQPlot::dataColumnChanged, this, &QQPlotDock::plotDataColumnChanged);
}

void QQPlotDock::retranslateUi() {
	ui.cbDistribution->clear();
	ui.cbDistribution->addItem(i18n("Normal"));
}

/*
 * updates the locale in the widgets. called when the application settins are changed.
 */
void QQPlotDock::updateLocale() {
	lineWidget->updateLocale();
	symbolWidget->updateLocale();
}

void QQPlotDock::updatePlotRanges() {
	const int cSystemCount{m_plot->coordinateSystemCount()};
	const int cSystemIndex{m_plot->coordinateSystemIndex()};
	DEBUG(Q_FUNC_INFO << ", plot ranges count: " << cSystemCount)
	DEBUG(Q_FUNC_INFO << ", current plot range: " << cSystemIndex + 1)

	// fill ui.cbPlotRanges
	ui.cbPlotRanges->clear();
	for (int i{0}; i < cSystemCount; i++)
		ui.cbPlotRanges->addItem(QString::number(i + 1) + QLatin1String(" : ") + m_plot->coordinateSystemInfo(i));
	ui.cbPlotRanges->setCurrentIndex(cSystemIndex);
	// disable when there is only on plot range
	ui.cbPlotRanges->setEnabled(cSystemCount == 1 ? false : true);
}

//*************************************************************
//**** SLOTs for changes triggered in QQPlotDock *****
//*************************************************************

// "General"-tab
void QQPlotDock::dataColumnChanged(const QModelIndex& index) {
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

void QQPlotDock::visibilityChanged(bool state) {
	if (m_initializing)
		return;

	for (auto* plot : m_plots)
		plot->setVisible(state);
}

//*************************************************************
//*********** SLOTs for changes triggered in QQPlot *******
//*************************************************************
// General-Tab
void QQPlotDock::plotDataColumnChanged(const AbstractColumn* column) {
	m_initializing = true;
	cbDataColumn->setColumn(column, m_plot->dataColumnPath());
	m_initializing = false;
}

void QQPlotDock::plotVisibilityChanged(bool on) {
	m_initializing = true;
	ui.chkVisible->setChecked(on);
	m_initializing = false;
}

//*************************************************************
//************************* Settings **************************
//*************************************************************
void QQPlotDock::loadConfig(KConfig& config) {
	KConfigGroup group = config.group(QLatin1String("QQPlot"));

	// General
	// we don't load/save the settings in the general-tab, since they are not style related.
	// It doesn't make sense to load/save them in the template.
	// This data is read in QQPlotDock::setCurves().

	//TODO distribution

	lineWidget->loadConfig(group);
	symbolWidget->loadConfig(group);
}

void QQPlotDock::loadConfigFromTemplate(KConfig& config) {
	// extract the name of the template from the file name
	QString name;
	int index = config.name().lastIndexOf(QLatin1String("/"));
	if (index != -1)
		name = config.name().right(config.name().size() - index - 1);
	else
		name = config.name();

	int size = m_plots.size();
	if (size > 1)
		m_plot->beginMacro(i18n("%1 xy-curves: template \"%2\" loaded", size, name));
	else
		m_plot->beginMacro(i18n("%1: template \"%2\" loaded", m_plot->name(), name));

	this->loadConfig(config);

	m_plot->endMacro();
}

void QQPlotDock::saveConfigAsTemplate(KConfig& config) {
	KConfigGroup group = config.group("QQPlot");
	lineWidget->saveConfig(group);
	symbolWidget->saveConfig(group);
	config.sync();
}
