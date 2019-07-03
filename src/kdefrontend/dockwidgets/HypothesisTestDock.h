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

#include "backend/hypothesisTest/HypothesisTest.h"
#include "ui_hypothesistestdock.h"
#include <QSqlDatabase>

class AbstractAspect;
class AspectTreeModel;
class HypothesisTest;
class TreeViewComboBox;
class KConfig;
class QScrollArea;
class QStandardItemModel;
class QStandardItem;
class QComboBox;

class HypothesisTestDock : public QWidget {
	Q_OBJECT

public:
	explicit HypothesisTestDock(QWidget*);
	void setHypothesisTest(HypothesisTest*);

private:
	Ui::HypothesisTestDock ui;
	bool m_initializing{false};
	TreeViewComboBox* cbSpreadsheet{nullptr};
	HypothesisTest* m_hypothesisTest{nullptr};
	AspectTreeModel* m_aspectTreeModel{nullptr};
	QSqlDatabase m_db;
	QString m_configPath;
	double m_populationMean{0};
	double m_significanceLevel{0.05};
	//        void load();
	//        void loadConfig(KConfig&);
	void setModelIndexFromAspect(TreeViewComboBox*, const AbstractAspect*);
	//        void readConnections();
	//        void updateFields();
	//        bool fieldSelected(const QString&);
	HypothesisTest::Test m_test;
	QScrollArea* scrollDock;

	void countPartitions(Column *column, int &np, int &total_rows);
	void setColumnsComboBoxModel(Spreadsheet* spreadsheet);
	void setColumnsComboBoxView();
	bool nonEmptySelectedColumns();
	std::uint8_t setAllBits(const std::uint8_t& bits);

	QList<Column* > m_onlyValuesCols;
	QList<Column* > m_twoCategoricalCols;
	QList<Column* > m_multiCategoricalCols;

private slots:
	//SLOTs for changes triggered in PivotTableDock
	//        void nameChanged();
	//        void commentChanged();
	void dataSourceTypeChanged(int);
	void showTestType();
	void showHypothesisTest();
	void doHypothesisTest();
	void performLeveneTest();
	void spreadsheetChanged(const QModelIndex&);
	void changeCbCol2Label();
	void chbPopulationSigmaStateChanged();
	void col1IndexChanged(int index);

	void onRbH1OneTail1Toggled(bool checked);
	void onRbH1OneTail2Toggled(bool checked);
	void onRbH1TwoTailToggled(bool checked);
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
