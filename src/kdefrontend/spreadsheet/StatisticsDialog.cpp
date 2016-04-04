#include "StatisticsDialog.h"
#include "backend/core/column/Column.h"

#include <QTextEdit>
#include <QKeyEvent>
#include <QDesktopWidget>
#include <QVBoxLayout>

StatisticsDialog::StatisticsDialog(const QString & title, QWidget *parent) :
    QTabWidget(parent){
    ui.setupUi(this);

    removeTab(0);
    removeTab(0);
    setWindowTitle(title);

    m_htmlText = QString("<table border=0 width=100%>"
                         "<tr>"
                         "<td colspan=2 align=center bgcolor=#D1D1D1><b><big>Location measures</big><b></td>"
                         "</tr>"
                         "<tr></tr>"
                         "<tr>"
                         "<td width=38%><b>Minimum<b></td>"
                         "<td>%1</td>"
                         "</tr>"
                         "<tr>"
                         "<td><b>Maximum<b></td>"
                         "<td>%2</td>"
                         "</tr>"
                         "<tr>"
                         "<td><b>Arithmetic mean<b></td>"
                         "<td>%3</td>"
                         "</tr>"
                         "<tr>"
                         "<td><b>Geometric mean<b></td>"
                         "<td>%4</td>"
                         "</tr>"
                         "<tr>"
                         "<td><b>Harmonic mean<b></td>"
                         "<td>%5</td>"
                         "</tr>"
                         "<tr>"
                         "<td><b>Contraharmonic mean<b></td>"
                         "<td>%6</td>"
                         "</tr>"
                         "<tr></tr>"
                         "<tr>"
                         "<td colspan=2 align=center bgcolor=#D1D1D1><b><big>Dispersion measures</big></b></td>"
                         "</tr>"
                         "<tr></tr>"
                         "<tr>"
                         "<td><b>Variance<b></td>"
                         "<td>%7</td>"
                         "</tr>"
                         "<tr>"
                         "<td><b>Standard deviation<b></td>"
                         "<td>%8</td>"
                         "</tr>"
                         "<tr>"
                         "<td><b>Mean deviation<b></td>"
                         "<td>%9</td>"
                         "</tr>"
                         "<tr>"
                         "<td><b>Median absolute deviation<b></td>"
                         "<td>%10</td>"
                         "</tr>"
                         "<tr></tr>"
                         "<tr>"
                         "<td colspan=2 align=center bgcolor=#D1D1D1><b><big>Shape measures</big></b></td>"
                         "</tr>"
                         "<tr></tr>"
                         "<tr>"
                         "<td><b>Skewness<b></td>"
                         "<td>%11</td>"
                         "</tr>"
                         "<tr>"
                         "<td><b>Kurtosis<b></td>"
                         "<td>%12</td>"
                         "</tr>"
                         "<tr>"
                         "<td><b>Entropy<b></td>"
                         "<td>%13</td>"
                         "</tr>"
                         "</table>");

    move(QApplication::desktop()->screen()->rect().center() - rect().center());
    connect(this, SIGNAL(currentChanged(int)), this, SLOT(calculateStatisticsOnCurrentTab(int)));
}

void StatisticsDialog::setColumns(const QList<Column *> &columns){
    m_columns = columns;
}

void StatisticsDialog::addColumn(Column *col){
    m_columns << col;
}

void StatisticsDialog::showEvent(QShowEvent * event){
    addTabs();
    event->accept();
}

void StatisticsDialog::addTabs(){
    for (int i = 0; i < m_columns.size(); ++i){
        QTextEdit* textEdit = new QTextEdit;
        textEdit->setReadOnly(true);

        if (i == 0){
            m_columns[0]->calculateStatistics();
            textEdit->setHtml(m_htmlText.arg(m_columns[0]->statistics().minimum).
                    arg(m_columns[0]->statistics().maximum).
                    arg(m_columns[0]->statistics().arithmeticMean).
                    arg(m_columns[0]->statistics().geometricMean).
                    arg(m_columns[0]->statistics().harmonicMean).
                    arg(m_columns[0]->statistics().contraharmonicMean).
                    arg(m_columns[0]->statistics().variance).
                    arg(m_columns[0]->statistics().standardDeviation).
                    arg(m_columns[0]->statistics().meanDeviation).
                    arg(m_columns[0]->statistics().medianAbsoluteDeviation).
                    arg(m_columns[0]->statistics().skewness).
                    arg(m_columns[0]->statistics().kurtosis).
                    arg(m_columns[0]->statistics().entropy));
        }

        QWidget* widget = new QWidget;
        m_okButton = new QDialogButtonBox(QDialogButtonBox::Ok);

        QVBoxLayout* vBox = new QVBoxLayout;
        vBox->addWidget(textEdit);
        vBox->addWidget(m_okButton);

        widget->setLayout(vBox);

        connect(m_okButton, SIGNAL(accepted()), this, SLOT(close()));
        addTab(widget, m_columns[i]->name());
    }
}

void StatisticsDialog::calculateStatisticsOnCurrentTab(int index){

    if(!m_columns[index]->statisticsAvailable()){
        m_columns[index]->calculateStatistics();
        QTextEdit* textEdit = currentWidget()->findChild<QTextEdit*>();

        textEdit->setHtml(m_htmlText.arg(m_columns[index]->statistics().minimum).
                          arg(m_columns[index]->statistics().maximum).
                          arg(m_columns[index]->statistics().arithmeticMean).
                          arg(m_columns[index]->statistics().geometricMean).
                          arg(m_columns[index]->statistics().harmonicMean).
                          arg(m_columns[index]->statistics().contraharmonicMean).
                          arg(m_columns[index]->statistics().variance).
                          arg(m_columns[index]->statistics().standardDeviation).
                          arg(m_columns[index]->statistics().meanDeviation).
                          arg(m_columns[index]->statistics().medianAbsoluteDeviation).
                          arg(m_columns[index]->statistics().skewness).
                          arg(m_columns[index]->statistics().kurtosis).
                          arg(m_columns[index]->statistics().entropy));
    }
}

void StatisticsDialog::keyPressEvent(QKeyEvent * event){
    switch (event->key()) {
    case Qt::Key_Escape:
        this->close();
        break;
    case Qt::Key_Enter:
        this->close();
    case Qt::Key_Return:
        this->close();
    default:
        event->accept();
        break;
    }
}
