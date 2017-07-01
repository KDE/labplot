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
#include "DatabaseManagerWidget.h"
#include "backend/datasources/AbstractDataSource.h"
#include "backend/datasources/filters/AbstractFileFilter.h"
#include "backend/lib/macros.h"

#include <KGlobal>
#include <KLocale>
#include <KMessageBox>
#ifdef HAVE_KF5_SYNTAX_HIGHLIGHTING
#include <KF5/KSyntaxHighlighting/SyntaxHighlighter>
#include <KF5/KSyntaxHighlighting/Definition>
#include <KF5/KSyntaxHighlighting/Theme>
#endif

#include <QtSql>
#include <QStandardItem>

ImportSQLDatabaseWidget::ImportSQLDatabaseWidget(QWidget* parent) : QWidget(parent), m_databaseTreeModel(0), m_initializing(0), m_valid(false), m_numeric(false) {
	ui.setupUi(this);

	ui.cbImportFrom->addItem(i18n("Table"));
	ui.cbImportFrom->addItem(i18n("Custom query"));

	ui.bDatabaseManager->setIcon(QIcon::fromTheme("network-server-database"));
	ui.twPreview->setEditTriggers(QAbstractItemView::NoEditTriggers);

	ui.cbNumbersFormat->addItems(AbstractFileFilter::numberFormats());
	ui.cbDateTimeFormat->addItems(AbstractColumn::dateTimeFormats());

#ifdef HAVE_KF5_SYNTAX_HIGHLIGHTING
	m_highlighter = new KSyntaxHighlighting::SyntaxHighlighter(ui.teQuery->document());
	m_highlighter->setDefinition(m_repository.definitionForName("SQL"));
	m_highlighter->setTheme(  (palette().color(QPalette::Base).lightness() < 128)
								? m_repository.defaultTheme(KSyntaxHighlighting::Repository::DarkTheme)
								: m_repository.defaultTheme(KSyntaxHighlighting::Repository::LightTheme) );
#endif

	m_configPath = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).first() +  "sql_connections";

	connect( ui.cbConnection, SIGNAL(currentIndexChanged(int)), SLOT(connectionChanged()) );
	connect( ui.cbImportFrom, SIGNAL(currentIndexChanged(int)), SLOT(importFromChanged(int)) );
	connect( ui.bDatabaseManager, SIGNAL(clicked()), this, SLOT(showDatabaseManager()) );
	connect( ui.lwTables, SIGNAL(currentRowChanged(int)), this, SLOT(refreshPreview()) );
	connect( ui.bRefreshPreview, SIGNAL(clicked()), this, SLOT(refreshPreview()) );

	//defer the loading of settings a bit in order to show the dialog prior to blocking the GUI in refreshPreview()
	QTimer::singleShot( 100, this, SLOT(loadSettings()) );
}

void ImportSQLDatabaseWidget::loadSettings() {
	m_initializing = true;

	//read available connections
	readConnections();

	//load last used connection and other settings
	KConfigGroup config(KSharedConfig::openConfig(), "ImportSQLDatabaseWidget");
	ui.cbConnection->setCurrentIndex(ui.cbConnection->findText(config.readEntry("Connection", "")));
	ui.cbImportFrom->setCurrentIndex(config.readEntry("ImportFrom", 0));
	importFromChanged(ui.cbImportFrom->currentIndex());
	ui.cbNumbersFormat->setCurrentIndex(config.readEntry("NubmersFormat", (int)AbstractFileFilter::LocaleSystem));
	ui.cbDateTimeFormat->setCurrentItem(config.readEntry("DateTimeFormat", "hh:mm:ss"));
	QList<int> defaultSizes;
	defaultSizes << 100 << 100;
	ui.splitterMain->setSizes(config.readEntry("SplitterMainSizes", defaultSizes));
	ui.splitterPreview->setSizes(config.readEntry("SplitterPreviewSizes", defaultSizes));
	//TODO


	m_initializing = false;

	//all settings loaded -> trigger the selection of the last used connection in order to get the data preview
	connectionChanged();
}

ImportSQLDatabaseWidget::~ImportSQLDatabaseWidget() {
	// save current settings
	KConfigGroup config(KSharedConfig::openConfig(), "ImportSQLDatabaseWidget");
	config.writeEntry("Connection", ui.cbConnection->currentText());
	config.writeEntry("ImportFrom", ui.cbImportFrom->currentIndex());
	config.writeEntry("NumbersFormat", ui.cbNumbersFormat->currentText());
	config.writeEntry("DateTimeFormat", ui.cbDateTimeFormat->currentText());
	config.writeEntry("SplitterMainSizes", ui.splitterMain->sizes());
	config.writeEntry("SplitterPreviewSizes", ui.splitterPreview->sizes());
	//TODO
}

/*!
 * in case the import from a table is selected, returns the currently selected database table.
 * returns empty string otherwise.
 */
QString ImportSQLDatabaseWidget::selectedTable() const {
	if (ui.cbImportFrom->currentIndex() == 0) {
		if (ui.lwTables->currentItem())
			return ui.lwTables->currentItem()->text();
	}

	return QString();
}

/*!
	returns \c true if a working connections was selected and a table (or custom query) is provided and ready to be imported.
	returns \c false otherwise.
 */
bool ImportSQLDatabaseWidget::isValid() const {
	return m_valid;
}

/*!
	returns \c true if the selected table or the result of a custom query contains numeric data only.
	returns \c false otherwise.
 */
bool ImportSQLDatabaseWidget::isNumericData() const {
	return m_numeric;
}

/*!
	loads all available saved connections
*/
void ImportSQLDatabaseWidget::readConnections() {
	DEBUG("ImportSQLDatabaseWidget: reading available connections");
	KConfig config(m_configPath, KConfig::SimpleConfig);
	foreach(QString name, config.groupList())
		ui.cbConnection->addItem(name);
}

void ImportSQLDatabaseWidget::connectionChanged() {
	if (m_initializing)
		return;

	QDEBUG("ImportSQLDatabaseWidget: connecting to " + ui.cbConnection->currentText());

	//clear the previously shown content
	ui.teQuery->clear();
	ui.lwTables->clear();
	ui.twPreview->clear();

	if (ui.cbConnection->currentIndex() == -1)
		return;

	//connection name was changed, determine the current connections settings
	KConfig config(m_configPath, KConfig::SimpleConfig);
	KConfigGroup group = config.group(ui.cbConnection->currentText());

	//open the selected connection
	const QString driver = group.readEntry("Driver");
	m_db = QSqlDatabase::addDatabase(driver);
	m_db.setDatabaseName( group.readEntry("DatabaseName") );
	if (!DatabaseManagerWidget::isFileDB(driver)) {
		m_db.setHostName( group.readEntry("HostName") );
		m_db.setPort( group.readEntry("Port", 0) );
		m_db.setUserName( group.readEntry("UserName") );
		m_db.setPassword( group.readEntry("Password") );
	}

	if (!m_db.open()) {
		KMessageBox::error(this, i18n("Failed to connect to the database '%1'. Please check the connection settings.", ui.cbConnection->currentText()),
								 i18n("Connection failed"));
		setInvalid();
		return;
	}

	//show all available database tables
	if (m_db.tables().size()) {
		ui.lwTables->addItems(m_db.tables());
		ui.lwTables->setCurrentRow(0);
		for (int i = 0; i < ui.lwTables->count(); ++i)
			ui.lwTables->item(i)->setIcon(QIcon::fromTheme("view-form-table"));
	} else
		setInvalid();
}

void ImportSQLDatabaseWidget::refreshPreview() {
	if (!ui.lwTables->currentItem()) {
		setInvalid();
		return;
	}

	WAIT_CURSOR;
	ui.twPreview->clear();
	QString tableName = ui.lwTables->currentItem()->text();

	QString query;
	bool customQuery = (ui.cbImportFrom->currentIndex() != 0);
	if ( !customQuery ) {
		//preview the content of the currently selected table
		const QString driver = m_db.driverName();
		const QString limit = QString::number(ui.sbPreviewLines->value());
		if ( (driver == QLatin1String("QSQLITE")) || (driver == QLatin1String("QSQLITE3")) || (driver == QLatin1String("QMYSQL")) || (driver == QLatin1String("QPSQL")) )
			query = QLatin1String("SELECT * FROM ") + tableName + QLatin1String(" LIMIT ") +  limit;
		else if (driver == QLatin1String("QOCI"))
			query = QLatin1String("SELECT * FROM ") + tableName + QLatin1String(" ROWNUM<=") + limit;
		else if (driver == QLatin1String("QDB2"))
			query = QLatin1String("SELECT * FROM ") + tableName + QLatin1String(" FETCH FIRST ") + limit + QLatin1String(" ROWS ONLY");
		else if (driver == QLatin1String("QIBASE"))
			query = QLatin1String("SELECT * FROM ") + tableName + QLatin1String(" ROWS ") + limit;
		else
			query = QLatin1String("SELECT TOP ") + limit + QLatin1String(" * FROM ") + tableName;
	} else {
		//preview the result of a custom query
		query = ui.teQuery->toPlainText();
		if ( query.trimmed().isEmpty() ) {
			setInvalid();
			RESET_CURSOR;
			return;
		}
	}

	QSqlQuery q(query);
	if (!q.isActive()) {
		setInvalid();
		updateStatus();
		RESET_CURSOR;
		return;
	}

	//resize the table to the number of columns (=number of fields in the result set)
	const int m_cols = q.record().count();
	ui.twPreview->setColumnCount(m_cols);

	//determine the names and the data type (column modes) of the table columns.
	//check whether we have numerical data only by checking the data types of the first record.
	m_columnNames.clear();
	m_columnModes.clear();
	bool numeric = true;
	AbstractFileFilter::Locale numbersFormat = (AbstractFileFilter::Locale)ui.cbNumbersFormat->currentIndex();
	const QString& dateTimeFormat = ui.cbDateTimeFormat->currentText();
	q.next(); //go to the first record
	for (int i = 0; i < m_cols; ++i) {
		//name
		m_columnNames << q.record().fieldName(i);

		//value and type
		const QString valueString = q.record().value(i).toString();
		AbstractColumn::ColumnMode mode = AbstractFileFilter::columnMode(valueString, dateTimeFormat, numbersFormat);
		m_columnModes << mode;
		if (mode != AbstractColumn::Numeric)
			numeric = false;

		//header item
		QTableWidgetItem* item = new QTableWidgetItem(m_columnNames[i] + QLatin1String(" {") + ENUM_TO_STRING(AbstractColumn, ColumnMode, mode) + QLatin1String("}"));
		item->setTextAlignment(Qt::AlignLeft);
		item->setIcon(AbstractColumn::iconForMode(mode));
		ui.twPreview->setHorizontalHeaderItem(i, item);
	}

	//preview the data
	int row = 0;
	do {
		for(int col = 0; col < m_cols; ++col) {
			ui.twPreview->setRowCount(row+1);
			ui.twPreview->setItem(row, col, new QTableWidgetItem(q.value(col).toString()) );
		}
		row++;

		//in case a custom query is executed, check whether the row number limit is reached
		if (customQuery && row >= ui.sbPreviewLines->value())
			break;
	} while (q.next());

	ui.twPreview->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);

	setValid();

	if (numeric != m_numeric) {
		m_numeric = numeric;
		emit stateChanged();
	}

	RESET_CURSOR;
}

void ImportSQLDatabaseWidget::importFromChanged(int index) {
	if (index==0) { //import from a table
		ui.gbQuery->hide();
		ui.lwTables->show();
	} else { //import the result set of a custom query
		ui.gbQuery->show();
		ui.lwTables->hide();
		ui.twPreview->clear();
	}

	refreshPreview();
}

void ImportSQLDatabaseWidget::read(AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode) {
	int columnOffset = 0;	// indexes the "start column" in the datasource. Data will be imported starting from this column.
	QVector<void*> dataContainer;	// pointers to the actual data containers
	m_rows = 100;
	if (dataSource)
		columnOffset = dataSource->prepareImport(dataContainer, importMode, m_rows, m_cols, m_columnNames, m_columnModes);

// 	if (!m_db.isValid()) return;
// 	if (ui.sbEndRow->value() < ui.sbStartRow->value()) return;
//
// 	if (showPreview) {
// 		ui.twPreviewTable->setColumnCount(0);
// 		ui.twPreviewTable->setRowCount(ui.sbEndRow->value() - ui.sbStartRow->value() + 1);
// 	}
//
// 	m_db.open();
// 	if (!m_db.isOpen())
// 		return;
//
// 	for(int row = 0; row < m_databaseTreeModel->rowCount(); row++) {
// 		QString tableName = m_databaseTreeModel->item(row)->text();
// 		QString columnNameList;
// 		int columnCount = 0;
// 		for(int columnIndex = 0; columnIndex < m_databaseTreeModel->item(row)->rowCount(); columnIndex++) {
// 			QStandardItem* columnItem = m_databaseTreeModel->item(row)->child(columnIndex);
// 			if (columnItem->checkState() == Qt::Checked) {
// 				if (!columnNameList.isEmpty()) columnNameList += " , ";
// 				columnNameList += columnItem->text();
// 				columnCount++;
// 			}
// 		}
// // 		if (columnCount) previewColumn(columnNameList, tableName, columnCount, showPreview);
// 		emit completed(100 * row/m_rows);
// 	}
// 	m_db.close();
}

void ImportSQLDatabaseWidget::updateStatus() {
	QString msg = m_db.lastError().text().simplified();
	if (!msg.isEmpty())
		KMessageBox::error(this, msg, i18n("Database Error"));
}

/*!
	shows the database manager where the connections are created and edited.
	The selected connection is selected in the connection combo box in this widget.
**/
void ImportSQLDatabaseWidget::showDatabaseManager() {
	DatabaseManagerDialog* dlg = new DatabaseManagerDialog(this, ui.cbConnection->currentText());

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

void ImportSQLDatabaseWidget::setInvalid() {
	if (m_valid) {
		m_valid = false;
		emit stateChanged();
	}
}

void ImportSQLDatabaseWidget::setValid() {
	if (!m_valid) {
		m_valid = true;
		emit stateChanged();
	}
}
