/*
	File                 : KDEPlotDock.cpp
	Project              : LabPlot
	Description          : widget for KDE-plot properties
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "KDEPlotDock.h"
#include "backend/core/Project.h"
#include "backend/core/column/Column.h"
#include "backend/worksheet/plots/cartesian/KDEPlot.h"
#include "frontend/widgets/TreeViewComboBox.h"
#include "frontend/GuiTools.h"
#include "frontend/TemplateHandler.h"
#include "frontend/widgets/BackgroundWidget.h"
#include "frontend/widgets/LineWidget.h"

#include <KConfig>
#include <KLocalizedString>

/*!
  \class KDEPlotDock
  \brief  Provides a widget for editing the properties of KDE-plots.

  \ingroup frontend
*/
KDEPlotDock::KDEPlotDock(QWidget* parent)
	: BaseDock(parent)
	, cbDataColumn(new TreeViewComboBox) {
	ui.setupUi(this);
	setPlotRangeCombobox(ui.cbPlotRanges);
	setBaseWidgets(ui.leName, ui.teComment);
	setVisibilityWidgets(ui.chkVisible, ui.chkLegendVisible);

	// Tab "General"
	auto* gridLayout = qobject_cast<QGridLayout*>(ui.tabGeneral->layout());
	gridLayout->addWidget(cbDataColumn, 3, 2, 1, 1);

	// Tab "Estimation"
	auto* hBoxLayout = static_cast<QHBoxLayout*>(ui.tabKDE->layout());
	estimationLineWidget = new LineWidget(ui.tabKDE);
	hBoxLayout->insertWidget(1, estimationLineWidget);

	estimationBackgroundWidget = new BackgroundWidget(ui.tabKDE);
	hBoxLayout->insertWidget(5, estimationBackgroundWidget);

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
	connect(cbDataColumn, &TreeViewComboBox::currentModelIndexChanged, this, &KDEPlotDock::dataColumnChanged);
	connect(ui.cbKernelType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &KDEPlotDock::kernelTypeChanged);
	connect(ui.cbBandwidthType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &KDEPlotDock::bandwidthTypeChanged);
	connect(ui.sbBandwidth, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &KDEPlotDock::bandwidthChanged);

	// Margin Plots
	connect(ui.chkRugEnabled, &QCheckBox::toggled, this, &KDEPlotDock::rugEnabledChanged);
	connect(ui.sbRugLength, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &KDEPlotDock::rugLengthChanged);
	connect(ui.sbRugWidth, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &KDEPlotDock::rugWidthChanged);
	connect(ui.sbRugOffset, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &KDEPlotDock::rugOffsetChanged);

	// template handler
	auto* frame = new QFrame(this);
	auto* layout = new QHBoxLayout(frame);
	layout->setContentsMargins(0, 11, 0, 11);

	auto* templateHandler = new TemplateHandler(this, QLatin1String("KDEPlot"));
	layout->addWidget(templateHandler);
	connect(templateHandler, &TemplateHandler::loadConfigRequested, this, &KDEPlotDock::loadConfigFromTemplate);
	connect(templateHandler, &TemplateHandler::saveConfigRequested, this, &KDEPlotDock::saveConfigAsTemplate);
	connect(templateHandler, &TemplateHandler::info, this, &KDEPlotDock::info);

	ui.verticalLayout->addWidget(frame);

	updateLocale();
	retranslateUi();
}

KDEPlotDock::~KDEPlotDock() = default;

void KDEPlotDock::setModel() {
	auto* model = aspectModel();
	model->enablePlottableColumnsOnly(true);
	model->enableShowPlotDesignation(true);
	model->setSelectableAspects({AspectType::Column});
	cbDataColumn->setTopLevelClasses(TreeViewComboBox::plotColumnTopLevelClasses());
	cbDataColumn->setModel(model);
}

void KDEPlotDock::setPlots(QList<KDEPlot*> list) {
	Lock lock(m_initializing);
	m_plots = list;
	m_plot = list.first();
	setAspects(list);
	Q_ASSERT(m_plot);
	setModel();

	// initialize widgets for common properties
	QList<Line*> estimationLines;
	QList<Background*> estimationBackgrounds;
	QList<Line*> histogramLines;
	QList<Background*> histogramBackgrounds;
	for (auto* plot : m_plots) {
		estimationLines << plot->estimationCurve()->line();
		estimationBackgrounds << plot->estimationCurve()->background();
	}
	estimationLineWidget->setLines(estimationLines);
	estimationBackgroundWidget->setBackgrounds(estimationBackgrounds);

	// if there are more then one curve in the list, disable the content in the tab "general"
	if (m_plots.size() == 1) {
		cbDataColumn->setEnabled(true);
		cbDataColumn->setColumn(m_plot->dataColumn(), m_plot->dataColumnPath());
		ui.leName->setText(m_plot->name());
		ui.teComment->setText(m_plot->comment());
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
	connect(m_plot, &KDEPlot::dataColumnChanged, this, &KDEPlotDock::plotDataColumnChanged);
	connect(m_plot, &KDEPlot::kernelTypeChanged, this, &KDEPlotDock::plotKernelTypeChanged);
	connect(m_plot, &KDEPlot::bandwidthTypeChanged, this, &KDEPlotDock::plotBandwidthTypeChanged);
	connect(m_plot, &KDEPlot::bandwidthChanged, this, &KDEPlotDock::plotBandwidthChanged);

	//"Margin Plots"-Tab
	auto* curve = m_plot->rugCurve();
	connect(curve, &XYCurve::rugEnabledChanged, this, &KDEPlotDock::plotRugEnabledChanged);
	connect(curve, &XYCurve::rugLengthChanged, this, &KDEPlotDock::plotRugLengthChanged);
	connect(curve, &XYCurve::rugWidthChanged, this, &KDEPlotDock::plotRugWidthChanged);
	connect(curve, &XYCurve::rugOffsetChanged, this, &KDEPlotDock::plotRugOffsetChanged);
}

void KDEPlotDock::retranslateUi() {
	// TODO unify with nsl_smooth_weight_type_name in nsl_smooth.c.
	ui.cbKernelType->clear();
	ui.cbKernelType->addItem(i18n("Gauss"), static_cast<int>(nsl_kernel_gauss));
	ui.cbKernelType->addItem(i18n("Uniform (Rectangular)"), static_cast<int>(nsl_kernel_uniform));
	ui.cbKernelType->addItem(i18n("Triangular"), static_cast<int>(nsl_kernel_triangular));
	ui.cbKernelType->addItem(i18n("Parabolic (Epanechnikov)"), static_cast<int>(nsl_kernel_parabolic));
	ui.cbKernelType->addItem(i18n("Quartic (Biweight)"), static_cast<int>(nsl_kernel_quartic));
	ui.cbKernelType->addItem(i18n("Triweight"), static_cast<int>(nsl_kernel_triweight));
	ui.cbKernelType->addItem(i18n("Tricube"), static_cast<int>(nsl_kernel_tricube));
	ui.cbKernelType->addItem(i18n("Cosine"), static_cast<int>(nsl_kernel_cosine));

	ui.cbBandwidthType->clear();
	ui.cbBandwidthType->addItem(i18n("Silverman"), static_cast<int>(nsl_kde_bandwidth_silverman));
	ui.cbBandwidthType->addItem(i18n("Scott"), static_cast<int>(nsl_kde_bandwidth_scott));
	ui.cbBandwidthType->addItem(i18n("Custom"), static_cast<int>(nsl_kde_bandwidth_custom));
}

/*
 * updates the locale in the widgets. called when the application settins are changed.
 */
void KDEPlotDock::updateLocale() {
	estimationLineWidget->updateLocale();
}

//*************************************************************
//********* SLOTs for changes triggered in KDEPlotDock ********
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

void KDEPlotDock::kernelTypeChanged(int index) {
	const auto type = static_cast<nsl_kernel_type>(ui.cbKernelType->itemData(index).toInt());

	CONDITIONAL_LOCK_RETURN;

	for (auto* plot : m_plots)
		plot->setKernelType(type);
}

void KDEPlotDock::bandwidthTypeChanged(int index) {
	const auto type = static_cast<nsl_kde_bandwidth_type>(ui.cbBandwidthType->itemData(index).toInt());

	bool custom = (type == nsl_kde_bandwidth_custom);
	ui.lBandwidth->setVisible(custom);
	ui.sbBandwidth->setVisible(custom);

	CONDITIONAL_LOCK_RETURN;

	for (auto* plot : m_plots)
		plot->setBandwidthType(type);
}

void KDEPlotDock::bandwidthChanged(double value) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* plot : m_plots)
		plot->setBandwidth(value);
}

//"Margin Plots"-Tab
void KDEPlotDock::rugEnabledChanged(bool state) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* plot : m_plots)
		plot->rugCurve()->setRugEnabled(state);
}

void KDEPlotDock::rugLengthChanged(double value) {
	CONDITIONAL_RETURN_NO_LOCK;

	const double length = Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point);
	for (auto* plot : m_plots)
		plot->rugCurve()->setRugLength(length);
}

void KDEPlotDock::rugWidthChanged(double value) {
	CONDITIONAL_RETURN_NO_LOCK;

	const double width = Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point);
	for (auto* plot : m_plots)
		plot->rugCurve()->setRugWidth(width);
}

void KDEPlotDock::rugOffsetChanged(double value) {
	CONDITIONAL_RETURN_NO_LOCK;

	const double offset = Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point);
	for (auto* plot : m_plots)
		plot->rugCurve()->setRugOffset(offset);
}

//*************************************************************
//*********** SLOTs for changes triggered in KDEPlot **********
//*************************************************************
// General-Tab
void KDEPlotDock::plotDataColumnChanged(const AbstractColumn* column) {
	CONDITIONAL_LOCK_RETURN;
	cbDataColumn->setColumn(column, m_plot->dataColumnPath());
}

void KDEPlotDock::plotKernelTypeChanged(nsl_kernel_type type) {
	CONDITIONAL_LOCK_RETURN;
	int index = ui.cbKernelType->findData(static_cast<int>(type));
	ui.cbKernelType->setCurrentIndex(index);
}

void KDEPlotDock::plotBandwidthTypeChanged(nsl_kde_bandwidth_type type) {
	CONDITIONAL_LOCK_RETURN;
	int index = ui.cbBandwidthType->findData(static_cast<int>(type));
	ui.cbBandwidthType->setCurrentIndex(index);
}

void KDEPlotDock::plotBandwidthChanged(double value) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbBandwidth->setValue(value);
}

//"Margin Plot"-Tab
void KDEPlotDock::plotRugEnabledChanged(bool status) {
	CONDITIONAL_LOCK_RETURN;
	ui.chkRugEnabled->setChecked(status);
}
void KDEPlotDock::plotRugLengthChanged(double value) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbRugLength->setValue(Worksheet::convertFromSceneUnits(value, Worksheet::Unit::Point));
}
void KDEPlotDock::plotRugWidthChanged(double value) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbRugWidth->setValue(Worksheet::convertFromSceneUnits(value, Worksheet::Unit::Point));
}
void KDEPlotDock::plotRugOffsetChanged(double value) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbRugOffset->setValue(Worksheet::convertFromSceneUnits(value, Worksheet::Unit::Point));
}

//*************************************************************
//************************* Settings **************************
//*************************************************************
void KDEPlotDock::load() {
	// general
	cbDataColumn->setColumn(m_plot->dataColumn(), m_plot->dataColumnPath());

	int index = ui.cbKernelType->findData(static_cast<int>(m_plot->kernelType()));
	ui.cbKernelType->setCurrentIndex(index);

	index = ui.cbBandwidthType->findData(static_cast<int>(m_plot->bandwidthType()));
	ui.cbBandwidthType->setCurrentIndex(index);

	ui.sbBandwidth->setValue(m_plot->bandwidth());

	// Margin plots
	const auto* curve = m_plot->rugCurve();
	ui.chkRugEnabled->setChecked(curve->rugEnabled());
	ui.sbRugWidth->setValue(Worksheet::convertFromSceneUnits(curve->rugWidth(), Worksheet::Unit::Point));
	ui.sbRugLength->setValue(Worksheet::convertFromSceneUnits(curve->rugLength(), Worksheet::Unit::Point));
	ui.sbRugOffset->setValue(Worksheet::convertFromSceneUnits(curve->rugOffset(), Worksheet::Unit::Point));
}

void KDEPlotDock::loadConfig(KConfig& config) {
	KConfigGroup group = config.group(QStringLiteral("KDEPlot"));

	// general
	auto kernelType = group.readEntry(QStringLiteral("kernelType"), static_cast<int>(m_plot->kernelType()));
	int index = ui.cbKernelType->findData(kernelType);
	ui.cbKernelType->setCurrentIndex(index);

	auto bandwidthType = group.readEntry(QStringLiteral("bandwidthType"), static_cast<int>(m_plot->bandwidthType()));
	index = ui.cbBandwidthType->findData(bandwidthType);
	ui.cbBandwidthType->setCurrentIndex(index);

	ui.sbBandwidth->setValue(group.readEntry(QStringLiteral("bandwidth"), m_plot->bandwidth()));

	// properties of the estimation and margin curves
	// lineWidget->loadConfig(group);
}

void KDEPlotDock::loadConfigFromTemplate(KConfig& config) {
	auto name = TemplateHandler::templateName(config);
	int size = m_plots.size();
	if (size > 1)
		m_plot->beginMacro(i18n("%1 xy-curves: template \"%2\" loaded", size, name));
	else
		m_plot->beginMacro(i18n("%1: template \"%2\" loaded", m_plot->name(), name));

	this->loadConfig(config);

	m_plot->endMacro();
}

void KDEPlotDock::saveConfigAsTemplate(KConfig& config) {
	KConfigGroup group = config.group(QStringLiteral("KDEPlot"));

	// General
	group.writeEntry(QStringLiteral("kernelType"), static_cast<int>(m_plot->kernelType()));
	group.writeEntry(QStringLiteral("bandwidthType"), static_cast<int>(m_plot->bandwidthType()));
	group.writeEntry(QStringLiteral("bandwidth"), m_plot->bandwidth());

	// properties of the estimation and rug curves
	// lineWidget->saveConfig(group);

	config.sync();
}
