/***************************************************************************
	File                 : HypothesisTest.cpp
	Project              : LabPlot
	Description          : Doing Hypothesis-Test on data provided
	--------------------------------------------------------------------
	Copyright            : (C) 2019 Devanshu Agarwal(agarwaldevanshu8@gmail.com)

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/

#include "HypothesisTest.h"
#include "HypothesisTestPrivate.h"
#include "kdefrontend/hypothesisTest/HypothesisTestView.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/core/column/Column.h"
#include "backend/lib/macros.h"

#include <QVector>
#include <QStandardItemModel>
#include <QLocale>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>
#include <QDebug>
#include <QtMath>
#include <KLocalizedString>

#include <gsl/gsl_cdf.h>
#include <gsl/gsl_math.h>

extern "C" {
#include "backend/nsl/nsl_stats.h"
}

HypothesisTest::HypothesisTest(const QString &name) : AbstractPart(name),
	d(new HypothesisTestPrivate(this)) {
}

HypothesisTest::~HypothesisTest() {
	delete d;
}

void HypothesisTest::setDataSourceType(DataSourceType type) {
	if (type != d->dataSourceType)
		d->dataSourceType = type;
}

HypothesisTest::DataSourceType HypothesisTest::dataSourceType() const {
	return d->dataSourceType;
}

void HypothesisTest::setDataSourceSpreadsheet(Spreadsheet* spreadsheet) {
	if  (spreadsheet != d->dataSourceSpreadsheet)
		d->setDataSourceSpreadsheet(spreadsheet);
}

void HypothesisTest::setColumns(const QVector<Column* >& cols) {
	d->columns = cols;
}

void HypothesisTest::setColumns(QStringList cols) {
	return d->setColumns(cols);
}

QStringList HypothesisTest::allColumns() {
	return d->allColumns;
}

void HypothesisTest::setPopulationMean(QVariant populationMean) {
	d->populationMean = populationMean.toDouble();
}

void HypothesisTest::setSignificanceLevel(QVariant alpha) {
	d->significanceLevel = alpha.toDouble();
}


QString HypothesisTest::testName() {
	return d->currTestName;
}

QString HypothesisTest::statsTable() {
	return d->statsTable;
}

void HypothesisTest::performTest(Test test, bool categoricalVariable, bool equalVariance) {
	d->tailType = test.tail;
	switch (test.subtype) {
	case HypothesisTest::Test::SubType::TwoSampleIndependent: {
		d->currTestName = i18n( "<h2>Two Sample Independent Test</h2>");
		d->performTwoSampleIndependentTest(test.type, categoricalVariable, equalVariance);
		break;
	}
	case HypothesisTest::Test::SubType::TwoSamplePaired:
		d->currTestName = i18n( "<h2>Two Sample Paired Test</h2>");
		d->performTwoSamplePairedTest(test.type);
		break;
	case HypothesisTest::Test::SubType::OneSample: {
		d->currTestName = i18n( "<h2>One Sample Test</h2>");
		d->performOneSampleTest(test.type);
		break;
	}
	case HypothesisTest::Test::SubType::OneWay: {
		d->currTestName = i18n( "<h2>One Way Anova</h2>");
		d->performOneWayAnova();
		break;
	}
	case HypothesisTest::Test::SubType::TwoWay:
		break;
	case HypothesisTest::Test::SubType::NoneSubType:
		break;
	}

	emit changed();
}

void HypothesisTest::performLeveneTest(bool categoricalVariable) {
	d->currTestName = i18n( "<h2>Levene Test for Equality of Variance</h2>");
	d->performLeveneTest(categoricalVariable);

	emit changed();
}

double HypothesisTest::statisticValue() {
	return d->statisticValue;
}

double HypothesisTest::pValue() {
	return d->pValue;
}

QVBoxLayout* HypothesisTest::summaryLayout() {
	return d->summaryLayout;
}

/******************************************************************************
 *                      Private Implementations
 * ****************************************************************************/

//TODO: round off numbers while printing
//TODO: backend of z test;


HypothesisTestPrivate::HypothesisTestPrivate(HypothesisTest* owner) : q(owner),
	summaryLayout(new QVBoxLayout()) {

	for (int i = 0; i < 10; i++) {
		resultLine[i] = new QLabel();
		summaryLayout->addWidget(resultLine[i]);
	}
}

HypothesisTestPrivate::~HypothesisTestPrivate() {
}

void HypothesisTestPrivate::setDataSourceSpreadsheet(Spreadsheet* spreadsheet) {
	dataSourceSpreadsheet = spreadsheet;

	//setting rows and columns count;
	rowCount = dataSourceSpreadsheet->rowCount();
	columnCount = dataSourceSpreadsheet->columnCount();

	for (auto* col : dataSourceSpreadsheet->children<Column>())
		allColumns << col->name();
}


void HypothesisTestPrivate::setColumns(QStringList cols) {
	columns.clear();
	Column* column = new Column("column");
	for (QString col : cols) {
		if (!cols.isEmpty()) {
			column = dataSourceSpreadsheet->column(col);
			columns.append(column);
		}
	}
	delete[] column;
}


/**************************Two Sample Independent *************************************/

void HypothesisTestPrivate::performTwoSampleIndependentTest(HypothesisTest::Test::Type test, bool categoricalVariable, bool equalVariance) {
	clearTestView();

	if (columns.size() != 2) {
		printError("Inappropriate number of columns selected");
		return;
	}

	int n[2];
	double sum[2], mean[2], std[2];

	QString col1Name = columns[0]->name();
	QString col2Name = columns[1]->name();

	if (!categoricalVariable && isNumericOrInteger(columns[0])) {
		for (int i = 0; i < 2; i++) {
			findStats(columns[i], n[i], sum[i], mean[i], std[i]);

			if (n[i] < 1) {
				printError("At least one of selected column is empty");

				return;
			}
		}
	} else {
		QMap<QString, int> colName;
		QString baseColName;
		int np;
		int totalRows;

		countPartitions(columns[0], np, totalRows);
		if (np != 2) {
			printError( i18n("Number of Categorical Variable in Column %1 is not equal to 2", columns[0]->name()));
			return;
		}

		if (isNumericOrInteger(columns[0]))
			baseColName = columns[0]->name();

		ErrorType errorCode = findStatsCategorical(columns[0], columns[1], n, sum, mean, std, colName, np, totalRows);

		switch (errorCode) {
		case ErrorUnqualSize: {
			printError( i18n("Unequal size between Column %1 and Column %2", columns[0]->name(), columns[1]->name()));
			return;
		}
		case ErrorEmptyColumn: {
			printError("At least one of selected column is empty");

			return;
		}
		case NoError:
			break;
		}

		QMapIterator<QString, int> i(colName);
		while (i.hasNext()) {
			i.next();
			if (i.value() == 1)
				col1Name = baseColName + " " + i.key();
			else
				col2Name = baseColName + " " + i.key();
		}
	}

	QVariant rowMajor[] = {"", "N", "Sum", "Mean", "Std",
						   col1Name, n[0], sum[0], mean[0], std[0],
						   col2Name, n[1], sum[1], mean[1], std[1]
						  };

	statsTable = getHtmlTable(3, 5, rowMajor);

	QString testName;
	int df = 0;
	double sp = 0;

	switch (test) {
	case HypothesisTest::Test::Type::TTest: {
		testName = "T";

		if (equalVariance) {
			df = n[0] + n[1] - 2;
			sp = qSqrt(((n[0]-1) * gsl_pow_2(std[0]) +
					(n[1]-1) * gsl_pow_2(std[1]) ) / df );
			statisticValue = (mean[0] - mean[1]) / (sp * qSqrt(1.0/n[0] + 1.0/n[1]));
			printLine(9, "<b>Assumption:</b> Equal Variance b/w both population means");
		} else {
			double temp_val;
			temp_val = gsl_pow_2( gsl_pow_2(std[0]) / n[0] + gsl_pow_2(std[1]) / n[1]);
			temp_val = temp_val / ( (gsl_pow_2( (gsl_pow_2(std[0]) / n[0]) ) / (n[0]-1)) +
					(gsl_pow_2( (gsl_pow_2(std[1]) / n[1]) ) / (n[1]-1)));
			df = qRound(temp_val);

			statisticValue = (mean[0] - mean[1]) / (qSqrt( (gsl_pow_2(std[0])/n[0]) +
					(gsl_pow_2(std[1])/n[1])));
			printLine(9, "<b>Assumption:</b> UnEqual Variance b/w both population means");
		}

		printLine(8, "<b>Assumption:</b> Both Populations approximately follow normal distribution");
		break;
	}
	case HypothesisTest::Test::Type::ZTest: {
		testName = "Z";
		sp = qSqrt( ((n[0]-1) * gsl_pow_2(std[0]) + (n[1]-1) * gsl_pow_2(std[1])) / df);
		statisticValue = (mean[0] - mean[1]) / (sp * qSqrt( 1.0 / n[0] + 1.0 / n[1]));
		pValue = gsl_cdf_gaussian_P(statisticValue, sp);
		break;
	}
	case HypothesisTest::Test::Type::Anova:
		break;
	case HypothesisTest::Test::Type::NoneType:
		break;
	}

	currTestName = i18n( "<h2>Two Sample Independent %1 Test for %2 vs %3</h2>", testName, col1Name, col2Name);
	pValue = getPValue(test, statisticValue, col1Name, col2Name, (mean[0] - mean[1]), sp, df);

	printLine(2, i18n("Significance level is %1", significanceLevel), "blue");

	printLine(4, i18n("%1 Value is %2 ", testName, statisticValue), "green");
	printTooltip(4, i18n("More is the |%1-value|, more safely we can reject the null hypothesis", testName));

	printLine(5, i18n("P Value is %1 ", pValue), "green");

	printLine(6, i18n("Degree of Freedom is %1", df), "green");
	printTooltip(6, i18n("Number of independent Pieces of information that went into calculating the estimate"));

	if (pValue <= significanceLevel)
		printTooltip(5, i18n("We can safely reject Null Hypothesis for significance level %1", significanceLevel));
	else
		printTooltip(5, i18n("There is a plausibility for Null Hypothesis to be true"));
	return;
}

/********************************Two Sample Paired ***************************************/

void HypothesisTestPrivate::performTwoSamplePairedTest(HypothesisTest::Test::Type test) {
	clearTestView();

	if (columns.size() != 2) {
		printError("Inappropriate number of columns selected");

		return;
	}

	for (int i = 0; i < 2; i++) {
		if ( !isNumericOrInteger(columns[0])) {
			printError("select only columns with numbers");
			return;
		}
	}

	int n;
	double sum, mean, std;
	ErrorType errorCode = findStatsPaired(columns[0], columns[1], n, sum, mean, std);

	switch (errorCode) {
	case ErrorUnqualSize: {
		printError("both columns are having different sizes");

		return;
	}
	case ErrorEmptyColumn: {
		printError("columns are empty");

		return;
	}
	case NoError:
		break;
	}

	if (n == -1) {
		printError("both columns are having different sizes");

		return;
	}

	if (n < 1) {
		printError("columns are empty");

		return;
	}

	QVariant rowMajor[] = {"", "N", "Sum", "Mean", "Std",
						   "difference", n, sum, mean, std
						  };

	statsTable = getHtmlTable(2, 5, rowMajor);

	QString testName;
	int df = 0;

	switch (test) {
	case HypothesisTest::Test::Type::TTest: {
		statisticValue = mean / (std / qSqrt(n));
		df = n - 1;
		testName = "T";
		printLine(6, i18n("Degree of Freedom is %1</p", df), "green");
		break;
	}
	case HypothesisTest::Test::Type::ZTest: {
		testName = "Z";
		statisticValue = mean / (std / qSqrt(n));
		df = n - 1;
		break;
	}
	case HypothesisTest::Test::Type::Anova:
		break;
	case HypothesisTest::Test::Type::NoneType:
		break;

	}

	pValue = getPValue(test, statisticValue, columns[0]->name(), i18n("%1", populationMean), mean, std, df);
	currTestName = i18n( "<h2>One Sample %1 Test for %2 vs %3</h2>", testName, columns[0]->name(), columns[1]->name());

	printLine(2, i18n("Significance level is %1 ", significanceLevel), "blue");
	printLine(4, i18n("%1 Value is %2 ", testName, statisticValue), "green");
	printLine(5, i18n("P Value is %1 ", pValue), "green");

	if (pValue <= significanceLevel)
		printTooltip(5, i18n("We can safely reject Null Hypothesis for significance level %1", significanceLevel));
	else
		printTooltip(5, i18n("There is a plausibility for Null Hypothesis to be true"));

	return;
}

/******************************** One Sample ***************************************/

void HypothesisTestPrivate::performOneSampleTest(HypothesisTest::Test::Type test) {
	clearTestView();

	if (columns.size() != 1) {
		printError("Inappropriate number of columns selected");

		return;
	}

	if ( !isNumericOrInteger(columns[0])) {
		printError("select only columns with numbers");

		return;
	}

	int n;
	double sum, mean, std;
	ErrorType errorCode = findStats(columns[0], n, sum, mean, std);

	switch (errorCode) {
	case ErrorUnqualSize: {
		printError("column is empty");

		return;
	}
	case NoError:
		break;
	case ErrorEmptyColumn: {

		return;
	}
	}

	QVariant rowMajor[] = {"", "N", "Sum", "Mean", "Std",
						   columns[0]->name(), n, sum, mean, std
						  };

	statsTable = getHtmlTable(2, 5, rowMajor);

	QString testName;
	int df = 0;

	switch (test) {
	case HypothesisTest::Test::Type::TTest: {
		testName = "T";
		statisticValue = (mean - populationMean) / (std / qSqrt(n));
		df = n - 1;
		printLine(6, i18n("Degree of Freedom is %1", df), "blue");
		break;
	}
	case HypothesisTest::Test::Type::ZTest: {
		testName = "Z";
		df = 0;
		statisticValue = (mean - populationMean) / (std / qSqrt(n));
		break;
	}
	case HypothesisTest::Test::Type::Anova:
		break;
	case HypothesisTest::Test::Type::NoneType:
		break;
	}

	pValue = getPValue(test, statisticValue, columns[0]->name(), i18n("%1",populationMean), mean - populationMean, std, df);
	currTestName = i18n( "<h2>One Sample %1 Test for %2</h2>", testName, columns[0]->name());

	printLine(2, i18n("Significance level is %1", significanceLevel), "blue");
	printLine(4, i18n("%1 Value is %2", testName, statisticValue), "green");
	printLine(5, i18n("P Value is %1", pValue), "green");

	if (pValue <= significanceLevel)
		printTooltip(5, i18n("We can safely reject Null Hypothesis for significance level %1", significanceLevel));
	else
		printTooltip(5, i18n("There is a plausibility for Null Hypothesis to be true"));

	return;
}

/*************************************One Way Anova***************************************/
// all standard variables and formulas are taken from this wikipedia page:
// https://en.wikipedia.org/wiki/One-way_analysis_of_variance
// b stands for b/w groups
// w stands for within groups
// np is number of partition i.e., number of classes
void HypothesisTestPrivate::performOneWayAnova() {
	clearTestView();
	int np, totalRows;
	countPartitions(columns[0], np, totalRows);

	int* ni = new int[np];
	double* sum = new double[np];
	double* mean = new double[np];
	double* std = new double[np];
	QString* colNames = new QString[np];

	QMap<QString, int> classnameToIndex;
	QString baseColName;

	if (isNumericOrInteger(columns[0]))
		baseColName = columns[0]->name();

	findStatsCategorical(columns[0], columns[1], ni, sum, mean, std, classnameToIndex, np, totalRows);

	double yBar = 0;		// overall mean
	double sB = 0;			// sum of squares of (mean - overall_mean) between the groups
	int fB = 0;			// degree of freedom between the groups
	double msB = 0;		// mean sum of squares between the groups
	double sW = 0;			// sum of squares of (value - mean of group) within the groups
	int fW = 0;			// degree of freedom within the group
	double msW = 0;		// mean sum of squares within the groups
	double fValue = 0;


	// now finding mean of each group;

	for (int i = 0; i < np; i++)
		yBar += mean[i];
	yBar = yBar / np;

	for (int i = 0; i < np; i++) {
		sB += ni[i] * gsl_pow_2( ( mean[i] - yBar));
		if (ni[i] > 1)
			sW += gsl_pow_2( std[i])*(ni[i] - 1);
		else
			sW += gsl_pow_2( std[i]);
		fW += ni[i] - 1;
	}

	fB = np - 1;
	msB = sB / fB;

	msW = sW / fW;
	fValue = msB / msW;


	pValue = nsl_stats_fdist_p(fValue, static_cast<size_t>(np-1), fW);

	QMapIterator<QString, int> i(classnameToIndex);
	while (i.hasNext()) {
		i.next();
		colNames[i.value()-1] = baseColName + " " + i.key();
	}

	// now printing the statistics and result;
	int rowCount = np + 1, columnCount = 5;
	QVariant* rowMajor = new QVariant[rowCount*columnCount];
	// header data;
	rowMajor[0] = "";
	rowMajor[1] = "Ni";
	rowMajor[2] = "Sum";
	rowMajor[3] = "Mean";
	rowMajor[4] = "Std";

	// table data
	for (int row_i = 1; row_i < rowCount ; row_i++) {
		rowMajor[row_i*columnCount]		= colNames[row_i - 1];
		rowMajor[row_i*columnCount + 1]	= ni[row_i - 1];
		rowMajor[row_i*columnCount + 2]	= sum[row_i - 1];
		rowMajor[row_i*columnCount + 3]	= QString::number( mean[row_i - 1], 'f', 3);
		rowMajor[row_i*columnCount + 4]	= QString::number( std[row_i - 1], 'f', 3);
	}

	statsTable = i18n( "<h3>Group Summary Statistics</h3>");

	statsTable += getHtmlTable(rowCount, columnCount, rowMajor);

	statsTable += getLine("");
	statsTable += getLine("");
	statsTable += i18n( "<h3>Grand Summary Statistics</h3>");
	statsTable += getLine("");
	statsTable += getLine(i18n("Overall Mean is %1", yBar));

	rowCount = 4;
	columnCount = 3;
	rowMajor->clear();

	rowMajor[0] = "";
	rowMajor[1] = "Between Groups";
	rowMajor[2] = "Within Groups";

	int baseIndex = 0;
	baseIndex = 1 * columnCount;
	rowMajor[baseIndex + 0] = "Sum of Squares";
	rowMajor[baseIndex + 1] = sB;
	rowMajor[baseIndex + 2] = sW;
	baseIndex = 2 * columnCount;
	rowMajor[baseIndex + 0] = "Degree of Freedom";
	rowMajor[baseIndex + 1] = fB;
	rowMajor[baseIndex + 2] = fW;
	baseIndex = 3 * columnCount;
	rowMajor[baseIndex + 0] = "Mean Square Value";
	rowMajor[baseIndex + 1] = msB;
	rowMajor[baseIndex + 2] = msW;

	statsTable += getHtmlTable(rowCount, columnCount, rowMajor);

	delete[] ni;
	delete[] sum;
	delete[] mean;
	delete[] std;
	delete[] colNames;

	printLine(1, i18n("F Value is %1", fValue), "blue");
	printLine(2, i18n("P Value is %1 ", pValue), "green");

	if (pValue <= significanceLevel)
		printTooltip(2, i18n("We can safely reject Null Hypothesis for significance level %1", significanceLevel));
	else
		printTooltip(2, i18n("There is a plausibility for Null Hypothesis to be true"));

	return;
}

/**************************************Levene Test****************************************/
// TODO: Fix: Program crashes when n = np;

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
void HypothesisTestPrivate::performLeveneTest(bool categoricalVariable) {
	clearTestView();

	if (columns.size() != 2) {
		printError("Inappropriate number of columns selected");
		return;
	}

	int np = 0;
	int n = 0;

	if (!categoricalVariable && isNumericOrInteger(columns[0]))
		np = columns.size();
	else
		countPartitions(columns[0], np, n);

	if (np < 2) {
		printError("select atleast two columns / classes");
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
	if (!categoricalVariable && isNumericOrInteger(columns[0])) {
		totalRows = columns[0]->rowCount();

		double value = 0;
		for (int j = 0; j < totalRows; j++) {
			int numberNaNCols = 0;
			for (int i = 0; i < np; i++) {
				value = columns[i]->valueAt(j);
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
				yiBar[i] = yiBar[i] / ni[i];
		}

		for (int j = 0; j < totalRows; j++) {
			for (int i = 0; i < np; i++) {
				value = columns[i]->valueAt(j);
				if (!(std::isnan(value)))
					ziBar[i] += abs(value - yiBar[i]);
			}
		}

		for (int i = 0; i < np; i++) {
			ziBarBar += ziBar[i];
			if (ni[i] > 0)
				ziBar[i] = ziBar[i] / ni[i];
		}

		ziBarBar = ziBarBar / n;

		double numberatorValue = 0;
		double denominatorValue = 0;

		for (int j = 0; j < totalRows; j++) {
			for (int i = 0; i < np; i++) {
				value = columns[i]->valueAt(j);
				if (!(std::isnan(value))) {
					double zij = abs(value - yiBar[i]);
					denominatorValue += gsl_pow_2( (zij - ziBar[i]));
				}
			}
		}

		for (int i = 0; i < np; i++) {
			colNames[i] = columns[i]->name();
			numberatorValue += ni[i]*gsl_pow_2( (ziBar[i]-ziBarBar));
		}

		fValue = ((n - np) / (np - 1)) * (numberatorValue / denominatorValue);

	} else {
		QMap<QString, int> classnameToIndex;

		AbstractColumn::ColumnMode originalColMode = columns[0]->columnMode();
		columns[0]->setColumnMode(AbstractColumn::Text);

		int partitionNumber = 1;
		QString name;
		double value;
		int classIndex;

		for (int j = 0; j < n; j++) {
			name = columns[0]->textAt(j);
			value = columns[1]->valueAt(j);
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
				yiBar[i] = yiBar[i] / ni[i];
		}

		for (int j = 0; j < n; j++) {
			name = columns[0]->textAt(j);
			value = columns[1]->valueAt(j);
			classIndex = classnameToIndex[name] - 1;
			ziBar[classIndex] += abs(value - yiBar[classIndex]);
		}

		for (int i = 0; i < np; i++) {
			ziBarBar += ziBar[i];
			ziBar[i] = ziBar[i] / ni[i];
		}

		ziBarBar = ziBarBar / n;

		double numberatorValue = 0;
		double denominatorValue = 0;

		for (int j = 0; j < n; j++) {
			name = columns[0]->textAt(j);
			value = columns[1]->valueAt(j);
			classIndex = classnameToIndex[name] - 1;
			double zij = abs(value - yiBar[classIndex]);
			denominatorValue +=  gsl_pow_2( (zij - ziBar[classIndex]));
		}

		for (int i = 0; i < np; i++)
			numberatorValue += ni[i]*gsl_pow_2( (ziBar[i]-ziBarBar));

		if (denominatorValue <= 0) {
			printError( "number of data points is less or than equal to number of categorical variables");
			return;
		}

		fValue = ((n - np) / (np - 1)) * (numberatorValue / denominatorValue);

		QMapIterator<QString, int> i(classnameToIndex);
		while (i.hasNext()) {
			i.next();
			colNames[i.value()-1] = columns[0]->name() + " " + i.key();
		}

		columns[0]->setColumnMode(originalColMode);
	}

	df = n - np;

	// now making the stats table.
	int rowCount = np+1;
	int columnCount = 4;

	QVariant* rowMajor = new QVariant[rowCount*columnCount];
	// header data;
	rowMajor[0] = "";
	rowMajor[1] = "Ni";
	rowMajor[2] = "yiBar";
	rowMajor[3] = "ziBar";

	// table data
	for (int row_i = 1; row_i < rowCount; row_i++) {
		rowMajor[row_i*columnCount] = colNames[row_i-1];
		rowMajor[row_i*columnCount + 1] = ni[row_i-1];
		rowMajor[row_i*columnCount + 2] = yiBar[row_i-1];
		rowMajor[row_i*columnCount + 3] = ziBar[row_i-1];
	}

	statsTable = getHtmlTable(rowCount, columnCount, rowMajor);

	delete[] rowMajor;
	delete[] yiBar;
	delete[] ziBar;
	delete[] ni;

	pValue = nsl_stats_fdist_p(fValue, static_cast<size_t>(np-1), df);

	printLine(0, "Null Hypothesis: Variance is equal between all classes", "blue");
	printLine(1, "Alternate Hypothesis: Variance is not equal in at-least one pair of classes", "blue");
	printLine(2, i18n("Significance level is %1", significanceLevel), "blue");
	printLine(4, i18n("F Value is %1 ", fValue), "green");
	printLine(5, i18n("P Value is %1 ", pValue), "green");
	printLine(6, i18n("Degree of Freedom is %1", df), "green");

	if (pValue <= significanceLevel) {
		printTooltip(5, i18n("We can safely reject Null Hypothesis for significance level %1", significanceLevel));
		printLine(8, "Requirement for homogeneity is not met", "red");
	} else {
		printTooltip(5, i18n("There is a plausibility for Null Hypothesis to be true"));
		printLine(8, "Requirement for homogeneity is met", "green");
	}

	return;
}

/***************************************Helper Functions*************************************/

bool HypothesisTestPrivate::isNumericOrInteger(Column* column) {
	return (column->columnMode() == AbstractColumn::Numeric || column->columnMode() == AbstractColumn::Integer);
}

HypothesisTestPrivate::ErrorType HypothesisTestPrivate::findStats(const Column* column, int& count, double& sum, double& mean, double& std) {
	sum = 0;
	mean = 0;
	std = 0;

	count = column->rowCount();
	for (int i = 0; i < count; i++) {
		double row = column->valueAt(i);
		if ( std::isnan(row)) {
			count = i;
			break;
		}
		sum += row;
	}

	if (count < 1)
		return HypothesisTestPrivate::ErrorEmptyColumn;

	mean = sum / count;

	for (int i = 0; i < count; i++) {
		double row = column->valueAt(i);
		std += gsl_pow_2( (row - mean));
	}

	if (count > 1)
		std = std / (count-1);
	std = qSqrt(std);

	return HypothesisTestPrivate::NoError;
}

HypothesisTestPrivate::ErrorType HypothesisTestPrivate::findStatsPaired(const Column* column1, const Column* column2, int& count, double& sum, double& mean, double& std) {
	sum = 0;
	mean = 0;
	std = 0;

	int count1 = column1->rowCount();
	int count2 = column2->rowCount();

	count = qMin(count1, count2);
	double cell1, cell2;
	for (int i = 0; i < count; i++) {
		cell1 = column1->valueAt(i);
		cell2 = column2->valueAt(i);

		if (std::isnan(cell1) || std::isnan(cell2)) {
			if (std::isnan(cell1) && std::isnan(cell2))
				count = i;
			else
				return HypothesisTestPrivate::ErrorUnqualSize;
			break;
		}

		sum += cell1 - cell2;
	}

	if (count < 1)
		return HypothesisTestPrivate::ErrorEmptyColumn;

	mean = sum / count;

	double row;
	for (int i = 0; i < count; i++) {
		cell1 = column1->valueAt(i);
		cell2 = column2->valueAt(i);
		row = cell1 - cell2;
		std += gsl_pow_2( (row - mean));
	}

	if (count > 1)
		std = std / (count-1);

	std = qSqrt(std);
	return HypothesisTestPrivate::NoError;
}

void HypothesisTestPrivate::countPartitions(Column* column, int& np, int& totalRows) {
	totalRows = column->rowCount();
	np = 0;
	QString cellValue;
	QMap<QString, bool> discoveredCategoricalVar;

	AbstractColumn::ColumnMode originalColMode = column->columnMode();
	column->setColumnMode(AbstractColumn::Text);

	for (int i = 0; i < totalRows; i++) {
		cellValue = column->textAt(i);

		if (cellValue.isEmpty()) {
			totalRows = i;
			break;
		}

		if (discoveredCategoricalVar[cellValue])
			continue;

		discoveredCategoricalVar[cellValue] = true;
		np++;
	}
	column->setColumnMode(originalColMode);
}

HypothesisTestPrivate::ErrorType HypothesisTestPrivate::findStatsCategorical(Column* column1, Column* column2, int n[], double sum[], double mean[], double std[], QMap<QString, int>& colName, const int& np, const int& totalRows) {
	Column* columns[] = {column1, column2};

	for (int i = 0; i < np; i++) {
		n[i] = 0;
		sum[i] = 0;
		mean[i] = 0;
		std[i] = 0;
	}

	AbstractColumn::ColumnMode originalColMode = columns[0]->columnMode();
	columns[0]->setColumnMode(AbstractColumn::Text);

	int partitionNumber = 1;
	for (int i = 0; i < totalRows; i++) {
		QString name = columns[0]->textAt(i);
		double value = columns[1]->valueAt(i);

		if (std::isnan(value)) {
			columns[0]->setColumnMode(originalColMode);
			return HypothesisTestPrivate::ErrorUnqualSize;
		}

		if (colName[name] == 0) {
			colName[name] = partitionNumber;
			partitionNumber++;
		}

		n[colName[name]-1]++;
		sum[colName[name]-1] += value;
	}

	for (int i = 0; i < np; i++)
		mean[i] = sum[i] / n[i];

	for (int i = 0; i < totalRows; i++) {
		QString name = columns[0]->textAt(i);
		double value = columns[1]->valueAt(i);

		std[colName[name]-1] += gsl_pow_2( (value - mean[colName[name]-1]));
	}

	for (int i = 0; i < np; i++) {
		if (n[i] > 1)
			std[i] = std[i] / (n[i] - 1);
		std[i] = qSqrt(std[i]);
	}

	columns[0]->setColumnMode(originalColMode);
	if (isNumericOrInteger(columns[0])) {

	}

	return HypothesisTestPrivate::NoError;
}


//TODO change ("⋖") symbol to ("<"), currently macro UTF8_QSTRING is not working properly if used "<" symbol;
// TODO: check for correctness between: for TestZ with TailTwo
//       pValue = 2*gsl_cdf_tdist_P(value, df) v/s
//       pValue = gsl_cdf_tdis_P(value, df) + gsl_cdf_tdis_P(-value, df);
double HypothesisTestPrivate::getPValue(const HypothesisTest::Test::Type& test, double& value, const QString& col1Name, const QString& col2Name, const double mean, const double sp, const int df) {

	switch (test) {
	case HypothesisTest::Test::Type::TTest: {
		switch (tailType) {
		case HypothesisTest::Test::Tail::Negative: {
			pValue = gsl_cdf_tdist_P(value, df);
			printLine(0, i18n("Null Hypothesis: Population mean of %1 %2 Population mean of %3", col1Name, UTF8_QSTRING("≥"), col2Name), "blue");
			printLine(1, i18n("Alternate Hypothesis: Population mean of %1 %2 Population mean of %3", col1Name, UTF8_QSTRING("⋖"), col2Name), "blue");
			break;
		}
		case HypothesisTest::Test::Tail::Positive: {
			value *= -1;
			pValue = gsl_cdf_tdist_P(value, df);
			printLine(0, i18n("Null Hypothesis: Population mean of %1 %2 Population mean of %3", col1Name, UTF8_QSTRING("≤"), col2Name), "blue");
			printLine(1, i18n("Alternate Hypothesis: Population mean of %1 %2 Population mean of %3", col1Name, UTF8_QSTRING(">"), col2Name), "blue");
			break;
		}
		case HypothesisTest::Test::Tail::Two: {
			pValue = 2.*gsl_cdf_tdist_P(-1*abs(value), df);

			printLine(0, i18n("Null Hypothesis: Population mean of %1 %2 Population mean of %3", col1Name, UTF8_QSTRING("="), col2Name), "blue");
			printLine(1, i18n("Alternate Hypothesis: Population mean of %1 %2 Population mean of %3", col1Name, UTF8_QSTRING("≠"), col2Name), "blue");
			break;
		}
		}
		break;
	}
	case HypothesisTest::Test::Type::ZTest: {
		switch (tailType) {
		case HypothesisTest::Test::Tail::Negative: {
			pValue = gsl_cdf_gaussian_P(value - mean, sp);
			printLine(0, i18n("Null Hypothesis: Population mean of %1 %2 Population mean of %3 ", col1Name, UTF8_QSTRING("≥"), col2Name), "blue");
			printLine(1, i18n("Alternate Hypothesis: Population mean of %1 %2 Population mean of %3 ", col1Name, UTF8_QSTRING("⋖"), col2Name), "blue");
			break;
		}
		case HypothesisTest::Test::Tail::Positive: {
			value *= -1;
			pValue = nsl_stats_tdist_p(value - mean, sp);
			printLine(0, i18n("Null Hypothesis: Population mean of %1 %2 Population mean of %3 ", col1Name, UTF8_QSTRING("≤"), col2Name), "blue");
			printLine(1, i18n("Alternate Hypothesis: Population mean of %1 %2 Population mean of %3 ", col1Name, UTF8_QSTRING(">"), col2Name), "blue");
			break;
		}
		case HypothesisTest::Test::Tail::Two: {
			pValue = 2.*gsl_cdf_gaussian_P(value - mean, sp);
			printLine(0, i18n("Null Hypothesis: Population mean of %1 %2 Population mean of %3 ", col1Name, UTF8_QSTRING("="), col2Name), "blue");
			printLine(1, i18n("Alternate Hypothesis: Population mean of %1 %2 Population mean of %3 ", col1Name, UTF8_QSTRING("≠"), col2Name), "blue");
			break;
		}
		}
		break;
	}
	case HypothesisTest::Test::Type::Anova:
		break;
	case HypothesisTest::Test::Type::NoneType:
		break;
	}

	if (pValue > 1)
		return 1;
	return pValue;
}

QString HypothesisTestPrivate::getHtmlTable(int row, int column, QVariant* rowMajor) {
	if (row < 1 || column < 1)
		return QString();

	QString table;
	table = "<style type=text/css>"
			".tg  {border-collapse:collapse;border-spacing:0;border:none;border-color:#ccc;}"
			".tg td{font-family:Arial, sans-serif;font-size:14px;padding:10px 5px;border-style:solid;border-width:0px;overflow:hidden;word-break:normal;border-color:#ccc;color:#333;background-color:#fff;}"
			".tg th{font-family:Arial, sans-serif;font-size:14px;font-weight:normal;padding:10px 5px;border-style:solid;border-width:0px;overflow:hidden;word-break:normal;border-color:#ccc;color:#333;background-color:#f0f0f0;}"
			".tg .tg-0pky{border-color:inherit;text-align:left;vertical-align:top}"
			".tg .tg-btxf{background-color:#f9f9f9;border-color:inherit;text-align:left;vertical-align:top}"
			"</style>"
			"<table class=tg>"
			"  <tr>";

	QString bg = "tg-0pky";
	bool pky = true;

	QString element;
	table += "  <tr>";
	for (int j = 0; j < column; j++) {
		element = rowMajor[j].toString();
		table += i18n("    <th class=%1><b>%2</b></th>", bg, element);
	}
	table += "  </tr>";

	if (pky)
		bg = "tg-0pky";
	else
		bg = "tg-btxf";
	pky = !pky;

	for (int i = 1; i < row; i++) {
		table += "  <tr>";

		QString element = rowMajor[i*column].toString();
		table += i18n("    <td class=%1><b>%2</b></td>", bg, element);
		for (int j = 1; j < column; j++) {
			QString element = rowMajor[i*column+j].toString();
			table += i18n("    <td class=%1>%2</td>", bg, element);
		}

		table += "  </tr>";
		if (pky)
			bg = "tg-0pky";
		else
			bg = "tg-btxf";
		pky = !pky;
	}
	table +=  "</table>";

	return table;
}

QString HypothesisTestPrivate::getLine(const QString& msg, const QString& color) {
	return i18n("<p style=color:%1;>%2</p>", color, msg);
}

void HypothesisTestPrivate::printLine(const int& index, const QString& msg, const QString& color) {
	if (index < 0 || index >= 10)
		return;

	resultLine[index]->setText(getLine(msg, color));
	return;
}

void HypothesisTestPrivate::printTooltip(const int &index, const QString &msg) {
	if (index < 0 || index >= 10)
		return;

	resultLine[index]->setToolTip(i18n("%1", msg));
}

void HypothesisTestPrivate::printError(const QString& errorMsg) {
	printLine(0, errorMsg, "red");
}

void HypothesisTestPrivate::clearSummaryLayout() {
	for (int i = 0; i < 10; i++)
		resultLine[i]->clear();
}

void HypothesisTestPrivate::clearTestView() {
	statsTable = "";
	clearSummaryLayout();
}


/**********************************************************************************
 *                      virtual functions implementations
 * ********************************************************************************/

/*!
  Saves as XML.
 */
void HypothesisTest::save(QXmlStreamWriter* writer) const {
	writer->writeStartElement("hypothesisTest");
	writeBasicAttributes(writer);
	writeCommentElement(writer);
	//TODO:

	writer->writeEndElement();
}

/*!
  Loads from XML.
*/
bool HypothesisTest::load(XmlStreamReader* reader, bool preview) {
	Q_UNUSED(preview);
	if (!readBasicAttributes(reader))
		return false;

	//TODO:

	return !reader->hasError();
}

Spreadsheet *HypothesisTest::dataSourceSpreadsheet() const {
	return d->dataSourceSpreadsheet;
}


bool HypothesisTest::exportView() const {
	return true;
}

bool HypothesisTest::printView() {
	return true;
}

bool HypothesisTest::printPreview() const {
	return true;
}

/*! Constructs a primary view on me.
  This method may be called multiple times during the life time of an Aspect, or it might not get
  called at all. Aspects must not depend on the existence of a view for their operation.
*/
QWidget* HypothesisTest::view() const {
	if (!m_partView) {
		m_view = new HypothesisTestView(const_cast<HypothesisTest*>(this));
		m_partView = m_view;
	}
	return m_partView;
}

/*!
  Returns a new context menu. The caller takes ownership of the menu.
*/
QMenu* HypothesisTest::createContextMenu() {
	QMenu* menu = AbstractPart::createContextMenu();
	//    Q_ASSERT(menu);
	//    emit requestProjectContextMenu(menu);
	return menu;
}
