#ifdef HAVE_MQTT

#include "backend/datasources/MQTTSubscriptions.h"

#include "backend/datasources/MQTTTopic.h"
#include "backend/datasources/MQTTClient.h"

#include <KLocalizedString>


MQTTSubscriptions::MQTTSubscriptions(const QString& name)
	: Folder(name),
	  m_subscriptionName(name) {
	qDebug()<<"MQTTSubscriptions constructor";
}

MQTTSubscriptions::~MQTTSubscriptions() {	
}

void MQTTSubscriptions::addTopic(const QString& topicName) {
	MQTTTopic * newTopic = new MQTTTopic(0, topicName, this, false);
	m_topics.push_back(newTopic);
	qDebug()<<"Adding child topic: "+topicName;
	//this->parentAspect()->addChild(newTopic);
	addChild(newTopic);
}

const QVector<MQTTTopic*> MQTTSubscriptions::topics() {
	qDebug()<<"returning topics";
	return m_topics;
}

AbstractAspect* MQTTSubscriptions::mqttClient() const{
	return m_MQTTClient;
}


void MQTTSubscriptions::messageArrived(const QString& message, const QString& topicName){
	bool found = false;
	for(int i = 0; i < m_topics.count(); ++i) {
		if(topicName == m_topics[i]->topicName()) {
			m_topics[i]->newMessage(message);
			if(dynamic_cast<MQTTClient*> (m_MQTTClient)->updateType() == MQTTClient::UpdateType::NewData &&
					!dynamic_cast<MQTTClient*> (m_MQTTClient)->isPaused())
				m_topics[i]->read();
			found = true;
			break;
		}
	}

	if(!found) {
		addTopic(topicName);
		m_topics.last()->newMessage(message);
		if(dynamic_cast<MQTTClient*> (m_MQTTClient)->updateType() == MQTTClient::UpdateType::NewData &&
				!dynamic_cast<MQTTClient*> (m_MQTTClient)->isPaused())
			m_topics.last()->read();
	}
}

void MQTTSubscriptions::topicRead(const QString& topicName) {
	for(int i = 0; i < m_topics.count(); ++i) {
		if(topicName == m_topics[i]->topicName()) {
			m_topics[i]->read();
			break;
		}
	}
}

QString MQTTSubscriptions::subscriptionName() const {
	return m_subscriptionName;
}

void MQTTSubscriptions::save(QXmlStreamWriter* writer) const {
	writer->writeStartElement("MQTTSubscriptions");
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	//general
	writer->writeStartElement("general");
	writer->writeAttribute("subscriptionName", m_subscriptionName);
	writer->writeEndElement();

	//MQTTTopics
	for(auto* topic : children<MQTTTopic>(IncludeHidden))
		topic->save(writer);

	writer->writeEndElement(); // "MQTTSubscriptions"
}

bool MQTTSubscriptions::load(XmlStreamReader* reader, bool preview) {
	qDebug()<<"Start loading MQTTSubscripiton";
	if (!readBasicAttributes(reader))
		return false;

	QString attributeWarning = i18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == "MQTTSubscriptions")
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

		}  else if(reader->name() == "MQTTTopic") {
			qDebug()<<"Load MQTTTopic";
			MQTTTopic* topic = new MQTTTopic(0, "", this, false);
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

void MQTTSubscriptions::setMQTTClient(AbstractAspect* client) {
	m_MQTTClient = client;
}

#endif
