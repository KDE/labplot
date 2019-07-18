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
#include <QtMath>
#include <QQueue>

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

QMap<QString, QString> HypothesisTest::tooltips() {
    return d->tooltips;
}

void HypothesisTest::performTest(Test test, bool categoricalVariable, bool equalVariance) {
	d->tailType = test.tail;
    d->pValue.clear();
    d->statisticValue.clear();
    d->statsTable = "";
    d->tooltips.clear();
    for (int i = 0; i < 10; i++)
        d->resultLine[i]->clear();

	switch (test.subtype) {
	case HypothesisTest::Test::SubType::TwoSampleIndependent: {
        d->currTestName = "<h2>" + i18n("Two Sample Independent Test") + "</h2>";
		d->performTwoSampleIndependentTest(test.type, categoricalVariable, equalVariance);
		break;
	}
	case HypothesisTest::Test::SubType::TwoSamplePaired:
        d->currTestName = "<h2>" + i18n("Two Sample Paired Test") + "</h2>";
		d->performTwoSamplePairedTest(test.type);
		break;
	case HypothesisTest::Test::SubType::OneSample: {
        d->currTestName = "<h2>" + i18n("One Sample Test") + "</h2>";
		d->performOneSampleTest(test.type);
		break;
	}
	case HypothesisTest::Test::SubType::OneWay: {
        d->currTestName = "<h2>" + i18n("One Way Anova") + "</h2>";
		d->performOneWayAnova();
		break;
	}
    case HypothesisTest::Test::SubType::TwoWay: {
        d->currTestName = "<h2>" + i18n("Two Way Anova") + "</h2>";
        d->performTwoWayAnova();
        break;
    }
	case HypothesisTest::Test::SubType::NoneSubType:
		break;
	}

    emit changed();
}

void HypothesisTest::performLeveneTest(bool categoricalVariable) {
    d->currTestName = "<h2>" + i18n("Levene Test for Equality of Variance") + "</h2>";
	d->performLeveneTest(categoricalVariable);
	emit changed();
}

QList<double> HypothesisTest::statisticValue() {
	return d->statisticValue;
}

QList<double> HypothesisTest::pValue() {
	return d->pValue;
}

QVBoxLayout* HypothesisTest::summaryLayout() {
	return d->summaryLayout;
}

/******************************************************************************
 *                      Private Implementations
 * ****************************************************************************/
//TODO: backend of z test;
//TODO: add tooltip to tables. (currently it is not possible to use with QTextDocument);

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
//	rowCount = dataSourceSpreadsheet->rowCount();
//	columnCount = dataSourceSpreadsheet->columnCount();

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
            if (n[i] == 0) {
                printError("Atleast two values should be there in every column");
                return;
            }
            if (std[i] == 0) {
                printError(i18n("Standard Deviation of atleast one column is equal to 0: last column is: %1", columns[i]->name()));
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

    for (int i = 0; i < 2; i++) {
        if (n[i] == 0) {
            printError("Atleast two values should be there in every column");
            return;
        }
        if (std[i] == 0) {
            printError( i18n("Standard Deviation of atleast one column is equal to 0: last column is: %1", columns[i]->name()));
            return;
        }
    }

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
            statisticValue.append((mean[0] - mean[1]) / (sp * qSqrt(1.0/n[0] + 1.0/n[1])));
			printLine(9, "<b>Assumption:</b> Equal Variance b/w both population means");
		} else {
			double temp_val;
			temp_val = gsl_pow_2( gsl_pow_2(std[0]) / n[0] + gsl_pow_2(std[1]) / n[1]);
			temp_val = temp_val / ( (gsl_pow_2( (gsl_pow_2(std[0]) / n[0]) ) / (n[0]-1)) +
					(gsl_pow_2( (gsl_pow_2(std[1]) / n[1]) ) / (n[1]-1)));
			df = qRound(temp_val);

            statisticValue.append((mean[0] - mean[1]) / (qSqrt( (gsl_pow_2(std[0])/n[0]) +
                    (gsl_pow_2(std[1])/n[1]))));
			printLine(9, "<b>Assumption:</b> UnEqual Variance b/w both population means");
		}

		printLine(8, "<b>Assumption:</b> Both Populations approximately follow normal distribution");
		break;
	}
	case HypothesisTest::Test::Type::ZTest: {
		testName = "Z";
        sp = qSqrt( ((n[0]-1) * gsl_pow_2(std[0]) + (n[1]-1) * gsl_pow_2(std[1])) / df);
        statisticValue.append((mean[0] - mean[1]) / (sp * qSqrt( 1.0 / n[0] + 1.0 / n[1])));
//        pValue.append(gsl_cdf_gaussian_P(statisticValue, sp));
		break;
	}
	case HypothesisTest::Test::Type::Anova:
	case HypothesisTest::Test::Type::NoneType:
		break;
	}

    currTestName = "<h2>" + i18n("Two Sample Independent %1 Test for %2 vs %3", testName, col1Name, col2Name) + "</h2>";
    pValue.append(getPValue(test, statisticValue[0], col1Name, col2Name, (mean[0] - mean[1]), sp, df));

	printLine(2, i18n("Significance level is %1", round(significanceLevel)), "blue");

    printLine(4, i18n("%1 Value is %2 ", testName, round(statisticValue[0])), "green");
	printTooltip(4, i18n("More is the |%1-value|, more safely we can reject the null hypothesis", testName));

    printLine(5, i18n("P Value is %1 ", pValue[0]), "green");

	printLine(6, i18n("Degree of Freedom is %1", df), "green");
	printTooltip(6, i18n("Number of independent Pieces of information that went into calculating the estimate"));

    if (pValue[0] <= significanceLevel)
		printTooltip(5, i18n("We can safely reject Null Hypothesis for significance level %1", round(significanceLevel)));
	else
		printTooltip(5, i18n("There is a plausibility for Null Hypothesis to be true"));
	return;
}

/********************************Two Sample Paired ***************************************/

void HypothesisTestPrivate::performTwoSamplePairedTest(HypothesisTest::Test::Type test) {
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


	QVariant rowMajor[] = {"", "N", "Sum", "Mean", "Std",
						   "difference", n, sum, mean, std
						  };

	statsTable = getHtmlTable(2, 5, rowMajor);

	if (std == 0) {
	    printError("Standard deviation of the difference is 0");
	    return;
	}


	QString testName;
	int df = 0;

	switch (test) {
	case HypothesisTest::Test::Type::TTest: {
        statisticValue[0] = mean / (std / qSqrt(n));
		df = n - 1;
		testName = "T";
		printLine(6, i18n("Degree of Freedom is %1</p", df), "green");
		break;
	}
	case HypothesisTest::Test::Type::ZTest: {
		testName = "Z";
        statisticValue[0] = mean / (std / qSqrt(n));
		df = n - 1;
		break;
	}
	case HypothesisTest::Test::Type::Anova:
		break;
	case HypothesisTest::Test::Type::NoneType:
		break;

	}

    pValue.append(getPValue(test, statisticValue[0], columns[0]->name(), i18n("%1", populationMean), mean, std, df));
    currTestName = "<h2>" + i18n("One Sample %1 Test for %2 vs %3", testName, columns[0]->name(), columns[1]->name()) + "</h2>";

	printLine(2, i18n("Significance level is %1 ", round(significanceLevel)), "blue");
    printLine(4, i18n("%1 Value is %2 ", testName, round(statisticValue[0])), "green");
    printLine(5, i18n("P Value is %1 ", pValue[0]), "green");

    if (pValue[0] <= significanceLevel)
		printTooltip(5, i18n("We can safely reject Null Hypothesis for significance level %1", significanceLevel));
	else
		printTooltip(5, i18n("There is a plausibility for Null Hypothesis to be true"));

	return;
}

/******************************** One Sample ***************************************/

void HypothesisTestPrivate::performOneSampleTest(HypothesisTest::Test::Type test) {
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
	case ErrorEmptyColumn: {
		printError("column is empty");
		return;
	}
	case NoError:
		break;
	case ErrorUnqualSize: {
		return;
	}
	}

	QVariant rowMajor[] = {"", "N", "Sum", "Mean", "Std",
						   columns[0]->name(), n, sum, mean, std
						  };

	statsTable = getHtmlTable(2, 5, rowMajor);

	if (std == 0) {
	    printError("Standard deviation is 0");
	    return;
	}


	QString testName;
	int df = 0;

	switch (test) {
	case HypothesisTest::Test::Type::TTest: {
		testName = "T";
        statisticValue.append((mean - populationMean) / (std / qSqrt(n)));
		df = n - 1;
		printLine(6, i18n("Degree of Freedom is %1", df), "blue");
		break;
	}
	case HypothesisTest::Test::Type::ZTest: {
		testName = "Z";
		df = 0;
        statisticValue.append((mean - populationMean) / (std / qSqrt(n)));
		break;
	}
	case HypothesisTest::Test::Type::Anova:
	case HypothesisTest::Test::Type::NoneType:
		break;
	}

    pValue.append(getPValue(test, statisticValue[0], columns[0]->name(), i18n("%1",populationMean), mean - populationMean, std, df));
    currTestName = "<h2>" + i18n("One Sample %1 Test for %2", testName, columns[0]->name()) + "</h2>";

	printLine(2, i18n("Significance level is %1", round(significanceLevel)), "blue");
    printLine(4, i18n("%1 Value is %2", testName, round(statisticValue[0])), "green");
    printLine(5, i18n("P Value is %1", pValue[0]), "green");

    if (pValue[0] <= significanceLevel)
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
    statisticValue.append(msB / msW);


    pValue.append(nsl_stats_fdist_p(statisticValue[0], static_cast<size_t>(np-1), fW));

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
		rowMajor[row_i*columnCount + 3]	= mean[row_i - 1];
		rowMajor[row_i*columnCount + 4]	= std[row_i - 1];
	}

    statsTable = "<h3>" + i18n("Group Summary Statistics") + "</h3>";

	statsTable += getHtmlTable(rowCount, columnCount, rowMajor);

	statsTable += getLine("");
	statsTable += getLine("");
    statsTable += "<h3>" + i18n("Grand Summary Statistics") + "</h3>";
	statsTable += getLine("");
	statsTable += getLine(i18n("Overall Mean is %1", round(yBar)));

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

    printLine(1, i18n("F Value is %1", round(statisticValue[0])), "green");
    printLine(2, i18n("P Value is %1 ", pValue[0]), "green");

    if (pValue[0] <= significanceLevel)
		printTooltip(2, i18n("We can safely reject Null Hypothesis for significance level %1", significanceLevel));
	else
		printTooltip(2, i18n("There is a plausibility for Null Hypothesis to be true"));

    return;
}

/*************************************Two Way Anova***************************************/

// all formulas and symbols are taken from: http://statweb.stanford.edu/~susan/courses/s141/exanova.pdf

//TODO: suppress warning of variable length array are a C99 feature.
//TODO: add assumptions verification option
//TODO: add tail option (if needed)
void HypothesisTestPrivate::performTwoWayAnova() {
    int np_a, totalRows_a;
    int np_b, totalRows_b;
    countPartitions(columns[0], np_a, totalRows_a);
    countPartitions(columns[1], np_b, totalRows_b);

    double groupMean[np_a][np_b];
    int replicates[np_a][np_b];

    for (int i = 0; i < np_a; i++)
        for (int j = 0; j < np_b; j++) {
            groupMean[i][j] = 0;
            replicates[i][j] = 0;
        }

    if (totalRows_a != totalRows_b) {
        printError("There is missing data in atleast one of the rows");
        return;
    }

    QMap<QString, int> catToNumber_a;
    QMap<QString, int> catToNumber_b;

    int partitionNumber_a = 1;
    int partitionNumber_b = 1;
    for (int i = 0; i < totalRows_a; i++) {
        QString name_a = columns[0]->textAt(i);
        QString name_b = columns[1]->textAt(i);
        double value = columns[2]->valueAt(i);

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
                printError("Dataset should have atleast one data value corresponding to each feature combination");
                return;
            }
            if (replicates[i][j] != replicate) {
                printError("Number of experiments perfomed for each combination of levels <br/>"
                           "between Independet Var.1 and Independent Var.2 must be equal");
                return;
            }
            groupMean[i][j] /= replicates[i][j];
        }

//    for (int i = 0; i < np_a; i++)
//        for (int j = 0; j < np_b; j++)
//            groupMean[i][j] = int(groupMean[i][j]);

    double ss_within = 0;
    for (int i = 0; i < totalRows_a; i++) {
        QString name_a = columns[0]->textAt(i);
        QString name_b = columns[1]->textAt(i);
        double value = columns[2]->valueAt(i);

        ss_within += gsl_pow_2(value - groupMean[catToNumber_a[name_a] - 1][catToNumber_b[name_b] - 1]);
    }

    int df_within = (replicate - 1) * np_a * np_b;
    double ms_within = ss_within / df_within;

    double mean_a[np_a];
    double mean_b[np_b];
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

    QString partitionNames_a[np_a];
    QString partitionNames_b[np_b];

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
    // cell constructor structure; data, level, rowSpanCount, columnSpanCount, isHeader;
    QList<Cell*> rowMajor;
    rowMajor.append(new Cell("", 0, true, "", 2, 1));
    for (int i = 0; i < np_b; i++)
        rowMajor.append(new Cell(partitionNames_b[i], 0, true, "", 1, 2));
    rowMajor.append(new Cell("Mean", 0, true, "", 2));

    for (int i = 0; i < np_b; i++) {
        rowMajor.append(new Cell("Mean", 1, true));
        rowMajor.append(new Cell("Replicate", 1, true));
    }

    int level = 2;
    for (int i = 0; i < np_a; i++) {
        rowMajor.append(new Cell(partitionNames_a[i], level, true));
        for (int j = 0; j < np_b; j++) {
            rowMajor.append(new Cell(round(groupMean[i][j]), level));
            rowMajor.append(new Cell(replicates[i][j], level));
        }
        rowMajor.append(new Cell(round(mean_a[i]), level));
        level++;
    }

    rowMajor.append(new Cell("Mean", level, true));
    for (int i = 0; i < np_b; i++)
        rowMajor.append(new Cell(round(mean_b[i]), level, false, "", 1, 2));
    rowMajor.append(new Cell(round(mean), level));

    statsTable = "<h3>" + i18n("Contingency Table") + "</h3>";
    statsTable += getHtmlTable3(rowMajor);

    statsTable += "</br>";
    statsTable += "<h3>" + i18n("results table") + "</h3>";

    rowMajor.clear();
    level = 0;
    rowMajor.append(new Cell("", level, true));
    rowMajor.append(new Cell("SS", level, true));
    rowMajor.append(new Cell("DF", level, true, "degree of freedom"));
    rowMajor.append(new Cell("MS", level, true));

    level++;
    rowMajor.append(new Cell(columns[0]->name(), level, true));
    rowMajor.append(new Cell(round(ss_a), level));
    rowMajor.append(new Cell(df_a, level));
    rowMajor.append(new Cell(round(ms_a), level));

    level++;
    rowMajor.append(new Cell(columns[1]->name(), level, true));
    rowMajor.append(new Cell(round(ss_b), level));
    rowMajor.append(new Cell(df_b, level));
    rowMajor.append(new Cell(round(ms_b), level));

    level++;
    rowMajor.append(new Cell("Interaction", level, true));
    rowMajor.append(new Cell(round(ss_interaction), level));
    rowMajor.append(new Cell(df_interaction, level));
    rowMajor.append(new Cell(round(ms_interaction), level));

    level++;
    rowMajor.append(new Cell("Within", level, true));
    rowMajor.append(new Cell(round(ss_within), level));
    rowMajor.append(new Cell(df_within, level));
    rowMajor.append(new Cell(round(ms_within), level));

    statsTable += getHtmlTable3(rowMajor);

    double fValue_a = ms_a / ms_within;
    double fValue_b = ms_b / ms_within;
    double fValue_interaction = ms_interaction / ms_within;

    double pValue_a = nsl_stats_fdist_p(fValue_a, static_cast<size_t>(np_a - 1), df_a);
    double pValue_b = nsl_stats_fdist_p(fValue_b, static_cast<size_t>(np_b - 1), df_b);

    printLine(0, "F(df<sub>" + columns[0]->name() + "</sub>, df<sub>within</sub>) is " + round(fValue_a), "blue");
    printLine(1, "F(df<sub>" + columns[1]->name() + "</sub>, df<sub>within</sub>) is " + round(fValue_b), "blue");
    printLine(2, "F(df<sub>interaction</sub>, df<sub>within</sub>) is " + round(fValue_interaction), "blue");

    printLine(4, "P(df<sub>" + columns[0]->name() + "</sub>, df<sub>within</sub>) is " + round(pValue_a), "blue");
    printLine(5, "P(df<sub>" + columns[1]->name() + "</sub>, df<sub>within</sub>) is " + round(pValue_b), "blue");
//    printLine(2, "P(df<sub>interaction</sub>, df<sub>within</sub>) is " + round(fValue_interaction), "blue");

    statisticValue.append(fValue_a);
    statisticValue.append(fValue_b);
    statisticValue.append(fValue_interaction);

    pValue.append(pValue_a);
    pValue.append(pValue_b);

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
void HypothesisTestPrivate::performLeveneTest(bool categoricalVariable) {
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
		printError("Select atleast two columns / classes");
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
            else {
                printError("One of the selected columns is empty");
                return;
            }
		}

		for (int j = 0; j < totalRows; j++) {
			for (int i = 0; i < np; i++) {
				value = columns[i]->valueAt(j);
				if (!(std::isnan(value)))
					ziBar[i] += fabs(value - yiBar[i]);
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
					double zij = fabs(value - yiBar[i]);
					denominatorValue += gsl_pow_2( (zij - ziBar[i]));
				}
			}
		}


        if (denominatorValue <= 0) {
            printError( i18n("Denominator value is %1", denominatorValue));
            return;
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
            else {
                printError("One of the selected columns is empty");
                return;
            }
		}

		for (int j = 0; j < n; j++) {
			name = columns[0]->textAt(j);
			value = columns[1]->valueAt(j);
			classIndex = classnameToIndex[name] - 1;
			ziBar[classIndex] += fabs(value - yiBar[classIndex]);
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
			double zij = fabs(value - yiBar[classIndex]);
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

    pValue.append(nsl_stats_fdist_p(fValue, static_cast<size_t>(np-1), df));

	printLine(0, "Null Hypothesis: Variance is equal between all classes", "blue");
	printLine(1, "Alternate Hypothesis: Variance is not equal in at-least one pair of classes", "blue");
	printLine(2, i18n("Significance level is %1", round(significanceLevel)), "blue");
	printLine(4, i18n("F Value is %1 ", round(fValue)), "green");
    printLine(5, i18n("P Value is %1 ", pValue[0]), "green");
	printLine(6, i18n("Degree of Freedom is %1", df), "green");

    if (pValue[0] <= significanceLevel) {
		printTooltip(5, i18n("We can safely reject Null Hypothesis for significance level %1", significanceLevel));
		printLine(8, "Requirement for homogeneity is not met", "red");
	} else {
		printTooltip(5, i18n("There is a plausibility for Null Hypothesis to be true"));
		printLine(8, "Requirement for homogeneity is met", "green");
	}

    statisticValue.append(fValue);
	return;
}

/**************************************Spearman Correlation Test*****************************/

// N: Number of observations

//TODO: add functionality where col1 contains categorical variables.
void HypothesisTestPrivate::performSpearmanCorrelation() {
    int N = columns[0]->rowCount();

    QString col1Name = columns[0]->name();
    QString col2Name = columns[1]->name();

    currTestName = i18n("Pearson r Correlation Test between %1 and %2", col1Name, col2Name);
    double sumXY = 0;
    double sumX = 0;
    double sumY = 0;
    double sumXSq = 0;
    double sumYSq = 0;
    for (int i = 0; i < N; i++) {
        double valueCol1 = columns[0]->valueAt(i);
        double valueCol2 = columns[1]->valueAt(i);

        if (std::isnan(valueCol1) || std::isnan(valueCol2)) {
            if (std::isnan(valueCol1) && std::isnan(valueCol2)) {
                N = i;
                break;
            }
            printError(i18n("Number of values in %1 and %2 are not equal", col1Name, col2Name));
            return;
        }

        sumXY += valueCol1 * valueCol2;
        sumX += valueCol1;
        sumY += valueCol2;
        sumXSq += gsl_pow_2(valueCol1);
        sumYSq += gsl_pow_2(valueCol2);
    }

    QList<Cell*> rowMajor;
    int level = 0;
    rowMajor.append(new Cell("", level, true));
    rowMajor.append(new Cell("N", level, true, "Total Number of Observations"));
    rowMajor.append(new Cell("Sum " + col1Name, level, true, "Sum of Values in " + col1Name));
    rowMajor.append(new Cell("Sum " + col1Name + "<sup>2</sup>", level, true, "Sum of Square of Values in " + col1Name));
    rowMajor.append(new Cell("Sum " + col2Name, level, true, "Sum of Values in " + col2Name));
    rowMajor.append(new Cell("Sum " + col2Name + "<sup>2</sup>", level, true, "Sum of Square of Values in " + col2Name));
    rowMajor.append(new Cell("Sum " + col1Name + "x" + col2Name, level, true, "Sum of product of paired scores"));

    level++;
    rowMajor.append(new Cell("Results", level, true));
    rowMajor.append(new Cell(N, level));
    rowMajor.append(new Cell(sumX, level));
    rowMajor.append(new Cell(sumXSq, level));
    rowMajor.append(new Cell(sumY, level));
    rowMajor.append(new Cell(sumYSq, level));
    rowMajor.append(new Cell(sumXY, level));

    statsTable = "<h3> " + i18n("Statistic Table") + "</h3>";
    statsTable += getHtmlTable3(rowMajor);

    statisticValue.append((N * sumXY - sumX * sumY) /
                            qSqrt((N * sumXSq - gsl_pow_2(sumX)) *
                                    (N * sumYSq - gsl_pow_2(sumY))));

    printLine(0, "Correlation Value is " + round(statisticValue[0]), "blue");
}

/***************************************Helper Functions*************************************/

QString HypothesisTestPrivate::round(QVariant number, int precision) {
    if (number.userType() == QMetaType::Double || number.userType() == QMetaType::Float) {
        double multiplierPrecision = qPow(10, precision);
        int tempNum = int(number.toDouble()*multiplierPrecision*10);

        if (tempNum % 10 < 5)
            return QString::number((tempNum/10) / multiplierPrecision);
        else
            return QString::number((tempNum/10 + 1) / multiplierPrecision);
    }
    return i18n("%1", number.toString());
}


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
//       pValue.append(2*gsl_cdf_tdist_P(value, df) v/s
//       pValue.append(gsl_cdf_tdis_P(value, df) + gsl_cdf_tdis_P(-value, df);
double HypothesisTestPrivate::getPValue(const HypothesisTest::Test::Type& test, double& value, const QString& col1Name, const QString& col2Name, const double mean, const double sp, const int df) {

	switch (test) {
	case HypothesisTest::Test::Type::TTest: {
		switch (tailType) {
		case HypothesisTest::Test::Tail::Negative: {
            pValue.append(gsl_cdf_tdist_P(value, df));
			printLine(0, i18n("Null Hypothesis: Population mean of %1 %2 Population mean of %3", col1Name, UTF8_QSTRING("≥"), col2Name), "blue");
			printLine(1, i18n("Alternate Hypothesis: Population mean of %1 %2 Population mean of %3", col1Name, UTF8_QSTRING("⋖"), col2Name), "blue");
			break;
		}
		case HypothesisTest::Test::Tail::Positive: {
			value *= -1;
            pValue.append(gsl_cdf_tdist_P(value, df));
			printLine(0, i18n("Null Hypothesis: Population mean of %1 %2 Population mean of %3", col1Name, UTF8_QSTRING("≤"), col2Name), "blue");
			printLine(1, i18n("Alternate Hypothesis: Population mean of %1 %2 Population mean of %3", col1Name, UTF8_QSTRING(">"), col2Name), "blue");
			break;
		}
		case HypothesisTest::Test::Tail::Two: {
            pValue.append(2.*gsl_cdf_tdist_P(-fabs(value), df));

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
            pValue.append(gsl_cdf_gaussian_P(value - mean, sp));
			printLine(0, i18n("Null Hypothesis: Population mean of %1 %2 Population mean of %3 ", col1Name, UTF8_QSTRING("≥"), col2Name), "blue");
			printLine(1, i18n("Alternate Hypothesis: Population mean of %1 %2 Population mean of %3 ", col1Name, UTF8_QSTRING("⋖"), col2Name), "blue");
			break;
		}
		case HypothesisTest::Test::Tail::Positive: {
			value *= -1;
            pValue.append(nsl_stats_tdist_p(value - mean, sp));
			printLine(0, i18n("Null Hypothesis: Population mean of %1 %2 Population mean of %3 ", col1Name, UTF8_QSTRING("≤"), col2Name), "blue");
			printLine(1, i18n("Alternate Hypothesis: Population mean of %1 %2 Population mean of %3 ", col1Name, UTF8_QSTRING(">"), col2Name), "blue");
			break;
		}
		case HypothesisTest::Test::Tail::Two: {
            pValue.append(2.*gsl_cdf_gaussian_P(value - mean, sp));
			printLine(0, i18n("Null Hypothesis: Population mean of %1 %2 Population mean of %3 ", col1Name, UTF8_QSTRING("="), col2Name), "blue");
			printLine(1, i18n("Alternate Hypothesis: Population mean of %1 %2 Population mean of %3 ", col1Name, UTF8_QSTRING("≠"), col2Name), "blue");
			break;
		}
		}
		break;
	}
	case HypothesisTest::Test::Type::Anova:
	case HypothesisTest::Test::Type::NoneType:
		break;
	}

    if (pValue[0] > 1)
        return 1;
    return pValue[0];
}

int HypothesisTestPrivate::setSpanValues(HypothesisTestPrivate::Node* root, int& totalLevels) {
    if (root == nullptr) {
        totalLevels = 0;
        return 0;
    }

    int val = 0;
    int level = 0;
    int maxLevel = 0;
    for (int i = 0; i < root->children.size(); i++) {
        val += setSpanValues(root->children[i], level);
        maxLevel = std::max(level, maxLevel);
    }

    totalLevels = maxLevel + 1;
    if (val == 0)
        root->spanCount = 1;
    else
        root->spanCount = val;

    return root->spanCount;

}

QString HypothesisTestPrivate::getHtmlHeader(HypothesisTestPrivate::Node *root) {
    if (root == nullptr)
        return QString();

    QString header;

    int totalLevels = 0;

    setSpanValues(root, totalLevels);
    totalLevels -= 1;

    root->level = 0;
    QQueue<Node*> nodeQueue;

    for (int i = 0; i < root->children.size(); i++) {
        Node* child = root->children[i];
        child->level = 1;
        nodeQueue.enqueue(child);
    }

    int prevLevel = 1;
    header = "  <tr>";
    header += "    <td rowspan=" + QString::number(totalLevels) + "></td>";

    while(!nodeQueue.isEmpty()) {
        Node* node = nodeQueue.dequeue();
        int nodeLevel = node->level;

        for (int i = 0; i < node->children.size(); i++) {
            Node* child = node->children[i];
            child->level = nodeLevel + 1;
            nodeQueue.enqueue(child);
        }

        if (nodeLevel != prevLevel) {
            prevLevel = nodeLevel;
            header += "  </tr>";
            header += "  <tr>";
        }

        header += "    <th colspan=" + QString::number(node->spanCount) + ">" + node->data + "</th>";
    }
    header += "  </tr>";
    return header;
}

QString HypothesisTestPrivate::getHtmlTable2(int rowCount, int columnCount, Node* columnHeaderRoot, QVariant* rowMajor) {
    if (rowCount < 1 || columnCount < 1)
        return QString();

    QString table;

    table = "<style type=text/css>"
            ".tg  {border-collapse:collapse;border-spacing:0;border:1px;border-color:#ccc;}"
            ".tg td{font-family:Arial, sans-serif;font-size:14px;padding:10px 5px;border-style:solid;border-width:0px;overflow:hidden;word-break:normal;border-color:#ccc;color:#333;background-color:#fff;}"
            ".tg th{font-family:Arial, sans-serif;font-size:14px;font-weight:normal;padding:10px 5px;border-style:solid;border-width:0px;overflow:hidden;word-break:normal;border-color:#ccc;color:#333;background-color:#f0f0f0;}"
            "</style>"
            "<table class=tg>";
    table += getHtmlHeader(columnHeaderRoot);

    for (int i = 0; i < rowCount; i++) {
        table += "  <tr>";
        table += "    <th>" + round(rowMajor[i*columnCount]) + "</th>";
        for (int j = 1; j < columnCount; j++)
            table += "    <td>" + round(rowMajor[i*columnCount + j]) + "</td>";
        table += "  </tr>";
    }

    table += "</table>";
    return table;
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
        table += "    <th class=" + bg + "><b>" + i18n("%1", element) + "</b></th>";
    }
    table += "  </tr>";

    if (pky)
        bg = "tg-0pky";
    else
        bg = "tg-btxf";
    pky = !pky;

    for (int i = 1; i < row; i++) {
        table += "  <tr>";

        QString element = round(rowMajor[i*column]);
        table += "    <td class=" + bg + "><b>" + i18n("%1", element) + "</b></td>";
        for (int j = 1; j < column; j++) {
            element = round(rowMajor[i*column+j]);
            table += "    <td class=" + bg + ">" + i18n("%1", element) + "</td>";
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

QString HypothesisTestPrivate::getHtmlTable3(const QList<HypothesisTestPrivate::Cell*>& rowMajor) {
    tooltips.clear();
    int rowMajorSize = rowMajor.size();

    if (rowMajorSize == 0)
        return QString();

    QString table;
    table = "<style type=text/css>"
            ".tg  {border-collapse:collapse;border: 1px solid black;}"
            ".tg td{font-family:Arial, sans-serif;font-size:14px;padding:10px 5px;border: 1px solid black;overflow:hidden;word-break:normal;color:#333;background-color:#fff;}"
            ".tg th{font-family:Arial, sans-serif;font-size:14px;font-weight:normal;padding:10px 5px;border: 1px solid black;overflow:hidden;word-break:normal;color:#333;background-color:#f0f0f0;}"
            "</style>";

    table += "<table class=tg>";

    table += "  <tr>";
    int prevLevel = 0;
    for (int i = 0; i < rowMajorSize; i++) {
        Cell* currCell = rowMajor[i];
        if (currCell->level != prevLevel) {
            table += "  </tr>";
            table += "  <tr>";
            prevLevel = currCell->level;
        }
        QString cellStartTag = "<td ";
        QString cellEndTag = "</td>";

        if (currCell->isHeader) {
            cellStartTag = "<th ";
            cellEndTag = "</th>";
        }

        table += cellStartTag +
                "rowspan=" + QString::number(currCell->rowSpanCount) +
                " " +
                "colspan=" + QString::number(currCell->columnSpanCount) +
                ">" +
                i18n("%1", currCell->data) +
                cellEndTag;

        if (!currCell->tooltip.isEmpty())
            tooltips.insert(currCell->data, currCell->tooltip);
    }
    table += "  <tr>";
    table += "</table>";
    return table;
}

QString HypothesisTestPrivate::getLine(const QString& msg, const QString& color) {
    return "<p style=color:" + color + ";>" + i18n("%1", msg) + "</p>";
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

	writer->writeEndElement();
}

/*!
  Loads from XML.
*/
bool HypothesisTest::load(XmlStreamReader* reader, bool preview) {
	Q_UNUSED(preview);
	if (!readBasicAttributes(reader))
		return false;

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
