/***************************************************************************
    File                 : HypothesisTestPrivate.h
    Project              : LabPlot
    Description          : Private members of Hypothesis Test
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

#ifndef HYPOTHESISTESTPRIVATE_H
#define HYPOTHESISTESTPRIVATE_H

#include <backend/hypothesis_test/HypothesisTest.h>

class QStandardItemModel;

class HypothesisTestPrivate {
public:

        explicit HypothesisTestPrivate(HypothesisTest*);
        virtual ~HypothesisTestPrivate();

        enum TestType {TestT, TestZ};
        QString name() const;

        HypothesisTest* const q;

        HypothesisTest::DataSourceType dataSourceType{HypothesisTest::DataSourceSpreadsheet};
        Spreadsheet* dataSourceSpreadsheet{nullptr};
        
        void setDataSourceSpreadsheet(Spreadsheet* spreadsheet);
        void setColumns(QStringList cols);
        QVector<Column*> m_columns;
        
        QStringList all_columns;

        QStandardItemModel* dataModel{nullptr};
        QStandardItemModel* horizontalHeaderModel{nullptr};
        QStandardItemModel* verticalHeaderModel{nullptr};

        bool m_dbCreated{false};
        int m_rowCount{0};
        int m_columnCount{0};
        QString m_currTestName{"Result Table"};
        double m_population_mean;
        double m_significance_level;


        void performTwoSampleIndependentTest(TestType test, bool equal_variance = true);
        void performTwoSamplePairedTest(TestType test);
        void PerformOneSampleTest(TestType test);

        HypothesisTest::TailType tail_type;

        QStandardItemModel* resultModel{nullptr};
private:
        void findStats(Column* column, int &count, double &sum, double &mean, double &std);
        void findStatsPaired(Column *column1, Column *column2, int &count, double &sum, double &mean, double &std);

        void findStatsCategorical(int n[2], double sum[2], double mean[2], double std[2], QString &col1_name, QString &col2_name);
// 	QMap<QString, QStringList> m_members;
};

#endif
