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

class AspectTreeModel;
class PivotTable;
class TreeViewComboBox;
class KConfig;

class PivotTableDock : public BaseDock {
	Q_OBJECT

public:
	explicit PivotTableDock(QWidget*);
	void setPivotTable(PivotTable*);

	void updateLocale() override;
	void retranslateUi() override;

private:
	Ui::PivotTableDock ui;
	bool m_initializing{false};
	TreeViewComboBox* cbSpreadsheet{nullptr};
	PivotTable* m_pivotTable{nullptr};
	AspectTreeModel* m_aspectTreeModel{nullptr};
	QSqlDatabase m_db;
	QString m_configPath;

	void load();
	void loadConfig(KConfig&);
	void setModelIndexFromAspect(TreeViewComboBox*, const AbstractAspect*);
	void readConnections();
	void updateFields();
	bool fieldSelected(const QString&);

private Q_SLOTS:
	//SLOTs for changes triggered in PivotTableDock
	void nameChanged();
	void commentChanged();
	void dataSourceTypeChanged(int);
	void spreadsheetChanged(const QModelIndex&);
	void connectionChanged();
	void tableChanged();
	void showDatabaseManager();

	//SLOTs for changes triggered in PivotTable
	void pivotTableDescriptionChanged(const AbstractAspect*);

	void addRow();
	void removeRow();
	void addColumn();
	void removeColumn();

	//save/load template
	void loadConfigFromTemplate(KConfig&);
	void saveConfigAsTemplate(KConfig&);

Q_SIGNALS:
	void info(const QString&);
};

#endif // PIVOTTABLEDOCK_H
