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

extern "C" {
#include "backend/nsl/nsl_stats.h"
}

CorrelationCoefficient::CorrelationCoefficient(const QString &name) : GeneralTest (name, AspectType::CorrelationCoefficient) {
}

CorrelationCoefficient::~CorrelationCoefficient() {
}

void CorrelationCoefficient::performTest(Test test, bool categoricalVariable) {
    m_statsTable = "";
    m_tooltips.clear();
    for (int i = 0; i < 10; i++)
        m_resultLine[i]->clear();

    switch (test) {
    case CorrelationCoefficient::Test::Pearson: {
        m_currTestName = "<h2>" + i18n("Pearson's r Correlation Test") + "</h2>";
        performPearson(categoricalVariable);
        break;
    }
    case CorrelationCoefficient::Test::Kendall:
        m_currTestName = "<h2>" + i18n("Kendall's Correlation Test") + "</h2>";
        performKendall();
        break;
    case CorrelationCoefficient::Test::Spearman: {
        m_currTestName = "<h2>" + i18n("Spearman Correlation Test") + "</h2>";
        performSpearman();
        break;
    }
    }

    emit changed();
}


double CorrelationCoefficient::correlationValue() {
    return m_correlationValue;
}


/***************************************************************************************************************************
 *                                        Private Implementations
 * ************************************************************************************************************************/

/*********************************************Pearson r ******************************************************************/
// variables:
//  N           = total number of observations
//  sumColx     = sum of values in colx
//  sumSqColx   = sum of square of values in colx
//  sumColxColy = sum of product of values in colx and coly

//TODO: support for col1 is categorical.
//TODO: add symbols in stats table header.
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


    if (!isNumericOrInteger(m_columns[1])) {
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
    rowMajor.append(new Cell("", level, true));
    rowMajor.append(new Cell("N", level, true, "Total Number of Observations"));
    rowMajor.append(new Cell("Sigma", level, true, "Sum of Scores in each column"));
    rowMajor.append(new Cell("Sigma x2", level, true, "Sum of Squares of scores in each column"));
    rowMajor.append(new Cell("Sigma xy", level, true, "Sum of Squares of scores in each column"));

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

    printLine(0, QString("Correlation Value is %1").arg(m_correlationValue), "green");

}

/***********************************************Kendall ******************************************************************/
void CorrelationCoefficient::performKendall() {

}

/***********************************************Spearman ******************************************************************/
void CorrelationCoefficient::performSpearman() {

}

// Virtual functions
QWidget* CorrelationCoefficient::view() const {
    if (!m_partView) {
        m_view = new CorrelationCoefficientView(const_cast<CorrelationCoefficient*>(this));
        m_partView = m_view;
    }
    return m_partView;
}
