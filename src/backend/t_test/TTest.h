/***************************************************************************
    File                 : TTest.h
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

#ifndef TTEST_H
#define TTEST_H
#include <QVector>


class Spreadsheet;
class QString;
class Column;

class TTest {
public:
    explicit TTest(const QString& name);
    void setDataSourceSpreadsheet(Spreadsheet* spreadsheet);
    void setColumns(QVector<Column*> cols);
    void performTwoSampleTest();
private:
//    double findMean(Column* col);
//    double findStandardDeviation(Column* col, double mean);
    void findStats(Column* column, int &count, double &sum, double &mean, double &std);

    Spreadsheet* dataSourceSpreadsheet{nullptr};
    int m_rowCount{0};
    int m_columnCount{0};
    QVector<Column*> m_columns;
};

#endif // TTEST_H
