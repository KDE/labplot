/***************************************************************************
File		: MQTTClient.cpp
Project		: LabPlot
Description	: Represents a MQTT Client
--------------------------------------------------------------------
Copyright	: (C) 2018 Kovacs Ferencz (kferike98@gmail.com)

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

#ifdef HAVE_MQTT

#include "backend/datasources/MQTTClient.h"
#include "backend/datasources/MQTTSubscription.h"
#include "backend/datasources/MQTTTopic.h"

#include "backend/datasources/filters/AsciiFilter.h"
#include "backend/datasources/filters/FITSFilter.h"
#include "backend/datasources/filters/BinaryFilter.h"
#include "backend/core/Project.h"
#include "kdefrontend/spreadsheet/PlotDataDialog.h"

#include "commonfrontend/spreadsheet/SpreadsheetView.h"

#include "kdefrontend/datasources/MQTTErrorWidget.h"

#include <QFileInfo>
#include <QDateTime>
#include <QProcess>
#include <QDir>
#include <QMenu>
#include <QTimer>
#include <QMessageBox>

#include <QIcon>
#include <QAction>
#include <KLocalizedString>

#include <QDebug>

/*!
  \class MQTTClient
  \brief The MQTT Client connects to the broker set in ImportFileWidget.
It manages the MQTTSubscriptions, and the MQTTTopics.

  \ingroup datasources
*/
MQTTClient::MQTTClient(const QString& name)
	: Folder(name),
	  m_paused(false),
	  m_prepared(false),
	  m_keepLastValues(false),
	  m_filter(nullptr),
	  m_updateTimer(new QTimer(this)),
	  m_willTimer(new QTimer(this)),
	  m_client(new QMqttClient(this)),
	  m_mqttTest(false),
	  m_mqttRetain(false),
	  m_mqttUseWill(false),
	  m_mqttUseID(false),
	  m_loaded(false),
	  m_sampleSize(1),
	  m_keepNValues(0),
	  m_updateInterval(1000),
	  m_disconnectForWill(false),
	  m_mqttUseAuthentication(false),
	  m_subscriptionsLoaded(0),
	  m_subscriptionCountToLoad(0),
	  m_mqttFirstConnectEstablished(false) {

	qDebug()<<"MQTTClient constructor";
	connect(m_updateTimer, &QTimer::timeout, this, &MQTTClient::read);
	m_willStatistics.fill(false, 15);
	connect(m_client, &QMqttClient::connected, this, &MQTTClient::onMqttConnect);
	connect(m_willTimer, &QTimer::timeout, this, &MQTTClient::updateWillMessage);
	connect(m_client, &QMqttClient::errorChanged, this, &MQTTClient::mqttErrorChanged);
}

MQTTClient::~MQTTClient() {
	emit clientAboutToBeDeleted(m_client->hostname());
	//stop reading before deleting the objects
	pauseReading();
	qDebug()<<"destructor";
	if (m_filter)
		delete m_filter;
	qDebug()<<"delete timers";
	delete m_updateTimer;
	delete m_willTimer;
	qDebug()<<"disocnnect";
	m_client->disconnectFromHost();
	qDebug()<<"delete client";
	delete m_client;

}

/*!
 * depending on the update type, periodically or on data changes, starts the timer.
 */
void MQTTClient::ready() {
	if (m_updateType == TimeInterval)
		m_updateTimer->start(m_updateInterval);
}

/*!
 * \brief Updates the MQTTTopics of the client
 */
void MQTTClient::updateNow() {
	qDebug()<<"Update now";
	m_updateTimer->stop();
	read();
	if (m_updateType == TimeInterval && !m_paused)
		m_updateTimer->start(m_updateInterval);
}

/*!
 * \brief Continue reading from messages after it was paused.
 */
void MQTTClient::continueReading() {
	qDebug()<<"continue reading";
	m_paused = false;
	if (m_updateType == TimeInterval)
		m_updateTimer->start(m_updateInterval);
}

/*!
 * \brief Pause the reading from messages.
 */
void MQTTClient::pauseReading() {
	qDebug()<<"pause reading";
	m_paused = true;
	if (m_updateType == TimeInterval)
		m_updateTimer->stop();
}

/*!
 * \brief Sets the filter of the MQTTClient.
 *
 * \param f a pointer to the new filter
 */
void MQTTClient::setFilter(AbstractFileFilter* f) {
	m_filter = f;
}

/*!
 * \brief Returns the filter of the MQTTClient.
 */
AbstractFileFilter* MQTTClient::filter() const {
	return m_filter;
}

/*!
 * \brief Sets the MQTTclient's update interval to \c interval
 * \param interval
 */
void MQTTClient::setUpdateInterval(int interval) {
	qDebug()<<"Update interval " << interval;
	m_updateInterval = interval;
	if(!m_paused)
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
	qDebug()<<"Keep N Values" << keepNValues;
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
	qDebug()<<"Sample rate: " << sampleSize;
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
	qDebug()<<"Read Type : " << static_cast<int>(readingType);
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
void MQTTClient::setUpdateType(UpdateType updatetype) {
	qDebug()<<"Update Type : " << static_cast<int>(updatetype);
	if (updatetype == NewData) {
		m_updateTimer->stop();
	}
	m_updateType = updatetype;
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
	QIcon icon;
	icon = QIcon::fromTheme("labplot-MQTT");
	return icon;
}

/*!
 * \brief Sets the host and port for the client.
 *
 * \param host the hostname of the broker we want to connect to
 * \param port the port used by the broker
 */
void MQTTClient::setMqttClientHostPort(const QString& host, const quint16& port) {
	m_client->setHostname(host);
	m_client->setPort(port);
}

/*!
 * \brief Returns hostname of the broker the client is connected to.
 */
QString MQTTClient::clientHostName() const{
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
	m_mqttUseAuthentication = use;
}

/*!
 * \brief Returns whether the broker requires authentication or not.
 */
bool MQTTClient::mqttUseAuthentication() const {
	return m_mqttUseAuthentication;
}

/*!
 * \brief Sets the username and password for the client.
 *
 * \param username the username used for authentication
 * \param password the password used for authentication
 */
void MQTTClient::setMqttClientAuthentication(const QString& username, const QString& password) {
	m_client->setUsername(username);
	m_client->setPassword(password);
}

/*!
 * \brief Returns the username used for authentication.
 */
QString MQTTClient::clientUserName() const{
	return m_client->username();
}

/*!
 * \brief Returns the password used for authentication.
 */
QString MQTTClient::clientPassword() const{
	return m_client->password();
}

/*!
 * \brief Sets the flag on the given value.
 * If set true it means that user wants to set the client ID, otherwise it's not the case.
 *
 * \param use
 */
void MQTTClient::setMQTTUseID(bool use) {
	m_mqttUseID = use;
}

/*!
 * \brief Returns whether the user wants to set the client ID or not.
 */
bool MQTTClient::mqttUseID() const {
	return m_mqttUseID;
}

/*!
 * \brief Sets the ID of the client
 *
 * \param id
 */
void MQTTClient::setMqttClientId(const QString &id){
	m_client->setClientId(id);
}

/*!
 * \brief Returns the ID of the client
 */
QString MQTTClient::clientID () const{
	return m_client->clientId();
}

/*!
 * \brief Sets the flag on the given value.
 * If retain is true we interpret retain messages, otherwise we do not
 *
 * \param retain
 */
void MQTTClient::setMqttRetain(bool retain) {
	m_mqttRetain = retain;
}

/*!
 * \brief Returns the flag, which set to true means that interpret retain messages, otherwise we do not
 */
bool MQTTClient::mqttRetain() const {
	return m_mqttRetain;
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
void MQTTClient::addInitialMqttSubscriptions(const QMqttTopicFilter& filter, const quint8& qos) {
	m_subscribedTopicNameQoS[filter] = qos;
}

/*!
 * \brief Returns the name of every MQTTSubscription of the MQTTClient
 */
QVector<QString> MQTTClient::mqttSubscriptions() const {
	return m_subscriptions;
}

/*!
 * \brief Adds a new MQTTSubscription to the MQTTClient
 *
 * \param topic, the name of the topic
 * \param QoS
 */
void MQTTClient::addMQTTSubscription(const QString& topic, quint8 QoS) {
	//Check whether the subscription already exists, if not we can add it
	if(!m_subscriptions.contains(topic)) {
		QMqttTopicFilter filter {topic};
		QMqttSubscription* temp = m_client->subscribe(filter, QoS);

		if (temp) {
			qDebug()<<temp->topic()<<"  "<<temp->qos();
			m_subscriptions.push_back(temp->topic().filter());
			m_subscribedTopicNameQoS[temp->topic().filter()] = temp->qos();

			qDebug()<<"New MQTTSubscription";
			MQTTSubscription* newSubscription = new MQTTSubscription(temp->topic().filter());
			newSubscription->setMQTTClient(this);

			qDebug()<<"Add child";
			addChild(newSubscription);

			qDebug()<<"Add to vector";
			m_mqttSubscriptions.push_back(newSubscription);

			qDebug()<<"Added topic";

			qDebug()<<"Check for inferior subscriptions";

			//Search for inferior subscriptions, that the new subscription contains
			bool found = false;
			QVector<MQTTSubscription*> inferiorSubscriptions;
			for(int i = 0; i < m_mqttSubscriptions.size(); ++i) {
				if(checkTopicContains(topic, m_mqttSubscriptions[i]->subscriptionName())
						&& topic != m_mqttSubscriptions[i]->subscriptionName()) {
					found = true;
					inferiorSubscriptions.push_back(m_mqttSubscriptions[i]);
				}
			}

			//If there are some inferior subscripitons, we have to deal with them
			if(found) {
				for(int sub = 0; sub < inferiorSubscriptions.size(); ++sub) {
					qDebug()<<"Inferior subscription: "<<inferiorSubscriptions[sub]->subscriptionName();
					//We have to reparent every topic of the inferior subscription, so no data is lost
					QVector<MQTTTopic*> topics = inferiorSubscriptions[sub]->topics();
					qDebug()<< topics.size();
					for(int i = 0; i < topics.size() ; ++i) {
						qDebug()<<topics[i]->topicName();
						topics[i]->reparent(newSubscription);
					}

					//Then remove the subscription and every connected informaiton
					QMqttTopicFilter unsubscribeFilter {inferiorSubscriptions[sub]->subscriptionName()};
					m_client->unsubscribe(unsubscribeFilter);

					for (int j = 0; j < m_mqttSubscriptions.size(); ++j) {
						if(m_mqttSubscriptions[j]->subscriptionName() ==
								inferiorSubscriptions[sub]->subscriptionName()) {
							m_mqttSubscriptions.remove(j);
						}
					}
					m_subscriptions.removeAll(inferiorSubscriptions[sub]->subscriptionName());
					m_subscribedTopicNameQoS.remove(inferiorSubscriptions[sub]->subscriptionName());

					removeChild(inferiorSubscriptions[sub]);

				}
			}

			connect(temp, &QMqttSubscription::messageReceived, this, &MQTTClient::mqttSubscribtionMessageReceived);

			emit mqttTopicsChanged();
		}
	}
}

/*!
 * \brief Removes a MQTTSubscription from the MQTTClient
 *
 * \param name, the name of the subscription to remove
 */
void MQTTClient::removeMQTTSubscription(const QString &name) {
	//We can only remove the subscription if it exists
	if(m_subscriptions.contains(name)) {
		qDebug()<<"Start to remove subscription in MQTTClient: "<<name;

		//unsubscribe from the topic
		QMqttTopicFilter filter{name};
		m_client->unsubscribe(filter);
		qDebug()<<"QMqttClient's unsubscribe occured";

		//Remove every connected information
		m_subscriptions.removeAll(name);

		for (int i = 0; i < m_mqttSubscriptions.size(); ++i) {
			if(m_mqttSubscriptions[i]->subscriptionName() == name) {
				qDebug()<<"Subscription name"<<m_mqttSubscriptions[i]->subscriptionName() << "  "<<m_mqttSubscriptions[i]->name();
				MQTTSubscription* removeSubscription = m_mqttSubscriptions[i];
				m_mqttSubscriptions.remove(i);
				//Remove every topic of the subscription as well
				QVector<MQTTTopic*> topics = removeSubscription->topics();
				for (int j = 0; j < topics.size(); ++j) {
					qDebug()<<"Removing topic name: "<<topics[j]->topicName();
					m_topicNames.removeAll(topics[j]->topicName());
				}
				//Remove the MQTTSubscription
				removeChild(removeSubscription);
				qDebug()<<"removed child";
				break;
			}
		}

		QMapIterator<QMqttTopicFilter, quint8> j(m_subscribedTopicNameQoS);
		while(j.hasNext()) {
			j.next();
			if(j.key().filter() == name) {
				m_subscribedTopicNameQoS.remove(j.key());
				qDebug()<<"Removed from TopicNameQoS map  "<<j.key();
				break;
			}
		}

		//Signal that there was a change among the topics
		emit mqttTopicsChanged();
	}
}

/*!
 * \brief Adds a MQTTSubscription to the MQTTClient
 *Used when the user unsubscribes from a topic of a MQTTSubscription
 *
 * \param topic, the name of the topic
 * \param QoS
 */
void MQTTClient::addBeforeRemoveSubscription(const QString &topic, quint8 QoS) {
	//We can't add the subscription if it already exists
	if(!m_subscriptions.contains(topic)) {
		//Subscribe to the topic
		QMqttTopicFilter filter {topic};
		QMqttSubscription* temp = m_client->subscribe(filter, QoS);
		if (temp) {
			//Add the MQTTSubscription and other connected data
			qDebug()<<temp->topic()<<"  "<<temp->qos();
			m_subscriptions.push_back(temp->topic().filter());
			m_subscribedTopicNameQoS[temp->topic().filter()] = temp->qos();

			qDebug()<<"New MQTTSubscription";
			MQTTSubscription* newSubscription = new MQTTSubscription(temp->topic().filter());
			newSubscription->setMQTTClient(this);

			qDebug()<<"Add child";
			addChild(newSubscription);

			qDebug()<<"Add to vector";
			m_mqttSubscriptions.push_back(newSubscription);

			qDebug()<<"Added topic";

			//Search for the subscription the topic belonged to
			bool found = false;
			MQTTSubscription* superiorSubscription;

			for(int i = 0; i < m_mqttSubscriptions.size(); ++i) {
				if(checkTopicContains(m_mqttSubscriptions[i]->subscriptionName(), topic)
						&& topic != m_mqttSubscriptions[i]->subscriptionName()) {
					found = true;
					superiorSubscription = m_mqttSubscriptions[i];
					break;
				}
			}

			if(found) {
				qDebug()<<"Superior subscription: "<<superiorSubscription->subscriptionName();

				//Search for topics belonging to the superior(old) subscription
				//which are also contained by the new subscription
				QVector<MQTTTopic*> topics = superiorSubscription->topics();
				qDebug()<< topics.size();

				QVector<MQTTTopic*> inferiorTopics;
				for(int i = 0; i < topics.size(); ++i) {
					if(checkTopicContains(topic, topics[i]->topicName())) {
						inferiorTopics.push_back(topics[i]);
					}
				}

				//Reparent these topics, in order to avoid data loss
				for(int i = 0; i < inferiorTopics.size() ; ++i) {
					qDebug()<<inferiorTopics[i]->topicName();
					inferiorTopics[i]->reparent(newSubscription);
				}
			}
			connect(temp, &QMqttSubscription::messageReceived, this, &MQTTClient::mqttSubscribtionMessageReceived);
		}
	}
}

/*!
 * \brief Reparents the given MQTTTopic to the given MQTTSubscription
 *
 * \param topic, the name of the MQTTTopic
 * \param parent, the name of the MQTTSubscription
 */
void MQTTClient::reparentTopic(const QString& topic, const QString& parent) {
	//We can only reparent if the parent containd the topic
	if(m_subscriptions.contains(parent) && m_topicNames.contains(topic)) {
		//search for the parent MQTTSubscription
		bool found = false;
		MQTTSubscription* superiorSubscription;
		for(int i = 0; i < m_mqttSubscriptions.size(); ++i) {
			if(m_mqttSubscriptions[i]->subscriptionName() == parent) {
				found = true;
				superiorSubscription = m_mqttSubscriptions[i];
				break;
			}
		}

		if(found) {
			qDebug()<<"Superior subscription: "<<superiorSubscription->subscriptionName();
			//get every topic of the MQTTClient
			QVector<MQTTTopic*> topics = children<MQTTTopic>(AbstractAspect::Recursive);
			qDebug()<< topics.size();

			//Search for the given topic among the MQTTTopics
			for(int i = 0; i < topics.size(); ++i) {
				qDebug()<<topics.size()<<"  "<<i;
				qDebug()<<i<<" "<<topics[i]->topicName()<<" "<<topics[i]->parentAspect()->name();
				if(topic == topics[i]->topicName()) {
					qDebug()<<topics[i]->topicName()<<"  "<<superiorSubscription->subscriptionName();
					//if found, it is reparented to the parent MQTTSubscription
					topics[i]->reparent(superiorSubscription);
					break;
				}
			}
		}
		qDebug()<<"reparent done";
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
bool MQTTClient::checkTopicContains(const QString &superior, const QString& inferior) {
	if (superior == inferior)
		return true;
	else {
		if(superior.contains("/")) {
			QStringList superiorList = superior.split('/', QString::SkipEmptyParts);
			QStringList inferiorList = inferior.split('/', QString::SkipEmptyParts);

			//a longer topic can't contain a shorter one
			if(superiorList.size() > inferiorList.size())
				return false;

			bool ok = true;
			for(int i = 0; i < superiorList.size(); ++i) {
				if(superiorList.at(i) != inferiorList.at(i)) {
					if((superiorList.at(i) != "+") &&
							!(superiorList.at(i) == "#" && i == superiorList.size() - 1)) {
						qDebug() <<superiorList.at(i)<<"  "<<inferiorList.at(i);
						//if the two topics differ, and the superior's current level isn't + or #(which can be only in the last position)
						//then superior can't contain inferior
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
 *\brief Returns the "+" wildcard containing topic name, which includes the given topic names
 *
 * \param first the name of a topic
 * \param second the name of a topic
 * \return The name of the common topic, if it exists, otherwise ""
 */
QString MQTTClient::checkCommonLevel(const QString& first, const QString& second) {
	qDebug()<<first<<"  "<<second;
	QStringList firstList = first.split('/', QString::SkipEmptyParts);
	QStringList secondtList = second.split('/', QString::SkipEmptyParts);
	QString commonTopic = "";

	if(!firstList.isEmpty()) {
		//the two topics have to be the same size and can't be identic
		if(firstList.size() == secondtList.size() && (first != second))	{

			//the index where they differ
			int differIndex = -1;
			for(int i = 0; i < firstList.size(); ++i) {
				if(firstList.at(i) != secondtList.at(i)) {
					differIndex = i;
					break;
				}
			}

			//they can differ at only one level
			bool differ = false;
			if(differIndex > 0 && differIndex < firstList.size() -1) {
				for(int j = differIndex +1; j < firstList.size(); ++j) {
					if(firstList.at(j) != secondtList.at(j)) {
						differ = true;
						break;
					}
				}
			}
			else
				differ = true;

			if(!differ)
			{
				for(int i = 0; i < firstList.size(); ++i) {
					if(i != differIndex) {
						commonTopic.append(firstList.at(i));
					} else {
						//we put "+" wildcard at the level where they differ
						commonTopic.append("+");
					}

					if(i != firstList.size() - 1)
						commonTopic.append("/");
				}
			}
		}
	}
	qDebug() << "Common topic: "<<commonTopic;
	return commonTopic;
}

/*!
 * \brief Sets whether the user wants to use will message or not
 *
 * \param use
 */
void MQTTClient::setMqttWillUse(bool use) {
	m_mqttUseWill = use;
	if(use == false)
		m_willTimer->stop();
}

/*!
 * \brief Returns whether the user wants to use will message or not
 */
bool MQTTClient::mqttWillUse() const{
	return m_mqttUseWill;
}

/*!
 * \brief Sets the will topic of the client
 *
 * \param topic
 */
void  MQTTClient::setWillTopic(const QString& topic) {
	m_willTopic = topic;
}

/*!
 * \brief Returns the will topic of the client
 */
QString MQTTClient::willTopic() const{
	return m_willTopic;
}

/*!
 * \brief Sets the retain flag of the client's will message
 *
 * \param retain
 */
void MQTTClient::setWillRetain(bool retain) {
	m_willRetain = retain;
}

/*!
 * \brief Returns the retain flag of the client's will message
 */
bool MQTTClient::willRetain() const {
	return m_willRetain;
}

/*!
 * \brief Sets the QoS level of the client's will message
 *
 * \param QoS
 */
void MQTTClient::setWillQoS(quint8 QoS) {
	m_willQoS = QoS;
}

/*!
 * \brief Returns the QoS level of the client's will message
 */
quint8 MQTTClient::willQoS() const {
	return m_willQoS;
}

/*!
 * \brief Sets the will message type of the client
 *
 * \param messageType
 */
void MQTTClient::setWillMessageType(WillMessageType messageType) {
	m_willMessageType = messageType;
}

/*!
 * \brief Returns the will message type of the client
 */
MQTTClient::WillMessageType MQTTClient::willMessageType() const {
	return m_willMessageType;
}

/*!
 * \brief Sets the own will message of the user
 *
 * \param ownMessage
 */
void MQTTClient::setWillOwnMessage(const QString& ownMessage) {
	m_willOwnMessage = ownMessage;
}

/*!
 * \brief Returns the own will message of the user
 */
QString MQTTClient::willOwnMessage() const {
	return m_willOwnMessage;
}

/*!
 * \brief Updates the will message of the client
 */
void MQTTClient::updateWillMessage() {
	QVector<const MQTTTopic*> topics = children<const MQTTTopic>(AbstractAspect::Recursive);
	const AsciiFilter* asciiFilter = nullptr;
	const MQTTTopic* willTopic = nullptr;
	qDebug()<<"Searching for topic";
	//Search for the will topic
	for (int i = 0; i < topics.count(); ++i) {
		if(topics[i]->topicName() == m_willTopic) {
			willTopic = topics[i];
			break;
		}
	}

	//if the will topic is found we can update the will message
	if(willTopic != nullptr) {
		//To update the will message we have to disconnect first, then after setting everything connect again
		if(m_mqttUseWill && (m_client->state() == QMqttClient::ClientState::Connected) ) {
			//Disconnect only once (disconnecting may take a while)
			if(!m_disconnectForWill) {
				qDebug() << "Disconnecting from host";
				m_client->disconnectFromHost();
				m_disconnectForWill = true;
			}
			//Try to update again
			updateWillMessage();
		}
		//If client is disconnected we can update the settings
		else if(m_mqttUseWill && (m_client->state() == QMqttClient::ClientState::Disconnected) && m_disconnectForWill) {
			m_client->setWillQoS(m_willQoS);
			qDebug()<<"Will QoS" << m_willQoS;

			m_client->setWillRetain(m_willRetain);
			qDebug()<<"Will retain" << m_willRetain;

			m_client->setWillTopic(m_willTopic);
			qDebug()<<"Will Topic" << m_willTopic;

			//Set the will message according to m_willMessageType
			switch (m_willMessageType) {
			case WillMessageType::OwnMessage:
				m_client->setWillMessage(m_willOwnMessage.toUtf8());
				qDebug()<<"Will own message" << m_willOwnMessage;
				break;
			case WillMessageType::Statistics: {
				qDebug()<<"Start will statistics";
				asciiFilter = dynamic_cast<AsciiFilter*>(willTopic->filter());

				//If the topic's asciiFilter was found, get the needed statistics
				if(asciiFilter != nullptr) {
					qDebug()<<"Checking column mode";
					//Statistics is only possible if the data stored in the MQTTTopic is of type integer or numeric
					if((asciiFilter->mqttColumnMode() == AbstractColumn::ColumnMode::Integer) ||
							(asciiFilter->mqttColumnMode() == AbstractColumn::ColumnMode::Numeric)) {
						m_client->setWillMessage(asciiFilter->mqttColumnStatistics(tempTopic).toUtf8());
						qDebug() << "Will statistics message: "<< QString(m_client->willMessage());
					}
					//Otherwise set empty message
					else {
						m_client->setWillMessage(QString("").toUtf8());
						qDebug() << "Will statistics message: "<< QString(m_client->willMessage());
					}
				}
				break;
			}
			case WillMessageType::LastMessage:
				m_client->setWillMessage(m_willLastMessage.toUtf8());
				qDebug()<<"Will last message:\n" << m_willLastMessage;
				break;
			default:
				break;
			}
			m_disconnectForWill = false;
			//Reconnect with the updated message
			m_client->connectToHost();
			qDebug()<< "Reconnect to host";
		}
	}
}

/*!
 * \brief Returns the MQTTClient's will update type
 */
MQTTClient::WillUpdateType MQTTClient::willUpdateType() const{
	return m_willUpdateType;
}

/*!
 * \brief Sets the MQTTClient's will update type
 *
 * \param willUpdateType
 */
void MQTTClient::setWillUpdateType(WillUpdateType willUpdateType) {
	m_willUpdateType = willUpdateType;
}

/*!
 * \brief Returns the time interval of updating the MQTTClient's will message
 */
int MQTTClient::willTimeInterval() const{
	return m_willTimeInterval;
}

/*!
 * \brief Sets the time interval of updating the MQTTClient's will message, if update type is TimePeriod
 *
 * \param interval
 */
void MQTTClient::setWillTimeInterval(int interval) {
	m_willTimeInterval = interval;
}

/*!
 * \brief Clear the lastly received message by the will topic
 * Called when the will topic is changed
 */
void MQTTClient::clearLastMessage() {
	m_willLastMessage.clear();
}

/*!
 * \brief Sets true the corresponding flag of the statistic type,
 *  what means that the given statistic type will be added to the will message
 *
 * \param statistics
 */
void MQTTClient::addWillStatistics(WillStatistics statistic){
	m_willStatistics[static_cast<int>(statistic)] = true;
}

/*!
 * \brief Sets false the corresponding flag of the statistic type,
 * what means that the given statistic will no longer be added to the will message
 *
 * \param statistics
 */
void MQTTClient::removeWillStatistics(WillStatistics statistic) {
	m_willStatistics[static_cast<int>(statistic)] = false;
}

/*!
 * \brief Returns a bool vector, meaning which statistic types are included in the will message
 * If the corresponding value is true, the statistic type is included, otherwise it isn't
 */
QVector<bool> MQTTClient::willStatistics() const{
	return m_willStatistics;
}

/*!
 * \brief Starts the will timer, which will update the will message
 */
void MQTTClient::startWillTimer() const{
	if(m_willUpdateType == WillUpdateType::TimePeriod)
		m_willTimer->start(m_willTimeInterval);
}

/*!
 * \brief Stops the will timer
 */
void MQTTClient::stopWillTimer() const{
	m_willTimer->stop();
}

//##############################################################################
//#################################  SLOTS  ####################################
//##############################################################################

/*!
 *\brief called periodically when update type is TimeInterval
 */
void MQTTClient::read() {
	if (m_filter == nullptr)
		return;

	if (!m_prepared) {
		qDebug()<<"Read & Connect";
		//connect to the broker
		m_client->connectToHost();
		qDebug()<<"connectTOHost called";
		m_prepared = true;
	}

	if((m_client->state() == QMqttClient::ClientState::Connected) && m_mqttFirstConnectEstablished) {
		qDebug()<<"Read";
		//Signal for every MQTTTopic that they can read
		emit readFromTopics();
	}
}

/*!
 *\brief called when the client successfully connected to the broker
 */
void MQTTClient::onMqttConnect() {
	qDebug() << "on mqtt connect";
	if(m_client->error() == QMqttClient::NoError) {
		//if this is the first connection (after setting the options in ImportFileWidget or loading saved project)
		if(!m_mqttFirstConnectEstablished) {
			qDebug()<<"connection made in MQTTClient";

			//Subscribe to initial or loaded topics
			QMapIterator<QMqttTopicFilter, quint8> i(m_subscribedTopicNameQoS);
			while(i.hasNext()) {
				i.next();
				qDebug()<<i.key();
				QMqttSubscription *temp = m_client->subscribe(i.key(), i.value());
				if(temp) {
					qDebug()<<temp->topic()<<"  "<<temp->qos();
					//If we didn't load the MQTTClient from xml we have to add the MQTTSubscriptions
					if(!m_loaded) {
						m_subscriptions.push_back(temp->topic().filter());

						qDebug()<<"New MQTTSubscription";
						MQTTSubscription* newSubscription = new MQTTSubscription(temp->topic().filter());
						newSubscription->setMQTTClient(this);

						qDebug()<<"Add child";
						addChild(newSubscription);

						qDebug()<<"Add to vector";
						m_mqttSubscriptions.push_back(newSubscription);
					}

					connect(temp, &QMqttSubscription::messageReceived, this, &MQTTClient::mqttSubscribtionMessageReceived);
					qDebug()<<"Added topic";
				}
			}
			m_mqttFirstConnectEstablished = true;
			//Signal that the initial subscriptions were made
			emit mqttSubscribed();
		}
		//if there was already a connection made(happens after updating will message)
		else {
			qDebug() << "Resubscribing after will set";
			//Only the client has to make the subscriptions again, every other connected data is still avialable
			QMapIterator<QMqttTopicFilter, quint8> i(m_subscribedTopicNameQoS);
			while(i.hasNext()) {
				i.next();
				QMqttSubscription *temp = m_client->subscribe(i.key(), i.value());
				if(temp) {
					qDebug()<<temp->topic()<<"  "<<temp->qos();
					connect(temp, &QMqttSubscription::messageReceived, this, &MQTTClient::mqttSubscribtionMessageReceived);
				} else
					qDebug()<<"Couldn't subscribe after will change";
			}
		}
	}
}

/*!
 *\brief called when a message is received by a topic belonging to one of subscriptions of the client.
 * It passes the message to the appropriate MQTTSubscription which will pass it to the appropriate MQTTTopic
 */
void MQTTClient::mqttSubscribtionMessageReceived(const QMqttMessage& msg) {
	//Decide to interpret retain message or not
	if(!msg.retain() || (msg.retain() && m_mqttRetain) ) {
		qDebug()<<"message received from "<<msg.topic().name();
		//If this is the first message from the topic, save its name
		if(!m_topicNames.contains(msg.topic().name())) {
			m_topicNames.push_back(msg.topic().name());
			//Signal that a new topic is found
			emit mqttTopicsChanged();
		}

		//Pass the message and the topic name to the MQTTSubscription which contains the topic
		for(int i = 0; i < m_mqttSubscriptions.count(); ++i){
			if(checkTopicContains(m_mqttSubscriptions[i]->subscriptionName(), msg.topic().name())) {
				m_mqttSubscriptions[i]->messageArrived(QString(msg.payload()), msg.topic().name());
				break;
			}
		}

		//if the message was received by the will topic, update the last message received by it
		if(msg.topic().name() == m_willTopic)
			m_willLastMessage = QString(msg.payload());
	}
}

/*!
 *\brief Handles some of the possible errors of the client, using MQTTErrorWidget
 */
void MQTTClient::mqttErrorChanged(QMqttClient::ClientError clientError) {
	if(clientError != QMqttClient::ClientError::NoError) {
		MQTTErrorWidget* errorWidget = new MQTTErrorWidget(clientError, this);
		errorWidget->show();
	}
}

/*!
 *\brief Called when a subscription is loaded.
 * Checks whether every saved subscription was loaded or not.
 * If everything is loaded, it makes the conneciton and starts the reading
 *
 * \param name, the name of the subscription
 */
void MQTTClient::subscriptionLoaded(const QString &name) {
	qDebug()<<name;
	if(!name.isEmpty()) {
		qDebug()<<"name not empty";
		//Save information about the subscription
		m_subscriptionsLoaded++;
		m_subscriptions.push_back(name);
		QMqttTopicFilter filter {name};
		m_subscribedTopicNameQoS[filter] = 0;

		qDebug()<<m_subscriptionsLoaded<< " "<<m_subscriptionCountToLoad;
		//Check whether every subscription was loaded or not
		if(m_subscriptionsLoaded == m_subscriptionCountToLoad) {
			//if everything was loaded we can start reading
			m_loaded = true;
			read();
		}
	}
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
/*!
  Saves as XML.
 */
void MQTTClient::save(QXmlStreamWriter* writer) const {
	writer->writeStartElement("MQTTClient");
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	//general
	writer->writeStartElement("general");
	writer->writeAttribute("subscriptionCount", QString::number(m_mqttSubscriptions.size()));
	writer->writeAttribute("updateType", QString::number(m_updateType));
	writer->writeAttribute("readingType", QString::number(m_readingType));
	writer->writeAttribute("keepValues", QString::number(m_keepNValues));

	if (m_updateType == TimeInterval)
		writer->writeAttribute("updateInterval", QString::number(m_updateInterval));

	if (m_readingType != TillEnd)
		writer->writeAttribute("sampleSize", QString::number(m_sampleSize));

	writer->writeAttribute("host", m_client->hostname());
	writer->writeAttribute("port", QString::number(m_client->port()));
	writer->writeAttribute("username", m_client->username());
	writer->writeAttribute("pasword", m_client->password());
	writer->writeAttribute("clientId", m_client->clientId());
	writer->writeAttribute("useRetain", QString::number(m_mqttRetain));
	writer->writeAttribute("useWill", QString::number(m_mqttUseWill));
	writer->writeAttribute("willTopic", m_willTopic);
	writer->writeAttribute("willOwnMessage", m_willOwnMessage);
	writer->writeAttribute("willQoS", QString::number(m_willQoS));
	writer->writeAttribute("willRetain", QString::number(m_willRetain));
	writer->writeAttribute("willMessageType", QString::number(static_cast<int>(m_willMessageType)));
	writer->writeAttribute("willUpdateType", QString::number(static_cast<int>(m_willUpdateType)));
	writer->writeAttribute("willTimeInterval", QString::number(m_willTimeInterval));

	for( int i = 0; i < m_willStatistics.count(); ++i){
		writer->writeAttribute("willStatistics"+QString::number(i), QString::number(m_willStatistics[i]));
	}
	writer->writeAttribute("useID", QString::number(m_mqttUseID));
	writer->writeAttribute("useAuthentication", QString::number(m_mqttUseAuthentication));

	writer->writeEndElement();

	//filter
	m_filter->save(writer);

	//MQTTSubscription
	for(auto* sub : children<MQTTSubscription>(IncludeHidden))
		sub->save(writer);

	writer->writeEndElement(); // "MQTTClient"
}

/*!
  Loads from XML.
*/
bool MQTTClient::load(XmlStreamReader* reader, bool preview) {
	qDebug()<<"Start loading MQTTClient";
	if (!readBasicAttributes(reader))
		return false;

	QString attributeWarning = i18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == "MQTTClient")
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == "comment") {
			if (!readCommentElement(reader))
				return false;
		} else if (reader->name() == "general") {
			qDebug()<<"MQTTClient general";
			attribs = reader->attributes();

			str = attribs.value("subscriptionCount").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'subscriptionCount'"));
			else
				m_subscriptionCountToLoad =  str.toInt();

			str = attribs.value("keepValues").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'keepValues'"));
			else
				m_keepNValues =  str.toInt();

			str = attribs.value("updateType").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'updateType'"));
			else
				m_updateType =  static_cast<UpdateType>(str.toInt());

			str = attribs.value("readingType").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'readingType'"));
			else
				m_readingType =  static_cast<ReadingType>(str.toInt());

			if (m_updateType == TimeInterval) {
				str = attribs.value("updateInterval").toString();
				if(str.isEmpty())
					reader->raiseWarning(attributeWarning.arg("'updateInterval'"));
				else
					m_updateInterval = str.toInt();
			}

			if (m_readingType != TillEnd) {
				str = attribs.value("sampleSize").toString();
				if(str.isEmpty())
					reader->raiseWarning(attributeWarning.arg("'sampleSize'"));
				else
					m_sampleSize = str.toInt();
			}

			str = attribs.value("host").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'host'"));
			else
				m_client->setHostname(str);

			str =attribs.value("port").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'port'"));
			else
				m_client->setPort(str.toUInt());

			str = attribs.value("useAuthentication").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'useAuthentication'"));
			else
				m_mqttUseAuthentication = str.toInt();

			if(m_mqttUseAuthentication) {
				str =attribs.value("username").toString();
				if(!str.isEmpty())
					m_client->setUsername(str);

				str =attribs.value("password").toString();
				if(!str.isEmpty())
					m_client->setPassword(str);
			}

			str = attribs.value("useID").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'useID'"));
			else
				m_mqttUseID = str.toInt();

			if(m_mqttUseID) {
				str =attribs.value("clientId").toString();
				if(!str.isEmpty())
					m_client->setClientId(str);
			}

			str =attribs.value("useRetain").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'useRetain'"));
			else
				m_mqttRetain = str.toInt();

			str =attribs.value("useWill").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'useWill'"));
			else
				m_mqttUseWill = str.toInt();

			if(m_mqttUseWill) {
				str =attribs.value("willTopic").toString();
				if(str.isEmpty())
					reader->raiseWarning(attributeWarning.arg("'willTopic'"));
				else
					m_willTopic = str;

				str =attribs.value("willOwnMessage").toString();
				if(str.isEmpty())
					reader->raiseWarning(attributeWarning.arg("'willOwnMessage'"));
				else
					m_willOwnMessage = str;

				str =attribs.value("willQoS").toString();
				if(str.isEmpty())
					reader->raiseWarning(attributeWarning.arg("'willQoS'"));
				else
					m_willQoS = str.toUInt();

				str =attribs.value("willRetain").toString();
				if(str.isEmpty())
					reader->raiseWarning(attributeWarning.arg("'willRetain'"));
				else
					m_willRetain = str.toInt();

				str =attribs.value("willMessageType").toString();
				if(str.isEmpty())
					reader->raiseWarning(attributeWarning.arg("'willMessageType'"));
				else
					m_willMessageType = static_cast<MQTTClient::WillMessageType>(str.toInt());

				str =attribs.value("willUpdateType").toString();
				if(str.isEmpty())
					reader->raiseWarning(attributeWarning.arg("'willUpdateType'"));
				else
					m_willUpdateType = static_cast<MQTTClient::WillUpdateType>(str.toInt());

				str =attribs.value("willTimeInterval").toString();
				if(str.isEmpty())
					reader->raiseWarning(attributeWarning.arg("'willTimeInterval'"));
				else
					m_willTimeInterval = str.toInt();

				for( int i = 0; i < m_willStatistics.count(); ++i){
					str =attribs.value("willStatistics"+QString::number(i)).toString();
					if(str.isEmpty())
						reader->raiseWarning(attributeWarning.arg("'willTimeInterval'"));
					else
						m_willStatistics[i] = str.toInt();
				}
			}

		} else if (reader->name() == "asciiFilter") {
			qDebug()<<"load filter";
			m_filter = new AsciiFilter();
			if (!m_filter->load(reader))
				return false;
		} else if(reader->name() == "MQTTSubscription") {
			qDebug()<<"Load MQTTSubscription";
			MQTTSubscription* subscription = new MQTTSubscription("");
			subscription->setMQTTClient(this);
			connect(subscription, &MQTTSubscription::loaded, this, &MQTTClient::subscriptionLoaded);
			if (!subscription->load(reader, preview)) {
				delete subscription;
				return false;
			}
			m_mqttSubscriptions.push_back(subscription);
			addChildFast(subscription);
		} else {// unknown element
			reader->raiseWarning(i18n("unknown element '%1'", reader->name().toString()));
			if (!reader->skipToEndElement()) return false;
		}
	}

	return !reader->hasError();
}
#endif
