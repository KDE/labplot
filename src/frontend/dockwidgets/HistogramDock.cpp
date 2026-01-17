/*
	File                 : HistogramDock.cpp
	Project              : LabPlot
	Description          : widget for Histogram properties
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2016 Anu Mittal <anu22mittal@gmail.com>
	SPDX-FileCopyrightText: 2018-2024 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2021-2022 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "HistogramDock.h"
#include "backend/core/Settings.h"
#include "backend/core/column/Column.h"
#include "backend/core/datatypes/DateTime2StringFilter.h"
#include "backend/core/datatypes/Double2StringFilter.h"
#include "backend/worksheet/plots/cartesian/Histogram.h"
#include "backend/worksheet/plots/cartesian/Symbol.h"
#include "frontend/TemplateHandler.h"
#include "frontend/widgets/BackgroundWidget.h"
#include "frontend/widgets/ErrorBarWidget.h"
#include "frontend/widgets/LineWidget.h"
#include "frontend/widgets/SymbolWidget.h"
#include "frontend/widgets/TreeViewComboBox.h"
#include "frontend/widgets/ValueWidget.h"

/*!
  \class HistogramDock
  \brief  Provides a widget for editing the properties of the Histograms (2D-curves) currently selected in the project explorer.

  If more than one curves are set, the properties of the first column are shown. The changes of the properties are applied to all curves.
  The exclusions are the name, the comment and the datasets (columns) of the curves  - these properties can only be changed if there is only one single curve.

  \ingroup frontend
*/
HistogramDock::HistogramDock(QWidget* parent)
	: BaseDock(parent) {
	ui.setupUi(this);
	setPlotRangeCombobox(ui.cbPlotRanges);
	setBaseWidgets(ui.leName, ui.teComment);
	setVisibilityWidgets(ui.chkVisible, ui.chkLegendVisible);

	// Tab "General"
	cbDataColumn = new TreeViewComboBox(ui.tabGeneral);
	auto* gridLayout = qobject_cast<QGridLayout*>(ui.tabGeneral->layout());
	gridLayout->addWidget(cbDataColumn, 3, 2, 1, 1);

	// Tab "Shape"
	auto* layout = static_cast<QHBoxLayout*>(ui.tabLine->layout());
	lineWidget = new LineWidget(ui.tabLine);
	layout->insertWidget(0, lineWidget);

	// Tab "Symbols"
	auto* hboxLayout = new QHBoxLayout(ui.tabSymbol);
	symbolWidget = new SymbolWidget(ui.tabSymbol);
	hboxLayout->addWidget(symbolWidget);
	hboxLayout->setContentsMargins(2, 2, 2, 2);
	hboxLayout->setSpacing(2);

	// Tab "Values"
	hboxLayout = new QHBoxLayout(ui.tabValues);
	valueWidget = new ValueWidget(ui.tabValues);
	hboxLayout->addWidget(valueWidget);
	hboxLayout->setContentsMargins(2, 2, 2, 2);
	hboxLayout->setSpacing(2);

	// Tab "Filling"
	layout = static_cast<QHBoxLayout*>(ui.tabAreaFilling->layout());
	backgroundWidget = new BackgroundWidget(ui.tabAreaFilling);
	layout->insertWidget(0, backgroundWidget);

	// Tab "Error Bars"
	const auto group = Settings::group(QStringLiteral("Settings_General"));
	if (group.readEntry(QStringLiteral("GUMTerms"), false))
		ui.tabWidget->setTabText(ui.tabWidget->indexOf(ui.tabErrorBars), i18n("Uncertainty Bars"));

	errorBarWidget = new ErrorBarWidget(ui.tabErrorBars, true);
	auto* vLayout = qobject_cast<QVBoxLayout*>(ui.tabErrorBars->layout());
	vLayout->insertWidget(0, errorBarWidget);

	// adjust layouts in the tabs
	for (int i = 0; i < ui.tabWidget->count(); ++i) {
		auto* tabLayout = dynamic_cast<QGridLayout*>(ui.tabWidget->widget(i)->layout());
		if (!tabLayout)
			continue;

		tabLayout->setContentsMargins(2, 2, 2, 2);
		tabLayout->setHorizontalSpacing(2);
		tabLayout->setVerticalSpacing(2);
	}

	updateLocale();
	retranslateUi();

	// Slots
	// General
	connect(cbDataColumn, &TreeViewComboBox::currentModelIndexChanged, this, &HistogramDock::dataColumnChanged);
	connect(ui.cbType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &HistogramDock::typeChanged);
	connect(ui.cbOrientation, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &HistogramDock::orientationChanged);
	connect(ui.cbNormalization, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &HistogramDock::normalizationChanged);
	connect(ui.cbBinningMethod, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &HistogramDock::binningMethodChanged);
	connect(ui.sbBinCount, QOverload<int>::of(&QSpinBox::valueChanged), this, &HistogramDock::binCountChanged);
	connect(ui.sbBinWidth, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &HistogramDock::binWidthChanged);
	connect(ui.chkAutoBinRanges, &QCheckBox::toggled, this, &HistogramDock::autoBinRangesChanged);
	connect(ui.sbBinRangesMin, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &HistogramDock::binRangesMinChanged);
	connect(ui.sbBinRangesMax, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &HistogramDock::binRangesMaxChanged);
	connect(ui.dteBinRangesMin, &UTCDateTimeEdit::mSecsSinceEpochUTCChanged, this, &HistogramDock::binRangesMinDateTimeChanged);
	connect(ui.dteBinRangesMax, &UTCDateTimeEdit::mSecsSinceEpochUTCChanged, this, &HistogramDock::binRangesMaxDateTimeChanged);

	// Margin Plots
	connect(ui.chkRugEnabled, &QCheckBox::toggled, this, &HistogramDock::rugEnabledChanged);
	connect(ui.sbRugLength, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &HistogramDock::rugLengthChanged);
	connect(ui.sbRugWidth, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &HistogramDock::rugWidthChanged);
	connect(ui.sbRugOffset, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &HistogramDock::rugOffsetChanged);

	// template handler
	auto* frame = new QFrame(this);
	layout = new QHBoxLayout(frame);
	layout->setContentsMargins(0, 0, 0, 0);

	auto* templateHandler = new TemplateHandler(this, QLatin1String("Histogram"));
	layout->addWidget(templateHandler);
	connect(templateHandler, &TemplateHandler::loadConfigRequested, this, &HistogramDock::loadConfigFromTemplate);
	connect(templateHandler, &TemplateHandler::saveConfigRequested, this, &HistogramDock::saveConfigAsTemplate);
	connect(templateHandler, &TemplateHandler::info, this, &HistogramDock::info);

	ui.verticalLayout->addWidget(frame);
}

HistogramDock::~HistogramDock() = default;

void HistogramDock::setModel() {
	auto* model = aspectModel();
	model->enablePlottableColumnsOnly(true);
	model->enableShowPlotDesignation(true);
	model->setSelectableAspects({AspectType::Column});
	cbDataColumn->setTopLevelClasses(TreeViewComboBox::plotColumnTopLevelClasses());
	cbDataColumn->setModel(model);
	errorBarWidget->setModel(model);
}

void HistogramDock::setCurves(QList<Histogram*> list) {
	CONDITIONAL_LOCK_RETURN;
	m_curvesList = list;
	m_curve = list.first();
	setAspects(list);
	Q_ASSERT(m_curve);
	setModel();

	// initialize widgets for common properties
	QList<Line*> lines;
	QList<Symbol*> symbols;
	QList<Background*> backgrounds;
	QList<Value*> values;
	QList<ErrorBar*> errorBars;
	for (auto* hist : m_curvesList) {
		lines << hist->line();
		symbols << hist->symbol();
		backgrounds << hist->background();
		values << hist->value();
		errorBars << hist->errorBar();
	}
	lineWidget->setLines(lines);
	symbolWidget->setSymbols(symbols);
	backgroundWidget->setBackgrounds(backgrounds);
	valueWidget->setValues(values);
	errorBarWidget->setErrorBars(errorBars);

	// if there are more than one curve in the list, disable the content in the tab "general"
	if (m_curvesList.size() == 1) {
		cbDataColumn->setEnabled(true);
		cbDataColumn->setAspect(m_curve->dataColumn(), m_curve->dataColumnPath());
	} else {
		cbDataColumn->setEnabled(false);
		cbDataColumn->setCurrentModelIndex(QModelIndex());
	}

	// show the properties of the first curve
	const auto numberLocale = QLocale();
	ui.cbType->setCurrentIndex(m_curve->type());
	ui.cbOrientation->setCurrentIndex(static_cast<int>(m_curve->orientation()));
	ui.cbNormalization->setCurrentIndex(m_curve->normalization());
	ui.cbBinningMethod->setCurrentIndex(m_curve->binningMethod());
	ui.sbBinCount->setValue(m_curve->binCount());
	ui.sbBinWidth->setValue(m_curve->binWidth());
	ui.chkAutoBinRanges->setChecked(m_curve->autoBinRanges());
	ui.sbBinRangesMin->setValue(m_curve->binRangesMin());
	ui.sbBinRangesMax->setValue(m_curve->binRangesMax());
	ui.chkLegendVisible->setChecked(m_curve->legendVisible());
	ui.chkVisible->setChecked(m_curve->isVisible());

	// handle numeric vs. datetime widgets
	// TODO: we need to react on range format changes in the plot in general,
	// add signal-slot connection for this
	const auto* plot = static_cast<const CartesianPlot*>(m_curve->parent<CartesianPlot>());
	ui.dteBinRangesMin->setDisplayFormat(plot->rangeDateTimeFormat(Dimension::X));
	ui.dteBinRangesMax->setDisplayFormat(plot->rangeDateTimeFormat(Dimension::X));
	ui.dteBinRangesMin->setMSecsSinceEpochUTC(m_curve->binRangesMin());
	ui.dteBinRangesMax->setMSecsSinceEpochUTC(m_curve->binRangesMax());

	bool numeric = (plot->xRangeFormatDefault() == RangeT::Format::Numeric);

	ui.lBinRangesMin->setVisible(numeric);
	ui.lBinRangesMax->setVisible(numeric);
	ui.sbBinRangesMin->setVisible(numeric);
	ui.sbBinRangesMax->setVisible(numeric);

	ui.lBinRangesMinDateTime->setVisible(!numeric);
	ui.dteBinRangesMin->setVisible(!numeric);
	ui.lBinRangesMaxDateTime->setVisible(!numeric);
	ui.dteBinRangesMax->setVisible(!numeric);

	// load the remaining properties
	load();

	updatePlotRangeList();

	// Slots
	// General-tab
	connect(m_curve, &Histogram::dataColumnChanged, this, &HistogramDock::curveDataColumnChanged);
	connect(m_curve, &Histogram::typeChanged, this, &HistogramDock::curveTypeChanged);
	connect(m_curve, &Histogram::orientationChanged, this, &HistogramDock::curveOrientationChanged);
	connect(m_curve, &Histogram::normalizationChanged, this, &HistogramDock::curveNormalizationChanged);
	connect(m_curve, &Histogram::binningMethodChanged, this, &HistogramDock::curveBinningMethodChanged);
	connect(m_curve, &Histogram::binCountChanged, this, &HistogramDock::curveBinCountChanged);
	connect(m_curve, &Histogram::binWidthChanged, this, &HistogramDock::curveBinWidthChanged);
	connect(m_curve, &Histogram::autoBinRangesChanged, this, &HistogramDock::curveAutoBinRangesChanged);
	connect(m_curve, &Histogram::binRangesMinChanged, this, &HistogramDock::curveBinRangesMinChanged);
	connect(m_curve, &Histogram::binRangesMaxChanged, this, &HistogramDock::curveBinRangesMaxChanged);

	//"Margin Plots"-Tab
	connect(m_curve, &Histogram::rugEnabledChanged, this, &HistogramDock::curveRugEnabledChanged);
	connect(m_curve, &Histogram::rugLengthChanged, this, &HistogramDock::curveRugLengthChanged);
	connect(m_curve, &Histogram::rugWidthChanged, this, &HistogramDock::curveRugWidthChanged);
	connect(m_curve, &Histogram::rugOffsetChanged, this, &HistogramDock::curveRugOffsetChanged);
}

void HistogramDock::retranslateUi() {
	CONDITIONAL_LOCK_RETURN;

	// General
	// bins option
	ui.cbBinningMethod->clear();
	ui.cbBinningMethod->addItem(i18n("By Number"));
	ui.cbBinningMethod->addItem(i18n("By Width"));
	ui.cbBinningMethod->addItem(i18n("Square-root"));
	ui.cbBinningMethod->addItem(i18n("Rice")); // here and for the next three items, this is the name of the author, not sure it should be translated
	ui.cbBinningMethod->addItem(i18n("Sturges"));
	ui.cbBinningMethod->addItem(i18n("Doane"));
	ui.cbBinningMethod->addItem(i18n("Scott"));

	// histogram type
	ui.cbType->clear();
	ui.cbType->addItem(i18n("Ordinary Histogram"));
	ui.cbType->addItem(i18n("Cumulative Histogram"));
	// 	ui.cbType->addItem(i18n("AvgShifted Histogram"));

	// Orientation
	ui.cbOrientation->clear();
	ui.cbOrientation->addItem(i18n("Vertical"));
	ui.cbOrientation->addItem(i18n("Horizontal"));

	// Normalization
	ui.cbNormalization->clear();
	ui.cbNormalization->addItem(i18n("Count"));
	ui.cbNormalization->addItem(i18n("Probability"));
	ui.cbNormalization->addItem(i18n("Count Density"));
	ui.cbNormalization->addItem(i18n("Probability Density"));

	// TODO lineWidget->retranslateUi();

	// tooltip texts
	QString info = i18n(
		"Method used to determine the number of bins <i>k</i> and their width <i>h</i> for <i>n</i> values:"
		"<ul>"
		"<li>By Number - the number of bins is specified manually</li>"
		"<li>By Width - the number of bins is calculated based on the specified bin width via <i>k = (max(x) - min(x) / h</i>)</li>"
		"<li>Square-root - <i>k = sqrt(n)</i></li>"
		"<li>Rice -  <i>k = 2 * pow(n, 3/2)</i>, simpler alternative to Sturges' method</li>"
		"<li>Sturges - <i>k = log2(n) + 1</i>, assumes an approximately normal distribution</li>"
		"<li>Doane - modified version of Sturges' method, see the documentation for more details</li>"
		"<li>Scott - <i>h = 3.49 * sigma / pow(n, 3/2)</i>, optimal method for normally distributed data</li>"
		"</ul>");
	ui.lBinningMethod->setToolTip(info);
	ui.cbBinningMethod->setToolTip(info);

	info = i18n("Use 'Auto' to automatically determine the minimal and maximal values of the data to be used to calculate the histogram. Specify the values manually, otherwise."
		"<br><br>"
		"<b>Note:</b> any samples which fall on the upper end of the histogram are excluded. If you want to include these values for the last bin you will need to add an extra bin to your histogram."
	);
	ui.lBinRanges->setToolTip(info);
	ui.chkAutoBinRanges->setToolTip(info);
}

void HistogramDock::updateLocale() {
	lineWidget->updateLocale();
	symbolWidget->updateLocale();
	errorBarWidget->updateLocale();
}

//*************************************************************
//**** SLOTs for changes triggered in HistogramDock *****
//*************************************************************

// "General"-tab
void HistogramDock::typeChanged(int index) {
	CONDITIONAL_LOCK_RETURN;

	auto histogramType = Histogram::Type(index);
	for (auto* curve : m_curvesList)
		curve->setType(histogramType);
}

void HistogramDock::dataColumnChanged(const QModelIndex& index) {
	CONDITIONAL_LOCK_RETURN;

	auto aspect = static_cast<AbstractAspect*>(index.internalPointer());
	AbstractColumn* column(nullptr);
	if (aspect) {
		column = dynamic_cast<AbstractColumn*>(aspect);
		Q_ASSERT(column);
	}

	for (auto* curve : m_curvesList)
		curve->setDataColumn(column);
}

void HistogramDock::orientationChanged(int index) {
	CONDITIONAL_LOCK_RETURN;

	auto orientation = Histogram::Orientation(index);
	for (auto* curve : m_curvesList)
		curve->setOrientation(orientation);
}

void HistogramDock::normalizationChanged(int index) {
	CONDITIONAL_LOCK_RETURN;

	auto normalization = Histogram::Normalization(index);
	for (auto* curve : m_curvesList)
		curve->setNormalization(normalization);
}

void HistogramDock::binningMethodChanged(int index) {
	const auto binningMethod = Histogram::BinningMethod(index);
	if (binningMethod == Histogram::ByNumber) {
		ui.lBinCount->show();
		ui.sbBinCount->show();
		ui.lBinWidth->hide();
		ui.sbBinWidth->hide();
	} else if (binningMethod == Histogram::ByWidth) {
		ui.lBinCount->hide();
		ui.sbBinCount->hide();
		ui.lBinWidth->show();
		ui.sbBinWidth->show();
	} else {
		ui.lBinCount->hide();
		ui.sbBinCount->hide();
		ui.lBinWidth->hide();
		ui.sbBinWidth->hide();
	}

	CONDITIONAL_LOCK_RETURN;

	for (auto* curve : m_curvesList)
		curve->setBinningMethod(binningMethod);
}

void HistogramDock::binCountChanged(int value) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* curve : m_curvesList)
		curve->setBinCount(value);
}

void HistogramDock::binWidthChanged(double width) {
	CONDITIONAL_RETURN_NO_LOCK;

	for (auto* curve : m_curvesList)
		curve->setBinWidth(width);
}

void HistogramDock::autoBinRangesChanged(bool state) {
	ui.sbBinRangesMin->setEnabled(!state);
	ui.sbBinRangesMax->setEnabled(!state);
	ui.dteBinRangesMin->setEnabled(!state);
	ui.dteBinRangesMax->setEnabled(!state);

	CONDITIONAL_RETURN_NO_LOCK;

	for (auto* hist : m_curvesList)
		hist->setAutoBinRanges(state);
}

void HistogramDock::binRangesMinChanged(double value) {
	CONDITIONAL_RETURN_NO_LOCK;

	for (auto* hist : m_curvesList)
		hist->setBinRangesMin(value);
}

void HistogramDock::binRangesMaxChanged(double value) {
	CONDITIONAL_RETURN_NO_LOCK;

	for (auto* hist : m_curvesList)
		hist->setBinRangesMax(value);
}

void HistogramDock::binRangesMinDateTimeChanged(qint64 value) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* hist : m_curvesList)
		hist->setBinRangesMin(value);
}

void HistogramDock::binRangesMaxDateTimeChanged(qint64 value) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* hist : m_curvesList)
		hist->setBinRangesMax(value);
}

//"Margin Plots"-Tab
void HistogramDock::rugEnabledChanged(bool state) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* curve : std::as_const(m_curvesList))
		curve->setRugEnabled(state);
}

void HistogramDock::rugLengthChanged(double value) const {
	CONDITIONAL_RETURN_NO_LOCK;

	const double length = Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point);
	for (auto* curve : std::as_const(m_curvesList))
		curve->setRugLength(length);
}

void HistogramDock::rugWidthChanged(double value) const {
	CONDITIONAL_RETURN_NO_LOCK;

	const double width = Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point);
	for (auto* curve : std::as_const(m_curvesList))
		curve->setRugWidth(width);
}

void HistogramDock::rugOffsetChanged(double value) const {
	CONDITIONAL_RETURN_NO_LOCK;

	const double offset = Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point);
	for (auto* curve : std::as_const(m_curvesList))
		curve->setRugOffset(offset);
}

//*************************************************************
//*********** SLOTs for changes triggered in Histogram *******
//*************************************************************
// General-Tab
void HistogramDock::curveDataColumnChanged(const AbstractColumn* column) {
	CONDITIONAL_LOCK_RETURN;
	cbDataColumn->setAspect(column, m_curve->dataColumnPath());
}

void HistogramDock::curveTypeChanged(Histogram::Type type) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbType->setCurrentIndex((int)type);
}

void HistogramDock::curveOrientationChanged(Histogram::Orientation orientation) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbOrientation->setCurrentIndex((int)orientation);
}

void HistogramDock::curveNormalizationChanged(Histogram::Normalization normalization) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbNormalization->setCurrentIndex((int)normalization);
}

void HistogramDock::curveBinningMethodChanged(Histogram::BinningMethod method) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbBinningMethod->setCurrentIndex((int)method);
}

void HistogramDock::curveBinCountChanged(int count) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbBinCount->setValue(count);
}

void HistogramDock::curveBinWidthChanged(double width) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbBinWidth->setValue(width);
}

void HistogramDock::curveAutoBinRangesChanged(bool value) {
	CONDITIONAL_LOCK_RETURN;
	ui.chkAutoBinRanges->setChecked(value);
}

void HistogramDock::curveBinRangesMinChanged(double value) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbBinRangesMin->setValue(value);
	ui.dteBinRangesMin->setMSecsSinceEpochUTC(value);
}

void HistogramDock::curveBinRangesMaxChanged(double value) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbBinRangesMax->setValue(value);
	ui.dteBinRangesMax->setMSecsSinceEpochUTC(value);
}

//"Margin Plot"-Tab
void HistogramDock::curveRugEnabledChanged(bool status) {
	CONDITIONAL_LOCK_RETURN;
	ui.chkRugEnabled->setChecked(status);
}
void HistogramDock::curveRugLengthChanged(double value) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbRugLength->setValue(Worksheet::convertFromSceneUnits(value, Worksheet::Unit::Point));
}
void HistogramDock::curveRugWidthChanged(double value) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbRugWidth->setValue(Worksheet::convertFromSceneUnits(value, Worksheet::Unit::Point));
}
void HistogramDock::curveRugOffsetChanged(double value) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbRugOffset->setValue(Worksheet::convertFromSceneUnits(value, Worksheet::Unit::Point));
}

//*************************************************************
//************************* Settings **************************
//*************************************************************
void HistogramDock::load() {
	// General
	// we don't load/save the settings in the general-tab, since they are not style related.
	// It doesn't make sense to load/save them in the template.
	// This data is read in HistogramDock::setCurves().

	// Margin plots
	ui.chkRugEnabled->setChecked(m_curve->rugEnabled());
	ui.sbRugWidth->setValue(Worksheet::convertFromSceneUnits(m_curve->rugWidth(), Worksheet::Unit::Point));
	ui.sbRugLength->setValue(Worksheet::convertFromSceneUnits(m_curve->rugLength(), Worksheet::Unit::Point));
	ui.sbRugOffset->setValue(Worksheet::convertFromSceneUnits(m_curve->rugOffset(), Worksheet::Unit::Point));
}

void HistogramDock::loadConfig(KConfig& config) {
	KConfigGroup group = config.group(QStringLiteral("Histogram"));

	// General
	// we don't load/save the settings in the general-tab, since they are not style related.
	// It doesn't make sense to load/save them in the template.
	// This data is read in HistogramDock::setCurves().

	lineWidget->loadConfig(group);
	symbolWidget->loadConfig(group);
	valueWidget->loadConfig(group);
	backgroundWidget->loadConfig(group);
	errorBarWidget->loadConfig(group);

	// Margin plots
	ui.chkRugEnabled->setChecked(m_curve->rugEnabled());
	ui.sbRugWidth->setValue(Worksheet::convertFromSceneUnits(m_curve->rugWidth(), Worksheet::Unit::Point));
	ui.sbRugLength->setValue(Worksheet::convertFromSceneUnits(m_curve->rugLength(), Worksheet::Unit::Point));
	ui.sbRugOffset->setValue(Worksheet::convertFromSceneUnits(m_curve->rugOffset(), Worksheet::Unit::Point));
}

void HistogramDock::loadConfigFromTemplate(KConfig& config) {
	auto name = TemplateHandler::templateName(config);
	int size = m_curvesList.size();
	if (size > 1)
		m_curve->beginMacro(i18n("%1 histograms: template \"%2\" loaded", size, name));
	else
		m_curve->beginMacro(i18n("%1: template \"%2\" loaded", m_curve->name(), name));

	this->loadConfig(config);

	m_curve->endMacro();
}

void HistogramDock::saveConfigAsTemplate(KConfig& config) {
	KConfigGroup group = config.group(QStringLiteral("Histogram"));

	lineWidget->saveConfig(group);
	symbolWidget->saveConfig(group);
	valueWidget->saveConfig(group);
	backgroundWidget->saveConfig(group);
	errorBarWidget->saveConfig(group);

	config.sync();
}
