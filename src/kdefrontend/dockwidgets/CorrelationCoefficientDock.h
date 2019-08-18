/***************************************************************************
    File                 : CorrelationCoefficientDock.h
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

#ifndef CORRELATIONCOEFFICIENTDOCK_H
#define CORRELATIONCOEFFICIENTDOCK_H

#include "backend/generalTest/CorrelationCoefficient.h"
#include "ui_correlationcoefficientdock.h"
#include <QSqlDatabase>

//class Column;
//class Spreadsheet;
//class AbstractAspect;
class AspectTreeModel;
//class CorrelationCoefficient;
class TreeViewComboBox;
//class KConfig;
//class QStandardItemModel;
//class QStandardItem;
//class QComboBox;

class CorrelationCoefficientDock : public QWidget {
	Q_OBJECT

public:
	explicit CorrelationCoefficientDock(QWidget*);
	void setCorrelationCoefficient(CorrelationCoefficient*);

private:
	Ui::CorrelationCoefficientDock ui;
	bool m_initializing{false};
	TreeViewComboBox* cbSpreadsheet{nullptr};
	CorrelationCoefficient* m_correlationCoefficient{nullptr};
	AspectTreeModel* m_aspectTreeModel{nullptr};
	QSqlDatabase m_db;
	QString m_configPath;
	//        void load();
	//        void loadConfig(KConfig&);
	void setModelIndexFromAspect(TreeViewComboBox*, const AbstractAspect*);
	//        void readConnections();
	//        void updateFields();
	//        bool fieldSelected(const QString&);
	int m_test;
	void countPartitions(Column *column, int &np, int &total_rows);

	void setColumnsComboBoxModel(Spreadsheet* spreadsheet);
	void setColumnsComboBoxView();
	bool nonEmptySelectedColumns();

	int testType(int test);
	int testSubType(int test);

	QList<Column* > m_onlyValuesCols;
	QList<Column* > m_twoCategoricalCols;
	QList<Column* > m_multiCategoricalCols;
private slots:
	//SLOTs for changes triggered in PivotTableDock
	//        void nameChanged();
	//        void commentChanged();
	void dataSourceTypeChanged(int);
	void showTestType();
	void showCorrelationCoefficient();
	void findCorrelationCoefficient();
	void spreadsheetChanged(const QModelIndex&);
	void col1IndexChanged(int index);
	void changeCbCol2Label();
	void chbColumnStatsStateChanged();
	void leNRowsChanged();
	void leNColumnsChanged();
	void exportStatsTableToSpreadsheet();


	//        void connectionChanged();
	//        void tableChanged();
	//        void showDatabaseManager();

	//        //SLOTs for changes triggered in PivotTable
	void CorrelationCoefficientDescriptionChanged(const AbstractAspect*);

	//        //save/load template
	//        void loadConfigFromTemplate(KConfig&);
	//        void saveConfigAsTemplate(KConfig&);

signals:
	//        void info(const QString&);
};
#endif // CORRELATIONCOEFFICIENTDOCK_H
