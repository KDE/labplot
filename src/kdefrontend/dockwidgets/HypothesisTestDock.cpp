/***************************************************************************
    File                 : HypothesisTestDock.cpp
    Project              : LabPlot
    Description          : widget for hypothesis test properties
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

#include "HypothesisTestDock.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/core/AbstractAspect.h"
#include "backend/core/Project.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "commonfrontend/widgets/TreeViewComboBox.h"
#include "kdefrontend/datasources/DatabaseManagerDialog.h"
#include "kdefrontend/datasources/DatabaseManagerWidget.h"
//#include "kdefrontend/pivot/hypothesisTestView.h"
#include "kdefrontend/TemplateHandler.h"

#include <QDir>
#include <QSqlError>

#include <KConfig>
#include <KConfigGroup>
#include <KMessageBox>
#include <QDebug>
 /*!
  \class HypothesisTestDock
  \brief Provides a dock (widget) for hypothesis testing:
  \ingroup kdefrontend
*/

HypothesisTestDock::HypothesisTestDock(QWidget* parent) : QWidget(parent) {
    ui.setupUi(this);

    ui.cbDataSourceType->addItem(i18n("Spreadsheet"));
    ui.cbDataSourceType->addItem(i18n("Database"));

    cbSpreadsheet = new TreeViewComboBox;
    ui.gridLayout->addWidget(cbSpreadsheet, 1, 1);

    ui.bDatabaseManager->setIcon(QIcon::fromTheme("network-server-database"));
    ui.bDatabaseManager->setToolTip(i18n("Manage connections"));
    m_configPath = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).constFirst() +  "sql_connections";




    // adding item to tests and testtype combo box;

    ui.cbTest->addItem(i18n("T Test"));
    ui.cbTest->addItem(i18n("Z Test"));

    ui.cbTestType->addItem(i18n("Two Sample Independent"));
    ui.cbTestType->addItem(i18n("Two Sample Paired"));
    ui.cbTestType->addItem(i18n("One Sample"));

    // making all test blocks invisible at starting.
    ui.lCol1->setVisible(false);
    ui.cbCol1->setVisible(false);
    ui.lCol2->setVisible(false);
    ui.cbCol2->setVisible(false);
    ui.chbEqualVariance->setVisible(false);
    ui.chbEqualVariance->setChecked(true);
    ui.pbPerformTest->setEnabled(false);
    ui.rb_h1_one_tail_2->setVisible(false);
    ui.rb_h1_one_tail_1->setVisible(false);
    ui.rb_h1_two_tail->setVisible(false);
    ui.rb_h0_one_tail_1->setVisible(false);
    ui.rb_h0_one_tail_2->setVisible(false);
    ui.rb_h0_two_tail->setVisible(false);
    ui.l_h0->setVisible(false);
    ui.l_h1->setVisible(false);

    QString mu = QChar(0x3BC);
    QString mu0 = mu+QChar(0x2092);

    // radio button for null and alternate hypothesis
    // for alternative hypothesis
    // one_tail_1 is mu > mu0; one_tail_2 is mu < mu0; two_tail = mu != mu0;


    ui.rb_h1_one_tail_1->setText( i18n("%1 %2 %3", mu, QChar(0x3E), mu0));
    ui.rb_h1_one_tail_2->setText( i18n("%1 %2 %3", mu, QChar(0x3C), mu0));
    ui.rb_h1_two_tail->setText( i18n("%1 %2 %3", mu, QChar(0x2260), mu0));

    ui.rb_h0_one_tail_1->setText( i18n("%1 %2 %3", mu, QChar(0x2264), mu0));
    ui.rb_h0_one_tail_2->setText( i18n("%1 %2 %3",mu, QChar(0x2265), mu0));
    ui.rb_h0_two_tail->setText( i18n("%1 %2 %3", mu, QChar(0x3D), mu0));

    ui.rb_h0_two_tail->setEnabled(false);
    ui.rb_h0_one_tail_1->setEnabled(false);
    ui.rb_h0_one_tail_2->setEnabled(false);


    // setting muo and alpha buttons
    ui.l_muo->setText( i18n("%1", mu0));
    ui.l_alpha->setText( i18n("%1", QChar(0x3B1)));
    ui.le_muo->setText( i18n("%1", population_mean));
    ui.le_alpha->setText( i18n("%1", significance_level));

    ui.l_muo->setVisible(false);
    ui.l_alpha->setVisible(false);
    ui.le_muo->setVisible(false);
    ui.le_alpha->setVisible(false);

    //    readConnections();

//    auto* style = ui.bAddRow->style();
//    ui.bAddRow->setIcon(style->standardIcon(QStyle::SP_ArrowRight));
//    ui.bAddRow->setToolTip(i18n("Add the selected field to rows"));
//    ui.bRemoveRow->setIcon(style->standardIcon(QStyle::SP_ArrowLeft));
//    ui.bRemoveRow->setToolTip(i18n("Remove the selected field from rows"));

//    ui.bAddColumn->setIcon(style->standardIcon(QStyle::SP_ArrowRight));
//    ui.bAddColumn->setToolTip(i18n("Add the selected field to columns"));
//    ui.bRemoveColumn->setIcon(style->standardIcon(QStyle::SP_ArrowLeft));
//    ui.bRemoveColumn->setToolTip(i18n("Remove the selected field from columns"));

//    //add/remove buttons only enabled if something was selected
//    ui.bAddRow->setEnabled(false);
//    ui.bRemoveRow->setEnabled(false);
//    ui.bAddColumn->setEnabled(false);
//    ui.bRemoveColumn->setEnabled(false);

//    connect(ui.leName, &QLineEdit::textChanged, this, &HypothesisTestDock::nameChanged);
//    connect(ui.leComment, &QLineEdit::textChanged, this, &HypothesisTestDock::commentChanged);
    connect(ui.cbDataSourceType, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &HypothesisTestDock::dataSourceTypeChanged);

    connect(cbSpreadsheet, &TreeViewComboBox::currentModelIndexChanged, this, &HypothesisTestDock::spreadsheetChanged);
//    connect(ui.cbConnection, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
//            this, &HypothesisTestDock::connectionChanged);
//    connect(ui.cbTable, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
//            this, &HypothesisTestDock::tableChanged);
//    connect(ui.bDatabaseManager, &QPushButton::clicked, this, &HypothesisTestDock::showDatabaseManager);

//    connect(ui.bAddRow,  &QPushButton::clicked, this, &HypothesisTestDock::addRow);
//    connect(ui.bRemoveRow, &QPushButton::clicked, this,&HypothesisTestDock::removeRow);
//    connect(ui.bAddColumn,  &QPushButton::clicked, this, &HypothesisTestDock::addColumn);
//    connect(ui.bRemoveColumn, &QPushButton::clicked, this,&HypothesisTestDock::removeColumn);

//      connect(ui.cbCol1, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &HypothesisTestDock::doTTest);
//      connect(ui.cbCol2, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &HypothesisTestDock::doTTest);

//    connect(ui.lwFields, &QListWidget::itemSelectionChanged, this, [=]() {
//        bool enabled = !ui.lwFields->selectedItems().isEmpty();
//        ui.bAddRow->setEnabled(enabled);
//        ui.bAddColumn->setEnabled(enabled);
//    });

//    connect(ui.lwRows, &QListWidget::doubleClicked, this,&HypothesisTestDock::removeRow);
//    connect(ui.lwRows, &QListWidget::itemSelectionChanged, this, [=]() {
//        ui.bRemoveRow->setEnabled(!ui.lwRows->selectedItems().isEmpty());
//    });

//    connect(ui.lwColumns, &QListWidget::doubleClicked, this,&HypothesisTestDock::removeColumn);
//    connect(ui.lwColumns, &QListWidget::itemSelectionChanged, this, [=]() {
//        ui.bRemoveColumn->setEnabled(!ui.lwColumns->selectedItems().isEmpty());
//    });


    connect(ui.cbTestType, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &HypothesisTestDock::showHypothesisTest);
    connect(ui.pbPerformTest, &QPushButton::clicked, this, &HypothesisTestDock::doHypothesisTest);

    //connecting null hypothesis and alternate hypothesis radio button
    connect(ui.rb_h1_one_tail_1, &QRadioButton::toggled, this, &HypothesisTestDock::on_rb_h1_one_tail_1_toggled);
    connect(ui.rb_h1_one_tail_2, &QRadioButton::toggled, this, &HypothesisTestDock::on_rb_h1_one_tail_2_toggled);
    connect(ui.rb_h1_two_tail, &QRadioButton::toggled, this, &HypothesisTestDock::on_rb_h1_two_tail_toggled);

    connect(ui.cbCol1Categorical, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &HypothesisTestDock::col1CatIndexChanged);
}

void HypothesisTestDock::setHypothesisTest(HypothesisTest* HypothesisTest) {
//    m_initializing = true;
    m_hypothesisTest = HypothesisTest;

////    m_aspectTreeModel = new AspectTreeModel(m_hypothesisTest->project());

//    QList<const char*> list;
//    list << "Folder" << "Workbook" << "Spreadsheet" << "LiveDataSource";
//    cbSpreadsheet->setTopLevelClasses(list);

//    list.clear();
//    list << "Spreadsheet" << "LiveDataSource";
////    m_aspectTreeModel->setSelectableAspects(list);

////    cbSpreadsheet->setModel(m_aspectTreeModel);

    //show the properties
    ui.leName->setText(m_hypothesisTest->name());
    ui.leComment->setText(m_hypothesisTest->comment());
    ui.cbDataSourceType->setCurrentIndex(m_hypothesisTest->dataSourceType());
//    if (m_hypothesisTest->dataSourceType() == HypothesisTest::DataSourceSpreadsheet)
//        setModelIndexFromAspect(cbSpreadsheet, m_hypothesisTest->dataSourceSpreadsheet());
//    else
//        ui.cbConnection->setCurrentIndex(ui.cbConnection->findText(m_hypothesisTest->dataSourceConnection()));


    //clearing all cbCol*
    ui.cbCol1->clear();
    ui.cbCol2->clear();
    for (auto* col : m_hypothesisTest->dataSourceSpreadsheet()->children<Column>()) {
        ui.cbCol1Categorical->addItem(col->name());
        if (col->columnMode() == AbstractColumn::Integer || col->columnMode() == AbstractColumn::Numeric) {
            ui.cbCol2->addItem(col->name());
            ui.cbCol1->addItem(col->name());
        }
    }

    this->dataSourceTypeChanged(ui.cbDataSourceType->currentIndex());

    // setting rows and columns in combo box;


        //    //available dimensions and measures
//    ui.lwFields->clear();
//    for (auto dimension : m_hypothesisTest->dimensions())
//        ui.lwFields->addItem(new QListWidgetItem(QIcon::fromTheme("draw-text"), dimension));

//    for (auto measure : m_hypothesisTest->measures())
//        ui.lwFields->addItem(new QListWidgetItem(measure));

//    //undo functions
//    connect(m_hypothesisTest, SIGNAL(aspectDescriptionChanged(const AbstractAspect*)), this, SLOT(hypothesisTestDescriptionChanged(const AbstractAspect*)));
//    //TODO:

//    m_initializing = false;
}

void HypothesisTestDock::showHypothesisTest() {
    ttest = ui.cbTest->currentText() == "T Test";
    ztest = ui.cbTest->currentText() == "Z Test";

    two_sample_independent = ui.cbTestType->currentText() == "Two Sample Independent";
    two_sample_paired = ui.cbTestType->currentText() == "Two Sample Paired";
    one_sample = ui.cbTestType->currentText() == "One Sample";

    ui.lCol1Categorical->setVisible(two_sample_independent);
    ui.cbCol1Categorical->setVisible(two_sample_independent);
    ui.lCol1->setVisible(two_sample_paired);
    ui.cbCol1->setVisible(two_sample_paired);
    ui.lCol2->setVisible(two_sample_independent || two_sample_paired || one_sample);
    ui.cbCol2->setVisible(two_sample_independent || two_sample_paired || one_sample);
    ui.chbEqualVariance->setVisible(ttest && two_sample_independent);
    ui.chbEqualVariance->setChecked(true);
    ui.pbPerformTest->setEnabled(two_sample_independent || two_sample_paired || one_sample);

    ui.rb_h1_one_tail_2->setVisible(two_sample_independent || two_sample_paired || one_sample);
    ui.rb_h1_one_tail_1->setVisible(two_sample_independent || two_sample_paired || one_sample);
    ui.rb_h1_two_tail->setVisible(two_sample_independent || two_sample_paired || one_sample);
    ui.rb_h0_one_tail_1->setVisible(two_sample_independent || two_sample_paired || one_sample);
    ui.rb_h0_one_tail_2->setVisible(two_sample_independent || two_sample_paired || one_sample);
    ui.rb_h0_two_tail->setVisible(two_sample_independent || two_sample_paired || one_sample);
    ui.l_h0->setVisible(two_sample_independent || two_sample_paired || one_sample);
    ui.l_h1->setVisible(two_sample_independent || two_sample_paired || one_sample);

    ui.rb_h1_two_tail->setChecked(true);

    ui.l_muo->setVisible(one_sample);
    ui.le_muo->setVisible(one_sample);
    ui.l_alpha->setVisible(two_sample_independent || two_sample_paired || one_sample);
    ui.le_alpha->setVisible(two_sample_independent || two_sample_paired || one_sample);

    ui.le_muo->setText( i18n("%1", population_mean));
    ui.le_alpha->setText( i18n("%1", significance_level));

    if (two_sample_independent)
        ui.lCol2->setText("Independent Variable");
}

void HypothesisTestDock::doHypothesisTest()  {

    m_hypothesisTest->setPopulationMean(ui.le_muo->text());
    m_hypothesisTest->setSignificanceLevel(ui.le_alpha->text());

    QStringList cols;
    if(ttest) {
        if(two_sample_independent) {
            cols << ui.cbCol1Categorical->currentText() << ui.cbCol2->currentText();
            m_hypothesisTest->setColumns(cols);
            m_hypothesisTest->performTwoSampleIndependetTTest( ui.chbEqualVariance->isChecked());
        }
        else if(two_sample_paired) {
            cols << ui.cbCol1->currentText();
            cols << ui.cbCol2->currentText();
            m_hypothesisTest->setColumns(cols);
            m_hypothesisTest->performTwoSamplePairedTTest();
        }
        else if(one_sample){
            cols << ui.cbCol1->currentText();
            m_hypothesisTest->setColumns(cols);
            m_hypothesisTest->PerformOneSampleTTest();
        }
    }
    else if(ztest) {
        if(two_sample_independent) {
            cols << ui.cbCol1Categorical->currentText();
            cols << ui.cbCol2->currentText();
            m_hypothesisTest->setColumns(cols);
            m_hypothesisTest->performTwoSampleIndependetZTest();
        }
        else if(two_sample_paired) {
            cols << ui.cbCol1->currentText();
            cols << ui.cbCol2->currentText();
            m_hypothesisTest->setColumns(cols);
            m_hypothesisTest->performTwoSamplePairedZTest();
        }
        else if(one_sample){
            cols << ui.cbCol1->currentText();
            m_hypothesisTest->setColumns(cols);
            m_hypothesisTest->PerformOneSampleZTest();
        }
    }

    cols << ui.cbCol1->currentText();
    cols << ui.cbCol2->currentText();
    m_hypothesisTest->setColumns(cols);
    m_hypothesisTest->performTwoSampleTTest();
}

//void HypothesisTestDock::setModelIndexFromAspect(TreeViewComboBox* cb, const AbstractAspect* aspect) {
//    if (aspect)
//        cb->setCurrentModelIndex(m_aspectTreeModel->modelIndexOfAspect(aspect));
//    else
//        cb->setCurrentModelIndex(QModelIndex());
//}

///*!
//    shows the database manager where the connections are created and edited.
//    The selected connection is selected in the connection combo box in this widget.
//**/
//void HypothesisTestDock::showDatabaseManager() {
//    DatabaseManagerDialog* dlg = new DatabaseManagerDialog(this, ui.cbConnection->currentText());

//    if (dlg->exec() == QDialog::Accepted) {
//        //re-read the available connections to be in sync with the changes in DatabaseManager
//        m_initializing = true;
//        ui.cbConnection->clear();
//        readConnections();

//        //select the connection the user has selected in DatabaseManager
//        const QString& conn = dlg->connection();
//        ui.cbConnection->setCurrentIndex(ui.cbConnection->findText(conn));
//        m_initializing = false;

//        connectionChanged();
//    }

//    delete dlg;
//}

///*!
//    loads all available saved connections
//*/
//void HypothesisTestDock::readConnections() {
//    DEBUG("ImportSQLDatabaseWidget: reading available connections");
//    KConfig config(m_configPath, KConfig::SimpleConfig);
//    for (const auto& name : config.groupList())
//        ui.cbConnection->addItem(name);
//}

///*!
// * adds the selected field to the rows
// */
//void HypothesisTestDock::addRow() {
//    QString field = ui.lwFields->currentItem()->text();
//    ui.lwRows->addItem(field);
//    ui.lwFields->takeItem(ui.lwFields->currentRow());
//    m_hypothesisTest->addToRows(field);
//}

///*!
// * removes the selected field from the rows
// */
//void HypothesisTestDock::removeRow() {
//    const QString& field = ui.lwRows->currentItem()->text();
//    ui.lwRows->takeItem(ui.lwRows->currentRow());
//    m_hypothesisTest->removeFromRows(field);
//    updateFields();
//}

///*!
// * adds the selected field to the columns
// */
//void HypothesisTestDock::addColumn() {
//    QString field = ui.lwFields->currentItem()->text();
//    ui.lwColumns->addItem(field);
//    ui.lwFields->takeItem(ui.lwFields->currentRow());
//    m_hypothesisTest->addToColumns(field);
//}

///*!
// * removes the selected field from the columns
// */
//void HypothesisTestDock::removeColumn() {
//    const QString& field = ui.lwColumns->currentItem()->text();
//    ui.lwColumns->takeItem(ui.lwColumns->currentRow());
//    m_hypothesisTest->removeFromColumns(field);
//    updateFields();
//}

///*!
// * re-populates the content of the "Fields" list widget by adding the non-selected fields only.
// * called when a selected field is removed from rows or columns.
// */
//void HypothesisTestDock::updateFields() {
//    ui.lwFields->clear();
//    for (auto dimension : m_hypothesisTest->dimensions())
//        if (!fieldSelected(dimension))
//            ui.lwFields->addItem(new QListWidgetItem(QIcon::fromTheme("draw-text"), dimension));

//    for (auto measure : m_hypothesisTest->measures())
//        if (!fieldSelected(measure))
//            ui.lwFields->addItem(new QListWidgetItem(measure));
//}

///*!
// * return \c true if the field name \c field was selected among rows or columns,
// * return \c false otherwise.
// * */
//bool HypothesisTestDock::fieldSelected(const QString& field) {
//    for (int i = 0; i<ui.lwRows->count(); ++i)
//        if (ui.lwRows->item(i)->text() == field)
//            return true;

//    for (int i = 0; i<ui.lwColumns->count(); ++i)
//        if (ui.lwColumns->item(i)->text() == field)
//            return true;

//    return false;
//}

////*************************************************************
////****** SLOTs for changes triggered in HypothesisTestDock *******
////*************************************************************
//void HypothesisTestDock::nameChanged() {
//    if (m_initializing)
//        return;

//    m_hypothesisTest->setName(ui.leName->text());
//}

//void HypothesisTestDock::commentChanged() {
//    if (m_initializing)
//        return;

//    m_hypothesisTest->setComment(ui.leComment->text());
//}

void HypothesisTestDock::dataSourceTypeChanged(int index) {
    HypothesisTest::DataSourceType type = (HypothesisTest::DataSourceType)index;
    bool showDatabase = (type == HypothesisTest::DataSourceDatabase);
    ui.lSpreadsheet->setVisible(!showDatabase);
    cbSpreadsheet->setVisible(!showDatabase);
    ui.lConnection->setVisible(showDatabase);
    ui.cbConnection->setVisible(showDatabase);
    ui.bDatabaseManager->setVisible(showDatabase);
    ui.lTable->setVisible(showDatabase);
    ui.cbTable->setVisible(showDatabase);

//    if (m_initializing)
//        return;

    m_hypothesisTest->setComment(ui.leComment->text());
}

void HypothesisTestDock::spreadsheetChanged(const QModelIndex& index) {
    auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
    Spreadsheet* spreadsheet = dynamic_cast<Spreadsheet*>(aspect);

    //clear the previous definitions of combo-box columns
    //clearing all cbCol*
    ui.cbCol1->clear();
    ui.cbCol2->clear();
    for (auto* col : m_hypothesisTest->dataSourceSpreadsheet()->children<Column>()) {
        ui.cbCol1Categorical->addItem(col->name());
        if (col->columnMode() == AbstractColumn::Integer || col->columnMode() == AbstractColumn::Numeric) {
            ui.cbCol2->addItem(col->name());
            ui.cbCol1->addItem(col->name());
        }
    }

    m_hypothesisTest->setDataSourceSpreadsheet(spreadsheet);
}

// currently no need of this slot
void HypothesisTestDock::col1CatIndexChanged(int index) {
    if (index < 0) return;

    if (two_sample_paired) {
        ui.lCol2->setText("Independent Variable");
        return;
    }

    QString selected_text = ui.cbCol1Categorical->currentText();
    Column* col1 = m_hypothesisTest->dataSourceSpreadsheet()->column(selected_text);

    if (col1->columnMode() == AbstractColumn::Integer || col1->columnMode() == AbstractColumn::Numeric) {
        ui.lCol2->setText("Independent Variable");
    }
    else {
        ui.lCol2->setText("Dependent Variable");
    }

}


//void HypothesisTestDock::connectionChanged() {
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
//    QDEBUG("HypothesisTestDock: connecting to " + connection);
//    const QString& driver = group.readEntry("Driver");
//    m_db = QSqlDatabase::addDatabase(driver);

//    const QString& dbName = group.readEntry("DatabaseName");
//    if (DatabaseManagerWidget::isFileDB(driver)) {
//        if (!QFile::exists(dbName)) {
//            KMessageBox::error(this, i18n("Couldn't find the database file '%1'. Please check the connection settings.", dbName),
//                                    i18n("Connection Failed"));
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

//// 	m_hypothesisTest->setDataSourceConnection(connection);
//}

//void HypothesisTestDock::tableChanged() {
//    const QString& table = ui.cbTable->currentText();

//    //show all attributes of the selected table
//// 	for (const auto* col : spreadsheet->children<Column>()) {
//// 		QListWidgetItem* item = new QListWidgetItem(col->icon(), col->name());
//// 		ui.lwFields->addItem(item);
//// 	}

//    if (m_initializing)
//        return;

//// 	m_hypothesisTest->setDataSourceTable(table);
//}

////*************************************************************
////******** SLOTs for changes triggered in Matrix *********
////*************************************************************
//void HypothesisTestDock::hypothesisTestDescriptionChanged(const AbstractAspect* aspect) {
//    if (m_hypothesisTest != aspect)
//        return;

//    m_initializing = true;
//    if (aspect->name() != ui.leName->text())
//        ui.leName->setText(aspect->name());
//    else if (aspect->comment() != ui.leComment->text())
//        ui.leComment->setText(aspect->comment());

//    m_initializing = false;
//}

////*************************************************************
////******************** SETTINGS *******************************
////*************************************************************
//void HypothesisTestDock::load() {

//}

//void HypothesisTestDock::loadConfigFromTemplate(KConfig& config) {
//    Q_UNUSED(config);
//}

///*!
//    loads saved matrix properties from \c config.
// */
//void HypothesisTestDock::loadConfig(KConfig& config) {
//    Q_UNUSED(config);
//}

///*!
//    saves matrix properties to \c config.
// */
//void HypothesisTestDock::saveConfigAsTemplate(KConfig& config) {
//    Q_UNUSED(config);
//}

//TODO: Rather than inbuilt slots use own decided slots for checked rather than clicked

// for alternate hypothesis
// one_tail_1 is mu > mu0; one_tail_2 is mu < mu0; two_tail = mu != mu0;
void HypothesisTestDock::on_rb_h1_one_tail_1_toggled(bool checked) {
    if (!checked) return;
    ui.rb_h0_one_tail_1->setChecked(true);
    m_hypothesisTest->setTailType(HypothesisTest::TailPositive);
}

void HypothesisTestDock::on_rb_h1_one_tail_2_toggled(bool checked) {
    if (!checked) return;
    ui.rb_h0_one_tail_2->setChecked(true);
    m_hypothesisTest->setTailType(HypothesisTest::TailNegative);
}

void HypothesisTestDock::on_rb_h1_two_tail_toggled(bool checked) {
    if (!checked) return;
    ui.rb_h0_two_tail->setChecked(true);
    m_hypothesisTest->setTailType(HypothesisTest::TailTwo);
}
