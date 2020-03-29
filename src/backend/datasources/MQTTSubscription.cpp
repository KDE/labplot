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
MQTTSubscription::MQTTSubscription(const QString& name) : Folder(name, AspectType::MQTTSubscription),
	m_subscriptionName(name) {
	qDebug() << "New MQTTSubscription: " << name;
}

MQTTSubscription::~MQTTSubscription() {
	qDebug() << "Delete MQTTSubscription: " << m_subscriptionName;
}

/*!
 *\brief Returns the object's MQTTTopic children
 *
 * \return a vector of pointers to the children of the MQTTSubscription
 */
const QVector<MQTTTopic*> MQTTSubscription::topics() const {
	return children<MQTTTopic>();
}

/*!
 *\brief Returns the object's parent
 *
 * \return a pointer to the parent MQTTTopic of the object
 */
MQTTClient* MQTTSubscription::mqttClient() const {
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
void MQTTSubscription::messageArrived(const QString& message, const QString& topicName) {
	bool found = false;
	QVector<MQTTTopic*> topics = children<MQTTTopic>();
	//search for the topic among the MQTTTopic children
	for (auto* topic: topics) {
		if (topicName == topic->topicName()) {
			//pass the message to the topic
			topic->newMessage(message);

			//read the message if needed
			if ((m_MQTTClient->updateType() == MQTTClient::UpdateType::NewData) &&
			        !m_MQTTClient->isPaused())
				topic->read();

			found = true;
			break;
		}
	}

	//if the topic can't be found, we add it as a new MQTTTopic, and read from it if needed
	if (!found) {
		MQTTTopic* newTopic = new MQTTTopic(topicName, this, false);
		addChildFast(newTopic); //no need for undo/redo here
		newTopic->newMessage(message);
		if ((m_MQTTClient->updateType() == MQTTClient::UpdateType::NewData) && !m_MQTTClient->isPaused())
			newTopic->read();
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
	return QIcon::fromTheme("mail-signed-full");
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
	for (auto* topic : children<MQTTTopic>(IncludeHidden))
		topic->save(writer);

	writer->writeEndElement(); // "MQTTSubscription"
}

/*!
  Loads from XML.
*/
bool MQTTSubscription::load(XmlStreamReader* reader, bool preview) {
	if (!readBasicAttributes(reader))
		return false;

	QXmlStreamAttributes attribs;

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
			attribs = reader->attributes();
			m_subscriptionName = attribs.value("subscriptionName").toString();
		} else if(reader->name() == QLatin1String("MQTTTopic")) {
			MQTTTopic* topic = new MQTTTopic(QString(), this, false);
			if (!topic->load(reader, preview)) {
				delete topic;
				return false;
			}
			addChildFast(topic);
		} else {// unknown element
			reader->raiseWarning(i18n("unknown element '%1'", reader->name().toString()));
			if (!reader->skipToEndElement()) return false;
		}
	}

	emit loaded(this->subscriptionName());
	return !reader->hasError();
}

