/***************************************************************************
File                 : DatabaseManagerWidget.cpp
Project              : LabPlot
Description          : widget for managing database connections
--------------------------------------------------------------------
Copyright            : (C) 2016 Alexander Semke (alexander.semke@web.de)

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

#include <KConfig>
#include <KConfigGroup>
#include <KStandardDirs>

#include <QFileDialog>
#include <QTimer>
#include <QSqlDatabase>
#include <QtSql>
#include <QDebug>

/*!
   \class DatabaseManagerWidget
   \brief widget for managing database connections, embedded in \c DatabaseManagerDialog.

   \ingroup kdefrontend
*/

DatabaseManagerWidget::DatabaseManagerWidget(QWidget* parent) : QWidget(parent), m_initializing(false) {
	ui.setupUi(this);

	ui.tbAdd->setIcon(KIcon("list-add"));
	ui.tbDelete->setIcon(KIcon("list-remove"));
	ui.bOpen->setIcon(KIcon("document-open"));

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

	connect( ui.leName, SIGNAL(returnPressed()), this, SLOT(nameChanged()) );
	connect( ui.kleDatabase, SIGNAL(textChanged(QString)), this, SLOT(databaseNameChanged()) );
	connect( ui.leHost, SIGNAL(textChanged(QString)), this, SLOT(hostChanged()) );
	connect( ui.sbPort, SIGNAL(valueChanged(int)), this, SLOT(portChanged()) );
	connect( ui.leUserName, SIGNAL(textChanged(QString)), this, SLOT(userNameChanged()) );
	connect( ui.lePassword, SIGNAL(textChanged(QString)), this, SLOT(passwordChanged()) );

	QTimer::singleShot( 100, this, SLOT(loadSettings()) );
	loadConnection();
	driverChanged();
}

/*!
	read and show all available database connections
 */
void DatabaseManagerWidget::loadSettings() {
//	KConfigGroup conf(KSharedConfig::openConfig(), QLatin1String("DatabaseManager"));

//    ui.leName->setText( conf.readEntry("Name", "") );
//    ui.cbDriver->setCurrentIndex( conf.readEntry( "VendorIndex", 0) );
//    ui.kleDatabase->setText( conf.readEntry( "DatabaseName", "") );

//    QString driver =  QSqlDatabase::drivers().at(ui.cbDriver->currentIndex());
//    bool fileDB = driver == QLatin1String("QSQLITE") ||
//            driver == QLatin1String("QSQLITE3");
//    if (!fileDB) {
//        ui.leHost->setText( conf.readEntry( "HostName", "127.0.0.1") );
//        ui.sbPort->setValue( conf.readEntry( "Port", 3306) );
//        ui.leUserName->setText( conf.readEntry("UserName", "root") );
//        ui.lePassword->setText( conf.readEntry("Password", "") );
//    }
}

DatabaseManagerWidget::~DatabaseManagerWidget() {
	// save available database connections
	saveConnections();
//	KConfigGroup conf(KSharedConfig::openConfig(), QLatin1String("DatabaseManager"));
//    conf.writeEntry( "Name", ui.leName->text() );
//    conf.writeEntry( "VendorIndex", ui.cbDriver->currentIndex() );
//    conf.writeEntry( "DatabaseName", ui.kleDatabase->text() );

//    QString driver =  QSqlDatabase::drivers().at(ui.cbDriver->currentIndex());
//    bool fileDB = driver == QLatin1String("QSQLITE") ||
//            driver == QLatin1String("QSQLITE3");
//    if (!fileDB) {
//        conf.writeEntry( "HostName", ui.leHost->text() );
//        conf.writeEntry( "Port", ui.sbPort->value() );
//        conf.writeEntry( "UserName", ui.leUserName->text() );
//        conf.writeEntry( "Password", ui.lePassword->text() );
//    }
}

void DatabaseManagerWidget::driverChanged() {
	//determine whether it's a file database (like SQLite) and hide non-relevant fields (like host name, etc.)
	bool fileDB = ui.cbDriver->currentText() == QLatin1String("QSQLITE") ||
	              ui.cbDriver->currentText() == QLatin1String("QSQLITE3");
	ui.lHost->setVisible(!fileDB);
	ui.leHost->setVisible(!fileDB);
	ui.lPort->setVisible(!fileDB);
	ui.sbPort->setVisible(!fileDB);
	ui.bOpen->setVisible(fileDB);
	ui.gbAuthentication->setVisible(!fileDB);

	if (m_initializing || ui.lwConnections->currentRow() == -1)
		return;

	connectionList[ui.lwConnections->currentRow()]->vendorIndex = ui.cbDriver->currentIndex();
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

	ui.kleDatabase->setText(path);
}

/*!
	shows the settings of the currently selected connection
 */
void DatabaseManagerWidget::connectionChanged(int index) {
	if (m_initializing || index == -1)
		return;

	ui.leName->setText( connectionList[index]->name );
	ui.cbDriver->setCurrentIndex( connectionList[index]->vendorIndex );
	ui.kleDatabase->setText( connectionList[index]->dbName );

	QString driver =  QSqlDatabase::drivers().at(connectionList[index]->vendorIndex);
	bool fileDB = driver == QLatin1String("QSQLITE") ||
	              driver == QLatin1String("QSQLITE3");
	if (!fileDB) {
		ui.leHost->setText( connectionList[index]->hostName );
		ui.sbPort->setValue( connectionList[index]->port );
		ui.leUserName->setText( connectionList[index]->userName );
		ui.lePassword->setText( connectionList[index]->password );
	}
}

void DatabaseManagerWidget::testConnection() {
	if (ui.lwConnections->currentRow() == -1)
		return;

	int row = ui.lwConnections->currentRow();
	QSqlDatabase db = QSqlDatabase::addDatabase( QSqlDatabase::drivers().at(connectionList[row]->vendorIndex) );
	db.setDatabaseName(connectionList[row]->dbName);
	QString driver =  QSqlDatabase::drivers().at(connectionList[row]->vendorIndex);
	bool fileDB = driver == QLatin1String("QSQLITE") ||
	              driver == QLatin1String("QSQLITE3");
	if (!fileDB) {
		db.setHostName(connectionList[row]->hostName);
		db.setPort(connectionList[row]->port);
		db.setUserName(connectionList[row]->userName);
		db.setPassword(connectionList[row]->password);
	}

	if (db.isValid() && db.open() && db.isOpen()) {
		qDebug() << "connected";
		db.close();
	} else
		qDebug() << db.lastError().text();
}

void DatabaseManagerWidget::nameChanged() {
	if (m_initializing || ui.lwConnections->currentRow() == -1)
		return;

	QString new_name = uniqueName();
	m_initializing = true;
	ui.leName->setText(new_name);
	m_initializing = false;
	ui.lwConnections->currentItem()->setText(new_name);
	connectionList[ui.lwConnections->currentRow()]->name = uniqueName();
}

void DatabaseManagerWidget::hostChanged() {
	if (m_initializing || ui.lwConnections->currentRow() == -1)
		return;

	connectionList[ui.lwConnections->currentRow()]->hostName = ui.leHost->text();
}

void DatabaseManagerWidget::portChanged() {
	if (m_initializing || ui.lwConnections->currentRow() == -1)
		return;

	connectionList[ui.lwConnections->currentRow()]->port = ui.sbPort->value();
}

void DatabaseManagerWidget::databaseNameChanged() {
	if (m_initializing || ui.lwConnections->currentRow() == -1)
		return;

	connectionList[ui.lwConnections->currentRow()]->dbName = ui.kleDatabase->text();
}

void DatabaseManagerWidget::userNameChanged() {
	if (m_initializing || ui.lwConnections->currentRow() == -1)
		return;

	connectionList[ui.lwConnections->currentRow()]->userName = ui.leUserName->text();
}

void DatabaseManagerWidget::passwordChanged() {
	if (m_initializing || ui.lwConnections->currentRow() == -1)
		return;

	connectionList[ui.lwConnections->currentRow()]->password = ui.lePassword->text();
}

void DatabaseManagerWidget::addConnection() {
	SQLConnection* new_conn = new SQLConnection;
	new_conn->name = uniqueName();
	new_conn->vendorIndex = ui.cbDriver->currentIndex();
	new_conn->dbName = ui.kleDatabase->text();

	bool fileDB = ui.cbDriver->currentText() == QLatin1String("QSQLITE") ||
	              ui.cbDriver->currentText() == QLatin1String("QSQLITE3");
	if (!fileDB) {
		new_conn->hostName = ui.leHost->text();
		new_conn->port = ui.sbPort->value();
		new_conn->userName = ui.leUserName->text();
		new_conn->password = ui.lePassword->text();
	}

	m_initializing = true;
	connectionList.append(new_conn);
	ui.lwConnections->addItem(new_conn->name);
	ui.lwConnections->setCurrentRow(ui.lwConnections->count()-1);
	ui.leName->setText(new_conn->name);
	m_initializing = false;
}

void DatabaseManagerWidget::deleteConnection() {
	int row = ui.lwConnections->currentRow();
	if (row == -1)
		return;

	SQLConnection* conn = connectionList[row];
	connectionList.erase(connectionList.begin() + row);
	delete conn;
	m_initializing = true;
	delete ui.lwConnections->item(row);
	m_initializing = false;
	connectionChanged(ui.lwConnections->currentRow());
}

QString DatabaseManagerWidget::uniqueName() {
	QString current_name = ui.leName->text();
	QStringList connection_names;
	for(int row = 0; row < ui.lwConnections->count(); row++)
		connection_names << ui.lwConnections->item(row)->text();

	if (!connection_names.contains(current_name))
		return current_name;

	QString base = current_name;
	int last_non_digit;
	for (last_non_digit = base.size()-1; last_non_digit>=0 &&
	        base[last_non_digit].category() == QChar::Number_DecimalDigit; --last_non_digit)
		base.chop(1);

	if (last_non_digit >=0 && base[last_non_digit].category() != QChar::Separator_Space)
		base.append(" ");

	int new_nr = current_name.right(current_name.size() - base.size()).toInt();
	QString new_name;
	do
		new_name = base + QString::number(++new_nr);
	while (connection_names.contains(new_name));

	return new_name;
}

void DatabaseManagerWidget::loadConnection() {
	QStringList list = KGlobal::dirs()->findAllResources("appdata", "templates/sql connections/*");
	for (int i = 0; i < list.size(); ++i) {
		KConfig config(list.at(i), KConfig::SimpleConfig);
		KConfigGroup group = config.group( "SQLConnection" );

		QFileInfo fileinfo(list.at(i));
		SQLConnection* new_conn = new SQLConnection;
		new_conn->name = fileinfo.fileName();
		new_conn->vendorIndex = group.readEntry("VendorIndex", 0);
		new_conn->dbName = group.readEntry( "DatabaseName", "" );

		QString driver =  QSqlDatabase::drivers().at(new_conn->vendorIndex);
		bool fileDB = driver == QLatin1String("QSQLITE") ||
		              driver == QLatin1String("QSQLITE3");
		if (!fileDB) {
			new_conn->hostName = group.readEntry("HostName", "127.0.0.1");
			new_conn->port = group.readEntry("Port", 3306);
			new_conn->userName = group.readEntry("UserName", "root");
			new_conn->password = group.readEntry("Password", "");
		}

		m_initializing = true;
		connectionList.append(new_conn);
		ui.lwConnections->addItem(new_conn->name);
		ui.lwConnections->setCurrentRow(ui.lwConnections->count()-1);
		m_initializing = false;
	}

	connectionChanged(ui.lwConnections->currentRow());
}

void DatabaseManagerWidget::saveConnections() {
	//delete saved connections
	QStringList list = KGlobal::dirs()->findAllResources("appdata", "templates/sql connections/*");
	for (int i = 0; i < list.size(); ++i)
		QFile::remove(list.at(i));

	QString template_dir = KGlobal::dirs()->locateLocal("appdata", "templates") + "/sql connections/";
	foreach(SQLConnection* conn, connectionList) {
		KConfig config(template_dir + conn->name, KConfig::SimpleConfig);
		KConfigGroup group = config.group( "SQLConnection" );
		group.writeEntry("VendorIndex", conn->vendorIndex);
		group.writeEntry("DatabaseName", conn->dbName);

		QString driver =  QSqlDatabase::drivers().at(conn->vendorIndex);
		bool fileDB = driver == QLatin1String("QSQLITE") ||
		              driver == QLatin1String("QSQLITE3");
		if (!fileDB) {
			group.writeEntry( "HostName", conn->hostName );
			group.writeEntry( "Port", conn->port );
			group.writeEntry( "UserName", conn->userName );
			group.writeEntry( "Password", conn->password );
		}
	}
}
