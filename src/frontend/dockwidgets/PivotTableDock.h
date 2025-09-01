/***************************************************************************
	File                 : PivotTableDock.ch
	Project              : LabPlot
	Description          : widget for pivot table properties
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2019-2025 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PIVOTTABLEDOCK_H
#define PIVOTTABLEDOCK_H

#include "frontend/dockwidgets/BaseDock.h"
#include "backend/pivot/PivotTable.h"
#include "ui_pivottabledock.h"

#include <QSqlDatabase>

class PivotTable;
class TreeViewComboBox;
class QComboBox;
class KConfig;

class PivotTableDock : public BaseDock {
	Q_OBJECT

public:
	explicit PivotTableDock(QWidget*);
	void setPivotTable(PivotTable*);

	void retranslateUi() override;

private:
	Ui::PivotTableDock ui;
	bool m_initializing{false};
	TreeViewComboBox* cbSpreadsheet{nullptr};
	PivotTable* m_pivotTable{nullptr};
	QSqlDatabase m_db;
	QString m_configPath;
	QVector<QComboBox*> m_valueNameComboBoxes;
	QVector<QComboBox*> m_valueAggregationComboBoxes;
	QVector<QPushButton*> m_removeValueButtons;

	void load();
	void loadFields();
	void loadValues();
	void readConnections();
	bool fieldSelected(const QString&);

private Q_SLOTS:
	//SLOTs for changes triggered in PivotTableDock
	void dataSourceTypeChanged(int);
	void spreadsheetChanged(const QModelIndex&);
	void connectionChanged();
	void tableChanged();
	void showDatabaseManager();

	//SLOTs for changes triggered in PivotTable

	void addRow();
	void removeRow();

	void addColumn();
	void removeColumn();

	void addValue();
	void removeValue();

Q_SIGNALS:
	void info(const QString&);
};

#endif // PIVOTTABLEDOCK_H
