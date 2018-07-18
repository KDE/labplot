/***************************************************************************
File		: MQTTSubscription.cpp
Project		: LabPlot
Description	: Represents a subscription made in MQTTClient
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

#include "backend/datasources/MQTTSubscription.h"
#include "backend/datasources/MQTTTopic.h"
#include "backend/datasources/MQTTClient.h"

#include <KLocalizedString>
#include <QIcon>

/*!
  \class MQTTSubscription
  \brief Represents a subscription made in a MQTTClient object. It plays a role in managing MQTTTopic objects
  and makes possible representing the subscriptions and topics in a tree like structure

  \ingroup datasources
*/
MQTTSubscription::MQTTSubscription(const QString& name)
	: Folder(name),
	  m_subscriptionName(name) {
	qDebug()<<"MQTTSubscription constructor";
}

MQTTSubscription::~MQTTSubscription() {
	qDebug()<<"MQTTSubscription destructor";
}

/*!
 *\brief Adds an MQTTTopic as a child
 *
 * \param topicName the name of the topic, which will be added to the tree widget
 */
void MQTTSubscription::addTopic(const QString& topicName) {
	MQTTTopic * newTopic = new MQTTTopic(topicName, this, false);
	m_topics.push_back(newTopic);
	qDebug()<<"Adding child topic: "+topicName;
	addChild(newTopic);
}

/*!
 *\brief Returns the object's MQTTTopic children
 *
 * \return a vector of pointers to the children of the MQTTSubscription
 */
const QVector<MQTTTopic*> MQTTSubscription::topics() {
	qDebug()<<"returning topics";
	return  children<MQTTTopic>();
}

/*!
 *\brief Returns the object's parent
 *
 * \return a pointer to the parent MQTTTopic of the object
 */
MQTTClient* MQTTSubscription::mqttClient() const{
	return m_MQTTClient;
}

/*!
 *\brief Called when a message arrived to a topic contained by the MQTTSubscription
 * If the topic can't be found among the children, a new MQTTTopic is instantiated
 * Passes the messages to the appropriate MQTTTopic
 *
 * \param message the message to pass
 * \param topicName the name of the topic the message was sent to
 */
void MQTTSubscription::messageArrived(const QString& message, const QString& topicName){
	bool found = false;
	QVector<MQTTTopic*> topics = children<MQTTTopic>();
	//search for the topic among the MQTTTopic children
	for(int i = 0; i < topics.size(); ++i) {
		if(topicName == topics[i]->topicName()) {
			//pass the message to the topic
			topics[i]->newMessage(message);

			//read the message if needed
			if((m_MQTTClient->updateType() == MQTTClient::UpdateType::NewData) &&
					!m_MQTTClient->isPaused())
				topics[i]->read();

			//add topic to m_topics if it isn't part of it
			bool addKnown = true;
			for(int j = 0; j < m_topics.size(); ++j) {
				if(m_topics[j]->topicName() == topics[i]->topicName()) {
					addKnown = false;
					break;
				}
			}
			if(addKnown)
				m_topics.push_back(topics[i]);

			found = true;
			break;
		}
	}

	//if the topic can't be found, we add it as a new MQTTTopic, and read from it if needed
	if(!found) {
		addTopic(topicName);
		m_topics.last()->newMessage(message);
		if((m_MQTTClient->updateType() == MQTTClient::UpdateType::NewData) &&
				!m_MQTTClient->isPaused())
			m_topics.last()->read();
	}
}

/*!
 *\brief Returns the subscription's name
 *
 * \return m_subscriptionName
 */
QString MQTTSubscription::subscriptionName() const {
	return m_subscriptionName;
}

/*!
 *\brief Sets the MQTTClient the subscription belongs to
 *
 * \param client
 */
void MQTTSubscription::setMQTTClient(MQTTClient* client) {
	m_MQTTClient = client;
}

/*!
 *\brief Returns the icon of MQTTSubscription
 */
QIcon MQTTSubscription::icon() const {
	QIcon icon;
	icon = QIcon::fromTheme("labplot-MQTT");
	return icon;
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
/*!
  Saves as XML.
 */
void MQTTSubscription::save(QXmlStreamWriter* writer) const {
	writer->writeStartElement("MQTTSubscription");
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	//general
	writer->writeStartElement("general");
	writer->writeAttribute("subscriptionName", m_subscriptionName);
	writer->writeEndElement();

	//MQTTTopics
	for(auto* topic : children<MQTTTopic>(IncludeHidden))
		topic->save(writer);

	writer->writeEndElement(); // "MQTTSubscription"
}

/*!
  Loads from XML.
*/
bool MQTTSubscription::load(XmlStreamReader* reader, bool preview) {
	qDebug()<<"Start loading MQTTSubscripiton";
	if (!readBasicAttributes(reader))
		return false;

	QString attributeWarning = i18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == "MQTTSubscription")
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == "comment") {
			if (!readCommentElement(reader))
				return false;
		} else if (reader->name() == "general") {
			qDebug()<<"MQTTSub general";
			attribs = reader->attributes();

			str = attribs.value("subscriptionName").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'subscriptionName'"));
			else {
				qDebug()<<str;
				m_subscriptionName =  str;
				setName(str);
			}

		}  else if(reader->name() == QLatin1String("MQTTTopic")) {
			qDebug()<<"Load MQTTTopic";
			MQTTTopic* topic = new MQTTTopic("", this, false);
			if (!topic->load(reader, preview)) {
				delete topic;
				return false;
			}
			m_topics.push_back(topic);
			addChildFast(topic);
		} else {// unknown element
			reader->raiseWarning(i18n("unknown element '%1'", reader->name().toString()));
			if (!reader->skipToEndElement()) return false;
		}
	}
	qDebug()<<"End loading MQTTSubscripiton";
	qDebug()<<name();
	loaded(this->name());
	return !reader->hasError();
}
#endif
