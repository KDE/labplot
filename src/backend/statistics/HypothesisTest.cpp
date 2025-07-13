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

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <QLocale>
#include <cmath>
#include <limits>

/*!
 * \brief This class implements various statistical hypothesis tests.
 */
HypothesisTest::HypothesisTest(const QString& name)
	: GeneralTest(name, AspectType::HypothesisTest) {
	setTest(m_test);
}

HypothesisTest::~HypothesisTest() {
}

void HypothesisTest::setTestMean(double mean) {
	m_testMean = mean;
}

double HypothesisTest::testMean() const {
	return m_testMean;
}

void HypothesisTest::setSignificanceLevel(double alpha) {
	if (alpha <= 0)
		return;
	m_significanceLevel = alpha;
}

double HypothesisTest::significanceLevel() const {
	return m_significanceLevel;
}

void HypothesisTest::setTail(nsl_stats_tail_type type) {
	m_tail = type;
}

nsl_stats_tail_type HypothesisTest::tail() const {
	return m_tail;
}

void HypothesisTest::setColumns(const QVector<Column*>& cols) {
	auto [min, max] = variableCount(m_test);
	if (cols.size() < min || cols.size() > max)
		return;

	m_columns = cols;
}

void HypothesisTest::recalculate() {
	if (m_columns.isEmpty()) {
		Q_EMIT info(i18n("No variable column provided."));
		return;
	}

	m_result.clear(); // clear the previous result

	switch (m_test) {
	case Test::t_test_one_sample:
		performOneSampleTTest();
		break;
	case Test::t_test_two_sample:
		performTwoSampleTTest(false /* paired */);
		break;
	case Test::t_test_two_sample_paired:
		performTwoSampleTTest(true /* paired */);
		break;
	case Test::one_way_anova:
		performOneWayANOVATest();
		break;
	case Test::mann_whitney_u_test:
		performMannWhitneyUTest();
		break;
	case Test::kruskal_wallis_test:
		performKruskalWallisTest();
		break;
	case Test::log_rank_test:
		performLogRankTest();
		break;
	}

	if (m_result.isEmpty())
		resetResult();

	Q_EMIT changed(); // notify the view about the changes
}

void HypothesisTest::resetResult() {
	m_result.clear(); // clear the previous result

	if (m_test == HypothesisTest::Test::t_test_one_sample) {
		addResultTitle(i18n("One-Sample t-Test"));

		// test setup
		addResultLine(i18n("Sample"), QStringLiteral("-"));
		addResultLine(i18n("Test Mean"), QStringLiteral("-"));

		addResultLine(i18n("Null Hypothesis"), QStringLiteral("-"));
		addResultLine(i18n("Alternate Hypothesis"), QStringLiteral("-"));

		// descriptive statistics
		addResultSection(i18n("Descriptive Statistics"));
		addResultLine(i18n("Size"), QStringLiteral("-"));
		addResultLine(i18n("Mean"), QStringLiteral("-"));
		addResultLine(i18n("Standard Deviation"), QStringLiteral("-"));

		// test statistics
		addResultSection(i18n("t-Test Statistics"));
		addResultLine(i18n("Significance Level"), QStringLiteral("-"));
		addResultLine(i18n("t-Value"), QStringLiteral("-"));
		addResultLine(i18n("p-Value"), QStringLiteral("-"));
		addResultLine(i18n("Degrees of Freedom"), QStringLiteral("-"));

		// conclusion
		addResultSection(i18n("Statistical Conclusion"));
		addResultLine(i18n("Test result not available"));
	} else if (m_test == HypothesisTest::Test::t_test_two_sample || m_test == HypothesisTest::Test::t_test_two_sample_paired) {
		if (m_test == HypothesisTest::Test::t_test_two_sample_paired) {
			addResultTitle(i18n("Paired Two-Sample t-Test"));
		} else if (m_test == HypothesisTest::Test::t_test_two_sample) {
			addResultTitle(i18n("Independent Two-Sample t-Test"));
		}

		addResultLine(i18n("Sample 1"), QStringLiteral("-"));
		addResultLine(i18n("Sample 2"), QStringLiteral("-"));

		addResultLine(i18n("Null Hypothesis"), QStringLiteral("-"));
		addResultLine(i18n("Alternate Hypothesis"), QStringLiteral("-"));

		addResultSection(i18n("Descriptive Statistics"));
		addResultLine(i18n("Size 1"), QStringLiteral("-"));
		addResultLine(i18n("Size 2"), QStringLiteral("-"));
		addResultLine(i18n("Mean 1"), QStringLiteral("-"));
		addResultLine(i18n("Mean 2"), QStringLiteral("-"));
		addResultLine(i18n("Standard Deviation 1"), QStringLiteral("-"));
		addResultLine(i18n("Standard Deviation 2"), QStringLiteral("-"));

		addResultSection(i18n("t-Test Statistics"));
		addResultLine(i18n("Significance Level"), QStringLiteral("-"));
		addResultLine(i18n("t-Value"), QStringLiteral("-"));
		addResultLine(i18n("p-Value"), QStringLiteral("-"));
		addResultLine(i18n("Degrees of Freedom"), QStringLiteral("-"));

		// conclusion
		addResultSection(i18n("Statistical Conclusion"));
		addResultLine(i18n("Test result not available"));
	}

	Q_EMIT changed(); // notify the view about the changes
}

void HypothesisTest::performOneSampleTTest() {
	double pValue = NAN;
	double tValue = NAN;
	const auto* col = m_columns.constFirst();
	const auto& statistics = col->statistics();
	const int n = statistics.size;
	if (n != 0) {
		// copy valid values only
		QVector<double> sample;
		sample.reserve(n);
		for (int row = 0; row < col->rowCount(); ++row) {
			auto val = col->valueAt(row);
			if (!std::isfinite(val) || col->isMasked(row))
				continue;
			sample.append(val);
		}

		Q_ASSERT(sample.size() == n);

		tValue = nsl_stats_one_sample_t(sample.constData(), static_cast<size_t>(n), m_testMean);
		pValue = nsl_stats_one_sample_t_p(sample.constData(), static_cast<size_t>(n), m_testMean, m_tail);
	}

	// title
	addResultTitle(i18n("One-Sample t-Test"));

	// test setup
	addResultLine(i18n("Sample"), col->name());
	addResultLine(i18n("Test Mean"), QStringLiteral("µ₀ = ") + QLocale().toString(m_testMean));

	const auto [nullHypothesisText, alternateHypothesisText] = HypothesisTest::hypothesisText(m_test, m_tail);

	addResultLine(i18n("Null Hypothesis"), nullHypothesisText);
	addResultLine(i18n("Alternate Hypothesis"), alternateHypothesisText);

	// descriptive statistics
	addResultSection(i18n("Descriptive Statistics"));
	addResultLine(i18n("Size"), statistics.size);
	addResultLine(i18n("Mean"), statistics.arithmeticMean);
	addResultLine(i18n("Standard Deviation"), statistics.standardDeviation);

	// test statistics
	addResultSection(i18n("t-Test Statistics"));
	addResultLine(i18n("Significance Level"), m_significanceLevel);
	addResultLine(i18n("t-Value"), tValue);
	addResultLine(i18n("p-Value"), pValue);
	addResultLine(i18n("Degrees of Freedom"), n - 1);

	// conclusion
	addResultSection(i18n("Statistical Conclusion"));
	if (!std::isnan(pValue)) {
		if (pValue <= m_significanceLevel)
			addResultLine(i18n("At the significance level %1, the population mean is significantly different from %2. Reject the null Hypothesis",
							   m_significanceLevel,
							   m_testMean));
		else
			addResultLine(i18n("At the significance level %1, the population mean is not significantly different from %2. Fail to reject the null Hypothesis",
							   m_significanceLevel,
							   m_testMean));
	} else
		addResultLine(i18n("Test result not available"));
}

void HypothesisTest::performTwoSampleTTest(bool paired) {
	if (m_columns.size() < 2) // we need two columns for the two sample t test
		return;

	const Column* col1 = m_columns.at(0);
	const Column* col2 = m_columns.at(1);

	if (!col1->isNumeric() || !col2->isNumeric())
		return;

	const int n1 = col1->statistics().size;
	const int n2 = col2->statistics().size;

	double pValue = NAN;
	double tValue = NAN;

	if (n1 != 0 && n2 != 0) {
		QVector<double> sample1;
		QVector<double> sample2;

		sample1.reserve(n1);
		sample2.reserve(n2);

		for (int row = 0; row < qMax(col1->rowCount(), col2->rowCount()); ++row) {
			if (row < col1->rowCount()) {
				auto val = col1->valueAt(row);
				if (std::isfinite(val) && !col1->isMasked(row))
					sample1.append(val);
			}
			if (row < col2->rowCount()) {
				auto val = col2->valueAt(row);
				if (std::isfinite(val) && !col2->isMasked(row))
					sample2.append(val);
			}
		}

		Q_ASSERT(sample1.size() == n1);
		Q_ASSERT(sample2.size() == n2);

		if (paired) {
			if (n1 != n2) {
				Q_EMIT info(i18n("Paired t-test requires equal sample sizes."));
				return;
			}

			QVector<double> differences;
			differences.reserve(n1);
			for (int i = 0; i < n1; ++i)
				differences.append(sample1[i] - sample2[i]);

			Q_ASSERT(differences.size() == n1);

			tValue = nsl_stats_one_sample_t(differences.constData(), static_cast<size_t>(n1), 0.0);
			pValue = nsl_stats_one_sample_t_p(differences.constData(), static_cast<size_t>(n1), 0.0, m_tail);
		} else {
			tValue = nsl_stats_independent_t(sample1.constData(), static_cast<size_t>(n1), sample2.constData(), static_cast<size_t>(n2));
			pValue = nsl_stats_independent_t_p(sample1.constData(), static_cast<size_t>(n1), sample2.constData(), static_cast<size_t>(n2), m_tail);
		}
	}

	if (paired)
		addResultTitle(i18n("Paired Two-Sample t-Test"));
	else
		addResultTitle(i18n("Independent Two-Sample t-Test"));

	addResultLine(i18n("Sample 1"), col1->name());
	addResultLine(i18n("Sample 2"), col2->name());

	const auto [nullHypothesisText, alternateHypothesisText] = HypothesisTest::hypothesisText(m_test, m_tail);

	addResultLine(i18n("Null Hypothesis"), nullHypothesisText);
	addResultLine(i18n("Alternate Hypothesis"), alternateHypothesisText);

	addResultSection(i18n("Descriptive Statistics"));
	addResultLine(i18n("Size 1"), n1);
	addResultLine(i18n("Size 2"), n2);
	addResultLine(i18n("Mean 1"), col1->statistics().arithmeticMean);
	addResultLine(i18n("Mean 2"), col2->statistics().arithmeticMean);
	addResultLine(i18n("Standard Deviation 1"), col1->statistics().standardDeviation);
	addResultLine(i18n("Standard Deviation 2"), col2->statistics().standardDeviation);

	addResultSection(i18n("t-Test Statistics"));
	addResultLine(i18n("Significance Level"), m_significanceLevel);
	addResultLine(i18n("t-Value"), tValue);
	addResultLine(i18n("p-Value"), pValue);

	int degreesOfFreedom;
	if (paired)
		degreesOfFreedom = n1 - 1; // n-1 for paired
	else
		degreesOfFreedom = n1 + n2 - 2; // n1+n2-2 for independent

	addResultLine(i18n("Degrees of Freedom"), degreesOfFreedom);

	// conclusion
	addResultSection(i18n("Statistical Conclusion"));
	if (!std::isnan(pValue)) {
		if (pValue <= m_significanceLevel)
			addResultLine(
				i18n("At the significance level %1, the population means are significantly different. Reject the null Hypothesis", m_significanceLevel));
		else
			addResultLine(i18n("At the significance level %1, the population means are not significantly different. Fail to reject the null Hypothesis",
							   m_significanceLevel));
	} else
		addResultLine(i18n("Test result not available"));
}

void HypothesisTest::performOneWayANOVATest() {
}

void HypothesisTest::performMannWhitneyUTest() {
}

void HypothesisTest::performKruskalWallisTest() {
}

void HypothesisTest::performLogRankTest() {
}

QPair<int, int> HypothesisTest::variableCount(HypothesisTest::Test test) {
	switch (test) {
	case HypothesisTest::Test::t_test_one_sample:
		return {1, 1};
		break;
	case HypothesisTest::Test::t_test_two_sample:
	case HypothesisTest::Test::t_test_two_sample_paired:
		return {2, 2};
		break;
	}

	return {0, 0};
}

void HypothesisTest::setTest(HypothesisTest::Test test) {
	m_test = test;

	m_columns.clear();

	resetResult();
}

HypothesisTest::Test HypothesisTest::test() const {
	return m_test;
}

QPair<QString, QString> HypothesisTest::hypothesisSigns(nsl_stats_tail_type tail) {
	QString nullHypothesisSign;
	QString alternateHypothesisSign;

	switch (tail) {
	case nsl_stats_tail_type_two:
		nullHypothesisSign = QStringLiteral("=");
		alternateHypothesisSign = QStringLiteral("≠");
		break;
	case nsl_stats_tail_type_negative:
		nullHypothesisSign = QStringLiteral("≥");
		alternateHypothesisSign = QStringLiteral("&lt;");
		break;
	case nsl_stats_tail_type_positive:
		nullHypothesisSign = QStringLiteral("≤");
		alternateHypothesisSign = QStringLiteral(">");
		break;
	}

	return {nullHypothesisSign, alternateHypothesisSign};
}

QPair<QString, QString> HypothesisTest::hypothesisSymbols(HypothesisTest::Test test) {
	QString lHSymbol;
	QString rHSymbol;

	switch (test) {
	case HypothesisTest::Test::t_test_one_sample:
		lHSymbol = QStringLiteral("µ");
		rHSymbol = QStringLiteral("µ₀");
		break;
	case HypothesisTest::Test::t_test_two_sample:
		lHSymbol = QStringLiteral("µ<sub>1</sub>");
		rHSymbol = QStringLiteral("µ<sub>2</sub>");
		break;
	case HypothesisTest::Test::t_test_two_sample_paired:
		lHSymbol = QStringLiteral("µ<sub>D</sub>");
		rHSymbol = QStringLiteral("0");
		break;
	}

	return {lHSymbol, rHSymbol};
}

QPair<QString, QString> HypothesisTest::hypothesisText(HypothesisTest::Test test, nsl_stats_tail_type tail) {
	const auto [nullHypothesisSign, alternateHypothesisSign] = HypothesisTest::hypothesisSigns(tail);
	const auto [lhs, rhs] = HypothesisTest::hypothesisSymbols(test);

	return {lhs + QChar(QChar::Space) + nullHypothesisSign + QChar(QChar::Space) + rhs,
			lhs + QChar(QChar::Space) + alternateHypothesisSign + QChar(QChar::Space) + rhs};
}
