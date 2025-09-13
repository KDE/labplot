/***************************************************************************
	File                 : PivotTableDock.cpp
	Project              : LabPlot
	Description          : widget for pivot table properties
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2019-2025 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "PivotTableDock.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/core/Project.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "frontend/widgets/TreeViewComboBox.h"
#include "frontend/datasources/DatabaseManagerDialog.h"
#include "frontend/datasources/DatabaseManagerWidget.h"
#include "frontend/pivot/PivotTableView.h"

#include <QFile>
#include <QSqlError>

#include <KConfig>
#include <KConfigGroup>
#include <KMessageBox>

 /*!
  \class PivotTableDock
  \brief Provides a widget for editing the properties of the pivot table currently selected in the project explorer.

  \ingroup kdefrontend
*/
PivotTableDock::PivotTableDock(QWidget* parent) : BaseDock(parent) {
	ui.setupUi(this);
	setBaseWidgets(ui.leName, ui.teComment);

	cbSpreadsheet = new TreeViewComboBox;
	ui.gridLayout_6->addWidget(cbSpreadsheet, 5, 2, 1, 1);

	QList<AspectType> list{AspectType::Folder,
						   AspectType::Workbook,
						   AspectType::Spreadsheet,
						   AspectType::LiveDataSource};
	cbSpreadsheet->setTopLevelClasses(list);

	m_configPath = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).constFirst() +  QLatin1String("sql_connections");
	readConnections();

	const auto* style = ui.bAddRow->style();
	ui.bAddRow->setIcon(style->standardIcon(QStyle::SP_ArrowRight));
	ui.bRemoveRow->setIcon(style->standardIcon(QStyle::SP_ArrowLeft));
	ui.bAddColumn->setIcon(style->standardIcon(QStyle::SP_ArrowRight));
	ui.bRemoveColumn->setIcon(style->standardIcon(QStyle::SP_ArrowLeft));
	ui.bDatabaseManager->setIcon(QIcon::fromTheme(QLatin1String("network-server-database")));
	ui.bAddValue->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
	ui.bAddFilter->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));

	//add/remove buttons only enabled if something was selected
	ui.bAddRow->setEnabled(false);
	ui.bRemoveRow->setEnabled(false);
	ui.bAddColumn->setEnabled(false);
	ui.bRemoveColumn->setEnabled(false);

	retranslateUi();

	//**********************************  Slots **********************************************
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
	connect(ui.bAddValue,  &QPushButton::clicked,  this, [=]() {
		addValue();
	});

	connect(ui.lwFields, &QListWidget::itemSelectionChanged, this, [=]() {
		const bool enabled = !ui.lwFields->selectedItems().isEmpty();
		ui.bAddRow->setEnabled(enabled);
		ui.bAddColumn->setEnabled(enabled);
	});

	connect(ui.lwRows, &QListWidget::doubleClicked, this,&PivotTableDock::removeRow);
	connect(ui.lwRows, &QListWidget::itemSelectionChanged, this, [=]() {
		ui.bRemoveRow->setEnabled(!ui.lwRows->selectedItems().isEmpty());
	});

	connect(ui.lwColumns, &QListWidget::doubleClicked, this,&PivotTableDock::removeColumn);
	connect(ui.lwColumns, &QListWidget::itemSelectionChanged, this, [=]() {
		ui.bRemoveColumn->setEnabled(!ui.lwColumns->selectedItems().isEmpty());
	});
}

void PivotTableDock::setPivotTable(PivotTable* pivotTable) {
	CONDITIONAL_LOCK_RETURN;
	m_pivotTable = pivotTable;
	setAspects(QList{pivotTable});

	auto* model = aspectModel();
	model->setSelectableAspects({AspectType::Spreadsheet, AspectType::LiveDataSource});
	cbSpreadsheet->setModel(model);

	// show the properties
	load();

	//undo functions
	//TODO:
}

void PivotTableDock::retranslateUi() {
	ui.cbDataSourceType->clear();
	ui.cbDataSourceType->addItem(i18n("Spreadsheet"));
	ui.cbDataSourceType->addItem(i18n("Database"));

	ui.bDatabaseManager->setToolTip(i18n("Manage connections"));
	ui.bAddRow->setToolTip(i18n("Add the selected field to rows"));
	ui.bRemoveRow->setToolTip(i18n("Remove the selected field from rows"));
	ui.bAddColumn->setToolTip(i18n("Add the selected field to columns"));
	ui.bRemoveColumn->setToolTip(i18n("Remove the selected field from columns"));
	ui.bAddValue->setToolTip(i18n("Add new value"));
}

/*!
	shows the database manager where the connections are created and edited.
	The selected connection is selected in the connection combo box in this widget.
**/
void PivotTableDock::showDatabaseManager() {
	auto* dlg = new DatabaseManagerDialog(this, ui.cbConnection->currentText());

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

/*!
 * adds the selected field to the rows
 */
void PivotTableDock::addRow() {
	QString field = ui.lwFields->currentItem()->text();
	ui.lwRows->addItem(field);
	ui.lwFields->takeItem(ui.lwFields->currentRow());
	m_pivotTable->addToRows(field);
}

/*!
 * removes the selected field from the rows
 */
void PivotTableDock::removeRow() {
	const QString& field = ui.lwRows->currentItem()->text();
	ui.lwRows->takeItem(ui.lwRows->currentRow());
	m_pivotTable->removeFromRows(field);
	loadFields();
}

/*!
 * adds the selected field to the columns
 */
void PivotTableDock::addColumn() {
	QString field = ui.lwFields->currentItem()->text();
	ui.lwColumns->addItem(field);
	ui.lwFields->takeItem(ui.lwFields->currentRow());
	m_pivotTable->addToColumns(field);
}

/*!
 * removes the selected field from the columns
 */
void PivotTableDock::removeColumn() {
	const QString& field = ui.lwColumns->currentItem()->text();
	ui.lwColumns->takeItem(ui.lwColumns->currentRow());
	m_pivotTable->removeFromColumns(field);
	loadFields();
}

/*!
 * creates the widgets for a new value and adds them to the layout
  \param name Name of the column to be used as value
  \param aggregation Aggregation method to be used for this value
 */
void PivotTableDock::addValue(const QString& name, PivotTable::Aggregation aggregation) {
	// value name
	auto* cbValueName = new QComboBox(this);
	m_valueNameComboBoxes << cbValueName;
	cbValueName->addItem(QString());
	for (auto measure : m_pivotTable->measures())
		cbValueName->addItem(measure);
	cbValueName->setCurrentText(name);
	connect(cbValueName, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			this, &PivotTableDock::valueNameChanged);

	// aggregation
	auto* cbValueAggregation = new QComboBox(this);
	m_valueAggregationComboBoxes << cbValueAggregation;

	cbValueAggregation->addItem(i18n("Count"), static_cast<int>(PivotTable::Aggregation::Count));
	cbValueAggregation->addItem(i18n("Sum"), static_cast<int>(PivotTable::Aggregation::Sum));
	cbValueAggregation->addItem(i18n("Minimum"), static_cast<int>(PivotTable::Aggregation::Min));
	cbValueAggregation->addItem(i18n("Maximum"), static_cast<int>(PivotTable::Aggregation::Max));
	cbValueAggregation->addItem(i18n("Average"), static_cast<int>(PivotTable::Aggregation::Avg));
	cbValueAggregation->setCurrentIndex(cbValueAggregation->findData(static_cast<int>(aggregation)));
	connect(cbValueAggregation, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			this, &PivotTableDock::valueAggregationChanged);

	// add to layout
	const int index = m_valueNameComboBoxes.count() - 1;
	auto* gridLayout = static_cast<QGridLayout*>(ui.frameValues->layout());
	gridLayout->addWidget(cbValueName, index, 0, 1, 1);
	gridLayout->addWidget(cbValueAggregation, index, 1, 1, 1);

	// remove button (not for the first value)
	if (index != 0) {
		auto* bRemoveValue = new QPushButton(this);
		bRemoveValue->setIcon(QIcon::fromTheme(QStringLiteral("list-remove")));
		connect(bRemoveValue, &QPushButton::clicked, this, &PivotTableDock::removeValue);
		m_removeValueButtons << bRemoveValue;
		gridLayout->addWidget(bRemoveValue, index, 2, 1, 1);
	}

	// move the "Add Value" button one row down
	gridLayout->addWidget(ui.bAddValue, index + 1, 2, 1, 1);

	CONDITIONAL_LOCK_RETURN;

	// add new value to the pivot table
	auto values = m_pivotTable->values();
	PivotTable::Value value;
	value.name = name;
	value.aggregation = aggregation;
	values.append(value);
	m_pivotTable->setValues(values);
}

/*!
 * removes the selected field from the values
 */
void PivotTableDock::removeValue() {
	// remove widgets
	auto* b = static_cast<QPushButton*>(sender());
	const int index = m_removeValueButtons.indexOf(b);
	delete m_valueNameComboBoxes.takeAt(index + 1); // +1 because the first value has no remove button
	delete m_valueAggregationComboBoxes.takeAt(index + 1);
	delete m_removeValueButtons.takeAt(index);

	// remove the value from the pivot table
	auto values = m_pivotTable->values();
	if (index + 1 < values.size()) // +1 because the first value has no remove button
		values.removeAt(index + 1);
	m_pivotTable->setValues(values);
}

/*!
* called when the user changes the name of a value
*/
void PivotTableDock::valueNameChanged(int) {
	CONDITIONAL_LOCK_RETURN;

	auto* cb = static_cast<QComboBox*>(sender());
	const int i = m_valueNameComboBoxes.indexOf(cb);
	if (i == -1)
		return;

	auto values = m_pivotTable->values();
	Q_ASSERT(i < values.size());
	auto value = values.at(i);
	value.name = cb->currentText();
	values[i] = value;
	m_pivotTable->setValues(values);
}

/*!
* called when the user changes the aggregation method of a value
*/
void PivotTableDock::valueAggregationChanged(int) {
	CONDITIONAL_LOCK_RETURN;

	auto* cb = static_cast<QComboBox*>(sender());
	const int i = m_valueAggregationComboBoxes.indexOf(cb);
	if (i == -1)
		return;

	const auto aggregation = static_cast<PivotTable::Aggregation>(cb->currentData().toInt());
	auto values = m_pivotTable->values();
	Q_ASSERT(i < values.size());
	auto value = values.at(i);
	value.aggregation = aggregation;
	values[i] = value;
	m_pivotTable->setValues(values);
}

/*!
 * re-populates the content of the "Fields" list widget by adding the non-selected fields only.
 * called when a selected field is removed from rows or columns.
 */
void PivotTableDock::loadFields() {
	ui.lwFields->clear();
	for (auto dimension : m_pivotTable->dimensions())
		if (!fieldSelected(dimension))
			ui.lwFields->addItem(new QListWidgetItem(QIcon::fromTheme(QLatin1String("draw-text")), dimension));
}

void PivotTableDock::loadValues() {
	const auto& values = m_pivotTable->values();
	for (const auto& value : values)
		addValue(value.name, value.aggregation);
}

/*!
 * return \c true if the field name \c field was selected among rows or columns.
 * return \c false otherwise.
 * */
bool PivotTableDock::fieldSelected(const QString& field) {
	const bool selected = m_pivotTable->rows().indexOf(field) != -1 || m_pivotTable->columns().indexOf(field) != -1;
	return selected;
}

//*************************************************************
//****** SLOTs for changes triggered in PivotTableDock *******
//*************************************************************
void PivotTableDock::dataSourceTypeChanged(int index) {
	auto type = (PivotTable::DataSourceType)index;
	const bool showDatabase = (type == PivotTable::DataSourceType::Database);
	ui.lSpreadsheet->setVisible(!showDatabase);
	cbSpreadsheet->setVisible(!showDatabase);
	ui.lConnection->setVisible(showDatabase);
	ui.cbConnection->setVisible(showDatabase);
	ui.bDatabaseManager->setVisible(showDatabase);
	ui.lTable->setVisible(showDatabase);
	ui.cbTable->setVisible(showDatabase);

	if (m_initializing)
		return;

	// TODO:
}

void PivotTableDock::spreadsheetChanged(const QModelIndex& index) {
	// clear the previous content shown in the list widgets
	//ui.lwFields->clear();
	ui.lwColumns->clear();
	ui.lwRows->clear();

	// show all spreadsheet columns as available dimensions
	if (m_initializing)
		return;

	const auto* spreadsheet = static_cast<Spreadsheet*>(index.internalPointer());
	m_pivotTable->setDataSourceSpreadsheet(spreadsheet);
	loadFields();
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
	// QDEBUG("PivotTableDock: connecting to " + connection);
	const QString& driver = group.readEntry(QLatin1String("Driver"));
	m_db = QSqlDatabase::addDatabase(driver);

	const QString& dbName = group.readEntry(QLatin1String("DatabaseName"));
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
			ui.cbTable->addItem(QIcon::fromTheme(QLatin1String("view-form-table")), table);
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
// 		ui.lwFields->addItem(item);
// 	}

	if (m_initializing)
		return;

// 	m_pivotTable->setDataSourceTable(table);
}

//*************************************************************
//******** SLOTs for changes triggered in PivotTable **********
//*************************************************************


//*************************************************************
//******************** SETTINGS *******************************
//*************************************************************
void PivotTableDock::load() {
	ui.cbDataSourceType->setCurrentIndex(static_cast<int>(m_pivotTable->dataSourceType()));
	cbSpreadsheet->setAspect(m_pivotTable->dataSourceSpreadsheet());
	ui.cbConnection->setCurrentIndex(ui.cbConnection->findText(m_pivotTable->dataSourceConnection()));
	dataSourceTypeChanged(ui.cbDataSourceType->currentIndex());

	ui.lwRows->clear();
	for (const auto& row : m_pivotTable->rows())
		ui.lwRows->addItem(new QListWidgetItem(row));

	ui.lwColumns->clear();
	for (const auto& column : m_pivotTable->columns())
		ui.lwColumns->addItem(new QListWidgetItem(column));

	// fields, values
	loadFields();
	loadValues();
}
