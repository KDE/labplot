#ifdef HAVE_MQTT

#include "backend/datasources/MQTTClient.h"
#include "backend/datasources/MQTTSubscriptions.h"
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
#include <QFileSystemWatcher>
#include <QFile>
#include <QTimer>
#include <QMessageBox>

#include <QIcon>
#include <QAction>
#include <KLocalizedString>

#include <QDebug>

/*!
  \class MQTTClient
  \brief Represents data stored in a file. Reading and writing is done with the help of appropriate I/O-filters.

  \ingroup datasources
*/
MQTTClient::MQTTClient(const QString& name)
	: Folder(name),
	  m_fileType(Ascii),
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
	  m_mqttUseAuthentication(false),
	  m_mqttFirstConnectEstablished(false) {

	//initActions();
	qDebug()<<"MQTTClient constructor";
	connect(m_updateTimer, &QTimer::timeout, this, &MQTTClient::read);
	m_willStatistics.fill(false, 15);
	connect(m_client, &QMqttClient::connected, this, &MQTTClient::onMqttConnect);
	connect(m_willTimer, &QTimer::timeout, this, &MQTTClient::setWillForMqtt);
	connect(m_client, &QMqttClient::errorChanged, this, &MQTTClient::mqttErrorChanged);
	//connect(this, &MQTTClient::mqttAllArrived, this, &MQTTClient::onAllArrived );

}

MQTTClient::~MQTTClient() {
	//stop reading before deleting the objects
	pauseReading();
	qDebug()<<"destructor";
	if (m_filter)
		delete m_filter;

	delete m_updateTimer;
	delete m_willTimer;
	m_client->disconnectFromHost();
	delete m_client;

}

/*!
 * depending on the update type, periodically or on data changes, starts the timer or activates the file watchers, respectively.
 */
void MQTTClient::ready() {
	if (m_updateType == TimeInterval)
		m_updateTimer->start(m_updateInterval);
}

/*void MQTTClient::initActions() {
	m_reloadAction = new QAction(QIcon::fromTheme("view-refresh"), i18n("Reload"), this);
	connect(m_reloadAction, &QAction::triggered, this, &MQTTClient::read);

	m_toggleLinkAction = new QAction(i18n("Link the file"), this);
	m_toggleLinkAction->setCheckable(true);
	connect(m_toggleLinkAction, &QAction::triggered, this, &MQTTClient::linkToggled);

	m_plotDataAction = new QAction(QIcon::fromTheme("office-chart-line"), i18n("Plot data"), this);
	connect(m_plotDataAction, &QAction::triggered, this, &MQTTClient::plotData);
}*/


/*!
 * \brief Updates this data source at this moment
 */
void MQTTClient::updateNow() {
	qDebug()<<"Update now";
	m_updateTimer->stop();
	read();
	if (m_updateType == TimeInterval && !m_paused)
		m_updateTimer->start(m_updateInterval);
}

/*!
 * \brief Continue reading from the live data source after it was paused.
 */
void MQTTClient::continueReading() {
	qDebug()<<"continue reading";
	m_paused = false;
	if (m_updateType == TimeInterval)
		m_updateTimer->start(m_updateInterval);
	else if (m_updateType == NewData) {
		connect(this, &MQTTClient::mqttAllArrived, this, &MQTTClient::onAllArrived);
	}
}

/*!
 * \brief Pause the reading of the live data source.
 */
void MQTTClient::pauseReading() {
	qDebug()<<"pause reading";
	m_paused = true;
	if (m_updateType == TimeInterval)
		m_updateTimer->stop();
	else if (m_updateType == NewData) {
		disconnect(this, &MQTTClient::mqttAllArrived, this, &MQTTClient::onAllArrived);
	}
}

/*!
  returns the list with all supported data file formats.
*/
QStringList MQTTClient::fileTypes() {
	// see MQTTClient::FileType
	return (QStringList()<< i18n("ASCII data")
			<< i18n("Binary data")
			<< i18n("Image")
			<< i18n("Hierarchical Data Format 5 (HDF5)")
			<< i18n("Network Common Data Format (NetCDF)")
			//		<< "CDF"
			<< i18n("Flexible Image Transport System Data Format (FITS)")
			//		<< i18n("Sound")
			);
}


void MQTTClient::setFileType(FileType type) {
	m_fileType = type;
}

MQTTClient::FileType MQTTClient::fileType() const {
	return m_fileType;
}

void MQTTClient::setFilter(AbstractFileFilter* f) {
	m_filter = f;
}

AbstractFileFilter* MQTTClient::filter() const {
	return m_filter;
}

/*!
 * \brief Sets whether we'll keep the last values or append it to the previous ones
 * \param keepLastValues
 */
void MQTTClient::setKeepLastValues(bool keepLastValues) {
	qDebug()<<"KeepLastValues " << keepLastValues;
	m_keepLastValues = keepLastValues;
}

bool MQTTClient::keepLastValues() const {
	return m_keepLastValues;
}

/*!
 * \brief Sets the source's update interval to \c interval
 * \param interval
 */
void MQTTClient::setUpdateInterval(int interval) {
	qDebug()<<"Update interval " << interval;
	m_updateInterval = interval;
	if(!m_paused)
		m_updateTimer->start(m_updateInterval);
}

int MQTTClient::updateInterval() const {
	return m_updateInterval;
}

/*!
 * \brief Sets how many values we should store
 * \param keepnvalues
 */
void MQTTClient::setKeepNvalues(int keepnvalues) {
	qDebug()<<"Keep N values" << keepnvalues;
	m_keepNvalues = keepnvalues;
}

int MQTTClient::keepNvalues() const {
	return m_keepNvalues;
}

bool MQTTClient::isPaused() const {
	return m_paused;
}

/*!
 * \brief Sets the sample rate to samplerate
 * \param samplerate
 */
void MQTTClient::setSampleRate(int samplerate) {
	qDebug()<<"Sample rate: " << samplerate;
	m_sampleRate = samplerate;
}

int MQTTClient::sampleRate() const {
	return m_sampleRate;
}

/*!
 * \brief Sets the source's reading type to readingType
 * \param readingType
 */
void MQTTClient::setReadingType(ReadingType readingType) {
	qDebug()<<"Read Type : " << static_cast<int>(readingType);
	m_readingType = readingType;
}

MQTTClient::ReadingType MQTTClient::readingType() const {
	return m_readingType;
}

/*!
 * \brief Sets the source's update type to updatetype and handles this change
 * \param updatetype
 */
void MQTTClient::setUpdateType(UpdateType updatetype) {
	qDebug()<<"Update Type : " << static_cast<int>(updatetype);
	if (updatetype == NewData) {
		m_updateTimer->stop();
		connect(this, &MQTTClient::mqttAllArrived, this, &MQTTClient::onAllArrived)	;
	} else {
		if (m_updateType == UpdateType::NewData)
			disconnect(this, &MQTTClient::mqttAllArrived, this, &MQTTClient::onAllArrived)	;
	}
	m_updateType = updatetype;
}

MQTTClient::UpdateType MQTTClient::updateType() const {
	return m_updateType;
}

/*QIcon MQTTClient::icon() const {
	QIcon icon;
	if (m_fileType == MQTTClient::Ascii)
		icon = QIcon::fromTheme("text-plain");
	else if (m_fileType == MQTTClient::Binary)
		icon = QIcon::fromTheme("application-octet-stream");
	else if (m_fileType == MQTTClient::Image)
		icon = QIcon::fromTheme("image-x-generic");
	// TODO: HDF5, NetCDF, FITS, etc.

	return icon;
}*/

/*QMenu* MQTTClient::createContextMenu() {
	QMenu* menu = AbstractPart::createContextMenu();

	QAction* firstAction = 0;
	// if we're populating the context menu for the project explorer, then
	//there're already actions available there. Skip the first title-action
	//and insert the action at the beginning of the menu.
	if (menu->actions().size()>1)
		firstAction = menu->actions().at(1);

	menu->insertAction(firstAction, m_plotDataAction);
	menu->insertSeparator(firstAction);

	//TODO: doesnt' always make sense...
	// 	if (!m_fileWatched)
	// 		menu->insertAction(firstAction, m_reloadAction);
	//
	// 	m_toggleWatchAction->setChecked(m_fileWatched);
	// 	menu->insertAction(firstAction, m_toggleWatchAction);
	//
	// 	m_toggleLinkAction->setChecked(m_fileLinked);
	// 	menu->insertAction(firstAction, m_toggleLinkAction);

	return menu;
}*/

//##############################################################################
//#################################  SLOTS  ####################################
//##############################################################################

/*
 * called periodically or on new data changes (file changed, new data in the socket, etc.)
 */
void MQTTClient::read() {
	if (m_filter == nullptr)
		return;

	//initialize the device (file, socket, serial port), when calling this function for the first time
	if (!m_prepared) {
		qDebug()<<"Read & Connect";
		m_client->connectToHost();
		qDebug()<<"connectTOHost called";
		m_prepared = true;
	}

	if((m_client->state() == QMqttClient::ClientState::Connected) && m_mqttFirstConnectEstablished) {
		qDebug()<<"Read";
		/*for(int i = 0; i <= m_mqttSubscriptions.count(); ++i) {
			qDebug()<<"Uploading topics";
			QVector<MQTTTopic*> topics = m_mqttSubscriptions[i]->topics();
			qDebug()<<"reading from topics";
			for(int j = 0; j < topics.count(); ++j)	{
				qDebug()<<"Trying to read from topic: "<<j;
				topics[j]->read();
			}
		}*/
		emit readFromTopics();
	}
}

/*!
  Saves as XML.
 */
void MQTTClient::save(QXmlStreamWriter* writer) const {
	writer->writeStartElement("MQTTClient");
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	//general
	writer->writeStartElement("general");
	writer->writeAttribute("fileType", QString::number(m_fileType));
	writer->writeAttribute("updateType", QString::number(m_updateType));
	writer->writeAttribute("readingType", QString::number(m_readingType));
	writer->writeAttribute("keepValues", QString::number(m_keepNvalues));

	if (m_updateType == TimeInterval)
		writer->writeAttribute("updateInterval", QString::number(m_updateInterval));

	if (m_readingType != TillEnd)
		writer->writeAttribute("sampleRate", QString::number(m_sampleRate));

	writer->writeAttribute("host", m_client->hostname());
	writer->writeAttribute("port", QString::number(m_client->port()));
	writer->writeAttribute("username", m_client->username());
	writer->writeAttribute("pasword", m_client->password());
	writer->writeAttribute("clientId", m_client->clientId());
	writer->writeAttribute("subscriptionNumber", QString::number(m_subscriptions.count()) );
	for(int i = 0; i<m_subscriptions.count(); i++) {
		writer->writeAttribute("subscription"+QString::number(i), m_subscriptions[i]);
		writer->writeAttribute("subscription"+QString::number(i)+"Qos", QString::number(m_topicMap[m_subscriptions[i]]));
	}
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
		if (reader->isEndElement() && reader->name() == "MQTTClient")
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == "comment") {
			if (!readCommentElement(reader))
				return false;
		} else if (reader->name() == "general") {
			attribs = reader->attributes();

			str = attribs.value("fileType").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'fileType'"));
			else
				m_fileType = (FileType)str.toInt();

			str = attribs.value("updateType").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'updateType'"));
			else
				m_updateType =  static_cast<UpdateType>(str.toInt());

			str = attribs.value("sourceType").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'sourceType'"));
			else
				m_sourceType =  static_cast<SourceType>(str.toInt());

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
				str = attribs.value("sampleRate").toString();
				if(str.isEmpty())
					reader->raiseWarning(attributeWarning.arg("'sampleRate'"));
				else
					m_sampleRate = str.toInt();
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
				reader->raiseWarning(attributeWarning.arg("'useAuthentication"));
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
				reader->raiseWarning(attributeWarning.arg("'useID"));
			else
				m_mqttUseID = str.toInt();

			if(m_mqttUseID) {
				str =attribs.value("clientId").toString();
				if(!str.isEmpty())
					m_client->setClientId(str);
			}

			int subscribtions;
			str =attribs.value("subscriptionNumber").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'subscriptionNumber'"));
			else
				subscribtions = str.toInt();

			for (int i = 0; i < subscribtions; i++) {
				str =attribs.value("subscription"+QString::number(i)).toString();
				if(!str.isEmpty())
					reader->raiseWarning(attributeWarning.arg("'subscription"+QString::number(i)+"'"));
				else {
					m_subscriptions.push_back(str);
				}

				str =attribs.value("subscription"+QString::number(i)+"Qos").toString();
				if(!str.isEmpty())
					reader->raiseWarning(attributeWarning.arg("'subscription"+QString::number(i)+"Qos'"));
				else {
					m_topicMap[m_subscriptions[i]] = str.toUInt();
				}

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

		} else if (reader->name() == "asciiFilter") {
			m_filter = new AsciiFilter();
			if (!m_filter->load(reader))
				return false;
		} else {// unknown element
			reader->raiseWarning(i18n("unknown element '%1'", reader->name().toString()));
			if (!reader->skipToEndElement()) return false;
		}
	}

	return !reader->hasError();
}


void MQTTClient::setMqttClient(const QString& host, const quint16& port) {
	m_client->setHostname(host);
	m_client->setPort(port);
}

void MQTTClient::setMqttClientAuthentication(const QString& username, const QString& password) {
	m_client->setUsername(username);
	m_client->setPassword(password);
}

void MQTTClient::setMqttClientId(const QString &Id){
	m_client->setClientId(Id);
}

void MQTTClient::addMqttSubscriptions(const QMqttTopicFilter& filter, const quint8& qos) {
	m_topicMap[filter] = qos;
}

void MQTTClient::onMqttConnect() {
	qDebug() << "on mqtt connect";
	if(m_client->error() == QMqttClient::NoError) {
		if(!m_mqttFirstConnectEstablished) {
			qDebug()<<"connection made in MQTTClient";

			QMapIterator<QMqttTopicFilter, quint8> i(m_topicMap);
			while(i.hasNext()) {
				i.next();
				QMqttSubscription *temp = m_client->subscribe(i.key(), i.value());
				if(temp) {
					qDebug()<<temp->topic()<<"  "<<temp->qos();
					m_messageArrived[temp->topic().filter()] = false;
					m_subscriptions.push_back(temp->topic().filter());

					qDebug()<<"New MQTTSubscription";
					MQTTSubscriptions* tempSubscription = new MQTTSubscriptions(temp->topic().filter());
					tempSubscription->setMQTTClient(this);

					qDebug()<<"Add child";
					addChild(tempSubscription);

					qDebug()<<"Add to vector";
					m_mqttSubscriptions.push_back(tempSubscription);

					connect(temp, &QMqttSubscription::messageReceived, this, &MQTTClient::mqttSubscribtionMessageReceived);
					qDebug()<<"Added topic";
				}
			}
			m_mqttFirstConnectEstablished = true;
			emit mqttSubscribed();
		}
		else {
			qDebug() << "Resubscribing after will set";
			QMapIterator<QMqttTopicFilter, quint8> i(m_topicMap);
			while(i.hasNext()) {
				i.next();
				QMqttSubscription *temp = m_client->subscribe(i.key(), i.value());
				if(temp) {
					qDebug()<<temp->topic()<<"  "<<temp->qos();
					connect(temp, &QMqttSubscription::messageReceived, this, &MQTTClient::mqttSubscribtionMessageReceived);
				}
				else
					qDebug()<<"Couldn't subscribe after will change";
			}
		}
	}
}

void MQTTClient::mqttSubscribtionMessageReceived(const QMqttMessage& msg) {
	if(!msg.retain() || (msg.retain() && m_mqttRetain) ) {
		qDebug()<<"message received from "<<msg.topic().name();
		if(!m_topicNames.contains(msg.topic().name())) {
			m_topicNames.push_back(msg.topic().name());
			emit mqttNewTopicArrived();
		}

		for(int i = 0; i < m_mqttSubscriptions.count(); ++i){
			QString subscriptionName = m_mqttSubscriptions[i]->subscriptionName();
			if(subscriptionName.contains('#') || subscriptionName.contains('+')) {
				if(subscriptionName.contains('#')) {
					if(msg.topic().name().startsWith(subscriptionName.left(subscriptionName.count() - 2)) ){
						m_mqttSubscriptions[i]->messageArrived(QString(msg.payload()), msg.topic().name());
						break;
					}
				}
				else if (subscriptionName.contains('+')) {
					int pos = subscriptionName.indexOf('+');
					QString start = subscriptionName.left(pos);
					QString end = subscriptionName.right(subscriptionName.count() - pos);
					if(msg.topic().name().startsWith(start) && msg.topic().name().endsWith(end)) {
						m_mqttSubscriptions[i]->messageArrived(QString(msg.payload()), msg.topic().name());
						break;
					}
				}
			}
			else if(subscriptionName == msg.topic().name()) {
				m_mqttSubscriptions[i]->messageArrived(QString(msg.payload()), msg.topic().name());
				break;
			}
		}

		if(msg.topic().name() == m_willTopic)
			m_willLastMessage = QString(msg.payload());

	}
}

void MQTTClient::onAllArrived() {
	qDebug()<<"all arrived";
	if (m_fileType == Ascii) {
		qDebug()<<"Ascii ok";
		QMapIterator<QMqttTopicName, QVector<QMqttMessage>> k(m_messagePuffer);
		qDebug()<<"first iterator created";
		bool ok = true;
		while(k.hasNext()){
			k.next();
			qDebug()<<"investigating"<<k.key();
			if(k.value().isEmpty()) {
				ok = false;
				qDebug()<<k.key()<<" has no messages";
			}
		}
		if(ok){
			qDebug()<<"topics ok start read";
			QMapIterator<QMqttTopicName, QVector<QMqttMessage>> i(m_messagePuffer);
			while(i.hasNext()) {
				i.next();
				if(!i.value().isEmpty()) {
					QMqttMessage temp_msg = m_messagePuffer[i.key()].takeFirst();
					qDebug()<<"Start reading from "<<i.key();
					//dynamic_cast<AsciiFilter*>(m_filter)->readFromMqtt(temp_msg.payload(), temp_msg.topic().name(), this);
					qDebug()<<"readfrommqtt occured";
				}
			}
		}
		qDebug()<<"start checking";
		QMapIterator<QMqttTopicName, bool> j(m_messageArrived);
		while(j.hasNext()) {
			j.next();
			if(m_messagePuffer[j.key()].isEmpty()) {
				m_messageArrived[j.key()] = false;
			}
		}
		qDebug()<<"end checking";
	}
}

int MQTTClient::topicNumber() {
	return m_subscriptions.count();
}

int MQTTClient::topicIndex(const QString& topic) {
	return m_subscriptions.indexOf(topic, 0);
}

bool MQTTClient::checkAllArrived() {
	bool check = true;
	QMapIterator<QMqttTopicName, bool> i(m_messageArrived);
	while(i.hasNext()) {
		i.next();
		if(i.value() == false )
		{
			check = false;
			break;
		}
	}
	return check;
}

void MQTTClient::setMqttWillUse(bool use) {
	m_mqttUseWill = use;
	if(use == false)
		m_willTimer->stop();
}

bool MQTTClient::mqttWillUse() const{
	return m_mqttUseWill;
}

void  MQTTClient::setWillTopic(const QString& topic) {
	m_willTopic = topic;
}

QString MQTTClient::willTopic() const{
	return m_willTopic;
}

void MQTTClient::setWillRetain(bool retain) {
	m_willRetain = retain;
}
bool MQTTClient::willRetain() const {
	return m_willRetain;
}

void MQTTClient::setWillQoS(quint8 QoS) {
	m_willQoS = QoS;
}
quint8 MQTTClient::willQoS() const {
	return m_willQoS;
}

void MQTTClient::setWillMessageType(WillMessageType messageType) {
	m_willMessageType = messageType;
}

MQTTClient::WillMessageType MQTTClient::willMessageType() const {
	return m_willMessageType;
}

void MQTTClient::setWillOwnMessage(const QString& ownMessage) {
	m_willOwnMessage = ownMessage;
}

QString MQTTClient::willOwnMessage() const {
	return m_willOwnMessage;
}

QVector<QString> MQTTClient::topicNames() const {
	return m_topicNames;
}

void MQTTClient::setWillForMqtt() {
	if(m_mqttUseWill && (m_client->state() == QMqttClient::ClientState::Connected) ) {
		qDebug() << "Disconnecting from host";
		m_client->disconnectFromHost();

		m_client->setWillQoS(m_willQoS);
		qDebug()<<"Will QoS" << m_willQoS;

		m_client->setWillRetain(m_willRetain);
		qDebug()<<"Will retain" << m_willRetain;

		m_client->setWillTopic(m_willTopic);
		qDebug()<<"Will Topic" << m_willTopic;

		switch (m_willMessageType) {
		case WillMessageType::OwnMessage:
			m_client->setWillMessage(m_willOwnMessage.toUtf8());
			qDebug()<<"Will own message" << m_willOwnMessage;
			break;
		case WillMessageType::Statistics: {
			qDebug()<<"Start will statistics";
			QVector<const MQTTTopic*> topics = this->children<const MQTTTopic>(AbstractAspect::Recursive);
			const AsciiFilter* asciiFilter = nullptr;
			const MQTTTopic* tempTopic = nullptr;
			qDebug()<<"Searching for topic";
			for (int i = 0; i < topics.count(); ++i) {
				if(topics[i]->name() == m_willTopic) {
					asciiFilter = dynamic_cast<AsciiFilter*>(topics[i]->filter());
					tempTopic = topics[i];
					break;
				}
			}
			qDebug()<<"Check if topic found";
			if(asciiFilter != nullptr && tempTopic != nullptr) {
				qDebug()<<"Checking column mode";
				if((asciiFilter->mqttColumnMode() == AbstractColumn::ColumnMode::Integer) ||
						(asciiFilter->mqttColumnMode() == AbstractColumn::ColumnMode::Numeric)) {
					m_client->setWillMessage(asciiFilter->mqttColumnStatistics(tempTopic, this).toUtf8());
					qDebug() << "Will statistics message: "<< QString(m_client->willMessage());
				}
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

		m_client->connectToHost();
		qDebug()<< "Reconnect to host";
	}
}

MQTTClient::WillUpdateType MQTTClient::willUpdateType() const{
	return m_willUpdateType;
}

void MQTTClient::setWillUpdateType(WillUpdateType updateType) {
	m_willUpdateType = updateType;
}

int MQTTClient::willTimeInterval() const{
	return m_willTimeInterval;
}

void MQTTClient::setWillTimeInterval(int interval) {
	m_willTimeInterval = interval;
}

void MQTTClient::clearLastMessage() {
	m_willLastMessage.clear();
}

void MQTTClient::addWillStatistics(WillStatistics statistic){
	m_willStatistics[static_cast<int>(statistic)] = true;
}

void MQTTClient::removeWillStatistics(WillStatistics statistic) {
	m_willStatistics[static_cast<int>(statistic)] = false;
}

QVector<bool> MQTTClient::willStatistics() const{
	return m_willStatistics;
}

void MQTTClient::startWillTimer() const{
	if(m_willUpdateType == WillUpdateType::TimePeriod)
		m_willTimer->start(m_willTimeInterval);
}
void MQTTClient::stopWillTimer() const{
	m_willTimer->stop();
}

void MQTTClient::setMqttRetain(bool retain) {
	m_mqttRetain = retain;
}

bool MQTTClient::mqttRetain() const {
	return m_mqttRetain;
}

void MQTTClient::mqttErrorChanged(QMqttClient::ClientError clientError) {
	if(clientError != QMqttClient::ClientError::NoError) {
		MQTTErrorWidget* errorWidget = new MQTTErrorWidget(clientError, this);
		errorWidget->show();
	}
}

QString MQTTClient::clientHostName() const{
	return m_client->hostname();
}
quint16 MQTTClient::clientPort() const {
	return m_client->port();
}

QString MQTTClient::clientPassword() const{
	return m_client->password();
}

QString MQTTClient::clientUserName() const{
	return m_client->username();
}

QString MQTTClient::clientID () const{
	return m_client->clientId();
}

void MQTTClient::setMQTTUseID(bool use) {
	m_mqttUseID = use;
}
bool MQTTClient::mqttUseID() const {
	return m_mqttUseID;
}

void MQTTClient::setMQTTUseAuthentication(bool use) {
	m_mqttUseAuthentication = use;
}
bool MQTTClient::mqttUseAuthentication() const {
	return m_mqttUseAuthentication;
}

QVector<QString> MQTTClient::mqttSubscribtions() const {
	return m_subscriptions;
}

void MQTTClient::newMQTTTopic(const QString& topic, quint8 QoS) {
	QMqttTopicFilter filter {topic};
	QMqttSubscription* temp = m_client->subscribe(filter, QoS);

	if (temp) {
		qDebug()<<temp->topic()<<"  "<<temp->qos();
		m_subscriptions.push_back(temp->topic().filter());

		qDebug()<<"New MQTTSubscription";
		MQTTSubscriptions* tempSubscription = new MQTTSubscriptions(temp->topic().filter());
		tempSubscription->setMQTTClient(this);

		qDebug()<<"Add child";
		addChild(tempSubscription);

		qDebug()<<"Add to vector";
		m_mqttSubscriptions.push_back(tempSubscription);

		connect(temp, &QMqttSubscription::messageReceived, this, &MQTTClient::mqttSubscribtionMessageReceived);
		qDebug()<<"Added topic";
		emit mqttSubscribed();
	}
}
#endif
