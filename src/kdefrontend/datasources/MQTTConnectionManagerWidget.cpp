/*
	File                 : MQTTConnectionManagerWidget.cpp
	Project              : LabPlot
	Description          : widget for managing MQTT connections
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2018 Ferencz Kovacs <kferike98@gmail.com>
	SPDX-FileCopyrightText: 2018-2024 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "MQTTConnectionManagerWidget.h"
#include "backend/lib/macros.h"

#include <QListWidgetItem>
#include <QTimer>
#include <QtMqtt>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KMessageBox>

/*!
   \class MQTTConnectionManagerWidget
   \brief widget for managing MQTT connections, embedded in \c MQTTConnectionManagerDialog.

   \ingroup kdefrontend
*/
MQTTConnectionManagerWidget::MQTTConnectionManagerWidget(QWidget* parent, const QString& conn)
	: QWidget(parent)
	, m_initConnName(conn) {
	ui.setupUi(this);

	m_configPath = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).constFirst() + QStringLiteral("MQTT_connections");

	ui.lePort->setValidator(new QIntValidator(ui.lePort));
	ui.bAdd->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
	ui.bRemove->setIcon(QIcon::fromTheme(QStringLiteral("list-remove")));
	ui.bAdd->setToolTip(i18n("Add new MQTT connection"));
	ui.bRemove->setToolTip(i18n("Remove selected MQTT connection"));
	ui.bTest->setIcon(QIcon::fromTheme(QStringLiteral("network-connect")));

	// SIGNALs/SLOTs
	connect(ui.leName, &QLineEdit::textChanged, this, &MQTTConnectionManagerWidget::nameChanged);
	connect(ui.lwConnections, &QListWidget::currentRowChanged, this, &MQTTConnectionManagerWidget::connectionChanged);
	connect(ui.bAdd, &QPushButton::clicked, this, &MQTTConnectionManagerWidget::addConnection);
	connect(ui.bRemove, &QPushButton::clicked, this, &MQTTConnectionManagerWidget::deleteConnection);
	connect(ui.leHost, &QLineEdit::textChanged, this, &MQTTConnectionManagerWidget::hostChanged);
	connect(ui.lePort, &QLineEdit::textChanged, this, &MQTTConnectionManagerWidget::portChanged);
	connect(ui.leUserName, &QLineEdit::textChanged, this, &MQTTConnectionManagerWidget::userNameChanged);
	connect(ui.lePassword, &QLineEdit::textChanged, this, &MQTTConnectionManagerWidget::passwordChanged);
	connect(ui.leID, &QLineEdit::textChanged, this, &MQTTConnectionManagerWidget::clientIdChanged);
	connect(ui.chbAuthentication, &QCheckBox::toggled, this, &MQTTConnectionManagerWidget::authenticationChecked);
	connect(ui.chbID, &QCheckBox::toggled, this, &MQTTConnectionManagerWidget::idChecked);
	connect(ui.chbRetain, &QCheckBox::toggled, this, &MQTTConnectionManagerWidget::retainChecked);
	connect(ui.bTest, &QPushButton::clicked, this, &MQTTConnectionManagerWidget::testConnection);

	ui.lePassword->hide();
	ui.lPassword->hide();
	ui.lePassword->setToolTip(i18n("Please set a password."));
	ui.leUserName->hide();
	ui.lUsername->hide();
	ui.leUserName->setToolTip(i18n("Please set a username."));
	ui.leID->hide();
	ui.lID->hide();
	ui.leID->setToolTip(i18n("Please set a client ID."));
	ui.leHost->setToolTip(i18n("Please set a valid host name."));
	ui.leHost->setToolTip(i18n("Please set a valid name."));

	QTimer::singleShot(100, this, SLOT(loadConnections()));
}

MQTTConnectionManagerWidget::~MQTTConnectionManagerWidget() {
	delete m_client;
}

/*!
 * \brief Returns the currently selected connection's name
 */
QString MQTTConnectionManagerWidget::connection() const {
	if (ui.lwConnections->currentItem())
		return ui.lwConnections->currentItem()->text();

	return {};
}

/*!
 * \brief Shows the settings of the currently selected connection
 * \param index the index of the new connection
 */
void MQTTConnectionManagerWidget::connectionChanged(int index) {
	CONDITIONAL_RETURN_NO_LOCK;

	if (index == -1) {
		m_currentConnection = nullptr;
		return;
	}

	m_initializing = true;
	m_currentConnection = &m_connections[index];

	// show the settings for the selected connection
	ui.leName->setText(m_currentConnection->name);
	ui.leHost->setText(m_currentConnection->hostName);
	ui.lePort->setText(QString::number(m_currentConnection->port));

	if (m_currentConnection->useAuthentication) {
		ui.chbAuthentication->setChecked(true);
		ui.leUserName->setText(m_currentConnection->userName);
		ui.lePassword->setText(m_currentConnection->password);
	} else
		ui.chbAuthentication->setChecked(false);

	if (m_currentConnection->useID) {
		ui.chbID->setChecked(true);
		ui.leID->setText(m_currentConnection->clientID);
	} else
		ui.chbID->setChecked(false);

	ui.chbRetain->setChecked(m_currentConnection->retain);

	m_initializing = false;
}

/*!
 * \brief Called when the name is changed
 * Sets the name for the current connection
 */
void MQTTConnectionManagerWidget::nameChanged(const QString& name) {
	if (name.isEmpty()) {
		SET_WARNING_STYLE(ui.leName)
		ui.leHost->setToolTip(i18n("Please set a valid name."));
	} else {
		// check uniqueness of the provided name
		bool unique = true;
		for (int i = 0; i < ui.lwConnections->count(); ++i) {
			if (ui.lwConnections->currentRow() == i)
				continue;

			if (name == ui.lwConnections->item(i)->text()) {
				unique = false;
				break;
			}
		}

		if (unique) {
			ui.leName->setStyleSheet(QString());
			ui.leName->setToolTip(QString());
			ui.lwConnections->currentItem()->setText(name);

			if (!m_initializing) {
				m_currentConnection->name = name;
				Q_EMIT changed();
			}
		} else {
			SET_WARNING_STYLE(ui.leName)
			ui.leHost->setToolTip(i18n("Please provide a unique name."));
		}
	}
}

/*!
 * \brief Called when the host name is changed
 * Sets the host name for the current connection
 */
void MQTTConnectionManagerWidget::hostChanged(const QString& hostName) {
	if (hostName.isEmpty()) {
		SET_WARNING_STYLE(ui.leHost)
		ui.leHost->setToolTip(i18n("Please set a valid host name."));
	} else {
		m_currentConnection->hostName = hostName;
		// check uniqueness of the provided host name
		bool unique = true;
		for (auto& c : m_connections) {
			if (m_currentConnection == &c)
				continue;

			if (m_currentConnection->hostName == c.hostName && m_currentConnection->port == c.port) {
				unique = false;
				break;
			}
		}

		if (!unique) {
			SET_WARNING_STYLE(ui.leHost)
			SET_WARNING_STYLE(ui.lePort)
			ui.leHost->setToolTip(i18n("Host name and port must be unique."));
			ui.lePort->setToolTip(i18n("Host name and port must be unique."));
		} else {
			ui.leHost->setStyleSheet(QString());
			ui.lePort->setStyleSheet(QString());
			ui.leHost->setToolTip(QString());
			ui.lePort->setToolTip(QString());

			if (!m_initializing)
				Q_EMIT changed();
		}
	}
}

/*!
 * \brief Called when the port is changed
 * Sets the port for the current connection
 */
void MQTTConnectionManagerWidget::portChanged(const QString& portString) {
	if (portString.isEmpty()) {
		SET_WARNING_STYLE(ui.leHost)
		ui.leHost->setToolTip(i18n("Please set a valid port."));
	} else {
		m_currentConnection->port = portString.simplified().toInt();
		// check uniqueness of the provided host name
		bool unique = true;
		for (auto& c : m_connections) {
			if (m_currentConnection == &c)
				continue;

			if (m_currentConnection->hostName == c.hostName && m_currentConnection->port == c.port) {
				unique = false;
				break;
			}
		}

		if (!unique) {
			SET_WARNING_STYLE(ui.leHost)
			SET_WARNING_STYLE(ui.lePort)
			ui.leHost->setToolTip(i18n("Host name and port must be unique."));
			ui.lePort->setToolTip(i18n("Host name and port must be unique."));
		} else {
			ui.leHost->setStyleSheet(QString());
			ui.leHost->setToolTip(QString());
			ui.lePort->setStyleSheet(QString());
			ui.lePort->setToolTip(QString());

			if (!m_initializing)
				Q_EMIT changed();
		}
	}
}

/*!
 *\brief Called when authentication checkbox's state is changed,
 *       if checked two lineEdits are shown so the user can set the username and password
 *
 * \param state the state of the checkbox
 */
void MQTTConnectionManagerWidget::authenticationChecked(bool state) {
	if (state) {
		ui.lPassword->show();
		ui.lePassword->show();
		ui.lUsername->show();
		ui.leUserName->show();

		if (m_currentConnection) {
			ui.lePassword->setText(m_currentConnection->password);
			ui.leUserName->setText(m_currentConnection->userName);
			if (!m_initializing)
				m_currentConnection->useAuthentication = true;
		}
	} else {
		ui.lPassword->hide();
		ui.lePassword->hide();
		ui.lePassword->clear();
		ui.lUsername->hide();
		ui.leUserName->hide();
		ui.leUserName->clear();

		if (m_currentConnection && !m_initializing)
			m_currentConnection->useAuthentication = false;
	}

	if (!m_initializing)
		Q_EMIT changed();
}

/*!
 *\brief called when ID checkbox's state is changed, if checked a lineEdit is shown so the user can set the ID
 * \param state the state of the checkbox
 */
void MQTTConnectionManagerWidget::idChecked(bool state) {
	if (state) {
		ui.lID->show();
		ui.leID->show();

		if (m_currentConnection) {
			ui.leID->setText(m_currentConnection->clientID);
			if (!m_initializing)
				m_currentConnection->useID = true;
		}

	} else {
		ui.lID->hide();
		ui.leID->hide();
		ui.leID->clear();

		if (m_currentConnection && !m_initializing) {
			m_currentConnection->useID = false;
		}
	}

	if (!m_initializing)
		Q_EMIT changed();
}

/*!
 * \brief called when retain checkbox's state is changed
 * \param state the state of the checkbox
 */
void MQTTConnectionManagerWidget::retainChecked(bool state) {
	CONDITIONAL_RETURN_NO_LOCK;

	if (m_currentConnection) {
		if (state)
			m_currentConnection->retain = true;
		else
			m_currentConnection->retain = false;
	}
	Q_EMIT changed();
}

/*!
 * \brief Called when the username is changed
 * Sets the username for the current connection
 */
void MQTTConnectionManagerWidget::userNameChanged(const QString& userName) {
	if (userName.isEmpty()) {
		SET_WARNING_STYLE(ui.leUserName)
		ui.leUserName->setToolTip(i18n("Please set a username."));
	} else {
		ui.leUserName->setStyleSheet(QString());
		ui.leUserName->setToolTip(QString());
	}

	CONDITIONAL_RETURN_NO_LOCK;

	if (m_currentConnection)
		m_currentConnection->userName = userName;
	Q_EMIT changed();
}

/*!
 * \brief Called when the password is changed
 * Sets the password for the current connection
 */
void MQTTConnectionManagerWidget::passwordChanged(const QString& password) {
	if (password.isEmpty()) {
		SET_WARNING_STYLE(ui.lePassword)
		ui.lePassword->setToolTip(i18n("Please set a password."));
	} else {
		ui.lePassword->setStyleSheet(QString());
		ui.lePassword->setToolTip(QString());
	}

	CONDITIONAL_RETURN_NO_LOCK;

	if (m_currentConnection)
		m_currentConnection->password = password;
	Q_EMIT changed();
}

/*!
 * \brief Called when the client ID is changed
 * Sets the client ID for the current connection
 */
void MQTTConnectionManagerWidget::clientIdChanged(const QString& clientID) {
	if (clientID.isEmpty()) {
		SET_WARNING_STYLE(ui.leID)
		ui.leID->setToolTip(i18n("Please set a client ID."));
	} else {
		ui.leID->setStyleSheet(QString());
		ui.leID->setToolTip(QString());
	}

	CONDITIONAL_RETURN_NO_LOCK;

	if (m_currentConnection)
		m_currentConnection->clientID = clientID;
	Q_EMIT changed();
}

/*!
	adds a new sample connection to the end of the list.
 */
void MQTTConnectionManagerWidget::addConnection() {
	MQTTConnection conn;
	conn.name = uniqueName();
	conn.hostName = QStringLiteral("localhost");

	m_connections.append(conn);
	m_currentConnection = &m_connections.back();
	ui.lwConnections->addItem(conn.hostName);
	ui.lwConnections->setCurrentRow(m_connections.size() - 1);

	// we have now more than one connection, enable widgets
	ui.bRemove->setEnabled(true);
	ui.leHost->setEnabled(true);
	ui.lePort->setEnabled(true);
	ui.leUserName->setEnabled(true);
	ui.lePassword->setEnabled(true);
	ui.leID->setEnabled(true);
	ui.leName->setEnabled(true);
	Q_EMIT changed();
}

/*!
	removes the current selected connection.
 */
void MQTTConnectionManagerWidget::deleteConnection() {
	auto status = KMessageBox::questionTwoActions(this,
												  i18n("Do you really want to delete the connection '%1'?", ui.lwConnections->currentItem()->text()),
												  i18n("Delete Connection"),
												  KStandardGuiItem::del(),
												  KStandardGuiItem::cancel());
	if (status != KMessageBox::PrimaryAction)
		return;

	// remove the current selected connection
	m_connections.removeAt(ui.lwConnections->currentRow());
	m_initializing = true;
	delete ui.lwConnections->takeItem(ui.lwConnections->currentRow());
	m_initializing = false;

	// show the connection for the item that was automatically selected after the deletion
	connectionChanged(ui.lwConnections->currentRow());

	// disable widgets if there are no connections anymore
	if (!m_currentConnection) {
		m_initializing = true;
		ui.leName->clear();
		ui.leName->setEnabled(false);
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
	Q_EMIT changed();
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
		conn.name = groupName;
		conn.hostName = group.readEntry("Host", "");
		conn.port = group.readEntry("Port", 0);

		conn.useAuthentication = group.readEntry("UseAuthentication", false);
		if (conn.useAuthentication) {
			conn.userName = group.readEntry("UserName", "");
			conn.password = group.readEntry("Password", "");
		}

		conn.useID = group.readEntry("UseID", false);
		if (conn.useID)
			conn.clientID = group.readEntry("ClientID", "");

		conn.retain = group.readEntry("Retain", false);

		m_connections.append(conn);
		ui.lwConnections->addItem(conn.name);
	}

	// show the first connection if available, create a new connection otherwise
	if (!m_connections.empty()) {
		if (!m_initConnName.isEmpty()) {
			auto items = ui.lwConnections->findItems(m_initConnName, Qt::MatchExactly);
			if (items.empty())
				ui.lwConnections->setCurrentRow(ui.lwConnections->count() - 1);
			else
				ui.lwConnections->setCurrentItem(items.constFirst());
		} else {
			ui.lwConnections->setCurrentRow(ui.lwConnections->count() - 1);
		}
	} else
		addConnection();

	m_initializing = false;

	// show the settings of the current connection
	connectionChanged(ui.lwConnections->currentRow());
}

/*!
 * \brief Saves the connections present in the list widget.
 */
void MQTTConnectionManagerWidget::saveConnections() {
	// delete saved connections
	KConfig config(m_configPath, KConfig::SimpleConfig);
	for (const auto& group : config.groupList())
		config.deleteGroup(group);

	// save connections
	for (const auto& conn : m_connections) {
		KConfigGroup group = config.group(conn.name);
		group.writeEntry("Host", conn.hostName);
		group.writeEntry("Port", conn.port);
		group.writeEntry("UseAuthentication", QString::number(conn.useAuthentication));
		group.writeEntry("UserName", conn.userName);
		group.writeEntry("Password", conn.password);
		group.writeEntry("UseID", QString::number(conn.useID));
		group.writeEntry("ClientID", conn.clientID);
		group.writeEntry("Retain", QString::number(conn.retain));
	}

	config.sync();
}

/*!
 * \brief Checks whether every connection's settings are alright or not
 */
bool MQTTConnectionManagerWidget::checkConnections() {
	if (m_connections.isEmpty())
		return true;

	bool connectionsOk = true;

	for (int i = 0; i < m_connections.size(); ++i) {
		auto& c1 = m_connections[i];
		auto equalNames = ui.lwConnections->findItems(c1.name, Qt::MatchExactly);
		bool nameOK = (!c1.name.isEmpty()) && (equalNames.size() == 1);

		bool authenticationUsed = c1.useAuthentication;
		bool idUsed = c1.useID;
		bool authenticationFilled = !c1.userName.isEmpty() && !c1.password.isEmpty();
		bool idFilled = !c1.clientID.isEmpty();
		bool authenticationOK = (!authenticationUsed || authenticationFilled);
		bool idOK = (!idUsed || idFilled);

		bool uniqueHost = true;
		for (int j = 0; j < m_connections.size(); ++j) {
			if (i == j)
				continue;
			auto& c2 = m_connections[j];

			if (c2.hostName == c1.hostName && c2.port == c1.port) {
				uniqueHost = false;
				break;
			}
		}
		bool hostOK = (!c1.hostName.isEmpty()) && uniqueHost;
		bool allOk = authenticationOK && idOK && hostOK && nameOK;

		if (!allOk) {
			connectionsOk = false;
			SET_WARNING_BACKGROUND(ui.lwConnections->item(i))
		} else
			ui.lwConnections->item(i)->setBackground(QBrush());
	}
	return connectionsOk;
}

/*!
 * \brief Provides a sample host name, which has to be changed before use.
 * \return
 */
QString MQTTConnectionManagerWidget::uniqueName() {
	QString name = i18n("New connection");

	QStringList connectionNames;
	for (int row = 0; row < ui.lwConnections->count(); row++)
		connectionNames << ui.lwConnections->item(row)->text();

	if (!connectionNames.contains(name))
		return name;

	QString base = name;
	int lastNonDigit;
	for (lastNonDigit = base.size() - 1; lastNonDigit >= 0 && base[lastNonDigit].category() == QChar::Number_DecimalDigit; --lastNonDigit)
		base.chop(1);

	if (lastNonDigit >= 0 && base[lastNonDigit].category() != QChar::Separator_Space)
		base.append(QLatin1Char(' '));

	int newNr = name.right(name.size() - base.size()).toInt();
	QString newName;
	do
		newName = base + QString::number(++newNr);
	while (connectionNames.contains(newName));

	return newName;
}

/*!
 * \brief Tests the currently selected connection
 */
void MQTTConnectionManagerWidget::testConnection() {
	if (!m_currentConnection)
		return;

	WAIT_CURSOR;

	m_testing = true;

	if (!m_client) {
		m_client = new QMqttClient;
		m_testTimer = new QTimer(this);
		m_testTimer->setInterval(5000);
		connect(m_client, &QMqttClient::connected, this, &MQTTConnectionManagerWidget::onConnect);
		connect(m_client, &QMqttClient::disconnected, this, &MQTTConnectionManagerWidget::onDisconnect);
		connect(m_testTimer, &QTimer::timeout, this, &MQTTConnectionManagerWidget::testTimeout);
	}

	m_client->setHostname(m_currentConnection->hostName);
	m_client->setPort(m_currentConnection->port);

	if (m_currentConnection->useID)
		m_client->setClientId(m_currentConnection->clientID);

	if (m_currentConnection->useAuthentication) {
		m_client->setUsername(m_currentConnection->userName);
		m_client->setPassword(m_currentConnection->password);
	}

	m_testTimer->start();
	m_client->connectToHost();
}

/*!
 * \brief Called when the client connects to the host, this means the test was successful
 */
void MQTTConnectionManagerWidget::onConnect() {
	RESET_CURSOR;
	m_testTimer->stop();

	KMessageBox::information(this,
							 i18n("Connection to the broker '%1:%2' was successful.", m_currentConnection->hostName, QString::number(m_currentConnection->port)),
							 i18n("Connection Successful"));

	m_client->disconnectFromHost();
}

/*!
 * \brief Called when testTimer times out, this means that the test wasn't successful
 */
void MQTTConnectionManagerWidget::testTimeout() {
	RESET_CURSOR;
	m_testTimer->stop();

	KMessageBox::error(this,
					   i18n("Failed to connect to the broker '%1:%2'.", m_currentConnection->hostName, m_currentConnection->port),
					   i18n("Connection Failed"));

	m_client->disconnectFromHost();
}

/*!
 * \brief Called when the client disconnects from the host
 */
void MQTTConnectionManagerWidget::onDisconnect() {
	RESET_CURSOR;
	if (m_testTimer->isActive()) {
		KMessageBox::error(
			this,
			i18n("Disconnected from the broker '%1:%2' before the connection was successful.", m_currentConnection->hostName, m_currentConnection->port),
			i18n("Connection Failed"));
		m_testTimer->stop();
	}
}
