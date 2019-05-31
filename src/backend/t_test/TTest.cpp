/***************************************************************************
    File                 : TTest.cpp
    Project              : LabPlot
    Description          : Doing T-Test on data provided
    --------------------------------------------------------------------
    Copyright            : (C) 2019 Alexander Semke(alexander.semke@web.de)

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

#include "TTest.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/core/column/Column.h"

extern "C" {
#include "backend/nsl/nsl_stats.h"
}

#include <QVector>
#include <QMessageBox>
#include <QtMath>
#include <KLocalizedString>


TTest::TTest(const QString &name) {
    Q_UNUSED(name);
}

void TTest::setDataSourceSpreadsheet(Spreadsheet *spreadsheet) {
    dataSourceSpreadsheet = spreadsheet;

    m_rowCount = dataSourceSpreadsheet->rowCount();
    m_columnCount = dataSourceSpreadsheet->columnCount();
    DEBUG("in ttest::setDataSourceSpreadsheet")

    // now finding the number of columns and rows;
    DEBUG("row count is " << m_rowCount)
    DEBUG("row count is " << m_columnCount)
    DEBUG("exiting ttest::setDataSourceSpreadsheet")
}

void TTest::setColumns(QVector<Column*> cols) {
    m_columns = cols;
    return;
}

void TTest::performTwoSampleTest() {
    QMessageBox* msg_box = new QMessageBox();
    // checking for cols;
    if (m_columns.size() != 2) {
        msg_box->setText(i18n("Inappropriate number of columns selected"));
        msg_box->exec();
        return;
    }

    bool modeOk = true;
    for (int i = 0; i < 2; i++) {
        if(m_columns[0]->columnMode() == AbstractColumn::Numeric || m_columns[i]->columnMode() == AbstractColumn::Integer)
            continue;
        modeOk = false;
    }

    if (!modeOk) {
        msg_box->setText(i18n("select only columns with numbers"));
        msg_box->exec();
        return;
    }

    // use of three than two for human readiblity of code;
    int n[2];
    double sum[2], mean[2], std[2];

    for (int i = 0; i < 2; i++) {
        findStats(m_columns[i], n[i], sum[i], mean[i], std[i]);
        DEBUG("for " << i);
        DEBUG("n is "<<n[i]);
        DEBUG("mean is " << mean[i]);
        DEBUG("std is " << std[i]);

        if (n[i] < 1) {
            msg_box->setText(i18n("atleast one of selected column empty"));
            msg_box->exec();
            return;
        }
    }
    int df = n[0] + n[1] - 2;

    //Assuming equal variance
    double sp = qSqrt( ((n[0]-1)*qPow(std[0],2) + (n[1]-1)*qPow(std[1],2))/df);

    QDEBUG("sp is " << sp);

    double t = (mean[0] - mean[1])/(sp*qSqrt(1.0/n[0] + 1.0/n[1]));

    // now finding p value from t value
    double p_value = nsl_stats_tdist_p(t, df);

    QString text = i18n("T value for test is %1 and\n p value is %2",t, p_value);
    msg_box->setText(text);
    msg_box->exec();
    return;

//    double t_value =

}

void TTest::findStats(Column* column, int &count, double &sum, double &mean, double &std) {
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

    if (count < 1) return;
    mean = sum/count;

    for (int i = 0; i < count; i++) {
        double row = column->valueAt(i);
        std += qPow((row - mean),2);
    }

    if (count > 1)
        std = std / (count-1);
    std = qSqrt(std);
    return;
}
