/*
	File                 : StatisticsSpreadsheetDock.cpp
	Project              : LabPlot
	Description          : widget for statistics spreadsheet properties
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023-2024 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "StatisticsSpreadsheetDock.h"

#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/spreadsheet/StatisticsSpreadsheet.h"
#include "kdefrontend/TemplateHandler.h"

#include <KConfig>
#include <KConfigGroup>

/*!
 \class StatisticsSpreadsheetDock
 \brief Provides a widget for editing which statistical metrics of the parent spreadsheet should be shown in the statistics spreadsheet.

 \ingroup kdefrontend
*/

StatisticsSpreadsheetDock::StatisticsSpreadsheetDock(QWidget* parent)
	: BaseDock(parent) {
	ui.setupUi(this);

	m_mappingComboBoxMetric[ui.cbCount] = StatisticsSpreadsheet::Metric::Count;
	m_mappingComboBoxMetric[ui.cbMinimum] = StatisticsSpreadsheet::Metric::Minimum;
	m_mappingComboBoxMetric[ui.cbMaximum] = StatisticsSpreadsheet::Metric::Maximum;
	m_mappingComboBoxMetric[ui.cbArithmeticMean] = StatisticsSpreadsheet::Metric::ArithmeticMean;
	m_mappingComboBoxMetric[ui.cbGeometricMean] = StatisticsSpreadsheet::Metric::GeometricMean;
	m_mappingComboBoxMetric[ui.cbHarmonicMean] = StatisticsSpreadsheet::Metric::HarmonicMean;
	m_mappingComboBoxMetric[ui.cbContraharmonicMean] = StatisticsSpreadsheet::Metric::ContraharmonicMean;
	m_mappingComboBoxMetric[ui.cbMode] = StatisticsSpreadsheet::Metric::Mode;
	m_mappingComboBoxMetric[ui.cbFirstQuartile] = StatisticsSpreadsheet::Metric::FirstQuartile;
	m_mappingComboBoxMetric[ui.cbMedian] = StatisticsSpreadsheet::Metric::Median;
	m_mappingComboBoxMetric[ui.cbThirdQuartile] = StatisticsSpreadsheet::Metric::ThirdQuartile;
	m_mappingComboBoxMetric[ui.cbTrimean] = StatisticsSpreadsheet::Metric::Trimean;
	m_mappingComboBoxMetric[ui.cbRange] = StatisticsSpreadsheet::Metric::Range;
	m_mappingComboBoxMetric[ui.cbVariance] = StatisticsSpreadsheet::Metric::Variance;
	m_mappingComboBoxMetric[ui.cbStandardDeviation] = StatisticsSpreadsheet::Metric::StandardDeviation;
	m_mappingComboBoxMetric[ui.cbMeanDeviation] = StatisticsSpreadsheet::Metric::MeanDeviation;
	m_mappingComboBoxMetric[ui.cbMeanDeviationAroundMedian] = StatisticsSpreadsheet::Metric::MeanDeviationAroundMedian;
	m_mappingComboBoxMetric[ui.cbMedianDeviation] = StatisticsSpreadsheet::Metric::MedianDeviation;
	m_mappingComboBoxMetric[ui.cbIQR] = StatisticsSpreadsheet::Metric::IQR;
	m_mappingComboBoxMetric[ui.cbSkewness] = StatisticsSpreadsheet::Metric::Skewness;
	m_mappingComboBoxMetric[ui.cbKurtosis] = StatisticsSpreadsheet::Metric::Kurtosis;
	m_mappingComboBoxMetric[ui.cbEntropy] = StatisticsSpreadsheet::Metric::Entropy;

	ui.bSelectAll->setIcon(QIcon::fromTheme(QLatin1String("edit-select-symbolic")));
	ui.bSelectNone->setIcon(QIcon::fromTheme(QLatin1String("edit-none-symbolic")));

	connect(ui.cbCount, &QCheckBox::toggled, this, &StatisticsSpreadsheetDock::metricChanged);
	connect(ui.cbMinimum, &QCheckBox::toggled, this, &StatisticsSpreadsheetDock::metricChanged);
	connect(ui.cbMaximum, &QCheckBox::toggled, this, &StatisticsSpreadsheetDock::metricChanged);
	connect(ui.cbArithmeticMean, &QCheckBox::toggled, this, &StatisticsSpreadsheetDock::metricChanged);
	connect(ui.cbGeometricMean, &QCheckBox::toggled, this, &StatisticsSpreadsheetDock::metricChanged);
	connect(ui.cbHarmonicMean, &QCheckBox::toggled, this, &StatisticsSpreadsheetDock::metricChanged);
	connect(ui.cbContraharmonicMean, &QCheckBox::toggled, this, &StatisticsSpreadsheetDock::metricChanged);
	connect(ui.cbMode, &QCheckBox::toggled, this, &StatisticsSpreadsheetDock::metricChanged);
	connect(ui.cbFirstQuartile, &QCheckBox::toggled, this, &StatisticsSpreadsheetDock::metricChanged);
	connect(ui.cbMedian, &QCheckBox::toggled, this, &StatisticsSpreadsheetDock::metricChanged);
	connect(ui.cbThirdQuartile, &QCheckBox::toggled, this, &StatisticsSpreadsheetDock::metricChanged);
	connect(ui.cbTrimean, &QCheckBox::toggled, this, &StatisticsSpreadsheetDock::metricChanged);
	connect(ui.cbRange, &QCheckBox::toggled, this, &StatisticsSpreadsheetDock::metricChanged);
	connect(ui.cbVariance, &QCheckBox::toggled, this, &StatisticsSpreadsheetDock::metricChanged);
	connect(ui.cbStandardDeviation, &QCheckBox::toggled, this, &StatisticsSpreadsheetDock::metricChanged);
	connect(ui.cbMeanDeviation, &QCheckBox::toggled, this, &StatisticsSpreadsheetDock::metricChanged);
	connect(ui.cbMeanDeviationAroundMedian, &QCheckBox::toggled, this, &StatisticsSpreadsheetDock::metricChanged);
	connect(ui.cbMedianDeviation, &QCheckBox::toggled, this, &StatisticsSpreadsheetDock::metricChanged);
	connect(ui.cbIQR, &QCheckBox::toggled, this, &StatisticsSpreadsheetDock::metricChanged);
	connect(ui.cbSkewness, &QCheckBox::toggled, this, &StatisticsSpreadsheetDock::metricChanged);
	connect(ui.cbKurtosis, &QCheckBox::toggled, this, &StatisticsSpreadsheetDock::metricChanged);
	connect(ui.cbEntropy, &QCheckBox::toggled, this, &StatisticsSpreadsheetDock::metricChanged);

	connect(ui.bSelectAll, &QPushButton::clicked, this, &StatisticsSpreadsheetDock::selectAll);
	connect(ui.bSelectNone, &QPushButton::clicked, this, &StatisticsSpreadsheetDock::selectNone);

	// templates
	auto* templateHandler = new TemplateHandler(this, QLatin1String("StatisticsSpreadsheet"));
	ui.verticalLayout->addWidget(templateHandler);
	templateHandler->show();
	connect(templateHandler, &TemplateHandler::loadConfigRequested, this, &StatisticsSpreadsheetDock::loadConfigFromTemplate);
	connect(templateHandler, &TemplateHandler::saveConfigRequested, this, &StatisticsSpreadsheetDock::saveConfigAsTemplate);
	connect(templateHandler, &TemplateHandler::info, this, &StatisticsSpreadsheetDock::info);
}

/*!
	set the current spreadsheet(s)
*/
void StatisticsSpreadsheetDock::setSpreadsheets(const QList<StatisticsSpreadsheet*> list) {
	m_spreadsheets = list;
	m_spreadsheet = list.first();
	load();
}

void StatisticsSpreadsheetDock::metricChanged(bool state) {
	auto* cb = static_cast<QCheckBox*>(QObject::sender());
	if (!m_mappingComboBoxMetric.contains(cb))
		return;

	const auto metric = m_mappingComboBoxMetric[cb];
	auto metrics = m_spreadsheet->metrics();
	metrics.setFlag(metric, state);

	for (auto* spreadsheet : m_spreadsheets)
		spreadsheet->setMetrics(metrics);
}

void StatisticsSpreadsheetDock::selectAll() {
	StatisticsSpreadsheet::Metrics metrics;
	for (const auto& metric : m_mappingComboBoxMetric)
		metrics.setFlag(metric, true);

	for (auto* spreadsheet : m_spreadsheets)
		spreadsheet->setMetrics(metrics);

	load();
}

void StatisticsSpreadsheetDock::selectNone() {
	StatisticsSpreadsheet::Metrics metrics;
	for (const auto& metric : m_mappingComboBoxMetric)
		metrics.setFlag(metric, false);

	for (auto* spreadsheet : m_spreadsheets)
		spreadsheet->setMetrics(metrics);

	load();
}

//*************************************************************
//******************** SETTINGS *******************************
//*************************************************************
void StatisticsSpreadsheetDock::load() {
	const auto metrics = m_spreadsheet->metrics();
	ui.cbCount->setChecked(metrics.testFlag(StatisticsSpreadsheet::Metric::Count));
	ui.cbMinimum->setChecked(metrics.testFlag(StatisticsSpreadsheet::Metric::Minimum));
	ui.cbMaximum->setChecked(metrics.testFlag(StatisticsSpreadsheet::Metric::Maximum));
	ui.cbArithmeticMean->setChecked(metrics.testFlag(StatisticsSpreadsheet::Metric::ArithmeticMean));
	ui.cbGeometricMean->setChecked(metrics.testFlag(StatisticsSpreadsheet::Metric::GeometricMean));
	ui.cbHarmonicMean->setChecked(metrics.testFlag(StatisticsSpreadsheet::Metric::HarmonicMean));
	ui.cbContraharmonicMean->setChecked(metrics.testFlag(StatisticsSpreadsheet::Metric::ContraharmonicMean));
	ui.cbMode->setChecked(metrics.testFlag(StatisticsSpreadsheet::Metric::Mode));
	ui.cbFirstQuartile->setChecked(metrics.testFlag(StatisticsSpreadsheet::Metric::FirstQuartile));
	ui.cbMedian->setChecked(metrics.testFlag(StatisticsSpreadsheet::Metric::Median));
	ui.cbThirdQuartile->setChecked(metrics.testFlag(StatisticsSpreadsheet::Metric::ThirdQuartile));
	ui.cbTrimean->setChecked(metrics.testFlag(StatisticsSpreadsheet::Metric::Trimean));
	ui.cbRange->setChecked(metrics.testFlag(StatisticsSpreadsheet::Metric::Range));
	ui.cbVariance->setChecked(metrics.testFlag(StatisticsSpreadsheet::Metric::Variance));
	ui.cbStandardDeviation->setChecked(metrics.testFlag(StatisticsSpreadsheet::Metric::StandardDeviation));
	ui.cbMeanDeviation->setChecked(metrics.testFlag(StatisticsSpreadsheet::Metric::MeanDeviation));
	ui.cbMeanDeviationAroundMedian->setChecked(metrics.testFlag(StatisticsSpreadsheet::Metric::MeanDeviationAroundMedian));
	ui.cbMedianDeviation->setChecked(metrics.testFlag(StatisticsSpreadsheet::Metric::MedianDeviation));
	ui.cbIQR->setChecked(metrics.testFlag(StatisticsSpreadsheet::Metric::IQR));
	ui.cbSkewness->setChecked(metrics.testFlag(StatisticsSpreadsheet::Metric::Skewness));
	ui.cbKurtosis->setChecked(metrics.testFlag(StatisticsSpreadsheet::Metric::Kurtosis));
	ui.cbEntropy->setChecked(metrics.testFlag(StatisticsSpreadsheet::Metric::Entropy));
}

void StatisticsSpreadsheetDock::loadConfigFromTemplate(KConfig& config) {
	KConfigGroup group = config.group(QLatin1String("StatisticsSpreadsheet"));

	auto metrics = static_cast<StatisticsSpreadsheet::Metrics>(group.readEntry(QStringLiteral("Metrics"), static_cast<int>(m_spreadsheet->metrics())));
	m_spreadsheet->setMetrics(metrics);
	load();
}

/*!
	saves spreadsheet properties to \c config.
 */
void StatisticsSpreadsheetDock::saveConfigAsTemplate(KConfig& config) {
	KConfigGroup group = config.group(QLatin1String("StatisticsSpreadsheet"));
	group.writeEntry(QStringLiteral("Metrics"), static_cast<int>(m_spreadsheet->metrics()));
}
