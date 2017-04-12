/***************************************************************************
    File                 : ImportSQLDatabaseWidget.cpp
    Project              : LabPlot
    Description          : Datapicker
    --------------------------------------------------------------------
    Copyright            : (C) 2016 by Ankit Wagadre (wagadre.ankit@gmail.com)
    Copyright            : (C) 2016-2017 Alexander Semke (alexander.semke@web.de)

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

#include "ImportSQLDatabaseWidget.h"
#include "DatabaseManagerDialog.h"
#include "backend/lib/macros.h"

#include <KStandardDirs>

#include <QtSql>
#include <QStandardItem>
#include <QTreeView>

ImportSQLDatabaseWidget::ImportSQLDatabaseWidget(QWidget* parent):QWidget(parent),
	m_aspectTreeModel(0), m_databaseTreeModel(0) {
	ui.setupUi(this);

	ui.bDatabaseManager->setIcon(KIcon("network-server-database"));
	connect( ui.bDatabaseManager, SIGNAL(clicked()), this, SLOT(showDatabaseManager()) );

	//defer the loading of settings a bit in order to show the dialog prior to blocking the GUI in refreshPreview()
	QTimer::singleShot( 100, this, SLOT(loadSettings()) );
}

void ImportSQLDatabaseWidget::loadSettings() {
	//read available connections
	readConnections();

	//load last used connection and other settings
	KConfigGroup conf(KSharedConfig::openConfig(), "ImportSQLDatabaseWidget");
	ui.cbConnection->setCurrentIndex(ui.cbConnection->findText(conf.readEntry("Connection", "")));
	//TODO
}

ImportSQLDatabaseWidget::~ImportSQLDatabaseWidget() {
	// save current settings
	KConfigGroup conf(KSharedConfig::openConfig(), "ImportSQLDatabaseWidget");
	conf.writeEntry("Connection", ui.cbConnection->currentText());
	//TODO
}

/*!
	loads all available saved connections
*/
void ImportSQLDatabaseWidget::readConnections() {
	DEBUG_LOG("ImportSQLDatabaseWidget: reading available connections");
	const QString m_configPath = KGlobal::dirs()->locateLocal("appdata", "") + QLatin1String("sql_connections");
	KConfig confConn(m_configPath, KConfig::SimpleConfig);
	foreach(QString name, confConn.groupList())
		ui.cbConnection->addItem(name);
}

void ImportSQLDatabaseWidget::setDatabaseModel() {
	m_databaseTreeModel = new QStandardItemModel();
	QTreeView* databaseTreeView = new QTreeView(this);
	QSqlQuery tableListQuery(m_db);
	tableListQuery.prepare("SHOW TABLES");
	tableListQuery.exec();
	if(tableListQuery.isActive()) {
		while(tableListQuery.next()) {
			QString tableName = tableListQuery.value(0).toString().simplified();
			QStandardItem* tableItem = new QStandardItem(tableName);
			tableItem->setSelectable(false);
			m_databaseTreeModel->appendRow(tableItem);
			QSqlQuery columnListQuery(m_db);
			columnListQuery.prepare("SELECT COLUMN_NAME FROM INFORMATION_SCHEMA.COLUMNS \
                          WHERE TABLE_NAME = '" + tableName + "'");
			columnListQuery.exec();
			if (columnListQuery.isActive()) {
				while(columnListQuery.next()) {
					QString columnName = columnListQuery.value(0).toString().simplified();
					QStandardItem* columnItem = new QStandardItem(columnName);
					columnItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
					columnItem->setData(Qt::Unchecked, Qt::CheckStateRole);
					tableItem->appendRow(columnItem);
				}
			}
		}
	}

	databaseTreeView->setModel(m_databaseTreeModel);
	ui.cbDatabaseTree->setModel(m_databaseTreeModel);
	databaseTreeView->header()->close();
	ui.cbDatabaseTree->setView(databaseTreeView);
}

void ImportSQLDatabaseWidget::connectDatabase() {
// 	m_db = QSqlDatabase::addDatabase( vendorList.at(ui.cbVendor->currentIndex()) );
// 	m_db.setHostName( ui.leHostName->text() );
// 	m_db.setPort( ui.sbPort->value() );
// 	m_db.setDatabaseName( ui.leDatabaseName->text() );
// 	m_db.setUserName( ui.leUserName->text() );
// 	m_db.setPassword( ui.lePassword->text() );
// 
// 	if (m_db.isValid()) {
// 		m_db.open();
// 		if (m_db.isOpen()) {
// 			setDatabaseModel();
// 			m_db.close();
// 		}
// 	}
// 
// 	updateStatus();
}

void ImportSQLDatabaseWidget::previewColumn(QString columnNameList, QString tableName, int columnCount, bool showPreview) {
	QSqlQuery searchQuery(m_db);
	QString query = "SELECT " + columnNameList + " FROM " + tableName;
	searchQuery.prepare(query);
	searchQuery.exec();
	if(searchQuery.isActive()) {
		int row = 0;
		int rowCount = ui.sbEndRow->value()-ui.sbStartRow->value()+1;
		if (showPreview) {
			int row = 0;
			int prevColumnCount = ui.twPreviewTable->columnCount();
			ui.twPreviewTable->setColumnCount(prevColumnCount + columnCount);
			while(searchQuery.next()) {
				if (row >= ui.sbStartRow->value() && row <= ui.sbEndRow->value()) {
					for(int column = 0; column < columnCount; column++) {
						ui.twPreviewTable->setItem( row - ui.sbStartRow->value(), column + prevColumnCount,
						                            new QTableWidgetItem(searchQuery.value(column).toString()) );
					}
				}
				row++;
			}
		} else {
			//TODO
// 			Spreadsheet* spreadsheet = dynamic_cast<Spreadsheet*>(m_sheet);
// 			if (spreadsheet) {
// 				int prevColumnCount = spreadsheet->columnCount();
// 				spreadsheet->setColumnCount(prevColumnCount + columnCount);
// 				if (rowCount > spreadsheet->rowCount()) spreadsheet->setRowCount(rowCount);
// 				while(searchQuery.next()) {
// 					if (row >= ui.sbStartRow->value() && row <= ui.sbEndRow->value()) {
// 						for(int index = 0; index < columnCount; index++) {
// 							spreadsheet->column(index + prevColumnCount)->setColumnMode(AbstractColumn::Text);
// 							spreadsheet->column(index + prevColumnCount)->setTextAt(row - ui.sbStartRow->value(), searchQuery.value(index).toString());
// 						}
// 					}
// 					row++;
// 				}
// 			}
		}
	}

	updateStatus();
}

void ImportSQLDatabaseWidget::showPreview() {
	importData(true);
}

void ImportSQLDatabaseWidget::importData(bool showPreview) {
	if (!m_db.isValid()) return;
	if (ui.sbEndRow->value() < ui.sbStartRow->value()) return;

	if (showPreview) {
		ui.twPreviewTable->setColumnCount(0);
		ui.twPreviewTable->setRowCount(ui.sbEndRow->value() - ui.sbStartRow->value() + 1);
	}

	m_db.open();
	if (m_db.isOpen()) {
		for(int tableIndex = 0; tableIndex < m_databaseTreeModel->rowCount(); tableIndex++) {
			QString tableName = m_databaseTreeModel->item(tableIndex)->text();
			QString columnNameList;
			int columnCount = 0;
			for(int columnIndex = 0; columnIndex < m_databaseTreeModel->item(tableIndex)->rowCount(); columnIndex++) {
				QStandardItem* columnItem = m_databaseTreeModel->item(tableIndex)->child(columnIndex);
				if (columnItem->checkState() == Qt::Checked) {
					if (!columnNameList.isEmpty()) columnNameList += " , ";
					columnNameList += columnItem->text();
					columnCount++;
				}
			}
			if (columnCount) previewColumn(columnNameList, tableName, columnCount, showPreview);
		}
		m_db.close();
	}

	updateStatus();
}

void ImportSQLDatabaseWidget::updateStatus() {
	QString msg = m_db.lastError().text().simplified();
	if (msg.isEmpty())
		msg = "Done";

	emit statusChanged( msg );
}

/*!
	shows the database manager where the connections are created and edited.
	The selected connection is selected in the connection combo box in this widget.
**/
void ImportSQLDatabaseWidget::showDatabaseManager() {
	DatabaseManagerDialog* dlg = new DatabaseManagerDialog(this);

	if (dlg->exec() == QDialog::Accepted) {
		//re-read the available connections to be in sync with the changes in DatabaseManager
		ui.cbConnection->clear();
		readConnections();

		//select the connection the user has selected in DataabaseManager
		QString conn = dlg->connection();
		ui.cbConnection->setCurrentIndex(ui.cbConnection->findText(conn));
	}

	delete dlg;
}
