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

#include <backend/hypothesisTest/HypothesisTest.h>

class QStandardItemModel;

class HypothesisTestPrivate {
public:
        explicit HypothesisTestPrivate(HypothesisTest*);
        virtual ~HypothesisTestPrivate();

		enum ErrorType {ErrorUnqualSize, ErrorEmptyColumn, NoError};

        QString name() const;
        void setDataSourceSpreadsheet(Spreadsheet* spreadsheet);
        void setColumns(QStringList cols);
		void performTwoSampleIndependentTest(HypothesisTest::Test::Type test, bool categorical_variable = false, bool equal_variance = true);
		void performTwoSamplePairedTest(HypothesisTest::Test::Type test);
		void performOneSampleTest(HypothesisTest::Test::Type test);
        void performOneWayAnova();

        void performLeveneTest(bool categorical_variable);

		HypothesisTest* const q;
        HypothesisTest::DataSourceType dataSourceType{HypothesisTest::DataSourceSpreadsheet};
        Spreadsheet* dataSourceSpreadsheet{nullptr};
        QVector<Column*> m_columns;
        QStringList all_columns;

		bool m_dbCreated{false};
        int m_rowCount{0};
        int m_columnCount{0};
        QString m_currTestName{"Result Table"};
        double m_population_mean;
        double m_significance_level;
        QString m_stats_table;
		HypothesisTest::Test::Tail tail_type;

private:
		bool isNumericOrInteger(Column* column);

		void countPartitions(Column* column, int& np, int& total_rows);
		ErrorType findStats(const Column* column,int& count, double& sum, double& mean, double& std);
		ErrorType findStatsPaired(const Column* column1, const Column* column2, int& count, double& sum, double& mean, double& std);
		ErrorType findStatsCategorical(Column* column1, Column* column2, int n[], double sum[], double mean[], double std[], QMap<QString, int>& col_name, const int& np, const int& total_rows);

		double getPValue(const HypothesisTest::Test::Type& test, double& value, const QString& col1_name, const QString& col2_name, const double mean, const double sp, const int df);
		QString getHtmlTable(int row, int column, QVariant* row_major);

		QString getLine(const QString& msg, const QString& color = "black");
		void printLine(const int& index, const QString& msg, const QString& color = "black");
		void printTooltip(const int& index, const QString& msg);
		void printError(const QString& error_msg);
        void clearTestView();
};

#endif
