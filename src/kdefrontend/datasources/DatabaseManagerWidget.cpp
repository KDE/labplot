/***************************************************************************
File                 : DatabaseManagerWidget.cpp
Project              : LabPlot
Description          : widget for managing database connections
--------------------------------------------------------------------
Copyright            : (C) 2017-2018 Alexander Semke (alexander.semke@web.de)

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

#include "DatabaseManagerWidget.h"
#include "backend/lib/macros.h"

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KMessageBox>

#include <QFileDialog>
#include <QTimer>
#include <QSqlDatabase>
#include <QtSql>

/*!
   \class DatabaseManagerWidget
   \brief widget for managing database connections, embedded in \c DatabaseManagerDialog.

   \ingroup kdefrontend
*/
DatabaseManagerWidget::DatabaseManagerWidget(QWidget* parent, const QString& conn) : QWidget(parent),
	m_initializing(false), m_initConnName(conn) {

	m_configPath = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).constFirst() +  "sql_connections";

	ui.setupUi(this);

	ui.tbAdd->setIcon(QIcon::fromTheme("list-add"));
	ui.tbDelete->setIcon(QIcon::fromTheme("list-remove"));
	ui.bOpen->setIcon(QIcon::fromTheme("document-open"));
	ui.bTestConnection->setIcon(QIcon::fromTheme("network-connect"));

	ui.tbAdd->setToolTip(i18n("Add new database connection"));
	ui.tbDelete->setToolTip(i18n("Delete selected database connection"));
	ui.bOpen->setToolTip(i18n("Open database file"));
	ui.bTestConnection->setToolTip(i18n("Test selected database connection"));

	//add the list of supported SQL drivers
	ui.cbDriver->addItems(QSqlDatabase::drivers());

	//SIGNALs/SLOTs
	connect( ui.lwConnections, SIGNAL(currentRowChanged(int)), this, SLOT(connectionChanged(int)) );
	connect( ui.tbAdd, SIGNAL(clicked()), this, SLOT(addConnection()) );
	connect( ui.tbDelete, SIGNAL(clicked()), this, SLOT(deleteConnection()) );
	connect( ui.bTestConnection, SIGNAL(clicked()), this, SLOT(testConnection()) );
	connect( ui.bOpen, SIGNAL(clicked()), this, SLOT(selectFile()) );
	connect( ui.cbDriver, SIGNAL(currentIndexChanged(int)), SLOT(driverChanged()) );

	connect( ui.leName, SIGNAL(textChanged(QString)), this, SLOT(nameChanged(QString)) );
	connect( ui.leDatabase, SIGNAL(textChanged(QString)), this, SLOT(databaseNameChanged()) );
	connect( ui.leHost, SIGNAL(textChanged(QString)), this, SLOT(hostChanged()) );
	connect( ui.sbPort, SIGNAL(valueChanged(int)), this, SLOT(portChanged()) );
	connect( ui.leUserName, SIGNAL(textChanged(QString)), this, SLOT(userNameChanged()) );
	connect( ui.lePassword, SIGNAL(textChanged(QString)), this, SLOT(passwordChanged()) );

	QTimer::singleShot( 100, this, SLOT(loadConnections()) );
}

QString DatabaseManagerWidget::connection() const {
	if (ui.lwConnections->currentItem())
		return ui.lwConnections->currentItem()->text();
	else
		return QString();
}

/*!
	shows the settings of the currently selected connection
 */
void DatabaseManagerWidget::connectionChanged(int index) {
	if (m_initializing)
		return;

	if (index == -1)
		return;

	//show the settings for the selected connection
	m_initializing = true;
	ui.leName->setText(m_connections[index].name);
	ui.cbDriver->setCurrentIndex(ui.cbDriver->findText(m_connections[index].driver));
	ui.leDatabase->setText(m_connections[index].dbName);

	if (!isFileDB(m_connections[index].driver)) {
		ui.leHost->setText(m_connections[index].hostName);
		ui.sbPort->setValue(m_connections[index].port);
		ui.leUserName->setText(m_connections[index].userName);
		ui.lePassword->setText(m_connections[index].password);
	}
	m_initializing = false;
}

void DatabaseManagerWidget::nameChanged(const QString& name) {
	//check uniqueness of the provided name
	bool unique = true;
	for(int i = 0; i < ui.lwConnections->count(); ++i) {
		if (ui.lwConnections->currentRow() == i)
			continue;

		if (name == ui.lwConnections->item(i)->text()) {
			unique = false;
			break;
		}
	}

	if (unique) {
		ui.leName->setStyleSheet("");
		ui.lwConnections->currentItem()->setText(name);

		if (!m_initializing) {
			m_connections[ui.lwConnections->currentRow()].name = name;
			emit changed();
		}
	} else
		ui.leName->setStyleSheet("QLineEdit{background: red;}");
}

void DatabaseManagerWidget::driverChanged() {
	//hide non-relevant fields (like host name, etc.) for file DBs
	bool fileDB = isFileDB(ui.cbDriver->currentText());
	ui.lHost->setVisible(!fileDB);
	ui.leHost->setVisible(!fileDB);
	ui.lPort->setVisible(!fileDB);
	ui.sbPort->setVisible(!fileDB);
	ui.bOpen->setVisible(fileDB);
	ui.gbAuthentication->setVisible(!fileDB);

	if (m_initializing)
		return;

	m_connections[ui.lwConnections->currentRow()].driver = ui.cbDriver->currentText();
	emit changed();
}

void DatabaseManagerWidget::selectFile() {
	KConfigGroup conf(KSharedConfig::openConfig(), QLatin1String("DatabaseManagerWidget"));
	QString dir = conf.readEntry(QLatin1String("LastDir"), "");
	QString path = QFileDialog::getOpenFileName(this, i18n("Select the database file"), dir);
	if (path.isEmpty())
		return; //cancel was clicked in the file-dialog

	int pos = path.lastIndexOf(QDir::separator());
	if (pos != -1) {
		QString newDir = path.left(pos);
		if (newDir != dir)
			conf.writeEntry(QLatin1String("LastDir"), newDir);
	}

	ui.leDatabase->setText(path);
}

void DatabaseManagerWidget::hostChanged() {
	if (m_initializing)
		return;

	m_connections[ui.lwConnections->currentRow()].hostName = ui.leHost->text();
	emit changed();
}

void DatabaseManagerWidget::portChanged() {
	if (m_initializing)
		return;

	m_connections[ui.lwConnections->currentRow()].port = ui.sbPort->value();
	emit changed();
}

void DatabaseManagerWidget::databaseNameChanged() {
	if (m_initializing)
		return;

	QString dbName = ui.leDatabase->text().simplified();
	if (isFileDB(ui.cbDriver->currentText())) {
#ifndef HAVE_WINDOWS
		// make relative path
		if ( !dbName.isEmpty() && dbName.at(0) != QDir::separator())
			dbName = QDir::homePath() + QDir::separator() + dbName;
#endif

		if (!dbName.isEmpty()) {
			bool fileExists = QFile::exists(dbName);
			if (fileExists)
				ui.leDatabase->setStyleSheet("");
			else
				ui.leDatabase->setStyleSheet("QLineEdit{background:red;}");
		} else {
			ui.leDatabase->setStyleSheet("");
		}
	} else {
		ui.leDatabase->setStyleSheet("");
	}

	//don't allow to try to connect if no database name was provided
	ui.bTestConnection->setEnabled( !dbName.isEmpty() );

	m_connections[ui.lwConnections->currentRow()].dbName = dbName;
	emit changed();
}

void DatabaseManagerWidget::userNameChanged() {
	if (m_initializing)
		return;

	m_connections[ui.lwConnections->currentRow()].userName = ui.leUserName->text();
	emit changed();
}

void DatabaseManagerWidget::passwordChanged() {
	if (m_initializing)
		return;

	m_connections[ui.lwConnections->currentRow()].password = ui.lePassword->text();
	emit changed();
}

void DatabaseManagerWidget::addConnection() {
	DEBUG("Adding new connection");
	SQLConnection conn;
	conn.name = uniqueName();
	conn.driver = ui.cbDriver->currentText();
	conn.hostName = QLatin1String("localhost");

	if (!isFileDB(conn.driver))
		conn.port = defaultPort(conn.driver);

	m_connections.append(conn);
	ui.lwConnections->addItem(conn.name);
	ui.lwConnections->setCurrentRow(m_connections.size()-1);

	m_initializing = true;
	//call this to properly update the widgets for the very first added connection
	driverChanged();
	m_initializing = false;

	//we have now more then one connection, enable widgets
	ui.tbDelete->setEnabled(true);
	ui.leName->setEnabled(true);
	ui.leDatabase->setEnabled(true);
	ui.cbDriver->setEnabled(true);
	ui.leHost->setEnabled(true);
	ui.sbPort->setEnabled(true);
	ui.leUserName->setEnabled(true);
	ui.lePassword->setEnabled(true);
}

/*!
	removes the current selected connection.
 */
void DatabaseManagerWidget::deleteConnection() {
	int ret = KMessageBox::questionYesNo(this,
				i18n("Do you really want to delete the connection '%1'?", ui.lwConnections->currentItem()->text()),
				i18n("Delete Connection"));
	if (ret != KMessageBox::Yes)
		return;

	//remove the current selected connection
	m_connections.removeAt(ui.lwConnections->currentRow());
	m_initializing = true;
	QListWidgetItem* item = ui.lwConnections->takeItem(ui.lwConnections->currentRow());
	if (item) delete item;
	m_initializing = false;

	//show the connection for the item that was automatically selected afte the deletion
	connectionChanged(ui.lwConnections->currentRow());

	//disable widgets if there're no connections anymore
	if (m_connections.size() == 0) {
		m_initializing = true;
		ui.tbDelete->setEnabled(false);
		ui.bTestConnection->setEnabled(false);
		ui.leName->clear();
		ui.leName->setEnabled(false);
		ui.leDatabase->clear();
		ui.leDatabase->setEnabled(false);
		ui.cbDriver->setEnabled(false);
		ui.leHost->clear();
		ui.leHost->setEnabled(false);
		ui.sbPort->clear();
		ui.sbPort->setEnabled(false);
		ui.leUserName->clear();
		ui.leUserName->setEnabled(false);
		ui.lePassword->clear();
		ui.lePassword->setEnabled(false);
		m_initializing = false;
	}

	emit changed();
}

void DatabaseManagerWidget::loadConnections() {
	QDEBUG("Loading connections from " << m_configPath);

	m_initializing = true;

	KConfig config(m_configPath, KConfig::SimpleConfig);
	for (const auto& groupName : config.groupList()) {
		const KConfigGroup& group = config.group(groupName);
		SQLConnection conn;
		conn.name = groupName;
		conn.driver = group.readEntry("Driver","");
		conn.dbName = group.readEntry("DatabaseName", "");
		if (!isFileDB(conn.driver)) {
			conn.hostName = group.readEntry("HostName", "localhost");
			conn.port = group.readEntry("Port", defaultPort(conn.driver));
			conn.userName = group.readEntry("UserName", "root");
			conn.password = group.readEntry("Password", "");
		}

		m_connections.append(conn);
		ui.lwConnections->addItem(conn.name);
	}

	//show the first connection if available, create a new connection otherwise
	if (m_connections.size()) {
		if (!m_initConnName.isEmpty()) {
			QListWidgetItem* item = ui.lwConnections->findItems(m_initConnName, Qt::MatchExactly).constFirst();
			if (item)
				ui.lwConnections->setCurrentItem(item);
			else
				ui.lwConnections->setCurrentRow(ui.lwConnections->count()-1);
		} else {
			ui.lwConnections->setCurrentRow(ui.lwConnections->count()-1);
		}
	} else {
		addConnection();
	}

	//show/hide the driver dependent options
	driverChanged();

	m_initializing = false;

	//show the settings of the current connection
	connectionChanged(ui.lwConnections->currentRow());
}

void DatabaseManagerWidget::saveConnections() {
	QDEBUG("Saving connections to " + m_configPath);
	//delete saved connections
	KConfig config(m_configPath, KConfig::SimpleConfig);
	for (const auto& group : config.groupList())
		config.deleteGroup(group);

	//save connections
	for (const auto& conn : m_connections) {
		KConfigGroup group = config.group(conn.name);
		group.writeEntry("Driver", conn.driver);
		group.writeEntry("DatabaseName", conn.dbName);
		if (!isFileDB(conn.driver)) {
			group.writeEntry("HostName", conn.hostName);
			group.writeEntry("Port", conn.port);
			group.writeEntry("UserName", conn.userName);
			group.writeEntry("Password", conn.password);
		}
	}

	config.sync();
}

void DatabaseManagerWidget::testConnection() {
	int row = ui.lwConnections->currentRow();

	//don't allow to test the connection for file DBs if the file doesn't exist
	if (isFileDB(ui.cbDriver->currentText())) {
		QString fileName = ui.leDatabase->text();
#ifndef HAVE_WINDOWS
		// make relative path
		if ( !fileName.isEmpty() && fileName.at(0) != QDir::separator())
			fileName = QDir::homePath() + QDir::separator() + fileName;
#endif

		if (!QFile::exists(fileName)) {
			KMessageBox::error(this, i18n("Failed to connect to the database '%1'.", m_connections[row].dbName),
								 i18n("Connection Failed"));
			return;
		}
	}

	QSqlDatabase db = QSqlDatabase::addDatabase(m_connections[row].driver);
	db.setDatabaseName(m_connections[row].dbName);
	if (!isFileDB(m_connections[row].driver)) {
		db.setHostName(m_connections[row].hostName);
		db.setPort(m_connections[row].port);
		db.setUserName(m_connections[row].userName);
		db.setPassword(m_connections[row].password);
	}

	if (db.isValid() && db.open() && db.isOpen()) {
		db.close();
		KMessageBox::information(this, i18n("Connection to the database '%1' was successful.", m_connections[row].dbName),
								 i18n("Connection Successful"));
	} else {
		KMessageBox::error(this, i18n("Failed to connect to the database '%1'.", m_connections[row].dbName),
								 i18n("Connection Failed"));
	}
}

bool DatabaseManagerWidget::isFileDB(const QString& driver) {
	return (driver == QLatin1String("QSQLITE"))
			|| (driver == QLatin1String("QSQLITE3"));
}

QString DatabaseManagerWidget::uniqueName() {
	QString name = i18n("New connection");

	//TODO
	QStringList connection_names;
	for(int row = 0; row < ui.lwConnections->count(); row++)
		connection_names << ui.lwConnections->item(row)->text();

	if (!connection_names.contains(name))
		return name;

	QString base = name;
	int last_non_digit;
	for (last_non_digit = base.size()-1; last_non_digit>=0 &&
	        base[last_non_digit].category() == QChar::Number_DecimalDigit; --last_non_digit)
		base.chop(1);

	if (last_non_digit >=0 && base[last_non_digit].category() != QChar::Separator_Space)
		base.append(" ");

	int new_nr = name.rightRef(name.size() - base.size()).toInt();
	QString new_name;
	do
		new_name = base + QString::number(++new_nr);
	while (connection_names.contains(new_name));

	return new_name;
}

int DatabaseManagerWidget::defaultPort(const QString& driver) const {
	// QDB2     IBM DB2 (version 7.1 and above)
	// QIBASE   Borland InterBase
	// QMYSQL   MySQL
	// QOCI     Oracle Call Interface Driver
	// QODBC    Open Database Connectivity (ODBC) - Microsoft SQL Server and other ODBC-compliant databases
	// QPSQL    PostgreSQL (versions 7.3 and above)

	if (driver == "QDB2")
		return 50000;
	else if (driver == "QIBASE")
		return 3050;
	else if (driver == "QMYSQL3" || driver == "QMYSQL")
		return 3306;
	else if (driver == "QOCI")
		return 1521;
	else if (driver == "QODBC")
		return 1433;
	else if (driver == "QPSQL")
		return 5432;
	else
		return 0;
}
