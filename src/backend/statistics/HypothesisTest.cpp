/*
	File                 : HypothesisTest.cpp
	Project              : LabPlot
	Description          : Hypothesis Test
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2019  Devanshu Agarwal(agarwaldevanshu8@gmail.com)
	SPDX-FileCopyrightText: 2023 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "HypothesisTest.h"
#include "backend/core/column/Column.h"
#include "backend/statistics/MyTableModel.h"
#include "backend/lib/macros.h"
#include "backend/nsl/nsl_stats.h"

#include <QLabel>
#include <KLocalizedString>

#include <gsl/gsl_cdf.h>
#include <gsl/gsl_math.h>

HypothesisTest::HypothesisTest(const QString &name) : GeneralTest(name, AspectType::HypothesisTest) {
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

void HypothesisTest::test(int test, bool categoricalVariable, bool equalVariance, bool calculateStats) {
	m_pValue.clear();
	m_statisticValue.clear();
	for (int i = 0; i < RESULTLINESCOUNT; i++) {
		m_resultLine[i]->clear();
		m_resultLine[i]->setToolTip(QString());
	}

	if (calculateStats)
		m_inputStatsTableModel->clear();

	switch (testSubtype(test)) {
	case TwoSampleIndependent: {
		m_currTestName = QLatin1String("<h2>") + i18n("Two Sample Independent Test") + QLatin1String("</h2>");
		performTwoSampleIndependentTest(testType(test), categoricalVariable, equalVariance, calculateStats);
		break;
	}
	case TwoSamplePaired:
		m_currTestName = QLatin1String("<h2>") + i18n("Two Sample Paired Test") + QLatin1String("</h2>");
		performTwoSamplePairedTest(testType(test));
		break;
	case OneSample: {
		m_currTestName = QLatin1String("<h2>") + i18n("One Sample Test") + QLatin1String("</h2>");
		performOneSampleTest(testType(test));
		break;
	}
	case OneWay: {
		m_currTestName = QLatin1String("<h2>") + i18n("One Way Anova") + QLatin1String("</h2>");
		performOneWayAnova();
		break;
	}
	case TwoWay: {
		m_currTestName = QLatin1String("<h2>") + i18n("Two Way Anova") + QLatin1String("</h2>");
		performTwoWayAnova();
		break;
	}
	}

	emit changed();
}

void HypothesisTest::leveneTest(bool categoricalVariable) {
	m_pValue.clear();
	m_statisticValue.clear();
	m_statsTable = QString();
	for (int i = 0; i < RESULTLINESCOUNT; i++) {
		m_resultLine[i]->clear();
		m_resultLine[i]->setToolTip(QString());
	}

	m_currTestName = QLatin1String("<h2>") + i18n("Levene Test for Equality of Variance") + QLatin1String("</h2>");
	performLeveneTest(categoricalVariable);
	emit changed();
}

void HypothesisTest::initInputStatsTable(int test, bool calculateStats) {
	m_inputStatsTableModel->clear();

	if (!calculateStats) {
		if (testSubtype(test) == TwoSampleIndependent) {
			m_inputStatsTableModel->setRowCount(3);
			m_inputStatsTableModel->setColumnCount(5);

			m_inputStatsTableModel->setData(m_inputStatsTableModel->index(0, 1), i18n("N"));
			m_inputStatsTableModel->setData(m_inputStatsTableModel->index(0, 2), i18n("Sum"));
			m_inputStatsTableModel->setData(m_inputStatsTableModel->index(0, 3), i18n("Mean"));
			m_inputStatsTableModel->setData(m_inputStatsTableModel->index(0, 4), i18n("Standard Deviation"));

			for (int i = 1; i < 5; i++)
				m_inputStatsTableModel->item(0, i)->setEditable(false);

			m_inputStatsTableModel->setData(m_inputStatsTableModel->index(1, 0), i18n("Row 1"));
			m_inputStatsTableModel->setData(m_inputStatsTableModel->index(2, 0), i18n("Row 2"));
		}
	}

	emit changed();
}

QList<double>& HypothesisTest::statisticValue() {
	return m_statisticValue;
}

QList<double>& HypothesisTest::pValue() {
	return m_pValue;
}

/******************************************************************************
 *                      Private Implementations
 * ****************************************************************************/
//TODO: backend of z test;
//TODO: use https://www.gnu.org/software/gsl/doc/html/statistics.html for basic statistic calculations

/**************************Two Sample Independent *************************************/
// program is crashing on some input.
void HypothesisTest::performTwoSampleIndependentTest(int test, bool categoricalVariable, bool equalVariance, bool calculateStats) {
	int n[2];
	double sum[2], mean[2], std[2];
	QString col1Name;
	QString col2Name;

	if (!calculateStats) {
		//QDEBUG("m_inputStatsTable row and column count " << m_inputStatsTableModel->rowCount() << m_inputStatsTableModel->columnCount());

		for (int i = 1; i < 3; i++) {
			n[i - 1] = m_inputStatsTableModel->data(m_inputStatsTableModel->index(i, 1)).toInt();
			sum[i - 1] = m_inputStatsTableModel->data(m_inputStatsTableModel->index(i, 2)).toDouble();
			mean[i - 1] = m_inputStatsTableModel->data(m_inputStatsTableModel->index(i, 3)).toDouble();
			std[i - 1] = m_inputStatsTableModel->data(m_inputStatsTableModel->index(i, 4)).toDouble();

			if (sum[i] == 0.0)
				sum[i] = mean[i] * n[i];

			if (mean[i] == 0.0 && n[i] > 0)
				mean[i] = sum[i] / n[i];
		}
		col1Name = m_inputStatsTableModel->data(m_inputStatsTableModel->index(1, 0)).toString();
		col2Name = m_inputStatsTableModel->data(m_inputStatsTableModel->index(2, 0)).toString();
	} else {
		if (m_columns.size() != 2) {
			printError(i18n("Inappropriate number of columns selected"));
			return;
		}

		col1Name = m_columns[0]->name();
		col2Name = m_columns[1]->name();

		if (!categoricalVariable && m_columns[0]->isNumeric()) {
			for (int i = 0; i < 2; i++) {
				findStats(m_columns[i], n[i], sum[i], mean[i], std[i]);
				if (n[i] == 0) {
					printError(i18n("At least two values should be there in every column"));
					return;
				}
				if (std[i] == 0.0) {
					printError(i18n("Standard Deviation of at least one column is equal to 0: last column is: %1", m_columns[i]->name()));
					return;
				}
			}
		} else {
			QMap<QString, int> colName;
			QString baseColName;
			int np;
			int totalRows;

			countPartitions(m_columns[0], np, totalRows);
			if (np != 2) {
				printError( i18n("Number of Categorical Variable in Column %1 is not equal to 2", m_columns[0]->name()));
				return;
			}

			if (m_columns[0]->isNumeric())
				baseColName = m_columns[0]->name();

			GeneralErrorType errorCode = findStatsCategorical(m_columns[0], m_columns[1], n, sum, mean, std, colName, np, totalRows);

			switch (errorCode) {
			case ErrorUnqualSize: {
				printError( i18n("Unequal size between Column %1 and Column %2", m_columns[0]->name(), m_columns[1]->name()));
				return;
			}
			case ErrorEmptyColumn: {
				printError(i18n("At least one of selected column is empty"));
				return;
			}
			case NoError:
				break;
			}

			QMapIterator<QString, int> iter(colName);
			while (iter.hasNext()) {
				iter.next();
				if (iter.value() == 1)
					col1Name = baseColName + QLatin1String(" ") + iter.key();
				else
					col2Name = baseColName + QLatin1String(" ") + iter.key();
			}
		}
	}

	QVariant rowMajor[] = {QString(), QLatin1String("N"), QLatin1String("Sum"), QLatin1String("Mean"), QLatin1String("Std"),
						   col1Name, n[0], sum[0], mean[0], std[0],
						   col2Name, n[1], sum[1], mean[1], std[1]
						  };

	m_statsTable = getHtmlTable(3, 5, rowMajor);

	for (int i = 0; i < 2; i++) {
		if (n[i] == 0) {
			if (calculateStats)
				printError(i18n("At least two values should be there in every column"));
			else
				printError(i18n("In Statistic Table you filled above, value of N for every row should be > 0"));
			return;
		}
		if (std[i] == 0.0) {
			printError( i18n("Standard Deviation of at least one column is equal to 0: last column is: %1", m_columns[i]->name()));
			return;
		}
	}

	double stdSq[2] = {gsl_pow_2(std[0]), gsl_pow_2(std[1])};

	QString testName;
	int df = 0;
	double spSq = 0;

	switch (testType(test)) {
	case TTest: {
		testName = QLatin1String("T");

		if (equalVariance) {
			df = n[0] + n[1] - 2;

			spSq = ((n[0]-1) * stdSq[0] +
					(n[1]-1) * stdSq[1] ) / df;
			//				QDEBUG("equal variance : spSq is " << spSq);
			m_statisticValue.append((mean[0] - mean[1]) / sqrt(spSq / n[0] + spSq / n[1]));
			printLine(9, i18n("<b>Assumption:</b> Equal Variance b/w both population means"));
		} else {
			double temp_val;
			temp_val = gsl_pow_2( gsl_pow_2(std[0]) / n[0] + gsl_pow_2(std[1]) / n[1]);
			temp_val = temp_val / ( (gsl_pow_2( (gsl_pow_2(std[0]) / n[0]) ) / (n[0]-1)) +
					(gsl_pow_2( (gsl_pow_2(std[1]) / n[1]) ) / (n[1]-1)));
			df = qRound(temp_val);

			m_statisticValue.append((mean[0] - mean[1]) / (sqrt( (gsl_pow_2(std[0])/n[0]) +
					(gsl_pow_2(std[1])/n[1]))));
			printLine(9, i18n("<b>Assumption:</b> UnEqual Variance b/w both population means"));
		}

		printLine(6, i18n("Degree of Freedom is %1", df), QLatin1String("green"));
		printTooltip(6, i18n("Number of independent Pieces of information that went into calculating the estimate"));

		printLine(8, i18n("<b>Assumption:</b> Both Populations approximately follow normal distribution"));
		break;
	}
	case ZTest: {
		testName = QLatin1String("Z");
		spSq = stdSq[0] / n[0] + stdSq[1] / n[1];

		double zValue = (mean[0] - mean[1]) / sqrt(spSq);
		m_statisticValue.append(zValue);
		break;
	}
	}

	m_currTestName = QLatin1String("<h2>") + i18n("Two Sample Independent %1 Test for %2 vs %3", testName, col1Name, col2Name) + QLatin1String("</h2>");
	m_pValue.append(getPValue(test, m_statisticValue[0], col1Name, col2Name, df));

	printLine(2, i18n("Significance level is %1", round(m_significanceLevel)), QLatin1String("blue"));

	printLine(4, i18n("%1 Value is %2 ", testName, round(m_statisticValue[0])), QLatin1String("green"));
	printTooltip(4, i18n("More is the |%1-value|, more safely we can reject the null hypothesis", testName));

	printLine(5, i18n("P Value is %1 ", m_pValue[0]), QLatin1String("green"));
	printTooltip(5, getPValueTooltip(m_pValue[0]));
}

/********************************Two Sample Paired ***************************************/

void HypothesisTest::performTwoSamplePairedTest(int test) {
	if (m_columns.size() != 2) {
		printError(i18n("Inappropriate number of m_columns selected"));
		return;
	}

	for (int i = 0; i < 2; i++) {
		if ( !m_columns[0]->isNumeric()) {
			printError(i18n("select only columns with numbers"));
			return;
		}
	}

	int n;
	double sum, mean, std;
	GeneralErrorType errorCode = findStatsPaired(m_columns[0], m_columns[1], n, sum, mean, std);

	switch (errorCode) {
	case ErrorUnqualSize: {
		printError(i18n("both columns have different sizes"));

		return;
	}
	case ErrorEmptyColumn: {
		printError(i18n("columns are empty"));
		return;
	}
	case NoError:
		break;
	}

	QVariant rowMajor[] = {QString(), QLatin1String("N"), QLatin1String("Sum"), QLatin1String("Mean"), QLatin1String("Std"),
						   QLatin1String("difference"), n, sum, mean, std
						  };

	m_statsTable = getHtmlTable(2, 5, rowMajor);

	if (std == 0.0) {
		printError(i18n("Standard deviation of the difference is 0"));
		return;
	}


	QString testName;
	int df = 0;

	switch (test) {
	case TTest: {
		m_statisticValue[0] = mean / (std / sqrt(n));
		df = n - 1;
		testName = QLatin1String("T");
		printLine(6, i18n("Degree of Freedom is %1</p", df), QLatin1String("green"));
		break;
	}
	case ZTest: {
		testName = QLatin1String("Z");
		m_statisticValue[0] = mean / (std / sqrt(n));
		df = n - 1;
		break;
	}
	}

	m_pValue.append(getPValue(test, m_statisticValue[0], m_columns[0]->name(), i18n("%1", m_populationMean), df));
	m_currTestName = QLatin1String("<h2>") + i18n("One Sample %1 Test for %2 vs %3", testName, m_columns[0]->name(), m_columns[1]->name()) + QLatin1String("</h2>");

	printLine(2, i18n("Significance level is %1 ", round(m_significanceLevel)), QLatin1String("blue"));
	printLine(4, i18n("%1 Value is %2 ", testName, round(m_statisticValue[0])), QLatin1String("green"));
	printLine(5, i18n("P Value is %1 ", m_pValue[0]), QLatin1String("green"));

	printTooltip(5, getPValueTooltip(m_pValue[0]));
	return;
}

/******************************** One Sample ***************************************/

void HypothesisTest::performOneSampleTest(int test) {
	if (m_columns.size() != 1) {
		printError(i18n("Inappropriate number of m_columns selected"));
		return;
	}

	if ( !m_columns[0]->isNumeric()) {
		printError(i18n("select only m_columns with numbers"));
		return;
	}

	int n;
	double sum, mean, std;
	GeneralErrorType errorCode = findStats(m_columns[0], n, sum, mean, std);

	switch (errorCode) {
	case ErrorEmptyColumn: {
		printError(i18n("column is empty"));
		return;
	}
	case NoError:
		break;
	case ErrorUnqualSize: {
		return;
	}
	}

	QVariant rowMajor[] = {QString(), QLatin1String("N"), QLatin1String("Sum"), QLatin1String("Mean"), QLatin1String("Std"),
						   m_columns[0]->name(), n, sum, mean, std
						  };

	m_statsTable = getHtmlTable(2, 5, rowMajor);

	if (std == 0.0) {
		printError(i18n("Standard deviation is 0"));
		return;
	}

	QString testName;
	int df = 0;

	switch (test) {
	case TTest: {
		testName = QLatin1String("T");
		m_statisticValue.append((mean - m_populationMean) / (std / sqrt(n)));
		df = n - 1;
		printLine(6, i18n("Degree of Freedom is %1", df), QLatin1String("blue"));
		break;
	}
	case ZTest: {
		testName = QLatin1String("Z");
		df = 0;
		m_statisticValue.append((mean - m_populationMean) / (std / sqrt(n)));
		break;
	}
	}

	m_pValue.append(getPValue(test, m_statisticValue[0], m_columns[0]->name(), i18n("%1",m_populationMean), df));
	m_currTestName = QLatin1String("<h2>") + i18n("One Sample %1 Test for %2", testName, m_columns[0]->name()) + QLatin1String("</h2>");

	printLine(2, i18n("Significance level is %1", round(m_significanceLevel)), QLatin1String("blue"));
	printLine(4, i18n("%1 Value is %2", testName, round(m_statisticValue[0])), QLatin1String("green"));
	printLine(5, i18n("P Value is %1", m_pValue[0]), QLatin1String("green"));

	printTooltip(5, getPValueTooltip(m_pValue[0]));
	return;
}

/*************************************One Way Anova***************************************/
// all standard variables and formulas are taken from this wikipedia page:
// https://en.wikipedia.org/wiki/One-way_analysis_of_variance
// b stands for b/w groups
// w stands for within groups
// np is number of partition i.e., number of classes
void HypothesisTest::performOneWayAnova() {
	int np, totalRows;
	countPartitions(m_columns[0], np, totalRows);

	int* ni = new int[np];
	double* sum = new double[np];
	double* mean = new double[np];
	double* std = new double[np];
	QString* colNames = new QString[np];

	QMap<QString, int> classnameToIndex;
	QString baseColName;

	if (m_columns[0]->isNumeric())
		baseColName = m_columns[0]->name();

	findStatsCategorical(m_columns[0], m_columns[1], ni, sum, mean, std, classnameToIndex, np, totalRows);

	double yBar = 0;		// overall mean
	double sB = 0;			// sum of squares of (mean - overall_mean) between the groups
	int fB = 0;			// degree of freedom between the groups
	double msB = 0;		// mean sum of squares between the groups
	double sW = 0;			// sum of squares of (value - mean of group) within the groups
	int fW = 0;			// degree of freedom within the group
	double msW = 0;		// mean sum of squares within the groups

	// now finding mean of each group;

	for (int i = 0; i < np; i++)
		yBar += mean[i];
	yBar /= np;

	for (int i = 0; i < np; i++) {
		sB += ni[i] * gsl_pow_2(mean[i] - yBar);
		if (ni[i] > 1)
			sW += gsl_pow_2(std[i])*(ni[i] - 1);
		else
			sW += gsl_pow_2(std[i]);
		fW += ni[i] - 1;
	}

	fB = np - 1;
	msB = sB / fB;

	msW = sW / fW;
	m_statisticValue.append(msB / msW);

	m_pValue.append(nsl_stats_fdist_p(m_statisticValue[0], static_cast<size_t>(np-1), fW));

	QMapIterator<QString, int> i(classnameToIndex);
	while (i.hasNext()) {
		i.next();
		colNames[i.value()-1] = baseColName + QLatin1String(" ") + i.key();
	}

	// now printing the statistics and result;
	int rowCount = np + 1, columnCount = 5;
	QVariant* rowMajor = new QVariant[rowCount*columnCount];
	// header data;
	rowMajor[0] = QString();
	rowMajor[1] = QLatin1String("Ni");
	rowMajor[2] = QLatin1String("Sum");
	rowMajor[3] = QLatin1String("Mean");
	rowMajor[4] = QLatin1String("Std");

	// table data
	for (int row_i = 1; row_i < rowCount ; row_i++) {
		rowMajor[row_i*columnCount]		= colNames[row_i - 1];
		rowMajor[row_i*columnCount + 1]	= ni[row_i - 1];
		rowMajor[row_i*columnCount + 2]	= sum[row_i - 1];
		rowMajor[row_i*columnCount + 3]	= mean[row_i - 1];
		rowMajor[row_i*columnCount + 4]	= std[row_i - 1];
	}

	m_statsTable = QLatin1String("<h3>") + i18n("Group Summary Statistics") + QLatin1String("</h3>");

	m_statsTable += getHtmlTable(rowCount, columnCount, rowMajor);

	m_statsTable += getLine(QString());
	m_statsTable += getLine(QString());
	m_statsTable += QLatin1String("<h3>") + i18n("Grand Summary Statistics") + QLatin1String("</h3>");
	m_statsTable += getLine(QString());
	m_statsTable += getLine(i18n("Overall Mean is %1", round(yBar)));

	rowCount = 4;
	columnCount = 3;
	rowMajor->clear();

	rowMajor[0] = QString();
	rowMajor[1] = i18n("Between Groups");
	rowMajor[2] = i18n("Within Groups");

	int baseIndex = 0;
	baseIndex = 1 * columnCount;
	rowMajor[baseIndex + 0] = i18n("Sum of Squares");
	rowMajor[baseIndex + 1] = sB;
	rowMajor[baseIndex + 2] = sW;
	baseIndex = 2 * columnCount;
	rowMajor[baseIndex + 0] = i18n("Degree of Freedom");
	rowMajor[baseIndex + 1] = fB;
	rowMajor[baseIndex + 2] = fW;
	baseIndex = 3 * columnCount;
	rowMajor[baseIndex + 0] = i18n("Mean Square Value");
	rowMajor[baseIndex + 1] = msB;
	rowMajor[baseIndex + 2] = msW;

	m_statsTable += getHtmlTable(rowCount, columnCount, rowMajor);

	delete[] ni;
	delete[] sum;
	delete[] mean;
	delete[] std;
	delete[] colNames;

	printLine(1, i18n("F Value is %1", round(m_statisticValue[0])), QLatin1String("green"));
	printLine(2, i18n("P Value is %1 ", m_pValue[0]), QLatin1String("green"));

	printTooltip(2, getPValueTooltip(m_pValue[0]));
}

/*************************************Two Way Anova***************************************/

// all formulas and symbols are taken from: http://statweb.stanford.edu/~susan/courses/s141/exanova.pdf

//TODO: suppress warning of variable length array are a C99 feature.
//TODO: add assumptions verification option
//TODO: add tail option (if needed)
void HypothesisTest::performTwoWayAnova() {
	int np_a, totalRows_a;
	int np_b, totalRows_b;
	countPartitions(m_columns[0], np_a, totalRows_a);
	countPartitions(m_columns[1], np_b, totalRows_b);

	QVector<QVector<double>> groupMean(np_a, QVector<double>(np_b));
	QVector<QVector<int>> replicates(np_a, QVector<int>(np_b));

	for (int i = 0; i < np_a; i++)
		for (int j = 0; j < np_b; j++) {
			groupMean[i][j] = 0;
			replicates[i][j] = 0;
		}

	if (totalRows_a != totalRows_b) {
		printError(i18n("There is missing data in at least one of the rows"));
		return;
	}

	QMap<QString, int> catToNumber_a;
	QMap<QString, int> catToNumber_b;

	int partitionNumber_a = 1;
	int partitionNumber_b = 1;
	for (int i = 0; i < totalRows_a; i++) {
		QString name_a = m_columns[0]->textAt(i);
		QString name_b = m_columns[1]->textAt(i);
		double value = m_columns[2]->valueAt(i);

		if (catToNumber_a[name_a] == 0) {
			catToNumber_a[name_a] = partitionNumber_a;
			partitionNumber_a++;
		}

		if (catToNumber_b[name_b] == 0) {
			catToNumber_b[name_b] = partitionNumber_b;
			partitionNumber_b++;
		}

		groupMean[catToNumber_a[name_a] - 1][catToNumber_b[name_b] - 1] += value;
		replicates[catToNumber_a[name_a] - 1][catToNumber_b[name_b] - 1] += 1;
	}

	int replicate = replicates[0][0];
	for (int i = 0; i < np_a; i++)
		for (int j = 0; j < np_b; j++) {
			if (replicates[i][j] == 0) {
				printError(i18n("Dataset should have at least one data value corresponding to each feature combination"));
				return;
			}
			if (replicates[i][j] != replicate) {
				printError(i18n("Number of experiments perfomed for each combination of levels <br/>"
						   "between Independet Var.1 and Independent Var.2 must be equal"));
				return;
			}
			groupMean[i][j] /= replicates[i][j];
		}

	double ss_within = 0;
	for (int i = 0; i < totalRows_a; i++) {
		QString name_a = m_columns[0]->textAt(i);
		QString name_b = m_columns[1]->textAt(i);
		double value = m_columns[2]->valueAt(i);

		ss_within += gsl_pow_2(value - groupMean[catToNumber_a[name_a] - 1][catToNumber_b[name_b] - 1]);
	}

	int df_within = (replicate - 1) * np_a * np_b;
	double ms_within = ss_within / df_within;

	double* mean_a = new double[np_a];
	double* mean_b = new double[np_b];
	for (int i = 0; i < np_a; i++) {
		for (int j = 0; j < np_b; j++) {
			mean_a[i] += groupMean[i][j] / np_b;
			mean_b[j] += groupMean[i][j] / np_a;
		}
	}

	double mean = 0;
	for (int i = 0; i < np_a; i++)
		mean += mean_a[i] / np_a;


	double ss_a = 0;
	for (int i = 0; i < np_a; i++)
		ss_a += gsl_pow_2(mean_a[i] - mean);
	ss_a *= replicate * np_b;

	int df_a = np_a - 1;
	double ms_a = ss_a / df_a;

	double ss_b = 0;
	for (int i = 0; i < np_b; i++)
		ss_b += gsl_pow_2(mean_b[i] - mean);
	ss_b *= replicate * np_a;

	int df_b = np_b - 1;
	double ms_b = ss_b / df_b;

	double ss_interaction = 0;

	for (int i = 0; i < np_a; i++)
		for (int j = 0; j < np_b; j++)
			ss_interaction += gsl_pow_2(groupMean[i][j] - mean_a[i] - mean_b[j] + mean);

	ss_interaction *= replicate;
	int df_interaction = (np_a - 1) * (np_b - 1);
	double ms_interaction = ss_interaction / df_interaction;

	QString* partitionNames_a = new QString[np_a];
	QString* partitionNames_b = new QString[np_b];

	QMapIterator<QString, int> itr_a(catToNumber_a);
	while (itr_a.hasNext()) {
		itr_a.next();
		partitionNames_a[itr_a.value()-1] = itr_a.key();
	}

	QMapIterator<QString, int> itr_b(catToNumber_b);
	while (itr_b.hasNext()) {
		itr_b.next();
		partitionNames_b[itr_b.value()-1] = itr_b.key();
	}

	// printing table;
	// HtmlCell constructor structure; data, level, rowSpanCount, m_columnspanCount, isHeader;
	QList<HtmlCell*> rowMajor;
	rowMajor.append(new HtmlCell(QString(), 0, true, QString(), 2, 1));
	for (int i = 0; i < np_b; i++)
		rowMajor.append(new HtmlCell(partitionNames_b[i], 0, true, QString(), 1, 2));
	rowMajor.append(new HtmlCell(i18n("Mean"), 0, true, QString(), 2));

	for (int i = 0; i < np_b; i++) {
		rowMajor.append(new HtmlCell(i18n("Mean"), 1, true));
		rowMajor.append(new HtmlCell(i18n("Replicate"), 1, true));
	}

	int level = 2;
	for (int i = 0; i < np_a; i++) {
		rowMajor.append(new HtmlCell(partitionNames_a[i], level, true));
		for (int j = 0; j < np_b; j++) {
			rowMajor.append(new HtmlCell(round(groupMean[i][j]), level));
			rowMajor.append(new HtmlCell(replicates[i][j], level));
		}
		rowMajor.append(new HtmlCell(round(mean_a[i]), level));
		level++;
	}

	rowMajor.append(new HtmlCell(i18n("Mean"), level, true));
	for (int i = 0; i < np_b; i++)
		rowMajor.append(new HtmlCell(round(mean_b[i]), level, false, QString(), 1, 2));
	rowMajor.append(new HtmlCell(round(mean), level));

	m_statsTable = QLatin1String("<h3>") + i18n("Contingency Table") + QLatin1String("</h3>");
	m_statsTable += getHtmlTable3(rowMajor);

	m_statsTable += QLatin1String("</br>");
	m_statsTable += QLatin1String("<h3>") + i18n("results table") + QLatin1String("</h3>");

	rowMajor.clear();
	level = 0;
	rowMajor.append(new HtmlCell(QString(), level, true));
	rowMajor.append(new HtmlCell(QLatin1String("SS"), level, true));
	rowMajor.append(new HtmlCell(QLatin1String("DF"), level, true, i18n("degree of freedom")));
	rowMajor.append(new HtmlCell(QLatin1String("MS"), level, true));

	level++;
	rowMajor.append(new HtmlCell(m_columns[0]->name(), level, true));
	rowMajor.append(new HtmlCell(round(ss_a), level));
	rowMajor.append(new HtmlCell(df_a, level));
	rowMajor.append(new HtmlCell(round(ms_a), level));

	level++;
	rowMajor.append(new HtmlCell(m_columns[1]->name(), level, true));
	rowMajor.append(new HtmlCell(round(ss_b), level));
	rowMajor.append(new HtmlCell(df_b, level));
	rowMajor.append(new HtmlCell(round(ms_b), level));

	level++;
	rowMajor.append(new HtmlCell(i18n("Interaction"), level, true));
	rowMajor.append(new HtmlCell(round(ss_interaction), level));
	rowMajor.append(new HtmlCell(df_interaction, level));
	rowMajor.append(new HtmlCell(round(ms_interaction), level));

	level++;
	rowMajor.append(new HtmlCell(i18n("Within"), level, true));
	rowMajor.append(new HtmlCell(round(ss_within), level));
	rowMajor.append(new HtmlCell(df_within, level));
	rowMajor.append(new HtmlCell(round(ms_within), level));

	m_statsTable += getHtmlTable3(rowMajor);

	double fValue_a = ms_a / ms_within;
	double fValue_b = ms_b / ms_within;
	double fValue_interaction = ms_interaction / ms_within;

	double m_pValue_a = nsl_stats_fdist_p(fValue_a, static_cast<size_t>(np_a - 1), df_a);
	double m_pValue_b = nsl_stats_fdist_p(fValue_b, static_cast<size_t>(np_b - 1), df_b);

	printLine(0, QLatin1String("F(df<sub>") + m_columns[0]->name() + i18n("</sub>, df<sub>within</sub>) is ") + round(fValue_a), QLatin1String("blue"));
	printLine(1, QLatin1String("F(df<sub>") + m_columns[1]->name() + i18n("</sub>, df<sub>within</sub>) is ") + round(fValue_b), QLatin1String("blue"));
	printLine(2, QLatin1String("F(df<sub>interaction</sub>, df<sub>within</sub>) is ") + round(fValue_interaction), QLatin1String("blue"));

	printLine(4, QLatin1String("P(df<sub>") + m_columns[0]->name() + i18n("</sub>, df<sub>within</sub>) is ") + round(m_pValue_a), QLatin1String("blue"));
	printLine(5, QLatin1String("P(df<sub>") + m_columns[1]->name() + i18n("</sub>, df<sub>within</sub>) is ") + round(m_pValue_b), QLatin1String("blue"));
	//    printLine(2, "P(df<sub>interaction</sub>, df<sub>within</sub>) is " + round(fValue_interaction), "blue");

	m_statisticValue.append(fValue_a);
	m_statisticValue.append(fValue_b);
	m_statisticValue.append(fValue_interaction);

	m_pValue.append(m_pValue_a);
	m_pValue.append(m_pValue_b);

	delete[] mean_a;
	delete[] mean_b;
	delete[] partitionNames_a;
	delete[] partitionNames_b;

	return;
}

/**************************************Levene Test****************************************/
// Some reference to local variables.
// np = number of partitions
// df = degree of fredom
// totalRows = total number of rows in column

// these variables are taken from: https://en.wikipedia.org/wiki/Levene%27s_test
// yiBar		= mean of ith group;
// Zij			= |Yij - yiBar|
// ziBar		= mean of Zij for group i
// ziBarBar	= mean for all zij
// ni			= number of elements in group i
void HypothesisTest::performLeveneTest(bool categoricalVariable) {
	if (m_columns.size() != 2) {
		printError(i18n("Inappropriate number of columns selected"));
		return;
	}

	int np = 0;
	int n = 0;

	if (!categoricalVariable && m_columns[0]->isNumeric())
		np = m_columns.size();
	else
		countPartitions(m_columns[0], np, n);

	if (np < 2) {
		printError(i18n("Select at least two columns / classes"));
		return;
	}

	double* yiBar = new double[np];
	double* ziBar = new double[np];
	double ziBarBar = 0;
	double* ni = new double[np];

	for (int i = 0; i < np; i++) {
		yiBar[i] = 0;
		ziBar[i] = 0;
		ni[i] = 0;
	}

	double fValue;
	int df = 0;

	int totalRows = 0;

	QString* colNames = new QString[np];
	if (!categoricalVariable && m_columns[0]->isNumeric()) {
		totalRows = m_columns[0]->rowCount();

		double value = 0;
		for (int j = 0; j < totalRows; j++) {
			int numberNaNCols = 0;
			for (int i = 0; i < np; i++) {
				value = m_columns[i]->valueAt(j);
				if (std::isnan(value)) {
					numberNaNCols++;
					continue;
				}
				yiBar[i] += value;
				ni[i]++;
				n++;
			}
			if (numberNaNCols == np) {
				totalRows = j;
				break;
			}
		}

		for (int i = 0; i < np; i++) {
			if (ni[i] > 0)
				yiBar[i] /= ni[i];
			else {
				printError(i18n("One of the selected m_columns is empty <br/> "
						   "or have choosen Independent Var.1 wrongly"));
				return;
			}
		}

		for (int j = 0; j < totalRows; j++) {
			for (int i = 0; i < np; i++) {
				value = m_columns[i]->valueAt(j);
				if (!(std::isnan(value)))
					ziBar[i] += fabs(value - yiBar[i]);
			}
		}

		for (int i = 0; i < np; i++) {
			ziBarBar += ziBar[i];
			if (ni[i] > 0)
				ziBar[i] /= ni[i];
		}

		ziBarBar /= n;

		double numberatorValue = 0;
		double denominatorValue = 0;

		for (int j = 0; j < totalRows; j++) {
			for (int i = 0; i < np; i++) {
				value = m_columns[i]->valueAt(j);
				if (!(std::isnan(value))) {
					double zij = fabs(value - yiBar[i]);
					denominatorValue += gsl_pow_2( (zij - ziBar[i]));
				}
			}
		}


		if (denominatorValue == 0.0) {
			printError( i18n("Denominator value is %1", denominatorValue));
			return;
		}

		for (int i = 0; i < np; i++) {
			colNames[i] = m_columns[i]->name();
			numberatorValue += ni[i]*gsl_pow_2( (ziBar[i]-ziBarBar));
		}

		fValue = ((n - np) / (np - 1)) * (numberatorValue / denominatorValue);

	} else {
		QMap<QString, int> classnameToIndex;

		AbstractColumn::ColumnMode originalColMode = m_columns[0]->columnMode();
		m_columns[0]->setColumnMode(AbstractColumn::ColumnMode::Text);

		int partitionNumber = 1;
		QString name;
		double value;
		int classIndex;

		for (int j = 0; j < n; j++) {
			name = m_columns[0]->textAt(j);
			value = m_columns[1]->valueAt(j);
			if (std::isnan(value)) {
				n = j;
				break;
			}

			if (classnameToIndex[name] == 0) {
				classnameToIndex[name] = partitionNumber;
				partitionNumber++;
			}

			classIndex = classnameToIndex[name]-1;
			ni[classIndex]++;
			yiBar[classIndex] += value;
		}

		for (int i = 0; i < np; i++) {
			if (ni[i] > 0)
				yiBar[i] /= ni[i];
			else {
				printError(i18n("One of the selected m_columns is empty <br/> "
						   "or have choosen Independent Var.1 wrongly"));
				m_columns[0]->setColumnMode(originalColMode);
				return;
			}
		}

		for (int j = 0; j < n; j++) {
			name = m_columns[0]->textAt(j);
			value = m_columns[1]->valueAt(j);
			classIndex = classnameToIndex[name] - 1;
			ziBar[classIndex] += fabs(value - yiBar[classIndex]);
		}

		for (int i = 0; i < np; i++) {
			ziBarBar += ziBar[i];
			ziBar[i] /= ni[i];
		}

		ziBarBar /= n;

		double numberatorValue = 0;
		double denominatorValue = 0;

		for (int j = 0; j < n; j++) {
			name = m_columns[0]->textAt(j);
			value = m_columns[1]->valueAt(j);
			classIndex = classnameToIndex[name] - 1;
			double zij = fabs(value - yiBar[classIndex]);
			denominatorValue +=  gsl_pow_2( (zij - ziBar[classIndex]));
		}

		for (int i = 0; i < np; i++)
			numberatorValue += ni[i]*gsl_pow_2( (ziBar[i]-ziBarBar));

		if (denominatorValue == 0.0) {
			printError(i18n("number of data points is less or than equal to number of categorical variables"));
			m_columns[0]->setColumnMode(originalColMode);
			return;
		}

		fValue = ((n - np) / (np - 1)) * (numberatorValue / denominatorValue);

		QMapIterator<QString, int> i(classnameToIndex);
		while (i.hasNext()) {
			i.next();
			colNames[i.value()-1] = m_columns[0]->name() + QLatin1String(" ") + i.key();
		}

		m_columns[0]->setColumnMode(originalColMode);
	}

	df = n - np;

	// now making the stats table.
	int rowCount = np+1;
	int columnCount = 4;

	QVariant* rowMajor = new QVariant[rowCount*columnCount];
	// header data;
	rowMajor[0] = QString();
	rowMajor[1] = QLatin1String("Ni");
	rowMajor[2] = QLatin1String("yiBar");
	rowMajor[3] = QLatin1String("ziBar");

	// table data
	for (int row_i = 1; row_i < rowCount; row_i++) {
		rowMajor[row_i*columnCount] = colNames[row_i-1];
		rowMajor[row_i*columnCount + 1] = ni[row_i-1];
		rowMajor[row_i*columnCount + 2] = yiBar[row_i-1];
		rowMajor[row_i*columnCount + 3] = ziBar[row_i-1];
	}

	m_statsTable = getHtmlTable(rowCount, columnCount, rowMajor);

	delete[] rowMajor;
	delete[] yiBar;
	delete[] ziBar;
	delete[] ni;

	m_pValue.append(nsl_stats_fdist_p(fValue, static_cast<size_t>(np-1), df));

	printLine(0, i18n("Null Hypothesis: Variance is equal between all classes"), QLatin1String("blue"));
	printLine(1, i18n("Alternate Hypothesis: Variance is not equal in at-least one pair of classes"), QLatin1String("blue"));
	printLine(2, i18n("Significance level is %1", round(m_significanceLevel)), QLatin1String("blue"));
	printLine(4, i18n("F Value is %1 ", round(fValue)), QLatin1String("green"));
	printLine(5, i18n("P Value is %1 ", m_pValue[0]), QLatin1String("green"));
	printLine(6, i18n("Degree of Freedom is %1", df), QLatin1String("green"));

	printTooltip(5, getPValueTooltip(m_pValue[0]));

	if (m_pValue[0] <= m_significanceLevel)
		printLine(8, i18n("Requirement for homogeneity is not met"), QLatin1String("red"));
	else
		printLine(8, i18n("Requirement for homogeneity is met"), QLatin1String("green"));

	m_statisticValue.append(fValue);
	return;
}

//TODO change ("⋖") symbol to ("<"), currently macro UTF8_QSTRING is not working properly if used "<" symbol;
// TODO: check for correctness between: for TestZ with TailTwo
//       m_pValue.append(2*gsl_cdf_tdist_P(value, df) v/s
//       m_pValue.append(gsl_cdf_tdis_P(value, df) + gsl_cdf_tdis_P(-value, df);
double HypothesisTest::getPValue(const int& test, double& value, const QString& col1Name, const QString& col2Name, const int df) {
	double pValue = 0;

	QString nullHypothesisSign;
	QString alternateHypothesisSign;

	switch (testType(test)) {
	case TTest: {
		switch (m_tail) {
		case Negative:
			pValue = gsl_cdf_tdist_P(value, df);
			nullHypothesisSign = UTF8_QSTRING("≥");
			alternateHypothesisSign = UTF8_QSTRING("⋖");
			break;
		case Positive:
			value *= -1;
			pValue = gsl_cdf_tdist_P(value, df);
			nullHypothesisSign = UTF8_QSTRING("≤");
			alternateHypothesisSign = UTF8_QSTRING(">");
			break;
		case Two:
			pValue = 2.*gsl_cdf_tdist_P(-fabs(value), df);
			nullHypothesisSign = UTF8_QSTRING("=");
			alternateHypothesisSign = UTF8_QSTRING("≠");
			break;
		}
		break;
	}
	case ZTest: {
		switch (m_tail) {
		case Negative:
			pValue = gsl_cdf_ugaussian_P(value);
			nullHypothesisSign = UTF8_QSTRING("≥");
			alternateHypothesisSign = UTF8_QSTRING("⋖");
			break;
		case Positive:
			value *= -1;
			pValue = gsl_cdf_ugaussian_P(value);
			nullHypothesisSign = UTF8_QSTRING("≤");
			alternateHypothesisSign = UTF8_QSTRING(">");
			break;
		case Two:
			pValue = 2*gsl_cdf_ugaussian_P(-fabs(value));
			nullHypothesisSign = UTF8_QSTRING("=");
			alternateHypothesisSign = UTF8_QSTRING("≠");
			break;
		}
		break;
	}
	}

	printLine(0, i18n("Null Hypothesis: Population mean of %1 %2 Population mean of %3 ", col1Name, nullHypothesisSign, col2Name), QLatin1String("blue"));
	printLine(1, i18n("Alternate Hypothesis: Population mean of %1 %2 Population mean of %3 ", col1Name, alternateHypothesisSign, col2Name), QLatin1String("blue"));

	if (pValue > 1)
		return 1;
	return pValue;
}

QString HypothesisTest::getPValueTooltip(const double &pValue) {
	if (pValue <= m_significanceLevel)
		return i18n("We can safely reject Null Hypothesis for significance level %1", round(m_significanceLevel));

	return i18n("There is a plausibility for Null Hypothesis to be true");
}
