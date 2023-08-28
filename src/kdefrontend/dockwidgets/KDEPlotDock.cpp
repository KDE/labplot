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
#include "backend/worksheet/plots/cartesian/KDEPlot.h"
#include "commonfrontend/widgets/TreeViewComboBox.h"
#include "kdefrontend/GuiTools.h"
#include "kdefrontend/TemplateHandler.h"
#include "kdefrontend/widgets/BackgroundWidget.h"
#include "kdefrontend/widgets/LineWidget.h"

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
	hBoxLayout->insertWidget(5, estimationBackgroundWidget);

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
	connect(cbDataColumn, &TreeViewComboBox::currentModelIndexChanged, this, &KDEPlotDock::dataColumnChanged);
	connect(ui.cbKernelType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &KDEPlotDock::kernelTypeChanged);
	connect(ui.cbBandwidthType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &KDEPlotDock::bandwidthTypeChanged);
	connect(ui.sbBandwidth, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &KDEPlotDock::bandwidthChanged);

	connect(ui.chkVisible, &QCheckBox::clicked, this, &KDEPlotDock::visibilityChanged);
	connect(ui.cbPlotRanges, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &KDEPlotDock::plotRangeChanged);

	// Margin Plots
	connect(ui.chkRugEnabled, &QCheckBox::toggled, this, &KDEPlotDock::rugEnabledChanged);
	connect(ui.sbRugLength, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &KDEPlotDock::rugLengthChanged);
	connect(ui.sbRugWidth, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &KDEPlotDock::rugWidthChanged);
	connect(ui.sbRugOffset, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &KDEPlotDock::rugOffsetChanged);

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
		estimationBackgrounds << plot->estimationCurve()->background();
		histogramLines << plot->histogram()->line();
		histogramBackgrounds << plot->histogram()->background();
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
	ui.cbKernelType->addItem(i18n("Binomial"), static_cast<int>(nsl_kernel_binomial));
	ui.cbKernelType->addItem(i18n("Parabolic (Epanechnikov)"), static_cast<int>(nsl_kernel_parabolic));
	ui.cbKernelType->addItem(i18n("Quartic (Biweight)"), static_cast<int>(nsl_kernel_quartic));
	ui.cbKernelType->addItem(i18n("Triweight"), static_cast<int>(nsl_kernel_triweight));
	ui.cbKernelType->addItem(i18n("Tricube"), static_cast<int>(nsl_kernel_tricube));
	ui.cbKernelType->addItem(i18n("Cosine"), static_cast<int>(nsl_kernel_cosine));

	ui.cbBandwidthType->clear();
	ui.cbBandwidthType->addItem(i18n("Gaussian approximation"), static_cast<int>(nsl_kde_bandwidth_gaussian));
	ui.cbBandwidthType->addItem(i18n("Custom"), static_cast<int>(nsl_kde_bandwidth_custom));
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
	const nsl_kernel_type type = static_cast<nsl_kernel_type>(ui.cbKernelType->itemData(index).toInt());

	CONDITIONAL_LOCK_RETURN

	for (auto* plot : m_plots)
		plot->setKernelType(type);
}

void KDEPlotDock::bandwidthTypeChanged(int index) {
	const nsl_kde_bandwidth_type type = static_cast<nsl_kde_bandwidth_type>(ui.cbBandwidthType->itemData(index).toInt());

	bool custom = (type == nsl_kde_bandwidth_custom);
	ui.lBandwidth->setEnabled(custom);
	ui.sbBandwidth->setEnabled(custom);

	CONDITIONAL_LOCK_RETURN

	for (auto* plot : m_plots)
		plot->setBandwidthType(type);

	if (custom)
		ui.sbBandwidth->setValue(m_plot->bandwidth());
}

void KDEPlotDock::bandwidthChanged(double value) {
	CONDITIONAL_LOCK_RETURN

	for (auto* plot : m_plots)
		plot->setBandwidth(value);
}

void KDEPlotDock::visibilityChanged(bool state) {
	if (m_initializing)
		return;

	for (auto* plot : m_plots)
		plot->setVisible(state);
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
	CONDITIONAL_LOCK_RETURN
	cbDataColumn->setColumn(column, m_plot->dataColumnPath());
}

void KDEPlotDock::plotKernelTypeChanged(nsl_kernel_type type) {
	CONDITIONAL_LOCK_RETURN
	int index = ui.cbKernelType->findData(static_cast<int>(type));
	ui.cbKernelType->setCurrentIndex(index);
}

void KDEPlotDock::plotBandwidthTypeChanged(nsl_kde_bandwidth_type type) {
	CONDITIONAL_LOCK_RETURN
	int index = ui.cbBandwidthType->findData(static_cast<int>(type));
	ui.cbBandwidthType->setCurrentIndex(index);
}

void KDEPlotDock::plotBandwidthChanged(double value) {
	CONDITIONAL_LOCK_RETURN
	ui.sbBandwidth->setValue(value);
}

void KDEPlotDock::plotVisibilityChanged(bool on) {
	CONDITIONAL_LOCK_RETURN
	ui.chkVisible->setChecked(on);
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
