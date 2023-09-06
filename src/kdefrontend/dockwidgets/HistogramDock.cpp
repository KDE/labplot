/*
	File                 : HistogramDock.cpp
	Project              : LabPlot
	Description          : widget for Histogram properties
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2016 Anu Mittal <anu22mittal@gmail.com>
	SPDX-FileCopyrightText: 2018-2023 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2021-2022 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "HistogramDock.h"
#include "backend/core/Settings.h"
#include "backend/core/column/Column.h"
#include "backend/core/datatypes/DateTime2StringFilter.h"
#include "backend/core/datatypes/Double2StringFilter.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/Histogram.h"
#include "backend/worksheet/plots/cartesian/Symbol.h"
#include "commonfrontend/widgets/TreeViewComboBox.h"
#include "kdefrontend/GuiTools.h"
#include "kdefrontend/TemplateHandler.h"
#include "kdefrontend/widgets/BackgroundWidget.h"
#include "kdefrontend/widgets/LineWidget.h"
#include "kdefrontend/widgets/SymbolWidget.h"
#include "kdefrontend/widgets/ValueWidget.h"

#include <QCompleter>

#include <KConfig>
#include <KLocalizedString>

/*!
  \class HistogramDock
  \brief  Provides a widget for editing the properties of the Histograms (2D-curves) currently selected in the project explorer.

  If more than one curves are set, the properties of the first column are shown. The changes of the properties are applied to all curves.
  The exclusions are the name, the comment and the datasets (columns) of the curves  - these properties can only be changed if there is only one single curve.

  \ingroup kdefrontend
*/
HistogramDock::HistogramDock(QWidget* parent)
	: BaseDock(parent)
	, cbDataColumn(new TreeViewComboBox) {
	ui.setupUi(this);
	m_leName = ui.leName;
	m_teComment = ui.teComment;
	m_teComment->setFixedHeight(2 * m_leName->height());

	// Tab "General"
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
	const KConfigGroup group = Settings::group(QStringLiteral("Settings_General"));
	if (group.readEntry("GUMTerms", false)) {
		ui.tabWidget->setTabText(ui.tabWidget->indexOf(ui.tabErrorBars), i18n("Uncertainty Bars"));
		ui.lErrorBar->setText(i18n("X Uncertainty"));
	}

	gridLayout = qobject_cast<QGridLayout*>(ui.tabErrorBars->layout());

	cbErrorPlusColumn = new TreeViewComboBox(ui.tabErrorBars);
	gridLayout->addWidget(cbErrorPlusColumn, 2, 2, 1, 1);

	cbErrorMinusColumn = new TreeViewComboBox(ui.tabErrorBars);
	gridLayout->addWidget(cbErrorMinusColumn, 3, 2, 1, 1);

	errorBarsLineWidget = new LineWidget(ui.tabErrorBars);
	gridLayout->addWidget(errorBarsLineWidget, 6, 0, 1, 3);

	// adjust layouts in the tabs
	for (int i = 0; i < ui.tabWidget->count(); ++i) {
		auto* layout = dynamic_cast<QGridLayout*>(ui.tabWidget->widget(i)->layout());
		if (!layout)
			continue;

		layout->setContentsMargins(2, 2, 2, 2);
		layout->setHorizontalSpacing(2);
		layout->setVerticalSpacing(2);
	}

	// validators
	ui.leBinWidth->setValidator(new QDoubleValidator(ui.leBinWidth));
	ui.leBinRangesMin->setValidator(new QDoubleValidator(ui.leBinRangesMin));
	ui.leBinRangesMax->setValidator(new QDoubleValidator(ui.leBinRangesMax));

	// Slots
	// General
	connect(ui.leName, &QLineEdit::textChanged, this, &HistogramDock::nameChanged);
	connect(ui.teComment, &QTextEdit::textChanged, this, &HistogramDock::commentChanged);
	connect(ui.chkVisible, &QCheckBox::clicked, this, &HistogramDock::visibilityChanged);
	connect(cbDataColumn, &TreeViewComboBox::currentModelIndexChanged, this, &HistogramDock::dataColumnChanged);
	connect(ui.cbType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &HistogramDock::typeChanged);
	connect(ui.cbOrientation, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &HistogramDock::orientationChanged);
	connect(ui.cbNormalization, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &HistogramDock::normalizationChanged);
	connect(ui.cbBinningMethod, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &HistogramDock::binningMethodChanged);
	connect(ui.sbBinCount, QOverload<int>::of(&QSpinBox::valueChanged), this, &HistogramDock::binCountChanged);
	connect(ui.leBinWidth, &QLineEdit::textChanged, this, &HistogramDock::binWidthChanged);
	connect(ui.chkAutoBinRanges, &QCheckBox::toggled, this, &HistogramDock::autoBinRangesChanged);
	connect(ui.leBinRangesMin, &QLineEdit::textChanged, this, &HistogramDock::binRangesMinChanged);
	connect(ui.leBinRangesMax, &QLineEdit::textChanged, this, &HistogramDock::binRangesMaxChanged);
	connect(ui.dteBinRangesMin, &UTCDateTimeEdit::mSecsSinceEpochUTCChanged, this, &HistogramDock::binRangesMinDateTimeChanged);
	connect(ui.dteBinRangesMax, &UTCDateTimeEdit::mSecsSinceEpochUTCChanged, this, &HistogramDock::binRangesMaxDateTimeChanged);
	connect(ui.cbPlotRanges, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &HistogramDock::plotRangeChanged);

	// Error bars
	connect(ui.cbErrorType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &HistogramDock::errorTypeChanged);
	connect(cbErrorPlusColumn, &TreeViewComboBox::currentModelIndexChanged, this, &HistogramDock::errorPlusColumnChanged);
	connect(cbErrorMinusColumn, &TreeViewComboBox::currentModelIndexChanged, this, &HistogramDock::errorMinusColumnChanged);

	// Margin Plots
	connect(ui.chkRugEnabled, &QCheckBox::toggled, this, &HistogramDock::rugEnabledChanged);
	connect(ui.sbRugLength, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &HistogramDock::rugLengthChanged);
	connect(ui.sbRugWidth, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &HistogramDock::rugWidthChanged);
	connect(ui.sbRugOffset, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &HistogramDock::rugOffsetChanged);

	// template handler
	auto* frame = new QFrame(this);
	layout = new QHBoxLayout(frame);
	layout->setContentsMargins(0, 11, 0, 11);

	auto* templateHandler = new TemplateHandler(this, QLatin1String("Histogram"));
	layout->addWidget(templateHandler);
	connect(templateHandler, &TemplateHandler::loadConfigRequested, this, &HistogramDock::loadConfigFromTemplate);
	connect(templateHandler, &TemplateHandler::saveConfigRequested, this, &HistogramDock::saveConfigAsTemplate);
	connect(templateHandler, &TemplateHandler::info, this, &HistogramDock::info);

	ui.verticalLayout->addWidget(frame);

	updateLocale();
	retranslateUi();
	init();
}

HistogramDock::~HistogramDock() = default;

void HistogramDock::init() {
	// General
	// bins option
	ui.cbBinningMethod->addItem(i18n("By Number"));
	ui.cbBinningMethod->addItem(i18n("By Width"));
	ui.cbBinningMethod->addItem(i18n("Square-root"));
	ui.cbBinningMethod->addItem(i18n("Rice"));
	ui.cbBinningMethod->addItem(i18n("Sturges"));
	ui.cbBinningMethod->addItem(i18n("Doane"));
	ui.cbBinningMethod->addItem(i18n("Scott"));

	// histogram type
	ui.cbType->addItem(i18n("Ordinary Histogram"));
	ui.cbType->addItem(i18n("Cumulative Histogram"));
	// 	ui.cbType->addItem(i18n("AvgShifted Histogram"));

	// Orientation
	ui.cbOrientation->addItem(i18n("Vertical"));
	ui.cbOrientation->addItem(i18n("Horizontal"));

	// Normalization
	ui.cbNormalization->addItem(i18n("Count"));
	ui.cbNormalization->addItem(i18n("Probability"));
	ui.cbNormalization->addItem(i18n("Count Density"));
	ui.cbNormalization->addItem(i18n("Probability Density"));

	// Error-bars
	const KConfigGroup group = Settings::group(QStringLiteral("Settings_General"));
	if (group.readEntry("GUMTerms", false)) {
		ui.cbErrorType->addItem(i18n("No Uncertainties"));
		ui.cbErrorType->addItem(i18n("Poisson variance, sqrt(N)"));
		ui.cbErrorType->addItem(i18n("Custom Uncertainty Values, symmetric"));
		ui.cbErrorType->addItem(i18n("Custom Uncertainty Values, asymmetric"));
	} else {
		ui.cbErrorType->addItem(i18n("No Errors"));
		ui.cbErrorType->addItem(i18n("Poisson variance, sqrt(N)"));
		ui.cbErrorType->addItem(i18n("Custom Error Values, symmetric"));
		ui.cbErrorType->addItem(i18n("Custom Error Values, asymmetric"));
	}
}

void HistogramDock::setModel() {
	auto* model = aspectModel();
	model->enablePlottableColumnsOnly(true);
	model->enableShowPlotDesignation(true);

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
	cbErrorPlusColumn->setTopLevelClasses(list);
	cbErrorMinusColumn->setTopLevelClasses(list);

	list = {AspectType::Column};
	model->setSelectableAspects(list);

	cbDataColumn->setModel(model);
	cbErrorPlusColumn->setModel(model);
	cbErrorMinusColumn->setModel(model);
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
	QList<Line*> errorBarLines;
	for (auto* hist : m_curvesList) {
		lines << hist->line();
		symbols << hist->symbol();
		backgrounds << hist->background();
		values << hist->value();
		errorBarLines << hist->errorBarsLine();
	}
	lineWidget->setLines(lines);
	symbolWidget->setSymbols(symbols);
	backgroundWidget->setBackgrounds(backgrounds);
	valueWidget->setValues(values);
	errorBarsLineWidget->setLines(errorBarLines);

	// if there are more than one curve in the list, disable the content in the tab "general"
	if (m_curvesList.size() == 1) {
		cbDataColumn->setEnabled(true);
		cbDataColumn->setColumn(m_curve->dataColumn(), m_curve->dataColumnPath());
		cbErrorPlusColumn->setColumn(m_curve->errorPlusColumn(), m_curve->errorPlusColumnPath());
		cbErrorMinusColumn->setColumn(m_curve->errorMinusColumn(), m_curve->errorMinusColumnPath());
	} else {
		cbDataColumn->setEnabled(false);
		cbDataColumn->setCurrentModelIndex(QModelIndex());
		cbErrorPlusColumn->setCurrentModelIndex(QModelIndex());
		cbErrorMinusColumn->setCurrentModelIndex(QModelIndex());
	}

	// show the properties of the first curve
	const auto numberLocale = QLocale();
	ui.cbType->setCurrentIndex(m_curve->type());
	ui.cbOrientation->setCurrentIndex(m_curve->orientation());
	ui.cbNormalization->setCurrentIndex(m_curve->normalization());
	ui.cbBinningMethod->setCurrentIndex(m_curve->binningMethod());
	ui.sbBinCount->setValue(m_curve->binCount());
	ui.leBinWidth->setText(numberLocale.toString(m_curve->binWidth()));
	ui.chkAutoBinRanges->setChecked(m_curve->autoBinRanges());
	ui.leBinRangesMin->setText(numberLocale.toString(m_curve->binRangesMin()));
	ui.leBinRangesMax->setText(numberLocale.toString(m_curve->binRangesMax()));
	ui.chkVisible->setChecked(m_curve->isVisible());

	// handle numeric vs. datetime widgets
	// TODO: we need to react on range format changes in the plot in general,
	// add signal-slot connection for this
	const auto* plot = static_cast<const CartesianPlot*>(m_curve->parent(AspectType::CartesianPlot));
	ui.dteBinRangesMin->setDisplayFormat(plot->rangeDateTimeFormat(Dimension::X));
	ui.dteBinRangesMax->setDisplayFormat(plot->rangeDateTimeFormat(Dimension::X));
	ui.dteBinRangesMin->setMSecsSinceEpochUTC(m_curve->binRangesMin());
	ui.dteBinRangesMax->setMSecsSinceEpochUTC(m_curve->binRangesMax());

	bool numeric = (plot->xRangeFormatDefault() == RangeT::Format::Numeric);

	ui.lBinRangesMin->setVisible(numeric);
	ui.lBinRangesMax->setVisible(numeric);
	ui.leBinRangesMin->setVisible(numeric);
	ui.leBinRangesMax->setVisible(numeric);

	ui.lBinRangesMinDateTime->setVisible(!numeric);
	ui.dteBinRangesMin->setVisible(!numeric);
	ui.lBinRangesMaxDateTime->setVisible(!numeric);
	ui.dteBinRangesMax->setVisible(!numeric);

	// load the remaining properties
	load();

	updatePlotRanges();

	// Slots
	// General-tab
	connect(m_curve, &Histogram::aspectDescriptionChanged, this, &HistogramDock::aspectDescriptionChanged);
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
	connect(m_curve, &Histogram::visibleChanged, this, &HistogramDock::curveVisibilityChanged);

	//"Error bars"-Tab
	connect(m_curve, &Histogram::errorTypeChanged, this, &HistogramDock::curveErrorTypeChanged);
	connect(m_curve, &Histogram::errorPlusColumnChanged, this, &HistogramDock::curveErrorPlusColumnChanged);
	connect(m_curve, &Histogram::errorMinusColumnChanged, this, &HistogramDock::curveErrorMinusColumnChanged);

	//"Margin Plots"-Tab
	connect(m_curve, &Histogram::rugEnabledChanged, this, &HistogramDock::curveRugEnabledChanged);
	connect(m_curve, &Histogram::rugLengthChanged, this, &HistogramDock::curveRugLengthChanged);
	connect(m_curve, &Histogram::rugWidthChanged, this, &HistogramDock::curveRugWidthChanged);
	connect(m_curve, &Histogram::rugOffsetChanged, this, &HistogramDock::curveRugOffsetChanged);
}

void HistogramDock::retranslateUi() {
	// TODO:
	// 	ui.lName->setText(i18n("Name"));
	// 	ui.lComment->setText(i18n("Comment"));
	// 	ui.chkVisible->setText(i18n("Visible"));
	// 	ui.lXColumn->setText(i18n("x-data"));
	// 	ui.lYColumn->setText(i18n("y-data"));

	// TODO updatePenStyles, updateBrushStyles for all comboboxes
}
void HistogramDock::updatePlotRanges() {
	const int cSystemCount{m_curve->coordinateSystemCount()};
	const int cSystemIndex{m_curve->coordinateSystemIndex()};
	DEBUG(Q_FUNC_INFO << ", plot ranges count: " << cSystemCount)
	DEBUG(Q_FUNC_INFO << ", current plot range: " << cSystemIndex + 1)

	// fill ui.cbPlotRanges
	ui.cbPlotRanges->clear();
	for (int i{0}; i < cSystemCount; i++)
		ui.cbPlotRanges->addItem(QString::number(i + 1) + QLatin1String(" : ") + m_curve->coordinateSystemInfo(i));
	ui.cbPlotRanges->setCurrentIndex(cSystemIndex);
	// disable when there is only on plot range
	ui.cbPlotRanges->setEnabled(cSystemCount == 1 ? false : true);
}

void HistogramDock::updateLocale() {
	lineWidget->updateLocale();
	symbolWidget->updateLocale();
	errorBarsLineWidget->updateLocale();
}

//*************************************************************
//**** SLOTs for changes triggered in HistogramDock *****
//*************************************************************

// "General"-tab
void HistogramDock::visibilityChanged(bool state) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* curve : m_curvesList)
		curve->setVisible(state);
}

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
		ui.leBinWidth->hide();
	} else if (binningMethod == Histogram::ByWidth) {
		ui.lBinCount->hide();
		ui.sbBinCount->hide();
		ui.lBinWidth->show();
		ui.leBinWidth->show();
	} else {
		ui.lBinCount->hide();
		ui.sbBinCount->hide();
		ui.lBinWidth->hide();
		ui.leBinWidth->hide();
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

void HistogramDock::binWidthChanged() {
	CONDITIONAL_LOCK_RETURN;

	bool ok;
	const double width{QLocale().toDouble(ui.leBinWidth->text(), &ok)};
	if (ok) {
		for (auto* curve : m_curvesList)
			curve->setBinWidth(width);
	}
}

void HistogramDock::autoBinRangesChanged(bool state) {
	ui.leBinRangesMin->setEnabled(!state);
	ui.leBinRangesMax->setEnabled(!state);
	ui.dteBinRangesMin->setEnabled(!state);
	ui.dteBinRangesMax->setEnabled(!state);

	CONDITIONAL_LOCK_RETURN;

	for (auto* hist : m_curvesList)
		hist->setAutoBinRanges(state);
}

void HistogramDock::binRangesMinChanged(const QString& value) {
	CONDITIONAL_LOCK_RETURN;

	bool ok;
	const double min{QLocale().toDouble(value, &ok)};
	if (ok) {
		for (auto* hist : m_curvesList)
			hist->setBinRangesMin(min);
	}
}

void HistogramDock::binRangesMaxChanged(const QString& value) {
	CONDITIONAL_LOCK_RETURN;

	bool ok;
	const double max{QLocale().toDouble(value, &ok)};
	if (ok) {
		for (auto* hist : m_curvesList)
			hist->setBinRangesMax(max);
	}
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

//"Error bars"-Tab
void HistogramDock::errorTypeChanged(int index) {
	if (index == 0 /* no errors */ || index == 1 /* Poisson */) {
		// no error
		ui.lErrorDataPlus->setVisible(false);
		cbErrorPlusColumn->setVisible(false);
		ui.lErrorDataMinus->setVisible(false);
		cbErrorMinusColumn->setVisible(false);
	} else if (index == 2) {
		// symmetric error
		ui.lErrorDataPlus->setVisible(true);
		cbErrorPlusColumn->setVisible(true);
		ui.lErrorDataMinus->setVisible(false);
		cbErrorMinusColumn->setVisible(false);
		ui.lErrorDataPlus->setText(i18n("Data, +-:"));
	} else if (index == 3) {
		// asymmetric error
		ui.lErrorDataPlus->setVisible(true);
		cbErrorPlusColumn->setVisible(true);
		ui.lErrorDataMinus->setVisible(true);
		cbErrorMinusColumn->setVisible(true);
		ui.lErrorDataPlus->setText(i18n("Data, +:"));
	}

	const bool b = (index != 0);
	ui.lErrorFormat->setVisible(b);
	errorBarsLineWidget->setVisible(b);

	CONDITIONAL_LOCK_RETURN;

	for (auto* curve : m_curvesList)
		curve->setErrorType(Histogram::ErrorType(index));
}

void HistogramDock::errorPlusColumnChanged(const QModelIndex& index) {
	CONDITIONAL_LOCK_RETURN;

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	auto* column = dynamic_cast<AbstractColumn*>(aspect);
	Q_ASSERT(column);

	for (auto* curve : m_curvesList)
		curve->setErrorPlusColumn(column);
}

void HistogramDock::errorMinusColumnChanged(const QModelIndex& index) {
	CONDITIONAL_LOCK_RETURN;

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	auto* column = dynamic_cast<AbstractColumn*>(aspect);
	Q_ASSERT(column);

	for (auto* curve : m_curvesList)
		curve->setErrorMinusColumn(column);
}

//"Margin Plots"-Tab
void HistogramDock::rugEnabledChanged(bool state) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* curve : qAsConst(m_curvesList))
		curve->setRugEnabled(state);
}

void HistogramDock::rugLengthChanged(double value) const {
	CONDITIONAL_RETURN_NO_LOCK;

	const double length = Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point);
	for (auto* curve : qAsConst(m_curvesList))
		curve->setRugLength(length);
}

void HistogramDock::rugWidthChanged(double value) const {
	CONDITIONAL_RETURN_NO_LOCK;

	const double width = Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point);
	for (auto* curve : qAsConst(m_curvesList))
		curve->setRugWidth(width);
}

void HistogramDock::rugOffsetChanged(double value) const {
	CONDITIONAL_RETURN_NO_LOCK;

	const double offset = Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point);
	for (auto* curve : qAsConst(m_curvesList))
		curve->setRugOffset(offset);
}

//*************************************************************
//*********** SLOTs for changes triggered in Histogram *******
//*************************************************************
// General-Tab
void HistogramDock::curveDataColumnChanged(const AbstractColumn* column) {
	CONDITIONAL_LOCK_RETURN;
	cbDataColumn->setColumn(column, m_curve->dataColumnPath());
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
	ui.leBinWidth->setText(QLocale().toString(width));
}

void HistogramDock::curveAutoBinRangesChanged(bool value) {
	CONDITIONAL_LOCK_RETURN;
	ui.chkAutoBinRanges->setChecked(value);
}

void HistogramDock::curveBinRangesMinChanged(double value) {
	CONDITIONAL_LOCK_RETURN;
	ui.leBinRangesMin->setText(QLocale().toString(value));
	ui.dteBinRangesMin->setMSecsSinceEpochUTC(value);
}

void HistogramDock::curveBinRangesMaxChanged(double value) {
	CONDITIONAL_LOCK_RETURN;
	ui.leBinRangesMax->setText(QLocale().toString(value));
	ui.dteBinRangesMax->setMSecsSinceEpochUTC(value);
}

void HistogramDock::curveVisibilityChanged(bool on) {
	CONDITIONAL_LOCK_RETURN;
	ui.chkVisible->setChecked(on);
}

//"Error bars"-Tab
void HistogramDock::curveErrorTypeChanged(Histogram::ErrorType type) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbErrorType->setCurrentIndex((int)type);
}
void HistogramDock::curveErrorPlusColumnChanged(const AbstractColumn* column) {
	CONDITIONAL_LOCK_RETURN;
	cbErrorPlusColumn->setColumn(column, m_curve->errorPlusColumnPath());
}
void HistogramDock::curveErrorMinusColumnChanged(const AbstractColumn* column) {
	CONDITIONAL_LOCK_RETURN;
	cbErrorMinusColumn->setColumn(column, m_curve->errorMinusColumnPath());
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

	// Error bars
	ui.cbErrorType->setCurrentIndex((int)m_curve->errorType());

	// Margin plots
	ui.chkRugEnabled->setChecked(m_curve->rugEnabled());
	ui.sbRugWidth->setValue(Worksheet::convertFromSceneUnits(m_curve->rugWidth(), Worksheet::Unit::Point));
	ui.sbRugLength->setValue(Worksheet::convertFromSceneUnits(m_curve->rugLength(), Worksheet::Unit::Point));
	ui.sbRugOffset->setValue(Worksheet::convertFromSceneUnits(m_curve->rugOffset(), Worksheet::Unit::Point));
}

void HistogramDock::loadConfig(KConfig& config) {
	KConfigGroup group = config.group(QLatin1String("Histogram"));

	// General
	// we don't load/save the settings in the general-tab, since they are not style related.
	// It doesn't make sense to load/save them in the template.
	// This data is read in HistogramDock::setCurves().

	lineWidget->loadConfig(group);
	symbolWidget->loadConfig(group);
	valueWidget->loadConfig(group);
	backgroundWidget->loadConfig(group);

	// Error bars
	ui.cbErrorType->setCurrentIndex(group.readEntry("ErrorType", (int)m_curve->errorType()));
	errorBarsLineWidget->loadConfig(group);

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
	KConfigGroup group = config.group("Histogram");

	lineWidget->saveConfig(group);
	symbolWidget->saveConfig(group);
	valueWidget->saveConfig(group);
	backgroundWidget->saveConfig(group);
	group.writeEntry("ErrorType", ui.cbErrorType->currentIndex());

	config.sync();
}
