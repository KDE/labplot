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
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/macros.h"
#include "frontend/statistics/HypothesisTestView.h"

#include <QMenu>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QPrinter>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <QLocale>

#include <QtConcurrent/QtConcurrent>
#include <cmath>
#include <cstddef>
#include <limits>

/*!
 * \brief This class implements various statistical hypothesis tests.
 */
HypothesisTest::HypothesisTest(const QString& name)
	: AbstractPart(name, AspectType::HypothesisTest) {
	setTest(m_test);
}

HypothesisTest::~HypothesisTest() = default;

QString HypothesisTest::resultHtml() const {
	return m_result;
}

void HypothesisTest::setColumns(const QVector<Column*>& cols) {
	auto [min, max] = variableCount(m_test);
	if (cols.size() < min || cols.size() > max)
		return;

	for (auto* col : cols)
		if (!col->isNumeric())
			return;

	m_columns = cols;
}

const QVector<Column*>& HypothesisTest::columns() const {
	return m_columns;
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

void HypothesisTest::setTest(Test test) {
	m_test = test;
	m_columns.clear();
	resetResult();
}

HypothesisTest::Test HypothesisTest::test() const {
	return m_test;
}

// ##############################################################################
// #######################  Virtual functions  ##################################
// ##############################################################################
bool HypothesisTest::exportView() const {
	// TODO: implement the export of the html to a file
	return true;
}

bool HypothesisTest::printView() {
#ifndef SDK
	QPrinter printer;
	auto* dlg = new QPrintDialog(&printer, m_view);
	dlg->setWindowTitle(i18nc("@title:window", "Statistical Test"));
	bool ret;
	if ((ret = (dlg->exec() == QDialog::Accepted)))
		m_view->print(&printer);

	delete dlg;
	return ret;
#else
	return false;
#endif
}

bool HypothesisTest::printPreview() const {
#ifndef SDK
	auto* dlg = new QPrintPreviewDialog(m_view);
	connect(dlg, &QPrintPreviewDialog::paintRequested, m_view, &HypothesisTestView::print);
	return dlg->exec();
#else
	return false;
#endif
}

QMenu* HypothesisTest::createContextMenu() {
	QMenu* menu = AbstractPart::createContextMenu();
	// Additional menu customization can be done here.
	return menu;
}

QWidget* HypothesisTest::view() const {
	if (!m_partView) {
		m_view = new HypothesisTestView(const_cast<HypothesisTest*>(this));
		m_partView = m_view;
	}
	return m_partView;
}

// ##############################################################################
// ##################  Result output related functions  #########################
// ##############################################################################
void HypothesisTest::addResultTitle(const QString& text) {
	m_result += QStringLiteral("<h1>") + text + QStringLiteral("</h1>");
}

void HypothesisTest::addResultSection(const QString& text) {
	m_result += QStringLiteral("<h2>") + text + QStringLiteral("</h2>");
}

void HypothesisTest::addResultLine(const QString& name, const QString& value) {
	m_result += QStringLiteral("<b>") + name + QStringLiteral("</b>: ") + value + QStringLiteral("<br>");
}

void HypothesisTest::addResultLine(const QString& name, double value) {
	if (!std::isnan(value))
		addResultLine(name, QLocale().toString(value));
	else
		addResultLine(name, QStringLiteral("-"));
}

void HypothesisTest::addResultLine(const QString& name) {
	m_result += name;
}

// ##############################################################################
// ##################  Result output related functions  #########################
// ##############################################################################
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

	if (m_test == Test::t_test_one_sample) {
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
	} else if (m_test == Test::t_test_two_sample || m_test == Test::t_test_two_sample_paired) {
		if (m_test == Test::t_test_two_sample_paired) {
			addResultTitle(i18n("Paired Two-Sample t-Test"));
		} else if (m_test == Test::t_test_two_sample) {
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
	} else if (m_test == Test::one_way_anova) {
		addResultTitle(i18n("One-Way ANOVA"));

		addResultLine(i18n("Sample x"), QStringLiteral("-"));

		addResultLine(i18n("Null Hypothesis"), QStringLiteral("-"));
		addResultLine(i18n("Alternate Hypothesis"), QStringLiteral("-"));

		addResultSection(i18n("Descriptive Statistics"));
		addResultLine(i18n("Group x Size"), QStringLiteral("-"));
		addResultLine(i18n("Group x Mean"), QStringLiteral("-"));
		addResultLine(i18n("Group x Standard Deviation"), QStringLiteral("-"));

		addResultSection(i18n("ANOVA Statistics"));
		addResultLine(i18n("Significance Level"), QStringLiteral("-"));
		addResultLine(i18n("F-Statistic"), QStringLiteral("-"));
		addResultLine(i18n("p-Value"), QStringLiteral("-"));
		addResultLine(i18n("Between-Groups Degrees of Freedom"), QStringLiteral("-"));
		addResultLine(i18n("Within-Groups Degrees of Freedom"), QStringLiteral("-"));
	} else if (m_test == Test::mann_whitney_u_test) {
	} else if (m_test == Test::kruskal_wallis_test) {
		addResultTitle(i18n("Kruskal-Wallis Test"));

		addResultLine(i18n("Sample x"), QStringLiteral("-"));

		addResultLine(i18n("Null Hypothesis"), QStringLiteral("-"));
		addResultLine(i18n("Alternate Hypothesis"), QStringLiteral("-"));

		addResultSection(i18n("Descriptive Statistics"));
		addResultLine(i18n("Group x Size"), QStringLiteral("-"));
		addResultLine(i18n("Group x Mean"), QStringLiteral("-"));
		addResultLine(i18n("Group x Standard Deviation"), QStringLiteral("-"));

		addResultSection(i18n("Kruskal-Wallis Statistics"));
		addResultLine(i18n("Significance Level"), QStringLiteral("-"));
		addResultLine(i18n("H-Statistic"), QStringLiteral("-"));
		addResultLine(i18n("p-Value"), QStringLiteral("-"));
		addResultLine(i18n("Degrees of Freedom"), QStringLiteral("-"));
	} else if (m_test == Test::log_rank_test) {
		addResultTitle(i18n("Log-Rank Test"));

		addResultLine(i18n("Null Hypothesis"), QStringLiteral("-"));
		addResultLine(i18n("Alternate Hypothesis"), QStringLiteral("-"));

		addResultSection(i18n("Descriptive Statistics"));
		addResultLine(i18n("Group 0 Size"), QStringLiteral("-"));
		addResultLine(i18n("Group 1 Size"), QStringLiteral("-"));

		addResultSection(i18n("Log-Rank Statistics"));
		addResultLine(i18n("Significance Level"), QStringLiteral("-"));
		addResultLine(i18n("Statistic"), QStringLiteral("-"));
		addResultLine(i18n("p-Value"), QStringLiteral("-"));
		addResultLine(i18n("Degrees of Freedom"), QStringLiteral("-"));
	}

	addResultSection(i18n("Statistical Conclusion"));
	addResultLine(i18n("Test result not available"));

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

		tValue = nsl_stats_one_sample_t(sample.constData(), static_cast<size_t>(n), m_testMean, m_tail, &pValue);
	}

	// title
	addResultTitle(i18n("One-Sample t-Test"));

	// test setup
	addResultLine(i18n("Sample"), col->name());
	addResultLine(i18n("Test Mean"), QStringLiteral("µ₀ = ") + QLocale().toString(m_testMean));

	const auto [nullHypothesisText, alternateHypothesisText] = HypothesisTest::hypothesisText(m_test).at(m_tail);

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

			tValue = nsl_stats_one_sample_t(differences.constData(), static_cast<size_t>(n1), 0.0, m_tail, &pValue);
		} else {
			tValue = nsl_stats_independent_t(sample1.constData(), static_cast<size_t>(n1), sample2.constData(), static_cast<size_t>(n2), m_tail, &pValue);
		}
	}

	addResultTitle(paired ? i18n("Paired Two-Sample t-Test") : i18n("Independent Two-Sample t-Test"));
	addResultLine(i18n("Sample 1"), col1->name());
	addResultLine(i18n("Sample 2"), col2->name());

	const auto [nullHypothesisText, alternateHypothesisText] = HypothesisTest::hypothesisText(m_test).at(m_tail);

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

	int degreesOfFreedom = (paired) ? n1 - 1 : n1 + n2 - 2;

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
	if (m_columns.size() < 2) {
		Q_EMIT info(i18n("At least two columns are required to perform One-way ANOVA test."));
		return;
	}

	const int groupCount = m_columns.size();

	QVector<QVector<double>> groupData(filterColumnsParallel(m_columns));

	Q_ASSERT(groupCount == groupData.size());

	QVector<size_t> groupSizes(groupCount);

	for (int n = 0; n < groupCount; n++)
		groupSizes[n] = static_cast<size_t>(groupData[n].size());

	double** groups = toArrayOfArrays(groupData);

	double p = NAN;
	double f = nsl_stats_anova_oneway_f(groups, groupSizes.data(), static_cast<size_t>(groupCount), &p);

	delete[] groups;

	addResultTitle(i18n("One-Way ANOVA"));

	const auto [nullHypothesisText, alternateHypothesisText] = HypothesisTest::hypothesisText(m_test).at(0);

	for (int g = 0; g < groupCount; ++g)
		addResultLine(i18n("Sample %1", g + 1), m_columns.at(g)->name());

	addResultLine(i18n("Null Hypothesis"), nullHypothesisText);
	addResultLine(i18n("Alternate Hypothesis"), alternateHypothesisText);

	addResultSection(i18n("Descriptive Statistics"));
	for (int g = 0; g < groupCount; ++g) {
		auto columnStatistics = m_columns.at(g)->statistics();
		addResultLine(i18n("Group %1 Size", g + 1), columnStatistics.size);
		addResultLine(i18n("Group %1 Mean", g + 1), columnStatistics.arithmeticMean);
		addResultLine(i18n("Group %1 Standard Deviation", g + 1), columnStatistics.standardDeviation);
	}
	addResultSection(i18n("ANOVA Statistics"));
	addResultLine(i18n("Significance Level"), m_significanceLevel);
	addResultLine(i18n("F-Statistic"), f);
	addResultLine(i18n("p-Value"), p);
	addResultLine(i18n("Between-Groups Degrees of Freedom"), groupCount - 1);
	addResultLine(i18n("Within-Groups Degrees of Freedom"), std::accumulate(groupSizes.begin(), groupSizes.end(), 0) - groupCount);

	addResultSection(i18n("Statistical Conclusion"));
	if (!std::isnan(p)) {
		if (p <= m_significanceLevel)
			addResultLine(
				i18n("At the significance level %1, at least one group mean is significantly different. Reject the null Hypothesis", m_significanceLevel));
		else
			addResultLine(
				i18n("At the significance level %1, no significant difference between group means. Fail to reject the null Hypothesis", m_significanceLevel));
	} else {
		addResultLine(i18n("Test result not available"));
	}
}

void HypothesisTest::performMannWhitneyUTest() {
}

void HypothesisTest::performKruskalWallisTest() {
	if (m_columns.size() < 2) {
		Q_EMIT info(i18n("At least two columns are required to perform Kruskal-Wallis test."));
		return;
	}

	const int groupCount = m_columns.size();

	QVector<QVector<double>> groupData(filterColumnsParallel(m_columns));

	Q_ASSERT(groupCount == groupData.size());

	QVector<size_t> groupSizes(groupCount);

	for (int n = 0; n < groupCount; n++)
		groupSizes[n] = static_cast<size_t>(groupData[n].size());

	double** groups = toArrayOfArrays(groupData);

	double p = NAN;
	double h = nsl_stats_kruskal_wallis_h(groups, groupSizes.data(), static_cast<size_t>(groupCount), &p);

	delete[] groups;

	addResultTitle(i18n("Kruskal-Wallis Test"));

	for (int g = 0; g < groupCount; ++g)
		addResultLine(i18n("Sample %1", g + 1), m_columns.at(g)->name());

	const auto [nullHypothesisText, alternateHypothesisText] = HypothesisTest::hypothesisText(m_test).at(0);

	addResultLine(i18n("Null Hypothesis"), nullHypothesisText);
	addResultLine(i18n("Alternate Hypothesis"), alternateHypothesisText);

	addResultSection(i18n("Descriptive Statistics"));
	for (int g = 0; g < groupCount; ++g) {
		auto columnStatistics = m_columns.at(g)->statistics();
		addResultLine(i18n("Group %1 Size", g + 1), columnStatistics.size);
		addResultLine(i18n("Group %1 Mean", g + 1), columnStatistics.arithmeticMean);
		addResultLine(i18n("Group %1 Standard Deviation", g + 1), columnStatistics.standardDeviation);
	}

	addResultSection(i18n("Kruskal-Wallis Statistics"));
	addResultLine(i18n("Significance Level"), m_significanceLevel);
	addResultLine(i18n("H-Statistic"), h);
	addResultLine(i18n("p-Value"), p);
	addResultLine(i18n("Degrees of Freedom"), groupCount - 1);

	addResultSection(i18n("Statistical Conclusion"));
	if (!std::isnan(p)) {
		if (p <= m_significanceLevel)
			addResultLine(i18n("At the significance level %1, at least one group distribution is significantly different. Reject the null Hypothesis",
							   m_significanceLevel));
		else
			addResultLine(i18n("At the significance level %1, no significant difference between group distributions. Fail to reject the null Hypothesis",
							   m_significanceLevel));
	} else {
		addResultLine(i18n("Test result not available"));
	}
}

void HypothesisTest::performLogRankTest() {
	// time, status and group columns required
	if (m_columns.size() < 3) {
		Q_EMIT info(i18n("Three columns are required to perform LogRank test."));
		return;
	}

	const Column* timeCol = m_columns.at(0);
	const Column* statusCol = m_columns.at(1);
	const Column* groupCol = m_columns.at(2);

	if (statusCol->rowCount() != timeCol->rowCount() || groupCol->rowCount() != timeCol->rowCount()) {
		Q_EMIT info(i18n("Time, Status and Group columns should have equal size."));
		return;
	}

	const int n = timeCol->rowCount();

	QVector<double> time;
	QVector<int> status;
	QVector<size_t> group0Indices, group1Indices;

	for (int row = 0; row < n; ++row) {
		if (!rowIsValid(timeCol, row) || !rowIsValid(statusCol, row) || !rowIsValid(groupCol, row))
			continue;

		double t = timeCol->valueAt(row);
		int s = static_cast<int>(statusCol->valueAt(row)); // must be 0 or 1
		int g = static_cast<int>(groupCol->valueAt(row)); // must be 0 or 1

		if (s != 0 && s != 1) {
			Q_EMIT info(i18n("Status column values must be either 0 or 1."));
			return;
		}

		if (g != 0 && g != 1) {
			Q_EMIT info(i18n("Group column values must be either 0 or 1."));
			return;
		}

		time.append(t);
		status.append(s);

		size_t index = time.size() - 1;

		if (g == 0)
			group0Indices.append(index);
		else if (g == 1)
			group1Indices.append(index);
	}

	Q_ASSERT((time.size() == status.size()) && group0Indices.size() + group1Indices.size() == time.size());

	double p = NAN;
	double stat = nsl_stats_log_rank_test_statistic(time.constData(),
													status.constData(),
													group0Indices.constData(),
													group0Indices.size(),
													group1Indices.constData(),
													group1Indices.size(),
													&p);

	addResultTitle(i18n("Log-Rank Test"));

	const auto [nullHypothesisText, alternateHypothesisText] = HypothesisTest::hypothesisText(m_test).at(0);

	addResultLine(i18n("Null Hypothesis"), nullHypothesisText);
	addResultLine(i18n("Alternate Hypothesis"), alternateHypothesisText);

	addResultSection(i18n("Descriptive Statistics"));
	addResultLine(i18n("Group 0 Size"), group0Indices.size());
	addResultLine(i18n("Group 1 Size"), group1Indices.size());

	addResultSection(i18n("Log-Rank Statistics"));
	addResultLine(i18n("Significance Level"), m_significanceLevel);
	addResultLine(i18n("Statistic"), stat);
	addResultLine(i18n("p-Value"), p);
	addResultLine(i18n("Degrees of Freedom"), 1); // no of groups - 1 (we support only two groups)

	addResultSection(i18n("Statistical Conclusion"));
	if (!std::isnan(p)) {
		if (p <= m_significanceLevel)
			addResultLine(
				i18n("At the significance level %1, the survival curves are significantly different. Reject the null Hypothesis", m_significanceLevel));
		else
			addResultLine(i18n("At the significance level %1, no significant difference between survival curves. Fail to reject the null Hypothesis",
							   m_significanceLevel));
	} else {
		addResultLine(i18n("Test result not available"));
	}
}

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################

//! Save as XML
void HypothesisTest::save(QXmlStreamWriter* writer) const {
	writer->writeStartElement(QLatin1String("HypothesisTest"));
	writeBasicAttributes(writer);
	writeCommentElement(writer);
	writer->writeEndElement();
}

//! Load from XML
bool HypothesisTest::load(XmlStreamReader* reader, bool preview) {
	Q_UNUSED(preview)
	if (!readBasicAttributes(reader))
		return false;
	return !reader->hasError();
}

QPair<int, int> HypothesisTest::variableCount(Test test) {
	switch (test) {
	case Test::t_test_one_sample:
		return {1, 1};
		break;
	case Test::t_test_two_sample:
	case Test::t_test_two_sample_paired:
	case Test::mann_whitney_u_test:
		return {2, 2};
		break;
	case Test::one_way_anova:
	case Test::kruskal_wallis_test:
		return {2, std::numeric_limits<int>::max()};
		break;
	case Test::log_rank_test:
		return {3, 3};
		break;
	}
	return {0, 0};
}

QVector<QPair<QString, QString>> HypothesisTest::hypothesisText(Test test) {
	int hypCount = hypothesisCount(test);

	QVector<QPair<QString, QString>> hypothesis(hypCount);

	if (hypCount == 3) {
		QString lHSymbol;
		QString rHSymbol;
		if (test == Test::t_test_one_sample) {
			lHSymbol = QStringLiteral("µ");
			rHSymbol = QStringLiteral("µ₀");
		} else if (test == Test::t_test_two_sample) {
			lHSymbol = QStringLiteral("µ<sub>1</sub>");
			rHSymbol = QStringLiteral("µ<sub>2</sub>");
		} else if (test == Test::t_test_two_sample_paired) {
			lHSymbol = QStringLiteral("µ<sub>D</sub>");
			rHSymbol = QStringLiteral("0");
		} else if (test == Test::mann_whitney_u_test) {
			lHSymbol = QStringLiteral("µ<sub>r1</sub>");
			rHSymbol = QStringLiteral("µ<sub>r2</sub>");
		}

		hypothesis[nsl_stats_tail_type_two] = QPair<QString, QString>(lHSymbol + QStringLiteral(" = ") + rHSymbol, lHSymbol + QStringLiteral(" ≠ ") + rHSymbol);
		hypothesis[nsl_stats_tail_type_negative] =
			QPair<QString, QString>(lHSymbol + QStringLiteral(" ≥ ") + rHSymbol, lHSymbol + QStringLiteral(" &lt; ") + rHSymbol);
		hypothesis[nsl_stats_tail_type_positive] =
			QPair<QString, QString>(lHSymbol + QStringLiteral(" ≤ ") + rHSymbol, lHSymbol + QStringLiteral(" > ") + rHSymbol);
	} else if (hypCount == 1) {
		if (test == Test::one_way_anova) {
			hypothesis[nsl_stats_tail_type_two] = QPair<QString, QString>(QStringLiteral("µ<sub>1</sub> = µ<sub>2</sub> = ... = µ<sub>k</sub>"),
																		  QStringLiteral("At least one µ<sub>i</sub> is different"));
		} else if (test == Test::kruskal_wallis_test) {
			hypothesis[nsl_stats_tail_type_two] = QPair<QString, QString>(QStringLiteral("µ<sub>r1</sub> = µ<sub>r2</sub> = ... = µ<sub>rk</sub>"),
																		  QStringLiteral("At least one µ<sub>ri</sub> is different"));
		} else if (test == Test::log_rank_test) {
			hypothesis[nsl_stats_tail_type_two] =
				QPair<QString, QString>(QStringLiteral("S<sub>0</sub> = S<sub>1</sub>"), QStringLiteral("S<sub>0</sub> ≠ S<sub>1</sub>"));
		}
	}

	return hypothesis;
}

int HypothesisTest::hypothesisCount(Test test) {
	switch (test) {
	case Test::t_test_one_sample:
	case Test::t_test_two_sample:
	case Test::t_test_two_sample_paired:
	case Test::mann_whitney_u_test:
		return 3;
	case Test::one_way_anova:
	case Test::kruskal_wallis_test:
	case Test::log_rank_test:
		return 1;
	}

	return 0;
}

bool HypothesisTest::rowIsValid(const Column* column, int row) {
	return std::isfinite(column->valueAt(row)) && !column->isMasked(row);
}

QVector<double> HypothesisTest::filterColumn(const Column* col) {
	QVector<double> sample;

	for (int row = 0; row < col->rowCount(); ++row)
		if (rowIsValid(col, row))
			sample.append(col->valueAt(row));

	return sample;
}

QVector<QVector<double>> HypothesisTest::filterColumns(const QVector<Column*>& columns) {
	QVector<QVector<double>> samples;
	samples.reserve(columns.size());

	for (const Column* col : columns)
		samples.append(std::move(filterColumn(col)));

	return samples;
}

QVector<QVector<double>> HypothesisTest::filterColumnsParallel(const QVector<Column*>& columns) {
	return QtConcurrent::mapped(columns, filterColumn).results();
}

double** HypothesisTest::toArrayOfArrays(QVector<QVector<double>>& data) {
	double** result = new double*[data.size()];
	for (int i = 0; i < data.size(); ++i)
		result[i] = data[i].data();
	return result;
}
