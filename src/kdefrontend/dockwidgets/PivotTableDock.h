/***************************************************************************
    File                 : PivotTableDock.h
    Project              : LabPlot
    Description          : widget for pivot table properties
    --------------------------------------------------------------------
    Copyright            : (C) 2019 by Alexander Semke (alexander.semke@web.de)

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

#ifndef PIVOTTABLEDOCK_H
#define PIVOTTABLEDOCK_H

#include "backend/pivot/PivotTable.h"
#include "ui_pivottabledock.h"
#include <QSqlDatabase>

class AspectTreeModel;
class PivotTable;
class TreeViewComboBox;
class KConfig;

class PivotTableDock : public QWidget {
	Q_OBJECT

public:
	explicit PivotTableDock(QWidget*);
	void setPivotTable(PivotTable*);

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

private slots:
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

	//save/load template
	void loadConfigFromTemplate(KConfig&);
	void saveConfigAsTemplate(KConfig&);

signals:
	void info(const QString&);
};

#endif // PIVOTTABLEDOCK_H
