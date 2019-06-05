/***************************************************************************
    File                 : HypothesisTest.cpp
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

void HypothesisTest::setDataSourceType(DataSourceType type) {
    if (type != d->dataSourceType) {
        d->dataSourceType = type;
    }
}

void HypothesisTest::setDataSourceSpreadsheet(Spreadsheet *spreadsheet) {
    if  (spreadsheet != d->dataSourceSpreadsheet)
        d->setDataSourceSpreadsheet(spreadsheet);
}

QStringList HypothesisTest::allColumns()
{
    return d->all_columns;
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
    d->performTwoSampleTTest();
}

QString HypothesisTest::testName() {
    return d->m_currTestName;
}

/******************************************************************************
 *                      Private Implementations
 * ****************************************************************************/

HypothesisTestPrivate::HypothesisTestPrivate(HypothesisTest* owner) : q(owner) ,
    dataModel(new QStandardItemModel) ,
    horizontalHeaderModel(new QStandardItemModel) {
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
//            qDebug() << "col is " << col;
            m_columns.append(column);
        }
    }
}

void HypothesisTestPrivate::performTwoSampleTTest() {
    dataModel->clear();
    horizontalHeaderModel->clear();
    m_currTestName = i18n("Independent Two Sample T Test");

    QMessageBox* msg_box = new QMessageBox();
    // checking for cols;
    if (m_columns.size() != 2) {
        msg_box->setText(i18n("Inappropriate number of columns selected"));
        msg_box->exec();
        return;
    }

    bool modeOk = true;
    bool allColumnsValid = true;
    for (int i = 0; i < 2; i++) {
        if(m_columns[i]->columnMode() == AbstractColumn::Numeric || m_columns[i]->columnMode() == AbstractColumn::Integer)
            continue;
        modeOk = false;
    }

    if(!allColumnsValid) {
        msg_box->setText(i18n("one of the selected columns is invalid"));
        msg_box->exec();
        return;
    }

    if (!modeOk) {
        msg_box->setText(i18n("select only columns with numbers"));
        msg_box->exec();
        return;
    }

    dataModel->setRowCount(1);
    dataModel->setColumnCount(3);

    horizontalHeaderModel->setColumnCount(3);

    int n[2];
    double sum[2], mean[2], std[2];

    for (int i = 0; i < 2; i++) {
        findStats(m_columns[i], n[i], sum[i], mean[i], std[i]);
        DEBUG("for " << i);
        DEBUG("n is "<<n[i]);
        DEBUG("mean is " << mean[i]);
        DEBUG("std is " << std[i]);

        if (n[i] < 1) {
            msg_box->setText(i18n("atleast one of selected column is empty"));
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
        std += qPow((row - mean),2);
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
