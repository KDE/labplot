#include "TTest.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/core/column/Column.h"
//#include "commonfrontend/spreadsheet/SpreadsheetView.h"

#include <QVector>
#include <QMessageBox>
#include <QtMath>
#include <KLocalizedString>


TTest::TTest(const QString &name){
    Q_UNUSED(name);
}

void TTest::setDataSourceSpreadsheet(Spreadsheet *spreadsheet){
    dataSourceSpreadsheet = spreadsheet;

    m_rowCount = dataSourceSpreadsheet->rowCount();
    m_columnCount = dataSourceSpreadsheet->columnCount();
    QDEBUG("in ttest::setDataSourceSpreadsheet");

    // now finding the number of columns and rows;
    QDEBUG("row count is " << m_rowCount);
    QDEBUG("row count is " << m_columnCount);
    QDEBUG("exiting ttest::setDataSourceSpreadsheet");
}

void TTest::setColumns(QVector<Column*> cols){
    m_columns = cols;
    return;
}

void TTest::performTwoSampleTest(){
    QMessageBox* msg_box = new QMessageBox();
    // checking for cols;
    if(m_columns.size() != 2){
        msg_box->setText(i18n("Inappropriate number of columns selected"));
        msg_box->exec();
        return;
    }

    bool modeOk = true;
    for (int i = 0; i < 2; i++){
        if(m_columns[0]->columnMode() == AbstractColumn::Numeric || m_columns[i]->columnMode() == AbstractColumn::Integer)
            continue;
        modeOk = false;
    }

    if(!modeOk){
        msg_box->setText(i18n("select only columns with numbers"));
        msg_box->exec();
        return;
    }

    // use of three than two for human readiblity of code;
    int n[2];
    double sum[2], mean[2], std[2];

    for (int i = 0; i < 2; i++) {
        findStats(m_columns[i], n[i], sum[i], mean[i], std[i]);
        QDEBUG("for " << i);
        QDEBUG("n is "<<n[i]);
        QDEBUG("mean is " << mean[i]);
        QDEBUG("std is " << std[i]);

        if(n[i] < 1) {
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
    QString text = i18n("T value for test is %1",t);
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



