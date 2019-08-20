/***************************************************************************
    File                 : CorrelationCoefficientDock.cpp
    Project              : LabPlot
    Description          : widget for correlation test properties
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

#include "CorrelationCoefficientDock.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/core/AbstractAspect.h"
#include "backend/core/Project.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "commonfrontend/widgets/TreeViewComboBox.h"
#include "kdefrontend/datasources/DatabaseManagerDialog.h"
#include "kdefrontend/datasources/DatabaseManagerWidget.h"
#include "kdefrontend/TemplateHandler.h"

#include <QDir>
#include <QSqlError>

#include <KConfig>
#include <KConfigGroup>
#include <KMessageBox>
#include <KLocalizedString>

#include <QStandardItemModel>
#include <QAbstractItemModel>
/*!
  \class CorrelationCoefficientDock
  \brief Provides a dock (widget) for correlation testing:
  \ingroup kdefrontend
*/

//TODO: To add tooltips in docks for non obvious widgets.
//TODO: Add functionality for database along with spreadsheet.

CorrelationCoefficientDock::CorrelationCoefficientDock(QWidget* parent) : QWidget(parent) {
	ui.setupUi(this);

	ui.cbDataSourceType->addItem(i18n("Spreadsheet"));
	ui.cbDataSourceType->addItem(i18n("Database"));

	cbSpreadsheet = new TreeViewComboBox;
	ui.gridLayout->addWidget(cbSpreadsheet, 5, 4, 1, 3);

	ui.bDatabaseManager->setIcon(QIcon::fromTheme("network-server-database"));
	ui.bDatabaseManager->setToolTip(i18n("Manage connections"));
	m_configPath = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).constFirst() +  "sql_connections";

	ui.cbTest->addItem( i18n("Pearson r"), CorrelationCoefficient::Pearson);
	ui.cbTest->addItem( i18n("Kendall"), CorrelationCoefficient::Kendall);
	ui.cbTest->addItem( i18n("Spearman"), CorrelationCoefficient::Spearman);
	ui.cbTest->addItem( i18n("Chi Square"), CorrelationCoefficient::ChiSquare);

	ui.leNRows->setText("2");
	ui.leNColumns->setText("2");

	ui.leNRows->setValidator(new QIntValidator(this));
	ui.leNColumns->setValidator(new QIntValidator(this));

	ui.lTestType->hide();
	ui.cbTestType->hide();
	ui.lCalculateStats->hide();
	ui.chbCalculateStats->hide();
	ui.lNRows->hide();
	ui.leNRows->hide();
	ui.lNColumns->hide();
	ui.leNColumns->hide();
	ui.lCategorical->hide();
	ui.chbCategorical->hide();
	ui.lCol1->hide();
	ui.cbCol1->hide();
	ui.lCol2->hide();
	ui.cbCol2->hide();
	ui.lAlpha->hide();
	ui.leAlpha->hide();
	ui.pbPerformTest->setEnabled(false);
	ui.pbPerformTest->setIcon(QIcon::fromTheme("run-build"));

	//    readConnections();
	connect(ui.cbDataSourceType, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
	        this, &CorrelationCoefficientDock::dataSourceTypeChanged);

	connect(cbSpreadsheet, &TreeViewComboBox::currentModelIndexChanged, this, &CorrelationCoefficientDock::spreadsheetChanged);
	//    connect(ui.cbConnection, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
	//            this, &CorrelationCoefficientDock::connectionChanged);
	//    connect(ui.cbTable, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
	//            this, &CorrelationCoefficientDock::tableChanged);
	//    connect(ui.bDatabaseManager, &QPushButton::clicked, this, &CorrelationCoefficientDock::showDatabaseManager);

	//    connect(ui.bAddRow,  &QPushButton::clicked, this, &CorrelationCoefficientDock::addRow);
	//    connect(ui.bRemoveRow, &QPushButton::clicked, this,&CorrelationCoefficientDock::removeRow);
	//    connect(ui.bAddColumn,  &QPushButton::clicked, this, &CorrelationCoefficientDock::addColumn);
	//    connect(ui.bRemoveColumn, &QPushButton::clicked, this,&CorrelationCoefficientDock::removeColumn);

	//      connect(ui.cbCol1, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &CorrelationCoefficientDock::doTTest);
	//      connect(ui.cbCol2, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &CorrelationCoefficientDock::doTTest);

	//    connect(ui.lwFields, &QListWidget::itemSelectionChanged, this, [=]() {
	//        bool enabled = !ui.lwFields->selectedItems().isEmpty();
	//        ui.bAddRow->setEnabled(enabled);
	//        ui.bAddColumn->setEnabled(enabled);
	//    });

	//    connect(ui.lwRows, &QListWidget::doubleClicked, this,&CorrelationCoefficientDock::removeRow);
	//    connect(ui.lwRows, &QListWidget::itemSelectionChanged, this, [=]() {
	//        ui.bRemoveRow->setEnabled(!ui.lwRows->selectedItems().isEmpty());
	//    });

	//    connect(ui.lwColumns, &QListWidget::doubleClicked, this,&CorrelationCoefficientDock::removeColumn);
	//    connect(ui.lwColumns, &QListWidget::itemSelectionChanged, this, [=]() {
	//        ui.bRemoveColumn->setEnabled(!ui.lwColumns->selectedItems().isEmpty());
	//    });

	connect(ui.cbTest, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &CorrelationCoefficientDock::showTestType);
	connect(ui.cbTestType, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &CorrelationCoefficientDock::showCorrelationCoefficient);
	connect(ui.chbCategorical, &QCheckBox::stateChanged, this, &CorrelationCoefficientDock::changeCbCol2Label);
	connect(ui.chbCalculateStats, &QCheckBox::stateChanged, this, &CorrelationCoefficientDock::chbColumnStatsStateChanged);
	connect(ui.leNRows, &QLineEdit::textChanged, this, &CorrelationCoefficientDock::leNRowsChanged);
	connect(ui.leNColumns, &QLineEdit::textChanged, this, &CorrelationCoefficientDock::leNColumnsChanged);

	connect(ui.pbPerformTest, &QPushButton::clicked, this, &CorrelationCoefficientDock::findCorrelationCoefficient);
	connect(ui.cbCol1, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &CorrelationCoefficientDock::col1IndexChanged);

	ui.cbTest->setCurrentIndex(0);
	emit ui.cbTest->currentIndexChanged(0);
	ui.cbTestType->setCurrentIndex(0);
	emit ui.cbTestType->currentIndexChanged(0);
}

void CorrelationCoefficientDock::setCorrelationCoefficient(CorrelationCoefficient* CorrelationCoefficient) {
	m_initializing = true;
	m_correlationCoefficient = CorrelationCoefficient;

	m_aspectTreeModel = new AspectTreeModel(m_correlationCoefficient->project());

	QList<AspectType> list{AspectType::Folder, AspectType::Workbook,
	                       AspectType::Spreadsheet, AspectType::LiveDataSource};
	cbSpreadsheet->setTopLevelClasses(list);

	list = {AspectType::Spreadsheet, AspectType::LiveDataSource};
	m_aspectTreeModel->setSelectableAspects(list);

	cbSpreadsheet->setModel(m_aspectTreeModel);

	//show the properties
	ui.leName->setText(m_correlationCoefficient->name());
	ui.leComment->setText(m_correlationCoefficient->comment());
	ui.cbDataSourceType->setCurrentIndex(m_correlationCoefficient->dataSourceType());
	if (m_correlationCoefficient->dataSourceType() == CorrelationCoefficient::DataSourceType::DataSourceSpreadsheet)
		setModelIndexFromAspect(cbSpreadsheet, m_correlationCoefficient->dataSourceSpreadsheet());
	//    else
	//        ui.cbConnection->setCurrentIndex(ui.cbConnection->findText(m_correlationCoefficient->dataSourceConnection()));

	setColumnsComboBoxModel(m_correlationCoefficient->dataSourceSpreadsheet());

	this->dataSourceTypeChanged(ui.cbDataSourceType->currentIndex());

	//setting rows and columns in combo box;

	//undo functions
//	connect(m_correlationCoefficient, SIGNAL(aspectDescriptionChanged(const AbstractAspect*)), this, SLOT(CorrelationCoefficientDescriptionChanged(const AbstractAspect*)));

	m_initializing = false;

}

void CorrelationCoefficientDock::showTestType() {
	if (ui.cbTest->count() == 0)
		return;

	m_test = ui.cbTest->currentData().toInt();

	ui.cbTestType->clear();
	switch (m_test) {
	case CorrelationCoefficient::ChiSquare:
		ui.lTestType->show();
		ui.cbTestType->show();
		ui.cbTestType->addItem( i18n("Test for Independence"), CorrelationCoefficient::IndependenceTest);
		break;
	case CorrelationCoefficient::Pearson:
	case CorrelationCoefficient::Spearman:
	case CorrelationCoefficient::Kendall:
	case CorrelationCoefficient::IndependenceTest:
		ui.lTestType->hide();
		ui.cbTestType->hide();
		showCorrelationCoefficient();
		break;
	}
}

void CorrelationCoefficientDock::showCorrelationCoefficient() {
	m_test = testType(m_test) | ui.cbTestType->currentData().toInt();

	ui.lCalculateStats->setVisible(testType(m_test) == CorrelationCoefficient::ChiSquare);
	ui.chbCalculateStats->setVisible(testType(m_test) == CorrelationCoefficient::ChiSquare);

	if (testType(m_test) != CorrelationCoefficient::ChiSquare)
		ui.chbCalculateStats->setChecked(true);

	chbColumnStatsStateChanged();
}

void CorrelationCoefficientDock::findCorrelationCoefficient()  {
	QVector<Column*> cols;

	if (ui.cbCol1->count() == 0)
		return;

	cols << reinterpret_cast<Column*>(ui.cbCol1->currentData().toLongLong());
	cols << reinterpret_cast<Column*>(ui.cbCol2->currentData().toLongLong());

	m_correlationCoefficient->setColumns(cols);
	m_correlationCoefficient->performTest(m_test, ui.chbCategorical->isChecked());
}

void CorrelationCoefficientDock::setModelIndexFromAspect(TreeViewComboBox* cb, const AbstractAspect* aspect) {
	if (aspect)
		cb->setCurrentModelIndex(m_aspectTreeModel->modelIndexOfAspect(aspect));
	else
		cb->setCurrentModelIndex(QModelIndex());
}


////*************************************************************
////****** SLOTs for changes triggered in CorrelationCoefficientDock *******
////*************************************************************
//void CorrelationCoefficientDock::nameChanged() {
//    if (m_initializing)
//        return;

//    m_correlationCoefficient->setName(ui.leName->text());
//}

//void CorrelationCoefficientDock::commentChanged() {
//    if (m_initializing)
//        return;

//    m_correlationCoefficient->setComment(ui.leComment->text());
//}

void CorrelationCoefficientDock::dataSourceTypeChanged(int index) {
	//QDEBUG("in dataSourceTypeChanged");
	CorrelationCoefficient::DataSourceType type = static_cast<CorrelationCoefficient::DataSourceType>(index);
	bool showDatabase = (type == CorrelationCoefficient::DataSourceType::DataSourceDatabase);
	ui.lSpreadsheet->setVisible(!showDatabase);
	cbSpreadsheet->setVisible(!showDatabase);
	ui.lConnection->setVisible(showDatabase);
	ui.cbConnection->setVisible(showDatabase);
	ui.bDatabaseManager->setVisible(showDatabase);
	ui.lTable->setVisible(showDatabase);
	ui.cbTable->setVisible(showDatabase);

	if (m_initializing)
		return;

	m_correlationCoefficient->setComment(ui.leComment->text());

}

void CorrelationCoefficientDock::spreadsheetChanged(const QModelIndex& index) {
	//QDEBUG("in spreadsheetChanged");
	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	Spreadsheet* spreadsheet = dynamic_cast<Spreadsheet*>(aspect);
	setColumnsComboBoxModel(spreadsheet);
	m_correlationCoefficient->setDataSourceSpreadsheet(spreadsheet);
}

void CorrelationCoefficientDock::col1IndexChanged(int index) {
	if (index < 0) return;
	changeCbCol2Label();
}


//void CorrelationCoefficientDock::connectionChanged() {
//    if (ui.cbConnection->currentIndex() == -1) {
//        ui.lTable->hide();
//        ui.cbTable->hide();
//        return;
//    }

//    //clear the previously shown tables
//    ui.cbTable->clear();
//    ui.lTable->show();
//    ui.cbTable->show();

//    const QString& connection = ui.cbConnection->currentText();

//    //connection name was changed, determine the current connections settings
//    KConfig config(m_configPath, KConfig::SimpleConfig);
//    KConfigGroup group = config.group(connection);

//    //close and remove the previos connection, if available
//    if (m_db.isOpen()) {
//        m_db.close();
//        QSqlDatabase::removeDatabase(m_db.driverName());
//    }

//    //open the selected connection
//    //QDEBUG("CorrelationCoefficientDock: connecting to " + connection);
//    const QString& driver = group.readEntry("Driver");
//    m_db = QSqlDatabase::addDatabase(driver);

//    const QString& dbName = group.readEntry("DatabaseName");
//    if (DatabaseManagerWidget::isFileDB(driver)) {
//        if (!QFile::exists(dbName)) {
//            KMessageBox::error(this, i18n("Couldn't find the database file '%1'. Please check the connection settings.", dbName),
//                               appendRow     i18n("Connection Failed"));
//            return;
//        } else
//            m_db.setDatabaseName(dbName);
//    } else if (DatabaseManagerWidget::isODBC(driver)) {
//        if (group.readEntry("CustomConnectionEnabled", false))
//            m_db.setDatabaseName(group.readEntry("CustomConnectionString"));
//        else
//            m_db.setDatabaseName(dbName);
//    } else {
//        m_db.setDatabaseName(dbName);
//        m_db.setHostName( group.readEntry("HostName") );
//        m_db.setPort( group.readEntry("Port", 0) );
//        m_db.setUserName( group.readEntry("UserName") );
//        m_db.setPassword( group.readEntry("Password") );
//    }

//    WAIT_CURSOR;
//    if (!m_db.open()) {
//        RESET_CURSOR;
//        KMessageBox::error(this, i18n("Failed to connect to the database '%1'. Please check the connection settings.", ui.cbConnection->currentText()) +
//                                    QLatin1String("\n\n") + m_db.lastError().databaseText(),
//                                 i18n("Connection Failed"));
//        return;
//    }

//    //show all available database tables
//    if (m_db.tables().size()) {
//        for (auto table : m_db.tables())
//            ui.cbTable->addItem(QIcon::fromTheme("view-form-table"), table);
//        ui.cbTable->setCurrentIndex(0);
//    }

//    RESET_CURSOR;

//    if (m_initializing)
//        return;

//// 	m_correlationCoefficient->setDataSourceConnection(connection);
//}

//void CorrelationCoefficientDock::tableChanged() {
//    const QString& table = ui.cbTable->currentText();

//    //show all attributes of the selected table
//// 	for (const auto* col : spreadsheet->children<Column>()) {
//// 		QListWidgetItem* item = new QListWidgetItem(col->icon(), col->name());
//// 		ui.lwFields->addItem(item);
//// 	}

//    if (m_initializing)
//        return;

//// 	m_correlationCoefficient->setDataSourceTable(table);
//}

////*************************************************************
////******** SLOTs for changes triggered in Spreadsheet *********
////*************************************************************
void CorrelationCoefficientDock::CorrelationCoefficientDescriptionChanged(const AbstractAspect* aspect) {
	if (m_correlationCoefficient != aspect)
		return;

	m_initializing = true;
	if (aspect->name() != ui.leName->text())
		ui.leName->setText(aspect->name());
	else if (aspect->comment() != ui.leComment->text())
		ui.leComment->setText(aspect->comment());

	m_initializing = false;
}


void CorrelationCoefficientDock::changeCbCol2Label() {
	if (ui.cbCol1->count() == 0) return;

	QString selected_text = ui.cbCol1->currentText();
	Column* col1 = m_correlationCoefficient->dataSourceSpreadsheet()->column(selected_text);

	if (m_test == (CorrelationCoefficient::Kendall | CorrelationCoefficient::Spearman) ||
			(!ui.chbCategorical->isChecked() && col1->isNumeric())) {
		ui.lCol2->setText( i18n("Independent Var. 2"));
		ui.chbCategorical->setChecked(false);
		ui.chbCategorical->setEnabled(true);
	} else {
		ui.lCol2->setText( i18n("Dependent Var. 1"));
		if (!ui.chbCategorical->isChecked())
			ui.chbCategorical->setEnabled(false);
		else
			ui.chbCategorical->setEnabled(true);
		ui.chbCategorical->setChecked(true);
	}
}

void CorrelationCoefficientDock::chbColumnStatsStateChanged() {
	bool chbChecked = ui.chbCalculateStats->isChecked();

	ui.lVariables->setVisible(chbChecked);
	ui.lCol1->setVisible(chbChecked);
	ui.cbCol1->setVisible(chbChecked);
	ui.lCol2->setVisible(chbChecked);
	ui.cbCol2->setVisible(chbChecked);

	ui.lCategorical->setVisible(chbChecked && testType(m_test) == CorrelationCoefficient::Pearson);
	ui.chbCategorical->setVisible(chbChecked && testType(m_test) == CorrelationCoefficient::Pearson);

	ui.lNRows->setVisible(!chbChecked);
	ui.leNRows->setVisible(!chbChecked);

	ui.lNColumns->setVisible(!chbChecked);
	ui.leNColumns->setVisible(!chbChecked);

	if (chbChecked) {
		setColumnsComboBoxView();
		ui.pbPerformTest->setEnabled(nonEmptySelectedColumns());
	} else
		ui.pbPerformTest->setEnabled(true);

	if (m_correlationCoefficient != nullptr)
		m_correlationCoefficient->initInputStatsTable(m_test, chbChecked, ui.leNRows->text().toInt(), ui.leNColumns->text().toInt());
}

void CorrelationCoefficientDock::leNRowsChanged() {
	if (m_correlationCoefficient != nullptr)
		m_correlationCoefficient->setInputStatsTableNRows(ui.leNRows->text().toInt());
}

void CorrelationCoefficientDock::leNColumnsChanged() {
	if (m_correlationCoefficient != nullptr)
		m_correlationCoefficient->setInputStatsTableNCols(ui.leNColumns->text().toInt());
}

////*************************************************************
////******************** SETTINGS *******************************
////*************************************************************
//void CorrelationCoefficientDock::load() {

//}

//void CorrelationCoefficientDock::loadConfigFromTemplate(KConfig& config) {
//    Q_UNUSED(config);
//}

///*!
//    loads saved matrix properties from \c config.
// */
//void CorrelationCoefficientDock::loadConfig(KConfig& config) {
//    Q_UNUSED(config);
//}

///*!
//    saves matrix properties to \c config.
// */
//void CorrelationCoefficientDock::saveConfigAsTemplate(KConfig& config) {
//    Q_UNUSED(config);
//}

void CorrelationCoefficientDock::setColumnsComboBoxModel(Spreadsheet* spreadsheet) {
	m_onlyValuesCols.clear();
	m_twoCategoricalCols.clear();
	m_multiCategoricalCols.clear();

	for (auto* col : spreadsheet->children<Column>()) {
		if (col->isNumeric())
			m_onlyValuesCols.append(col);
		else {
			int np = 0, n_rows = 0;
			countPartitions(col, np, n_rows);
			if (np <= 1)
				continue;
			else if (np == 2)
				m_twoCategoricalCols.append(col);
			else
				m_multiCategoricalCols.append(col);
		}
	}
	setColumnsComboBoxView();
	showCorrelationCoefficient();
}


//TODO: change from if else to switch case:
void CorrelationCoefficientDock::setColumnsComboBoxView() {
	ui.cbCol1->clear();
	ui.cbCol2->clear();

	QList<Column*>::iterator i;

	switch (testType(m_test)) {
	case (CorrelationCoefficient::Pearson): {
		for (i = m_onlyValuesCols.begin(); i != m_onlyValuesCols.end(); i++) {
			ui.cbCol1->addItem( (*i)->name(), qint64(*i));
			ui.cbCol2->addItem( (*i)->name(), qint64(*i));
		}
		for (i = m_twoCategoricalCols.begin(); i != m_twoCategoricalCols.end(); i++)
			ui.cbCol1->addItem( (*i)->name(), qint64(*i));
		break;
	}
	case CorrelationCoefficient::Kendall: {
		for (i = m_onlyValuesCols.begin(); i != m_onlyValuesCols.end(); i++) {
			ui.cbCol1->addItem( (*i)->name(), qint64(*i));
			ui.cbCol2->addItem( (*i)->name(), qint64(*i));
		}
		for (i = m_twoCategoricalCols.begin(); i != m_twoCategoricalCols.end(); i++) {
			ui.cbCol1->addItem( (*i)->name(), qint64(*i));
			ui.cbCol2->addItem( (*i)->name(), qint64(*i));
		}
		for (i = m_multiCategoricalCols.begin(); i != m_multiCategoricalCols.end(); i++) {
			ui.cbCol1->addItem( (*i)->name(), qint64(*i));
			ui.cbCol2->addItem( (*i)->name(), qint64(*i));
		}
		break;
	}
	case CorrelationCoefficient::Spearman: {
		for (i = m_onlyValuesCols.begin(); i != m_onlyValuesCols.end(); i++) {
			ui.cbCol1->addItem( (*i)->name(), qint64(*i));
			ui.cbCol2->addItem( (*i)->name(), qint64(*i));
		}
		for (i = m_twoCategoricalCols.begin(); i != m_twoCategoricalCols.end(); i++)
			ui.cbCol1->addItem( (*i)->name(), qint64(*i));
		break;
	}
	case CorrelationCoefficient::ChiSquare: {
		switch (testSubType(m_test)) {
		case CorrelationCoefficient::IndependenceTest:
			for (i = m_twoCategoricalCols.begin(); i != m_twoCategoricalCols.end(); i++) {
				ui.cbCol1->addItem( (*i)->name(), qint64(*i));
				ui.cbCol2->addItem( (*i)->name(), qint64(*i));
			}
			for (i = m_multiCategoricalCols.begin(); i != m_multiCategoricalCols.end(); i++) {
				ui.cbCol1->addItem( (*i)->name(), qint64(*i));
				ui.cbCol2->addItem( (*i)->name(), qint64(*i));
			}
			break;
		}
		break;
	}
	}
}

bool CorrelationCoefficientDock::nonEmptySelectedColumns() {
	if ((ui.cbCol1->isVisible() && ui.cbCol1->count() < 1) ||
			(ui.cbCol2->isVisible() && ui.cbCol2->count() < 1))
		return false;
	return true;
}

int CorrelationCoefficientDock::testType(int test) {
	return test & 0x0F;
}

int CorrelationCoefficientDock::testSubType(int test) {
	return test & 0xF0;
}

void CorrelationCoefficientDock::countPartitions(Column *column, int &np, int &total_rows) {
	total_rows = column->rowCount();
	np = 0;
	QString cell_value;
	QMap<QString, bool> discovered_categorical_var;

	AbstractColumn::ColumnMode original_col_mode = column->columnMode();
	column->setColumnMode(AbstractColumn::Text);

	for (int i = 0; i < total_rows; i++) {
		cell_value = column->textAt(i);

		if (cell_value.isEmpty()) {
			total_rows = i;
			break;
		}

		if (discovered_categorical_var[cell_value])
			continue;

		discovered_categorical_var[cell_value] = true;
		np++;
	}
	column->setColumnMode(original_col_mode);
}

