#ifdef HAVE_MQTT

#include "backend/datasources/MQTTSubscriptions.h"

#include "backend/datasources/MQTTTopic.h"
#include "backend/datasources/MQTTClient.h"


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

void MQTTSubscriptions::save(QXmlStreamWriter*) const {

}

bool MQTTSubscriptions::load(XmlStreamReader*, bool preview) {
	return true;
}

void MQTTSubscriptions::setMQTTClient(AbstractAspect* client) {
	m_MQTTClient = client;
}

#endif
