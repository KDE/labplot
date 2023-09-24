/*
	File                 : CorrelationCoefficient.cpp
	Project              : LabPlot
	Description          : Correlation Coefficients/Tests
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2019  Devanshu Agarwal(agarwaldevanshu8@gmail.com)
	SPDX-FileCopyrightText: 2023 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "CorrelationCoefficient.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/lib/macros.h"
#include "backend/generalTest/MyTableModel.h"

#include <QLabel>
#include <KLocalizedString>

#include <gsl/gsl_math.h>
#include <gsl/gsl_statistics.h>
#include <algorithm>

extern "C" {
#include "backend/nsl/nsl_stats.h"
}

CorrelationCoefficient::CorrelationCoefficient(const QString& name) : GeneralTest(name, AspectType::CorrelationCoefficient) {
}

CorrelationCoefficient::~CorrelationCoefficient() {
}

void CorrelationCoefficient::performTest(Method method, bool categoricalVariable, bool calculateStats) {
	m_correlationValue = 0;
	m_statisticValue.clear();
	m_pValue.clear();

	for (int i = 0; i < RESULTLINESCOUNT; i++)
		m_resultLine[i]->clear();

	switch (method) {
	case CorrelationCoefficient::Pearson: {
		m_currTestName = QLatin1String("<h2>") + i18n("Pearson's r Correlation Test") + QLatin1String("</h2>");
		performPearson(categoricalVariable);
		break;
	}
	case CorrelationCoefficient::Kendall:
		m_currTestName = QLatin1String("<h2>") + i18n("Kendall's Rank Correlation Test") + QLatin1String("</h2>");
		performKendall();
		break;
	case CorrelationCoefficient::Spearman: {
		m_currTestName = QLatin1String("<h2>") + i18n("Spearman Correlation Coefficient Test") + QLatin1String("</h2>");
		performSpearman();
		break;
	}
	case CorrelationCoefficient::ChiSquare:
		m_currTestName = QLatin1String("<h2>") + i18n("Chi Square Independence Test") + QLatin1String("</h2>");
		performChiSquareIndpendence(calculateStats);
		break;
	}

	emit changed();
}

void CorrelationCoefficient::initInputStatsTable(int test, bool calculateStats, int nRows, int nColumns) {
	m_inputStatsTableModel->clear();

	if (!calculateStats) {
		m_inputStatsTableModel->setRowCount(nRows + 1);
		m_inputStatsTableModel->setColumnCount(nColumns + 1);
	}

	for (int i = 1; i < nRows + 1; i++)
		m_inputStatsTableModel->setData(m_inputStatsTableModel->index(i, 0), i18n("Row %1", i));

	for (int i = 1; i < nColumns + 1; i++)
		m_inputStatsTableModel->setData(m_inputStatsTableModel->index(0, i), i18n("Column %1", i));

	emit changed();
}

void CorrelationCoefficient::setInputStatsTableNRows(int nRows) {
	int nRows_old = m_inputStatsTableModel->rowCount();
	m_inputStatsTableModel->setRowCount(nRows + 1);

	for (int i = nRows_old; i < nRows + 1; i++)
		m_inputStatsTableModel->setData(m_inputStatsTableModel->index(i, 0), i18n("Row %1", i));
}

void CorrelationCoefficient::setInputStatsTableNCols(int nColumns) {
	int nColumns_old = m_inputStatsTableModel->columnCount();
	m_inputStatsTableModel->setColumnCount(nColumns + 1);

	for (int i = nColumns_old; i < nColumns + 1; i++)
		m_inputStatsTableModel->setData(m_inputStatsTableModel->index(0, i), i18n("Column %1", i));
}

void CorrelationCoefficient::exportStatTableToSpreadsheet() {
	if (m_dataSourceSpreadsheet == nullptr)
		return;

	int rowCount = m_inputStatsTableModel->rowCount();
	int columnCount = m_inputStatsTableModel->columnCount();
	int spreadsheetColCount = m_dataSourceSpreadsheet->columnCount();

	m_dataSourceSpreadsheet->insertColumns(spreadsheetColCount, 3);

	Column* col1 = m_dataSourceSpreadsheet->column(spreadsheetColCount);
	Column* col2 = m_dataSourceSpreadsheet->column(spreadsheetColCount + 1);
	Column* col3 = m_dataSourceSpreadsheet->column(spreadsheetColCount + 2);

	col1->setName(i18n("Independent Var. 1"));
	col2->setName(i18n("Independent Var. 2"));
	col3->setName(i18n("Data Values"));

	col1->setColumnMode(AbstractColumn::ColumnMode::Text);
	col2->setColumnMode(AbstractColumn::ColumnMode::Text);
	col3->setColumnMode(AbstractColumn::ColumnMode::Double);

	int index = 0;
	for (int i = 1; i < rowCount; i++)
		for (int j = 1; j < columnCount; j++) {
			col1->setTextAt(index, m_inputStatsTableModel->data(
								m_inputStatsTableModel->index(i, 0)).toString());

			col2->setTextAt(index, m_inputStatsTableModel->data(
								m_inputStatsTableModel->index(0, j)).toString());

			col3->setValueAt(index, m_inputStatsTableModel->data(
								m_inputStatsTableModel->index(i, j)).toDouble());
			index++;
		}
}

double CorrelationCoefficient::correlationValue() const {
	return m_correlationValue;
}

QList<double> CorrelationCoefficient::statisticValue() const {
	return m_statisticValue;
}

/***************************************************************************************************************************
 *                                        Private Implementations
 * ************************************************************************************************************************/

/*********************************************Pearson r ******************************************************************/
//Formulaes are taken from https://www.statisticssolutions.com/correlation-pearson-kendall-spearman/

// variables:
//  N           = total number of observations
//  sumColx     = sum of values in colx
//  sumSqColx   = sum of square of values in colx
//  sumColxColy = sum of product of values in colx and coly

//TODO: support for col1 is categorical.
//TODO: add tooltip for correlation value result
//TODO: find p value
void CorrelationCoefficient::performPearson(bool categoricalVariable) {
	if (m_columns.count() != 2) {
		printError(i18n("Select only 2 columns"));
		return;
	}

	if (categoricalVariable) {
		printLine(1, i18n("currently categorical variable not supported"), QLatin1String("blue"));
		return;
	}

	auto* col1 = m_columns.at(0);
	auto* col2 = m_columns.at(1);
	QString col1Name = col1->name();
	QString col2Name = col2->name();

	if (!col2->isNumeric())
		printError(i18n("Column %1 should contain only numeric or interger values", col2Name));

	int N = col1->statistics().size;
	if (N != col2->statistics().size) {
		printError(i18n("Number of data values in Column %1 and in Column %2 are not equal", col1Name, col2Name));
		return;
	}

	// calculate the correlation coefficient
	double sumCol1 = findSum(col1, N);
	double sumCol2 = findSum(col2, N);
	double sumSqCol1 = findSumSq(col1, N);
	double sumSqCol2 = findSumSq(col2, N);

	double sumCol12 = 0;
	for (int i = 0; i < N; i++)
		sumCol12 += col1->valueAt(i) * col2->valueAt(i);

	m_correlationValue = (N * sumCol12 - sumCol1*sumCol2) /
			sqrt((N * sumSqCol1 - gsl_pow_2(sumCol1)) *
				 (N * sumSqCol2 - gsl_pow_2(sumCol2)));

	// printing table;
	// HtmlCell constructor structure; data, level, rowSpanCount, m_columnspanCount, isHeader;

	QList<HtmlCell*> rowMajor;
	int level = 0;

	// horizontal header
	QString sigma = UTF8_QSTRING("Σ");
	rowMajor.append(new HtmlCell(QString(), level, true));
	rowMajor.append(new HtmlCell(QLatin1String("N"), level, true, i18n("Total Number of Observations")));
	rowMajor.append(new HtmlCell(QString(sigma + QLatin1String("Scores")), level, true, i18n("Sum of Scores in each column")));
	rowMajor.append(new HtmlCell(QString(sigma + QLatin1String("Scores<sup>2</sup>")), level, true, i18n("Sum of Squares of scores in each column")));
	rowMajor.append(new HtmlCell(QString(sigma + QLatin1String("(") + UTF8_QSTRING("∏") + QLatin1String("Scores)")), level, true, i18n("Sum of product of scores of both columns")));

	//data with vertical header.
	level++;
	rowMajor.append(new HtmlCell(col1Name, level, true));
	rowMajor.append(new HtmlCell(N, level));
	rowMajor.append(new HtmlCell(sumCol1, level));
	rowMajor.append(new HtmlCell(sumSqCol1, level));

	rowMajor.append(new HtmlCell(sumCol12, level, false, QString(), 2, 1));

	level++;
	rowMajor.append(new HtmlCell(col2Name, level, true));
	rowMajor.append(new HtmlCell(N, level));
	rowMajor.append(new HtmlCell(sumCol2, level));
	rowMajor.append(new HtmlCell(sumSqCol2, level));

	m_statsTable = getHtmlTable3(rowMajor);

	printLine(0, i18n("Correlation Value is %1", round(m_correlationValue)), QLatin1String("green"));
}

/***********************************************Kendall ******************************************************************/
// used knight algorithm for fast performance O(nlogn) rather than O(n^2)
// http://adereth.github.io/blog/2013/10/30/efficiently-computing-kendalls-tau/

// TODO: Change date format type to original for numeric type;
// TODO: add tooltips.
// TODO: Compute tauB for ties.
// TODO: find P Value from Z Value
void CorrelationCoefficient::performKendall() {
	if (m_columns.count() != 2) {
		printError(i18n("Select only 2 columns "));
		return;
	}

	auto* col1 = m_columns.at(0);
	auto* col2 = m_columns.at(1);
	QString col1Name = col1->name();
	QString col2Name = col2->name();

	int N = col1->statistics().size;
	if (N != col2->statistics().size) {
		printError(i18n("Number of data values in Column %1 and in Column %2 are not equal", col1Name, col2Name));
		return;
	}

	QVector<int> col2Ranks(N);
	if (col1->isNumeric()) {
		if (col1->isNumeric() && col2->isNumeric()) {
			for (int i = 0; i < N; i++)
				col2Ranks[int(col1->valueAt(i)) - 1] = int(col2->valueAt(i));
		} else {
			printError(i18n("Ranking System should be same for both Column: %1 and Column: %2 <br/>"
							   "Hint: Check for data types of columns", col1Name, col2Name));
			return;
		}
	} else {
		AbstractColumn::ColumnMode origCol1Mode = col1->columnMode();
		AbstractColumn::ColumnMode origCol2Mode = col2->columnMode();

		col1->setColumnMode(AbstractColumn::ColumnMode::Text);
		col2->setColumnMode(AbstractColumn::ColumnMode::Text);

		QMap<QString, int> ValueToRank;

		for (int i = 0; i < N; i++) {
			if (ValueToRank[col1->textAt(i)] != 0) {
				printError(i18n("Currently ties are not supported"));
				col1->setColumnMode(origCol1Mode);
				col2->setColumnMode(origCol2Mode);
				return;
			}
			ValueToRank[col1->textAt(i)] = i + 1;
		}

		for (int i = 0; i < N; i++)
			col2Ranks[i] = ValueToRank[col2->textAt(i)];

		col1->setColumnMode(origCol1Mode);
		col2->setColumnMode(origCol2Mode);
	}

	const int nPossiblePairs = (N * (N - 1)) / 2;
	const int nDiscordant = findDiscordants(col2Ranks.data(), 0, N - 1);
	const int nCorcordant = nPossiblePairs - nDiscordant;
	m_correlationValue = double(nCorcordant - nDiscordant) / nPossiblePairs;

	m_statisticValue.append((3 * (nCorcordant - nDiscordant)) /
							sqrt(N * (N- 1) * (2 * N + 5) / 2));

	printLine(0, i18n("Number of Discordants are %1", nDiscordant), QLatin1String("green"));
	printLine(1, i18n("Number of Concordant are %1", nCorcordant), QLatin1String("green"));

	printLine(2, i18n("Tau a is %1", round(m_correlationValue)), QLatin1String("green"));
	printLine(3, i18n("Z Value is %1", round(m_statisticValue[0])), QLatin1String("green"));

	return;
}

/***********************************************Spearman ******************************************************************/
// All formulaes and symbols are taken from : https://www.statisticshowto.datasciencecentral.com/spearman-rank-correlation-definition-calculate/

void CorrelationCoefficient::performSpearman() {
	if (m_columns.count() != 2) {
		printError(i18n("Select only 2 columns "));
		return;
	}

	auto* col1 = m_columns.at(0);
	auto* col2 = m_columns.at(1);
	QString col1Name = col1->name();
	QString col2Name = col2->name();

	int N = col1->statistics().size;
	if (N != col2->statistics().size) {
		printError(i18n("Number of data values in Column %1 and in Column %2 are not equal", col1Name, col2Name));
		return;
	}

	QMap<double, int> col1Ranks;
	convertToRanks(col1, N, col1Ranks);

	QMap<double, int> col2Ranks;
	convertToRanks(col2, N, col2Ranks);

	double ranksCol1Mean = 0;
	double ranksCol2Mean = 0;

	for (int i = 0; i < N; i++) {
		ranksCol1Mean += col1Ranks[int(col1->valueAt(i))];
		ranksCol2Mean += col2Ranks[int(col2->valueAt(i))];
	}

	ranksCol1Mean /= N;
	ranksCol2Mean /= N;

	double s12 = 0;
	double s1 = 0;
	double s2 = 0;

	for (int i = 0; i < N; i++) {
		double centeredRank_1 = col1Ranks[int(col1->valueAt(i))] - ranksCol1Mean;
		double centeredRank_2 = col2Ranks[int(col2->valueAt(i))] - ranksCol2Mean;

		s12 += centeredRank_1 * centeredRank_2;

		s1 += gsl_pow_2(centeredRank_1);
		s2 += gsl_pow_2(centeredRank_2);
	}

	s12 /= N;
	s1 /= N;
	s2 /= N;

	m_correlationValue = s12 / std::sqrt(s1 * s2);

	printLine(0, i18n("Spearman Rank Correlation value is %1", m_correlationValue), QLatin1String("green"));
}

/***********************************************Chi Square Test for Indpendence******************************************************************/

// TODO: Find P value from chi square test statistic:
void CorrelationCoefficient::performChiSquareIndpendence(bool calculateStats) {
	int rowCount;
	int columnCount;

	QVector<double> sumRows;
	QVector<double> sumColumns;
	int overallTotal = 0;
	QVector<QVector<int>> observedValues;

	QStringList horizontalHeader;
	QStringList verticalHeader;

	if (!calculateStats) {
		rowCount = m_inputStatsTableModel->rowCount() - 1;
		columnCount = m_inputStatsTableModel->columnCount() - 1;

		sumRows.resize(rowCount);
		sumColumns.resize(columnCount);
		observedValues.resize(rowCount);

		for (int i = 1; i <= rowCount; i++) {
			observedValues[i - 1].resize(columnCount);
			for (int j = 1; j <= columnCount; j++) {
				int cellValue = m_inputStatsTableModel->data(m_inputStatsTableModel->index(i, j)).toInt();
				sumRows[i - 1] += cellValue;
				sumColumns[j - 1] += cellValue;
				overallTotal += cellValue;
				observedValues[i - 1][j - 1] = cellValue;
			}
		}

		for (int i = 0; i < columnCount + 1; i++)
			horizontalHeader.append(m_inputStatsTableModel->data(m_inputStatsTableModel->index(0, i)).toString());

		for (int i = 0; i < rowCount + 1; i++)
			verticalHeader.append(m_inputStatsTableModel->data(m_inputStatsTableModel->index(i, 0)).toString());
	} else {
		if (m_columns.count() != 3) {
			printError(i18n("Select only 3 columns"));
			return;
		}

		auto* col1 = m_columns.at(0);
		auto* col2 = m_columns.at(1);
		int nRows = col1->statistics().size;

		rowCount = 0;
		columnCount = 0;

		horizontalHeader.append(QString());
		verticalHeader.append(QString());

		QMap<QString, int> independentVar1;
		QMap<QString, int> independentVar2;
		for (int i = 0; i < nRows; i++) {
			QString cell1Text = col1->textAt(i);
			QString cell2Text = col2->textAt(i);

			if (independentVar1[cell1Text] == 0) {
				independentVar1[cell1Text] = ++columnCount;
				horizontalHeader.append(cell1Text);
			}

			if (independentVar2[cell2Text] == 0) {
				independentVar2[cell2Text] = ++rowCount;
				verticalHeader.append(cell2Text);
			}
		}

		sumRows.resize(rowCount);
		sumColumns.resize(columnCount);
		observedValues.resize(rowCount);
		for (int i = 0; i < rowCount; i++)
			observedValues[i].resize(columnCount);

		for (int i = 0; i < nRows; i++) {
			QString cell1Text = col1->textAt(i);
			QString cell2Text = col2->textAt(i);
			int cellValue = int(m_columns[2]->valueAt(i));

			int partition1Number = independentVar1[cell1Text] - 1;
			int partition2Number = independentVar2[cell2Text] - 1;

			sumRows[partition1Number] += cellValue;
			sumColumns[partition2Number] += cellValue;
			overallTotal += cellValue;
			observedValues[partition1Number][partition2Number] = cellValue;
		}
	}

	if (overallTotal == 0)
		printError(i18n("Enter some data: All columns are empty"));

	QVector<QVector<double>> expectedValues(rowCount, QVector<double>(columnCount));

	for (int i = 0; i < rowCount; i++)
		for (int j = 0; j < columnCount; j++)
			expectedValues[i][j] = (double(sumRows[i]) * double(sumColumns[j])) / overallTotal;

	m_statsTable += QLatin1String("<h3>") + i18n("Observed Value Table") + QLatin1String("</h3>");
	QList<HtmlCell*> rowMajor;
	int level = 0;
	// horizontal header
	for (int i = 0; i < columnCount + 1; i++)
		rowMajor.append(new HtmlCell(horizontalHeader[i], level, true));

	rowMajor.append(new HtmlCell(i18n("Total"), level, true));

	//data with vertical header.
	for (int i = 1; i < rowCount + 1; i++) {
		level++;
		rowMajor.append(new HtmlCell(verticalHeader[i], level, true));
		for (int j = 0; j < columnCount; j++)
			rowMajor.append(new HtmlCell(round(observedValues[i - 1][j]), level));

		rowMajor.append(new HtmlCell(round(sumRows[i - 1]), level));
	}

	level++;
	rowMajor.append(new HtmlCell(i18n("Total"), level, true));
	for (int i = 0; i < columnCount; i++)
		rowMajor.append(new HtmlCell(round(sumColumns[i]), level));

	rowMajor.append(new HtmlCell(round(overallTotal), level));
	m_statsTable += getHtmlTable3(rowMajor);

	m_statsTable += QLatin1String("<br>");

	m_statsTable += QLatin1String("<h3>") + i18n("Expected Value Table") + QLatin1String("</h3>");
	rowMajor.clear();
	level = 0;
	// horizontal header
	for (int i = 0; i < columnCount + 1; i++)
		rowMajor.append(new HtmlCell(horizontalHeader[i], level, true));

	rowMajor.append(new HtmlCell(i18n("Total"), level, true));

	//data with vertical header.
	for (int i = 1; i < rowCount + 1; i++) {
		level++;
		rowMajor.append(new HtmlCell(verticalHeader[i], level, true));
		for (int j = 0; j < columnCount; j++)
			rowMajor.append(new HtmlCell(round(expectedValues[i - 1][j]), level));

		rowMajor.append(new HtmlCell(round(sumRows[i - 1]), level));
	}

	level++;
	rowMajor.append(new HtmlCell(i18n("Total"), level, true));
	for (int i = 0; i < columnCount; i++)
		rowMajor.append(new HtmlCell(round(sumColumns[i]), level));

	rowMajor.append(new HtmlCell(round(overallTotal), level));

	m_statsTable += getHtmlTable3(rowMajor);

	double chiSquareVal = 0;
	// finding chi-square value;
	for (int i = 0; i < rowCount; i++)
		for (int j = 0; j < columnCount; j++)
			chiSquareVal += gsl_pow_2(observedValues[i][j] - expectedValues[i][j]) / expectedValues[i][j];

	m_statisticValue.append(chiSquareVal);
	int df = (rowCount - 1) * (columnCount - 1);
	printLine(0, i18n("Degree of Freedom is %1", df), QLatin1String("blue"));
	printLine(1, i18n("Chi Square Statistic Value is %1", round(chiSquareVal)), QLatin1String("green"));
}

/***********************************************Helper Functions******************************************************************/

int CorrelationCoefficient::findDiscordants(int *ranks, int start, int end) {
	if (start >= end)
		return 0;

	int mid = (start + end) / 2;

	int leftDiscordants = findDiscordants(ranks, start, mid);
	int rightDiscordants = findDiscordants(ranks, mid + 1, end);

	int len = end - start + 1;
	int leftLen = mid - start + 1;
	int rightLen = end - mid;
	int leftLenRemain = leftLen;

	QVector<int> leftRanks(leftLen);
	QVector<int> rightRanks(rightLen);

	for (int i = 0; i < leftLen; i++)
		leftRanks[i] = ranks[start + i];

	for (int i = leftLen; i < leftLen + rightLen; i++)
		rightRanks[i - leftLen] = ranks[start + i];

	int mergeDiscordants = 0;
	int i = 0, j = 0, k =0;
	while (i < len) {
		if (j >= leftLen) {
			ranks[start + i] = rightRanks[k];
			k++;
		} else if (k >= rightLen) {
			ranks[start + i] = leftRanks[j];
			j++;
		} else if (leftRanks[j] < rightRanks[k]) {
			ranks[start + i] = leftRanks[j];
			j++;
			leftLenRemain--;
		} else if (leftRanks[j] > rightRanks[k]) {
			ranks[start + i] = rightRanks[k];
			mergeDiscordants += leftLenRemain;
			k++;
		}
		i++;
	}
	return leftDiscordants + rightDiscordants + mergeDiscordants;
}

void CorrelationCoefficient::convertToRanks(const Column* col, int N, QMap<double, int> &ranks) {
	if (col->isNumeric())
		return;

	double* sortedList = new double[N];
	for (int i = 0; i < N; i++)
		sortedList[i] = col->valueAt(i);

	std::sort(sortedList, sortedList + N, std::greater<double>());

	ranks.clear();
	for (int i = 0; i < N; i++)
		ranks[sortedList[i]] = i + 1;

	delete[] sortedList;
}
