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
    d->m_currTestName = "<h2>Two Sample Independent T Test</h2>";
    d->performTwoSampleIndependentTest(HypothesisTestPrivate::TestT, categorical_variable, equal_variance);
}

void HypothesisTest::performTwoSamplePairedTTest() {
    d->m_currTestName = "<h2>Two Sample Paried T Test</h2>";
    d->performTwoSamplePairedTest(HypothesisTestPrivate::TestT);
}

void HypothesisTest::PerformOneSampleTTest() {
    d->m_currTestName = "<h2>One Sample T Test</h2>";
    d->PerformOneSampleTest(HypothesisTestPrivate::TestT);
}

void HypothesisTest::performTwoSampleIndependentZTest() {
    d->m_currTestName = "<h2>Two Sample Independent Z Test</h2>";
    d->performTwoSampleIndependentTest(HypothesisTestPrivate::TestZ);
}

void HypothesisTest::performTwoSamplePairedZTest() {
    d->m_currTestName = "<h2>Two Sample Paired Z Test</h2>";
    d->performTwoSamplePairedTest(HypothesisTestPrivate::TestZ);
}

void HypothesisTest::PerformOneSampleZTest() {
    d->m_currTestName = "<h2>One Sample Z Test</h2>";
    d->PerformOneSampleTest(HypothesisTestPrivate::TestZ);
}




/******************************************************************************
 *                      Private Implementations
 * ****************************************************************************/

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
        ErrorType error_code = findStatsCategorical(m_columns[0], m_columns[1], n, sum, mean, std, col1_name, col2_name);
        switch (error_code) {
            case ErrorUnqualSize: {
                printError( i18n("Unequal size between Column %1 and Column %2", m_columns[0]->name(), m_columns[1]->name()));
                emit q->changed();
                return;
            } case ErrorNotTwoCategoricalVariables: {
                printError( i18n("Number of Categorical Variable in Column %1 is not equal to 2", m_columns[0]->name()));
                emit q->changed();
                return;
            } case ErrorEmptyColumn: {
                printError("At least one of selected column is empty");
                emit q->changed();
                return;
            } case NoError:
                break;
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
            double sp = qSqrt( ((n[0]-1)*qPow(std[0],2) + (n[1]-1)*qPow(std[1],2))/df);
            value = (mean[0] - mean[1])/(sp*qSqrt(1.0/n[0] + 1.0/n[1]));
            printLine(9, "<b>Assumption:</b> Equal Variance b/w both population means");
        } else {
            double temp_val;
            temp_val = qPow( qPow(std[0], 2)/n[0] + qPow(std[1], 2)/n[1], 2);
            temp_val = temp_val / ( (qPow( (qPow(std[0], 2)/n[0]), 2)/(n[0]-1)) + (qPow( (qPow(std[1], 2)/n[1]), 2)/(n[1]-1)));
            df = qRound(temp_val);

            value = (mean[0] - mean[1]) / (qSqrt( (qPow(std[0], 2)/n[0]) + (qPow(std[1], 2)/n[1])));
            printLine(9, "<b>Assumption:</b> UnEqual Variance b/w both population means");
        }
        break;
    } case TestZ: {
        test_name = "Z";
        df = n[0] + n[1] - 2;

        double sp = qSqrt( ((n[0]-1)*qPow(std[0],2) + (n[1]-1)*qPow(std[1],2))/df);
        value = (mean[0] - mean[1])/(sp*qSqrt(1.0/n[0] + 1.0/n[1]));
    }        
    }

    m_currTestName = i18n("<h2>Two Sample Independent %1 Test for %2 vs %3</h2>", test_name, col1_name, col2_name);
    p_value = getPValue(test, value, col1_name, col2_name, df);

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
        default:
            emit q->changed();
            return;
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

    p_value = getPValue(test, value, m_columns[0]->name(), i18n("%1",m_population_mean), df);
    m_currTestName = i18n("<h2>One Sample %1 Test for %2 vs %3</h2>", test_name, m_columns[0]->name(), m_columns[1]->name());

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

void HypothesisTestPrivate::PerformOneSampleTest(TestType test) {
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
        default: {
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

    p_value = getPValue(test, value, m_columns[0]->name(), i18n("%1",m_population_mean), df);
    m_currTestName = i18n("<h2>One Sample %1 Test for %2</h2>", test_name, m_columns[0]->name());

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
        std += qPow( (row - mean), 2);
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
        std += qPow( (row - mean), 2);
    }

    if (count > 1)
        std = std / (count-1);

    std = qSqrt(std);
    return HypothesisTestPrivate::NoError;
}

HypothesisTestPrivate::ErrorType HypothesisTestPrivate::findStatsCategorical(const Column *column1, const Column *column2, int n[], double sum[], double mean[], double std[], QString &col1_name, QString &col2_name) {
    // clearing and initialising variables;

    const Column* columns[] = {column1, column2};

    for (int i = 0; i < 2; i++) {
        sum[i] = 0;
        mean[i] = 0;
        std[i] = 0;
        n[i] = 0;
    }

    int count_temp = columns[0]->rowCount();
    col1_name = "";
    col2_name = "";
    for (int i = 0; i < count_temp; i++) {
        QString name = columns[0]->textAt(i);
        double value = columns[1]->valueAt(i);

        if (name.isEmpty() || std::isnan(value)) {
            if (name.isEmpty() && std::isnan(value))
                break;
            else
                return HypothesisTestPrivate::ErrorUnqualSize;
        }

        if (name == col1_name) {
            n[0]++;
            sum[0] += value;
        } else if (name == col2_name) {
            n[1]++;
            sum[1] += value;
        } else if (col1_name.isEmpty()) {
            n[0]++;
            sum[0] += value;
            col1_name = name;
        } else if (col2_name.isEmpty()) {
            n[1]++;
            sum[1] += value;
            col2_name = name;
        }
        else
            return HypothesisTestPrivate::ErrorNotTwoCategoricalVariables;
    }

    if (col1_name.isEmpty() || col2_name.isEmpty())
        return HypothesisTestPrivate::ErrorNotTwoCategoricalVariables;


    mean[0] = sum[0]/n[0];
    mean[1] = sum[1]/n[1];


    for (int i = 0; i < n[0]+n[1]; i++) {
        QString name = columns[0]->textAt(i);
        double value = columns[1]->valueAt(i);

        if (name == col1_name)
            std[0] += qPow( (value - mean[0]), 2);
        else
            std[1] += qPow( (value - mean[1]), 2);
    }

    for (int i = 0; i < 2; i++) {
        if (n[i] > 1)
            std[i] = std[i] / (n[i] - 1);
        std[i] = qSqrt(std[i]);
    }

    return HypothesisTestPrivate::NoError;
}


double HypothesisTestPrivate::getPValue(const HypothesisTestPrivate::TestType &test, double &value, const QString &col1_name, const QString &col2_name, const int df) {
    double p_value = 0;

    //TODO change ("⋖") symbol to ("<"), currently macro UTF8_QSTRING is not working properly if used "<" symbol;
    switch (test) {
    case TestT: {
        switch (tail_type) {
        case HypothesisTest::TailNegative:
            p_value = nsl_stats_tdist_p(value, df);
            printLine(0, i18n("Null Hypothesis: Population mean of %1 %2 Population mean of %3", col1_name, UTF8_QSTRING("≥"), col2_name), "blue");
            printLine(1, i18n("Alternate Hypothesis: Population mean of %1 %2 Population mean of %3", col1_name, UTF8_QSTRING("⋖"), col2_name), "blue");
            break;
        case HypothesisTest::TailPositive:
            value *= -1;
            p_value = nsl_stats_tdist_p(value, df);
            printLine(0, i18n("Null Hypothesis: Population mean of %1 %2 Population mean of %3", col1_name, UTF8_QSTRING("≤"), col2_name), "blue");
            printLine(1, i18n("Alternate Hypothesis: Population mean of %1 %2 Population mean of %3", col1_name, UTF8_QSTRING(">"), col2_name), "blue");
            break;
        case HypothesisTest::TailTwo:
            p_value = nsl_stats_tdist_p(value, df) + nsl_stats_tdist_p(-1*value, df);

            printLine(0, i18n("Null Hypothesis: Population mean of %1 %2 Population mean of %3", col1_name, UTF8_QSTRING("="), col2_name), "blue");
            printLine(1, i18n("Alternate Hypothesis: Population mean of %1 %2 Population mean of %3", col1_name, UTF8_QSTRING("≠"), col2_name), "blue");
            break;
        }
        break;
    } case TestZ: {
        switch (tail_type) {
        case HypothesisTest::TailNegative:
            p_value = nsl_stats_tdist_p(value, df);
            printLine(0, i18n("Null Hypothesis: Population mean of %1 %2 Population mean of %3 ", col1_name, UTF8_QSTRING("≥"), col2_name), "blue");
            printLine(1, i18n("Alternate Hypothesis: Population mean of %1 %2 Population mean of %3 ", col1_name, UTF8_QSTRING("⋖"), col2_name), "blue");
            break;
        case HypothesisTest::TailPositive:
            value *= -1;
            p_value = nsl_stats_tdist_p(value, df);
            printLine(0, i18n("Null Hypothesis: Population mean of %1 %2 Population mean of %3 ", col1_name, UTF8_QSTRING("≤"), col2_name), "blue");
            printLine(1, i18n("Alternate Hypothesis: Population mean of %1 %2 Population mean of %3 ", col1_name, UTF8_QSTRING(">"), col2_name), "blue");
            break;
        case HypothesisTest::TailTwo:
            p_value = nsl_stats_tdist_p(value, df) + nsl_stats_tdist_p(-1*value, df);

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


void HypothesisTestPrivate::printLine(const int &index, const QString &msg, const QString &color) {
    q->m_view->setResultLine(index, i18n("<p style=color:%1;>%2</p>", color, msg));
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
