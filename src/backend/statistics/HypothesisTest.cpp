/***************************************************************************
	File                 : HypothesisTest.cpp
	Project              : LabPlot
	Description          : Hypothesis Test – One Sample T-Test
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2019  Devanshu Agarwal
	SPDX-FileCopyrightText: 2023  Alexander Semke
	SPDX-FileCopyrightText: 2025  Kuntal Bar

 SPDX-License-Identifier: GPL-2.0-or-later
***************************************************************************/

#include "HypothesisTest.h"
#include "HtmlTableBuilder.h"
#include "backend/core/column/Column.h"
#include "backend/lib/commandtemplates.h"
#include <QLabel>
#include <QList>
#include <QString>
#include <QVector>
#include <cmath>
#include <gsl/gsl_cdf.h>

// Include the updated nsl_statistical_test functions.
extern "C" {
#include "backend/nsl/nsl_statistical_test.h"
}

HypothesisTest::HypothesisTest(const QString& name)
	: GeneralTest(name, AspectType::HypothesisTest)
	, m_populationMean(0.0)
	, m_significanceLevel(0.05)
	, m_tail(TailTwo) {
}

HypothesisTest::~HypothesisTest() {
}

void HypothesisTest::setPopulationMean(double mean) {
	m_populationMean = mean;
}

void HypothesisTest::setSignificanceLevel(double alpha) {
	m_significanceLevel = alpha;
}

void HypothesisTest::setTail(HypothesisTailType tail) {
	m_tail = tail;
}

void HypothesisTest::setNullHypothesis(NullHypothesisType type) {
	m_nullHypothesisType = type;
}

void HypothesisTest::runTest() {
	m_currentTestName = QLatin1String("<h2>") + i18n("One Sample Test") + QLatin1String("</h2>");
	performOneSampleTTest();
	Q_EMIT changed();
}

void HypothesisTest::performOneSampleTTest() {
	if (m_columns.size() != 1) {
		displayError(i18n("Inappropriate number of columns selected."));
		return;
	}
	if (!m_columns[0]->isNumeric()) {
		displayError(i18n("Only numeric columns can be used for the test."));
		return;
	}

	int n = 0;
	double sum = 0.0, mean = 0.0, stdDev = 0.0;
	GeneralTest::GeneralErrorType errorCode = computeColumnStats(m_columns[0], n, sum, mean, stdDev);
	switch (errorCode) {
	case ErrorEmptyColumn:
		displayError(i18n("The selected column is empty."));
		return;
	case ErrorUnqualSize:
		displayError(i18n("Column size mismatch."));
		return;
	case NoError:
		break;
	}

	HtmlTableBuilder tableBuilder;
	tableBuilder.setStyle(
		QStringLiteral("table { border-collapse: collapse; margin: auto; width: 90%; font-size: 16px; } "
					   "th, td { border: 1px solid #333; padding: 8px; text-align: center; } "
					   "th { background-color: #ddd; }"));
	tableBuilder.setTableAttributes(QStringLiteral("id='statsTable'"));
	HtmlRow headerRow;
	headerRow.addCell(HtmlCell(QStringLiteral(""), true))
		.addCell(HtmlCell(QStringLiteral("N"), true))
		.addCell(HtmlCell(QStringLiteral("Sum"), true))
		.addCell(HtmlCell(QStringLiteral("Mean"), true))
		.addCell(HtmlCell(QStringLiteral("Std Dev"), true));
	tableBuilder.addRow(headerRow);
	HtmlRow dataRow;
	dataRow.addCell(HtmlCell(m_columns[0]->name()))
		.addCell(HtmlCell(QString::number(n)))
		.addCell(HtmlCell(QString::number(sum)))
		.addCell(HtmlCell(formatRoundedValue(mean)))
		.addCell(HtmlCell(formatRoundedValue(stdDev)));
	tableBuilder.addRow(dataRow);
	m_statsTable = tableBuilder.build();

	if (stdDev == 0.0) {
		displayError(i18n("Standard deviation is zero."));
		return;
	}

	// Build an array of sample values from the column.
	QVector<double> sample;
	sample.reserve(n);
	for (int i = 0; i < n; ++i) {
		sample.append(m_columns[0]->valueAt(i));
	}

	// Map our tail enum to an integer parameter for the nsl_stats function:
	// TailTwo → 0, TailNegative → 1, TailPositive → 2.
	int tail_param = 0;
	QString nullHypothesisSign;
	QString alternateHypothesisSign;

	switch (m_tail) {
	case TailNegative:
		nullHypothesisSign = UTF8_QSTRING("≥");
		alternateHypothesisSign = UTF8_QSTRING("<");
		tail_param = 1;
		break;
	case TailPositive:
		nullHypothesisSign = UTF8_QSTRING("≤");
		alternateHypothesisSign = UTF8_QSTRING(">");
		tail_param = 2;
		break;
	case TailTwo:
		nullHypothesisSign = UTF8_QSTRING("=");
		alternateHypothesisSign = UTF8_QSTRING("≠");
		tail_param = 0;
		break;
	}

	double tValue = nsl_stats_one_sample_t(sample.constData(), static_cast<size_t>(n), m_populationMean);
	double pValue = nsl_stats_one_sample_t_p(sample.constData(), static_cast<size_t>(n), m_populationMean, tail_param);

	displayLine(0, i18n("<b>Null Hypothesis:</b> µ %1 µ₀", nullHypothesisSign), QStringLiteral("black"));
	displayLine(1, i18n("<b>Alternate Hypothesis:</b> µ %1 µ₀", alternateHypothesisSign), QStringLiteral("black"));

	int df = n - 1;
	m_statisticValue.append(tValue);
	displayLine(6, i18n("<b>Degrees of Freedom:</b> %1", df), QStringLiteral("black"));

	m_currentTestName = QStringLiteral("<h2>") + i18n("One Sample T-Test for %1", m_columns[0]->name()) + QStringLiteral("</h2>");
	displayLine(2, i18n("<b>Significance Level:</b> %1", m_significanceLevel), QStringLiteral("black"));
	displayLine(4, i18n("<b>T-Value:</b> %1", formatRoundedValue(tValue)), QStringLiteral("black"));
	displayLine(5, i18n("<b>P-Value:</b> %1", formatRoundedValue(pValue, 4)), QStringLiteral("black"));

	QString conclusion;
	if (pValue < m_significanceLevel) {
		conclusion = QStringLiteral("<span style='color:black;'><b>Conclusion:</b> Reject the Null Hypothesis.</span>");
	} else {
		conclusion = QStringLiteral("<span style='color:red;'><b>Conclusion:</b> Fail to Reject the Null Hypothesis.</span>");
	}
	displayLine(7, conclusion, QStringLiteral("black"));
}
void HypothesisTest::logError(const QString& errorMsg) {
	qCritical() << errorMsg;
}

QString HypothesisTest::generatePValueTooltip(const double& pValue) {
	if (pValue <= m_significanceLevel)
		return i18n("We can safely reject Null Hypothesis for significance level %1", formatRoundedValue(m_significanceLevel));

	return i18n("There is a plausibility for Null Hypothesis to be true");
}
