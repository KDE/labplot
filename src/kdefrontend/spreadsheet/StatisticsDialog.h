#ifndef STATISTICSDIALOG_H
#define STATISTICSDIALOG_H

#include "ui_statisticswidget.h"
#include <QTabWidget>
#include <QDialogButtonBox>

class Column;

class StatisticsDialog : public QTabWidget
{
    Q_OBJECT
public:
    explicit StatisticsDialog(const QString&, QWidget *parent = 0);
    void setColumns(const QList<Column*>& columns);
    void addColumn(Column* col);
private:
    void addTabs();
    const QString isNanValue(const double value);
    Ui::StatisticsDialog ui;
    QDialogButtonBox* m_okButton;
    QString m_htmlText;
    QList<Column*> m_columns;
private slots:
    void calculateStatisticsOnCurrentTab(int index);
protected:
    void showEvent(QShowEvent*);
    void keyPressEvent(QKeyEvent*);
};

#endif
