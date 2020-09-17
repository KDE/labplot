/***************************************************************************
    File                 : CorrelationCoefficientDock.h
    Project              : LabPlot
    Description          : widget for hypothesis testing properties
    --------------------------------------------------------------------
    Copyright            : (C) 2019 Devanshu Agarwal(agarwaldevanshu8@gmail.com)
    Copyright            : (C) 2020 Alexander Semke (alexander.semke@web.de)

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

#include "kdefrontend/dockwidgets/BaseDock.h"
#include "ui_correlationcoefficientdock.h"

class Column;
class Spreadsheet;
class AspectTreeModel;
class CorrelationCoefficient;
class TreeViewComboBox;

class CorrelationCoefficientDock : public BaseDock {
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
	void setModelIndexFromAspect(TreeViewComboBox*, const AbstractAspect*);
	int m_test = 0;
	void countPartitions(Column*, int &np, int &total_rows);

	void setColumnsComboBoxModel(Spreadsheet*);
	void setColumnsComboBoxView();
	bool nonEmptySelectedColumns();

	int testType(int test);
	int testSubType(int test);

	QList<Column*> m_onlyValuesCols;
	QList<Column*> m_twoCategoricalCols;
	QList<Column*> m_multiCategoricalCols;

private slots:
	//SLOTs for changes triggered in CorrelationCoefficientDock
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

	//SLOTs for changes triggered in CorrelationCoefficient
	void correlationCoefficientDescriptionChanged(const AbstractAspect*);
};
#endif // CORRELATIONCOEFFICIENTDOCK_H
