/***************************************************************************
	File                 : HypothesisTest.cpp
	Project              : LabPlot
	Description          : Hypothesis Test
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2025 Alexander Semke >alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
***************************************************************************/

#include "HypothesisTest.h"
#include "backend/core/column/Column.h"

#include <QLocale>
#include <KLocalizedString>

/*!
 * \brief Thsi class implements various statistical hypothesis tests.
 */
HypothesisTest::HypothesisTest(const QString& name)
	: GeneralTest(name, AspectType::HypothesisTest) {
}

HypothesisTest::~HypothesisTest() {
}

void HypothesisTest::setPopulationMean(double mean) {
	m_populationMean = mean;
}

void HypothesisTest::setSignificanceLevel(double alpha) {
	m_significanceLevel = alpha;
}

void HypothesisTest::setTail(nsl_stats_tail_type type) {
	m_tail = type;
}

void HypothesisTest::setNullHypothesis(NullHypothesisType type) {
	m_nullHypothesisType = type;
}

void HypothesisTest::runTest() {
	switch (m_test) {
	case Test::t_test_one_sample:
		performOneSampleTTest();
		break;
	}

	Q_EMIT changed();
}

void HypothesisTest::performOneSampleTTest() {
	// if (m_columns.size() != 1) {
	// 	displayError(i18n("Inappropriate number of columns selected."));
	// 	return;
	// }
 //
	const auto* col = m_columns.constFirst();
	// if (!col->isNumeric()) {
	// 	displayError(i18n("Only numeric columns can be used for the test."));
	// 	return;
	// }

	// copy valid values only
	const auto& statistics = col->statistics();
	const int n = statistics.size;
	QVector<double> sample;
	sample.reserve(n);
	for (int row = 0; row < col->rowCount(); ++row) {
		if (!col->isValid(row) || col->isMasked(row))
			continue;
		sample.append(col->valueAt(row));
	}

	// TODO: handle empty columns with n=0

	// perform the test
	double tValue = nsl_stats_one_sample_t(sample.constData(), static_cast<size_t>(n), m_populationMean);
	double pValue = nsl_stats_one_sample_t_p(sample.constData(), static_cast<size_t>(n), m_populationMean, m_tail);

	// show the results
	m_result.clear();
	bool outputTitle = true;
	bool outputTestSetup = true;
	bool outputDeccriptiveStatistics = true;
	bool outputConclusion = true;

	// title
	if (outputTitle) {
		addResultTitle(i18n("One-Sample t-Test"));
	}

	// test setup
	if (outputTestSetup) {
		addResultLine(i18n("Sample"), col->name());
		addResultLine(i18n("Test Mean"), QStringLiteral("µ₀ = ") + QLocale().toString(m_populationMean));

		QString nullHypothesisSign;
		QString alternateHypothesisSign;
		switch (m_tail) {
		case nsl_stats_tail_type_two:
			nullHypothesisSign = QString::fromUtf8("=");
			alternateHypothesisSign = QString::fromUtf8("≠");
			break;
		case nsl_stats_tail_type_negative:
			nullHypothesisSign = QString::fromUtf8("≥");
			alternateHypothesisSign = QString::fromUtf8("<");
			break;
		case nsl_stats_tail_type_positive:
			nullHypothesisSign = QString::fromUtf8("≤");
			alternateHypothesisSign = QString::fromUtf8(">");
			break;
		}

		addResultLine(i18n("Null Hypothesis"), QStringLiteral("µ ") + nullHypothesisSign + QStringLiteral(" µ₀"));
		addResultLine(i18n("Alternate Hypothesis"), QStringLiteral("µ ") + alternateHypothesisSign + QStringLiteral(" µ₀"));
	}

	// descriptive statistics
	if (outputDeccriptiveStatistics) {
		addResultSection(i18n("Descriptive Statistics"));
		addResultLine(i18n("Size"), statistics.size);
		addResultLine(i18n("Mean"),  statistics.arithmeticMean);
		addResultLine(i18n("Standard Deviation"), statistics.standardDeviation);
	}

	// test statistics
	addResultSection(i18n("t-Test Statistics"));
	addResultLine(i18n("Significance Level"), m_significanceLevel);
	addResultLine(i18n("t-Value"), tValue);
	addResultLine(i18n("p-Value"), pValue);
	addResultLine(i18n("Degrees of Freedom"), n - 1);

	// conclusion
	if (outputConclusion) {
		m_result += QStringLiteral("<h2>") + i18n("Statistical Conclusion") + QStringLiteral(":</h2>");
		if (pValue < m_significanceLevel)
			addResultLine(i18n("Reject the Null Hypothesis"));
		else
			addResultLine(i18n("Fail to reject the Null Hypothesis"));
	}

	Q_EMIT(changed());
}
