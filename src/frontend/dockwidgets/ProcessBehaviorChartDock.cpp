/*
	File                 : ProcessBehaviorChartDock.cpp
	Project              : LabPlot
	Description          : widget for properties of the process behavior chart
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024-2025 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ProcessBehaviorChartDock.h"
#include "backend/core/column/Column.h"
#include "backend/worksheet/TextLabel.h"
#include "backend/worksheet/plots/cartesian/ProcessBehaviorChart.h"
#include "backend/worksheet/plots/cartesian/Symbol.h"
#include "frontend/TemplateHandler.h"
#include "frontend/widgets/LineWidget.h"
#include "frontend/widgets/SymbolWidget.h"
#include "frontend/widgets/TreeViewComboBox.h"

#include <KConfig>
#include <KLocalizedString>
#include <KMessageWidget>

/*!
  \class ProcessBehaviorChartDock
  \brief  Provides a widget for editing the properties of process behavior charts.

  \ingroup frontend
*/
ProcessBehaviorChartDock::ProcessBehaviorChartDock(QWidget* parent)
	: BaseDock(parent)
	, cbDataColumn(new TreeViewComboBox)
	, cbData2Column(new TreeViewComboBox) {
	ui.setupUi(this);
	setPlotRangeCombobox(ui.cbPlotRanges);
	setBaseWidgets(ui.leName, ui.teComment);
	setVisibilityWidgets(ui.chkVisible, ui.chkLegendVisible);

	// Tab "General"
	auto* gridLayout = qobject_cast<QGridLayout*>(ui.tabGeneral->layout());
	gridLayout->addWidget(cbDataColumn, 4, 2, 1, 1);
	gridLayout->addWidget(cbData2Column, 5, 2, 1, 1);

	ui.cbType->addItem(QStringLiteral("X (XmR)"), static_cast<int>(ProcessBehaviorChart::Type::XmR));
	ui.cbType->addItem(QStringLiteral("mR"), static_cast<int>(ProcessBehaviorChart::Type::mR));
	ui.cbType->addItem(QStringLiteral("X̅  (X̅R)"), static_cast<int>(ProcessBehaviorChart::Type::XbarR));
	ui.cbType->addItem(QStringLiteral("R"), static_cast<int>(ProcessBehaviorChart::Type::R));
	ui.cbType->addItem(QStringLiteral("X̅  (X̅S)"), static_cast<int>(ProcessBehaviorChart::Type::XbarS));
	ui.cbType->addItem(QStringLiteral("S"), static_cast<int>(ProcessBehaviorChart::Type::S));
	ui.cbType->addItem(QStringLiteral("P"), static_cast<int>(ProcessBehaviorChart::Type::P));
	ui.cbType->addItem(QStringLiteral("NP"), static_cast<int>(ProcessBehaviorChart::Type::NP));
	ui.cbType->addItem(QStringLiteral("C"), static_cast<int>(ProcessBehaviorChart::Type::C));
	ui.cbType->addItem(QStringLiteral("U"), static_cast<int>(ProcessBehaviorChart::Type::U));

	ui.leCenterSpecification->setValidator(new QDoubleValidator(ui.leUpperLimitSpecification));
	ui.leUpperLimitSpecification->setValidator(new QDoubleValidator(ui.leUpperLimitSpecification));
	ui.leLowerLimitSpecification->setValidator(new QDoubleValidator(ui.leLowerLimitSpecification));
	ui.leMinLowerLimit->setValidator(new QDoubleValidator(ui.leMinLowerLimit));
	ui.leMaxUpperLimit->setValidator(new QDoubleValidator(ui.leMaxUpperLimit));

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

	gridLayout = qobject_cast<QGridLayout*>(ui.tabControlLimitLabels->layout());
	labelsBorderLineWidget = new LineWidget(this);
	gridLayout->addWidget(labelsBorderLineWidget, 11, 0, 1, 3);

	// Tab "Center/Limit Values"
	ui.kcbLabelsFontColor->setAlphaChannelEnabled(true);
	ui.kcbLabelsBackgroundColor->setAlphaChannelEnabled(true);

	// adjust layouts in the tabs
	for (int i = 0; i < ui.tabWidget->count(); ++i) {
		auto* layout = dynamic_cast<QGridLayout*>(ui.tabWidget->widget(i)->layout());
		if (!layout)
			continue;

		layout->setContentsMargins(2, 2, 2, 2);
		layout->setHorizontalSpacing(2);
		layout->setVerticalSpacing(2);
	}

	updateLocale();
	retranslateUi();

	// Slots
	// General
	connect(cbDataColumn, &TreeViewComboBox::currentModelIndexChanged, this, &ProcessBehaviorChartDock::dataColumnChanged);
	connect(cbData2Column, &TreeViewComboBox::currentModelIndexChanged, this, &ProcessBehaviorChartDock::data2ColumnChanged);
	connect(ui.cbType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ProcessBehaviorChartDock::typeChanged);
	connect(ui.cbLimitsType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ProcessBehaviorChartDock::limitsTypeChanged);
	connect(ui.cbLimitsMetric, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ProcessBehaviorChartDock::limitsMetricChanged);
	connect(ui.sbSampleSize, &QSpinBox::valueChanged, this, &ProcessBehaviorChartDock::sampleSizeChanged);
	connect(ui.chbExactLimits, &QCheckBox::clicked, this, &ProcessBehaviorChartDock::exactLimitsEnabledChanged);
	connect(ui.leMaxUpperLimit, &TimedLineEdit::textEdited, this, &ProcessBehaviorChartDock::maxUpperLimitChanged);
	connect(ui.leMinLowerLimit, &TimedLineEdit::textEdited, this, &ProcessBehaviorChartDock::minLowerLimitChanged);
	connect(ui.leCenterSpecification, &TimedLineEdit::textEdited, this, &ProcessBehaviorChartDock::centerSpecificationChanged);
	connect(ui.leLowerLimitSpecification, &TimedLineEdit::textEdited, this, &ProcessBehaviorChartDock::lowerLimitSpecificationChanged);
	connect(ui.leUpperLimitSpecification, &TimedLineEdit::textEdited, this, &ProcessBehaviorChartDock::upperLimitSpecificationChanged);

	// labels
	connect(ui.chbLabelsEnabled, &QCheckBox::clicked, this, &ProcessBehaviorChartDock::labelsEnabledChanged);
	connect(ui.sbLabelsPrecision, QOverload<int>::of(&QSpinBox::valueChanged), this, &ProcessBehaviorChartDock::labelsPrecisionChanged);
	connect(ui.chkLabelsAutoPrecision, &QCheckBox::toggled, this, &ProcessBehaviorChartDock::labelsAutoPrecisionChanged);
	connect(ui.kfrLabelsFont, &KFontRequester::fontSelected, this, &ProcessBehaviorChartDock::labelsFontChanged);
	connect(ui.kcbLabelsFontColor, &KColorButton::changed, this, &ProcessBehaviorChartDock::labelsFontColorChanged);
	connect(ui.kcbLabelsBackgroundColor, &KColorButton::changed, this, &ProcessBehaviorChartDock::labelsBackgroundColorChanged);
	connect(ui.cbLabelsBorderShape, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ProcessBehaviorChartDock::labelsBorderShapeChanged);

	// template handler
	auto* frame = new QFrame(this);
	auto* layout = new QHBoxLayout(frame);
	layout->setContentsMargins(0, 0, 0, 0);

	auto* templateHandler = new TemplateHandler(this, QLatin1String("ProcessBehaviorChart"));
	layout->addWidget(templateHandler);
	connect(templateHandler, &TemplateHandler::loadConfigRequested, this, &ProcessBehaviorChartDock::loadConfigFromTemplate);
	connect(templateHandler, &TemplateHandler::saveConfigRequested, this, &ProcessBehaviorChartDock::saveConfigAsTemplate);
	connect(templateHandler, &TemplateHandler::info, this, &ProcessBehaviorChartDock::info);

	ui.verticalLayout->addWidget(frame);
}

ProcessBehaviorChartDock::~ProcessBehaviorChartDock() = default;

void ProcessBehaviorChartDock::setModel() {
	auto* model = aspectModel();
	model->enablePlottableColumnsOnly(true);
	model->enableShowPlotDesignation(true);
	model->setSelectableAspects({AspectType::Column});
	cbDataColumn->setTopLevelClasses(TreeViewComboBox::plotColumnTopLevelClasses());
	cbDataColumn->setModel(model);
	cbData2Column->setTopLevelClasses(TreeViewComboBox::plotColumnTopLevelClasses());
	cbData2Column->setModel(model);
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
		if (m_plot->lowerLimitAvailable())
			lowerLimitLines << plot->lowerLimitLine();
	}
	dataLineWidget->setLines(dataLines);
	dataSymbolWidget->setSymbols(dataSymbols);
	centerLineWidget->setLines(centerLines);
	upperLimitLineWidget->setLines(upperLimitLines);
	if (m_plot->lowerLimitAvailable())
		lowerLimitLineWidget->setLines(lowerLimitLines);

	// if there are more then one curve in the list, disable the content in the tab "general"
	if (m_plots.size() == 1) {
		cbDataColumn->setEnabled(true);
		cbDataColumn->setAspect(m_plot->dataColumn(), m_plot->dataColumnPath());
		cbData2Column->setEnabled(true);
		cbData2Column->setAspect(m_plot->data2Column(), m_plot->data2ColumnPath());
	} else {
		cbDataColumn->setEnabled(false);
		cbDataColumn->setCurrentModelIndex(QModelIndex());
		cbData2Column->setEnabled(false);
		cbData2Column->setCurrentModelIndex(QModelIndex());
	}

	ui.chkLegendVisible->setChecked(m_plot->legendVisible());
	ui.chkVisible->setChecked(m_plot->isVisible());

	// load the remaining properties
	load();

	showStatusInfo(QString()); // remove the message from the previous chart, if available
	updatePlotRangeList();

	// Slots
	// General-tab
	connect(m_plot, &ProcessBehaviorChart::dataColumnChanged, this, &ProcessBehaviorChartDock::plotDataColumnChanged);
	connect(m_plot, &ProcessBehaviorChart::data2ColumnChanged, this, &ProcessBehaviorChartDock::plotData2ColumnChanged);
	connect(m_plot, &ProcessBehaviorChart::typeChanged, this, &ProcessBehaviorChartDock::plotTypeChanged);
	connect(m_plot, &ProcessBehaviorChart::limitsTypeChanged, this, &ProcessBehaviorChartDock::plotLimitsTypeChanged);
	connect(m_plot, &ProcessBehaviorChart::limitsMetricChanged, this, &ProcessBehaviorChartDock::plotLimitsMetricChanged);
	connect(m_plot, &ProcessBehaviorChart::sampleSizeChanged, this, &ProcessBehaviorChartDock::plotSampleSizeChanged);
	connect(m_plot, &ProcessBehaviorChart::maxUpperLimitChanged, this, &ProcessBehaviorChartDock::plotMaxUpperLimitChanged);
	connect(m_plot, &ProcessBehaviorChart::minLowerLimitChanged, this, &ProcessBehaviorChartDock::plotMinLowerLimitChanged);
	connect(m_plot, &ProcessBehaviorChart::exactLimitsEnabledChanged, this, &ProcessBehaviorChartDock::plotExactLimitsEnabledChanged);
	connect(m_plot, &ProcessBehaviorChart::statusInfo, this, &ProcessBehaviorChartDock::showStatusInfo);
	connect(m_plot, &ProcessBehaviorChart::recalculated, this, &ProcessBehaviorChartDock::updateLowerLimitWidgets);

	// Labels-tab
	connect(m_plot, &ProcessBehaviorChart::labelsEnabledChanged, this, &ProcessBehaviorChartDock::plotLabelsEnabledChanged);
	connect(m_plot, &ProcessBehaviorChart::labelsFontChanged, this, &ProcessBehaviorChartDock::plotLabelsFontChanged);
	connect(m_plot, &ProcessBehaviorChart::labelsFontColorChanged, this, &ProcessBehaviorChartDock::plotLabelsFontColorChanged);
	connect(m_plot, &ProcessBehaviorChart::labelsBackgroundColorChanged, this, &ProcessBehaviorChartDock::plotLabelsBackgroundColorChanged);
	connect(m_plot, &ProcessBehaviorChart::labelsBorderShapeChanged, this, &ProcessBehaviorChartDock::plotLabelsBorderShapeChanged);
}

void ProcessBehaviorChartDock::retranslateUi() {
	ui.cbLimitsType->clear();
	ui.cbLimitsType->addItem(i18n("Statistical"), static_cast<int>(ProcessBehaviorChart::LimitsType::Statistical));
	ui.cbLimitsType->addItem(i18n("Specification"), static_cast<int>(ProcessBehaviorChart::LimitsType::Specification));

	ui.cbLimitsMetric->clear();
	ui.cbLimitsMetric->addItem(i18n("Average"), static_cast<int>(ProcessBehaviorChart::LimitsMetric::Average));
	ui.cbLimitsMetric->addItem(i18n("Median"), static_cast<int>(ProcessBehaviorChart::LimitsMetric::Median));

	ui.cbLabelsBorderShape->clear();
	ui.cbLabelsBorderShape->addItem(i18n("No Border"), static_cast<int>(TextLabel::BorderShape::NoBorder));
	ui.cbLabelsBorderShape->addItem(i18n("Rectangle"), static_cast<int>(TextLabel::BorderShape::Rect));
	ui.cbLabelsBorderShape->addItem(i18n("Ellipse"), static_cast<int>(TextLabel::BorderShape::Ellipse));
	ui.cbLabelsBorderShape->addItem(i18n("Round sided rectangle"), static_cast<int>(TextLabel::BorderShape::RoundSideRect));
	ui.cbLabelsBorderShape->addItem(i18n("Round corner rectangle"), static_cast<int>(TextLabel::BorderShape::RoundCornerRect));
	ui.cbLabelsBorderShape->addItem(i18n("Inwards round corner rectangle"), static_cast<int>(TextLabel::BorderShape::InwardsRoundCornerRect));
	ui.cbLabelsBorderShape->addItem(i18n("Dented border rectangle"), static_cast<int>(TextLabel::BorderShape::DentedBorderRect));
	ui.cbLabelsBorderShape->addItem(i18n("Cuboid"), static_cast<int>(TextLabel::BorderShape::Cuboid));
	ui.cbLabelsBorderShape->addItem(i18n("Up pointing rectangle"), static_cast<int>(TextLabel::BorderShape::UpPointingRectangle));
	ui.cbLabelsBorderShape->addItem(i18n("Down pointing rectangle"), static_cast<int>(TextLabel::BorderShape::DownPointingRectangle));
	ui.cbLabelsBorderShape->addItem(i18n("Left pointing rectangle"), static_cast<int>(TextLabel::BorderShape::LeftPointingRectangle));
	ui.cbLabelsBorderShape->addItem(i18n("Right pointing rectangle"), static_cast<int>(TextLabel::BorderShape::RightPointingRectangle));

	// tooltips
	QString info = i18n(
		"The supported chart types are grouped according to the plotted statistics and to the metric defining the limits.<br><br>"
		"Individual Values and Moving Ranges, Limits Based on the Average or Median Moving Range:"
		"<ul>"
		"<li>X (XmR) - plot the <b>individual labels</b>.</li>"
		"<li>mR - plot the <b>moving ranges</b>.</li>"
		"</ul>"
		"Averages and Ranges, Limits based on the Average or Median Range:"
		"<ul>"
		"<li>X̅  (X̅R) - plot the <b>averages for each sample</b> .</li>"
		"<li>R (X̅R) - plot the <b>ranges for each sample</b>.</li>"
		"</ul>"
		"Averages and Standard Deviations, Limits Based on the Standard Deviations:"
		"<ul>"
		"<li>X̅  (X̅S) - plot the <b>averages for each sample</b>.</li>"
		"<li>S (X̅S) - plot the <b>standard deviations for each sample</b>.</li>"
		"</ul>"
		"Attributes:"
		"<ul>"
		"<li>P - plot <b>binomial proportions</b>.</li>"
		"<li>NP - plot <b>binomial counts</b>.</li>"
		"<li>C - plot <b>Poisson counts</b>.</li>"
		"<li>U - plot <b>Poisson rates</b>.</li>"
		"</ul>");
	ui.lType->setToolTip(info);
	ui.cbType->setToolTip(info);

	info = i18n(
		"Determine which values to use for the center and limit lines:"
		"<ul>"
		"<li>Statistical - values are calculated based on the statistical properties of the sample.</li>"
		"<li>Specification - custom user-defined values are used.</li>"
		"</ul>");
	ui.lLimitsType->setToolTip(info);
	ui.cbLimitsType->setToolTip(info);

	info = i18n("Maximal value for the upper control limit. No constraint, if empty.");
	ui.lMaxUpperLimit->setToolTip(info);
	ui.leMaxUpperLimit->setToolTip(info);

	info = i18n("Minimal value for the lower control limit. No constraint, if empty.");
	ui.lMinLowerLimit->setToolTip(info);
	ui.leMinLowerLimit->setToolTip(info);

	info = i18n("If checked, exact limits are calculated for every individual sample (\"stair-step limits\"), straight lines are drawn for the limits otherwise.");
	ui.lExactLimits->setToolTip(info);
	ui.chbExactLimits->setToolTip(info);
}

/*
 * updates the locale in the widgets. called when the application settins are changed.
 */
void ProcessBehaviorChartDock::updateLocale() {
	const auto numberLocale = QLocale();
	ui.leMaxUpperLimit->setLocale(numberLocale);
	ui.leMinLowerLimit->setLocale(numberLocale);
	dataLineWidget->updateLocale();
	dataSymbolWidget->updateLocale();
	centerLineWidget->updateLocale();
	upperLimitLineWidget->updateLocale();
	lowerLimitLineWidget->updateLocale();
	labelsBorderLineWidget->updateLocale();
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

void ProcessBehaviorChartDock::data2ColumnChanged(const QModelIndex& index) {
	if (m_initializing)
		return;

	auto aspect = static_cast<AbstractAspect*>(index.internalPointer());
	AbstractColumn* column(nullptr);
	if (aspect) {
		column = dynamic_cast<AbstractColumn*>(aspect);
		Q_ASSERT(column);
	}

	for (auto* plot : m_plots)
		plot->setData2Column(column);
}

void ProcessBehaviorChartDock::typeChanged(int) {
	const auto type = static_cast<ProcessBehaviorChart::Type>(ui.cbType->currentData().toInt());

	// depending on the current type, show/hide the settings for the sample type
	bool visible = (type == ProcessBehaviorChart::Type::XbarR || type == ProcessBehaviorChart::Type::R || type == ProcessBehaviorChart::Type::XbarS
					|| type == ProcessBehaviorChart::Type::S || type == ProcessBehaviorChart::Type::NP);
	ui.lSampleSize->setVisible(visible);
	ui.sbSampleSize->setVisible(visible);

	// depending on the current type, show/hide the settings for the metric used to define the limits
	visible = (type == ProcessBehaviorChart::Type::XmR || type == ProcessBehaviorChart::Type::mR || type == ProcessBehaviorChart::Type::XbarR
				|| type == ProcessBehaviorChart::Type::R);
	ui.lLimitsMetric->setVisible(visible);
	ui.cbLimitsMetric->setVisible(visible);

	// constraints for the limits are updated in ProcessBehaviorChart
	// and signals are emitted on changes to update the dock

	// second data column
	visible = (type == ProcessBehaviorChart::Type::P || type == ProcessBehaviorChart::Type::U);
	ui.lData2Column->setVisible(visible);
	cbData2Column->setVisible(visible);
	ui.lExactLimits->setVisible(visible);
	ui.chbExactLimits->setVisible(visible);

	CONDITIONAL_LOCK_RETURN;
	for (auto* plot : m_plots)
		plot->setType(type);
}

void ProcessBehaviorChartDock::limitsTypeChanged(int) {
	const auto limitsType = static_cast<ProcessBehaviorChart::LimitsType>(ui.cbLimitsType->currentData().toInt());
	const bool statistical = (limitsType == ProcessBehaviorChart::LimitsType::Statistical);
	ui.lLimitsMetric->setVisible(statistical);
	ui.cbLimitsMetric->setVisible(statistical);
	ui.lCenterSpecification->setVisible(!statistical);
	ui.leCenterSpecification->setVisible(!statistical);
	ui.lUpperLimitSpecification->setVisible(!statistical);
	ui.leUpperLimitSpecification->setVisible(!statistical);
	ui.lLowerLimitSpecification->setVisible(!statistical);
	ui.leLowerLimitSpecification->setVisible(!statistical);
	ui.lMaxUpperLimit->setVisible(statistical);
	ui.leMaxUpperLimit->setVisible(statistical);
	ui.lMinLowerLimit->setVisible(statistical);
	ui.leMinLowerLimit->setVisible(statistical);
	ui.lExactLimits->setVisible(statistical);
	ui.chbExactLimits->setVisible(statistical);
	CONDITIONAL_LOCK_RETURN;
	for (auto* plot : m_plots)
		plot->setLimitsType(limitsType);
}

void ProcessBehaviorChartDock::limitsMetricChanged(int) {
	CONDITIONAL_LOCK_RETURN;
	const auto limitsMetric = static_cast<ProcessBehaviorChart::LimitsMetric>(ui.cbLimitsMetric->currentData().toInt());
	for (auto* plot : m_plots)
		plot->setLimitsMetric(limitsMetric);
}

void ProcessBehaviorChartDock::sampleSizeChanged(int value) {
	CONDITIONAL_LOCK_RETURN;
	for (auto* plot : m_plots)
		plot->setSampleSize(value);
}

void ProcessBehaviorChartDock::maxUpperLimitChanged() {
	double max = INFINITY;
	const QString& value = ui.leMaxUpperLimit->text();
	if (!value.isEmpty()) {
		bool ok;
		max = QLocale().toDouble(value, &ok);
		if (!ok)
			return;
	}

	CONDITIONAL_LOCK_RETURN;
	for (auto* plot : m_plots)
		plot->setMaxUpperLimit(max);;
}

void ProcessBehaviorChartDock::minLowerLimitChanged() {
	double min = -INFINITY;
	const QString& value = ui.leMinLowerLimit->text();
	if (!value.isEmpty()) {
		bool ok;
		min = QLocale().toDouble(value, &ok);
		if (!ok)
			return;
	}

	CONDITIONAL_LOCK_RETURN;
	for (auto* plot : m_plots)
		plot->setMinLowerLimit(min);;
}

void ProcessBehaviorChartDock::exactLimitsEnabledChanged(bool enabled) {
	CONDITIONAL_LOCK_RETURN;
	for (auto* plot : m_plots)
		plot->setExactLimitsEnabled(enabled);
}

void ProcessBehaviorChartDock::centerSpecificationChanged() {
	double spec = NAN;
	const QString& value = ui.leCenterSpecification->text();
	if (!value.isEmpty()) {
		bool ok;
		spec = QLocale().toDouble(value, &ok);
		if (!ok)
			return;
	}

	CONDITIONAL_LOCK_RETURN;
	for (auto* plot : m_plots)
		plot->setCenterSpecification(spec);
}

void ProcessBehaviorChartDock::lowerLimitSpecificationChanged() {
	double spec = NAN;
	const QString& value = ui.leLowerLimitSpecification->text();
	if (!value.isEmpty()) {
		bool ok;
		spec = QLocale().toDouble(value, &ok);
		if (!ok)
			return;
	}

	CONDITIONAL_LOCK_RETURN;
	for (auto* plot : m_plots)
		plot->setLowerLimitSpecification(spec);
}

void ProcessBehaviorChartDock::upperLimitSpecificationChanged() {
	double spec = NAN;
	const QString& value = ui.leUpperLimitSpecification->text();
	if (!value.isEmpty()) {
		bool ok;
		spec = QLocale().toDouble(value, &ok);
		if (!ok)
			return;
	}

	CONDITIONAL_LOCK_RETURN;
	for (auto* plot : m_plots)
		plot->setUpperLimitSpecification(spec);;
}
/*!
 * toggle the properties for the lower limit line if the lower limit is not available,
 * called every time the chart is re-calculated since the precense of the lower limit
 * is depending on multiple factors and on the current result for the control limits.
 */
void ProcessBehaviorChartDock::updateLowerLimitWidgets() {
	const bool visible = m_plot->lowerLimitAvailable();
	ui.lLowerLimit->setVisible(visible);
	lowerLimitLineWidget->setVisible(visible);
}

// Labels-tab
void ProcessBehaviorChartDock::labelsEnabledChanged(bool enabled) {
	ui.lLabelsText->setVisible(enabled);
	ui.lLabelsPrecision->setVisible(enabled);
	ui.frameLabelsPrecision->setVisible(enabled);
	ui.lLabelsFont->setVisible(enabled);
	ui.kfrLabelsFont->setVisible(enabled);
	ui.lLabelsFontColor->setVisible(enabled);
	ui.kcbLabelsFontColor->setVisible(enabled);
	ui.lLabelsBackgroundColor->setVisible(enabled);
	ui.kcbLabelsBackgroundColor->setVisible(enabled);
	ui.lLabelsBorder->setVisible(enabled);
	ui.lLabelsBorderShape->setVisible(enabled);
	ui.cbLabelsBorderShape->setVisible(enabled);

	const auto shape = static_cast<TextLabel::BorderShape>(ui.cbLabelsBorderShape->currentData().toInt());
	const bool visible = (shape != TextLabel::BorderShape::NoBorder);
	labelsBorderLineWidget->setVisible(enabled && visible);

	CONDITIONAL_LOCK_RETURN;
	for (auto* plot : m_plots)
		plot->setLabelsEnabled(enabled);
}

void ProcessBehaviorChartDock::labelsPrecisionChanged(int value) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* plot : m_plots)
		plot->setLabelsPrecision(value);
}

void ProcessBehaviorChartDock::labelsAutoPrecisionChanged(bool state) {
	ui.sbLabelsPrecision->setEnabled(!state);

	CONDITIONAL_LOCK_RETURN;

	for (auto* plot : m_plots)
		plot->setLabelsAutoPrecision(state);
}

void ProcessBehaviorChartDock::labelsFontChanged(const QFont& font) {
	CONDITIONAL_LOCK_RETURN;
	for (auto* plot : m_plots)
		plot->setLabelsFont(font);
}
void ProcessBehaviorChartDock::labelsFontColorChanged(const QColor& color) {
	CONDITIONAL_LOCK_RETURN;
	for (auto* plot : m_plots)
		plot->setLabelsFontColor(color);
}

void ProcessBehaviorChartDock::labelsBackgroundColorChanged(const QColor& color) {
	CONDITIONAL_LOCK_RETURN;
	for (auto* plot : m_plots)
		plot->setLabelsBackgroundColor(color);
}

void ProcessBehaviorChartDock::labelsBorderShapeChanged(int) {
	const auto shape = static_cast<TextLabel::BorderShape>(ui.cbLabelsBorderShape->currentData().toInt());
	const bool visible = (shape != TextLabel::BorderShape::NoBorder);
	labelsBorderLineWidget->setVisible(visible);

	CONDITIONAL_LOCK_RETURN;
	for (auto* plot : m_plots)
		plot->setLabelsBorderShape(shape);
}

//*************************************************************
//**** SLOTs for changes triggered in ProcessBehaviorChart ****
//*************************************************************
// General-Tab
void ProcessBehaviorChartDock::plotDataColumnChanged(const AbstractColumn* column) {
	CONDITIONAL_LOCK_RETURN;
	cbDataColumn->setAspect(column, m_plot->dataColumnPath());
}

void ProcessBehaviorChartDock::plotData2ColumnChanged(const AbstractColumn* column) {
	CONDITIONAL_LOCK_RETURN;
	cbData2Column->setAspect(column, m_plot->data2ColumnPath());
}

void ProcessBehaviorChartDock::plotTypeChanged(ProcessBehaviorChart::Type type) {
	CONDITIONAL_LOCK_RETURN;
	const int index = ui.cbType->findData(static_cast<int>(type));
	ui.cbType->setCurrentIndex(index);
}

void ProcessBehaviorChartDock::plotLimitsTypeChanged(ProcessBehaviorChart::LimitsType type) {
	CONDITIONAL_LOCK_RETURN;
	const int index = ui.cbLimitsType->findData(static_cast<int>(type));
	ui.cbLimitsType->setCurrentIndex(index);
}

void ProcessBehaviorChartDock::plotLimitsMetricChanged(ProcessBehaviorChart::LimitsMetric limitsMetric) {
	CONDITIONAL_LOCK_RETURN;
	const int index = ui.cbLimitsMetric->findData(static_cast<int>(limitsMetric));
	ui.cbLimitsMetric->setCurrentIndex(index);
}

void ProcessBehaviorChartDock::plotSampleSizeChanged(int value) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbSampleSize->setValue(value);
}

void setValue(QLineEdit* le, double value, const QLocale& locale) {
	if (std::isfinite(value))
		le->setText(locale.toString(value));
	else
		le->setText(QString());
}

void ProcessBehaviorChartDock::plotMinLowerLimitChanged(double value) {
	CONDITIONAL_LOCK_RETURN;
	setValue(ui.leMinLowerLimit, value, QLocale());
}

void ProcessBehaviorChartDock::plotMaxUpperLimitChanged(double value) {
	CONDITIONAL_LOCK_RETURN;
	setValue(ui.leMaxUpperLimit, value, QLocale());
}

void ProcessBehaviorChartDock::plotExactLimitsEnabledChanged(bool enabled) {
	CONDITIONAL_LOCK_RETURN;
	ui.chbExactLimits->setChecked(enabled);
}

void ProcessBehaviorChartDock::plotCenterSpecificationChanged(double value) {
	CONDITIONAL_LOCK_RETURN;
	setValue(ui.leCenterSpecification, value, QLocale());
}

void ProcessBehaviorChartDock::plotLowerLimitSpecificationChanged(double value) {
	CONDITIONAL_LOCK_RETURN;
	setValue(ui.leLowerLimitSpecification, value, QLocale());
}

void ProcessBehaviorChartDock::plotUpperLimitSpecificationChanged(double value) {
	CONDITIONAL_LOCK_RETURN;
	setValue(ui.leUpperLimitSpecification, value, QLocale());
}

// Labels-tab
void ProcessBehaviorChartDock::plotLabelsEnabledChanged(bool enabled) {
	CONDITIONAL_LOCK_RETURN;
	ui.chbLabelsEnabled->setChecked(enabled);
}

void ProcessBehaviorChartDock::plotLabelsAutoPrecisionChanged(bool on) {
	CONDITIONAL_LOCK_RETURN;
	ui.chkLabelsAutoPrecision->setChecked(on);
}

void ProcessBehaviorChartDock::plotLabelsPrecisionChanged(int precision) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbLabelsPrecision->setValue(precision);
}

void ProcessBehaviorChartDock::plotLabelsFontChanged(const QFont& font) {
	CONDITIONAL_LOCK_RETURN;
	ui.kfrLabelsFont->setFont(font);
}

void ProcessBehaviorChartDock::plotLabelsFontColorChanged(const QColor& color) {
	CONDITIONAL_LOCK_RETURN;
	ui.kcbLabelsFontColor->setColor(color);
}

void ProcessBehaviorChartDock::plotLabelsBackgroundColorChanged(const QColor& color) {
	CONDITIONAL_LOCK_RETURN;
	ui.kcbLabelsBackgroundColor->setColor(color);
}

void ProcessBehaviorChartDock::plotLabelsBorderShapeChanged(TextLabel::BorderShape shape) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbLabelsBorderShape->setCurrentIndex(static_cast<int>(shape));
}

void ProcessBehaviorChartDock::showStatusInfo(const QString& info) {
	if (info.isEmpty()) {
		if (m_messageWidget && m_messageWidget->isVisible())
			m_messageWidget->close();
	} else {
		if (!m_messageWidget) {
			m_messageWidget = new KMessageWidget(this);
			m_messageWidget->setMessageType(KMessageWidget::Warning);
			static_cast<QGridLayout*>(ui.tabGeneral->layout())->addWidget(m_messageWidget, 11, 0, 1, 3);
		}
		m_messageWidget->setText(info);
		m_messageWidget->animatedShow();
		QDEBUG(info);
	}
}

//*************************************************************
//************************* Settings **************************
//*************************************************************
void ProcessBehaviorChartDock::load() {
	// type
	int index = ui.cbType->findData(static_cast<int>(m_plot->type()));
	ui.cbType->setCurrentIndex(index);
	typeChanged(index);

	// type
	index = ui.cbLimitsType->findData(static_cast<int>(m_plot->limitsType()));
	ui.cbLimitsType->setCurrentIndex(index);
	limitsTypeChanged(index);

	// limits metric
	index = ui.cbLimitsMetric->findData(static_cast<int>(m_plot->limitsMetric()));
	ui.cbLimitsMetric->setCurrentIndex(index);

	// sample size
	ui.sbSampleSize->setValue(static_cast<int>(m_plot->sampleSize()));

	// constraints for the limits
	const auto numberLocale = QLocale();
	setValue(ui.leMinLowerLimit, m_plot->minLowerLimit(), numberLocale);
	setValue(ui.leMaxUpperLimit, m_plot->maxUpperLimit(), numberLocale);

	// specification values
	setValue(ui.leCenterSpecification, m_plot->centerSpecification(), numberLocale);
	setValue(ui.leLowerLimitSpecification, m_plot->lowerLimitSpecification(), numberLocale);
	setValue(ui.leUpperLimitSpecification, m_plot->upperLimitSpecification(), numberLocale);

	// user exact/individual limits, relevant for P and U charts only
	ui.chbExactLimits->setChecked(m_plot->exactLimitsEnabled());

	updateLowerLimitWidgets();

	// labels
	ui.chbLabelsEnabled->setChecked(m_plot->labelsEnabled());
	ui.chkLabelsAutoPrecision->setChecked((int)m_plot->labelsAutoPrecision());
	ui.sbLabelsPrecision->setValue((int)m_plot->labelsPrecision());
	ui.kfrLabelsFont->setFont(m_plot->labelsFont());
	ui.kcbLabelsFontColor->setColor(m_plot->labelsFontColor());
	ui.kcbLabelsBackgroundColor->setColor(m_plot->labelsBackgroundColor());

	// Border
	index = ui.cbLabelsBorderShape->findData(static_cast<int>(m_plot->labelsBorderShape()));
	ui.cbLabelsBorderShape->setCurrentIndex(index);

	QList<Line*> borderLines;
	for (auto* plot : m_plots)
		borderLines << plot->labelsBorderLine();
	labelsBorderLineWidget->setLines(borderLines);

	labelsEnabledChanged(ui.chbLabelsEnabled->isChecked());
}

void ProcessBehaviorChartDock::loadConfig(KConfig& config) {
	KConfigGroup group = config.group(QStringLiteral("ProcessBehaviorChart"));

	// type
	const auto type = group.readEntry(QStringLiteral("Type"), static_cast<int>(m_plot->type()));
	int index = ui.cbType->findData(static_cast<int>(type));
	ui.cbType->setCurrentIndex(index);
	typeChanged(index);

	// limits type
	const auto limitsType = group.readEntry(QStringLiteral("LimitsType"), static_cast<int>(m_plot->limitsType()));
	index = ui.cbLimitsType->findData(static_cast<int>(limitsType));
	ui.cbLimitsType->setCurrentIndex(index);
	limitsTypeChanged(index);

	// limits metric
	const auto limitsMetric = group.readEntry(QStringLiteral("LimitsMetric"), static_cast<int>(m_plot->limitsMetric()));
	index = ui.cbLimitsMetric->findData(static_cast<int>(limitsMetric));
	ui.cbLimitsMetric->setCurrentIndex(index);

	// sample size
	const int size = group.readEntry(QStringLiteral("SampleSize"), static_cast<int>(m_plot->sampleSize()));
	ui.sbSampleSize->setValue(size);

	// TODO: limit constraints?

	// user exact/individual limits, relevant for P and U charts only
	ui.chbExactLimits->setChecked(group.readEntry(QStringLiteral("ExactLimitsEnabled"), false));

	ui.chbLabelsEnabled->setChecked(group.readEntry(QStringLiteral("LabelsEnabled"), false));
	labelsEnabledChanged(ui.chbLabelsEnabled->isChecked());
	ui.chkLabelsAutoPrecision->setChecked(group.readEntry(QStringLiteral("LabelsAutoPrecision"), (int)m_plot->labelsAutoPrecision()));
	ui.sbLabelsPrecision->setValue(group.readEntry(QStringLiteral("LabelsPrecision"), (int)m_plot->labelsPrecision()));

	// properties of the data and limit curves
	dataLineWidget->loadConfig(group);
	dataSymbolWidget->loadConfig(group);
	centerLineWidget->loadConfig(group);
	upperLimitLineWidget->loadConfig(group);
	if (m_plot->lowerLimitAvailable())
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
	group.writeEntry(QStringLiteral("Type"), static_cast<int>(m_plot->type()));
	group.writeEntry(QStringLiteral("LimitsMetric"), static_cast<int>(m_plot->limitsMetric()));
	group.writeEntry(QStringLiteral("SampleSize"), m_plot->sampleSize());
	// TODO: limit constraints?
	group.writeEntry(QStringLiteral("ExactLimitsEnabled"), m_plot->exactLimitsEnabled());

	// properties of the data and limit curves
	dataLineWidget->saveConfig(group);
	dataSymbolWidget->saveConfig(group);
	centerLineWidget->saveConfig(group);
	upperLimitLineWidget->saveConfig(group);
	if (m_plot->lowerLimitAvailable())
		lowerLimitLineWidget->saveConfig(group);
	config.sync();
}
