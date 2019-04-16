/***************************************************************************
    File                 : PivotTableDock.cpp
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

#include "PivotTableDock.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/core/Project.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "commonfrontend/widgets/TreeViewComboBox.h"
#include "kdefrontend/datasources/DatabaseManagerDialog.h"
#include "kdefrontend/datasources/DatabaseManagerWidget.h"
#include "kdefrontend/pivot/PivotTableView.h"
#include "kdefrontend/TemplateHandler.h"

#include <QDir>
#include <QSqlError>

#include <KConfig>
#include <KConfigGroup>
#include <KMessageBox>

 /*!
  \class PivotTableDock
  \brief Provides a widget for editing the properties of the matrices currently selected in the project explorer.

  \ingroup kdefrontend
*/
PivotTableDock::PivotTableDock(QWidget* parent) : QWidget(parent) {
	ui.setupUi(this);

	ui.cbDataSourceType->addItem(i18n("Spreadsheet"));
	ui.cbDataSourceType->addItem(i18n("Database"));

	cbSpreadsheet = new TreeViewComboBox;
	ui.gridLayout->addWidget(cbSpreadsheet, 5, 3, 1, 4);

	ui.bDatabaseManager->setIcon(QIcon::fromTheme("network-server-database"));
	ui.bDatabaseManager->setToolTip(i18n("Manage connections"));
	m_configPath = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).constFirst() +  "sql_connections";
	readConnections();

	auto* style = ui.bAddRow->style();
	ui.bAddRow->setIcon(style->standardIcon(QStyle::SP_ArrowRight));
	ui.bAddRow->setToolTip(i18n("Add the selected field to rows"));
	ui.bRemoveRow->setIcon(style->standardIcon(QStyle::SP_ArrowLeft));
	ui.bRemoveRow->setToolTip(i18n("Remove the selected field from rows"));

	ui.bAddColumn->setIcon(style->standardIcon(QStyle::SP_ArrowRight));
	ui.bAddColumn->setToolTip(i18n("Add the selected field to columns"));
	ui.bRemoveColumn->setIcon(style->standardIcon(QStyle::SP_ArrowLeft));
	ui.bRemoveColumn->setToolTip(i18n("Remove the selected field from columns"));

	//add/remove buttons only enabled if something was selected
	ui.bAddRow->setEnabled(false);
	ui.bRemoveRow->setEnabled(false);
	ui.bAddColumn->setEnabled(false);
	ui.bRemoveColumn->setEnabled(false);

	connect(ui.leName, &QLineEdit::textChanged, this, &PivotTableDock::nameChanged);
	connect(ui.leComment, &QLineEdit::textChanged, this, &PivotTableDock::commentChanged);
	connect(ui.cbDataSourceType, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			this, &PivotTableDock::dataSourceTypeChanged);

	connect(cbSpreadsheet, &TreeViewComboBox::currentModelIndexChanged, this, &PivotTableDock::spreadsheetChanged);
	connect(ui.cbConnection, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			this, &PivotTableDock::connectionChanged);
	connect(ui.cbTable, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			this, &PivotTableDock::tableChanged);
	connect(ui.bDatabaseManager, &QPushButton::clicked, this, &PivotTableDock::showDatabaseManager);

	connect(ui.bAddRow,  &QPushButton::clicked, this, &PivotTableDock::addRow);
	connect(ui.bRemoveRow, &QPushButton::clicked, this,&PivotTableDock::removeRow);
	connect(ui.bAddColumn,  &QPushButton::clicked, this, &PivotTableDock::addColumn);
	connect(ui.bRemoveColumn, &QPushButton::clicked, this,&PivotTableDock::removeColumn);

	connect(ui.lwDimensions, &QListWidget::itemSelectionChanged, this, [=]() {
		bool enabled = !ui.lwDimensions->selectedItems().isEmpty();
		ui.bAddRow->setEnabled(enabled);
		ui.bAddColumn->setEnabled(enabled);
	});
	connect(ui.lwRows, &QListWidget::itemSelectionChanged, this, [=]() {
		ui.bRemoveRow->setEnabled(!ui.lwRows->selectedItems().isEmpty());
	});
	connect(ui.lwColumns, &QListWidget::itemSelectionChanged, this, [=]() {
		ui.bRemoveColumn->setEnabled(!ui.lwColumns->selectedItems().isEmpty());
	});
}

void PivotTableDock::setPivotTable(PivotTable* pivotTable) {
	m_initializing = true;
	m_pivotTable = pivotTable;

	m_aspectTreeModel = new AspectTreeModel(m_pivotTable->project());

	QList<const char*> list;
	list << "Folder" << "Workbook" << "Spreadsheet" << "LiveDataSource";
	cbSpreadsheet->setTopLevelClasses(list);

	list.clear();
	list << "Spreadsheet" << "LiveDataSource";
	m_aspectTreeModel->setSelectableAspects(list);

	cbSpreadsheet->setModel(m_aspectTreeModel);

	//show the properties
	ui.leName->setText(m_pivotTable->name());
	ui.leComment->setText(m_pivotTable->comment());
	ui.cbDataSourceType->setCurrentIndex(m_pivotTable->dataSourceType());
	if (m_pivotTable->dataSourceType() == PivotTable::DataSourceSpreadsheet)
		setModelIndexFromAspect(cbSpreadsheet, m_pivotTable->dataSourceSpreadsheet());
	else
		ui.cbConnection->setCurrentIndex(ui.cbConnection->findText(m_pivotTable->dataSourceConnection()));

	this->dataSourceTypeChanged(ui.cbDataSourceType->currentIndex());

	//available dimensions and measures
	ui.lwDimensions->clear();
	for (auto dimension : m_pivotTable->dimensions())
		ui.lwDimensions->addItem(new QListWidgetItem(QIcon::fromTheme("draw-text"), dimension));

	for (auto measure : m_pivotTable->measures())
		ui.lwDimensions->addItem(new QListWidgetItem(measure));

	//undo functions
	connect(m_pivotTable, SIGNAL(aspectDescriptionChanged(const AbstractAspect*)), this, SLOT(pivotTableDescriptionChanged(const AbstractAspect*)));
	//TODO:

	m_initializing = false;
}

void PivotTableDock::setModelIndexFromAspect(TreeViewComboBox* cb, const AbstractAspect* aspect) {
	if (aspect)
		cb->setCurrentModelIndex(m_aspectTreeModel->modelIndexOfAspect(aspect));
	else
		cb->setCurrentModelIndex(QModelIndex());
}

/*!
	shows the database manager where the connections are created and edited.
	The selected connection is selected in the connection combo box in this widget.
**/
void PivotTableDock::showDatabaseManager() {
	DatabaseManagerDialog* dlg = new DatabaseManagerDialog(this, ui.cbConnection->currentText());

	if (dlg->exec() == QDialog::Accepted) {
		//re-read the available connections to be in sync with the changes in DatabaseManager
		m_initializing = true;
		ui.cbConnection->clear();
		readConnections();

		//select the connection the user has selected in DatabaseManager
		const QString& conn = dlg->connection();
		ui.cbConnection->setCurrentIndex(ui.cbConnection->findText(conn));
		m_initializing = false;

		connectionChanged();
	}

	delete dlg;
}

/*!
	loads all available saved connections
*/
void PivotTableDock::readConnections() {
	DEBUG("ImportSQLDatabaseWidget: reading available connections");
	KConfig config(m_configPath, KConfig::SimpleConfig);
	for (const auto& name : config.groupList())
		ui.cbConnection->addItem(name);
}

void PivotTableDock::addRow() {
	QString name = ui.lwDimensions->currentItem()->text();
	ui.lwRows->addItem(name);
	ui.lwDimensions->takeItem(ui.lwDimensions->currentRow());
	m_pivotTable->addToRows(name);
}

void PivotTableDock::removeRow() {
	ui.lwRows->takeItem(ui.lwRows->currentRow());
}

void PivotTableDock::addColumn() {
	QString name = ui.lwDimensions->currentItem()->text();
	ui.lwColumns->addItem(name);
	ui.lwDimensions->takeItem(ui.lwDimensions->currentRow());
	m_pivotTable->addToColumns(name);
}

void PivotTableDock::removeColumn() {
	ui.lwColumns->takeItem(ui.lwColumns->currentRow());
}

//*************************************************************
//****** SLOTs for changes triggered in PivotTableDock *******
//*************************************************************
void PivotTableDock::nameChanged() {
	if (m_initializing)
		return;

	m_pivotTable->setName(ui.leName->text());
}

void PivotTableDock::commentChanged() {
	if (m_initializing)
		return;

	m_pivotTable->setComment(ui.leComment->text());
}

void PivotTableDock::dataSourceTypeChanged(int index) {
	PivotTable::DataSourceType type = (PivotTable::DataSourceType)index;
	bool showDatabase = (type == PivotTable::DataSourceDatabase);
	ui.lSpreadsheet->setVisible(!showDatabase);
	cbSpreadsheet->setVisible(!showDatabase);
	ui.lConnection->setVisible(showDatabase);
	ui.cbConnection->setVisible(showDatabase);
	ui.bDatabaseManager->setVisible(showDatabase);
	ui.lTable->setVisible(showDatabase);
	ui.cbTable->setVisible(showDatabase);

	if (m_initializing)
		return;

	m_pivotTable->setComment(ui.leComment->text());
}

void PivotTableDock::spreadsheetChanged(const QModelIndex& index) {
	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	Spreadsheet* spreadsheet = dynamic_cast<Spreadsheet*>(aspect);

	//clear the previos definiot of the data fields
	ui.lwDimensions->clear();
	//TODO:
	//rows, columns, values

	//show all spreadsheet columns as available dimensions
	for (const auto* col : spreadsheet->children<Column>()) {
		QListWidgetItem* item = new QListWidgetItem(col->icon(), col->name());
		ui.lwDimensions->addItem(item);
	}

	if (m_initializing)
		return;

	m_pivotTable->setDataSourceSpreadsheet(spreadsheet);
}


void PivotTableDock::connectionChanged() {
	if (ui.cbConnection->currentIndex() == -1) {
		ui.lTable->hide();
		ui.cbTable->hide();
		return;
	}

	//clear the previously shown tables
	ui.cbTable->clear();
	ui.lTable->show();
	ui.cbTable->show();

	const QString& connection = ui.cbConnection->currentText();

	//connection name was changed, determine the current connections settings
	KConfig config(m_configPath, KConfig::SimpleConfig);
	KConfigGroup group = config.group(connection);

	//close and remove the previos connection, if available
	if (m_db.isOpen()) {
		m_db.close();
		QSqlDatabase::removeDatabase(m_db.driverName());
	}

	//open the selected connection
	QDEBUG("PivotTableDock: connecting to " + connection);
	const QString& driver = group.readEntry("Driver");
	m_db = QSqlDatabase::addDatabase(driver);

	const QString& dbName = group.readEntry("DatabaseName");
	if (DatabaseManagerWidget::isFileDB(driver)) {
		if (!QFile::exists(dbName)) {
			KMessageBox::error(this, i18n("Couldn't find the database file '%1'. Please check the connection settings.", dbName),
									i18n("Connection Failed"));
			return;
		} else
			m_db.setDatabaseName(dbName);
	} else if (DatabaseManagerWidget::isODBC(driver)) {
		if (group.readEntry("CustomConnectionEnabled", false))
			m_db.setDatabaseName(group.readEntry("CustomConnectionString"));
		else
			m_db.setDatabaseName(dbName);
	} else {
		m_db.setDatabaseName(dbName);
		m_db.setHostName( group.readEntry("HostName") );
		m_db.setPort( group.readEntry("Port", 0) );
		m_db.setUserName( group.readEntry("UserName") );
		m_db.setPassword( group.readEntry("Password") );
	}

	WAIT_CURSOR;
	if (!m_db.open()) {
		RESET_CURSOR;
		KMessageBox::error(this, i18n("Failed to connect to the database '%1'. Please check the connection settings.", ui.cbConnection->currentText()) +
									QLatin1String("\n\n") + m_db.lastError().databaseText(),
								 i18n("Connection Failed"));
		return;
	}

	//show all available database tables
	if (m_db.tables().size()) {
		for (auto table : m_db.tables())
			ui.cbTable->addItem(QIcon::fromTheme("view-form-table"), table);
		ui.cbTable->setCurrentIndex(0);
	}

	RESET_CURSOR;

	if (m_initializing)
		return;

// 	m_pivotTable->setDataSourceConnection(connection);
}

void PivotTableDock::tableChanged() {
	const QString& table = ui.cbTable->currentText();

	//show all attributes of the selected table
// 	for (const auto* col : spreadsheet->children<Column>()) {
// 		QListWidgetItem* item = new QListWidgetItem(col->icon(), col->name());
// 		ui.lwDimensions->addItem(item);
// 	}

	if (m_initializing)
		return;

// 	m_pivotTable->setDataSourceTable(table);
}

//*************************************************************
//******** SLOTs for changes triggered in Matrix *********
//*************************************************************
void PivotTableDock::pivotTableDescriptionChanged(const AbstractAspect* aspect) {
	if (m_pivotTable != aspect)
		return;

	m_initializing = true;
	if (aspect->name() != ui.leName->text())
		ui.leName->setText(aspect->name());
	else if (aspect->comment() != ui.leComment->text())
		ui.leComment->setText(aspect->comment());

	m_initializing = false;
}

//*************************************************************
//******************** SETTINGS *******************************
//*************************************************************
void PivotTableDock::load() {

}

void PivotTableDock::loadConfigFromTemplate(KConfig& config) {
	Q_UNUSED(config);
}

/*!
	loads saved matrix properties from \c config.
 */
void PivotTableDock::loadConfig(KConfig& config) {
	Q_UNUSED(config);
}

/*!
	saves matrix properties to \c config.
 */
void PivotTableDock::saveConfigAsTemplate(KConfig& config) {
	Q_UNUSED(config);
}
