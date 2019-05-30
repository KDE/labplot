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
