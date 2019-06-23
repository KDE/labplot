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
#include "kdefrontend/hypothesis_test/HypothesisTestView.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/core/column/Column.h"
#include "backend/lib/macros.h"
#include "QDebug"

extern "C" {
#include "backend/nsl/nsl_stats.h"
}

#include <QVector>
#include <QMessageBox>
#include <QtMath>
#include <KLocalizedString>
#include <QStandardItemModel>
#include <QLocale>
#include <QLabel>
#include <QLayout>
#include <gsl/gsl_cdf.h>
#include <gsl/gsl_math.h>

HypothesisTest::HypothesisTest(const QString &name) : AbstractPart(name),
    d(new HypothesisTestPrivate(this)) {
}

HypothesisTest::~HypothesisTest() {
    delete d;
}

void HypothesisTest::setDataSourceType(DataSourceType type) {
    if (type != d->dataSourceType) {
        d->dataSourceType = type;
    }
}

HypothesisTest::DataSourceType HypothesisTest::dataSourceType() const {
    return d->dataSourceType;
}

void HypothesisTest::setDataSourceSpreadsheet(Spreadsheet *spreadsheet) {
    if  (spreadsheet != d->dataSourceSpreadsheet)
        d->setDataSourceSpreadsheet(spreadsheet);
}

void HypothesisTest::setColumns(QVector<Column*> cols) {
    d->m_columns = cols;
}

void HypothesisTest::setColumns(QStringList cols) {
    return d->setColumns(cols);
}

QStringList HypothesisTest::allColumns() {
    return d->all_columns;
}

void HypothesisTest::setTailType(HypothesisTest::TailType tailType) {
    d->tail_type = tailType;
}

HypothesisTest::TailType HypothesisTest::tailType() {
    return d->tail_type;
}

void HypothesisTest::setPopulationMean(QVariant populationMean) {
    d->m_population_mean = populationMean.toDouble();
}

void HypothesisTest::setSignificanceLevel(QVariant alpha) {
    d->m_significance_level = alpha.toDouble();
}


QString HypothesisTest::testName() {
    return d->m_currTestName;
}

QString HypothesisTest::statsTable() {
    return d->m_stats_table;
}

void HypothesisTest::performTwoSampleIndependentTTest(bool categorical_variable, bool equal_variance) {
    d->m_currTestName = i18n( "<h2>Two Sample Independent T Test</h2>");
    d->performTwoSampleIndependentTest(HypothesisTestPrivate::TestT, categorical_variable, equal_variance);
}

void HypothesisTest::performTwoSamplePairedTTest() {
    d->m_currTestName = i18n( "<h2>Two Sample Paried T Test</h2>");
    d->performTwoSamplePairedTest(HypothesisTestPrivate::TestT);
}

void HypothesisTest::performOneSampleTTest() {
    d->m_currTestName = i18n( "<h2>One Sample T Test</h2>");
    d->performOneSampleTest(HypothesisTestPrivate::TestT);
}

void HypothesisTest::performTwoSampleIndependentZTest() {
    d->m_currTestName = i18n( "<h2>Two Sample Independent Z Test</h2>");
    d->performTwoSampleIndependentTest(HypothesisTestPrivate::TestZ);
}

void HypothesisTest::performTwoSamplePairedZTest() {
    d->m_currTestName = i18n( "<h2>Two Sample Paired Z Test</h2>");
    d->performTwoSamplePairedTest(HypothesisTestPrivate::TestZ);
}

void HypothesisTest::performOneSampleZTest() {
    d->m_currTestName = i18n( "<h2>One Sample Z Test</h2>");
    d->performOneSampleTest(HypothesisTestPrivate::TestZ);
}

void HypothesisTest::performOneWayAnova() {
    d->m_currTestName = i18n( "<h2>One Way Anova</h2>");
    d->performOneWayAnova();
}

void HypothesisTest::performLeveneTest(bool categorical_variable) {
    d->m_currTestName = i18n( "<h2>Levene Test for Equality of Variance</h2>");
    d->performLeveneTest(categorical_variable);
}

/******************************************************************************
 *                      Private Implementations
 * ****************************************************************************/

//TODO: round off numbers while printing

HypothesisTestPrivate::HypothesisTestPrivate(HypothesisTest* owner) : q(owner) {
}

HypothesisTestPrivate::~HypothesisTestPrivate() {
}

void HypothesisTestPrivate::setDataSourceSpreadsheet(Spreadsheet *spreadsheet) {
    dataSourceSpreadsheet = spreadsheet;

    //setting rows and columns count;
    m_rowCount = dataSourceSpreadsheet->rowCount();
    m_columnCount = dataSourceSpreadsheet->columnCount();

    for (auto* col : dataSourceSpreadsheet->children<Column>()) {
        all_columns << col->name();
    }
}


void HypothesisTestPrivate::setColumns(QStringList cols) {
    m_columns.clear();
    Column* column = new Column("column");
    for (QString col : cols) {
        if (!cols.isEmpty()) {
            column = dataSourceSpreadsheet->column(col);
            m_columns.append(column);
        }
    }
}


/**************************Two Sample Independent *************************************/

void HypothesisTestPrivate::performTwoSampleIndependentTest(TestType test,bool categorical_variable, bool equal_variance) {
    QString test_name;

    double value;
    int df = 0;
    double p_value = 0;
    double sp = 0;
    clearGlobalVariables();

    if (m_columns.size() != 2) {
        printError("Inappropriate number of columns selected");
        emit q->changed();
        return;
    }

    int n[2];
    double sum[2], mean[2], std[2];

    QString col1_name = m_columns[0]->name();
    QString col2_name = m_columns[1]->name();

    if (!categorical_variable && (m_columns[0]->columnMode() == AbstractColumn::Integer || m_columns[0]->columnMode() == AbstractColumn::Numeric)) {
        for (int i = 0; i < 2; i++) {
            findStats(m_columns[i], n[i], sum[i], mean[i], std[i]);

            if (n[i] < 1) {
                printError("At least one of selected column is empty");
                emit q->changed();
                return;
            }
        }
    } else {
        QMap<QString, int> col_name;
        int np;
        int total_rows;

        countPartitions(m_columns[0], np, total_rows);
        if (np != 2) {
            printError( i18n("Number of Categorical Variable in Column %1 is not equal to 2", m_columns[0]->name()));
            emit q->changed();
            return;
        }

        ErrorType error_code = findStatsCategorical(m_columns[0], m_columns[1], n, sum, mean, std, col_name, np, total_rows);

        switch (error_code) {
            case ErrorUnqualSize: {
                printError( i18n("Unequal size between Column %1 and Column %2", m_columns[0]->name(), m_columns[1]->name()));
                emit q->changed();
                return;
            }case ErrorEmptyColumn: {
                printError("At least one of selected column is empty");
                emit q->changed();
                return;
            } case NoError:
                break;
        }

        QMapIterator<QString, int> i(col_name);
        while (i.hasNext()) {
            i.next();
            if (i.value() == 1)
                col1_name = i.key();
            else
                col2_name = i.key();
        }
    }

    QVariant row_major[] = {"", "N", "Sum", "Mean", "Std",
                            col1_name, n[0], sum[0], mean[0], std[0],
                            col2_name, n[1], sum[1], mean[1], std[1]};

    m_stats_table = getHtmlTable(3, 5, row_major);

    switch (test) {
    case TestT: {
        test_name = "T";

        if (equal_variance) {
            df = n[0] + n[1] - 2;
            sp = qSqrt( ((n[0]-1)*gsl_pow_2(std[0]) + (n[1]-1)*gsl_pow_2(std[1]))/df);
            value = (mean[0] - mean[1])/(sp*qSqrt(1.0/n[0] + 1.0/n[1]));
            printLine(9, "<b>Assumption:</b> Equal Variance b/w both population means");
        } else {
            double temp_val;
            temp_val = gsl_pow_2( gsl_pow_2(std[0])/n[0] + gsl_pow_2(std[1])/n[1]);
            temp_val = temp_val / ( (gsl_pow_2( (gsl_pow_2(std[0])/n[0]))/(n[0]-1)) + (gsl_pow_2( (gsl_pow_2(std[1])/n[1]))/(n[1]-1)));
            df = qRound(temp_val);

            value = (mean[0] - mean[1]) / (qSqrt( (gsl_pow_2(std[0])/n[0]) + (gsl_pow_2(std[1])/n[1])));
            printLine(9, "<b>Assumption:</b> UnEqual Variance b/w both population means");
        }
        break;
    } case TestZ: {
        test_name = "Z";
        sp = qSqrt( ((n[0]-1)*gsl_pow_2(std[0]) + (n[1]-1)*gsl_pow_2(std[1]))/df);
        value = (mean[0] - mean[1])/(sp*qSqrt(1.0/n[0] + 1.0/n[1]));
        p_value = gsl_cdf_gaussian_P(value, sp);
    }        
    }

    m_currTestName = i18n( "<h2>Two Sample Independent %1 Test for %2 vs %3</h2>", test_name, col1_name, col2_name);
    p_value = getPValue(test, value, col1_name, col2_name, (mean[0] - mean[1]), sp, df);

    printLine(2, i18n("Significance level is %1", m_significance_level), "blue");
    printLine(4, i18n("%1 Value is %2 ", test_name, value), "green");
    printLine(5, i18n("P Value is %1 ", p_value), "green");
    printLine(6, i18n("Degree of Freedom is %1", df), "green");

    if (p_value <= m_significance_level)
        q->m_view->setResultLine(5, i18n("We can safely reject Null Hypothesis for significance level %1", m_significance_level), Qt::ToolTipRole);
    else
        q->m_view->setResultLine(5, i18n("There is a plausibility for Null Hypothesis to be true"), Qt::ToolTipRole);

    emit q->changed();
    return;
}

/********************************Two Sample Paired ***************************************/

void HypothesisTestPrivate::performTwoSamplePairedTest(TestType test) {
    QString test_name;
    int n;
    double sum, mean, std;
    double value;
    int df = 0;
    double p_value = 0;
    clearGlobalVariables();

    if (m_columns.size() != 2) {
        printError("Inappropriate number of columns selected");
        emit q->changed();
        return;
    }

    for (int i = 0; i < 2; i++) {
        if (!(m_columns[i]->columnMode() == AbstractColumn::Numeric || m_columns[i]->columnMode() == AbstractColumn::Integer)) {
            printError("select only columns with numbers");
            emit q->changed();
            return;
        }
    }

    ErrorType error_code = findStatsPaired(m_columns[0], m_columns[1], n, sum, mean, std);

    switch (error_code) {
        case ErrorUnqualSize: {
            printError("both columns are having different sizes");
            emit q->changed();
            return;
        } case ErrorEmptyColumn: {
            printError("columns are empty");
            emit q->changed();
            return;
        } case NoError:
            break;
    }

    if (n == -1) {
        printError("both columns are having different sizes");
        emit q->changed();
        return;
    }

    if (n < 1) {
        printError("columns are empty");
        emit q->changed();
        return;
    }

    QVariant row_major[] = {"", "N", "Sum", "Mean", "Std",
                            "difference", n, sum, mean, std};

    m_stats_table = getHtmlTable(2, 5, row_major);

    switch (test) {
    case TestT: {
        value = mean / (std/qSqrt(n));
        df = n - 1;
        test_name = "T";
        printLine(6, i18n("Degree of Freedom is %1</p", df), "green");
        break;
    } case TestZ: {
        test_name = "Z";
        value = mean / (std/qSqrt(n));
        df = n - 1;
        break;
    }}

    p_value = getPValue(test, value, m_columns[0]->name(), i18n("%1",m_population_mean), mean, std, df);
    m_currTestName = i18n( "<h2>One Sample %1 Test for %2 vs %3</h2>", test_name, m_columns[0]->name(), m_columns[1]->name());

    printLine(2, i18n("Significance level is %1 ", m_significance_level), "blue");
    printLine(4, i18n("%1 Value is %2 ", test_name, value), "green");
    printLine(5, i18n("P Value is %1 ", p_value), "green");

    if (p_value <= m_significance_level)
        q->m_view->setResultLine(5, i18n("We can safely reject Null Hypothesis for significance level %1", m_significance_level), Qt::ToolTipRole);
    else
        q->m_view->setResultLine(5, i18n("There is a plausibility for Null Hypothesis to be true"), Qt::ToolTipRole);

    emit q->changed();
    return;

}

/******************************** One Sample ***************************************/

void HypothesisTestPrivate::performOneSampleTest(TestType test) {
    QString test_name;
    double value;
    int df = 0;
    double p_value = 0;
    clearGlobalVariables();

    if (m_columns.size() != 1) {
        printError("Inappropriate number of columns selected");
        emit q->changed();
        return;
    }

    if ( !(m_columns[0]->columnMode() == AbstractColumn::Numeric || m_columns[0]->columnMode() == AbstractColumn::Integer)) {
        printError("select only columns with numbers");
        emit q->changed();
        return;
    }

    int n;
    double sum, mean, std;
    ErrorType error_code = findStats(m_columns[0], n, sum, mean, std);

    switch (error_code) {
        case ErrorUnqualSize: {
            printError("column is empty");
            emit q->changed();
            return;
        } case NoError:
            break;
        case ErrorEmptyColumn: {
            emit q->changed();
            return;
        }
    }

    QVariant row_major[] = {"", "N", "Sum", "Mean", "Std",
                            m_columns[0]->name(), n, sum, mean, std};

    m_stats_table = getHtmlTable(2, 5, row_major);

    switch (test) {
    case TestT: {
        test_name = "T";
        value = (mean - m_population_mean) / (std/qSqrt(n));
        df = n - 1;
        printLine(6, i18n("Degree of Freedom is %1", df), "blue");
        break;
    } case TestZ: {
        test_name = "Z";
        df = 0;
        value = (mean - m_population_mean) / (std/qSqrt(n));
    }}

    p_value = getPValue(test, value, m_columns[0]->name(), i18n("%1",m_population_mean), mean - m_population_mean, std, df);
    m_currTestName = i18n( "<h2>One Sample %1 Test for %2</h2>", test_name, m_columns[0]->name());

    printLine(2, i18n("Significance level is %1", m_significance_level), "blue");
    printLine(4, i18n("%1 Value is %2", test_name, value), "green");
    printLine(5, i18n("P Value is %1", p_value), "green");

    if (p_value <= m_significance_level)
        q->m_view->setResultLine(5, i18n("We can safely reject Null Hypothesis for significance level %1", m_significance_level), Qt::ToolTipRole);
    else
        q->m_view->setResultLine(5, i18n("There is a plausibility for Null Hypothesis to be true"), Qt::ToolTipRole);

    emit q->changed();
    return;

}

/*************************************One Way Anova***************************************/
// all standard variables and formulas are taken from this wikipedia page:
// https://en.wikipedia.org/wiki/One-way_analysis_of_variance
// b stands for b/w groups
// w stands for within groups
void HypothesisTestPrivate::performOneWayAnova() {
    clearGlobalVariables();
    int np, total_rows;     // np is number of partition i.e., number of classes
    countPartitions(m_columns[0], np, total_rows);

    int* ni = new int[np];
    double* sum = new double[np];
    double* mean = new double[np];
    double* std = new double[np];
    QString* col_names = new QString[np];

    QMap<QString, int> classname_to_index;
    findStatsCategorical(m_columns[0], m_columns[1], ni, sum, mean, std, classname_to_index, np, total_rows);

    double y_bar = 0;       // overall mean
    double s_b = 0;         // sum of squares of (mean - overall_mean) between the groups
    int f_b = 0;            // degree of freedom between the groups
    double ms_b = 0;        // mean sum of squares between the groups
    double s_w = 0;         // sum of squares of (value - mean of group) within the groups
    int f_w = 0;            // degree of freedom within the group
    double ms_w = 0;        // mean sum of squares within the groups
    double f_value = 0;
    double p_value = 0;

    // now finding mean of each group;

    for (int i = 0; i < np; i++)
        y_bar += mean[i];
    y_bar = y_bar / np;

    for (int i = 0; i < np; i++) {
        s_b += ni[i] * gsl_pow_2( ( mean[i] - y_bar));
        if (ni[i] > 1)
            s_w += gsl_pow_2( std[i])*(ni[i] - 1);
        else
            s_w += gsl_pow_2( std[i]);
        f_w += ni[i] - 1;
    }

    f_b = np - 1;
    ms_b = s_b / f_b;

    ms_w = s_w / f_w;
    f_value = ms_b / ms_w;


    p_value = nsl_stats_fdist_p(f_value, static_cast<size_t>(np-1), f_w);

    QMapIterator<QString, int> i(classname_to_index);
    while (i.hasNext()) {
        i.next();
        col_names[i.value()-1] = i.key();
    }

    // now printing the statistics and result;
    int row_count = np + 1, column_count = 5;
    QVariant* row_major = new QVariant[row_count*column_count];
    // header data;
    row_major[0] = ""; row_major[1] = "Ni"; row_major[2] = "Sum"; row_major[3] = "Mean"; row_major[4] = "Std";

    // table data
    for (int row_i = 1; row_i < row_count ; row_i++) {
        row_major[row_i*column_count] = col_names[row_i - 1];
        row_major[row_i*column_count + 1] = ni[row_i - 1];
        row_major[row_i*column_count + 2] = sum[row_i - 1];
        row_major[row_i*column_count + 3] = QString::number( mean[row_i - 1], 'f', 3);
        row_major[row_i*column_count + 4] = QString::number( std[row_i - 1], 'f', 3);
    }

    m_stats_table = i18n( "<h3>Group Summary Statistics</h3>");

    m_stats_table += getHtmlTable(row_count, column_count, row_major);

    m_stats_table += getLine("");
    m_stats_table += getLine("");
    m_stats_table += i18n( "<h3>Grand Summary Statistics</h3>");
    m_stats_table += getLine("");
    m_stats_table += getLine(i18n("Overall Mean is %1", y_bar));

    row_count = 4; column_count = 3;
    row_major->clear();

    row_major[0] = ""; row_major[1] = "Between Groups"; row_major[2] = "Within Groups";

    int base_index = 0;
    base_index = 1*column_count; row_major[base_index + 0] = "Sum of Squares"; row_major[base_index + 1] = s_b; row_major[base_index + 2] = s_w;
    base_index = 2*column_count; row_major[base_index + 0] = "Degree of Freedom"; row_major[base_index + 1] = f_b; row_major[base_index + 2] = f_w;
    base_index = 3*column_count; row_major[base_index + 0] = "Mean Square Value"; row_major[base_index + 1] = ms_b; row_major[base_index + 2] = ms_w;

    m_stats_table += getHtmlTable(row_count, column_count, row_major);

    printLine(1, i18n("F Value is %1", f_value), "blue");
    printLine(2, i18n("P Value is %1 ", p_value), "green");

    if (p_value <= m_significance_level)
        q->m_view->setResultLine(2, i18n("We can safely reject Null Hypothesis for significance level %1", m_significance_level), Qt::ToolTipRole);
    else
        q->m_view->setResultLine(2, i18n("There is a plausibility for Null Hypothesis to be true"), Qt::ToolTipRole);

    emit q->changed();
    return;
}

/**************************************Levene Test****************************************/
void HypothesisTestPrivate::performLeveneTest(bool categorical_variable) {
    QString test_name;
    double f_value;
    int df = 0;         // degree of freedom
    double p_value = 0;
    int np = 0;         // number of partitions
    int n = 0;
    int total_rows = 0;
    clearGlobalVariables();

    if (m_columns.size() != 2) {
        printError("Inappropriate number of columns selected");
        emit q->changed();
        return;
    }

    if (!categorical_variable && (m_columns[0]->columnMode() == AbstractColumn::Integer || m_columns[0]->columnMode() == AbstractColumn::Numeric))
        np = m_columns.size();
    else
        countPartitions(m_columns[0], np, n);

    if (np < 2) {
        printError("select atleast two columns/ classes");
        emit q->changed();
        return;
    }

    double* yi_bar = new double[np];
    double* zi_bar = new double[np];
    double zi_bar_bar = 0;
    double* ni = new double[np];

    for (int i = 0; i < np; i++) {
        yi_bar[i] = 0;
        zi_bar[i] = 0;
        ni[i] = 0;
    }

    QString* col_names = new QString[np];
    if (!categorical_variable && (m_columns[0]->columnMode() == AbstractColumn::Integer || m_columns[0]->columnMode() == AbstractColumn::Numeric)) {
        total_rows = m_columns[0]->rowCount();

        double value = 0;
        for (int j = 0; j < total_rows; j++) {
            int number_nan_cols = 0;
            for (int i = 0; i < np; i++) {
                value = m_columns[i]->valueAt(j);
                if (std::isnan(value)) {
                    number_nan_cols++;
                    continue;
                }
                yi_bar[i] += value;
                ni[i]++;
                n++;
            }
            if (number_nan_cols == np) {
                total_rows = j;
                break;
            }
        }

        for (int i = 0; i < np; i++) {
            if (ni[i] > 0)
                yi_bar[i] = yi_bar[i] / ni[i];
        }

        for (int j = 0; j < total_rows; j++) {
            for (int i = 0; i < np; i++) {
                value = m_columns[i]->valueAt(j);
                if (!(std::isnan(value)))
                    zi_bar[i] += abs(value - yi_bar[i]);
            }
        }

        for (int i = 0; i < np; i++) {
            zi_bar_bar += zi_bar[i];
            if (ni[i] > 0)
                zi_bar[i] = zi_bar[i] / ni[i];
        }

        zi_bar_bar = zi_bar_bar / n;

        double numerator_value = 0;
        double denominator_value = 0;

        for (int j = 0; j < total_rows; j++) {
            for (int i = 0; i < np; i++) {
                value = m_columns[i]->valueAt(j);
                if (!(std::isnan(value))) {
                    double zij = abs(value - yi_bar[i]);
                    denominator_value +=  gsl_pow_2( (zij - zi_bar[i]));
                }
            }
        }

        for (int i = 0; i < np; i++) {
            col_names[i] = m_columns[i]->name();
            numerator_value += ni[i]*gsl_pow_2( (zi_bar[i]-zi_bar_bar));
        }

        f_value = ((n - np) / (np - 1)) * (numerator_value / denominator_value);

//        qDebug() << "n is " << n;
//        qDebug() << "fvalue is " << f_value;
//        qDebug() << "numerator is " << numerator_value;
//        qDebug() << "denominator is " << denominator_value;

    }
    else {
        QMap<QString, int> classname_to_index;

        AbstractColumn::ColumnMode original_col_mode = m_columns[0]->columnMode();
        m_columns[0]->setColumnMode(AbstractColumn::Text);

        int partition_number = 1;
        QString name;
        double value;
        int class_index;

        for (int j = 0; j < n; j++) {
            name = m_columns[0]->textAt(j);
            value = m_columns[1]->valueAt(j);
            if (std::isnan(value)) {
                n = j;
                break;
            }

            if (classname_to_index[name] == 0) {
                classname_to_index[name] = partition_number;
                partition_number++;
            }

            class_index = classname_to_index[name]-1;
            ni[class_index]++;
            yi_bar[class_index] += value;
        }

        for (int i = 0; i < np; i++) {
            if (ni[i] > 0)
                yi_bar[i] = yi_bar[i] / ni[i];
        }

        for (int j = 0; j < n; j++) {
            name = m_columns[0]->textAt(j);
            value = m_columns[1]->valueAt(j);
            class_index = classname_to_index[name] - 1;
            zi_bar[class_index] += abs(value - yi_bar[class_index]);
        }

        for (int i = 0; i < np; i++) {
            zi_bar_bar += zi_bar[i];
            zi_bar[i] = zi_bar[i] / ni[i];
        }

        zi_bar_bar = zi_bar_bar / n;

        double numerator_value = 0;
        double denominator_value = 0;

        for (int j = 0; j < n; j++) {
            name = m_columns[0]->textAt(j);
            value = m_columns[1]->valueAt(j);
            class_index = classname_to_index[name] - 1;
            double zij = abs(value - yi_bar[class_index]);
            denominator_value +=  gsl_pow_2( (zij - zi_bar[class_index]));
        }

        for (int i = 0; i < np; i++)
            numerator_value += ni[i]*gsl_pow_2( (zi_bar[i]-zi_bar_bar));

        f_value = ((n - np) / (np - 1)) * (numerator_value / denominator_value);

//        qDebug() << "n is " << n;
//        qDebug() << "fvalue is " << f_value;
//        qDebug() << "numerator is " << numerator_value;
//        qDebug() << "denominator is " << denominator_value;

        QMapIterator<QString, int> i(classname_to_index);
        while (i.hasNext()) {
            i.next();
            col_names[i.value()-1] = i.key();
        }
        m_columns[0]->setColumnMode(original_col_mode);
    }

    df = n - np;

    int row_count = np+1;
    int column_count = 4;


    QVariant* row_major = new QVariant[row_count*column_count];
    // header data;
    row_major[0] = ""; row_major[1] = "Ni"; row_major[2] = "Yi_bar"; row_major[3] = "Zi_bar";

    // table data
    for (int row_i = 1; row_i < row_count; row_i++) {
        row_major[row_i*column_count] = col_names[row_i-1];
        row_major[row_i*column_count + 1] = ni[row_i-1];
        row_major[row_i*column_count + 2] = yi_bar[row_i-1];
        row_major[row_i*column_count + 3] = zi_bar[row_i-1];

    }
    m_stats_table = getHtmlTable(row_count, column_count, row_major);

    p_value = nsl_stats_fdist_p(f_value, static_cast<size_t>(np-1), df);

    printLine(0, "Null Hypothesis: Variance is equal between all classes", "blue");
    printLine(1, "Alternate Hypothesis: Variance is not equal in at-least one pair of classes", "blue");
    printLine(2, i18n("Significance level is %1", m_significance_level), "blue");
    printLine(4, i18n("F Value is %1 ", f_value), "green");
    printLine(5, i18n("P Value is %1 ", p_value), "green");
    printLine(6, i18n("Degree of Freedom is %1", df), "green");

    if (p_value <= m_significance_level) {
        q->m_view->setResultLine(5, i18n("We can safely reject Null Hypothesis for significance level %1", m_significance_level), Qt::ToolTipRole);
        printLine(8, "Requirement for homogeneity is not met", "red");
    } else {
        q->m_view->setResultLine(5, i18n("There is a plausibility for Null Hypothesis to be true"), Qt::ToolTipRole);
        printLine(8, "Requirement for homogeneity is met", "green");
    }
    emit q->changed();
    return;
}
/***************************************Helper Functions*************************************/

HypothesisTestPrivate::ErrorType HypothesisTestPrivate::findStats(const Column* column, int &count, double &sum, double &mean, double &std) {
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

    if (count < 1) return HypothesisTestPrivate::ErrorEmptyColumn;
    mean = sum/count;

    for (int i = 0; i < count; i++) {
        double row = column->valueAt(i);
        std += gsl_pow_2( (row - mean));
    }

    if (count > 1)
        std = std / (count-1);
    std = qSqrt(std);

    return HypothesisTestPrivate::NoError;
}

HypothesisTestPrivate::ErrorType HypothesisTestPrivate::findStatsPaired(const Column* column1, const Column* column2, int &count, double &sum, double &mean, double &std) {
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

    mean = sum/count;

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

void HypothesisTestPrivate::countPartitions(Column *column, int &np, int &total_rows) {
    total_rows = column->rowCount();
    np = 0;
    QString cell_value;
    QMap<QString, bool> discovered_categorical_var;

    AbstractColumn::ColumnMode original_col_mode = column->columnMode();
    column->setColumnMode(AbstractColumn::Text);

    for (int i = 0; i < total_rows; i++) {
        cell_value = column->textAt(i);

        if (cell_value.isEmpty()) {
            total_rows = i;
            break;
        }

        if (discovered_categorical_var[cell_value])
            continue;

        discovered_categorical_var[cell_value] = true;
        np++;
    }
    column->setColumnMode(original_col_mode);
}

HypothesisTestPrivate::ErrorType HypothesisTestPrivate::findStatsCategorical(Column *column1, Column *column2, int n[], double sum[], double mean[], double std[], QMap<QString, int> &col_name, const int &np, const int &total_rows) {
    Column* columns[] = {column1, column2};
    for (int i = 0; i < np; i++) {
        n[i] = 0;
        sum[i] = 0;
        mean[i] = 0;
        std[i] = 0;
    }
    AbstractColumn::ColumnMode original_col_mode = columns[0]->columnMode();
    columns[0]->setColumnMode(AbstractColumn::Text);

    int partition_number = 1;
    for (int i = 0; i < total_rows; i++) {
        QString name = columns[0]->textAt(i);

        name = columns[0]->textAt(i);
        double value = columns[1]->valueAt(i);

        if (std::isnan(value)) {
            columns[0]->setColumnMode(original_col_mode);
            return HypothesisTestPrivate::ErrorUnqualSize;
        }

        if (col_name[name] == 0) {
            col_name[name] = partition_number;
            partition_number++;
        }

        n[col_name[name]-1]++;
        sum[col_name[name]-1] += value;
    }

    for (int i = 0; i < np; i++)
        mean[i] = sum[i] / n[i];

    for (int i = 0; i < total_rows; i++) {
        QString name = columns[0]->textAt(i);
        double value = columns[1]->valueAt(i);

        std[col_name[name]-1] += gsl_pow_2( (value - mean[col_name[name]-1]));
    }

    for (int i = 0; i < np; i++) {
        if (n[i] > 1)
            std[i] = std[i] / (n[i] - 1);
        std[i] = qSqrt(std[i]);
    }

    columns[0]->setColumnMode(original_col_mode);
    return HypothesisTestPrivate::NoError;
}


double HypothesisTestPrivate::getPValue(const HypothesisTestPrivate::TestType &test, double &value, const QString &col1_name, const QString &col2_name, const double mean, const double sp, const int df) {
    double p_value = 0;

    //TODO change ("⋖") symbol to ("<"), currently macro UTF8_QSTRING is not working properly if used "<" symbol;
    switch (test) {
    case TestT: {
        switch (tail_type) {
        case HypothesisTest::TailNegative:
            p_value = gsl_cdf_tdist_P(value, df);
            printLine(0, i18n("Null Hypothesis: Population mean of %1 %2 Population mean of %3", col1_name, UTF8_QSTRING("≥"), col2_name), "blue");
            printLine(1, i18n("Alternate Hypothesis: Population mean of %1 %2 Population mean of %3", col1_name, UTF8_QSTRING("⋖"), col2_name), "blue");
            break;
        case HypothesisTest::TailPositive:
            value *= -1;
            p_value = gsl_cdf_tdist_P(value, df);
            printLine(0, i18n("Null Hypothesis: Population mean of %1 %2 Population mean of %3", col1_name, UTF8_QSTRING("≤"), col2_name), "blue");
            printLine(1, i18n("Alternate Hypothesis: Population mean of %1 %2 Population mean of %3", col1_name, UTF8_QSTRING(">"), col2_name), "blue");
            break;
        case HypothesisTest::TailTwo:
            p_value = 2.*gsl_cdf_tdist_P(value, df);

            printLine(0, i18n("Null Hypothesis: Population mean of %1 %2 Population mean of %3", col1_name, UTF8_QSTRING("="), col2_name), "blue");
            printLine(1, i18n("Alternate Hypothesis: Population mean of %1 %2 Population mean of %3", col1_name, UTF8_QSTRING("≠"), col2_name), "blue");
            break;
        }
        break;
    } case TestZ: {
        switch (tail_type) {
        case HypothesisTest::TailNegative:
            p_value = gsl_cdf_gaussian_P(value - mean, sp);
            printLine(0, i18n("Null Hypothesis: Population mean of %1 %2 Population mean of %3 ", col1_name, UTF8_QSTRING("≥"), col2_name), "blue");
            printLine(1, i18n("Alternate Hypothesis: Population mean of %1 %2 Population mean of %3 ", col1_name, UTF8_QSTRING("⋖"), col2_name), "blue");
            break;
        case HypothesisTest::TailPositive:
            value *= -1;
            p_value = nsl_stats_tdist_p(value - mean, sp);
            printLine(0, i18n("Null Hypothesis: Population mean of %1 %2 Population mean of %3 ", col1_name, UTF8_QSTRING("≤"), col2_name), "blue");
            printLine(1, i18n("Alternate Hypothesis: Population mean of %1 %2 Population mean of %3 ", col1_name, UTF8_QSTRING(">"), col2_name), "blue");
            break;
        case HypothesisTest::TailTwo:
            p_value = 2.*gsl_cdf_gaussian_P(value - mean, sp);
            printLine(0, i18n("Null Hypothesis: Population mean of %1 %2 Population mean of %3 ", col1_name, UTF8_QSTRING("="), col2_name), "blue");
            printLine(1, i18n("Alternate Hypothesis: Population mean of %1 %2 Population mean of %3 ", col1_name, UTF8_QSTRING("≠"), col2_name), "blue");
            break;
        }
        break;
    }
    }

    if (p_value > 1)
        return 1;
    return p_value;
}

QString HypothesisTestPrivate::getHtmlTable(int row, int column, QVariant *row_major) {
    if (row < 1 || column < 1)
        return QString();

    QString table = "";
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
        element = row_major[j].toString();
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

        QString element = row_major[i*column].toString();
        table += i18n("    <td class=%1><b>%2</b></td>", bg, element);
        for (int j = 1; j < column; j++) {
            QString element = row_major[i*column+j].toString();
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

QString HypothesisTestPrivate::getLine(const QString &msg, const QString &color) {
    return i18n("<p style=color:%1;>%2</p>", color, msg);
}


void HypothesisTestPrivate::printLine(const int &index, const QString &msg, const QString &color) {
    q->m_view->setResultLine(index, getLine(msg, color));
    return;
}

void HypothesisTestPrivate::printError(const QString &error_msg) {
    printLine(0, error_msg, "red");
    emit q->changed();
}


void HypothesisTestPrivate::clearGlobalVariables() {
    m_stats_table = "";
    q->m_view->clearResult();
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
