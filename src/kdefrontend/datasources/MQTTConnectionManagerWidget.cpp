/***************************************************************************
File                 : MQTTConnectionManagerWidget.cpp
Project              : LabPlot
Description          : widget for managing MQTT connections
--------------------------------------------------------------------
Copyright            : (C) 2018 Ferencz Kovacs (kferike98@gmail.com)

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

#include "MQTTConnectionManagerWidget.h"

#ifdef HAVE_MQTT

#include "backend/lib/macros.h"

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KMessageBox>
#include <KSharedConfig>

#include <QTimer>
#include <QtMqtt>
#include <QDebug>
#include <QListWidgetItem>

/*!
   \class MQTTConnectionManagerWidget
   \brief widget for managing MQTT connections, embedded in \c MQTTConnectionManagerDialog.

   \ingroup kdefrontend
*/
MQTTConnectionManagerWidget::MQTTConnectionManagerWidget(QWidget* parent, const QString& conn) : QWidget(parent),
	m_initializing(false), m_initConnName(conn) {

	m_configPath = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).constFirst() +  "mqtt_connections";

	ui.setupUi(this);

	ui.bAdd->setIcon(QIcon::fromTheme("list-add"));
	ui.bRemove->setIcon(QIcon::fromTheme("list-remove"));
	ui.lePort->setValidator( new QIntValidator(ui.lePort) );
	ui.bAdd->setToolTip(i18n("Add new MQTT connection"));
	ui.bRemove->setToolTip(i18n("Remove selected MQTT connection"));

	//SIGNALs/SLOTs
	connect( ui.lwConnections, &QListWidget::currentRowChanged, this, &MQTTConnectionManagerWidget::connectionChanged);
	connect( ui.bAdd, &QPushButton::clicked, this, &MQTTConnectionManagerWidget::addConnection);
	connect( ui.bRemove, &QPushButton::clicked, this, &MQTTConnectionManagerWidget::deleteConnection);
	connect( ui.leHost, &QLineEdit::textChanged, this, &MQTTConnectionManagerWidget::hostChanged);
	connect( ui.lePort, &QLineEdit::textChanged, this, &MQTTConnectionManagerWidget::portChanged);
	connect( ui.leUserName, &QLineEdit::textChanged, this, &MQTTConnectionManagerWidget::userNameChanged);
	connect( ui.lePassword, &QLineEdit::textChanged, this, &MQTTConnectionManagerWidget::passwordChanged);
	connect(ui.leID, &QLineEdit::textChanged, this, &MQTTConnectionManagerWidget::clientIdChanged);
	connect(ui.chbAuthentication, &QCheckBox::stateChanged, this, &MQTTConnectionManagerWidget::authenticationChecked);
	connect(ui.chbID, &QCheckBox::stateChanged, this, &MQTTConnectionManagerWidget::idChecked);

	ui.lePassword->hide();
	ui.lPassword->hide();
	ui.lePassword->setStyleSheet("QLineEdit{background: red;}");
	ui.lePassword->setToolTip("Please set a password");
	ui.leUserName->hide();
	ui.lUsername->hide();
	ui.leUserName->setStyleSheet("QLineEdit{background: red;}");
	ui.leUserName->setToolTip("Please set a username");
	ui.leID->hide();
	ui.lID->hide();
	ui.leID->setStyleSheet("QLineEdit{background: red;}");
	ui.leID->setToolTip("Please set a client ID");

	QTimer::singleShot( 100, this, SLOT(loadConnections()) );
}

/*!
 * \brief Returns the currently selected connection's name
 */
QString MQTTConnectionManagerWidget::connection() const {
	if (ui.lwConnections->currentItem())
		return ui.lwConnections->currentItem()->text();
	else
		return QString();
}

/*!
		shows the settings of the currently selected connection
 */
void MQTTConnectionManagerWidget::connectionChanged(int index) {
	if (m_initializing)
		return;

	if (index == -1)
		return;

	//show the settings for the selected connection
	m_initializing = true;

	ui.leHost->setText(m_connections[index].hostName);
	ui.lePort->setText(QString::number(m_connections[index].port));

	if(m_connections[index].useAuthentication) {
		ui.chbAuthentication->setChecked(true);
		ui.leUserName->setText(m_connections[index].userName);
		ui.lePassword->setText(m_connections[index].password);
	} else
		ui.chbAuthentication->setChecked(false);

	if(m_connections[index].useID) {
		ui.chbID->setChecked(true);
		ui.leID->setText(m_connections[index].clientID);
	} else
		ui.chbID->setChecked(false);

	m_initializing = false;
}

/*!
 * \brief Called when the host name is changed
 * Sets the host name for the current connection
 */
void MQTTConnectionManagerWidget::hostChanged(const QString& hostName) {
	if(hostName.isEmpty()) {
		ui.leHost->setStyleSheet("QLineEdit{background: red;}");
		ui.leHost->setToolTip("Please set a valid host name");
	} else {
		ui.leHost->setStyleSheet("");
		ui.leHost->setToolTip("");

		if(hostName.contains("New host", Qt::CaseInsensitive)) {
			ui.leHost->setStyleSheet("QLineEdit{background: red;}");
			ui.leHost->setToolTip("Please set a valid host name");
		} else {
			ui.leHost->setStyleSheet("");
			ui.leHost->setToolTip("");

			//check uniqueness of the provided name
			bool unique = true;
			for(int i = 0; i < ui.lwConnections->count(); ++i) {
				if (ui.lwConnections->currentRow() == i)
					continue;

				if (hostName == ui.lwConnections->item(i)->text()) {
					unique = false;
					break;
				}
			}

			if (!unique) {
				ui.leHost->setStyleSheet("QLineEdit{background: red;}");
				ui.leHost->setToolTip("There can't be more identical hostnames");
			} else {
				ui.leHost->setStyleSheet("");
				ui.leHost->setToolTip("");

				if (m_initializing)
					return;

				ui.lwConnections->currentItem()->setText(hostName);
				if (!m_initializing) {
					m_connections[ui.lwConnections->currentRow()].hostName = hostName;
					emit changed();
				}
			}
		}
	}
}

/*!
 * \brief Called when the port is changed
 * Sets the port for the current connection
 */
void MQTTConnectionManagerWidget::portChanged() {
	if (m_initializing)
		return;

	m_connections[ui.lwConnections->currentRow()].port = ui.lePort->text().simplified().toUInt();
	emit changed();
}

/*!
 *\brief called when authentication checkbox's state is changed,
 *       if checked two lineEdits are shown so the user can set the username and password
 *
 * \param state the state of the checbox
 */
void MQTTConnectionManagerWidget::authenticationChecked(int state) {
	if (state == Qt::CheckState::Checked) {
		ui.lPassword->show();
		ui.lePassword->show();
		ui.lUsername->show();
		ui.leUserName->show();

		if(m_connections.size() > 0) {
			ui.lePassword->setText(m_connections[ui.lwConnections->currentRow()].password);
			ui.leUserName->setText(m_connections[ui.lwConnections->currentRow()].userName);
		}

		if(!m_initializing) {
			m_connections[ui.lwConnections->currentRow()].useAuthentication = true;
		}

	} else if (state == Qt::CheckState::Unchecked) {
		ui.lPassword->hide();
		ui.lePassword->hide();
		ui.lePassword->clear();
		ui.lUsername->hide();
		ui.leUserName->hide();
		ui.leUserName->clear();

		if(!m_initializing) {
			m_connections[ui.lwConnections->currentRow()].useAuthentication = false;
		}
	}

	if(!m_initializing)
		emit changed();
}

/*!
 *\brief called when ID checkbox's state is changed, if checked a lineEdit is shown so the user can set the ID
 * \param state the state of the checbox
 */
void MQTTConnectionManagerWidget::idChecked(int state) {
	if (state == Qt::CheckState::Checked) {
		ui.lID->show();
		ui.leID->show();

		if(m_connections.size() > 0) {
			ui.leID->setText(m_connections[ui.lwConnections->currentRow()].clientID);
		}

		if(!m_initializing) {
			m_connections[ui.lwConnections->currentRow()].useID = true;
		}

	} else if (state == Qt::CheckState::Unchecked) {
		ui.lID->hide();
		ui.leID->hide();
		ui.leID->clear();

		if(!m_initializing) {
			m_connections[ui.lwConnections->currentRow()].useID = false;
		}
	}

	if(!m_initializing)
		emit changed();
}

/*!
 * \brief Called when the username is changed
 * Sets the username for the current connection
 */
void MQTTConnectionManagerWidget::userNameChanged(const QString& userName) {
	if(userName.isEmpty()) {
		ui.leUserName->setStyleSheet("QLineEdit{background: red;}");
		ui.leUserName->setToolTip("Please set a username");
	} else {
		ui.leUserName->setStyleSheet("");
		ui.leUserName->setToolTip("");
	}

	if (m_initializing)
		return;

	m_connections[ui.lwConnections->currentRow()].userName = userName;
	emit changed();
}

/*!
 * \brief Called when the password is changed
 * Sets the password for the current connection
 */
void MQTTConnectionManagerWidget::passwordChanged(const QString& password) {
	if(password.isEmpty()) {
		ui.lePassword->setStyleSheet("QLineEdit{background: red;}");
		ui.lePassword->setToolTip("Please set a password");
	} else {
		ui.lePassword->setStyleSheet("");
		ui.lePassword->setToolTip("");
	}

	if (m_initializing)
		return;

	m_connections[ui.lwConnections->currentRow()].password = password;
	emit changed();
}

/*!
 * \brief Called when the client ID is changed
 * Sets the client ID for the current connection
 */
void MQTTConnectionManagerWidget::clientIdChanged(const QString& clientID) {
	if(clientID.isEmpty()) {
		ui.leID->setStyleSheet("QLineEdit{background: red;}");
		ui.leID->setToolTip("Please set a client ID");
	} else {
		ui.leID->setStyleSheet("");
		ui.leID->setToolTip("");
	}

	if(m_initializing)
		return;

	m_connections[ui.lwConnections->currentRow()].clientID = clientID;
	emit changed();
}

/*!
		adds a new sample connection to the end of the list.
 */
void MQTTConnectionManagerWidget::addConnection() {
	qDebug() << "Adding new connection";
	MQTTConnection conn;
	conn.hostName = uniqueName();
	conn.port = 1883;
	conn.useAuthentication = false;
	conn.useID = false;

	m_connections.append(conn);
	ui.lwConnections->addItem(conn.hostName);
	ui.lwConnections->setCurrentRow(m_connections.size()-1);

	//we have now more than one connection, enable widgets
	ui.bRemove->setEnabled(true);
	ui.leHost->setEnabled(true);
	ui.lePort->setEnabled(true);
	ui.leUserName->setEnabled(true);
	ui.lePassword->setEnabled(true);
	ui.leID->setEnabled(true);

	emit changed();
}

/*!
		removes the current selected connection.
 */
void MQTTConnectionManagerWidget::deleteConnection() {
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

	//show the connection for the item that was automatically selected after the deletion
	connectionChanged(ui.lwConnections->currentRow());

	//disable widgets if there are no connections anymore
	if (m_connections.size() == 0) {
		m_initializing = true;
		ui.bRemove->setEnabled(false);
		ui.leHost->clear();
		ui.leHost->setEnabled(false);
		ui.lePort->clear();
		ui.lePort->setEnabled(false);
		ui.leUserName->clear();
		ui.leUserName->setEnabled(false);
		ui.lePassword->clear();
		ui.lePassword->setEnabled(false);
		ui.leID->clear();
		ui.leID->setEnabled(false);

		m_initializing = false;
	}

	emit changed();
}

/*!
		Loads the saved connections.
 */
void MQTTConnectionManagerWidget::loadConnections() {
	qDebug() << "Loading connections from " << m_configPath;

	m_initializing = true;

	KConfig config(m_configPath, KConfig::SimpleConfig);
	for (const auto& groupName : config.groupList()) {
		const KConfigGroup& group = config.group(groupName);
		MQTTConnection conn;
		conn.hostName = groupName;
		conn.port = group.readEntry("Port", 0);

		conn.useAuthentication = group.readEntry("UseAuthentication", false);
		if(conn.useAuthentication) {
			conn.userName = group.readEntry("UserName", "");
			conn.password = group.readEntry("Password", "");
		}

		conn.useID = group.readEntry("UseID", false);
		if(conn.useID) {
			conn.clientID = group.readEntry("ClientID", "");
		}

		m_connections.append(conn);
		ui.lwConnections->addItem(conn.hostName);
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

	m_initializing = false;

	//show the settings of the current connection
	connectionChanged(ui.lwConnections->currentRow());
}

/*!
		Saves the connections present in the list widget.
 */
void MQTTConnectionManagerWidget::saveConnections() {
	qDebug() << "Saving connections to " << m_configPath;
	//delete saved connections
	KConfig config(m_configPath, KConfig::SimpleConfig);
	for (const auto& group : config.groupList())
		config.deleteGroup(group);

	//save connections
	for (const auto& conn : m_connections) {
		KConfigGroup group = config.group(conn.hostName);
		group.writeEntry("Port", conn.port);
		group.writeEntry("UseAuthentication", QString::number(conn.useAuthentication));
		group.writeEntry("UserName", conn.userName);
		group.writeEntry("Password", conn.password);
		group.writeEntry("UseID", QString::number(conn.useID));
		group.writeEntry("ClientID", conn.clientID);
	}

	config.sync();
}

/*!
 * \brief Checks whether every connection's settings are alright or not
 */
bool MQTTConnectionManagerWidget::checkConnections() {
	if(m_connections.isEmpty())
		return false;

	bool connectionsOk = true;

	for(int i = 0; i < m_connections.size(); ++i) {
		bool authenticationUsed = m_connections[i].useAuthentication;
		bool idUsed = m_connections[i].useID;
		bool authenticationFilled = !m_connections[i].userName.isEmpty() && !m_connections[i].password.isEmpty();
		bool idFilled = !m_connections[i].clientID.isEmpty();
		bool authenticationOK = !authenticationUsed || (authenticationUsed && authenticationFilled);
		bool idOK = !idUsed || (idUsed && idFilled);

		QList<QListWidgetItem*> equalHosts = ui.lwConnections->findItems(m_connections[i].hostName, Qt::MatchExactly);
		bool hostOK = (!m_connections[i].hostName.isEmpty()) && (!m_connections[i].hostName.contains("New host", Qt::CaseInsensitive)) && (equalHosts.size() == 1);
		bool allOk = authenticationOK && idOK && hostOK;

		if(!allOk) {
			connectionsOk = false;
			ui.lwConnections->item(i)->setBackground(QBrush(Qt::red));
		} else
			ui.lwConnections->item(i)->setBackground(QBrush(Qt::white));
	}
	return connectionsOk;
}

/*!
		Provides a sample host name, which has to be changed before use.
 */
QString MQTTConnectionManagerWidget::uniqueName() {
	QString name = i18n("New host");

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

#endif
