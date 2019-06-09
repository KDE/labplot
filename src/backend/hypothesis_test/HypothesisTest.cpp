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

QAbstractItemModel* HypothesisTest::dataModel() {
    return d->dataModel;
}

QAbstractItemModel* HypothesisTest::horizontalHeaderModel() {
    return d->horizontalHeaderModel;
}

QAbstractItemModel* HypothesisTest::verticalHeaderModel() {
    return d->verticalHeaderModel;
}

QAbstractItemModel* HypothesisTest::resultModel() {
    return d->resultModel;
}


void HypothesisTest::setDataSourceType(DataSourceType type) {
    if (type != d->dataSourceType) {
        d->dataSourceType = type;
    }
}

void HypothesisTest::setDataSourceSpreadsheet(Spreadsheet *spreadsheet) {
    if  (spreadsheet != d->dataSourceSpreadsheet)
        d->setDataSourceSpreadsheet(spreadsheet);
}

QStringList HypothesisTest::allColumns() {
    return d->all_columns;
}

HypothesisTest::TailType HypothesisTest::tailType() {
    return d->tail_type;
}

void HypothesisTest::setTailType(HypothesisTest::TailType tailType) {
    d->tail_type = tailType;
}

void HypothesisTest::setColumns(QVector<Column*> cols) {
    d->m_columns = cols;
}

void HypothesisTest::setColumns(QStringList cols) {
    return d->setColumns(cols);
}

HypothesisTest::DataSourceType HypothesisTest::dataSourceType() const {
    return d->dataSourceType;
}

void HypothesisTest::performTwoSampleTTest() {

}

void HypothesisTest::performTwoSampleIndependetTTest(bool equal_variance) {
    d->performTwoSampleIndependetTest(HypothesisTestPrivate::TestT, equal_variance);
}

void HypothesisTest::performTwoSamplePairedTTest() {
    d->performTwoSamplePairedTest(HypothesisTestPrivate::TestT);
}

void HypothesisTest::PerformOneSampleTTest() {
    d->PerformOneSampleTest(HypothesisTestPrivate::TestT);
}

void HypothesisTest::performTwoSampleIndependetZTest() {
    d->performTwoSampleIndependetTest(HypothesisTestPrivate::TestZ);
}

void HypothesisTest::performTwoSamplePairedZTest() {
    d->performTwoSamplePairedTest(HypothesisTestPrivate::TestZ);
}

void HypothesisTest::PerformOneSampleZTest() {
    d->PerformOneSampleTest(HypothesisTestPrivate::TestZ);
}


QString HypothesisTest::testName() {
    return d->m_currTestName;
}

/******************************************************************************
 *                      Private Implementations
 * ****************************************************************************/

HypothesisTestPrivate::HypothesisTestPrivate(HypothesisTest* owner) : q(owner) ,
    dataModel(new QStandardItemModel) ,
    horizontalHeaderModel(new QStandardItemModel) ,
    verticalHeaderModel(new QStandardItemModel) ,
    resultModel(new QStandardItemModel) {
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
        if (col != "") {
            column = dataSourceSpreadsheet->column(col);
            m_columns.append(column);
        }
    }
}


/**************************Two Sample Independent *************************************/

void HypothesisTestPrivate::performTwoSampleIndependetTest(Test test, bool equal_variance) {
    dataModel->clear();
    horizontalHeaderModel->clear();


    QMessageBox* msg_box = new QMessageBox();
    // checking for cols;
    if (m_columns.size() != 2) {
        msg_box->setText(i18n("Inappropriate number of columns selected"));
        msg_box->exec();
        return;
    }

    bool modeOk = true;
    for (int i = 0; i < 2; i++) {
        if(m_columns[i]->columnMode() == AbstractColumn::Numeric || m_columns[i]->columnMode() == AbstractColumn::Integer)
            continue;
        modeOk = false;
    }

    if (!modeOk) {
        msg_box->setText(i18n("select only columns with numbers"));
        msg_box->exec();
        return;
    }


    int n[2];
    double sum[2], mean[2], std[2];

    for (int i = 0; i < 2; i++) {
        findStats(m_columns[i], n[i], sum[i], mean[i], std[i]);

        if (n[i] < 1) {
            msg_box->setText(i18n("atleast one of selected column is empty"));
            msg_box->exec();
            return;
        }
    }


    dataModel->setRowCount(2);
    dataModel->setColumnCount(3);
    horizontalHeaderModel->setColumnCount(3);
    verticalHeaderModel->setRowCount(2);

    for (int i = 0; i < 2; i++) {
        dataModel->setItem(i, 0, new QStandardItem(QString::number(n[i])));
        dataModel->setItem(i, 1, new QStandardItem(QString::number(mean[i])));
        dataModel->setItem(i, 2, new QStandardItem(QString::number(std[i])));
    }

    horizontalHeaderModel->setHeaderData(0, Qt::Horizontal, "N");
    horizontalHeaderModel->setHeaderData(1, Qt::Horizontal, "Mean");
    horizontalHeaderModel->setHeaderData(2, Qt::Horizontal, "StDev");

    verticalHeaderModel->setHeaderData(0, Qt::Vertical, m_columns[0]->name());
    verticalHeaderModel->setHeaderData(1, Qt::Vertical, m_columns[1]->name());

    if (test == TestT) {
        m_currTestName = i18n("Two Sample Independent T Test for %1 vs %2", m_columns[0]->name(), m_columns[1]->name());
        double t;
        int df;
        QString temp = "";
        double p_value = 0;

        resultModel->setRowCount(8);
        resultModel->setColumnCount(1);


        if (equal_variance) {
            df = n[0] + n[1] - 2;

            //Assuming equal variance
            double sp = qSqrt( ((n[0]-1)*qPow(std[0],2) + (n[1]-1)*qPow(std[1],2))/df);
            t = (mean[0] - mean[1])/(sp*qSqrt(1.0/n[0] + 1.0/n[1]));

            resultModel->setData(resultModel->index(7, 0), i18n("Assumption: Equal Variance between both populations"), Qt::DisplayRole);
        } else {
            double temp;
            temp = qPow( qPow(std[0], 2)/n[0] + qPow(std[1], 2)/n[1], 2);
            temp = temp / ( (qPow( (qPow(std[0], 2)/n[0]), 2)/(n[0]-1)) + (qPow( (qPow(std[1], 2)/n[1]), 2)/(n[1]-1)));
            df = qRound(temp);

            t = (mean[0] - mean[1]) / (qSqrt( (qPow(std[0], 2)/n[0]) + (qPow(std[1], 2)/n[1])));

            resultModel->setData(resultModel->index(7, 0), i18n("Assumption: Non-Equal Variance between both populations"), Qt::DisplayRole);
        }



        switch (tail_type) {
            case HypothesisTest::TailNegative:
                p_value = nsl_stats_tdist_p(t, df);

                temp = i18n("Null Hypothesis : mean of %1 %2 mean of %3", m_columns[0]->name(), QChar(0x2265), m_columns[1]->name());
                resultModel->setData(resultModel->index(0, 0), temp, Qt::DisplayRole);

                temp = i18n("Alternate Hypothesis : mean of %1 %2 mean of %3", m_columns[0]->name(), QChar(0x3C), m_columns[1]->name());
                resultModel->setData(resultModel->index(1, 0), temp, Qt::DisplayRole);
                break;
            case HypothesisTest::TailPositive:
                t *= -1;
                p_value = nsl_stats_tdist_p(t, df);

                temp = i18n("Null Hypothesis : mean of %1 %2 mean of %3", m_columns[0]->name(), QChar(0x2264), m_columns[1]->name());
                resultModel->setData(resultModel->index(0, 0), temp, Qt::DisplayRole);

                temp = i18n("Alternate Hypothesis : mean of %1 %2 mean of %3", m_columns[0]->name(), QChar(0x3E), m_columns[1]->name());
                resultModel->setData(resultModel->index(1, 0), temp, Qt::DisplayRole);
                break;
            case HypothesisTest::TailTwo:
                p_value = nsl_stats_tdist_p(t, df) + nsl_stats_tdist_p(-1*t, df);

                temp = i18n("Null Hypothesis : mean of %1 %2 mean of %3", m_columns[0]->name(), QChar(0x3D), m_columns[1]->name());
                resultModel->setData(resultModel->index(0, 0), temp, Qt::DisplayRole);

                temp = i18n("Alternate Hypothesis : mean of %1 %2 mean of %3", m_columns[0]->name(), QChar(0x2260), m_columns[1]->name());
                resultModel->setData(resultModel->index(1, 0), temp, Qt::DisplayRole);
                break;
        }


        resultModel->setData(resultModel->index(3, 0), i18n("T value is %1", t), Qt::DisplayRole);
        resultModel->setData(resultModel->index(4, 0), i18n("P value is %1", p_value), Qt::DisplayRole);
        resultModel->setData(resultModel->index(5, 0), i18n("DoF is %1", df), Qt::DisplayRole);

        if (p_value <= 0.05)
            temp = i18n("We can safely reject Null Hypothesis for significance level %1", 0.05);
        else
            temp = i18n("There is a plausibility for Null Hypothesis to be true");

        // tool tips
        resultModel->setData(resultModel->index(4, 0), temp, Qt::ToolTipRole);
//        resultModel->setData(resultModel->index(0, 0), QIcon("open.xpm"), Qt::DecorationRole);

        emit q->changed();
        return;
    }
    else if (test == TestZ) {
        m_currTestName = i18n("Two Sample Independent Z Test for %1 vs %2", m_columns[0]->name(), m_columns[1]->name());
        double t;
        int df;
        QString temp = "";
        double p_value = 0;

        resultModel->setRowCount(7);
        resultModel->setColumnCount(1);

        df = n[0] + n[1] - 2;

        //Assuming equal variance
        double sp = qSqrt( ((n[0]-1)*qPow(std[0],2) + (n[1]-1)*qPow(std[1],2))/df);
        t = (mean[0] - mean[1])/(sp*qSqrt(1.0/n[0] + 1.0/n[1]));

        resultModel->setData(resultModel->index(6, 0), i18n("Central Limit Theorem is Valid"), Qt::DisplayRole);

        switch (tail_type) {
            case HypothesisTest::TailNegative:
                p_value = nsl_stats_tdist_p(t, df);

                temp = i18n("Null Hypothesis : mean of %1 %2 mean of %3", m_columns[0]->name(), QChar(0x2265), m_columns[1]->name());
                resultModel->setData(resultModel->index(0, 0), temp, Qt::DisplayRole);

                temp = i18n("Alternate Hypothesis : mean of %1 %2 mean of %3", m_columns[0]->name(), QChar(0x3C), m_columns[1]->name());
                resultModel->setData(resultModel->index(1, 0), temp, Qt::DisplayRole);
                break;
            case HypothesisTest::TailPositive:
                t *= -1;
                p_value = nsl_stats_tdist_p(t, df);

                temp = i18n("Null Hypothesis : mean of %1 %2 mean of %3", m_columns[0]->name(), QChar(0x2264), m_columns[1]->name());
                resultModel->setData(resultModel->index(0, 0), temp, Qt::DisplayRole);

                temp = i18n("Alternate Hypothesis : mean of %1 %2 mean of %3", m_columns[0]->name(), QChar(0x3E), m_columns[1]->name());
                resultModel->setData(resultModel->index(1, 0), temp, Qt::DisplayRole);
                break;
            case HypothesisTest::TailTwo:
                p_value = nsl_stats_tdist_p(t, df) + nsl_stats_tdist_p(-1*t, df);

                temp = i18n("Null Hypothesis : mean of %1 %2 mean of %3", m_columns[0]->name(), QChar(0x3D), m_columns[1]->name());
                resultModel->setData(resultModel->index(0, 0), temp, Qt::DisplayRole);

                temp = i18n("Alternate Hypothesis : mean of %1 %2 mean of %3", m_columns[0]->name(), QChar(0x2260), m_columns[1]->name());
                resultModel->setData(resultModel->index(1, 0), temp, Qt::DisplayRole);
                break;
        }


        resultModel->setData(resultModel->index(3, 0), i18n("Z value is %1", t), Qt::DisplayRole);
        resultModel->setData(resultModel->index(4, 0), i18n("P value is %1", p_value), Qt::DisplayRole);
//        resultModel->setData(resultModel->index(5, 0), i18n("DoF is %1", df), Qt::DisplayRole);

        if (p_value <= 0.05)
            temp = i18n("We can safely reject Null Hypothesis for significance level %1", 0.05);
        else
            temp = i18n("There is a plausibility for Null Hypothesis to be true");

        // tool tips
        resultModel->setData(resultModel->index(4, 0), temp, Qt::ToolTipRole);
//        resultModel->setData(resultModel->index(0, 0), QIcon("open.xpm"), Qt::DecorationRole);

        emit q->changed();
        return;
    }

}

/********************************Two Sample Paired ***************************************/

void HypothesisTestPrivate::performTwoSamplePairedTest(Test test) {
    dataModel->clear();
    horizontalHeaderModel->clear();

    QMessageBox* msg_box = new QMessageBox();
    // checking for cols;
    if (m_columns.size() != 2) {
        msg_box->setText(i18n("Inappropriate number of columns selected"));
        msg_box->exec();
        return;
    }

    bool modeOk = true;
    for (int i = 0; i < 2; i++) {
        if(m_columns[i]->columnMode() == AbstractColumn::Numeric || m_columns[i]->columnMode() == AbstractColumn::Integer)
            continue;
        modeOk = false;
    }


    if (!modeOk) {
        msg_box->setText(i18n("select only columns with numbers"));
        msg_box->exec();
        return;
    }

    int n;
    double sum, mean, std;
    findStatsPaired(m_columns[0], m_columns[1], n, sum, mean, std);

    if (n == -1) {
        msg_box->setText(i18n("both columns are having different sizes"));
        msg_box->exec();
        return;
    }

    if (n < 1) {
        msg_box->setText(i18n("columns are empty"));
        msg_box->exec();
        return;
    }

    if (test == TestT) {
        m_currTestName = i18n("Two Sample Paired T Test");
        dataModel->setRowCount(1);
        dataModel->setColumnCount(3);

        horizontalHeaderModel->setColumnCount(3);

        double t = mean / (std/qSqrt(n));
        int df = n - 1;

        double p_value = 0;
        if (tail_type == HypothesisTest::TailNegative)
            p_value = nsl_stats_tdist_p(t, df);
        else if (tail_type == HypothesisTest::TailPositive) {
            t *= -1;
            p_value = nsl_stats_tdist_p(t, df);
        } else {
            p_value = nsl_stats_tdist_p(t, df) + nsl_stats_tdist_p(-1*t, df);
        }



    //    QString text = i18n("T value for test is %1 and\n p value is %2",t, p_value);
    //    msg_box->setText(text);
    //    msg_box->exec();

        //setting dataModel
        dataModel->setItem(0, 0, new QStandardItem(QString::number(t)));
        dataModel->setItem(0, 1, new QStandardItem(QString::number(df)));
        dataModel->setItem(0, 2, new QStandardItem(QString::number(p_value)));


        //setting horizontal header model
        horizontalHeaderModel->setHeaderData(0, Qt::Horizontal, "t value");
        horizontalHeaderModel->setHeaderData(1, Qt::Horizontal, "dof");
        horizontalHeaderModel->setHeaderData(2, Qt::Horizontal, "p value");

        emit q->changed();
        return;
    } else if (test == TestZ) {
        m_currTestName = i18n("Two Sample Paired Z Test");
        dataModel->setRowCount(1);
        dataModel->setColumnCount(2);

        horizontalHeaderModel->setColumnCount(2);

        double z = mean / (std/qSqrt(n));
        int df = n - 1;

        double p_value = 0;
        if (tail_type == HypothesisTest::TailNegative)
            p_value = nsl_stats_tdist_p(z, df);
        else if (tail_type == HypothesisTest::TailPositive) {
            z *= -1;
            p_value = nsl_stats_tdist_p(z, df);
        } else {
            p_value = nsl_stats_tdist_p(z, df) + nsl_stats_tdist_p(-1*z, df);
        }


        // now finding p value from t value

    //    QString text = i18n("T value for test is %1 and\n p value is %2",t, p_value);
    //    msg_box->setText(text);
    //    msg_box->exec();

        //setting dataModel
        dataModel->setItem(0, 0, new QStandardItem(QString::number(z)));
        dataModel->setItem(0, 1, new QStandardItem(QString::number(p_value)));


        //setting horizontal header model
        horizontalHeaderModel->setHeaderData(0, Qt::Horizontal, "z value");
        horizontalHeaderModel->setHeaderData(1, Qt::Horizontal, "p value");

        emit q->changed();
        return;
    }
}

/******************************** One Sample ***************************************/

void HypothesisTestPrivate::PerformOneSampleTest(Test test) {
    double population_mean = 0;
    dataModel->clear();
    horizontalHeaderModel->clear();

    QMessageBox* msg_box = new QMessageBox();
    // checking for cols;
    if (m_columns.size() != 1) {
        msg_box->setText(i18n("Inappropriate number of columns selected"));
        msg_box->exec();
        return;
    }

    if ( !(m_columns[0]->columnMode() == AbstractColumn::Numeric || m_columns[0]->columnMode() == AbstractColumn::Integer)) {
        msg_box->setText(i18n("select only columns with numbers"));
        msg_box->exec();
        return;
    }

    int n;
    double sum, mean, std;
    findStats(m_columns[0], n, sum, mean, std);

    if (n < 1) {
        msg_box->setText(i18n("column is empty"));
        msg_box->exec();
        return;
    }

    if (test == TestT) {
        m_currTestName = i18n("One Sample T Test");
        dataModel->setRowCount(1);
        dataModel->setColumnCount(3);

        horizontalHeaderModel->setColumnCount(3);

        double t = (mean - population_mean) / (std/qSqrt(n));
        int df = n - 1;


        double p_value = 0;
        if (tail_type == HypothesisTest::TailNegative)
            p_value = nsl_stats_tdist_p(t, df);
        else if (tail_type == HypothesisTest::TailPositive) {
            t *= -1;
            p_value = nsl_stats_tdist_p(t, df);
        } else {
            p_value = nsl_stats_tdist_p(t, df) + nsl_stats_tdist_p(-1*t, df);
        }


    //    QString text = i18n("T value for test is %1 and\n p value is %2",t, p_value);
    //    msg_box->setText(text);
    //    msg_box->exec();

        //setting dataModel
        dataModel->setItem(0, 0, new QStandardItem(QString::number(t)));
        dataModel->setItem(0, 1, new QStandardItem(QString::number(df)));
        dataModel->setItem(0, 2, new QStandardItem(QString::number(p_value)));


        //setting horizontal header model
        horizontalHeaderModel->setHeaderData(0, Qt::Horizontal, "t value");
        horizontalHeaderModel->setHeaderData(1, Qt::Horizontal, "dof");
        horizontalHeaderModel->setHeaderData(2, Qt::Horizontal, "p value");

        emit q->changed();
        return;
    } else if (test == TestZ) {
        m_currTestName = i18n("One Sample Z Test");
        dataModel->setRowCount(1);
        dataModel->setColumnCount(2);

        horizontalHeaderModel->setColumnCount(2);

        double z = (mean - population_mean) / (std/qSqrt(n));
        int df = n - 1;

        double p_value = 0;
        if (tail_type == HypothesisTest::TailNegative)
            p_value = nsl_stats_tdist_p(z, df);
        else if (tail_type == HypothesisTest::TailPositive) {
            z *= -1;
            p_value = nsl_stats_tdist_p(z, df);
        } else {
            p_value = nsl_stats_tdist_p(z, df) + nsl_stats_tdist_p(-1*z, df);
        }
        // now finding p value from t value

    //    QString text = i18n("T value for test is %1 and\n p value is %2",t, p_value);
    //    msg_box->setText(text);
    //    msg_box->exec();

        //setting dataModel
        dataModel->setItem(0, 0, new QStandardItem(QString::number(z)));
        dataModel->setItem(0, 1, new QStandardItem(QString::number(p_value)));


        //setting horizontal header model
        horizontalHeaderModel->setHeaderData(0, Qt::Horizontal, "z value");
        horizontalHeaderModel->setHeaderData(1, Qt::Horizontal, "p value");

        emit q->changed();
        return;
    }
}

void HypothesisTestPrivate::findStatsPaired(Column* column1, Column* column2, int &count, double &sum, double &mean, double &std) {
    sum = 0;
    mean = 0;
    std = 0;

    int count1 = column1->rowCount();
    int count2 = column2->rowCount();

    count = qMin(count1, count2);
    double row1, row2;
    for (int i = 0; i < count; i++) {
        row1 = column1->valueAt(i);
        row2 = column2->valueAt(i);

        if (std::isnan(row1) || std::isnan(row2)) {
            if (std::isnan(row1) && std::isnan(row2))
                count = i;
            else {
                count = -1;
                return;
            }
            break;
        }

        sum += row1 - row2;
    }

    if (count < 1) return;
    mean = sum/count;

    double row;
    for (int i = 0; i < count; i++) {
        row1 = column1->valueAt(i);
        row2 = column2->valueAt(i);
        row = row1 - row2;
        std += qPow( (row - mean), 2);
    }

    if (count > 1)
        std = std / (count-1);

    std = qSqrt(std);
    return;
}

void HypothesisTestPrivate::findStats(Column* column, int &count, double &sum, double &mean, double &std) {
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
        std += qPow( (row - mean), 2);
    }

    if (count > 1)
        std = std / (count-1);
    std = qSqrt(std);
    return;
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
