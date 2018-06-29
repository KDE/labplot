/***************************************************************************
File                 : LiveDataDock.cpp
Project              : LabPlot
Description          : Dock widget for live data properties
--------------------------------------------------------------------
Copyright            : (C) 2017 by Fabian Kristof (fkristofszabolcs@gmail.com)
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
#include "LiveDataDock.h"
#include <KLocalizedString>

LiveDataDock::LiveDataDock(QWidget* parent) :
	QWidget(parent),
#ifdef HAVE_MQTT
	m_client(new QMqttClient()),
	m_editing(true),
	m_MQTTUsed(true),
	m_previousMQTTClient(nullptr),
	m_timer(new QTimer()),
	m_messageTimer(new QTimer()),
	m_interpretMessage(true),
	m_mqttSubscribeButton(true),
#endif
	m_paused(false) {
	ui.setupUi(this);

	ui.bUpdateNow->setIcon(QIcon::fromTheme(QLatin1String("view-refresh")));

	connect(ui.bPausePlayReading, &QPushButton::clicked, this, &LiveDataDock::pauseContinueReading);
	connect(ui.bUpdateNow, &QPushButton::clicked, this, &LiveDataDock::updateNow);
	connect(ui.sbUpdateInterval, static_cast<void (QSpinBox::*) (int)>(&QSpinBox::valueChanged), this, &LiveDataDock::updateIntervalChanged);

	connect(ui.leKeepNValues, &QLineEdit::textChanged, this, &LiveDataDock::keepNvaluesChanged);
	connect(ui.sbSampleRate, static_cast<void (QSpinBox::*) (int)>(&QSpinBox::valueChanged), this, &LiveDataDock::sampleRateChanged);
	connect(ui.cbUpdateType, static_cast<void (QComboBox::*) (int)>(&QComboBox::currentIndexChanged), this, &LiveDataDock::updateTypeChanged);
	connect(ui.cbReadingType, static_cast<void (QComboBox::*) (int)>(&QComboBox::currentIndexChanged), this, &LiveDataDock::readingTypeChanged);

#ifdef HAVE_MQTT
	m_timer->setInterval(10000);

	connect(m_client, &QMqttClient::connected, this, &LiveDataDock::onMQTTConnect);
	connect(m_client, &QMqttClient::messageReceived, this, &LiveDataDock::mqttMessageReceived);
	connect(this, &LiveDataDock::newTopic, this, &LiveDataDock::setCompleter);
	connect(ui.cbTopics, &QComboBox::currentTextChanged, this, &LiveDataDock::topicBeingTyped);
	connect(m_timer, &QTimer::timeout, this, &LiveDataDock::topicTimeout);
	connect(ui.bTopics, &QPushButton::clicked, this, &LiveDataDock::addSubscription);
	connect(m_messageTimer, &QTimer::timeout, this, &LiveDataDock::stopStartReceive);
	connect(ui.chbWill, &QCheckBox::stateChanged, this, &LiveDataDock::useWillMessage);
	connect(ui.cbWillQoS, static_cast<void (QComboBox::*) (int)>(&QComboBox::currentIndexChanged), this, &LiveDataDock::willQoSChanged);
	connect(ui.chbWillRetain, &QCheckBox::stateChanged, this, &LiveDataDock::willRetainChanged);
	connect(ui.cbWillTopic, &QComboBox::currentTextChanged, this, &LiveDataDock::willTopicChanged);
	connect(ui.cbWillMessageType, static_cast<void (QComboBox::*) (int)>(&QComboBox::currentIndexChanged), this, &LiveDataDock::willMessageTypeChanged);
	connect(ui.leWillOwnMessage, &QLineEdit::textChanged, this, &LiveDataDock::willOwnMessageChanged);
	connect(ui.cbWillUpdate, static_cast<void (QComboBox::*) (int)>(&QComboBox::currentIndexChanged), this, &LiveDataDock::willUpdateChanged);
	connect(ui.bWillUpdateNow, &QPushButton::clicked, this, &LiveDataDock::willUpdateNow);
	connect(ui.leWillUpdateInterval, &QLineEdit::textChanged, this, &LiveDataDock::willUpdateIntervalChanged);
	connect(ui.lwWillStatistics, &QListWidget::itemChanged, this, &LiveDataDock::statisticsChanged);
	connect(ui.cbTopics, &QComboBox::currentTextChanged, this, &LiveDataDock::mqttButtonSubscribe);
	connect(ui.lwSubscriptions, &QListWidget::currentTextChanged, this, &LiveDataDock::mqttButtonUnsubscribe);
#endif
}

LiveDataDock::~LiveDataDock() {
}

#ifdef HAVE_MQTT
/*!
 * \brief Sets the MQTTClients of this dock widget
 * \param clients
 */
void LiveDataDock::setMQTTClients(const QList<MQTTClient *> &clients) {
	m_MQTTUsed = true;
	m_liveDataSources.clear();
	m_mqttClients.clear();
	m_mqttClients = clients;
	const MQTTClient* const fds = clients.at(0);

	ui.sbUpdateInterval->setValue(fds->updateInterval());
	ui.cbUpdateType->setCurrentIndex(static_cast<int>(fds->updateType()));
	ui.cbReadingType->setCurrentIndex(static_cast<int>(fds->readingType()));

	if (fds->updateType() == MQTTClient::UpdateType::NewData) {
		ui.lUpdateInterval->hide();
		ui.sbUpdateInterval->hide();
	}

	if (fds->isPaused()) {
		ui.bPausePlayReading->setText(i18n("Continue reading"));
		ui.bPausePlayReading->setIcon(QIcon::fromTheme(QLatin1String("media-record")));
	} else {
		ui.bPausePlayReading->setText(i18n("Pause reading"));
		ui.bPausePlayReading->setIcon(QIcon::fromTheme(QLatin1String("media-playback-pause")));
	}

	if(!fds->keepLastValues()) {
		ui.leKeepNValues->hide();
		ui.lKeepNvalues->hide();
	} else {
		ui.leKeepNValues->setValidator(new QIntValidator(2, 100000));
		ui.leKeepNValues->setText(QString::number(fds->keepNvalues()));
	}

	if (fds->readingType() == MQTTClient::ReadingType::TillEnd) {
		ui.lSampleRate->hide();
		ui.sbSampleRate->hide();
	} else
		ui.sbSampleRate->setValue(fds->sampleRate());

	int itemIdx = -1;
	for (int i = 0; i < ui.cbReadingType->count(); ++i) {
		if (ui.cbReadingType->itemText(i) == QLatin1String("Read whole file")) {
			itemIdx = i;
			break;
		}
	}
	if (itemIdx != -1) {
		ui.cbReadingType->removeItem(itemIdx);
	}

	ui.chbWill->hide();
	ui.chbWillRetain->hide();
	ui.cbWillQoS->hide();
	ui.cbWillMessageType->hide();
	ui.cbWillTopic->hide();
	ui.cbWillUpdate->hide();
	ui.leWillOwnMessage->hide();
	ui.leWillUpdateInterval->setValidator(new QIntValidator(2, 1000000) );
	ui.leWillUpdateInterval->hide();
	ui.lWillMessageType->hide();
	ui.lWillOwnMessage->hide();
	ui.lWillQos->hide();
	ui.lWillTopic->hide();
	ui.lWillUpdate->hide();
	ui.lWillUpdateInterval->hide();
	ui.bWillUpdateNow->hide();
	ui.lwWillStatistics->hide();
	ui.lWillStatistics->hide();
	ui.bTopics->show();
	ui.cbTopics->show();
	ui.lSubscriptions->show();
	ui.lwSubscriptions->show();
	ui.lQoS->show();
	ui.cbQoS->show();

	if((m_client->state() == QMqttClient::Connected && m_client->hostname() != fds->clientHostName())
			|| m_client->state() == QMqttClient::Disconnected) {
		if(m_client->state() == QMqttClient::Connected)
			m_client->disconnectFromHost();

		m_client->setHostname(fds->clientHostName());
		m_client->setPort(fds->clientPort());

		if(fds->mqttUseAuthentication()) {
			m_client->setUsername(fds->clientUserName());
			m_client->setPassword(fds->clientPassword());
		}

		if(fds->mqttUseID()) {
			m_client->setClientId(fds->clientID());
		}
		m_client->connectToHost();
	}

	ui.chbWill->show();

	if(m_previousMQTTClient != nullptr) {
		disconnect(m_previousMQTTClient, &MQTTClient::mqttSubscribed, this, &LiveDataDock::fillSubscriptions);
		disconnect(m_previousMQTTClient, &MQTTClient::mqttNewTopicArrived, this, &LiveDataDock::updateTopics);
	}
	connect(fds, &MQTTClient::mqttSubscribed, this, &LiveDataDock::fillSubscriptions);
	connect(fds, &MQTTClient::mqttNewTopicArrived, this, &LiveDataDock::updateTopics);
	ui.leWillOwnMessage->setText(fds->willOwnMessage());
	ui.leWillUpdateInterval->setText(QString::number(fds->willTimeInterval()));
	qDebug()<<"update type at setup "<<static_cast<int>(fds->willUpdateType()) <<"  start index "<<ui.cbWillUpdate->currentIndex();
	ui.cbWillUpdate->setCurrentIndex(static_cast<int>(fds->willUpdateType()) );
	fds->startWillTimer();
	ui.cbWillMessageType->setCurrentIndex(static_cast<int>(fds->willMessageType()) );
	ui.cbWillQoS->setCurrentIndex(fds->willQoS());
	ui.cbWillTopic->setCurrentText(fds->willTopic());
	ui.chbWillRetain->setChecked(fds->willRetain());
	QVector<bool> statitics = fds->willStatistics();
	for(int i = 0; i < statitics.count(); ++i) {
		QListWidgetItem* item = ui.lwWillStatistics->item(static_cast<int>(i));
		if(statitics[i]) {
			item->setCheckState(Qt::Checked);
		}
		else {
			item->setCheckState(Qt::Unchecked);
		}
	}
	qDebug()<<"chbWill is set to: "<<fds->mqttWillUse();
	//when chbWill's isChecked corresponds with source's m_mqttWillUse it doesn't emit state changed signal, we have to force it
	bool checked = fds->mqttWillUse();
	ui.chbWill->setChecked(!checked);
	ui.chbWill->setChecked(checked);
	m_previousMQTTClient = fds;
}
#endif

/*!
 * \brief Sets the live data sources of this dock widget
 * \param sources
 */
void LiveDataDock::setLiveDataSources(const QList<LiveDataSource*>& sources) {
#ifdef HAVE_MQTT
	m_MQTTUsed = false;
	m_mqttClients.clear();
#endif
	m_liveDataSources = sources;
	const LiveDataSource* const fds = sources.at(0);
	ui.sbUpdateInterval->setValue(fds->updateInterval());
	ui.cbUpdateType->setCurrentIndex(static_cast<int>(fds->updateType()));
	ui.cbReadingType->setCurrentIndex(static_cast<int>(fds->readingType()));

	if (fds->updateType() == LiveDataSource::UpdateType::NewData) {
		ui.lUpdateInterval->hide();
		ui.sbUpdateInterval->hide();
	}

	if (fds->isPaused()) {
		ui.bPausePlayReading->setText(i18n("Continue reading"));
		ui.bPausePlayReading->setIcon(QIcon::fromTheme(QLatin1String("media-record")));
	} else {
		ui.bPausePlayReading->setText(i18n("Pause reading"));
		ui.bPausePlayReading->setIcon(QIcon::fromTheme(QLatin1String("media-playback-pause")));
	}

	if(!fds->keepLastValues()) {
		ui.leKeepNValues->hide();
		ui.lKeepNvalues->hide();
	} else {
		ui.leKeepNValues->setValidator(new QIntValidator(2, 100000));
		ui.leKeepNValues->setText(QString::number(fds->keepNvalues()));
	}

	if (fds->sourceType() != LiveDataSource::SourceType::FileOrPipe) {
		int itemIdx = -1;
		for (int i = 0; i < ui.cbReadingType->count(); ++i) {
			if (ui.cbReadingType->itemText(i) == QLatin1String("Read whole file")) {
				itemIdx = i;
				break;
			}
		}
		if (itemIdx != -1)
			ui.cbReadingType->removeItem(itemIdx);
	}

	if (fds->readingType() == LiveDataSource::ReadingType::TillEnd) {
		ui.lSampleRate->hide();
		ui.sbSampleRate->hide();
	} else if (fds->readingType() == LiveDataSource::ReadingType::WholeFile) {
		ui.lSampleRate->hide();
		ui.sbSampleRate->hide();
	} else
		ui.sbSampleRate->setValue(fds->sampleRate());

	ui.chbWill->hide();
	ui.chbWillRetain->hide();
	ui.cbWillQoS->hide();
	ui.cbWillMessageType->hide();
	ui.cbWillTopic->hide();
	ui.cbWillUpdate->hide();
	ui.leWillOwnMessage->hide();
	ui.leWillUpdateInterval->setValidator(new QIntValidator(2, 1000000) );
	ui.leWillUpdateInterval->hide();
	ui.lWillMessageType->hide();
	ui.lWillOwnMessage->hide();
	ui.lWillQos->hide();
	ui.lWillTopic->hide();
	ui.lWillUpdate->hide();
	ui.lWillUpdateInterval->hide();
	ui.bWillUpdateNow->hide();
	ui.lwWillStatistics->hide();
	ui.lWillStatistics->hide();
	ui.bTopics->hide();
	ui.cbTopics->hide();
	ui.lSubscriptions->hide();
	ui.lwSubscriptions->hide();
	ui.lQoS->hide();
	ui.cbQoS->hide();
}

/*!
 * \brief Modifies the sample rate of the live data sources or MQTTClient objects
 * \param sampleRate
 */
void LiveDataDock::sampleRateChanged(int sampleRate) {
	if(!m_liveDataSources.isEmpty()) {
		for (auto* source : m_liveDataSources)
			source->setSampleRate(sampleRate);
	}
#ifdef HAVE_MQTT
	else if (!m_mqttClients.isEmpty()) {
		for (auto* client : m_mqttClients)
			client->setSampleRate(sampleRate);
	}
#endif
}

/*!
 * \brief Updates the live data sources now
 */
void LiveDataDock::updateNow() {
	if(!m_liveDataSources.isEmpty()) {
		for (auto* source : m_liveDataSources)
			source->updateNow();
	}
#ifdef HAVE_MQTT
	else if (!m_mqttClients.isEmpty()) {
		for (auto* client : m_mqttClients)
			client->updateNow();
	}
#endif
}

/*!
 * \brief LiveDataDock::updateTypeChanged
 * \param idx
 */
void LiveDataDock::updateTypeChanged(int idx) {
	if(!m_liveDataSources.isEmpty())  {
		DEBUG("LiveDataDock::updateTypeChanged()");
		LiveDataSource::UpdateType type = static_cast<LiveDataSource::UpdateType>(idx);

		switch (type) {
		case LiveDataSource::UpdateType::TimeInterval:
			DEBUG("	interval value = " << ui.sbUpdateInterval->value());
			ui.lUpdateInterval->show();
			ui.sbUpdateInterval->show();

			for (auto* source: m_liveDataSources) {
				source->setUpdateType(type);
				source->setUpdateInterval(ui.sbUpdateInterval->value());
				source->setFileWatched(false);
			}
			break;
		case LiveDataSource::UpdateType::NewData:
			ui.lUpdateInterval->hide();
			ui.sbUpdateInterval->hide();

			for (auto* source: m_liveDataSources) {
				source->setFileWatched(true);
				source->setUpdateType(type);
			}
		}
	}
#ifdef HAVE_MQTT
	else if (!m_mqttClients.isEmpty()) {
		MQTTClient::UpdateType type = static_cast<MQTTClient::UpdateType>(idx);

		if (type == MQTTClient::UpdateType::TimeInterval) {
			ui.lUpdateInterval->show();
			ui.sbUpdateInterval->show();

			for (auto* client : m_mqttClients) {
				client->setUpdateType(type);
				client->setUpdateInterval(ui.sbUpdateInterval->value());
			}
		} else if (type == MQTTClient::UpdateType::NewData) {
			ui.lUpdateInterval->hide();
			ui.sbUpdateInterval->hide();

			for (auto* client : m_mqttClients) {
				client->setUpdateType(type);
			}
		}
	}
#endif
}

/*!
 * \brief Handles the change of the reading type in the dock widget
 * \param idx
 */
void LiveDataDock::readingTypeChanged(int idx) {
	if(!m_liveDataSources.isEmpty())  {
		LiveDataSource::ReadingType type = static_cast<LiveDataSource::ReadingType>(idx);

		if (type == LiveDataSource::ReadingType::TillEnd) {
			ui.lSampleRate->hide();
			ui.sbSampleRate->hide();
		} else {
			ui.lSampleRate->show();
			ui.sbSampleRate->show();
		}

		for (auto* source : m_liveDataSources)
			source->setReadingType(type);
	}
#ifdef HAVE_MQTT
	else if (!m_mqttClients.isEmpty()) {
		MQTTClient::ReadingType type = static_cast<MQTTClient::ReadingType>(idx);

		if (type == MQTTClient::ReadingType::TillEnd) {
			ui.lSampleRate->hide();
			ui.sbSampleRate->hide();
		} else {
			ui.lSampleRate->show();
			ui.sbSampleRate->show();
		}

		for (auto* client : m_mqttClients)
			client->setReadingType(type);
	}
#endif
}

/*!
 * \brief Modifies the update interval of the live data sources
 * \param updateInterval
 */
void LiveDataDock::updateIntervalChanged(int updateInterval) {
	if(!m_liveDataSources.isEmpty())  {
		for (auto* source : m_liveDataSources)
			source->setUpdateInterval(updateInterval);
	}
#ifdef HAVE_MQTT
	else if (!m_mqttClients.isEmpty()) {
		for (auto* client : m_mqttClients)
			client->setUpdateInterval(updateInterval);
	}
#endif
}

/*!
 * \brief Modifies the number of samples to keep in each of the live data sources
 * \param keepNvalues
 */
void LiveDataDock::keepNvaluesChanged(const QString& keepNvalues) {
	if(!m_liveDataSources.isEmpty())  {
		for (auto* source : m_liveDataSources)
			source->setKeepNvalues(keepNvalues.toInt());
	}
#ifdef HAVE_MQTT
	else if (!m_mqttClients.isEmpty()) {
		for (auto* client : m_mqttClients)
			client->setKeepNvalues(keepNvalues.toInt());
	}
#endif
}

/*!
 * \brief Pauses the reading of the live data source
 */
void LiveDataDock::pauseReading() {
	if(!m_liveDataSources.isEmpty())  {
		for (auto* source: m_liveDataSources)
			source->pauseReading();
	}
#ifdef HAVE_MQTT
	else if (!m_mqttClients.isEmpty()) {
		for (auto* client : m_mqttClients)
			client->pauseReading();
	}
#endif
}

/*!
 * \brief Continues the reading of the live data source
 */
void LiveDataDock::continueReading() {
	if(!m_liveDataSources.isEmpty())  {
		for (auto* source: m_liveDataSources)
			source->continueReading();
	}
#ifdef HAVE_MQTT
	else if (!m_mqttClients.isEmpty()) {
		for (auto* client : m_mqttClients)
			client->continueReading();
	}
#endif
}

/*!
 * \brief Handles the pausing/continuing of reading of the live data source
 */
void LiveDataDock::pauseContinueReading() {
	m_paused = !m_paused;

	if (m_paused) {
		pauseReading();
		ui.bPausePlayReading->setText(i18n("Continue reading"));
		ui.bPausePlayReading->setIcon(QIcon::fromTheme(QLatin1String("media-record")));
	} else {
		continueReading();
		ui.bPausePlayReading->setText(i18n("Pause reading"));
		ui.bPausePlayReading->setIcon(QIcon::fromTheme(QLatin1String("media-playback-pause")));
	}
}

#ifdef HAVE_MQTT
void LiveDataDock::useWillMessage(int state) {
	qDebug()<<"will checkstate changed" <<state;
	if(state == Qt::Checked) {
		for (auto* source: m_mqttClients)
			source->setMqttWillUse(true);

		ui.chbWillRetain->show();
		ui.cbWillQoS->show();
		ui.cbWillMessageType->show();
		ui.cbWillTopic->show();
		ui.cbWillUpdate->show();
		ui.lWillMessageType->show();
		ui.lWillQos->hide();
		ui.lWillTopic->show();
		ui.lWillUpdate->show();

		if (ui.cbWillMessageType->currentIndex() == (int)MQTTClient::WillMessageType::OwnMessage) {
			ui.leWillOwnMessage->show();
			ui.lWillOwnMessage->show();
		}
		else if(ui.cbWillMessageType->currentIndex() == (int)MQTTClient::WillMessageType::Statistics){
			ui.lWillStatistics->show();
			ui.lwWillStatistics->show();
		}


		if(ui.cbWillUpdate->currentIndex() == static_cast<int>(MQTTClient::WillUpdateType::TimePeriod)) {
			ui.leWillUpdateInterval->show();
			ui.lWillUpdateInterval->show();
		}
		else if (ui.cbWillUpdate->currentIndex() == static_cast<int>(MQTTClient::WillUpdateType::OnClick))
			ui.bWillUpdateNow->show();
	}
	else if (state == Qt::Unchecked) {
		for (auto* source: m_mqttClients)
			source->setMqttWillUse(false);

		ui.chbWillRetain->hide();
		ui.cbWillQoS->hide();
		ui.cbWillMessageType->hide();
		ui.cbWillTopic->hide();
		ui.cbWillUpdate->hide();
		ui.leWillOwnMessage->hide();
		ui.leWillUpdateInterval->hide();
		ui.lWillMessageType->hide();
		ui.lWillOwnMessage->hide();
		ui.lWillQos->hide();
		ui.lWillTopic->hide();
		ui.lWillUpdate->hide();
		ui.lWillUpdateInterval->hide();
		ui.bWillUpdateNow->hide();
		ui.lWillStatistics->hide();
		ui.lwWillStatistics->hide();
	}
}

void LiveDataDock::willQoSChanged(int QoS) {
	for (auto* source: m_mqttClients)
		source->setWillQoS(QoS);
}

void LiveDataDock::willRetainChanged(int state) {
	if(state == Qt::Checked) {
		for (auto* source: m_mqttClients)
			source->setWillRetain(true);
	}
	else if (state == Qt::Unchecked) {
		for (auto* source: m_mqttClients)
			source->setWillRetain(false);
	}
}

void LiveDataDock::willTopicChanged(const QString& topic) {
	qDebug()<<"topic  changed" << topic;
	for (auto* source: m_mqttClients) {
		if(source->willTopic() != topic)
			source->clearLastMessage();
		source->setWillTopic(topic);
	}
}

void LiveDataDock::willMessageTypeChanged(int type) {
	qDebug()<<"message type changed" << type;
	for (auto* source: m_mqttClients)
		source->setWillMessageType(static_cast<MQTTClient::WillMessageType> (type));
	if(static_cast<MQTTClient::WillMessageType> (type) == MQTTClient::WillMessageType::OwnMessage) {
		ui.leWillOwnMessage->show();
		ui.lWillOwnMessage->show();
		ui.lWillStatistics->hide();
		ui.lwWillStatistics->hide();
	}
	else if(static_cast<MQTTClient::WillMessageType> (type) == MQTTClient::WillMessageType::LastMessage) {
		ui.leWillOwnMessage->hide();
		ui.lWillOwnMessage->hide();
		ui.lWillStatistics->hide();
		ui.lwWillStatistics->hide();
	}
	else if(static_cast<MQTTClient::WillMessageType> (type) == MQTTClient::WillMessageType::Statistics) {
		ui.lWillStatistics->show();
		ui.lwWillStatistics->show();
		ui.leWillOwnMessage->hide();
		ui.lWillOwnMessage->hide();
	}
}

void LiveDataDock::willOwnMessageChanged(const QString& message) {
	qDebug()<<"own message changed" << message;
	for (auto* source: m_mqttClients)
		source->setWillOwnMessage(message);
}

void LiveDataDock::updateTopics() {
	ui.cbWillTopic->clear();
	const MQTTClient* const fds = m_mqttClients.at(0);
	QVector<QString> topics = fds->topicNames();

	if(!topics.isEmpty()) {
		for(int i = 0; i < topics.count(); i++) {
			qDebug()<<"Live Data Dock: updating will topics: "<<topics[i];
			//if(ui.cbWillTopic->findText(topics[i]) < 0)
			ui.cbWillTopic->addItem(topics[i]);
		}
		if(!fds->willTopic().isEmpty())
			ui.cbWillTopic->setCurrentText(fds->willTopic());
	}
	else
		qDebug()<<"Topic Vector Empty";
}

void LiveDataDock::willUpdateChanged(int updateType) {
	qDebug()<<"Update type changed" << updateType;

	for (auto* source: m_mqttClients)
		source->setWillUpdateType(static_cast<MQTTClient::WillUpdateType>(updateType));

	if(static_cast<MQTTClient::WillUpdateType>(updateType) == MQTTClient::WillUpdateType::TimePeriod) {
		ui.bWillUpdateNow->hide();
		ui.leWillUpdateInterval->show();
		ui.lWillUpdateInterval->show();

		for (auto* source: m_mqttClients) {
			source->setWillTimeInterval(ui.leWillUpdateInterval->text().toInt());
			source->startWillTimer();
		}
	}
	else if (static_cast<MQTTClient::WillUpdateType>(updateType) == MQTTClient::WillUpdateType::OnClick) {
		ui.bWillUpdateNow->show();
		ui.leWillUpdateInterval->hide();
		ui.lWillUpdateInterval->hide();

		for (auto* source: m_mqttClients)
			source->stopWillTimer();
	}

}

void LiveDataDock::willUpdateNow() {
	for (auto* source: m_mqttClients)
		source->setWillForMqtt();
}

void LiveDataDock::willUpdateIntervalChanged(const QString& interval) {
	qDebug()<<"Update interval changed " <<interval;
	for (auto* source: m_mqttClients) {
		source->setWillTimeInterval(interval.toInt());
		source->startWillTimer();
	}
}

void LiveDataDock::statisticsChanged(QListWidgetItem *item) {
	int idx = -1;
	for(int i =  0; i < ui.lwWillStatistics->count(); i++)
		if(item->text() == ui.lwWillStatistics->item(i)->text()) {
			idx = i;
			break;
		}

	if(item->checkState() == Qt::Checked) {
		if(idx >= 0) {
			for (auto* source: m_mqttClients)
				source->addWillStatistics(static_cast<MQTTClient::WillStatistics>(idx) );
		}
	}
	else {
		if(idx >= 0){
			for (auto* source: m_mqttClients)
				source->removeWillStatistics(static_cast<MQTTClient::WillStatistics>(idx) );
		}
	}
}

void LiveDataDock::onMQTTConnect() {
	QMqttTopicFilter globalFilter{"#"};
	QMqttSubscription * subscription = m_client->subscribe(globalFilter, 1);
	if(!subscription)
		qDebug()<<"Couldn't make global subscription in LiveDataDock";
	m_messageTimer->start(3000);
}

void LiveDataDock::mqttMessageReceived(const QByteArray& message, const QMqttTopicName& topic) {
	QString topicName = topic.name();
	if(ui.cbTopics->findText(topicName) == -1) {
		QStringList topicList = topicName.split('/', QString::SkipEmptyParts);
		for(int i = topicList.count() - 1; i >= 0; --i) {
			QString tempTopic = "";
			for(int j = 0; j <= i; ++j) {
				tempTopic = tempTopic + topicList.at(j) + "/";
			}
			if(i < topicList.count() - 1) {
				tempTopic = tempTopic + "#";
			}
			else
				tempTopic.remove(tempTopic.size()-1, 1);

			//qDebug()<<"checking topic: " << tempTopic;
			if (ui.cbTopics->findText(tempTopic) == -1) {
				//qDebug()<<"Adding: "<<tempTopic;
				ui.cbTopics->addItem(tempTopic);
				emit newTopic(tempTopic);
			}
			else {
				//qDebug() << "Not adding " + tempTopic;
				break;
			}
		}
	}
}

void LiveDataDock::setCompleter(const QString& topic) {
	if(!m_editing) {
		m_topicList.append(topic);
		m_completer = new QCompleter(m_topicList, this);
		m_completer->setCompletionMode(QCompleter::PopupCompletion);
		m_completer->setCaseSensitivity(Qt::CaseSensitive);
		ui.cbTopics->setCompleter(m_completer);
	}
}

void LiveDataDock::topicBeingTyped(const QString& topic) {
	if(!m_editing) {
		bool found = false;
		for (int i=0; i<ui.cbTopics->count(); ++i) {
			if(ui.cbTopics->itemText(i) == topic) {
				found = true;
				break;
			}
		}

		if(!found) {
			qDebug() << topic;
			m_editing = true;
			m_timer->start();
		}
	}
}

void LiveDataDock::topicTimeout() {
	qDebug()<<"lejart ido";
	m_editing = false;
	m_timer->stop();
}

void LiveDataDock::addSubscription() {
	if(m_mqttSubscribeButton) {
		QString newTopicName = ui.cbTopics->currentText();
		if(ui.lwSubscriptions->findItems(newTopicName, Qt::MatchExactly).isEmpty()) {
			if(ui.cbTopics->findText( newTopicName ) != -1) {
				QListWidgetItem* item;
				bool noWildcard = true;
				for(int i = 0; i < ui.lwSubscriptions->count(); ++i){
					item = ui.lwSubscriptions->item(i);
					QString subscriptionName = item->text();
					if(subscriptionName.contains('#') || subscriptionName.contains('+')) {
						if(subscriptionName.contains('#')) {
							if(newTopicName.startsWith(subscriptionName.left(subscriptionName.count() - 2)) ){
								noWildcard = false;
								break;
							}
						}
						else if (subscriptionName.contains('+')) {
							int pos = subscriptionName.indexOf('+');
							QString start = subscriptionName.left(pos);
							QString end = subscriptionName.right(subscriptionName.count() - pos);
							if(newTopicName.startsWith(start) && newTopicName.endsWith(end)) {
								noWildcard = false;
								break;
							}
						}
					}
				}
				if(noWildcard) {
					for (auto* source: m_mqttClients) {
						source->newMQTTSubscription(newTopicName, ui.cbQoS->currentIndex());
					}
				}
				else
					qDebug()<<"Another subscription, which includes wildcards, already contains this topic";

			}
			else
				qDebug()<< "There is no such topic listed in the combo box";
		}
		else
			qDebug()<< "You already subscribed to this topic";
	} else if (!m_mqttUnsubscribeName.isEmpty()) {
		for (auto* source: m_mqttClients) {
			source->removeMQTTSubscription(m_mqttUnsubscribeName);
		}
		m_mqttUnsubscribeName.clear();

		for(int row = 0; row<ui.lwSubscriptions->count(); row++)  {
			if(ui.lwSubscriptions->item(row)->text() == m_mqttUnsubscribeName) {
				qDebug()<<"subscription found at  "<<ui.lwSubscriptions->item(row)->text() <<"and removed in list widget";
				delete ui.lwSubscriptions->item(row);
			}
		}
	}
}

void LiveDataDock::fillSubscriptions() {
	const MQTTClient* const fds = m_mqttClients.at(0);
	ui.lwSubscriptions->clear();
	QVector<QString> subscriptions = fds->mqttSubscribtions();
	for (int i = 0; i < subscriptions.count(); ++i) {
		ui.lwSubscriptions->addItem(subscriptions[i]);
	}
	m_editing = false;
}

void LiveDataDock::stopStartReceive() {
	if (m_interpretMessage) {
		m_messageTimer->stop();
		disconnect(m_client, &QMqttClient::messageReceived, this, &LiveDataDock::mqttMessageReceived);
		m_interpretMessage = false;
		m_messageTimer->start(10000);
	}
	else {
		m_messageTimer->stop();
		if(m_MQTTUsed) {
			connect(m_client, &QMqttClient::messageReceived, this, &LiveDataDock::mqttMessageReceived);
			m_interpretMessage = true;
			m_messageTimer->start(3000);
		}
	}
}

void LiveDataDock::mqttButtonSubscribe(const QString& text) {
	if(!m_mqttSubscribeButton) {
		ui.bTopics->setText("Subscribe");
		m_mqttSubscribeButton = true;
	}
}

void LiveDataDock::mqttButtonUnsubscribe(const QString& item) {
	qDebug()<< "trying to set unsubscribe, mqttSubscribeButton's value: "<<m_mqttSubscribeButton;
	ui.bTopics->setText("Unsubscribe");
	m_mqttSubscribeButton = false;
	m_mqttUnsubscribeName = item;
	qDebug()<<"LiveDataDock: Unsubscribe from:"<<m_mqttUnsubscribeName;
}
#endif
