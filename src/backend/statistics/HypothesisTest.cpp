/***************************************************************************
	File                 : HypothesisTest.cpp
	Project              : LabPlot
	Description          : Hypothesis Test
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2025 Alexander Semke >alexander.semke@web.de>
	SPDX-FileCopyrightText: 2025 Israel Galadima <izzygaladima@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
***************************************************************************/

#include "HypothesisTest.h"
#include "HypothesisTestPrivate.h"
#include "backend/core/Settings.h"
#include "backend/core/column/Column.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/macros.h"
#include "backend/lib/trace.h"
#include "frontend/statistics/HypothesisTestView.h"

#include <QAction>
#include <QActionGroup>
#include <QFileDialog>
#include <QLocale>
#include <QMenu>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QPrinter>
#include <QtConcurrent/QtConcurrent>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>

/*!
 * \brief This class implements various statistical hypothesis tests.
 */
HypothesisTest::HypothesisTest(const QString& name)
	: AbstractPart(name, AspectType::HypothesisTest)
	, d_ptr(new HypothesisTestPrivate(this)) {
}

HypothesisTest::~HypothesisTest() = default;

QString HypothesisTest::resultHtml() const {
	Q_D(const HypothesisTest);
	return d->result;
}

void HypothesisTest::setDataColumns(const QVector<const AbstractColumn*>& cols) {
	Q_D(HypothesisTest);
	auto [min, max] = variableCount(d->test);
	if (cols.size() < min || cols.size() > max) {
		if (min == max)
			Q_EMIT info(i18np("One column is required.", "%1 columns are required.", min));
		else
			Q_EMIT info(i18n("At least %1 and at most %2 columns are required.", min, max));
		return;
	}

	QVector<QString> columnPaths;
	for (auto* col : cols) {
		// currently excluding bigint because bigint > double and then we have to start using long double everywhere to accommodate
		if (!col->isNumeric()) {
			Q_EMIT info(i18n("Column '%1' must be of numeric type.", col->name()));
			return;
		}
		columnPaths.append(col->path());
	}

	d->dataColumns = cols;
	d->dataColumnPaths = columnPaths;
}

const QVector<const AbstractColumn*>& HypothesisTest::dataColumns() const {
	Q_D(const HypothesisTest);
	return d->dataColumns;
}

const QVector<QString>& HypothesisTest::dataColumnPaths() const {
	Q_D(const HypothesisTest);
	return d->dataColumnPaths;
}

void HypothesisTest::setTestMean(double mean) {
	Q_D(HypothesisTest);
	d->testMean = mean;
}

double HypothesisTest::testMean() const {
	Q_D(const HypothesisTest);
	return d->testMean;
}

void HypothesisTest::setSignificanceLevel(double alpha) {
	if (alpha <= 0)
		return;
	Q_D(HypothesisTest);
	d->significanceLevel = alpha;
}

double HypothesisTest::significanceLevel() const {
	Q_D(const HypothesisTest);
	return d->significanceLevel;
}

void HypothesisTest::setTail(nsl_stats_tail_type type) {
	Q_D(HypothesisTest);
	d->tail = type;
}

nsl_stats_tail_type HypothesisTest::tail() const {
	Q_D(const HypothesisTest);
	return d->tail;
}

void HypothesisTest::setTest(Test test) {
	Q_D(HypothesisTest);
	d->test = test;
	d->dataColumns.clear();
	d->dataColumnPaths.clear();
	d->resetResult();
}

HypothesisTest::Test HypothesisTest::test() const {
	Q_D(const HypothesisTest);
	return d->test;
}

void HypothesisTest::recalculate() {
	PERFTRACE(name() + QLatin1String(Q_FUNC_INFO));
	Q_D(HypothesisTest);
	d->recalculate();
}

// ##############################################################################
// #######################  Virtual functions  ##################################
// ##############################################################################
bool HypothesisTest::exportView() const {
#ifndef SDK
	auto conf = Settings::group(QStringLiteral("HypothesisTest"));
	const QString dir = conf.readEntry("LastDir", "");

	QString path = QFileDialog::getSaveFileName(m_view, i18nc("@title:window", "Export to File"), dir, i18n("Portable Data Format (*.pdf *.PDF)"));
	if (path.isEmpty())
		return false;

	int pos = path.lastIndexOf(QLatin1String("/"));
	if (pos != -1) {
		const QString newDir = path.left(pos);
		if (newDir != dir && QDir(newDir).exists())
			conf.writeEntry("LastDir", newDir);
	}

	QPrinter printer;
	printer.setOutputFormat(QPrinter::PdfFormat);
	printer.setOutputFileName(path);
	printer.setCreator(QStringLiteral("LabPlot ") + QLatin1String(LVERSION));
	m_view->print(&printer);
	return true;
#else
	return false;
#endif
}

bool HypothesisTest::printView() {
#ifndef SDK
	QPrinter printer;
	auto* dlg = new QPrintDialog(&printer, m_view);
	dlg->setWindowTitle(i18nc("@title:window", "Hypothesis Test"));
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

QWidget* HypothesisTest::view() const {
	if (!m_partView) {
		m_view = new HypothesisTestView(const_cast<HypothesisTest*>(this));
		m_partView = m_view;
	}
	return m_partView;
}

QPair<int, int> HypothesisTest::variableCount(Test test) {
	switch (test) {
	case Test::t_test_one_sample:
		return {1, 1};
		break;
	case Test::t_test_two_sample:
	case Test::t_test_two_sample_paired:
	case Test::mann_whitney_u_test:
	case Test::t_test_welch:
	case Test::wilcoxon_test:
	case Test::chisq_goodness_of_fit:
		return {2, 2};
		break;
	case Test::one_way_anova:
	case Test::kruskal_wallis_test:
	case Test::one_way_anova_repeated:
	case Test::friedman_test:
	case Test::chisq_independence:
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
			lHSymbol = QStringLiteral("η<sub>1</sub>");
			rHSymbol = QStringLiteral("η<sub>2</sub>");
		} else if (test == Test::t_test_welch) {
			lHSymbol = QStringLiteral("µ<sub>1</sub>");
			rHSymbol = QStringLiteral("µ<sub>2</sub>");
		} else if (test == Test::wilcoxon_test) {
			lHSymbol = QStringLiteral("η<sub>D</sub>");
			rHSymbol = QStringLiteral("0");
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
			hypothesis[nsl_stats_tail_type_two] = QPair<QString, QString>(QStringLiteral("η<sub>1</sub> = η<sub>2</sub> = ... = η<sub>k</sub>"),
																		  QStringLiteral("At least one η<sub>i</sub> is different"));
		} else if (test == Test::log_rank_test) {
			hypothesis[nsl_stats_tail_type_two] =
				QPair<QString, QString>(QStringLiteral("S<sub>1</sub> = S<sub>2</sub>"), QStringLiteral("S<sub>1</sub> ≠ S<sub>2</sub>"));
		} else if (test == Test::one_way_anova_repeated) {
			hypothesis[nsl_stats_tail_type_two] = QPair<QString, QString>(QStringLiteral("µ<sub>1</sub> = µ<sub>2</sub> = ... = µ<sub>k</sub>"),
																		  QStringLiteral("At least one µ<sub>i</sub> is different"));
		} else if (test == Test::friedman_test) {
			hypothesis[nsl_stats_tail_type_two] = QPair<QString, QString>(QStringLiteral("η<sub>1</sub> = η<sub>2</sub> = ... = η<sub>k</sub>"),
																		  QStringLiteral("At least one η<sub>i</sub> is different"));
		} else if (test == Test::chisq_goodness_of_fit) {
			hypothesis[nsl_stats_tail_type_two] = QPair<QString, QString>(QStringLiteral("O = E"), QStringLiteral("O ≠ E"));
		} else if (test == Test::chisq_independence) {
			hypothesis[nsl_stats_tail_type_two] = QPair<QString, QString>(QStringLiteral("P(A ∩ B) = P(A) * P(B)"), QStringLiteral("P(A ∩ B) ≠ P(A) * P(B)"));
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
	case Test::t_test_welch:
	case Test::wilcoxon_test:
		return 3;
	case Test::one_way_anova:
	case Test::kruskal_wallis_test:
	case Test::log_rank_test:
	case Test::friedman_test:
	case Test::chisq_independence:
	case Test::chisq_goodness_of_fit:
	case Test::one_way_anova_repeated:
		return 1;
	}

	return 0;
}

QString HypothesisTest::testName(Test test) {
	switch (test) {
	case Test::t_test_one_sample:
		return i18n("One-Sample t-Test");
	case Test::t_test_two_sample:
		return i18n("Independent Two-Sample t-Test");
	case Test::t_test_two_sample_paired:
		return i18n("Paired Two-Sample t-Test");
	case Test::mann_whitney_u_test:
		return i18n("Mann-Whitney U Test");
	case Test::t_test_welch:
		return i18n("Welch t-Test");
	case Test::wilcoxon_test:
		return i18n("Wilcoxon Signed Rank Test");
	case Test::one_way_anova:
		return i18n("One-Way ANOVA Test");
	case Test::kruskal_wallis_test:
		return i18n("Kruskal-Wallis Test");
	case Test::log_rank_test:
		return i18n("Log-Rank Test");
	case Test::friedman_test:
		return i18n("Friedman Test");
	case Test::chisq_independence:
		return i18n("Chi-Square Independence Test");
	case Test::chisq_goodness_of_fit:
		return i18n("Chi-Square Goodness of Fit Test");
	case Test::one_way_anova_repeated:
		return i18n("One-Way ANOVA with Repeated Measures Test");
	}

	return {};
}

size_t HypothesisTestPrivate::minSampleCount(HypothesisTest::Test test) {
	switch (test) {
	case HypothesisTest::Test::t_test_one_sample:
	case HypothesisTest::Test::t_test_two_sample:
	case HypothesisTest::Test::t_test_two_sample_paired:
	case HypothesisTest::Test::t_test_welch:
	case HypothesisTest::Test::one_way_anova:
	case HypothesisTest::Test::one_way_anova_repeated:
	case HypothesisTest::Test::mann_whitney_u_test:
	case HypothesisTest::Test::kruskal_wallis_test:
	case HypothesisTest::Test::wilcoxon_test:
	case HypothesisTest::Test::friedman_test:
		return 5;
	case HypothesisTest::Test::chisq_independence:
	case HypothesisTest::Test::chisq_goodness_of_fit:
		return 2;
	case HypothesisTest::Test::log_rank_test:
		return 2;
	}

	return 0;
}

// ##############################################################################
// ######################### Private implementation #############################
// ##############################################################################
HypothesisTestPrivate::HypothesisTestPrivate(HypothesisTest* owner)
	: q(owner) {
}

QString HypothesisTestPrivate::name() const {
	return q->name();
}

template<>
QVector<double> HypothesisTestPrivate::filterColumn<double>(const AbstractColumn* col) {
	QVector<double> sample;
	for (int row = 0; row < col->rowCount(); ++row) {
		if (rowIsValid(col, row))
			sample.append(col->valueAt(row));
	}
	return sample;
}

template<>
QVector<qint64> HypothesisTestPrivate::filterColumn<qint64>(const AbstractColumn* col) {
	QVector<qint64> sample;
	if (col->columnMode() == AbstractColumn::ColumnMode::Integer) {
		for (int row = 0; row < col->rowCount(); ++row) {
			if (rowIsValid(col, row))
				sample.append(static_cast<qint64>(col->integerAt(row)));
		}
	} else if (col->columnMode() == AbstractColumn::ColumnMode::BigInt) {
		for (int row = 0; row < col->rowCount(); ++row) {
			if (rowIsValid(col, row))
				sample.append(col->bigIntAt(row));
		}
	}
	return sample;
}

template<typename T>
QVector<QVector<T>> HypothesisTestPrivate::filterColumns(const QVector<const AbstractColumn*>& columns) {
	QVector<QVector<T>> samples;
	samples.reserve(columns.size());
	for (const auto* col : columns)
		samples.append(filterColumn<T>(col));
	return samples;
}

template<typename T>
QVector<QVector<T>> HypothesisTestPrivate::filterColumnsParallel(const QVector<const AbstractColumn*>& columns) {
	return QtConcurrent::mapped(columns, filterColumn<T>).results();
}

template<typename T>
T** HypothesisTestPrivate::toArrayOfArrays(const QVector<QVector<T>>& data) {
	T** result = new T*[data.size()];
	for (int i = 0; i < data.size(); ++i)
		result[i] = const_cast<T*>(data[i].constData());
	return result;
}

// ##############################################################################
// ##################  Result output related functions  #########################
// ##############################################################################
QString HypothesisTestPrivate::addResultTitle(const QString& text) {
	return QStringLiteral("<h1>") + text + QStringLiteral("</h1>");
}

QString HypothesisTestPrivate::addResultSection(const QString& text) {
	return QStringLiteral("<h2>") + text + QStringLiteral("</h2>");
}

QString HypothesisTestPrivate::addResultLine(const QString& name, const QString& value) {
	return QStringLiteral("<b>") + name + QStringLiteral("</b>: ") + value + QStringLiteral("<br>");
}

QString HypothesisTestPrivate::addResultLine(const QString& name, double value) {
	if (!std::isnan(value))
		return addResultLine(name, QLocale().toString(value));
	else
		return addResultLine(name, QStringLiteral("-"));
}

QString HypothesisTestPrivate::addResultColumnStatistics(const QVector<const AbstractColumn*>& columns) {
	QVector<QVector<QString>> data;

	data << columnStatisticsHeaders();

	for (auto* acol : columns) {
		auto* col = static_cast<const Column*>(acol);
		auto& statistics = col->statistics();
		data << QVector<QString>{col->name(),
								 QLocale().toString(statistics.size),
								 QLocale().toString(statistics.arithmeticMean),
								 QLocale().toString(statistics.median),
								 QLocale().toString(statistics.variance),
								 QLocale().toString(statistics.skewness),
								 QLocale().toString(statistics.kurtosis)};
	}

	return addResultTable(data, true);
}

QString HypothesisTestPrivate::addResultTable(const QVector<QVector<QString>>& data, bool horizontal) {
	if (data.size() < 2)
		return {};

	size_t n = data[0].size(); // assuming all columns have same length...

	QString html;
	html += QStringLiteral("<table>");

	if (horizontal) {
		size_t rowCount = data.size();
		html += QStringLiteral("<thead><tr>");
		for (const auto& header : data[0])
			html += QStringLiteral("<th>") + header + QStringLiteral("</th>");
		html += QStringLiteral("</tr></thead>");
		html += QStringLiteral("<tbody>");
		for (size_t rowIdx = 1; rowIdx < rowCount; ++rowIdx) {
			html += QStringLiteral("<tr>");
			for (const auto& cell : data[rowIdx])
				html += QStringLiteral("<td>") + cell + QStringLiteral("</td>");
			html += QStringLiteral("</tr>");
		}
		html += QStringLiteral("</tbody>");
	} else {
		size_t colCount = data.size();
		html += QStringLiteral("<tbody>");
		for (size_t colIdx = 0; colIdx < n; ++colIdx) {
			html += QStringLiteral("<tr>");
			html += QStringLiteral("<th>") + data[0][colIdx] + QStringLiteral("</th>");
			for (size_t rowIdx = 1; rowIdx < colCount; ++rowIdx)
				html += QStringLiteral("<td>") + data[rowIdx][colIdx] + QStringLiteral("</td>");
			html += QStringLiteral("</tr>");
		}
		html += QStringLiteral("</tbody>");
	}

	html += QStringLiteral("</table>");
	return html;
}

QString HypothesisTestPrivate::notAvailable() {
	return i18n("N/A");
}

QString HypothesisTestPrivate::testResultNotAvailable() {
	return i18n("Test result is not available.");
}

QString HypothesisTestPrivate::twoColumnsRequired() {
	return i18n("Two columns are required for this test.");
}

QString HypothesisTestPrivate::atLeastTwoColumnsRequired() {
	return i18n("At least two columns are required.");
}

QString HypothesisTestPrivate::atLeastXSamplesRequired() {
	return i18n("Column '%1' requires at least %2 samples.");
}

QString HypothesisTestPrivate::samplesMustBeEqualSize() {
	return i18n("The size of each sample must be equal.");
}

QVector<QString> HypothesisTestPrivate::columnStatisticsHeaders() {
	return {i18n("Name"), i18n("Size"), i18n("Mean"), i18n("Median"), i18n("Variance"), i18n("Skewness"), i18n("Kurtosis")};
}

QString HypothesisTestPrivate::emptyResultColumnStatistics() {
	return addResultTable(
		QVector<QVector<QString>>{columnStatisticsHeaders(),
								  {notAvailable(), notAvailable(), notAvailable(), notAvailable(), notAvailable(), notAvailable(), notAvailable()}},
		true);
}

QString HypothesisTestPrivate::resultTemplate(HypothesisTest::Test test) {
	bool hasDescriptiveStatistics = (test != HypothesisTest::Test::chisq_independence) && (test != HypothesisTest::Test::log_rank_test);
	bool hasDegreesOfFreedom = (test != HypothesisTest::Test::mann_whitney_u_test) && (test != HypothesisTest::Test::wilcoxon_test);

	QString result;
	result += (addResultTitle(HypothesisTest::testName(test)) + addResultLine(i18n("Null Hypothesis"), QStringLiteral("%1"))
			   + addResultLine(i18n("Alternate Hypothesis"), QStringLiteral("%2")));
	if (hasDescriptiveStatistics)
		result += (addResultSection(i18n("Descriptive Statistics")) + QStringLiteral("%3"));
	result += (addResultSection(i18n("Test Statistics")) + addResultLine(i18n("Significance Level"), QStringLiteral("%L4"))
			   + addResultLine(i18n("p-Value"), QStringLiteral("%L5")));
	if (hasDegreesOfFreedom) {
		if (test == HypothesisTest::Test::one_way_anova) {
			result += (addResultLine(i18n("Degrees of Freedom Between Groups"), QStringLiteral("%L6"))
					   + addResultLine(i18n("Degrees of Freedom Within Groups"), QStringLiteral("%L7")));
		} else if (test == HypothesisTest::Test::one_way_anova_repeated) {
			result += (addResultLine(i18n("Degrees of Freedom Treatment"), QStringLiteral("%L6"))
					   + addResultLine(i18n("Degrees of Freedom Residuals"), QStringLiteral("%L7")));
		} else {
			result += addResultLine(i18n("Degrees of Freedom"), QStringLiteral("%L6"));
		}
	}

	if (test == HypothesisTest::Test::t_test_one_sample) {
		result += (addResultLine(i18n("t-Value"), QStringLiteral("%L20")) + addResultLine(i18n("Test Mean"), QStringLiteral("%L21"))
				   + addResultLine(i18n("Mean Difference"), QStringLiteral("%L22")));
	} else if (test == HypothesisTest::Test::t_test_two_sample) {
		result +=
			(addResultLine(i18n("t-Value"), QStringLiteral("%L20")) + addResultLine(i18n("Pooled Variance"), QStringLiteral("%L21"))
			 + addResultLine(i18n("Mean Difference"), QStringLiteral("%L22")) + addResultLine(i18n("Mean Difference Standard Error"), QStringLiteral("%L23")));
	} else if (test == HypothesisTest::Test::t_test_two_sample_paired) {
		result += (addResultLine(i18n("t-Value"), QStringLiteral("%L20")) + addResultLine(i18n("Mean Difference"), QStringLiteral("%L21")));
	} else if (test == HypothesisTest::Test::t_test_welch) {
		result += (addResultLine(i18n("t-Value"), QStringLiteral("%L20")) + addResultLine(i18n("Mean Difference"), QStringLiteral("%L21"))
				   + addResultLine(i18n("Mean Difference Standard Error"), QStringLiteral("%L22")));
	} else if (test == HypothesisTest::Test::one_way_anova) {
		result += (addResultLine(i18n("F-Statistic"), QStringLiteral("%L20")) + addResultLine(i18n("Sum of Squares Between Groups"), QStringLiteral("%L21"))
				   + addResultLine(i18n("Sum of Squares Within Groups"), QStringLiteral("%L22"))
				   + addResultLine(i18n("Mean Squares Between Groups"), QStringLiteral("%L23"))
				   + addResultLine(i18n("Mean Squares Within Groups"), QStringLiteral("%L24")));
	} else if (test == HypothesisTest::Test::one_way_anova_repeated) {
		result +=
			(addResultLine(i18n("F-Statistic"), QStringLiteral("%L20")) + addResultLine(i18n("Sum of Squares Treatment"), QStringLiteral("%L21"))
			 + addResultLine(i18n("Sum of Squares Residuals"), QStringLiteral("%L22"))
			 + addResultLine(i18n("Sum of Squares Within Subjects"), QStringLiteral("%L23"))
			 + addResultLine(i18n("Mean Squares Treatment"), QStringLiteral("%L24")) + addResultLine(i18n("Mean Squares Residuals"), QStringLiteral("%L25")));
	} else if (test == HypothesisTest::Test::mann_whitney_u_test) {
		result += (addResultLine(i18n("U"), QStringLiteral("%L20")) + addResultLine(i18n("z-Value"), QStringLiteral("%L21"))
				   + addResultLine(i18n("Rank Sum 1"), QStringLiteral("%L22")) + addResultLine(i18n("Mean Rank 1"), QStringLiteral("%L23"))
				   + addResultLine(i18n("Rank Sum 2"), QStringLiteral("%L24")) + addResultLine(i18n("Mean Rank 2"), QStringLiteral("%L25")));
	} else if (test == HypothesisTest::Test::wilcoxon_test) {
		result += (addResultLine(i18n("W"), QStringLiteral("%L20")) + addResultLine(i18n("z-Value"), QStringLiteral("%L21"))
				   + addResultLine(i18n("Positive Rank Sum"), QStringLiteral("%L22")) + addResultLine(i18n("Positive Rank Mean"), QStringLiteral("%L23"))
				   + addResultLine(i18n("Positive Rank Count"), QStringLiteral("%L24")) + addResultLine(i18n("Negative Rank Sum"), QStringLiteral("%L25"))
				   + addResultLine(i18n("Negative Rank Mean"), QStringLiteral("%L26")) + addResultLine(i18n("Negative Rank Count"), QStringLiteral("%L27"))
				   + addResultLine(i18n("Tie Count"), QStringLiteral("%L28")));
	} else if (test == HypothesisTest::Test::kruskal_wallis_test) {
		result += addResultLine(i18n("H"), QStringLiteral("%L20"));
	} else if (test == HypothesisTest::Test::friedman_test) {
		result += addResultLine(i18n("Q"), QStringLiteral("%L20"));
	} else if (test == HypothesisTest::Test::chisq_goodness_of_fit) {
		result += addResultLine(i18n("Chi2"), QStringLiteral("%L20"));
	} else if (test == HypothesisTest::Test::chisq_independence) {
		result += addResultLine(i18n("Chi2"), QStringLiteral("%L20"));
	} else if (test == HypothesisTest::Test::log_rank_test) {
		result += (addResultLine(i18n("H"), QStringLiteral("%L20")) + addResultLine(i18n("Event Count 1"), QStringLiteral("%L21"))
				   + addResultLine(i18n("Censored Count 1"), QStringLiteral("%L22")) + addResultLine(i18n("Total Count 1"), QStringLiteral("%L23"))
				   + addResultLine(i18n("Event Count 2"), QStringLiteral("%L24")) + addResultLine(i18n("Censored Count 2"), QStringLiteral("%L25"))
				   + addResultLine(i18n("Total Count 2"), QStringLiteral("%L26")));
	}

	result += addResultSection(i18n("Statistical Conclusion")) + QStringLiteral("%99");
	return result;
}

// ##############################################################################
// ##################  (Re-)Calculation of the tests  ##########################
// ##############################################################################
void HypothesisTestPrivate::recalculate() {
	result.clear(); // clear the previous result
	Q_EMIT q->statusError(QString()); // reset the previous info message

	if (dataColumns.isEmpty()) {
		Q_EMIT q->statusError(i18n("No variable columns provided."));
		return;
	}

	switch (test) {
	case HypothesisTest::Test::t_test_one_sample:
		performOneSampleTTest();
		break;
	case HypothesisTest::Test::t_test_two_sample:
		performTwoSampleTTest(false /* paired */);
		break;
	case HypothesisTest::Test::t_test_two_sample_paired:
		performTwoSampleTTest(true /* paired */);
		break;
	case HypothesisTest::Test::t_test_welch:
		performWelchTTest();
		break;
	case HypothesisTest::Test::one_way_anova:
		performOneWayANOVATest();
		break;
	case HypothesisTest::Test::one_way_anova_repeated:
		performOneWayANOVARepeatedTest();
		break;
	case HypothesisTest::Test::mann_whitney_u_test:
		performMannWhitneyUTest();
		break;
	case HypothesisTest::Test::kruskal_wallis_test:
		performKruskalWallisTest();
		break;
	case HypothesisTest::Test::wilcoxon_test:
		performWilcoxonTest();
		break;
	case HypothesisTest::Test::friedman_test:
		performFriedmanTest();
		break;
	case HypothesisTest::Test::chisq_goodness_of_fit:
		performChisqGoodnessOfFitTest();
		break;
	case HypothesisTest::Test::chisq_independence:
		performChisqIndependenceTest();
		break;
	case HypothesisTest::Test::log_rank_test:
		performLogRankTest();
		break;
	}

	if (result.isEmpty())
		resetResult();

	Q_EMIT q->changed(); // notify the view about the changes
}

void HypothesisTestPrivate::resetResult() {
	result.clear(); // clear the previous result

	if (test == HypothesisTest::Test::t_test_one_sample) {
		result = resultTemplate(test)
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(emptyResultColumnStatistics())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(testResultNotAvailable());
	} else if (test == HypothesisTest::Test::t_test_two_sample) {
		result = resultTemplate(test)
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(emptyResultColumnStatistics())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(testResultNotAvailable());
	} else if (test == HypothesisTest::Test::t_test_two_sample_paired) {
		result = resultTemplate(test)
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(emptyResultColumnStatistics())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(testResultNotAvailable());
	} else if (test == HypothesisTest::Test::t_test_welch) {
		result = resultTemplate(test)
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(emptyResultColumnStatistics())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(testResultNotAvailable());
	} else if (test == HypothesisTest::Test::one_way_anova) {
		result = resultTemplate(test)
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(emptyResultColumnStatistics())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(testResultNotAvailable());
	} else if (test == HypothesisTest::Test::one_way_anova_repeated) {
		result = resultTemplate(test)
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(emptyResultColumnStatistics())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(testResultNotAvailable());
	} else if (test == HypothesisTest::Test::mann_whitney_u_test) {
		result = resultTemplate(test)
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(emptyResultColumnStatistics())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(testResultNotAvailable());
	} else if (test == HypothesisTest::Test::kruskal_wallis_test) {
		result = resultTemplate(test)
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(emptyResultColumnStatistics())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(testResultNotAvailable());
	} else if (test == HypothesisTest::Test::wilcoxon_test) {
		result = resultTemplate(test)
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(emptyResultColumnStatistics())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(testResultNotAvailable());
	} else if (test == HypothesisTest::Test::friedman_test) {
		result = resultTemplate(test)
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(emptyResultColumnStatistics())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(testResultNotAvailable());
	} else if (test == HypothesisTest::Test::chisq_goodness_of_fit) {
		result = resultTemplate(test)
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(emptyResultColumnStatistics())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(testResultNotAvailable());
	} else if (test == HypothesisTest::Test::chisq_independence) {
		result = resultTemplate(test)
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(testResultNotAvailable());
	} else if (test == HypothesisTest::Test::log_rank_test) {
		result = resultTemplate(test)
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(notAvailable())
					 .arg(testResultNotAvailable());
	}

	Q_EMIT q->changed(); // notify the view about the changes
}

void HypothesisTestPrivate::performOneSampleTTest() {
	const auto* col = dataColumns.constFirst();

	QVector<double> sample = filterColumn<double>(col);
	size_t n = sample.size();

	if (n < minSampleCount(test)) {
		Q_EMIT q->statusError(atLeastXSamplesRequired().arg(col->name()).arg(minSampleCount(test)));
		return;
	}

	one_sample_t_test_result result;
	result.p = NAN;
	if (n != 0)
		result = nsl_stats_one_sample_t(sample.constData(), n, testMean, tail);

	const auto [nullHypothesisText, alternateHypothesisText] = HypothesisTest::hypothesisText(test).at(tail);

	QString conclusion;
	if (!std::isnan(result.p))
		if (result.p <= significanceLevel)
			conclusion = i18n("At the significance level %1, the population mean is significantly different from %2. Reject the null hypothesis.",
							  significanceLevel,
							  testMean);
		else
			conclusion = i18n("At the significance level %1, the population mean is not significantly different from %2. Fail to reject the null hypothesis.",
							  significanceLevel,
							  testMean);
	else
		conclusion = testResultNotAvailable();

	// present results
	this->result = resultTemplate(test)
					   .arg(nullHypothesisText)
					   .arg(alternateHypothesisText)
					   .arg(addResultColumnStatistics(QVector<const AbstractColumn*>({col})))
					   .arg(significanceLevel)
					   .arg(result.p)
					   .arg(result.df)
					   .arg(result.t)
					   .arg(testMean)
					   .arg(result.mean_difference)
					   .arg(conclusion);
}

void HypothesisTestPrivate::performTwoSampleTTest(bool paired) {
	if (dataColumns.size() < 2) { // we need two columns for the two sample t test
		Q_EMIT q->statusError(twoColumnsRequired());
		return;
	}

	const auto* col1 = dataColumns.at(0);
	const auto* col2 = dataColumns.at(1);

	QVector<QVector<double>> samples = filterColumnsParallel<double>(QVector<const AbstractColumn*>{col1, col2});
	size_t n1 = samples[0].size();
	size_t n2 = samples[1].size();

	if (n1 < minSampleCount(test)) {
		Q_EMIT q->statusError(atLeastXSamplesRequired().arg(col1->name()).arg(minSampleCount(test)));
		return;
	}

	if (n2 < minSampleCount(test)) {
		Q_EMIT q->statusError(atLeastXSamplesRequired().arg(col2->name()).arg(minSampleCount(test)));
		return;
	}

	double p = NAN;
	int df = 0;
	double t = 0;
	double pooledVariance = NAN;
	double meanDifference = 0;
	double meanDifferenceStandardError = NAN;

	if (n1 != 0 && n2 != 0) {
		if (paired) {
			if (n1 != n2) {
				Q_EMIT q->statusError(samplesMustBeEqualSize());
				return;
			}

			QVector<double> differences;
			differences.reserve(n1);
			for (size_t i = 0; i < n1; ++i)
				differences.append(samples[0][i] - samples[1][i]);

			one_sample_t_test_result result = nsl_stats_one_sample_t(differences.constData(), n1, 0.0, tail);
			p = result.p;
			df = result.df;
			t = result.t;
			meanDifference = result.mean_difference;
		} else {
			independent_t_test_result result = nsl_stats_independent_t(samples[0].constData(), n1, samples[1].constData(), n2, tail);
			p = result.p;
			df = result.df;
			t = result.t;
			pooledVariance = result.pooled_variance;
			meanDifference = result.mean_difference;
			meanDifferenceStandardError = result.mean_difference_standard_error;
		}
	}

	const auto [nullHypothesisText, alternateHypothesisText] = HypothesisTest::hypothesisText(test).at(tail);

	// conclusion
	QString conclusion;
	if (!std::isnan(p)) {
		if (p <= significanceLevel)
			conclusion = i18n("At the significance level %1, the population means are significantly different. Reject the null Hypothesis", significanceLevel);
		else
			conclusion = i18n("At the significance level %1, the population means are not significantly different. Fail to reject the null Hypothesis",
							  significanceLevel);
	} else
		conclusion = testResultNotAvailable();

	if (paired) {
		this->result = resultTemplate(test)
						   .arg(nullHypothesisText)
						   .arg(alternateHypothesisText)
						   .arg(addResultColumnStatistics(QVector<const AbstractColumn*>({col1, col2})))
						   .arg(significanceLevel)
						   .arg(p)
						   .arg(df)
						   .arg(t)
						   .arg(meanDifference)
						   .arg(conclusion);
	} else {
		this->result = resultTemplate(test)
						   .arg(nullHypothesisText)
						   .arg(alternateHypothesisText)
						   .arg(addResultColumnStatistics(QVector<const AbstractColumn*>({col1, col2})))
						   .arg(significanceLevel)
						   .arg(p)
						   .arg(df)
						   .arg(t)
						   .arg(pooledVariance)
						   .arg(meanDifference)
						   .arg(meanDifferenceStandardError)
						   .arg(conclusion);
	}
}

void HypothesisTestPrivate::performWelchTTest() {
	if (dataColumns.size() < 2) { // we need two columns for the welch t test
		Q_EMIT q->statusError(twoColumnsRequired());
		return;
	}

	const auto* col1 = dataColumns.at(0);
	const auto* col2 = dataColumns.at(1);

	auto samples = filterColumnsParallel<double>(QVector<const AbstractColumn*>{col1, col2});
	size_t n1 = samples.at(0).size();
	size_t n2 = samples.at(1).size();

	if (n1 < minSampleCount(test)) {
		Q_EMIT q->statusError(atLeastXSamplesRequired().arg(col1->name()).arg(minSampleCount(test)));
		return;
	}

	if (n2 < minSampleCount(test)) {
		Q_EMIT q->statusError(atLeastXSamplesRequired().arg(col2->name()).arg(minSampleCount(test)));
		return;
	}

	welch_t_test_result result;
	result.p = NAN;

	if (n1 != 0 && n2 != 0)
		result = nsl_stats_welch_t(samples.at(0).constData(), n1, samples.at(1).constData(), n2, tail);

	const auto [nullHypothesisText, alternateHypothesisText] = HypothesisTest::hypothesisText(test).at(tail);

	QString conclusion;
	if (!std::isnan(result.p)) {
		if (result.p <= significanceLevel)
			conclusion = i18n("At the significance level %1, the population means are significantly different. Reject the null Hypothesis", significanceLevel);
		else
			conclusion = i18n("At the significance level %1, the population means are not significantly different. Fail to reject the null Hypothesis",
							  significanceLevel);
	} else
		conclusion = testResultNotAvailable();

	this->result = resultTemplate(test)
					   .arg(nullHypothesisText)
					   .arg(alternateHypothesisText)
					   .arg(addResultColumnStatistics(QVector<const AbstractColumn*>({col1, col2})))
					   .arg(significanceLevel)
					   .arg(result.p)
					   .arg(result.df)
					   .arg(result.t)
					   .arg(result.mean_difference)
					   .arg(result.mean_difference_standard_error)
					   .arg(conclusion);
}

void HypothesisTestPrivate::performOneWayANOVATest() {
	if (dataColumns.size() < 2) {
		Q_EMIT q->statusError(atLeastTwoColumnsRequired());
		return;
	}

	const int groupCount = dataColumns.size();

	QVector<QVector<double>> groupData(filterColumnsParallel<double>(dataColumns));

	Q_ASSERT(groupCount == groupData.size());

	QVector<size_t> groupSizes(groupCount);

	for (int n = 0; n < groupCount; n++) {
		size_t sampleSize = groupData[n].size();
		if (sampleSize < minSampleCount(test)) {
			Q_EMIT q->statusError(atLeastXSamplesRequired().arg(dataColumns.at(n)->name()).arg(minSampleCount(test)));
			return;
		}
		groupSizes[n] = static_cast<size_t>(sampleSize);
	}

	double** groups = toArrayOfArrays(groupData);

	anova_oneway_test_result result = nsl_stats_anova_oneway_f(const_cast<const double**>(groups), groupSizes.data(), static_cast<size_t>(groupCount));

	delete[] groups;

	const auto [nullHypothesisText, alternateHypothesisText] = HypothesisTest::hypothesisText(test).at(0);

	QString conclusion;
	if (!std::isnan(result.p)) {
		if (result.p <= significanceLevel)
			conclusion =
				i18n("At the significance level %1, at least one group mean is significantly different. Reject the null Hypothesis", significanceLevel);
		else
			conclusion =
				i18n("At the significance level %1, no significant difference between group means. Fail to reject the null Hypothesis", significanceLevel);
	} else {
		conclusion = testResultNotAvailable();
	}

	this->result = resultTemplate(test)
					   .arg(nullHypothesisText)
					   .arg(alternateHypothesisText)
					   .arg(addResultColumnStatistics(dataColumns))
					   .arg(significanceLevel)
					   .arg(result.p)
					   .arg(result.df_between_groups)
					   .arg(result.df_within_groups)
					   .arg(result.F)
					   .arg(result.ss_between_groups)
					   .arg(result.ss_within_groups)
					   .arg(result.ms_between_groups)
					   .arg(result.ms_within_groups)
					   .arg(conclusion);
}

void HypothesisTestPrivate::performOneWayANOVARepeatedTest() {
	if (dataColumns.size() < 2) {
		Q_EMIT q->statusError(atLeastTwoColumnsRequired());
		return;
	}

	const int groupCount = dataColumns.size();

	QVector<QVector<double>> groupData(filterColumnsParallel<double>(dataColumns));

	Q_ASSERT(groupCount == groupData.size());

	auto groupSize = groupData[0].size();

	if (static_cast<size_t>(groupSize) < minSampleCount(test)) {
		Q_EMIT q->statusError(atLeastXSamplesRequired().arg(dataColumns.at(0)->name()).arg(minSampleCount(test)));
		return;
	}

	for (int n = 1; n < groupCount; n++) {
		if (groupData[n].size() != groupSize) {
			Q_EMIT q->statusError(samplesMustBeEqualSize());
			return;
		}
	}

	double** groups = toArrayOfArrays(groupData);

	anova_oneway_repeated_test_result result =
		nsl_stats_anova_oneway_repeated_f(const_cast<const double**>(groups), static_cast<size_t>(groupSize), static_cast<size_t>(groupCount));

	delete[] groups;

	const auto [nullHypothesisText, alternateHypothesisText] = HypothesisTest::hypothesisText(test).at(0);

	QString conclusion;
	if (!std::isnan(result.p)) {
		if (result.p <= significanceLevel)
			conclusion =
				i18n("At the significance level %1, at least one group mean is significantly different. Reject the null Hypothesis", significanceLevel);
		else
			conclusion =
				i18n("At the significance level %1, no significant difference between group means. Fail to reject the null Hypothesis", significanceLevel);
	} else {
		conclusion = testResultNotAvailable();
	}

	this->result = resultTemplate(test)
					   .arg(nullHypothesisText)
					   .arg(alternateHypothesisText)
					   .arg(addResultColumnStatistics(dataColumns))
					   .arg(significanceLevel)
					   .arg(result.p)
					   .arg(result.df_treatment)
					   .arg(result.df_residuals)
					   .arg(result.F)
					   .arg(result.ss_treatment)
					   .arg(result.ss_residuals)
					   .arg(result.ss_within_subjects)
					   .arg(result.ms_treatment)
					   .arg(result.ms_residuals)
					   .arg(conclusion);
}

void HypothesisTestPrivate::performMannWhitneyUTest() {
	if (dataColumns.size() < 2) {
		Q_EMIT q->statusError(twoColumnsRequired());
		return;
	}

	const auto* col1 = dataColumns.at(0);
	const auto* col2 = dataColumns.at(1);

	auto samples = filterColumnsParallel<double>(QVector<const AbstractColumn*>{col1, col2});
	size_t n1 = samples.at(0).size();
	size_t n2 = samples.at(1).size();

	if (n1 < minSampleCount(test)) {
		Q_EMIT q->statusError(atLeastXSamplesRequired().arg(col1->name()).arg(minSampleCount(test)));
		return;
	}

	if (n2 < minSampleCount(test)) {
		Q_EMIT q->statusError(atLeastXSamplesRequired().arg(col2->name()).arg(minSampleCount(test)));
		return;
	}

	mannwhitney_test_result result;
	result.p = NAN;

	if (n1 != 0 && n2 != 0)
		result = nsl_stats_mannwhitney_u(samples.at(0).constData(), n1, samples.at(1).constData(), n2, tail);

	const auto [nullHypothesisText, alternateHypothesisText] = HypothesisTest::hypothesisText(test).at(tail);

	QString conclusion;
	if (!std::isnan(result.p)) {
		if (result.p <= significanceLevel)
			conclusion =
				i18n("At the significance level %1, at least one group distribution is significantly different. Reject the null Hypothesis", significanceLevel);
		else
			conclusion = i18n("At the significance level %1, no significant difference between group distributions. Fail to reject the null Hypothesis",
							  significanceLevel);
	} else {
		conclusion = testResultNotAvailable();
	}

	this->result = resultTemplate(test)
					   .arg(nullHypothesisText)
					   .arg(alternateHypothesisText)
					   .arg(addResultColumnStatistics(QVector<const AbstractColumn*>({col1, col2})))
					   .arg(significanceLevel)
					   .arg(result.p)
					   .arg(result.U)
					   .arg(result.z)
					   .arg(result.rank_sum1)
					   .arg(result.mean_rank1)
					   .arg(result.rank_sum2)
					   .arg(result.mean_rank2)
					   .arg(conclusion);
}

void HypothesisTestPrivate::performKruskalWallisTest() {
	if (dataColumns.size() < 2) {
		Q_EMIT q->statusError(atLeastTwoColumnsRequired());
		return;
	}

	const int groupCount = dataColumns.size();

	QVector<QVector<double>> groupData(filterColumnsParallel<double>(dataColumns));

	Q_ASSERT(groupCount == groupData.size());

	QVector<size_t> groupSizes(groupCount);

	for (int n = 0; n < groupCount; n++) {
		size_t sampleSize = groupData[n].size();
		if (sampleSize < minSampleCount(test)) {
			Q_EMIT q->statusError(atLeastXSamplesRequired().arg(dataColumns.at(n)->name()).arg(minSampleCount(test)));
			return;
		}
		groupSizes[n] = sampleSize;
	}

	double** groups = toArrayOfArrays(groupData);

	auto result = nsl_stats_kruskal_wallis_h(const_cast<const double**>(groups), groupSizes.data(), static_cast<size_t>(groupCount));

	delete[] groups;

	const auto [nullHypothesisText, alternateHypothesisText] = HypothesisTest::hypothesisText(test).at(0);

	QString conclusion;
	if (!std::isnan(result.p)) {
		if (result.p <= significanceLevel)
			conclusion =
				i18n("At the significance level %1, at least one group distribution is significantly different. Reject the null Hypothesis", significanceLevel);
		else
			conclusion = i18n("At the significance level %1, no significant difference between group distributions. Fail to reject the null Hypothesis",
							  significanceLevel);
	} else {
		conclusion = testResultNotAvailable();
	}

	this->result = resultTemplate(test)
					   .arg(nullHypothesisText)
					   .arg(alternateHypothesisText)
					   .arg(addResultColumnStatistics(dataColumns))
					   .arg(significanceLevel)
					   .arg(result.p)
					   .arg(result.df)
					   .arg(result.H)
					   .arg(conclusion);
}

void HypothesisTestPrivate::performWilcoxonTest() {
	if (dataColumns.size() < 2) {
		Q_EMIT q->statusError(twoColumnsRequired());
		return;
	}

	const auto* col1 = dataColumns.at(0);
	const auto* col2 = dataColumns.at(1);

	auto samples = filterColumnsParallel<double>(QVector<const AbstractColumn*>{col1, col2});
	size_t n1 = samples.at(0).size();
	size_t n2 = samples.at(1).size();

	if (n1 < minSampleCount(test)) {
		Q_EMIT q->statusError(atLeastXSamplesRequired().arg(col1->name()).arg(minSampleCount(test)));
		return;
	}

	if (n2 < minSampleCount(test)) {
		Q_EMIT q->statusError(atLeastXSamplesRequired().arg(col2->name()).arg(minSampleCount(test)));
		return;
	}

	wilcoxon_test_result result;
	result.p = NAN;

	if (n1 != 0 && n2 != 0) {
		if (n1 != n2) {
			Q_EMIT q->statusError(samplesMustBeEqualSize());
			return;
		}
		result = nsl_stats_wilcoxon_w(samples.at(0).constData(), samples.at(1).constData(), n1, tail);
	}

	const auto [nullHypothesisText, alternateHypothesisText] = HypothesisTest::hypothesisText(test).at(tail);

	QString conclusion;
	if (!std::isnan(result.p)) {
		if (result.p <= significanceLevel)
			conclusion =
				i18n("At the significance level %1, at least one group distribution is significantly different. Reject the null Hypothesis", significanceLevel);
		else
			conclusion = i18n("At the significance level %1, no significant difference between group distributions. Fail to reject the null Hypothesis",
							  significanceLevel);
	} else {
		conclusion = testResultNotAvailable();
	}

	this->result = resultTemplate(test)
					   .arg(nullHypothesisText)
					   .arg(alternateHypothesisText)
					   .arg(addResultColumnStatistics(QVector<const AbstractColumn*>({col1, col2})))
					   .arg(significanceLevel)
					   .arg(result.p)
					   .arg(result.W)
					   .arg(result.z)
					   .arg(result.positive_rank_sum)
					   .arg(result.positive_rank_mean)
					   .arg(result.positive_rank_count)
					   .arg(result.negative_rank_sum)
					   .arg(result.negative_rank_mean)
					   .arg(result.negative_rank_count)
					   .arg(result.tie_count)
					   .arg(conclusion);
}

void HypothesisTestPrivate::performFriedmanTest() {
	if (dataColumns.size() < 2) {
		Q_EMIT q->statusError(atLeastTwoColumnsRequired());
		return;
	}

	const int groupCount = dataColumns.size();

	QVector<QVector<double>> groupData(filterColumnsParallel<double>(dataColumns));

	Q_ASSERT(groupCount == groupData.size());

	const auto groupSize = groupData.at(0).size();

	if (static_cast<size_t>(groupSize) < minSampleCount(test)) {
		Q_EMIT q->statusError(atLeastXSamplesRequired().arg(dataColumns.at(0)->name()).arg(minSampleCount(test)));
		return;
	}

	for (int n = 1; n < groupCount; n++) {
		if (groupData.at(n).size() != groupSize) {
			Q_EMIT q->statusError(samplesMustBeEqualSize());
			return;
		}
	}

	double** groups = toArrayOfArrays(groupData);

	auto result = nsl_stats_friedman_q(const_cast<const double**>(groups), static_cast<size_t>(groupSize), static_cast<size_t>(groupCount));

	delete[] groups;

	const auto [nullHypothesisText, alternateHypothesisText] = HypothesisTest::hypothesisText(test).at(0);

	QString conclusion;
	if (!std::isnan(result.p)) {
		if (result.p <= significanceLevel)
			conclusion = i18n("At the significance level %1, at least one group is significantly different. Reject the null Hypothesis", significanceLevel);
		else
			conclusion = i18n("At the significance level %1, no significant difference between groups. Fail to reject the null Hypothesis", significanceLevel);
	} else {
		conclusion = testResultNotAvailable();
	}

	this->result = resultTemplate(test)
					   .arg(nullHypothesisText)
					   .arg(alternateHypothesisText)
					   .arg(addResultColumnStatistics(dataColumns))
					   .arg(significanceLevel)
					   .arg(result.p)
					   .arg(result.df)
					   .arg(result.Q)
					   .arg(conclusion);
}

void HypothesisTestPrivate::performChisqGoodnessOfFitTest() {
	if (dataColumns.size() < 2) { // we need two columns for the welch t test
		Q_EMIT q->statusError(twoColumnsRequired());
		return;
	}

	const auto* observed = dataColumns.at(0);
	const auto* expected = dataColumns.at(1);

	if (observed->columnMode() != AbstractColumn::ColumnMode::Integer && observed->columnMode() != AbstractColumn::ColumnMode::BigInt) {
		Q_EMIT q->statusError(i18n("Column '%1' must be of integer type.", observed->name()));
		return;
	}

	auto sample1 = filterColumn<qint64>(observed);
	auto sample2 = filterColumn<double>(expected);

	size_t n1 = sample1.size();
	size_t n2 = sample2.size();

	if (n1 < minSampleCount(test)) {
		Q_EMIT q->statusError(atLeastXSamplesRequired().arg(observed->name()).arg(minSampleCount(test)));
		return;
	}

	if (n2 < minSampleCount(test)) {
		Q_EMIT q->statusError(atLeastXSamplesRequired().arg(expected->name()).arg(minSampleCount(test)));
		return;
	}

	chisq_gof_test_result result;
	result.p = NAN;

	if (n1 != 0 && n2 != 0) {
		if (n1 != n2) {
			Q_EMIT q->statusError(samplesMustBeEqualSize());
			return;
		}

		result = nsl_stats_chisq_gof_x2(sample1.constData(), sample2.constData(), static_cast<size_t>(n1), 0);
	}

	const auto [nullHypothesisText, alternateHypothesisText] = HypothesisTest::hypothesisText(test).at(tail);

	// conclusion
	QString conclusion;
	if (!std::isnan(result.p)) {
		if (result.p <= significanceLevel)
			conclusion =
				i18n("At the significance level %1, the observed frequencies do not follow the expected frequency distribution. Reject the null Hypothesis",
					 significanceLevel);
		else
			conclusion =
				i18n("At the significance level %1, the observed frequencies follow the expected frequency distribution. Fail to reject the null Hypothesis",
					 significanceLevel);
	} else
		conclusion = testResultNotAvailable();

	this->result = resultTemplate(test)
					   .arg(nullHypothesisText)
					   .arg(nullHypothesisText)
					   .arg(addResultColumnStatistics(QVector<const AbstractColumn*>({observed, expected})))
					   .arg(significanceLevel)
					   .arg(result.p)
					   .arg(result.df)
					   .arg(result.x2)
					   .arg(conclusion);
}

void HypothesisTestPrivate::performChisqIndependenceTest() {
	if (dataColumns.size() < 2) { // we need at least two columns for the chi square independence test
		Q_EMIT q->statusError(atLeastTwoColumnsRequired());
		return;
	}

	for (auto* col : dataColumns) {
		if (col->columnMode() != AbstractColumn::ColumnMode::Integer && col->columnMode() != AbstractColumn::ColumnMode::BigInt) {
			Q_EMIT q->statusError(i18n("Column '%1' must be of integer type.", col->name()));
			return;
		}
	}

	const int columnCount = dataColumns.size();

	QVector<QVector<qint64>> table(filterColumnsParallel<qint64>(dataColumns));

	Q_ASSERT(columnCount == table.size());

	auto rowCount = table[0].size();

	if (static_cast<size_t>(rowCount) < minSampleCount(test)) {
		Q_EMIT q->statusError(atLeastXSamplesRequired().arg(dataColumns.at(0)->name()).arg(minSampleCount(test)));
		return;
	}

	for (int n = 1; n < columnCount; n++) {
		if (table[n].size() != rowCount) {
			Q_EMIT q->statusError(i18n("The size of each column in the contingency table must be equal."));
			return;
		}
	}

	long long** tableData = toArrayOfArrays(table);

	auto result = nsl_stats_chisq_ind_x2(const_cast<const long long**>(tableData), static_cast<size_t>(rowCount), static_cast<size_t>(columnCount));

	delete[] tableData;

	const auto [nullHypothesisText, alternateHypothesisText] = HypothesisTest::hypothesisText(test).at(0);

	QString conclusion;
	if (!std::isnan(result.p)) {
		if (result.p <= significanceLevel)
			conclusion = i18n("At the significance level %1, both categorical variables are dependent. Reject the null Hypothesis", significanceLevel);
		else
			conclusion =
				i18n("At the significance level %1, both categorical variables are independent. Fail to reject the null Hypothesis", significanceLevel);
	} else {
		conclusion = testResultNotAvailable();
	}

	this->result = resultTemplate(test)
					   .arg(nullHypothesisText)
					   .arg(alternateHypothesisText)
					   .arg(significanceLevel)
					   .arg(result.p)
					   .arg(result.df)
					   .arg(result.x2)
					   .arg(conclusion);
}

void HypothesisTestPrivate::performLogRankTest() {
	// time, status and group columns required
	if (dataColumns.size() < 3) {
		Q_EMIT q->statusError(i18n("Three columns are required."));
		return;
	}

	const auto* timeCol = dataColumns.at(0);
	const auto* statusCol = dataColumns.at(1);
	const auto* groupCol = dataColumns.at(2);

	if (statusCol->columnMode() != AbstractColumn::ColumnMode::Integer) {
		Q_EMIT q->statusError(i18n("Status column should be of integer type."));
		return;
	}

	if (groupCol->columnMode() != AbstractColumn::ColumnMode::Integer) {
		Q_EMIT q->statusError(i18n("Group column should be of integer type."));
		return;
	}

	if (statusCol->rowCount() != timeCol->rowCount() || groupCol->rowCount() != timeCol->rowCount()) {
		Q_EMIT q->statusError(i18n("Time, Status and Group columns should have equal size."));
		return;
	}

	const int n = timeCol->rowCount();

	QVector<double> time;
	QVector<int> status;
	QVector<size_t> g0Indices, g1Indices;

	for (int row = 0; row < n; ++row) {
		if (!rowIsValid(timeCol, row) || !rowIsValid(statusCol, row) || !rowIsValid(groupCol, row))
			continue;

		double t = timeCol->valueAt(row);
		int s = statusCol->integerAt(row); // must be 0 or 1
		int g = groupCol->integerAt(row); // must be 0 or 1

		if (s != 0 && s != 1) {
			Q_EMIT q->statusError(i18n("Status column values must be either 0 or 1."));
			return;
		}

		if (g != 0 && g != 1) {
			Q_EMIT q->statusError(i18n("Group column values must be either 0 or 1."));
			return;
		}

		time.append(t);
		status.append(s);

		size_t index = time.size() - 1;

		if (g == 0)
			g0Indices.append(index);
		else if (g == 1)
			g1Indices.append(index);
	}

	if ((time.size() != status.size()) || g0Indices.size() + g1Indices.size() != time.size()) {
		Q_EMIT q->statusError(i18n("Time, Status and Group columns should have equal number of values."));
		return;
	}

	size_t group1size = g0Indices.size();
	size_t group2size = g1Indices.size();

	if (group1size < minSampleCount(test)) {
		Q_EMIT q->statusError(i18n("Group 1 requires at least %1 samples.", minSampleCount(test)));
		return;
	}

	if (group2size < minSampleCount(test)) {
		Q_EMIT q->statusError(i18n("Group 2 requires at least %1 samples.", minSampleCount(test)));
		return;
	}

	log_rank_test_result result =
		nsl_stats_log_rank_h(time.constData(), status.constData(), g0Indices.constData(), group1size, g1Indices.constData(), group2size);

	const auto [nullHypothesisText, alternateHypothesisText] = HypothesisTest::hypothesisText(test).at(0);

	QString conclusion;
	if (!std::isnan(result.p)) {
		if (result.p <= significanceLevel)
			conclusion = i18n("At the significance level %1, the survival curves are significantly different. Reject the null Hypothesis", significanceLevel);
		else
			conclusion =
				i18n("At the significance level %1, no significant difference between survival curves. Fail to reject the null Hypothesis", significanceLevel);
	} else {
		conclusion = testResultNotAvailable();
	}

	this->result = resultTemplate(test)
					   .arg(nullHypothesisText)
					   .arg(alternateHypothesisText)
					   .arg(significanceLevel)
					   .arg(result.p)
					   .arg(result.df)
					   .arg(result.H)
					   .arg(result.event_count1)
					   .arg(result.censored_count1)
					   .arg(result.total_count1)
					   .arg(result.event_count2)
					   .arg(result.censored_count2)
					   .arg(result.total_count2)
					   .arg(conclusion);
}

// ##############################################################################
// ##########################  Static helpers   #################################
// ##############################################################################
bool HypothesisTestPrivate::rowIsValid(const AbstractColumn* column, int row) {
	return column->isValid(row) && !column->isMasked(row);
}

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################

//! Save as XML
void HypothesisTest::save(QXmlStreamWriter* writer) const {
	Q_D(const HypothesisTest);
	writer->writeStartElement(QLatin1String("hypothesisTest"));
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	writer->writeStartElement(QLatin1String("general"));
	writer->writeAttribute(QLatin1String("test"), QString::number(static_cast<int>(d->test)));
	writer->writeAttribute(QLatin1String("testMean"), QString::number(d->testMean));
	writer->writeAttribute(QLatin1String("significanceLevel"), QString::number(d->significanceLevel));
	writer->writeAttribute(QLatin1String("tail"), QString::number(static_cast<int>(d->tail)));
	writer->writeAttribute(QLatin1String("result"), d->result);
	WRITE_COLUMNS(d->dataColumns, d->dataColumnPaths);
	writer->writeEndElement(); // general
	writer->writeEndElement(); // hypothesisTest
}

//! Load from XML
bool HypothesisTest::load(XmlStreamReader* reader, bool preview) {
	Q_UNUSED(preview)
	if (!readBasicAttributes(reader))
		return false;

	Q_D(HypothesisTest);
	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == QLatin1String("hypothesisTest"))
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == QLatin1String("comment")) {
			if (!readCommentElement(reader))
				return false;
		} else if (!preview && reader->name() == QLatin1String("general")) {
			attribs = reader->attributes();

			d->result = attribs.value(QStringLiteral("result")).toString();
			READ_INT_VALUE("test", test, HypothesisTest::Test);
			READ_DOUBLE_VALUE("testMean", testMean);
			READ_DOUBLE_VALUE("significanceLevel", significanceLevel);
			READ_INT_VALUE("tail", tail, nsl_stats_tail_type);
		} else if (reader->name() == QLatin1String("column")) {
			attribs = reader->attributes();

			str = attribs.value(QStringLiteral("path")).toString();
			if (!str.isEmpty())
				d->dataColumnPaths << str;
		} else { // unknown element
			reader->raiseWarning(i18n("unknown element '%1'", reader->name().toString()));
			if (!reader->skipToEndElement())
				return false;
		}
	}

	return !reader->hasError();
}

void HypothesisTest::fillAddNewHypothesisTest(QMenu* menu, QActionGroup* actionGroup) {
	auto* subMenu = new QMenu(i18n("Parametric"));

	auto* action = new QAction(testName(Test::t_test_one_sample), actionGroup);
	action->setData(static_cast<int>(Test::t_test_one_sample));
	subMenu->addAction(action);

	action = new QAction(testName(Test::t_test_two_sample), actionGroup);
	action->setData(static_cast<int>(Test::t_test_two_sample));
	subMenu->addAction(action);

	action = new QAction(testName(Test::t_test_two_sample_paired), actionGroup);
	action->setData(static_cast<int>(Test::t_test_two_sample_paired));
	subMenu->addAction(action);

	action = new QAction(testName(Test::t_test_welch), actionGroup);
	action->setData(static_cast<int>(Test::t_test_welch));
	subMenu->addAction(action);

	subMenu->addSeparator();

	action = new QAction(testName(Test::one_way_anova), actionGroup);
	action->setData(static_cast<int>(Test::one_way_anova));
	subMenu->addAction(action);

	action = new QAction(testName(Test::one_way_anova_repeated), actionGroup);
	action->setData(static_cast<int>(Test::one_way_anova_repeated));
	subMenu->addAction(action);

	menu->addMenu(subMenu);

	subMenu = new QMenu(i18n("Non-Parametric"));

	// t-type related tests
	action = new QAction(testName(Test::mann_whitney_u_test), actionGroup);
	action->setData(static_cast<int>(Test::mann_whitney_u_test));
	subMenu->addAction(action);

	action = new QAction(testName(Test::wilcoxon_test), actionGroup);
	action->setData(static_cast<int>(Test::wilcoxon_test));
	subMenu->addAction(action);

	subMenu->addSeparator();

	// ANOVA-related tests
	action = new QAction(testName(Test::kruskal_wallis_test), actionGroup);
	action->setData(static_cast<int>(Test::kruskal_wallis_test));
	subMenu->addAction(action);

	action = new QAction(testName(Test::friedman_test), actionGroup);
	action->setData(static_cast<int>(Test::friedman_test));
	subMenu->addAction(action);

	subMenu->addSeparator();

	// chi^2 tests
	action = new QAction(testName(Test::chisq_independence), actionGroup);
	action->setData(static_cast<int>(Test::chisq_independence));
	subMenu->addAction(action);

	action = new QAction(testName(Test::chisq_goodness_of_fit), actionGroup);
	action->setData(static_cast<int>(Test::chisq_goodness_of_fit));
	subMenu->addAction(action);

	subMenu->addSeparator();

	action = new QAction(testName(Test::log_rank_test), actionGroup);
	action->setData(static_cast<int>(Test::log_rank_test));
	subMenu->addAction(action);

	menu->addMenu(subMenu);
}
