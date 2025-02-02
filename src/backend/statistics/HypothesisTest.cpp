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
#include "backend/core/column/Column.h"
#include "backend/lib/commandtemplates.h"
#include <QLabel>
#include <QList>
#include <QString>
#include <cmath>
#include <gsl/gsl_cdf.h>

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

void HypothesisTest::runTest() {
	m_currentTestName = QLatin1String("<h2>") + i18n("One Sample Test") + QLatin1String("</h2>");
	performOneSampleTTest();
	Q_EMIT changed();
}

void HypothesisTest::performOneSampleTTest() {
	// Ensure exactly one column is selected.
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

		   // Create an HTML table with basic statistics.
	QVariant statsData[] =
		{QString(), QLatin1String("N"), QLatin1String("Sum"), QLatin1String("Mean"), QLatin1String("Std Dev"), m_columns[0]->name(), n, sum, mean, stdDev};

	m_statsTable = buildHtmlTable(2, 5, statsData);

	if (stdDev == 0.0) {
		displayError(i18n("Standard deviation is zero."));
		return;
	}

		   // Compute T-Value and Degrees of Freedom.
	int df = n - 1;
	double tValue = (mean - m_populationMean) / (stdDev / sqrt(n));
	m_statisticValue.append(tValue);

	displayLine(6, i18n("<b>Degrees of Freedom:</b> %1", df), QLatin1String("black"));

		   // Calculate the P-Value.
	double pValue = calculatePValue(tValue, m_columns[0]->name(), i18n("%1", m_populationMean), df);
	m_pValue.append(pValue);

		   // Display test header, significance level, and results.
	m_currentTestName = QLatin1String("<h2>") + i18n("One Sample T-Test for %1", m_columns[0]->name()) + QLatin1String("</h2>");
	displayLine(2, i18n("<b>Significance Level:</b> %1", m_significanceLevel), QLatin1String("black"));
	displayLine(4, i18n("<b>T-Value:</b> %1", formatRoundedValue(tValue)), QLatin1String("black"));
	displayLine(5, i18n("<b>P-Value:</b> %1", formatRoundedValue(pValue, 4)), QLatin1String("black"));

		   // Display the conclusion.
	QString conclusion;
	if (pValue < m_significanceLevel) {
		conclusion = QLatin1String("<span style='color:black;'><b>Conclusion:</b> Reject the Null Hypothesis.</span>");
	} else {
		conclusion = QLatin1String("<span style='color:red;'><b>Conclusion:</b> Fail to Reject the Null Hypothesis.</span>");
	}
	displayLine(7, conclusion, QLatin1String("black"));
}

double HypothesisTest::calculatePValue(double& tValue, const QString& col1Name, const QString& col2Name, int df) {
	double pValue = 0.0;
	QString nullHypothesisSign;
	QString alternateHypothesisSign;

		   // Determine the p-value and the alternate hypothesis sign based on m_tail.
	switch (m_tail) {
	case TailNegative:
		pValue = gsl_cdf_tdist_P(tValue, df);
		alternateHypothesisSign = UTF8_QSTRING("<");
		break;
	case TailPositive:
		tValue *= -1; // Adjust tValue for one-sided test.
		pValue = gsl_cdf_tdist_P(tValue, df);
		alternateHypothesisSign = UTF8_QSTRING(">");
		break;
	case TailTwo:
		pValue = 2.0 * gsl_cdf_tdist_P(-fabs(tValue), df);
		alternateHypothesisSign = UTF8_QSTRING("≠");
		break;
	}

		   // Now, determine the null hypothesis sign based on the user-selected m_nullHypothesisType.
	switch (m_nullHypothesisType) {
	case NullEquality:
		nullHypothesisSign = UTF8_QSTRING("=");
		break;
	case NullLessEqual:
		nullHypothesisSign = UTF8_QSTRING("≤");
		break;
	case NullGreaterEqual:
		nullHypothesisSign = UTF8_QSTRING("≥");
		break;
	}

	// Display the hypotheses.
	displayLine(0, i18n("<b>Null Hypothesis:</b> µ %1 µ₀", nullHypothesisSign), QLatin1String("black"));
	displayLine(1, i18n("<b>Alternate Hypothesis:</b> µ %1 µ₀", alternateHypothesisSign), QLatin1String("black"));

	return (pValue > 1.0) ? 1.0 : pValue;
}

HypothesisTest::NullHypothesisType HypothesisTest::nullHypothesis() const {
	return m_nullHypothesisType;
}

void HypothesisTest::setNullHypothesis(NullHypothesisType type) {
	m_nullHypothesisType = type;
}

void HypothesisTest::logError(const QString& errorMsg) {
	qCritical() << errorMsg;
}

QString HypothesisTest::generatePValueTooltip(const double& pValue) {
	if (pValue <= m_significanceLevel)
		return i18n("We can safely reject Null Hypothesis for significance level %1", formatRoundedValue(m_significanceLevel));

	return i18n("There is a plausibility for Null Hypothesis to be true");
}
