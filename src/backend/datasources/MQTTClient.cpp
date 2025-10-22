/*
	File		: MQTTClient.cpp
	Project		: LabPlot
	Description	: Represents a MQTT Client
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2018 Kovacs Ferencz <kferike98@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "backend/datasources/MQTTClient.h"
#include "backend/core/column/Column.h"
#include "backend/datasources/MQTTSubscription.h"
#include "backend/datasources/MQTTTopic.h"
#include "backend/datasources/filters/AsciiFilter.h"
#include "backend/lib/XmlStreamReader.h"
#include "frontend/datasources/MQTTErrorWidget.h"

#include <QTimer>
#include <QtMqtt/QMqttSubscription>
#include <QtMqtt/QMqttTopicName>

/*!
  \class MQTTClient
  \brief The MQTT Client connects to the broker set in ImportFileWidget.
It manages the MQTTSubscriptions, and the MQTTTopics.

  \ingroup datasources
*/
MQTTClient::MQTTClient(const QString& name)
	: Folder(name, AspectType::MQTTClient)
	, m_updateTimer(new QTimer(this))
	, m_client(new QMqttClient(this))
	, m_willTimer(new QTimer(this)) {
	connect(m_updateTimer, &QTimer::timeout, this, &MQTTClient::read);
	connect(m_client, &QMqttClient::connected, this, &MQTTClient::onMQTTConnect);
	connect(m_willTimer, &QTimer::timeout, this, &MQTTClient::updateWillMessage);
	connect(m_client, &QMqttClient::errorChanged, this, &MQTTClient::MQTTErrorChanged);
	connect(m_client, &QMqttClient::disconnected, this, &MQTTClient::disconnected);
}

MQTTClient::~MQTTClient() {
	Q_EMIT clientAboutToBeDeleted(m_client->hostname(), m_client->port());
	// stop reading before deleting the objects
	pauseReading();

	delete m_filter;
	delete m_updateTimer;
	delete m_willTimer;
	m_client->disconnectFromHost();
	delete m_client;
}

/*!
 * depending on the update type, periodically or on data changes, starts the timer.
 */
void MQTTClient::ready() {
	if (m_updateType == UpdateType::TimeInterval)
		m_updateTimer->start(m_updateInterval);
}

/*!
 * \brief Updates the MQTTTopics of the client
 */
void MQTTClient::updateNow() {
	m_updateTimer->stop();
	read();
	if ((m_updateType == UpdateType::TimeInterval) && !m_paused)
		m_updateTimer->start(m_updateInterval);
}

/*!
 * \brief Continue reading from messages after it was paused.
 */
void MQTTClient::continueReading() {
	m_paused = false;
	if (m_updateType == UpdateType::TimeInterval)
		m_updateTimer->start(m_updateInterval);
}

/*!
 * \brief Pause the reading from messages.
 */
void MQTTClient::pauseReading() {
	m_paused = true;
	if (m_updateType == UpdateType::TimeInterval)
		m_updateTimer->stop();
}

/*!
 * \brief Sets the filter of the MQTTClient.
 * The ownership of the filter is passed to MQTTClient.
 *
 * \param f a pointer to the new filter
 */
void MQTTClient::setFilter(AsciiFilter* f) {
	delete m_filter;
	auto* asciiFilter = dynamic_cast<AsciiFilter*>(f);
	if (asciiFilter)
		asciiFilter->setDataSource(nullptr); // MQTTClient does not have a dataSource
	m_filter = f;
}

/*!
 * \brief Returns the filter of the MQTTClient.
 */
AsciiFilter* MQTTClient::filter() const {
	return m_filter;
}

/*!
 * \brief Sets the MQTTclient's update interval to \c interval
 * \param interval
 */
void MQTTClient::setUpdateInterval(int interval) {
	m_updateInterval = interval;
	if (!m_paused)
		m_updateTimer->start(m_updateInterval);
}

/*!
 * \brief Returns the MQTTClient's update interval to \c interval
 * \param interval
 */
int MQTTClient::updateInterval() const {
	return m_updateInterval;
}

/*!
 * \brief Sets how many values we should store
 * \param keepNValues
 */
void MQTTClient::setKeepNValues(int keepNValues) {
	m_keepNValues = keepNValues;
}

/*!
 * \brief Returns how many values we should store
 */
int MQTTClient::keepNValues() const {
	return m_keepNValues;
}

/*!
 * \brief Provides information about whether the reading is paused or not
 *
 * \return true if the reading is paused
 * \return false otherwise
 */
bool MQTTClient::isPaused() const {
	return m_paused;
}

/*!
 * \brief Sets the size rate to sampleSize
 * \param sampleSize
 */
void MQTTClient::setSampleSize(int sampleSize) {
	m_sampleSize = sampleSize;
}

/*!
 * \brief Returns the size rate
 */
int MQTTClient::sampleSize() const {
	return m_sampleSize;
}

/*!
 * \brief Sets the MQTTClient's reading type to readingType
 * \param readingType
 */
void MQTTClient::setReadingType(ReadingType readingType) {
	m_readingType = readingType;
}

/*!
 * \brief Returns the MQTTClient's reading type
 */
MQTTClient::ReadingType MQTTClient::readingType() const {
	return m_readingType;
}

/*!
 * \brief Sets the MQTTClient's update type to updatetype and handles this change
 * \param updatetype
 */
void MQTTClient::setUpdateType(UpdateType updateType) {
	if (updateType == UpdateType::NewData)
		m_updateTimer->stop();

	m_updateType = updateType;
}

/*!
 * \brief Returns the MQTTClient's update type
 */
MQTTClient::UpdateType MQTTClient::updateType() const {
	return m_updateType;
}

/*!
 * \brief Returns the MQTTClient's icon
 */
QIcon MQTTClient::icon() const {
	return QIcon::fromTheme(QStringLiteral("network-server-database"));
}

/*!
 * \brief Sets the host and port for the client.
 *
 * \param host the hostname of the broker we want to connect to
 * \param port the port used by the broker
 */
void MQTTClient::setMQTTClientHostPort(const QString& host, quint16 port) {
	m_client->setHostname(host);
	m_client->setPort(port);
}

/*!
 * \brief Returns hostname of the broker the client is connected to.
 */
QString MQTTClient::clientHostName() const {
	return m_client->hostname();
}

/*!
 * \brief Returns the port used by the broker.
 */
quint16 MQTTClient::clientPort() const {
	return m_client->port();
}

/*!
 * \brief Sets the flag on the given value.
 * If set true it means that the broker requires authentication, otherwise it doesn't.
 *
 * \param use
 */
void MQTTClient::setMQTTUseAuthentication(bool use) {
	m_MQTTUseAuthentication = use;
}

/*!
 * \brief Returns whether the broker requires authentication or not.
 */
bool MQTTClient::MQTTUseAuthentication() const {
	return m_MQTTUseAuthentication;
}

/*!
 * \brief Sets the username and password for the client.
 *
 * \param username the username used for authentication
 * \param password the password used for authentication
 */
void MQTTClient::setMQTTClientAuthentication(const QString& username, const QString& password) {
	m_client->setUsername(username);
	m_client->setPassword(password);
}

/*!
 * \brief Returns the username used for authentication.
 */
QString MQTTClient::clientUserName() const {
	return m_client->username();
}

/*!
 * \brief Returns the password used for authentication.
 */
QString MQTTClient::clientPassword() const {
	return m_client->password();
}

/*!
 * \brief Sets the flag on the given value.
 * If set true it means that user wants to set the client ID, otherwise it's not the case.
 *
 * \param use
 */
void MQTTClient::setMQTTUseID(bool use) {
	m_MQTTUseID = use;
}

/*!
 * \brief Returns whether the user wants to set the client ID or not.
 */
bool MQTTClient::MQTTUseID() const {
	return m_MQTTUseID;
}

/*!
 * \brief Sets the ID of the client
 *
 * \param id
 */
void MQTTClient::setMQTTClientId(const QString& clientId) {
	m_client->setClientId(clientId);
}

/*!
 * \brief Returns the ID of the client
 */
QString MQTTClient::clientID() const {
	return m_client->clientId();
}

/*!
 * \brief Sets the flag on the given value.
 * If retain is true we interpret retain messages, otherwise we do not
 *
 * \param retain
 */
void MQTTClient::setMQTTRetain(bool retain) {
	m_MQTTRetain = retain;
}

/*!
 * \brief Returns the flag, which set to true means that interpret retain messages, otherwise we do not
 */
bool MQTTClient::MQTTRetain() const {
	return m_MQTTRetain;
}

/*!
 * \brief Returns the name of every MQTTTopics which already received a message, and is child of the MQTTClient
 */
QVector<QString> MQTTClient::topicNames() const {
	return m_topicNames;
}

/*!
 * \brief Adds the initial subscriptions that were set in ImportFileWidget
 *
 * \param filter the name of the subscribed topic
 * \param qos the qos level of the subscription
 */
void MQTTClient::addInitialMQTTSubscriptions(const QMqttTopicFilter& filter, quint8 qos) {
	m_subscribedTopicNameQoS[filter] = qos;
}

/*!
 * \brief Returns the name of every MQTTSubscription of the MQTTClient
 */
QVector<QString> MQTTClient::MQTTSubscriptions() const {
	return m_subscriptions;
}

/*!
 * \brief Adds a new MQTTSubscription to the MQTTClient
 *
 * \param topic, the name of the topic
 * \param QoS
 */
void MQTTClient::addMQTTSubscription(const QString& topicName, quint8 QoS) {
	// Check whether the subscription already exists, if it doesn't, we can add it
	if (!m_subscriptions.contains(topicName)) {
		const auto* temp = m_client->subscribe(QMqttTopicFilter(topicName), QoS);
		if (temp) {
			// qDebug()<<"Subscribe to: "<< temp->topic() << "  " << temp->qos();
			m_subscriptions.push_back(temp->topic().filter());
			m_subscribedTopicNameQoS[temp->topic().filter()] = temp->qos();

			auto* newSubscription = new MQTTSubscription(temp->topic().filter());
			newSubscription->setMQTTClient(this);

			addChildFast(newSubscription);
			m_MQTTSubscriptions.push_back(newSubscription);

			// Search for inferior subscriptions, that the new subscription contains
			bool found = false;
			QVector<MQTTSubscription*> inferiorSubscriptions;
			for (auto* subscription : m_MQTTSubscriptions) {
				if (checkTopicContains(topicName, subscription->subscriptionName()) && topicName != subscription->subscriptionName()) {
					found = true;
					inferiorSubscriptions.push_back(subscription);
				}
			}

			// If there are some inferior subscriptions, we have to deal with them
			if (found) {
				for (auto* inferiorSubscription : inferiorSubscriptions) {
					// qDebug()<<"Reparent topics of inferior subscription: "<< inferiorSubscription->subscriptionName();
					// We have to reparent every topic of the inferior subscription, so no data is lost
					const auto& topics = inferiorSubscription->topics();
					for (auto* topic : topics)
						topic->reparent(newSubscription);

					// Then remove the subscription and every connected information
					m_client->unsubscribe(QMqttTopicFilter(inferiorSubscription->subscriptionName()));
					for (int j = 0; j < m_MQTTSubscriptions.size(); ++j) {
						if (m_MQTTSubscriptions.at(j)->subscriptionName() == inferiorSubscription->subscriptionName()) {
							m_MQTTSubscriptions.remove(j);
						}
					}
					m_subscriptions.removeAll(inferiorSubscription->subscriptionName());
					m_subscribedTopicNameQoS.remove(inferiorSubscription->subscriptionName());

					removeChild(inferiorSubscription);
				}
			}

			connect(temp, &QMqttSubscription::messageReceived, this, &MQTTClient::MQTTSubscriptionMessageReceived);
			Q_EMIT MQTTTopicsChanged();
		}
	}
}

/*!
 * \brief Removes a MQTTSubscription from the MQTTClient
 *
 * \param name, the name of the subscription to remove
 */
void MQTTClient::removeMQTTSubscription(const QString& subscriptionName) {
	// We can only remove the subscription if it exists
	if (m_subscriptions.contains(subscriptionName)) {
		// unsubscribe from the topic
		const QMqttTopicFilter filter{subscriptionName};
		m_client->unsubscribe(filter);
		// qDebug()<<"Unsubscribe from: " << subscriptionName;

		// Remove every connected information
		m_subscriptions.removeAll(subscriptionName);

		for (int i = 0; i < m_MQTTSubscriptions.size(); ++i) {
			if (m_MQTTSubscriptions.at(i)->subscriptionName() == subscriptionName) {
				auto* removeSubscription = m_MQTTSubscriptions.at(i);
				m_MQTTSubscriptions.remove(i);

				// Remove every topic of the subscription as well
				const auto& topics = removeSubscription->topics();
				for (const auto& topic : topics)
					m_topicNames.removeAll(topic->topicName());

				// Remove the MQTTSubscription
				removeChild(removeSubscription);
				break;
			}
		}

		QMapIterator<QMqttTopicFilter, quint8> j(m_subscribedTopicNameQoS);
		while (j.hasNext()) {
			j.next();
			if (j.key().filter() == subscriptionName) {
				m_subscribedTopicNameQoS.remove(j.key());
				break;
			}
		}
		Q_EMIT MQTTTopicsChanged();
	}
}

/*!
 * \brief Adds a MQTTSubscription to the MQTTClient
 *Used when the user unsubscribes from a topic of a MQTTSubscription
 *
 * \param topic, the name of the topic
 * \param QoS
 */
void MQTTClient::addBeforeRemoveSubscription(const QString& topicName, quint8 QoS) {
	// We can't add the subscription if it already exists
	if (!m_subscriptions.contains(topicName)) {
		// Subscribe to the topic
		const QMqttTopicFilter filter{topicName};
		auto* temp = m_client->subscribe(filter, QoS);
		if (temp) {
			// Add the MQTTSubscription and other connected data
			// qDebug()<<"Add subscription before remove: " << temp->topic() << "  " << temp->qos();
			m_subscriptions.push_back(temp->topic().filter());
			m_subscribedTopicNameQoS[temp->topic().filter()] = temp->qos();

			auto* newSubscription = new MQTTSubscription(temp->topic().filter());
			newSubscription->setMQTTClient(this);

			addChildFast(newSubscription);
			m_MQTTSubscriptions.push_back(newSubscription);

			// Search for the subscription the topic belonged to
			bool found = false;
			MQTTSubscription* superiorSubscription = nullptr;
			for (auto* subscription : m_MQTTSubscriptions) {
				if (checkTopicContains(subscription->subscriptionName(), topicName) && topicName != subscription->subscriptionName()) {
					found = true;
					superiorSubscription = subscription;
					break;
				}
			}

			if (found) {
				// Search for topics belonging to the superior(old) subscription
				// which are also contained by the new subscription
				QVector<MQTTTopic*> topics = superiorSubscription->topics();
				// 				qDebug()<< topics.size();

				QVector<MQTTTopic*> inferiorTopics;
				for (auto* topic : topics) {
					if (checkTopicContains(topicName, topic->topicName())) {
						inferiorTopics.push_back(topic);
					}
				}

				// Reparent these topics, in order to avoid data loss
				for (auto* inferiorTopic : inferiorTopics) {
					inferiorTopic->reparent(newSubscription);
				}
			}
			connect(temp, &QMqttSubscription::messageReceived, this, &MQTTClient::MQTTSubscriptionMessageReceived);
		}
	}
}

/*!
 * \brief Reparents the given MQTTTopic to the given MQTTSubscription
 *
 * \param topic, the name of the MQTTTopic
 * \param parent, the name of the MQTTSubscription
 */
void MQTTClient::reparentTopic(const QString& topicName, const QString& parentTopicName) {
	// We can only reparent if the parent contained the topic
	if (m_subscriptions.contains(parentTopicName) && m_topicNames.contains(topicName)) {
		// qDebug() << "Reparent " << topicName << " to " << parentTopicName;
		// search for the parent MQTTSubscription
		bool found = false;
		MQTTSubscription* superiorSubscription = nullptr;
		for (auto* subscription : m_MQTTSubscriptions) {
			if (subscription->subscriptionName() == parentTopicName) {
				found = true;
				superiorSubscription = subscription;
				break;
			}
		}

		if (found) {
			// get every topic of the MQTTClient
			QVector<MQTTTopic*> topics = children<MQTTTopic>(AbstractAspect::ChildIndexFlag::Recursive);
			// Search for the given topic among the MQTTTopics
			for (auto* topic : topics) {
				if (topicName == topic->topicName()) {
					// if found, it is reparented to the parent MQTTSubscription
					topic->reparent(superiorSubscription);
					break;
				}
			}
		}
	}
}

/*!
 *\brief Checks if a topic contains another one
 *
 * \param superior the name of a topic
 * \param inferior the name of a topic
 * \return	true if superior is equal to or contains(if superior contains wildcards) inferior,
 *			false otherwise
 */
bool MQTTClient::checkTopicContains(const QString& superior, const QString& inferior) {
	if (superior == inferior)
		return true;
	else {
		if (superior.contains(QLatin1String("/"))) {
			QStringList superiorList = superior.split(QLatin1Char('/'), Qt::SkipEmptyParts);
			QStringList inferiorList = inferior.split(QLatin1Char('/'), Qt::SkipEmptyParts);

			// a longer topic can't contain a shorter one
			if (superiorList.size() > inferiorList.size())
				return false;

			bool ok = true;
			for (int i = 0; i < superiorList.size(); ++i) {
				if (superiorList.at(i) != inferiorList.at(i)) {
					if ((superiorList.at(i) != QLatin1Char('+')) && !(superiorList.at(i) == QLatin1Char('#') && i == superiorList.size() - 1)) {
						// if the two topics differ, and the superior's current level isn't + or #(which can be only in the last position)
						// then superior can't contain inferior
						ok = false;
						break;
					} else if (i == superiorList.size() - 1 && (superiorList.at(i) == QLatin1Char('+') && inferiorList.at(i) == QLatin1Char('#'))) {
						// if the two topics differ at the last level
						// and the superior's current level is + while the inferior's is #(which can be only in the last position)
						// then superior can't contain inferior
						ok = false;
						break;
					}
				}
			}
			return ok;
		}
		return false;
	}
}

/*!
 *\brief Returns the '+' wildcard containing topic name, which includes the given topic names
 *
 * \param first the name of a topic
 * \param second the name of a topic
 * \return The name of the common topic, if it exists, otherwise an empty string
 */
QString MQTTClient::checkCommonLevel(const QString& first, const QString& second) {
	QStringList firstList = first.split(QLatin1Char('/'), Qt::SkipEmptyParts);
	QStringList secondtList = second.split(QLatin1Char('/'), Qt::SkipEmptyParts);
	QString commonTopic;

	if (!firstList.isEmpty()) {
		// the two topics have to be the same size and can't be identic
		if ((firstList.size() == secondtList.size()) && (first != second)) {
			// the index where they differ
			int differIndex = -1;
			for (int i = 0; i < firstList.size(); ++i) {
				if (firstList.at(i) != secondtList.at(i)) {
					differIndex = i;
					break;
				}
			}

			// they can differ at only one level and that can't be the first
			bool differ = false;
			if (differIndex > 0) {
				for (int j = differIndex + 1; j < firstList.size(); ++j) {
					if (firstList.at(j) != secondtList.at(j)) {
						differ = true;
						break;
					}
				}
			} else
				differ = true;

			if (!differ) {
				for (int i = 0; i < firstList.size(); ++i) {
					if (i != differIndex) {
						commonTopic.append(firstList.at(i));
					} else {
						// we put '+' wildcard at the level where they differ
						commonTopic.append(QStringLiteral("+"));
					}

					if (i != firstList.size() - 1)
						commonTopic.append(QStringLiteral("/"));
				}
			}
		}
	}
	// qDebug() << first << " " << second << " common topic: "<<commonTopic;
	return commonTopic;
}

void MQTTClient::setWillSettings(const MQTTWill& settings) {
	m_MQTTWill = settings;
}

MQTTClient::MQTTWill MQTTClient::willSettings() const {
	return m_MQTTWill;
}

/*!
 * \brief Sets whether the user wants to use will message or not
 *
 * \param use
 */
void MQTTClient::setMQTTWillUse(bool use) {
	m_MQTTWill.enabled = use;
	if (use == false)
		m_willTimer->stop();
}

/*!
 * \brief Returns whether the user wants to use will message or not
 */
bool MQTTClient::MQTTWillUse() const {
	return m_MQTTWill.enabled;
}

/*!
 * \brief Sets the will topic of the client
 *
 * \param topic
 */
void MQTTClient::setWillTopic(const QString& topic) {
	// 	qDebug() << "Set will topic:" << topic;
	m_MQTTWill.willTopic = topic;
}

/*!
 * \brief Returns the will topic of the client
 */
QString MQTTClient::willTopic() const {
	return m_MQTTWill.willTopic;
}

/*!
 * \brief Sets the retain flag of the client's will message
 *
 * \param retain
 */
void MQTTClient::setWillRetain(bool retain) {
	m_MQTTWill.willRetain = retain;
}

/*!
 * \brief Returns the retain flag of the client's will message
 */
bool MQTTClient::willRetain() const {
	return m_MQTTWill.willRetain;
}

/*!
 * \brief Sets the QoS level of the client's will message
 *
 * \param QoS
 */
void MQTTClient::setWillQoS(quint8 QoS) {
	m_MQTTWill.willQoS = QoS;
}

/*!
 * \brief Returns the QoS level of the client's will message
 */
quint8 MQTTClient::willQoS() const {
	return m_MQTTWill.willQoS;
}

/*!
 * \brief Sets the will message type of the client
 *
 * \param messageType
 */
void MQTTClient::setWillMessageType(WillMessageType messageType) {
	m_MQTTWill.willMessageType = messageType;
}

/*!
 * \brief Returns the will message type of the client
 */
MQTTClient::WillMessageType MQTTClient::willMessageType() const {
	return m_MQTTWill.willMessageType;
}

/*!
 * \brief Sets the own will message of the user
 *
 * \param ownMessage
 */
void MQTTClient::setWillOwnMessage(const QString& ownMessage) {
	m_MQTTWill.willOwnMessage = ownMessage;
}

/*!
 * \brief Returns the own will message of the user
 */
QString MQTTClient::willOwnMessage() const {
	return m_MQTTWill.willOwnMessage;
}

/*!
 * \brief Updates the will message of the client
 */
void MQTTClient::updateWillMessage() {
	// Search for the will topic
	const auto& topics = children<const MQTTTopic>(AbstractAspect::ChildIndexFlag::Recursive);
	const MQTTTopic* willTopic = nullptr;
	for (const auto* topic : topics) {
		if (topic->topicName() == m_MQTTWill.willTopic) {
			willTopic = topic;
			break;
		}
	}

	// if the will topic is found we can update the will message
	if (willTopic != nullptr) {
		// To update the will message we have to disconnect first, then after setting everything connect again
		if (m_MQTTWill.enabled && (m_client->state() == QMqttClient::ClientState::Connected)) {
			// Disconnect only once (disconnecting may take a while)
			if (!m_disconnectForWill) {
				// 				qDebug() << "Disconnecting from host in order to update will message";
				m_client->disconnectFromHost();
				m_disconnectForWill = true;
			}
			// Try to update again
			updateWillMessage();
		}
		// If client is disconnected we can update the settings
		else if (m_MQTTWill.enabled && (m_client->state() == QMqttClient::ClientState::Disconnected) && m_disconnectForWill) {
			m_client->setWillQoS(m_MQTTWill.willQoS);
			m_client->setWillRetain(m_MQTTWill.willRetain);
			m_client->setWillTopic(m_MQTTWill.willTopic);

			// Set the will message according to m_willMessageType
			switch (m_MQTTWill.willMessageType) {
			case WillMessageType::OwnMessage:
				m_client->setWillMessage(m_MQTTWill.willOwnMessage.toUtf8());
				// 				qDebug()<<"Will own message" << m_MQTTWill.willOwnMessage;
				break;
			case WillMessageType::Statistics: {
				// Statistics is only possible if the data stored in the MQTTTopic is of type integer or numeric
				// check the column mode of the last column in the topic for this.
				// TODO: check this logic again - why last column only?
				const auto* col = willTopic->child<Column>(willTopic->childCount<Column>() - 1);
				auto mode = col->columnMode();
				if (mode == AbstractColumn::ColumnMode::Integer || mode == AbstractColumn::ColumnMode::Double)
					m_client->setWillMessage(this->statistics(willTopic).toUtf8());
				else
					m_client->setWillMessage(QByteArray()); // empty message
				// qDebug() << "Will statistics message: "<< QString(m_client->willMessage());
				break;
			}
			case WillMessageType::LastMessage:
				m_client->setWillMessage(m_MQTTWill.willLastMessage.toUtf8());
				// qDebug()<<"Will last message:\n" << m_MQTTWill.willLastMessage;
				break;
			default:
				break;
			}
			m_disconnectForWill = false;
			// Reconnect with the updated message
			m_client->connectToHost();
			// qDebug()<< "Reconnect to host after updating will message";
		}
	}
}

/*!
 * \brief Returns the statistical data that is needed by the topic for its MQTTClient's will message
 * \param topic
 */
QString MQTTClient::statistics(const MQTTTopic* topic) const {
	const auto* col = topic->child<Column>(topic->childCount<Column>() - 1);
	QString statistics;

	QVector<bool> willStatistics = topic->mqttClient()->willStatistics();
	// Add every statistical data to the string, the flag of which is set true
	for (int i = 0; i < willStatistics.size(); i++) {
		if (willStatistics[i]) {
			switch (static_cast<MQTTClient::WillStatisticsType>(i)) {
			case MQTTClient::WillStatisticsType::ArithmeticMean:
				statistics += QStringLiteral("Arithmetic mean: ") + QString::number(col->statistics().arithmeticMean) + QStringLiteral("\n");
				break;
			case MQTTClient::WillStatisticsType::ContraharmonicMean:
				statistics += QStringLiteral("Contraharmonic mean: ") + QString::number(col->statistics().contraharmonicMean) + QStringLiteral("\n");
				break;
			case MQTTClient::WillStatisticsType::Entropy:
				statistics += QStringLiteral("Entropy: ") + QString::number(col->statistics().entropy) + QStringLiteral("\n");
				break;
			case MQTTClient::WillStatisticsType::GeometricMean:
				statistics += QStringLiteral("Geometric mean: ") + QString::number(col->statistics().geometricMean) + QStringLiteral("\n");
				break;
			case MQTTClient::WillStatisticsType::HarmonicMean:
				statistics += QStringLiteral("Harmonic mean: ") + QString::number(col->statistics().harmonicMean) + QStringLiteral("\n");
				break;
			case MQTTClient::WillStatisticsType::Kurtosis:
				statistics += QStringLiteral("Kurtosis: ") + QString::number(col->statistics().kurtosis) + QStringLiteral("\n");
				break;
			case MQTTClient::WillStatisticsType::Maximum:
				statistics += QStringLiteral("Maximum: ") + QString::number(col->statistics().maximum) + QStringLiteral("\n");
				break;
			case MQTTClient::WillStatisticsType::MeanDeviation:
				statistics += QStringLiteral("Mean deviation: ") + QString::number(col->statistics().meanDeviation) + QStringLiteral("\n");
				break;
			case MQTTClient::WillStatisticsType::MeanDeviationAroundMedian:
				statistics +=
					QStringLiteral("Mean deviation around median: ") + QString::number(col->statistics().meanDeviationAroundMedian) + QStringLiteral("\n");
				break;
			case MQTTClient::WillStatisticsType::Median:
				statistics += QStringLiteral("Median: ") + QString::number(col->statistics().median) + QStringLiteral("\n");
				break;
			case MQTTClient::WillStatisticsType::MedianDeviation:
				statistics += QStringLiteral("Median deviation: ") + QString::number(col->statistics().medianDeviation) + QStringLiteral("\n");
				break;
			case MQTTClient::WillStatisticsType::Minimum:
				statistics += QStringLiteral("Minimum: ") + QString::number(col->statistics().minimum) + QStringLiteral("\n");
				break;
			case MQTTClient::WillStatisticsType::Skewness:
				statistics += QStringLiteral("Skewness: ") + QString::number(col->statistics().skewness) + QStringLiteral("\n");
				break;
			case MQTTClient::WillStatisticsType::StandardDeviation:
				statistics += QStringLiteral("Standard deviation: ") + QString::number(col->statistics().standardDeviation) + QStringLiteral("\n");
				break;
			case MQTTClient::WillStatisticsType::Variance:
				statistics += QStringLiteral("Variance: ") + QString::number(col->statistics().variance) + QStringLiteral("\n");
				break;
			case MQTTClient::WillStatisticsType::NoStatistics:
			default:
				break;
			}
		}
	}
	return statistics;
}

/*!
 * \brief Returns the MQTTClient's will update type
 */
MQTTClient::WillUpdateType MQTTClient::willUpdateType() const {
	return m_MQTTWill.willUpdateType;
}

/*!
 * \brief Sets the MQTTClient's will update type
 *
 * \param willUpdateType
 */
void MQTTClient::setWillUpdateType(WillUpdateType willUpdateType) {
	m_MQTTWill.willUpdateType = willUpdateType;
}

/*!
 * \brief Returns the time interval of updating the MQTTClient's will message
 */
int MQTTClient::willTimeInterval() const {
	return m_MQTTWill.willTimeInterval;
}

/*!
 * \brief Sets the time interval of updating the MQTTClient's will message, if update type is TimePeriod
 *
 * \param interval
 */
void MQTTClient::setWillTimeInterval(int interval) {
	m_MQTTWill.willTimeInterval = interval;
}

/*!
 * \brief Clear the lastly received message by the will topic
 * Called when the will topic is changed
 */
void MQTTClient::clearLastMessage() {
	m_MQTTWill.willLastMessage.clear();
}

/*!
 * \brief Sets true the corresponding flag of the statistic type,
 *  what means that the given statistic type will be added to the will message
 *
 * \param statistics
 */
void MQTTClient::addWillStatistics(WillStatisticsType statistic) {
	m_MQTTWill.willStatistics[static_cast<int>(statistic)] = true;
}

/*!
 * \brief Sets false the corresponding flag of the statistic type,
 * what means that the given statistic will no longer be added to the will message
 *
 * \param statistics
 */
void MQTTClient::removeWillStatistics(WillStatisticsType statistic) {
	m_MQTTWill.willStatistics[static_cast<int>(statistic)] = false;
}

/*!
 * \brief Returns a bool vector, meaning which statistic types are included in the will message
 * If the corresponding value is true, the statistic type is included, otherwise it isn't
 */
QVector<bool> MQTTClient::willStatistics() const {
	return m_MQTTWill.willStatistics;
}

/*!
 * \brief Starts the will timer, which will update the will message
 */
void MQTTClient::startWillTimer() const {
	if (m_MQTTWill.willUpdateType == WillUpdateType::TimePeriod)
		m_willTimer->start(m_MQTTWill.willTimeInterval);
}

/*!
 * \brief Stops the will timer
 */
void MQTTClient::stopWillTimer() const {
	m_willTimer->stop();
}

// ##############################################################################
// #################################  SLOTS  ####################################
// ##############################################################################

/*!
 *\brief called periodically when update type is TimeInterval
 */
void MQTTClient::read() {
	if (!m_filter)
		return;

	if (!m_prepared) {
		// qDebug()<<"Connect";
		// connect to the broker
		m_client->connectToHost();
		m_prepared = true;
	}

	if ((m_client->state() == QMqttClient::ClientState::Connected) && m_MQTTFirstConnectEstablished) {
		// qDebug()<<"Read";
		// Signal for every MQTTTopic that they can read
		Q_EMIT readFromTopics();
	}
}

/*!
 *\brief called when the client successfully connected to the broker
 */
void MQTTClient::onMQTTConnect() {
	if (m_client->error() == QMqttClient::NoError) {
		// if this is the first connection (after setting the options in ImportFileWidget or loading saved project)
		if (!m_MQTTFirstConnectEstablished) {
			// qDebug()<<"connection made in MQTTClient";

			// Subscribe to initial or loaded topics
			QMapIterator<QMqttTopicFilter, quint8> i(m_subscribedTopicNameQoS);
			while (i.hasNext()) {
				i.next();
				const auto* temp = m_client->subscribe(i.key(), i.value());
				if (temp) {
					// If we didn't load the MQTTClient from xml we have to add the MQTTSubscriptions
					if (!m_loaded) {
						m_subscriptions.push_back(temp->topic().filter());

						auto* newSubscription = new MQTTSubscription(temp->topic().filter());
						newSubscription->setMQTTClient(this);

						addChildFast(newSubscription);

						m_MQTTSubscriptions.push_back(newSubscription);
					}

					connect(temp, &QMqttSubscription::messageReceived, this, &MQTTClient::MQTTSubscriptionMessageReceived);
				}
			}
			m_MQTTFirstConnectEstablished = true;
			Q_EMIT MQTTSubscribed(); // notify about the initial subscriptiong made
		} else { // connection already available, happens after updating will message
			// qDebug() << "Start resubscribing after will message update";
			// Only the client has to make the subscriptions again, every other connected data is still available
			QMapIterator<QMqttTopicFilter, quint8> i(m_subscribedTopicNameQoS);
			while (i.hasNext()) {
				i.next();
				const auto* temp = m_client->subscribe(i.key(), i.value());
				if (temp) {
					// qDebug()<<temp->topic()<<"  "<<temp->qos();
					connect(temp, &QMqttSubscription::messageReceived, this, &MQTTClient::MQTTSubscriptionMessageReceived);
				}
				// else
				// qDebug()<<"Couldn't subscribe after will update";
			}
		}
	}
}

/*!
 *\brief called when a message is received by a topic belonging to one of subscriptions of the client.
 * It passes the message to the appropriate MQTTSubscription which will pass it to the appropriate MQTTTopic
 */
void MQTTClient::MQTTSubscriptionMessageReceived(const QMqttMessage& msg) {
	// Decide to interpret retain message or not
	if (!msg.retain() || m_MQTTRetain) {
		// If this is the first message from the topic, save its name
		if (!m_topicNames.contains(msg.topic().name()))
			m_topicNames.push_back(msg.topic().name());

		// Pass the message and the topic name to the MQTTSubscription which contains the topic
		for (auto* subscription : m_MQTTSubscriptions) {
			if (checkTopicContains(subscription->subscriptionName(), msg.topic().name())) {
				subscription->messageArrived(msg);
				Q_EMIT messagedReceived();
				break;
			}
		}

		// if the message was received by the will topic, update the last message received by it
		if (msg.topic().name() == m_MQTTWill.willTopic) {
			m_MQTTWill.willLastMessage = QLatin1String(msg.payload());
			Q_EMIT MQTTTopicsChanged();
		}
	}
}

/*!
 *\brief Handles some of the possible errors of the client, using MQTTErrorWidget
 */
void MQTTClient::MQTTErrorChanged(QMqttClient::ClientError clientError) {
	if (clientError != QMqttClient::ClientError::NoError) {
		auto* errorWidget = new MQTTErrorWidget(clientError, this);
		errorWidget->show();
	}
}

void MQTTClient::disconnected() {
	auto error = m_client->error();
	if (error != QMqttClient::ClientError::NoError) {
		auto* errorWidget = new MQTTErrorWidget(error, this);
		errorWidget->show();
	}
}

/*!
 *\brief Called when a subscription is loaded.
 * Checks whether every saved subscription was loaded or not.
 * If everything is loaded, it makes the connection and starts the reading
 *
 * \param name, the name of the subscription
 */
void MQTTClient::subscriptionLoaded(const QString& name) {
	if (!name.isEmpty()) {
		// qDebug() << "Finished loading: " << name;
		// Save information about the subscription
		m_subscriptionsLoaded++;
		m_subscriptions.push_back(name);
		QMqttTopicFilter filter{name};
		m_subscribedTopicNameQoS[filter] = 0;

		// Save the topics belonging to the subscription
		for (const auto* subscription : m_MQTTSubscriptions) {
			if (subscription->subscriptionName() == name) {
				const auto& topics = subscription->topics();
				for (auto* topic : topics) {
					m_topicNames.push_back(topic->topicName());
				}
				break;
			}
		}

		// Check whether every subscription was loaded or not
		if (m_subscriptionsLoaded == m_subscriptionCountToLoad) {
			// if everything was loaded we can start reading
			m_loaded = true;
			read();
		}
	}
}

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################
/*!
  Saves as XML.
 */
void MQTTClient::save(QXmlStreamWriter* writer) const {
	writer->writeStartElement(QStringLiteral("MQTTClient"));
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	// general
	writer->writeStartElement(QStringLiteral("general"));
	writer->writeAttribute(QStringLiteral("subscriptionCount"), QString::number(m_MQTTSubscriptions.size()));
	writer->writeAttribute(QStringLiteral("updateType"), QString::number(static_cast<int>(m_updateType)));
	writer->writeAttribute(QStringLiteral("readingType"), QString::number(static_cast<int>(m_readingType)));
	writer->writeAttribute(QStringLiteral("keepValues"), QString::number(m_keepNValues));

	if (m_updateType == UpdateType::TimeInterval)
		writer->writeAttribute(QStringLiteral("updateInterval"), QString::number(m_updateInterval));

	if (m_readingType != ReadingType::TillEnd)
		writer->writeAttribute(QStringLiteral("sampleSize"), QString::number(m_sampleSize));

	writer->writeAttribute(QStringLiteral("host"), m_client->hostname());
	writer->writeAttribute(QStringLiteral("port"), QString::number(m_client->port()));
	writer->writeAttribute(QStringLiteral("username"), m_client->username());
	writer->writeAttribute(QStringLiteral("password"), m_client->password());
	writer->writeAttribute(QStringLiteral("clientId"), m_client->clientId());
	writer->writeAttribute(QStringLiteral("useRetain"), QString::number(m_MQTTRetain));
	writer->writeAttribute(QStringLiteral("useWill"), QString::number(m_MQTTWill.enabled));
	writer->writeAttribute(QStringLiteral("willTopic"), m_MQTTWill.willTopic);
	writer->writeAttribute(QStringLiteral("willOwnMessage"), m_MQTTWill.willOwnMessage);
	writer->writeAttribute(QStringLiteral("willQoS"), QString::number(m_MQTTWill.willQoS));
	writer->writeAttribute(QStringLiteral("willRetain"), QString::number(m_MQTTWill.willRetain));
	writer->writeAttribute(QStringLiteral("willMessageType"), QString::number(static_cast<int>(m_MQTTWill.willMessageType)));
	writer->writeAttribute(QStringLiteral("willUpdateType"), QString::number(static_cast<int>(m_MQTTWill.willUpdateType)));
	writer->writeAttribute(QStringLiteral("willTimeInterval"), QString::number(m_MQTTWill.willTimeInterval));

	for (int i = 0; i < m_MQTTWill.willStatistics.count(); ++i)
		writer->writeAttribute(QStringLiteral("willStatistics") + QString::number(i), QString::number(m_MQTTWill.willStatistics[i]));
	writer->writeAttribute(QStringLiteral("useID"), QString::number(m_MQTTUseID));
	writer->writeAttribute(QStringLiteral("useAuthentication"), QString::number(m_MQTTUseAuthentication));

	writer->writeEndElement();

	// filter
	m_filter->save(writer);

	// MQTTSubscription
	for (auto* sub : children<MQTTSubscription>(AbstractAspect::ChildIndexFlag::IncludeHidden))
		sub->save(writer);

	writer->writeEndElement(); // "MQTTClient"
}

/*!
  Loads from XML.
*/
bool MQTTClient::load(XmlStreamReader* reader, bool preview) {
	if (!readBasicAttributes(reader))
		return false;

	QString attributeWarning = i18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == QLatin1String("MQTTClient"))
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == QLatin1String("comment")) {
			if (!readCommentElement(reader))
				return false;
		} else if (reader->name() == QLatin1String("general")) {
			attribs = reader->attributes();

			str = attribs.value(QStringLiteral("subscriptionCount")).toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg(QStringLiteral("'subscriptionCount'")));
			else
				m_subscriptionCountToLoad = str.toInt();

			str = attribs.value(QStringLiteral("keepValues")).toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg(QStringLiteral("'keepValues'")));
			else
				m_keepNValues = str.toInt();

			str = attribs.value(QStringLiteral("updateType")).toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg(QStringLiteral("'updateType'")));
			else
				m_updateType = static_cast<UpdateType>(str.toInt());

			str = attribs.value(QStringLiteral("readingType")).toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg(QStringLiteral("'readingType'")));
			else
				m_readingType = static_cast<ReadingType>(str.toInt());

			if (m_updateType == UpdateType::TimeInterval) {
				str = attribs.value(QStringLiteral("updateInterval")).toString();
				if (str.isEmpty())
					reader->raiseWarning(attributeWarning.arg(QStringLiteral("'updateInterval'")));
				else
					m_updateInterval = str.toInt();
			}

			if (m_readingType != ReadingType::TillEnd) {
				str = attribs.value(QStringLiteral("sampleSize")).toString();
				if (str.isEmpty())
					reader->raiseWarning(attributeWarning.arg(QStringLiteral("'sampleSize'")));
				else
					m_sampleSize = str.toInt();
			}

			str = attribs.value(QStringLiteral("host")).toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg(QStringLiteral("'host'")));
			else
				m_client->setHostname(str);

			str = attribs.value(QStringLiteral("port")).toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg(QStringLiteral("'port'")));
			else
				m_client->setPort(str.toUInt());

			str = attribs.value(QStringLiteral("useAuthentication")).toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg(QStringLiteral("'useAuthentication'")));
			else
				m_MQTTUseAuthentication = str.toInt();

			if (m_MQTTUseAuthentication) {
				str = attribs.value(QStringLiteral("username")).toString();
				if (!str.isEmpty())
					m_client->setUsername(str);

				str = attribs.value(QStringLiteral("password")).toString();
				if (!str.isEmpty())
					m_client->setPassword(str);
			}

			str = attribs.value(QStringLiteral("useID")).toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg(QStringLiteral("'useID'")));
			else
				m_MQTTUseID = str.toInt();

			if (m_MQTTUseID) {
				str = attribs.value(QStringLiteral("clientId")).toString();
				if (!str.isEmpty())
					m_client->setClientId(str);
			}

			str = attribs.value(QStringLiteral("useRetain")).toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg(QStringLiteral("'useRetain'")));
			else
				m_MQTTRetain = str.toInt();

			str = attribs.value(QStringLiteral("useWill")).toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg(QStringLiteral("'useWill'")));
			else
				m_MQTTWill.enabled = str.toInt();

			if (m_MQTTWill.enabled) {
				str = attribs.value(QStringLiteral("willTopic")).toString();
				if (str.isEmpty())
					reader->raiseWarning(attributeWarning.arg(QStringLiteral("'willTopic'")));
				else
					m_MQTTWill.willTopic = str;

				str = attribs.value(QStringLiteral("willOwnMessage")).toString();
				if (str.isEmpty())
					reader->raiseWarning(attributeWarning.arg(QStringLiteral("'willOwnMessage'")));
				else
					m_MQTTWill.willOwnMessage = str;

				str = attribs.value(QStringLiteral("willQoS")).toString();
				if (str.isEmpty())
					reader->raiseWarning(attributeWarning.arg(QStringLiteral("'willQoS'")));
				else
					m_MQTTWill.willQoS = str.toUInt();

				str = attribs.value(QStringLiteral("willRetain")).toString();
				if (str.isEmpty())
					reader->raiseWarning(attributeWarning.arg(QStringLiteral("'willRetain'")));
				else
					m_MQTTWill.willRetain = str.toInt();

				str = attribs.value(QStringLiteral("willMessageType")).toString();
				if (str.isEmpty())
					reader->raiseWarning(attributeWarning.arg(QStringLiteral("'willMessageType'")));
				else
					m_MQTTWill.willMessageType = static_cast<MQTTClient::WillMessageType>(str.toInt());

				str = attribs.value(QStringLiteral("willUpdateType")).toString();
				if (str.isEmpty())
					reader->raiseWarning(attributeWarning.arg(QStringLiteral("'willUpdateType'")));
				else
					m_MQTTWill.willUpdateType = static_cast<MQTTClient::WillUpdateType>(str.toInt());

				str = attribs.value(QStringLiteral("willTimeInterval")).toString();
				if (str.isEmpty())
					reader->raiseWarning(attributeWarning.arg(QStringLiteral("'willTimeInterval'")));
				else
					m_MQTTWill.willTimeInterval = str.toInt();

				for (int i = 0; i < m_MQTTWill.willStatistics.count(); ++i) {
					str = attribs.value(QStringLiteral("willStatistics") + QString::number(i)).toString();
					if (str.isEmpty())
						reader->raiseWarning(attributeWarning.arg(QStringLiteral("'willTimeInterval'")));
					else
						m_MQTTWill.willStatistics[i] = str.toInt();
				}
			}
		} else if (reader->name() == QLatin1String("asciiFilter")) {
			setFilter(new AsciiFilter);
			if (!m_filter->load(reader))
				return false;
		} else if (reader->name() == QLatin1String("MQTTSubscription")) {
			auto* subscription = new MQTTSubscription(QString());
			subscription->setMQTTClient(this);
			m_MQTTSubscriptions.push_back(subscription);
			connect(subscription, &MQTTSubscription::loaded, this, &MQTTClient::subscriptionLoaded);
			if (!subscription->load(reader, preview)) {
				delete subscription;
				return false;
			}
			addChildFast(subscription);
		} else { // unknown element
			reader->raiseWarning(i18n("unknown element '%1'", reader->name().toString()));
			if (!reader->skipToEndElement())
				return false;
		}
	}

	return !reader->hasError();
}
