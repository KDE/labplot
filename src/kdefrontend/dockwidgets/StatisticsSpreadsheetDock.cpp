/*
	File                 : StatisticsSpreadsheetDock.cpp
	Project              : LabPlot
	Description          : widget for statistics spreadsheet properties
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "StatisticsSpreadsheetDock.h"

#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/spreadsheet/StatisticsSpreadsheet.h"
#include "kdefrontend/TemplateHandler.h"

/*!
 \class StatisticsSpreadsheetDock
 \brief Provides a widget for editing the properties of the spreadsheets currently selected in the project explorer.

 \ingroup kdefrontend
*/

StatisticsSpreadsheetDock::StatisticsSpreadsheetDock(QWidget* parent)
	: BaseDock(parent) {
	ui.setupUi(this);

	auto* templateHandler = new TemplateHandler(this, TemplateHandler::ClassName::Spreadsheet);
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
	m_spreadsheetList = list;
	m_spreadsheet = list.first();
	load();
}

//*************************************************************
//* SLOTs for changes triggered in StatisticsSpreadsheetDock **
//*************************************************************

//*************************************************************
//******** SLOTs for changes triggered in Spreadsheet *********
//*************************************************************

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

}

/*!
	loads saved spreadsheet properties from \c config.
 */
void StatisticsSpreadsheetDock::loadConfig(KConfig& config) {

}

/*!
	saves spreadsheet properties to \c config.
 */
void StatisticsSpreadsheetDock::saveConfigAsTemplate(KConfig& config) {

}
