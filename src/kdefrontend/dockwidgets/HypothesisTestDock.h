/***************************************************************************
    File                 : HypothesisTestDock.h
    Project              : LabPlot
    Description          : widget for hypothesis testing properties
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

#ifndef HYPOTHESISTESTDOCK_H
#define HYPOTHESISTESTDOCK_H

#include "backend/hypothesis_test/HypothesisTest.h"
#include "ui_hypothesistestdock.h"
#include <QSqlDatabase>

class AbstractAspect;
class AspectTreeModel;
class HypothesisTest;
class TreeViewComboBox;
class KConfig;

class HypothesisTestDock : public QWidget {
        Q_OBJECT

public:
        explicit HypothesisTestDock(QWidget*);
        void setHypothesisTest(HypothesisTest*);

private slots:
        void on_rb_h1_one_tail_1_toggled(bool checked);
        void on_rb_h1_one_tail_2_toggled(bool checked);
        void on_rb_h1_two_tail_toggled(bool checked);

private slots:

private:
        Ui::HypothesisTestDock ui;
//        bool m_initializing{false};
        TreeViewComboBox* cbSpreadsheet{nullptr};
        HypothesisTest* m_hypothesisTest{nullptr};
////        AspectTreeModel* m_aspectTreeModel{nullptr};
        QSqlDatabase m_db;
        QString m_configPath;
        QWidget* wid;
//        void load();
//        void loadConfig(KConfig&);
//        void setModelIndexFromAspect(TreeViewComboBox*, const AbstractAspect*);
//        void readConnections();
//        void updateFields();
//        bool fieldSelected(const QString&);
        bool ttest{false};
        bool ztest{false};
        bool two_sample_independent{false};
        bool two_sample_paired{false};
        bool one_sample{false};


private slots:
        //SLOTs for changes triggered in PivotTableDock
//        void nameChanged();
//        void commentChanged();
        void dataSourceTypeChanged(int);
        void doHypothesisTest();
        void showHypothesisTest(QTreeWidgetItem* item, int col);
        void spreadsheetChanged(const QModelIndex&);
//        void connectionChanged();
//        void tableChanged();
//        void showDatabaseManager();

//        //SLOTs for changes triggered in PivotTable
//        void pivotTableDescriptionChanged(const AbstractAspect*);

//        void addRow();
//        void removeRow();
//        void addColumn();
//        void removeColumn();

//        //save/load template
//        void loadConfigFromTemplate(KConfig&);
//        void saveConfigAsTemplate(KConfig&);

signals:
//        void info(const QString&);
};

#endif // PIVOTTABLEDOCK_H
