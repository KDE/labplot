/***************************************************************************
    File                 : CorrelationCoefficient.cpp
    Project              : LabPlot
    Description          : Finding Correlation Coefficient on data provided
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

#include "CorrelationCoefficient.h"
#include "GeneralTest.h"
#include "kdefrontend/generalTest/CorrelationCoefficientView.h"
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

void CorrelationCoefficient::performTest(Test test, bool categoricalVariable) {
    m_statsTable = "";
    m_tooltips.clear();
    m_correlationValue = 0;
    m_statisticValue.clear();
    m_pValue.clear();
    for (int i = 0; i < RESULTLINESCOUNT; i++)
        m_resultLine[i]->clear();

    switch (test) {
    case CorrelationCoefficient::Test::Pearson: {
        m_currTestName = "<h2>" + i18n("Pearson's r Correlation Test") + "</h2>";
        performPearson(categoricalVariable);
        break;
    }
    case CorrelationCoefficient::Test::Kendall:
        m_currTestName = "<h2>" + i18n("Kendall's Rank Correlation Test") + "</h2>";
        performKendall();
        break;
    case CorrelationCoefficient::Test::Spearman: {
        m_currTestName = "<h2>" + i18n("Spearman Correlation Coefficient Test") + "</h2>";
        performSpearman();
        break;
    }
    }

    emit changed();
}


double CorrelationCoefficient::correlationValue() const{
    return m_correlationValue;
}

QList<double> CorrelationCoefficient::statisticValue() const{
    return m_statisticValue;
}

QList<double> CorrelationCoefficient::pValue() const{
    return m_pValue;
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
        printError("Select only 2 columns ");
        return;
    }

    if (categoricalVariable) {
        printLine(1, "currently categorical variable not supported", "blue");
        return;
    }

    QString col1Name = m_columns[0]->name();
    QString col2Name = m_columns[1]->name();


    if (!m_columns[1]->isNumeric()) {
        printError("Column " + col2Name + " should contain only numeric or interger values");
    }


    int N = findCount(m_columns[0]);
    if (N != findCount(m_columns[1])) {
        printError("Number of data values in Column: " + col1Name + "and Column: " + col2Name + "are not equal");
        return;
    }

    double sumCol1 = findSum(m_columns[0], N);
    double sumCol2 = findSum(m_columns[1], N);
    double sumSqCol1 = findSumSq(m_columns[0], N);
    double sumSqCol2 = findSumSq(m_columns[1], N);

    double sumCol12 = 0;

    for (int i = 0; i < N; i++)
        sumCol12 += m_columns[0]->valueAt(i) *
                    m_columns[1]->valueAt(i);

    // printing table;
    // cell constructor structure; data, level, rowSpanCount, m_columnspanCount, isHeader;
    QList<Cell*> rowMajor;
    int level = 0;

    // horizontal header
    QString sigma = UTF8_QSTRING("Σ");
    rowMajor.append(new Cell("", level, true));

    rowMajor.append(new Cell("N", level, true, "Total Number of Observations"));
    rowMajor.append(new Cell(QString(sigma + "Scores"), level, true, "Sum of Scores in each column"));
    rowMajor.append(new Cell(QString(sigma + "Scores<sup>2</sup>"), level, true, "Sum of Squares of scores in each column"));
    rowMajor.append(new Cell(QString(sigma + "(" + UTF8_QSTRING("∏") + "Scores)"), level, true, "Sum of product of scores of both columns"));

    //data with vertical header.
    level++;
    rowMajor.append(new Cell(col1Name, level, true));
    rowMajor.append(new Cell(N, level));
    rowMajor.append(new Cell(sumCol1, level));
    rowMajor.append(new Cell(sumSqCol1, level));

    rowMajor.append(new Cell(sumCol12, level, false, "", 2, 1));

    level++;
    rowMajor.append(new Cell(col2Name, level, true));
    rowMajor.append(new Cell(N, level));
    rowMajor.append(new Cell(sumCol2, level));
    rowMajor.append(new Cell(sumSqCol2, level));

    m_statsTable += getHtmlTable3(rowMajor);


    m_correlationValue = (N * sumCol12 - sumCol1*sumCol2) /
                        sqrt((N * sumSqCol1 - gsl_pow_2(sumCol1)) *
                             (N * sumSqCol2 - gsl_pow_2(sumCol2)));

    printLine(0, QString("Correlation Value is %1").arg(round(m_correlationValue)), "green");

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
        printError("Select only 2 columns ");
        return;
    }

    QString col1Name = m_columns[0]->name();
    QString col2Name = m_columns[1]->name();

    int N = findCount(m_columns[0]);
    if (N != findCount(m_columns[1])) {
        printError("Number of data values in Column: " + col1Name + "and Column: " + col2Name + "are not equal");
        return;
    }

    QVector<int> col2Ranks(N);
    if (m_columns[0]->isNumeric()) {
        if (m_columns[1]->isNumeric()) {
            for (int i = 0; i < N; i++)
                col2Ranks[int(m_columns[0]->valueAt(i)) - 1] = int(m_columns[1]->valueAt(i));
        } else {
            printError(QString("Ranking System should be same for both Column: %1 and Column: %2 <br/>"
                               "Hint: Check for data types of columns").arg(col1Name).arg(col2Name));
            return;
        }
    } else {
        AbstractColumn::ColumnMode origCol1Mode = m_columns[0]->columnMode();
        AbstractColumn::ColumnMode origCol2Mode = m_columns[1]->columnMode();

        m_columns[0]->setColumnMode(AbstractColumn::Text);
        m_columns[1]->setColumnMode(AbstractColumn::Text);

        QMap<QString, int> ValueToRank;

        for (int i = 0; i < N; i++) {
            if (ValueToRank[m_columns[0]->textAt(i)] != 0) {
                printError("Currently ties are not supported");
                m_columns[0]->setColumnMode(origCol1Mode);
                m_columns[1]->setColumnMode(origCol2Mode);
                return;
            }
            ValueToRank[m_columns[0]->textAt(i)] = i + 1;
        }

        for (int i = 0; i < N; i++)
            col2Ranks[i] = ValueToRank[m_columns[1]->textAt(i)];

        m_columns[0]->setColumnMode(origCol1Mode);
        m_columns[1]->setColumnMode(origCol2Mode);
    }

    int nPossiblePairs = (N * (N - 1)) / 2;

    int nDiscordant = findDiscordants(col2Ranks.data(), 0, N - 1);
    int nCorcordant = nPossiblePairs - nDiscordant;

    m_correlationValue = double(nCorcordant - nDiscordant) / nPossiblePairs;

    m_statisticValue.append((3 * (nCorcordant - nDiscordant)) /
                sqrt(N * (N- 1) * (2 * N + 5) / 2));

    printLine(0 , QString("Number of Discordants are %1").arg(nDiscordant), "green");
    printLine(1 , QString("Number of Concordant are %1").arg(nCorcordant), "green");

    printLine(2 , QString("Tau a is %1").arg(round(m_correlationValue)), "green");
    printLine(3 , QString("Z Value is %1").arg(round(m_statisticValue[0])), "green");

    return;

}

/***********************************************Spearman ******************************************************************/
// All formulaes and symbols are taken from : https://www.statisticshowto.datasciencecentral.com/spearman-rank-correlation-definition-calculate/

void CorrelationCoefficient::performSpearman() {
    if (m_columns.count() != 2) {
        printError("Select only 2 columns ");
        return;
    }

    QString col1Name = m_columns[0]->name();
    QString col2Name = m_columns[1]->name();

    int N = findCount(m_columns[0]);
    if (N != findCount(m_columns[1])) {
        printError("Number of data values in Column: " + col1Name + "and Column: " + col2Name + "are not equal");
        return;
    }

    QMap<double, int> col1Ranks;
    convertToRanks(m_columns[0], N, col1Ranks);

    QMap<double, int> col2Ranks;
    convertToRanks(m_columns[1], N, col2Ranks);

    double ranksCol1Mean = 0;
    double ranksCol2Mean = 0;

    for (int i = 0; i < N; i++) {
        ranksCol1Mean += col1Ranks[int(m_columns[0]->valueAt(i))];
        ranksCol2Mean += col2Ranks[int(m_columns[1]->valueAt(i))];
    }

    ranksCol1Mean = ranksCol1Mean / N;
    ranksCol2Mean = ranksCol2Mean / N;

    double s12 = 0;
    double s1 = 0;
    double s2 = 0;

    for (int i = 0; i < N; i++) {
        double centeredRank_1 = col1Ranks[int(m_columns[0]->valueAt(i))] - ranksCol1Mean;
        double centeredRank_2 = col2Ranks[int(m_columns[1]->valueAt(i))] - ranksCol2Mean;

        s12 += centeredRank_1 * centeredRank_2;

        s1 += gsl_pow_2(centeredRank_1);
        s2 += gsl_pow_2(centeredRank_2);
    }

    s12 = s12 / N;
    s1 = s1 / N;
    s2 = s2 / N;

    m_correlationValue = s12 / sqrt(s1 * s2);

    printLine(0, QString("Spearman Rank Correlation value is %1").arg(m_correlationValue), "green");
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
    for (int i = 0; i < N; i++) {
        ranks[sortedList[i]] = i + 1;
    }

    delete[] sortedList;
}

void CorrelationCoefficient::convertToRanks(const Column* col, QMap<double, int> &ranks) {
    convertToRanks(col, findCount(col), ranks);
}

/***********************************************Virtual Functions******************************************************************/

QWidget* CorrelationCoefficient::view() const {
    if (!m_partView) {
        m_view = new CorrelationCoefficientView(const_cast<CorrelationCoefficient*>(this));
        m_partView = m_view;
    }
    return m_partView;
}
