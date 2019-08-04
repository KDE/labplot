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

#include <QTimer>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QStandardItem>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KMessageBox>
#include <KSharedConfig>
#ifdef HAVE_KF5_SYNTAX_HIGHLIGHTING
#include <KF5/KSyntaxHighlighting/SyntaxHighlighter>
#include <KF5/KSyntaxHighlighting/Definition>
#include <KF5/KSyntaxHighlighting/Theme>
#endif

ImportSQLDatabaseWidget::ImportSQLDatabaseWidget(QWidget* parent) : QWidget(parent) {
	ui.setupUi(this);

	ui.cbImportFrom->addItem(i18n("Table"));
	ui.cbImportFrom->addItem(i18n("Custom query"));

	ui.bDatabaseManager->setIcon(QIcon::fromTheme("network-server-database"));
	ui.bDatabaseManager->setToolTip(i18n("Manage connections"));
	ui.twPreview->setEditTriggers(QAbstractItemView::NoEditTriggers);

	ui.cbNumberFormat->addItems(AbstractFileFilter::numberFormats());
	ui.cbDateTimeFormat->addItems(AbstractColumn::dateTimeFormats());

#ifdef HAVE_KF5_SYNTAX_HIGHLIGHTING
	m_highlighter = new KSyntaxHighlighting::SyntaxHighlighter(ui.teQuery->document());
	m_highlighter->setDefinition(m_repository.definitionForName("SQL"));
	m_highlighter->setTheme(  (palette().color(QPalette::Base).lightness() < 128)
								? m_repository.defaultTheme(KSyntaxHighlighting::Repository::DarkTheme)
								: m_repository.defaultTheme(KSyntaxHighlighting::Repository::LightTheme) );
#endif

	m_configPath = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).constFirst() +  "sql_connections";

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
	ui.cbNumberFormat->setCurrentIndex(config.readEntry("NumberFormat", (int)QLocale::AnyLanguage));
	ui.cbDateTimeFormat->setCurrentItem(config.readEntry("DateTimeFormat", "yyyy-dd-MM hh:mm:ss:zzz"));
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
	config.writeEntry("NumberFormat", ui.cbNumberFormat->currentIndex());
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
	for (const auto& name : config.groupList())
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
	ui.twPreview->setColumnCount(0);
	ui.twPreview->setRowCount(0);

	if (ui.cbConnection->currentIndex() == -1)
		return;

	//connection name was changed, determine the current connections settings
	KConfig config(m_configPath, KConfig::SimpleConfig);
	KConfigGroup group = config.group(ui.cbConnection->currentText());

	//close and remove the previos connection, if available
	if (m_db.isOpen()) {
		m_db.close();
		QSqlDatabase::removeDatabase(m_db.driverName());
	}

	//open the selected connection
	const QString& driver = group.readEntry("Driver");
	m_db = QSqlDatabase::addDatabase(driver);

	const QString& dbName = group.readEntry("DatabaseName");
	if (DatabaseManagerWidget::isFileDB(driver)) {
		if (!QFile::exists(dbName)) {
			KMessageBox::error(this, i18n("Couldn't find the database file '%1'. Please check the connection settings.", dbName),
									i18n("Connection Failed"));
			setInvalid();
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

	RESET_CURSOR;
}

void ImportSQLDatabaseWidget::refreshPreview() {
	if (!ui.lwTables->currentItem()) {
		setInvalid();
		return;
	}

	WAIT_CURSOR;
	ui.twPreview->clear();

	//execute the current query (select on a table or a custom query)
	const QString& query = currentQuery(true);
	if (query.isEmpty()) {
		RESET_CURSOR;
		setInvalid();
		return;
	}

	QSqlQuery q;
	q.prepare(currentQuery(true));
	q.setForwardOnly(true);
	q.exec();
	if (!q.isActive() || !q.next()) { // check if query was successful and got to first record
		RESET_CURSOR;
		if (!q.lastError().databaseText().isEmpty())
			KMessageBox::error(this, q.lastError().databaseText(), i18n("Unable to Execute Query"));

		setInvalid();
		return;
	}

	//resize the table to the number of columns (=number of fields in the result set)
	m_cols = q.record().count();
	ui.twPreview->setColumnCount(m_cols);

	//determine the names and the data type (column modes) of the table columns.
	//check whether we have numerical data only by checking the data types of the first record.
	m_columnNames.clear();
	m_columnModes.clear();
	bool numeric = true;
	const auto numberFormat = (QLocale::Language)ui.cbNumberFormat->currentIndex();
	const QString& dateTimeFormat = ui.cbDateTimeFormat->currentText();
// 	ui.twPreview->setRowCount(1); //add the first row for the check boxes
	for (int i = 0; i < m_cols; ++i) {
		//name
		m_columnNames << q.record().fieldName(i);

		//value and type
		const QString valueString = q.record().value(i).toString();
		AbstractColumn::ColumnMode mode = AbstractFileFilter::columnMode(valueString, dateTimeFormat, numberFormat);
		m_columnModes << mode;
		if (mode != AbstractColumn::Numeric)
			numeric = false;

		//header item
		QTableWidgetItem* item = new QTableWidgetItem(m_columnNames[i] + QLatin1String(" {") + ENUM_TO_STRING(AbstractColumn, ColumnMode, mode) + QLatin1String("}"));
		item->setTextAlignment(Qt::AlignLeft);
		item->setIcon(AbstractColumn::iconForMode(mode));
		ui.twPreview->setHorizontalHeaderItem(i, item);

		//create checked items
// 		QTableWidgetItem* itemChecked = new QTableWidgetItem();
// 		itemChecked->setCheckState(Qt::Checked);
// 		ui.twPreview->setItem(0, i, itemChecked);
	}

	//preview the data
	const bool customQuery = (ui.cbImportFrom->currentIndex() != 0);
	int row = 0;
	do {
		for (int col = 0; col < m_cols; ++col) {
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
	if (index == 0) { //import from a table
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
	if (!dataSource)
		return;

	WAIT_CURSOR;
	//execute the current query (select on a table or a custom query)
	QSqlQuery q;
// 	q.setForwardOnly(true); //TODO: crashes most probably because of q.last() and q.first() below
	q.prepare(currentQuery());
	if (!q.exec() || !q.isActive()) {
		RESET_CURSOR;
		if (!q.lastError().databaseText().isEmpty())
			KMessageBox::error(this, q.lastError().databaseText(), i18n("Unable to Execute Query"));

		setInvalid();
		return;
	}

	//determine the number of rows/records to read
	q.last();
	const int rows = q.at()+1;
	q.first();

	// pointers to the actual data containers
	//columnOffset indexes the "start column" in the datasource. Data will be imported starting from this column.
	QVector<void*> dataContainer;
	int columnOffset = dataSource->prepareImport(dataContainer, importMode, rows, m_cols, m_columnNames, m_columnModes);

	//number and DateTime formatting
	const QString& dateTimeFormat = ui.cbDateTimeFormat->currentText();
	const QLocale numberFormat = QLocale((QLocale::Language)ui.cbNumberFormat->currentIndex());

	//read the data
	int row = 0;
	do {
		for (int col = 0; col < m_cols; ++col) {
			const QString valueString = q.record().value(col).toString();

			// set value depending on data type
			switch (m_columnModes[col]) {
			case AbstractColumn::Numeric: {
				bool isNumber;
				const double value = numberFormat.toDouble(valueString, &isNumber);
				static_cast<QVector<double>*>(dataContainer[col])->operator[](row) = (isNumber ? value : NAN);
				break;
			}
			case AbstractColumn::Integer: {
				bool isNumber;
				const int value = numberFormat.toInt(valueString, &isNumber);
				static_cast<QVector<int>*>(dataContainer[col])->operator[](row) = (isNumber ? value : NAN);
				break;
			}
			case AbstractColumn::DateTime: {
				const QDateTime valueDateTime = QDateTime::fromString(valueString, dateTimeFormat);
				static_cast<QVector<QDateTime>*>(dataContainer[col])->operator[](row) = valueDateTime.isValid() ? valueDateTime : QDateTime();
				break;
			}
			case AbstractColumn::Text:
				static_cast<QVector<QString>*>(dataContainer[col])->operator[](row) = valueString;
				break;
			case AbstractColumn::Month:	// never happens
			case AbstractColumn::Day:
				break;
			}
		}

		row++;
		emit completed(100 * row/rows);
	} while (q.next());
	DEBUG("	Read " << row << " rows");

	dataSource->finalizeImport(columnOffset, 1, m_cols, row, dateTimeFormat, importMode);
	RESET_CURSOR;
}

QString ImportSQLDatabaseWidget::currentQuery(bool preview) {
	QString query;
	const bool customQuery = (ui.cbImportFrom->currentIndex() != 0);
	if ( !customQuery ) {
		const QString& tableName = ui.lwTables->currentItem()->text();
		if (!preview) {
			query = QLatin1String("SELECT * FROM ") + tableName;
		} else {
			//preview the content of the currently selected table
			const QString& driver = m_db.driverName();
			const QString& limit = QString::number(ui.sbPreviewLines->value());
			if ( (driver == QLatin1String("QSQLITE3")) || (driver == QLatin1String("QSQLITE"))
				|| (driver == QLatin1String("QMYSQL3")) || (driver == QLatin1String("QMYSQL"))
				|| (driver == QLatin1String("QPSQL"))  )
				query = QLatin1String("SELECT * FROM ") + tableName + QLatin1String(" LIMIT ") +  limit;
			else if (driver == QLatin1String("QOCI"))
				query = QLatin1String("SELECT * FROM ") + tableName + QLatin1String(" ROWNUM<=") + limit;
			else if (driver == QLatin1String("QDB2"))
				query = QLatin1String("SELECT * FROM ") + tableName + QLatin1String(" FETCH FIRST ") + limit + QLatin1String(" ROWS ONLY");
			else if (driver == QLatin1String("QIBASE"))
				query = QLatin1String("SELECT * FROM ") + tableName + QLatin1String(" ROWS ") + limit;
			else
				//for ODBC the DBMS is not known and it's not clear what syntax to use -> select all rows
				query = QLatin1String("SELECT * FROM ") + tableName;
		}
	} else {
		//preview the result of a custom query
		query = ui.teQuery->toPlainText().simplified();
	}

	return query;
}

/*!
	shows the database manager where the connections are created and edited.
	The selected connection is selected in the connection combo box in this widget.
**/
void ImportSQLDatabaseWidget::showDatabaseManager() {
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

void ImportSQLDatabaseWidget::setInvalid() {
	if (m_valid) {
		ui.twPreview->setColumnCount(0);
		ui.twPreview->setRowCount(0);

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
