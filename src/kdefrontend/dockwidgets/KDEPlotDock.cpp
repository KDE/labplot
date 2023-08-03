/*
	File                 : KDEPlotDock.cpp
	Project              : LabPlot
	Description          : widget for KDE-plot properties
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "KDEPlotDock.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/core/Project.h"
#include "backend/core/column/Column.h"
#include "backend/nsl/nsl_sf_stats.h"
#include "backend/worksheet/plots/cartesian/KDEPlot.h"
#include "commonfrontend/widgets/TreeViewComboBox.h"
#include "kdefrontend/GuiTools.h"
#include "kdefrontend/TemplateHandler.h"
#include "kdefrontend/widgets/BackgroundWidget.h"
#include "kdefrontend/widgets/LineWidget.h"
#include <QFrame>

#include <KConfig>
#include <KLocalizedString>

/*!
  \class KDEPlotDock
  \brief  Provides a widget for editing the properties of KDE-plots.

  \ingroup kdefrontend
*/
KDEPlotDock::KDEPlotDock(QWidget* parent)
	: BaseDock(parent)
	, cbDataColumn(new TreeViewComboBox) {
	ui.setupUi(this);
	m_leName = ui.leName;
	m_teComment = ui.teComment;
	m_teComment->setFixedHeight(2 * m_leName->height());

	// Tab "General"
	auto* gridLayout = qobject_cast<QGridLayout*>(ui.tabGeneral->layout());
	gridLayout->addWidget(cbDataColumn, 3, 2, 1, 1);

	// Tab "Estimation"
	auto* hBoxLayout = static_cast<QHBoxLayout*>(ui.tabKDE->layout());
	estimationLineWidget = new LineWidget(ui.tabKDE);
	hBoxLayout->insertWidget(1, estimationLineWidget);

	estimationBackgroundWidget = new BackgroundWidget(ui.tabKDE);
	hBoxLayout->insertWidget(4, estimationBackgroundWidget);

	// Tab "Histogram"
	gridLayout = static_cast<QGridLayout*>(ui.tabHistogram->layout());
	histogramLineWidget = new LineWidget(ui.tabHistogram);
	gridLayout->addWidget(histogramLineWidget, 3, 0, 0, 3);

	histogramBackgroundWidget = new BackgroundWidget(ui.tabHistogram);
	gridLayout->addWidget(histogramBackgroundWidget, 5, 0, 0, 3);

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
	connect(ui.leName, &QLineEdit::textChanged, this, &KDEPlotDock::nameChanged);
	connect(ui.teComment, &QTextEdit::textChanged, this, &KDEPlotDock::commentChanged);
	connect(ui.chkVisible, &QCheckBox::clicked, this, &KDEPlotDock::visibilityChanged);
	connect(cbDataColumn, &TreeViewComboBox::currentModelIndexChanged, this, &KDEPlotDock::dataColumnChanged);
	connect(ui.cbPlotRanges, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &KDEPlotDock::plotRangeChanged);

	// template handler
	auto* frame = new QFrame(this);
	auto* layout = new QHBoxLayout(frame);
	layout->setContentsMargins(0, 11, 0, 11);

	auto* templateHandler = new TemplateHandler(this, TemplateHandler::ClassName::Worksheet);
	layout->addWidget(templateHandler);
	connect(templateHandler, &TemplateHandler::loadConfigRequested, this, &KDEPlotDock::loadConfigFromTemplate);
	connect(templateHandler, &TemplateHandler::saveConfigRequested, this, &KDEPlotDock::saveConfigAsTemplate);
	connect(templateHandler, &TemplateHandler::info, this, &KDEPlotDock::info);

	ui.verticalLayout->addWidget(frame);

	updateLocale();
	retranslateUi();
}

KDEPlotDock::~KDEPlotDock() {
	if (m_aspectTreeModel)
		delete m_aspectTreeModel;
}

void KDEPlotDock::setModel() {
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

void KDEPlotDock::setPlots(QList<KDEPlot*> list) {
	Lock lock(m_initializing);
	m_plots = list;
	m_plot = list.first();
	setAspects(list);
	Q_ASSERT(m_plot);
	m_aspectTreeModel = new AspectTreeModel(m_plot->project());
	setModel();

	// initialize widgets for common properties
	QList<Line*> estimationLines;
	QList<Background*> estimationBackgrounds;
	QList<Line*> histogramLines;
	QList<Background*> histogramBackgrounds;
	for (auto* plot : m_plots) {
		estimationLines << plot->estimationCurve()->line();
		estimationBackgrounds<< plot->estimationCurve()->background();
		histogramLines << plot->histogram()->line();
		histogramBackgrounds<< plot->histogram()->background();
	}
	estimationLineWidget->setLines(estimationLines);
	estimationBackgroundWidget->setBackgrounds(estimationBackgrounds);
	histogramLineWidget->setLines(histogramLines);
	histogramBackgroundWidget->setBackgrounds(histogramBackgrounds);

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
	load();

	updatePlotRanges();

	// Slots
	// General-tab
	connect(m_plot, &KDEPlot::aspectDescriptionChanged, this, &KDEPlotDock::aspectDescriptionChanged);
	connect(m_plot, &KDEPlot::dataColumnChanged, this, &KDEPlotDock::plotDataColumnChanged);
}

void KDEPlotDock::retranslateUi() {

}

/*
 * updates the locale in the widgets. called when the application settins are changed.
 */
void KDEPlotDock::updateLocale() {
	estimationLineWidget->updateLocale();
	histogramLineWidget->updateLocale();
}

void KDEPlotDock::updatePlotRanges() {
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
//**** SLOTs for changes triggered in KDEPlotDock *****
//*************************************************************

// "General"-tab
void KDEPlotDock::dataColumnChanged(const QModelIndex& index) {
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

void KDEPlotDock::visibilityChanged(bool state) {
	if (m_initializing)
		return;

	for (auto* plot : m_plots)
		plot->setVisible(state);
}

//*************************************************************
//*********** SLOTs for changes triggered in KDEPlot *******
//*************************************************************
// General-Tab
void KDEPlotDock::plotDataColumnChanged(const AbstractColumn* column) {
	m_initializing = true;
	cbDataColumn->setColumn(column, m_plot->dataColumnPath());
	m_initializing = false;
}

void KDEPlotDock::plotVisibilityChanged(bool on) {
	m_initializing = true;
	ui.chkVisible->setChecked(on);
	m_initializing = false;
}

//*************************************************************
//************************* Settings **************************
//*************************************************************
void KDEPlotDock::load() {
	// TODO
}

void KDEPlotDock::loadConfig(KConfig& config) {
	KConfigGroup group = config.group(QLatin1String("KDEPlot"));

	// General
	// we don't load/save the settings in the general-tab, since they are not style related.
	// It doesn't make sense to load/save them in the template.
	// This data is read in KDEPlotDock::setCurves().

	// TODO
	// lineWidget->loadConfig(group);
}

void KDEPlotDock::loadConfigFromTemplate(KConfig& config) {
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

void KDEPlotDock::saveConfigAsTemplate(KConfig& config) {
	KConfigGroup group = config.group("KDEPlot");
	// TODO
	// lineWidget->saveConfig(group);
	config.sync();
}
