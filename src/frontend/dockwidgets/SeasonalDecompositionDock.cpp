/*
	File                 : SeasonalDecompositionDock.cpp
	Project              : LabPlot
	Description          : widget for properties of a time series seasonal decomposition
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2025 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "SeasonalDecompositionDock.h"
#include "backend/core/column/Column.h"
#include "frontend/TemplateHandler.h"
#include "frontend/widgets/TreeViewComboBox.h"

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KMessageWidget>

/*!
 * \class SeasonalDecompositionDock
 * \brief
 *
 * \ingroup frontend
*/
SeasonalDecompositionDock::SeasonalDecompositionDock(QWidget* parent)
	: BaseDock(parent) {
	ui.setupUi(this);
	setBaseWidgets(ui.leName, ui.teComment);

	// Tab "General"
	cbXColumn = new TreeViewComboBox();
	ui.gridLayout->addWidget(cbXColumn, 4, 2, 1, 1);

	cbYColumn = new TreeViewComboBox();
	ui.gridLayout->addWidget(cbYColumn, 5, 2, 1, 1);

	retranslateUi();

	// Slots
	// General
	connect(cbXColumn, &TreeViewComboBox::currentModelIndexChanged, this, &SeasonalDecompositionDock::xColumnChanged);
	connect(cbYColumn, &TreeViewComboBox::currentModelIndexChanged, this, &SeasonalDecompositionDock::yColumnChanged);
	connect(ui.cbMethod, &QComboBox::currentIndexChanged, this, &SeasonalDecompositionDock::methodChanged);

	// STL parameters
	connect(ui.sbSTLPeriod, &QSpinBox::valueChanged, this, &SeasonalDecompositionDock::stlPeriodChanged);
	connect(ui.chbSTLRobust, &QCheckBox::toggled, this, &SeasonalDecompositionDock::stlRobustChanged);

	connect(ui.sbSTLSeasonalLength, &QSpinBox::valueChanged, this, &SeasonalDecompositionDock::stlSeasonalLengthChanged);
	connect(ui.sbSTLTrendLength, &QSpinBox::valueChanged, this, &SeasonalDecompositionDock::stlTrendLengthChanged);
	connect(ui.chbSTLTrendLengthAuto, &QCheckBox::toggled, this, &SeasonalDecompositionDock::stlTrendLengthAutoChanged);
	connect(ui.sbSTLLowPassLength, &QSpinBox::valueChanged, this, &SeasonalDecompositionDock::stlLowPassLengthChanged);
	connect(ui.chbSTLLowPassLengthAuto, &QCheckBox::toggled, this, &SeasonalDecompositionDock::stlLowPassLengthAutoChanged);

	connect(ui.cbSTLSeasonalDegree, &QComboBox::currentIndexChanged, this, &SeasonalDecompositionDock::stlSeasonalDegreeChanged);
	connect(ui.cbSTLTrendDegree, &QComboBox::currentIndexChanged, this, &SeasonalDecompositionDock::stlTrendDegreeChanged);
	connect(ui.cbSTLLowPassDegree, &QComboBox::currentIndexChanged, this, &SeasonalDecompositionDock::stlLowPassDegreeChanged);

	connect(ui.sbSTLSeasonalJump, &QSpinBox::valueChanged, this, &SeasonalDecompositionDock::stlSeasonalJumpChanged);
	connect(ui.chbSTLSeasonalJumpAuto, &QCheckBox::toggled, this, &SeasonalDecompositionDock::stlSeasonalJumpAutoChanged);
	connect(ui.sbSTLTrendJump, &QSpinBox::valueChanged, this, &SeasonalDecompositionDock::stlTrendJumpChanged);
	connect(ui.chbSTLTrendJumpAuto, &QCheckBox::toggled, this, &SeasonalDecompositionDock::stlTrendJumpAutoChanged);
	connect(ui.sbSTLLowPassJump, &QSpinBox::valueChanged, this, &SeasonalDecompositionDock::stlLowPassJumpChanged);
	connect(ui.chbSTLLowPassJumpAuto, &QCheckBox::toggled, this, &SeasonalDecompositionDock::stlLowPassJumpAutoChanged);

	// MSTL parameters
	connect(ui.leMSTLPeriods, &TimedLineEdit::textEdited, this, &SeasonalDecompositionDock::mstlPeriodsChanged);
	connect(ui.sbMSTLLambda, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &SeasonalDecompositionDock::mstlLambdaChanged);
	connect(ui.sbMSTLIterations, &QSpinBox::valueChanged, this, &SeasonalDecompositionDock::mstlIterationsChanged);

	// template handler
	auto* frame = new QFrame(this);
	auto* layout = new QHBoxLayout(frame);
	layout->setContentsMargins(0, 0, 0, 0);

	auto* templateHandler = new TemplateHandler(this, QLatin1String("SeasonalDecomposition"));
	layout->addWidget(templateHandler);
	connect(templateHandler, &TemplateHandler::loadConfigRequested, this, &SeasonalDecompositionDock::loadConfigFromTemplate);
	connect(templateHandler, &TemplateHandler::saveConfigRequested, this, &SeasonalDecompositionDock::saveConfigAsTemplate);
	connect(templateHandler, &TemplateHandler::info, this, &SeasonalDecompositionDock::info);

	ui.gridLayout->addWidget(frame, 29, 0, 1, 3);
}

SeasonalDecompositionDock::~SeasonalDecompositionDock() = default;

void SeasonalDecompositionDock::setModel() {
	auto* model = aspectModel();
	model->enablePlottableColumnsOnly(true);
	model->enableShowPlotDesignation(true);
	model->setSelectableAspects({AspectType::Column});
	cbXColumn->setTopLevelClasses(TreeViewComboBox::plotColumnTopLevelClasses());
	cbXColumn->setModel(model);
	cbYColumn->setTopLevelClasses(TreeViewComboBox::plotColumnTopLevelClasses());
	cbYColumn->setModel(model);
}

void SeasonalDecompositionDock::setDecompositions(QList<SeasonalDecomposition*> list) {
	CONDITIONAL_LOCK_RETURN;
	m_decompositions = list;
	m_decomposition = list.first();
	setAspects(list);
	Q_ASSERT(m_decomposition);
	setModel();

	// show the properties of the first curve
	// if there are more than one curve in the list, disable the content in the tab "general"
	if (m_decompositions.size() == 1) {
		cbXColumn->setEnabled(true);
		cbXColumn->setAspect(m_decomposition->xColumn(), m_decomposition->xColumnPath());
		cbYColumn->setEnabled(true);
		cbYColumn->setAspect(m_decomposition->yColumn(), m_decomposition->yColumnPath());
	} else {
		cbXColumn->setEnabled(false);
		cbXColumn->setCurrentModelIndex(QModelIndex());
		cbYColumn->setEnabled(false);
		cbYColumn->setCurrentModelIndex(QModelIndex());
	}

	load(); // load the remaining properties
	showStatusError(QString()); // remove the message from the previous decomposition, if available

	// Slots
	// General-tab
	connect(m_decomposition, &SeasonalDecomposition::statusError, this, &SeasonalDecompositionDock::showStatusError);
	connect(m_decomposition, &SeasonalDecomposition::xColumnChanged, this, &SeasonalDecompositionDock::decompositionXColumnChanged);
	connect(m_decomposition, &SeasonalDecomposition::yColumnChanged, this, &SeasonalDecompositionDock::decompositionYColumnChanged);
	connect(m_decomposition, &SeasonalDecomposition::methodChanged, this, &SeasonalDecompositionDock::decompositionMethodChanged);

	// STL parameters
	connect(m_decomposition, &SeasonalDecomposition::stlPeriodChanged, this, &SeasonalDecompositionDock::decompositionSTLPeriodChanged);
	connect(m_decomposition, &SeasonalDecomposition::stlRobustChanged, this, &SeasonalDecompositionDock::decompositionSTLRobustChanged);

	connect(m_decomposition, &SeasonalDecomposition::stlSeasonalLengthChanged, this, &SeasonalDecompositionDock::decompositionSTLSeasonalLengthChanged);
	connect(m_decomposition, &SeasonalDecomposition::stlTrendLengthChanged, this, &SeasonalDecompositionDock::decompositionSTLTrendLengthChanged);
	connect(m_decomposition, &SeasonalDecomposition::stlTrendLengthAutoChanged, this, &SeasonalDecompositionDock::decompositionSTLTrendLengthAutoChanged);
	connect(m_decomposition, &SeasonalDecomposition::stlLowPassLengthChanged, this, &SeasonalDecompositionDock::decompositionSTLLowPassLengthChanged);
	connect(m_decomposition, &SeasonalDecomposition::stlLowPassLengthAutoChanged, this, &SeasonalDecompositionDock::decompositionSTLLowPassLengthAutoChanged);

	connect(m_decomposition, &SeasonalDecomposition::stlSeasonalDegreeChanged, this, &SeasonalDecompositionDock::decompositionSTLSeasonalDegreeChanged);
	connect(m_decomposition, &SeasonalDecomposition::stlTrendDegreeChanged, this, &SeasonalDecompositionDock::decompositionSTLTrendDegreeChanged);
	connect(m_decomposition, &SeasonalDecomposition::stlLowPassDegreeChanged, this, &SeasonalDecompositionDock::decompositionSTLLowPassDegreeChanged);

	connect(m_decomposition, &SeasonalDecomposition::stlSeasonalJumpChanged, this, &SeasonalDecompositionDock::decompositionSTLSeasonalJumpChanged);
	connect(m_decomposition, &SeasonalDecomposition::stlSeasonalJumpAutoChanged, this, &SeasonalDecompositionDock::decompositionSTLSeasonalJumpAutoChanged);
	connect(m_decomposition, &SeasonalDecomposition::stlTrendJumpChanged, this, &SeasonalDecompositionDock::decompositionSTLTrendJumpChanged);
	connect(m_decomposition, &SeasonalDecomposition::stlTrendJumpAutoChanged, this, &SeasonalDecompositionDock::decompositionSTLTrendJumpAutoChanged);
	connect(m_decomposition, &SeasonalDecomposition::stlLowPassJumpChanged, this, &SeasonalDecompositionDock::decompositionSTLLowPassJumpChanged);
	connect(m_decomposition, &SeasonalDecomposition::stlLowPassJumpAutoChanged, this, &SeasonalDecompositionDock::decompositionSTLLowPassJumpAutoChanged);

	// MSTL parameters
	connect(m_decomposition, &SeasonalDecomposition::mstlPeriodsChanged, this, &SeasonalDecompositionDock::decompositionMSTLPeriodsChanged);
	connect(m_decomposition, &SeasonalDecomposition::mstlLambdaChanged, this, &SeasonalDecompositionDock::decompositionMSTLLambdaChanged);
	connect(m_decomposition, &SeasonalDecomposition::mstlIterationsChanged, this, &SeasonalDecompositionDock::decompositionMSTLIterationsChanged);
 }

void SeasonalDecompositionDock::retranslateUi() {
	CONDITIONAL_LOCK_RETURN;

	// method
	ui.cbMethod->clear();
	ui.cbMethod->addItem(QStringLiteral("STL"), static_cast<int>(SeasonalDecomposition::Method::STL));
	ui.cbMethod->addItem(QStringLiteral("MSTL"), static_cast<int>(SeasonalDecomposition::Method::MSTL));

	QString info = i18n(
		"Method used to perform the seasonal-trend decomposition:"
		"<ul>"
		"<li>STL - Seasonal-trend decomposition based on LOESS (\"Locally Estimated Scatterplot Smoothing\")</li>"
		"<li>MSTL - STL for multiple seasonalities</li>"
		"</ul>");
	ui.lMethod->setToolTip(info);
	ui.cbMethod->setToolTip(info);

	// STL parameters

	// robust
	info = i18n("If enabled, uses a data-dependent weighting function to get more robust result if the data has outliers.");
	ui.lSTLRobust->setToolTip(info);
	ui.chbSTLRobust->setToolTip(info);

	// period
	info = i18n("Period of the seasonal component. Examples:"
		"<ul>"
		"<li>24 - for hourly data with daily seasonality (24h)</li>"
		"<li>168 - for hourly data with weekly seasonality (7 * 24h = 168h)</li>"
		"<li>7 - for daily data with weekly seasonality (7 days)</li>"
		"<li>365 - for daily data yearly (365 days) seasonality</li>"
		"<li>12 - for monthly data with yearly seasonality (12 months)</li>"
		"</ul>");
	ui.lSTLPeriod->setToolTip(info);
	ui.sbSTLPeriod->setToolTip(info);

	// lengths
	info = i18n("Length of the seasonal smoother, must be odd and at least 3.");
	ui.lSTLSeasonalLength->setToolTip(info);
	ui.sbSTLSeasonalLength->setToolTip(info);

	info = i18n("Length of the trend smoother, must be odd and at least 3.");
	ui.lSTLTrendLength->setToolTip(info);
	ui.sbSTLTrendLength->setToolTip(info);
	ui.chbSTLTrendLengthAuto->setToolTip(i18n("If enabled, uses the smallest odd integer greater than 1.5 * \"Period\" / (1 - 1.5 / \"Seasonal Length\")."));

	info = i18n("Length of the low-pass filter, must be odd and at least 3.");
	ui.lSTLLowPassLength->setToolTip(info);
	ui.sbSTLLowPassLength->setToolTip(info);
	ui.chbSTLLowPassLengthAuto->setToolTip(i18n("If enabled, uses the smallest odd integer greater than \"Period\"."));

	// degrees
	QString degree0 = i18n("0 - Constant");
	QString degree1 = i18n("1 - Constant and Trend");
	ui.cbSTLSeasonalDegree->clear();
	ui.cbSTLSeasonalDegree->addItem(degree0);
	ui.cbSTLSeasonalDegree->addItem(degree1);

	ui.cbSTLTrendDegree->clear();
	ui.cbSTLTrendDegree->addItem(degree0);
	ui.cbSTLTrendDegree->addItem(degree1);

	ui.cbSTLLowPassDegree->clear();
	ui.cbSTLLowPassDegree->addItem(degree0);
	ui.cbSTLLowPassDegree->addItem(degree1);

	info = i18n("The degree of locally-fitted polynomial for the seasonal component.");
	ui.lSTLSeasonalDegree->setToolTip(info);
	ui.cbSTLSeasonalDegree->setToolTip(info);

	info = i18n("The degree of locally-fitted polynomial for the trend component.");
	ui.lSTLTrendDegree->setToolTip(info);
	ui.cbSTLTrendDegree->setToolTip(info);

	info = i18n("The degree of locally-fitted polynomial for the low-pass component.");
	ui.lSTLLowPassDegree->setToolTip(info);
	ui.cbSTLLowPassDegree->setToolTip(info);

	// jumps
	info = i18n("The number of jumps to include in the seasonal component.");
	ui.lSTLSeasonalJump->setToolTip(info);
	ui.sbSTLSeasonalJump->setToolTip(info);
	ui.chbSTLSeasonalJumpAuto->setToolTip(i18n("If enabled, uses \"Seasonal Length\" / 10."));

	info = i18n("The number of jumps to include in the trend component.");
	ui.lSTLTrendJump->setToolTip(info);
	ui.sbSTLTrendJump->setToolTip(info);
	ui.chbSTLTrendJumpAuto->setToolTip(i18n("If enabled, uses \"Trend Length\" / 10."));

	info = i18n("The number of jumps to include in the low-pass component.");
	ui.lSTLLowPassJump->setToolTip(info);
	ui.sbSTLLowPassJump->setToolTip(info);
	ui.chbSTLLowPassJumpAuto->setToolTip(i18n("If enabled, uses \"Low-Pass Length\" / 10."));

	// MSTL parameters
	info = i18n("Periods of the seasonal components, coma-separated values. Examples:"
		"<ul>"
		"<li>24, 168 - for hourly data with daily (24h) and weekly (7 * 24h = 168h) seasonality</li>"
		"<li>7, 365 - for daily data with weekly (7 days) and yearly (365 days) seasonality</li>"
		"</ul>");
	ui.lMSTLPeriods->setToolTip(info);
	ui.leMSTLPeriods->setToolTip(info);

	info = i18n("The lambda parameter for the Box-Cox transformation applied prior to decomposition.");
	ui.lMSTLLambda->setToolTip(info);
	ui.sbMSTLLambda->setToolTip(info);

	info = i18n("Number of iterations used to refine the seasonal component.");
	ui.lMSTLIterations->setToolTip(info);
	ui.sbMSTLIterations->setToolTip(info);
}

//*************************************************************
//* SLOTs for changes triggered in SeasonalDecompositionDock **
//*************************************************************

// "General"-tab
void SeasonalDecompositionDock::methodChanged(int) {
	const auto method = static_cast<SeasonalDecomposition::Method>(ui.cbMethod->currentData().toInt());

	// show/hide the relevant parameter related widgets
	// TODO: generalize later to more methods
	bool mstl = (method == SeasonalDecomposition::Method::MSTL);
	ui.lMSTLPeriods->setVisible(mstl);
	ui.leMSTLPeriods->setVisible(mstl);
	ui.lMSTLLambda->setVisible(mstl);
	ui.sbMSTLLambda->setVisible(mstl);
	ui.lMSTLIterations->setVisible(mstl);
	ui.sbMSTLIterations->setVisible(mstl);
	ui.lSTLPeriod->setVisible(!mstl);
	ui.sbSTLPeriod->setVisible(!mstl);

	CONDITIONAL_LOCK_RETURN;

	for (auto* decomposition : m_decompositions)
		decomposition->setMethod(method);
}

void SeasonalDecompositionDock::xColumnChanged(const QModelIndex& index) {
	CONDITIONAL_LOCK_RETURN;

	auto aspect = static_cast<AbstractAspect*>(index.internalPointer());
	AbstractColumn* column(nullptr);
	if (aspect) {
		column = dynamic_cast<AbstractColumn*>(aspect);
		Q_ASSERT(column);
	}

	for (auto* decomposition : m_decompositions)
		decomposition->setXColumn(column);
}

void SeasonalDecompositionDock::yColumnChanged(const QModelIndex& index) {
	CONDITIONAL_LOCK_RETURN;

	auto aspect = static_cast<AbstractAspect*>(index.internalPointer());
	AbstractColumn* column(nullptr);
	if (aspect) {
		column = dynamic_cast<AbstractColumn*>(aspect);
		Q_ASSERT(column);
	}

	for (auto* decomposition : m_decompositions)
		decomposition->setYColumn(column);
}

// STL parameters
void SeasonalDecompositionDock::stlPeriodChanged(int value) {
	CONDITIONAL_LOCK_RETURN;
	for (auto* decomposition : m_decompositions)
		decomposition->setSTLPeriod(value);
}
void SeasonalDecompositionDock::stlRobustChanged(bool checked) {
	CONDITIONAL_LOCK_RETURN;
	for (auto* decomposition : m_decompositions)
		decomposition->setSTLRobust(checked);
}

void SeasonalDecompositionDock::stlSeasonalLengthChanged(int length) {
	CONDITIONAL_LOCK_RETURN;
	for (auto* decomposition : m_decompositions)
		decomposition->setSTLSeasonalLength(length);
}
void SeasonalDecompositionDock::stlTrendLengthChanged(int length) {
	CONDITIONAL_LOCK_RETURN;
	for (auto* decomposition : m_decompositions)
		decomposition->setSTLTrendLength(length);
}
void SeasonalDecompositionDock::stlTrendLengthAutoChanged(bool value) {
	ui.sbSTLTrendLength->setEnabled(!value);
	CONDITIONAL_LOCK_RETURN;
	for (auto* decomposition : m_decompositions)
		decomposition->setSTLTrendLengthAuto(value);
}
void SeasonalDecompositionDock::stlLowPassLengthChanged(int length) {
	CONDITIONAL_LOCK_RETURN;
	for (auto* decomposition : m_decompositions)
		decomposition->setSTLLowPassLength(length);
}
void SeasonalDecompositionDock::stlLowPassLengthAutoChanged(bool value) {
	ui.sbSTLLowPassLength->setEnabled(!value);
	CONDITIONAL_LOCK_RETURN;
	for (auto* decomposition : m_decompositions)
		decomposition->setSTLLowPassLengthAuto(value);
}

void SeasonalDecompositionDock::stlSeasonalDegreeChanged(int degree) {
	CONDITIONAL_LOCK_RETURN;
	for (auto* decomposition : m_decompositions)
		decomposition->setSTLSeasonalDegree(degree);
}
void SeasonalDecompositionDock::stlTrendDegreeChanged(int degree) {
	CONDITIONAL_LOCK_RETURN;
	for (auto* decomposition : m_decompositions)
		decomposition->setSTLTrendDegree(degree);
}
void SeasonalDecompositionDock::stlLowPassDegreeChanged(int degree) {
	CONDITIONAL_LOCK_RETURN;
	for (auto* decomposition : m_decompositions)
		decomposition->setSTLLowPassDegree(degree);
}

void SeasonalDecompositionDock::stlSeasonalJumpChanged(int jump) {
	CONDITIONAL_LOCK_RETURN;
	for (auto* decomposition : m_decompositions)
		decomposition->setSTLSeasonalJump(jump);
}
void SeasonalDecompositionDock::stlSeasonalJumpAutoChanged(bool value) {
	ui.sbSTLSeasonalJump->setEnabled(!value);
	CONDITIONAL_LOCK_RETURN;
	for (auto* decomposition : m_decompositions)
		decomposition->setSTLSeasonalJumpAuto(value);
}
void SeasonalDecompositionDock::stlTrendJumpChanged(int jump) {
	CONDITIONAL_LOCK_RETURN;
	for (auto* decomposition : m_decompositions)
		decomposition->setSTLTrendJump(jump);
}
void SeasonalDecompositionDock::stlTrendJumpAutoChanged(bool value) {
	ui.sbSTLTrendJump->setEnabled(!value);
	CONDITIONAL_LOCK_RETURN;
	for (auto* decomposition : m_decompositions)
		decomposition->setSTLTrendJumpAuto(value);
}
void SeasonalDecompositionDock::stlLowPassJumpChanged(int jump) {
	CONDITIONAL_LOCK_RETURN;
	for (auto* decomposition : m_decompositions)
		decomposition->setSTLLowPassJump(jump);
}
void SeasonalDecompositionDock::stlLowPassJumpAutoChanged(bool value) {
	ui.sbSTLLowPassJump->setEnabled(!value);
	CONDITIONAL_LOCK_RETURN;
	for (auto* decomposition : m_decompositions)
		decomposition->setSTLLowPassJumpAuto(value);
}

// MSTL parameters
void SeasonalDecompositionDock::mstlPeriodsChanged() {
	CONDITIONAL_LOCK_RETURN;
	std::vector<size_t> periods;
	auto list = ui.leMSTLPeriods->text().split(QLatin1Char(','));
	for (auto el : list)
		periods.push_back(el.simplified().toInt());

	for (auto* decomposition : m_decompositions)
		decomposition->setMSTLPeriods(periods);
}
void SeasonalDecompositionDock::mstlLambdaChanged(double value) {
	CONDITIONAL_LOCK_RETURN;
	for (auto* decomposition : m_decompositions)
		decomposition->setMSTLLambda(value);
}
void SeasonalDecompositionDock::mstlIterationsChanged(int value) {
	CONDITIONAL_LOCK_RETURN;
	for (auto* decomposition : m_decompositions)
		decomposition->setMSTLIterations(value);
}

//*************************************************************
//**** SLOTs for changes triggered in SeasonalDecomposition ***
//*************************************************************
// General-Tab
void SeasonalDecompositionDock::decompositionXColumnChanged(const AbstractColumn* column) {
	CONDITIONAL_LOCK_RETURN;
	cbXColumn->setAspect(column, m_decomposition->xColumnPath());
}

void SeasonalDecompositionDock::decompositionYColumnChanged(const AbstractColumn* column) {
	CONDITIONAL_LOCK_RETURN;
	cbYColumn->setAspect(column, m_decomposition->yColumnPath());
}

void SeasonalDecompositionDock::decompositionMethodChanged(SeasonalDecomposition::Method method) {
	CONDITIONAL_LOCK_RETURN;
	const int index = ui.cbMethod->findData(static_cast<int>(method));
	ui.cbMethod->setCurrentIndex(index);
}

// STL parameters
void SeasonalDecompositionDock::decompositionSTLPeriodChanged(int value) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbSTLPeriod->setValue(value);
}
void SeasonalDecompositionDock::decompositionSTLRobustChanged(bool value) {
	CONDITIONAL_LOCK_RETURN;
	ui.chbSTLRobust->setChecked(value);
}

void SeasonalDecompositionDock::decompositionSTLSeasonalLengthChanged(int length) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbSTLSeasonalLength->setValue(length);
}
void SeasonalDecompositionDock::decompositionSTLTrendLengthChanged(int length) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbSTLTrendLength->setValue(length);
}
void SeasonalDecompositionDock::decompositionSTLTrendLengthAutoChanged(bool value) {
	CONDITIONAL_LOCK_RETURN;
	ui.chbSTLTrendLengthAuto->setChecked(value);
}
void SeasonalDecompositionDock::decompositionSTLLowPassLengthChanged(int length) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbSTLLowPassLength->setValue(length);
}
void SeasonalDecompositionDock::decompositionSTLLowPassLengthAutoChanged(bool value) {
	CONDITIONAL_LOCK_RETURN;
	ui.chbSTLLowPassLengthAuto->setChecked(value);
}

void SeasonalDecompositionDock::decompositionSTLSeasonalDegreeChanged(int degree) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbSTLSeasonalDegree->setCurrentIndex(degree);
}
void SeasonalDecompositionDock::decompositionSTLTrendDegreeChanged(int degree) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbSTLTrendDegree->setCurrentIndex(degree);
}
void SeasonalDecompositionDock::decompositionSTLLowPassDegreeChanged(int degree) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbSTLLowPassDegree->setCurrentIndex(degree);
}

void SeasonalDecompositionDock::decompositionSTLSeasonalJumpChanged(int jump) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbSTLSeasonalJump->setValue(jump);
}
void SeasonalDecompositionDock::decompositionSTLSeasonalJumpAutoChanged(bool value) {
	CONDITIONAL_LOCK_RETURN;
	ui.chbSTLSeasonalJumpAuto->setChecked(value);
}
void SeasonalDecompositionDock::decompositionSTLTrendJumpChanged(int jump) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbSTLTrendJump->setValue(jump);
}
void SeasonalDecompositionDock::decompositionSTLTrendJumpAutoChanged(bool value) {
	CONDITIONAL_LOCK_RETURN;
	ui.chbSTLTrendJumpAuto->setChecked(value);
}
void SeasonalDecompositionDock::decompositionSTLLowPassJumpChanged(int jump) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbSTLLowPassJump->setValue(jump);
}
void SeasonalDecompositionDock::decompositionSTLLowPassJumpAutoChanged(bool value) {
	CONDITIONAL_LOCK_RETURN;
	ui.chbSTLLowPassJumpAuto->setChecked(value);
}

// MSTL parameters
QString mstlPeriodsToString(const std::vector<size_t>& periods) {
	QString text;
	const auto locale = QLocale();
	for (int period : periods) {
		if (!text.isEmpty())
			text += QLatin1String(", ");
		text += locale.toString(period);
	}

	return text;
}

void SeasonalDecompositionDock::decompositionMSTLPeriodsChanged(const std::vector<size_t>& periods) {
	CONDITIONAL_LOCK_RETURN;
	ui.leMSTLPeriods->setText(mstlPeriodsToString(periods));
}
void SeasonalDecompositionDock::decompositionMSTLLambdaChanged(double value) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbMSTLLambda->setValue(value);
}
void SeasonalDecompositionDock::decompositionMSTLIterationsChanged(int value) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbMSTLIterations->setValue(value);
}

void SeasonalDecompositionDock::showStatusError(const QString& error) {
	if (error.isEmpty()) {
		if (m_messageWidget && m_messageWidget->isVisible())
			m_messageWidget->close();
	} else {
		if (!m_messageWidget) {
			m_messageWidget = new KMessageWidget(this);
			m_messageWidget->setMessageType(KMessageWidget::Error);
			ui.gridLayout->addWidget(m_messageWidget, 27, 0, 1, 3);
		}
		m_messageWidget->setText(error);
		m_messageWidget->animatedShow();
		QDEBUG(error);
	}
}

//*************************************************************
//************************* Settings **************************
//*************************************************************
void SeasonalDecompositionDock::load() {
	// General

	// method
	const int index = ui.cbMethod->findData(static_cast<int>(m_decomposition->method()));
	ui.cbMethod->setCurrentIndex(index);
	methodChanged(index);

	// STL parameters
	ui.sbSTLPeriod->setValue(m_decomposition->stlPeriod());
	ui.chbSTLRobust->setChecked(m_decomposition->stlRobust());

	ui.sbSTLSeasonalLength->setValue(m_decomposition->stlSeasonalLength());
	ui.sbSTLTrendLength->setValue(m_decomposition->stlTrendLength());
	ui.chbSTLTrendLengthAuto->setChecked(m_decomposition->stlTrendLengthAuto());
	ui.sbSTLLowPassLength->setValue(m_decomposition->stlLowPassLength());
	ui.chbSTLLowPassLengthAuto->setChecked(m_decomposition->stlLowPassLengthAuto());

	ui.cbSTLSeasonalDegree->setCurrentIndex(m_decomposition->stlSeasonalDegree());
	ui.cbSTLTrendDegree->setCurrentIndex(m_decomposition->stlTrendDegree());
	ui.cbSTLLowPassDegree->setCurrentIndex(m_decomposition->stlLowPassDegree());

	ui.sbSTLSeasonalJump->setValue(m_decomposition->stlSeasonalJump());
	ui.chbSTLSeasonalJumpAuto->setChecked(m_decomposition->stlSeasonalJumpAuto());
	ui.sbSTLTrendJump->setValue(m_decomposition->stlTrendJump());
	ui.chbSTLTrendJumpAuto->setChecked(m_decomposition->stlTrendJumpAuto());
	ui.sbSTLLowPassJump->setValue(m_decomposition->stlLowPassJump());
	ui.chbSTLLowPassJumpAuto->setChecked(m_decomposition->stlLowPassJumpAuto());

	// MSTL parameters
	ui.leMSTLPeriods->setText(mstlPeriodsToString(m_decomposition->mstlPeriods()));
	ui.sbMSTLLambda->setValue(m_decomposition->mstlLambda());
	ui.sbMSTLIterations->setValue(m_decomposition->mstlIterations());
}

void SeasonalDecompositionDock::loadConfig(KConfig& config) {
	auto group = config.group(QStringLiteral("SeasonalDecomposition"));

	// method
	const auto method = group.readEntry(QStringLiteral("Method"), static_cast<int>(m_decomposition->method()));
	const int index = ui.cbMethod->findData(static_cast<int>(method));
	ui.cbMethod->setCurrentIndex(index);
	methodChanged(index);

	// STL parameters
	ui.sbSTLPeriod->setValue(group.readEntry("STLPeriod", m_decomposition->stlPeriod()));
	ui.chbSTLRobust->setChecked(group.readEntry("STLRobust", m_decomposition->stlRobust()));

	ui.sbSTLSeasonalLength->setValue(group.readEntry("STLSeasonalLength", m_decomposition->stlSeasonalLength()));
	ui.sbSTLTrendLength->setValue(group.readEntry("STLTrendLength", m_decomposition->stlTrendLength()));
	ui.chbSTLTrendLengthAuto->setChecked(group.readEntry("STLTrendLengthAuto", m_decomposition->stlTrendLengthAuto()));
	ui.sbSTLLowPassLength->setValue(group.readEntry("STLLowPassLength", m_decomposition->stlLowPassLength()));
	ui.chbSTLLowPassLengthAuto->setChecked(group.readEntry("STLLowPassLengthAuto", m_decomposition->stlLowPassLengthAuto()));

	ui.cbSTLSeasonalDegree->setCurrentIndex(group.readEntry("STLSeasonalDegree", m_decomposition->stlSeasonalDegree()));
	ui.cbSTLTrendDegree->setCurrentIndex(group.readEntry("STLTrendDegree", m_decomposition->stlTrendDegree()));
	ui.cbSTLLowPassDegree->setCurrentIndex(group.readEntry("STLLowPassDegree", m_decomposition->stlLowPassDegree()));

	ui.sbSTLSeasonalJump->setValue(group.readEntry("STLSeasonalJump", m_decomposition->stlSeasonalJump()));
	ui.chbSTLSeasonalJumpAuto->setChecked(group.readEntry("STLSeasonalJumpAuto", m_decomposition->stlSeasonalJumpAuto()));
	ui.sbSTLTrendJump->setValue(group.readEntry("STLTrendJump", m_decomposition->stlTrendJump()));
	ui.chbSTLTrendJumpAuto->setChecked(group.readEntry("STLTrendJumpAuto", m_decomposition->stlTrendJumpAuto()));
	ui.sbSTLLowPassJump->setValue(group.readEntry("STLLowPassJump", m_decomposition->stlLowPassJump()));
	ui.chbSTLLowPassJumpAuto->setChecked(group.readEntry("STLLowPassJumpAuto", m_decomposition->stlLowPassJumpAuto()));

	// MSTL parameters
	ui.leMSTLPeriods->setText(group.readEntry("MSTLPeriods", mstlPeriodsToString(m_decomposition->mstlPeriods())));
	ui.sbMSTLLambda->setValue(group.readEntry("MSTLLambda", m_decomposition->mstlLambda()));
	ui.sbMSTLIterations->setValue(group.readEntry("MSTLIterations", m_decomposition->mstlIterations()));
}

void SeasonalDecompositionDock::loadConfigFromTemplate(KConfig& config) {
	auto name = TemplateHandler::templateName(config);
	int size = m_decompositions.size();
	if (size > 1)
		m_decomposition->beginMacro(i18n("%1 seasonal decompositions: template \"%2\" loaded", size, name));
	else
		m_decomposition->beginMacro(i18n("%1: template \"%2\" loaded", m_decomposition->name(), name));

	this->loadConfig(config);

	m_decomposition->endMacro();
}

void SeasonalDecompositionDock::saveConfigAsTemplate(KConfig& config) {
	auto group = config.group(QStringLiteral("SeasonalDecomposition"));

	// STL parameters
	group.writeEntry("STLPeriod", ui.sbSTLPeriod->value());
	group.writeEntry("STLRobust", ui.chbSTLRobust->isChecked());

	group.writeEntry("STLSeasonalLength", ui.sbSTLSeasonalLength->value());
	group.writeEntry("STLTrendLength", ui.sbSTLTrendLength->value());
	group.writeEntry("STLTrendLengthAuto", ui.chbSTLTrendLengthAuto->isChecked());
	group.writeEntry("STLLowPassLength", ui.sbSTLLowPassLength->value());
	group.writeEntry("STLLowPassLengthAuto", ui.chbSTLLowPassLengthAuto->isChecked());

	group.writeEntry("STLSeasonalDegree", ui.cbSTLSeasonalDegree->currentIndex());
	group.writeEntry("STLTrendDegree", ui.cbSTLTrendDegree->currentIndex());
	group.writeEntry("STLLowPassDegree", ui.cbSTLLowPassDegree->currentIndex());

	group.writeEntry("STLSeasonalJump", ui.sbSTLSeasonalJump->value());
	group.writeEntry("STLSeasonalJumpAuto", ui.chbSTLSeasonalJumpAuto->isChecked());
	group.writeEntry("STLTrendJump", ui.sbSTLTrendJump->value());
	group.writeEntry("STLTrendJumpAuto", ui.chbSTLSeasonalJumpAuto->isChecked());
	group.writeEntry("STLLowPassJump", ui.sbSTLLowPassJump->value());
	group.writeEntry("STLLowPassJumpAuto", ui.chbSTLLowPassJumpAuto->isChecked());

	// MSTL parameters
	group.writeEntry("MSTLPeriods", ui.leMSTLPeriods->text());
	group.writeEntry("MSTLLambda", ui.sbMSTLLambda->value());
	group.writeEntry("MSTLIterations", ui.sbMSTLIterations->value());

	config.sync();
}
