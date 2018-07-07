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
#include <QStandardItemModel>

#ifdef HAVE_MQTT
#include <QMessageBox>
#endif


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

	connect(ui.sbKeepNValues, static_cast<void (QSpinBox::*) (int)>(&QSpinBox::valueChanged), this, &LiveDataDock::keepNValuesChanged);
	connect(ui.sbSampleSize, static_cast<void (QSpinBox::*) (int)>(&QSpinBox::valueChanged), this, &LiveDataDock::sampleSizeChanged);
	connect(ui.cbUpdateType, static_cast<void (QComboBox::*) (int)>(&QComboBox::currentIndexChanged), this, &LiveDataDock::updateTypeChanged);
	connect(ui.cbReadingType, static_cast<void (QComboBox::*) (int)>(&QComboBox::currentIndexChanged), this, &LiveDataDock::readingTypeChanged);

#ifdef HAVE_MQTT
	m_timer->setInterval(10000);

	connect(m_client, &QMqttClient::connected, this, &LiveDataDock::onMQTTConnect);
	connect(m_client, &QMqttClient::messageReceived, this, &LiveDataDock::mqttMessageReceived);
	connect(this, &LiveDataDock::newTopic, this, &LiveDataDock::setCompleter);
	connect(m_timer, &QTimer::timeout, this, &LiveDataDock::topicTimeout);
	connect(ui.bSubscribe, &QPushButton::clicked, this, &LiveDataDock::addSubscription);
	connect(ui.bUnsubscribe, &QPushButton::clicked, this, &LiveDataDock::removeSubscription);
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
	connect(ui.leTopics, &QLineEdit::textChanged, this, &LiveDataDock::searchTreeItem);

	ui.bSubscribe->setIcon(ui.bSubscribe->style()->standardIcon(QStyle::SP_ArrowRight));
	ui.bUnsubscribe->setIcon(ui.bUnsubscribe->style()->standardIcon(QStyle::SP_BrowserStop));
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

	ui.sbKeepNValues->setValue(fds->keepNvalues());

	if (fds->readingType() == MQTTClient::ReadingType::TillEnd) {
		ui.lSampleSize->hide();
		ui.sbSampleSize->hide();
	} else
		ui.sbSampleSize->setValue(fds->sampleRate());

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

	ui.gbManageSubscriptions->show();
	ui.bSubscribe->show();
	ui.bUnsubscribe->show();
	ui.twTopics->show();
	ui.leTopics->show();
	ui.lTopicSearch->show();

	ui.twSubscriptions->show();
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
		ui.bPausePlayReading->setText(i18n("Continue Reading"));
		ui.bPausePlayReading->setIcon(QIcon::fromTheme(QLatin1String("media-record")));
	} else {
		ui.bPausePlayReading->setText(i18n("Pause Reading"));
		ui.bPausePlayReading->setIcon(QIcon::fromTheme(QLatin1String("media-playback-pause")));
	}

	ui.sbKeepNValues->setValue(fds->keepNValues());

	// disable "whole file" when having no file (i.e. socket or port)
	const QStandardItemModel* model = qobject_cast<const QStandardItemModel*>(ui.cbReadingType->model());
	QStandardItem* item = model->item(LiveDataSource::ReadingType::WholeFile);
	if (fds->sourceType() == LiveDataSource::SourceType::FileOrPipe)
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	else
		item->setFlags(item->flags() & ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled));

	if (fds->readingType() == LiveDataSource::ReadingType::TillEnd || fds->readingType() == LiveDataSource::ReadingType::WholeFile) {
		ui.lSampleSize->hide();
		ui.sbSampleSize->hide();
	} else
		ui.sbSampleSize->setValue(fds->sampleSize());

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
	ui.bSubscribe->hide();
	ui.bUnsubscribe->hide();
	ui.twTopics->hide();
	ui.leTopics->hide();
	ui.lTopicSearch->hide();
	ui.twSubscriptions->hide();
	ui.gbManageSubscriptions->hide();
	ui.lQoS->hide();
	ui.cbQoS->hide();
}

/*!
 * \brief Modifies the sample rate of the live data sources or MQTTClient objects
 * \param sampleRate
 */
void LiveDataDock::sampleSizeChanged(int sampleSize) {
	if(!m_liveDataSources.isEmpty()) {
		for (auto* source : m_liveDataSources)
			source->setSampleSize(sampleSize);
	}
#ifdef HAVE_MQTT
	else if (!m_mqttClients.isEmpty()) {
		for (auto* client : m_mqttClients)
			client->setSampleRate(sampleSize);
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
			ui.lSampleSize->hide();
			ui.sbSampleSize->hide();
		} else {
			ui.lSampleSize->show();
			ui.sbSampleSize->show();
		}

		for (auto* source : m_liveDataSources)
			source->setReadingType(type);
	}
#ifdef HAVE_MQTT
	else if (!m_mqttClients.isEmpty()) {
		MQTTClient::ReadingType type = static_cast<MQTTClient::ReadingType>(idx);

		if (type == MQTTClient::ReadingType::TillEnd) {
			ui.lSampleSize->hide();
			ui.sbSampleSize->hide();
		} else {
			ui.lSampleSize->show();
			ui.sbSampleSize->show();
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
void LiveDataDock::keepNValuesChanged(const int keepNValues) {
	if(!m_liveDataSources.isEmpty())  {
		for (auto* source : m_liveDataSources)
			source->setKeepNValues(keepNValues);
	}
#ifdef HAVE_MQTT
	else if (!m_mqttClients.isEmpty()) {
		for (auto* client : m_mqttClients)
			client->setKeepNvalues(keepNValues);
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
		ui.bPausePlayReading->setText(i18n("Continue Reading"));
		ui.bPausePlayReading->setIcon(QIcon::fromTheme(QLatin1String("media-record")));
	} else {
		continueReading();
		ui.bPausePlayReading->setText(i18n("Pause Reading"));
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
	QTimer::singleShot(3000, this, &LiveDataDock::fillSubscriptions);
}

void LiveDataDock::mqttMessageReceived(const QByteArray& message, const QMqttTopicName& topic) {
	if(!m_addedTopics.contains(topic.name())) {
		m_addedTopics.push_back(topic.name());
		QStringList name;
		QChar sep = '/';
		QString rootName;
		if(topic.name().contains(sep)) {
			QStringList list = topic.name().split(sep, QString::SkipEmptyParts);

			rootName = list.at(0);
			name.append(list.at(0));
			QTreeWidgetItem* currentItem;
			int topItemIdx = -1;
			for(int i = 0; i < ui.twTopics->topLevelItemCount(); ++i) {
				if(ui.twTopics->topLevelItem(i)->text(0) == list.at(0)) {
					topItemIdx = i;
					break;
				}
			}
			if( topItemIdx < 0) {
				currentItem = new QTreeWidgetItem(name);
				ui.twTopics->addTopLevelItem(currentItem);
				for(int i = 1; i < list.size(); ++i) {
					name.clear();
					name.append(list.at(i));
					currentItem->addChild(new QTreeWidgetItem(name));
					currentItem = currentItem->child(0);
				}
			} else {
				currentItem = ui.twTopics->topLevelItem(topItemIdx);
				int listIdx = 1;
				for(; listIdx < list.size(); ++listIdx) {
					QTreeWidgetItem* childItem = nullptr;
					bool found = false;
					for(int j = 0; j < currentItem->childCount(); ++j) {
						childItem = currentItem->child(j);
						if(childItem->text(0) == list.at(listIdx)) {
							found = true;
							currentItem = childItem;
							break;
						}
					}
					if(!found)
						break;
				}

				for(; listIdx < list.size(); ++listIdx) {
					name.clear();
					name.append(list.at(listIdx));
					currentItem->addChild(new QTreeWidgetItem(name));
					currentItem = currentItem->child(currentItem->childCount() - 1);
				}
			}
		}
		else {
			rootName = topic.name();
			name.append(topic.name());
			ui.twTopics->addTopLevelItem(new QTreeWidgetItem(name));
		}

		for(int i = 0; i < ui.twSubscriptions->topLevelItemCount(); ++i) {
			QStringList subscriptionName = ui.twSubscriptions->topLevelItem(i)->text(0).split('/', QString::SkipEmptyParts);
			if (rootName == subscriptionName[0]) {
				qDebug()<<topic.name();
				fillSubscriptions();
				break;
			}
		}

		emit newTopic(rootName);
	}
}

void LiveDataDock::removeSubscription() {
	QTreeWidgetItem* unsubscribeItem = ui.twSubscriptions->currentItem();

	if(unsubscribeItem != nullptr) {
		if(unsubscribeItem->parent() == nullptr) {
			for (auto* source: m_mqttClients) {
				source->removeMQTTSubscription(unsubscribeItem->text(0));
			}
			ui.twSubscriptions->takeTopLevelItem(ui.twSubscriptions->indexOfTopLevelItem(unsubscribeItem));
		}
		else{
			while(unsubscribeItem->parent() != nullptr) {
				for(int i = 0; i < unsubscribeItem->parent()->childCount(); ++i) {
					if(unsubscribeItem->text(0) != unsubscribeItem->parent()->child(i)->text(0)) {						
						for (auto* source: m_mqttClients) {
							source->addBeforeRemoveSubscription(unsubscribeItem->parent()->child(i)->text(0), ui.cbQoS->currentIndex());
						}
						ui.twSubscriptions->addTopLevelItem(unsubscribeItem->parent()->takeChild(i));
						i--;
					}
				}
				unsubscribeItem = unsubscribeItem->parent();
			}

			for (auto* source: m_mqttClients) {
				source->removeMQTTSubscription(unsubscribeItem->text(0));
			}
			ui.twSubscriptions->takeTopLevelItem(ui.twSubscriptions->indexOfTopLevelItem(unsubscribeItem));
		}
	} else
		QMessageBox::warning(this, "Warning", "You didn't select any item from the Tree Widget");
}

void LiveDataDock::setCompleter(const QString& topic) {
	if(!m_editing) {
		if(!m_topicList.contains(topic)) {
			m_topicList.append(topic);
			m_completer = new QCompleter(m_topicList, this);
			m_completer->setCompletionMode(QCompleter::PopupCompletion);
			m_completer->setCaseSensitivity(Qt::CaseSensitive);
			ui.leTopics->setCompleter(m_completer);
		}
	}
}

void LiveDataDock::topicTimeout() {
	qDebug()<<"lejart ido";
	m_editing = false;
	m_timer->stop();
}

void LiveDataDock::addSubscription() {
	QString name;
	QTreeWidgetItem *item = ui.twTopics->currentItem();
	if(item != nullptr) {
		QTreeWidgetItem *tempItem = item;
		name.prepend(item->text(0));
		if(item->childCount() != 0)
			name.append("/#");

		while(tempItem->parent() != nullptr) {
			tempItem = tempItem->parent();
			name.prepend(tempItem->text(0) + "/");
		}

		QList<QTreeWidgetItem *> topLevelList = ui.twSubscriptions->findItems(name, Qt::MatchExactly);

		if(topLevelList.isEmpty() || topLevelList.first()->parent() != nullptr) {

			qDebug() << name;
			bool foundSuperior = false;
			bool foundEqual = false;
			QVector<QString> equalTopics;

			for(int i = 0; i < ui.twSubscriptions->topLevelItemCount(); ++i) {
				qDebug()<<i<<" "<<ui.twSubscriptions->topLevelItemCount();
				if(checkTopicContains(name, ui.twSubscriptions->topLevelItem(i)->text(0))
						&& name != ui.twSubscriptions->topLevelItem(i)->text(0)) {
					qDebug()<<"1"<<name<<" "<< ui.twSubscriptions->topLevelItem(i)->text(0);

					ui.twSubscriptions->topLevelItem(i)->takeChildren();
					ui.twSubscriptions->takeTopLevelItem(i);

					qDebug()<<"After Delete";
					i--;
					continue;
				}
				qDebug()<<"checked inferior";

				if(checkTopicContains(ui.twSubscriptions->topLevelItem(i)->text(0), name)
						&& name != ui.twSubscriptions->topLevelItem(i)->text(0)) {
					foundSuperior = true;
					qDebug()<<"2"<<name<<" "<< ui.twSubscriptions->topLevelItem(i)->text(0);
					break;
				}
				qDebug()<<"checked superior";

				QString commonTopic = checkCommonLevel(ui.twSubscriptions->topLevelItem(i)->text(0), name);
				if(!commonTopic.isEmpty()) {
					equalTopics.push_back(ui.twSubscriptions->topLevelItem(i)->text(0));
				}
				qDebug()<<"checked common";
			}

			if(!equalTopics.isEmpty()) {
				qDebug()<<"Equal topics not empty";
				int level = commonLevelIndex(name, equalTopics.first());
				QStringList commonList = name.split('/', QString::SkipEmptyParts);
				QTreeWidgetItem* currentItem;
				for(int i = 0; i < ui.twTopics->topLevelItemCount(); ++i) {
					if(ui.twTopics->topLevelItem(i)->text(0) == commonList.first()) {
						currentItem = ui.twTopics->topLevelItem(i);
						break;
					}
				}

				int levelIdx = 1;
				while(levelIdx < level) {
					for(int j = 0; j < currentItem->childCount(); ++j) {
						if(currentItem->child(j)->text(0) == commonList[levelIdx]) {
							qDebug()<<"Child: "<<currentItem->child(j)->text(0);
							currentItem = currentItem->child(j);
							levelIdx++;
							break;
						}
					}
				}

				qDebug()<<currentItem->text(0)<<"  "<<currentItem->childCount();
				if(equalTopics.size() == currentItem->childCount() - 1) {
					qDebug()<<"foundEqual set true";
					foundEqual = true;
				}
			}

			if(!foundSuperior) {
				qDebug()<<"Adding new topic";
				QStringList toplevelName;
				toplevelName.push_back(name);
				QTreeWidgetItem* newTopLevelItem = new QTreeWidgetItem(toplevelName);
				ui.twSubscriptions->addTopLevelItem(newTopLevelItem);

				if(name.endsWith("#")) {
					addSubscriptionChildren(item, newTopLevelItem);
				}

				for (auto* source: m_mqttClients) {
					source->newMQTTSubscription(name, ui.cbQoS->currentIndex());
				}
			}

			if(foundEqual) {
				QString commonTopic;

				commonTopic = checkCommonLevel(equalTopics.first(), name);
				QStringList nameList;
				nameList.append(commonTopic);
				QTreeWidgetItem* newTopic = new QTreeWidgetItem(nameList);
				ui.twSubscriptions->addTopLevelItem(newTopic);

				for(int j = 0; j < ui.twSubscriptions->topLevelItemCount(); ++j){
					if(ui.twSubscriptions->topLevelItem(j)->text(0) == name) {
						newTopic->addChild(ui.twSubscriptions->takeTopLevelItem(j));
						break;
					}
				}

				for(int i = 0; i < equalTopics.size(); ++i) {
					for(int j = 0; j < ui.twSubscriptions->topLevelItemCount(); ++j){
						if(ui.twSubscriptions->topLevelItem(j)->text(0) == equalTopics[i]) {
							newTopic->addChild(ui.twSubscriptions->takeTopLevelItem(j));
							break;
						}
					}
				}

				for(int i = 0; i < ui.twSubscriptions->topLevelItemCount(); ++i) {
					if(checkTopicContains(commonTopic, ui.twSubscriptions->topLevelItem(i)->text(0)) &&
							commonTopic != ui.twSubscriptions->topLevelItem(i)->text(0) ) {
						ui.twSubscriptions->topLevelItem(i)->takeChildren();
						ui.twSubscriptions->takeTopLevelItem(i);
						i--;
					}
				}

				bool foundNewEqual;
				do{
					foundNewEqual = false;
					equalTopics.clear();

					for(int i = 0; i < ui.twSubscriptions->topLevelItemCount(); ++i) {
						if(!checkCommonLevel(ui.twSubscriptions->topLevelItem(i)->text(0), commonTopic).isEmpty()) {
							equalTopics.push_back(ui.twSubscriptions->topLevelItem(i)->text(0));
						}
					}

					if(!equalTopics.isEmpty()) {
						int level = commonLevelIndex(name, equalTopics.first());
						QStringList commonList = name.split('/', QString::SkipEmptyParts);
						QTreeWidgetItem* currentItem;
						for(int i = 0; i < ui.twTopics->topLevelItemCount(); ++i) {
							if(ui.twTopics->topLevelItem(i)->text(0) == commonList.first()) {
								currentItem = ui.twTopics->topLevelItem(i);
								break;
							}
						}

						int levelIdx = 1;
						while(levelIdx < level) {
							for(int j = 0; j < currentItem->childCount(); ++j) {
								if(currentItem->child(j)->text(0) == commonList[j]) {
									currentItem = currentItem->child(j);
									levelIdx++;
									break;
								}
							}
						}

						qDebug()<<currentItem->text(0)<<"  "<<currentItem->childCount();
						if(equalTopics.size() == currentItem->childCount() - 1) {
							foundNewEqual = true;
						}
					}

					if(foundNewEqual) {
						QString newCommonTopic = checkCommonLevel(equalTopics.first(), commonTopic);
						nameList.clear();
						nameList.append(newCommonTopic);
						QTreeWidgetItem* oldTopic = newTopic;
						newTopic = new QTreeWidgetItem(nameList);
						ui.twSubscriptions->addTopLevelItem(newTopic);

						nameList.clear();
						nameList.append(commonTopic);
						newTopic->addChild(ui.twSubscriptions->takeTopLevelItem(ui.twSubscriptions->indexOfTopLevelItem(oldTopic)));

						for(int i = 0; i < equalTopics.size(); ++i) {
							for(int j = 0; j < ui.twSubscriptions->topLevelItemCount(); ++j){
								if(ui.twSubscriptions->topLevelItem(j)->text(0) == equalTopics[i]) {
									nameList.clear();
									nameList.append(equalTopics[i]);
									newTopic->addChild(ui.twSubscriptions->takeTopLevelItem(j));
									break;
								}
							}
						}

						for(int i = 0; i < ui.twSubscriptions->topLevelItemCount(); ++i) {
							if(checkTopicContains(newCommonTopic, ui.twSubscriptions->topLevelItem(i)->text(0)) &&
									newCommonTopic != ui.twSubscriptions->topLevelItem(i)->text(0) ) {
								ui.twSubscriptions->topLevelItem(i)->takeChildren();
								ui.twSubscriptions->takeTopLevelItem(i);
								i--;
							}
						}

						commonTopic = newCommonTopic;
					}
				} while(foundNewEqual);

				for (auto* source: m_mqttClients) {
					source->newMQTTSubscription(commonTopic, ui.cbQoS->currentIndex());
				}
			}

			if(foundSuperior) {
				QMessageBox::warning(this, "Warning", "You already subscribed to a topic containing this one");
			}
		}
		else
			QMessageBox::warning(this, "Warning", "You already subscribed to this topic");
	}
	else
		QMessageBox::warning(this, "Warning", "You didn't select any item from the Tree Widget");
}

void LiveDataDock::fillSubscriptions() {
	const MQTTClient* const fds = m_mqttClients.at(0);

	ui.twSubscriptions->clear();

	QVector<QString> subscriptions = fds->mqttSubscribtions();
	for (int i = 0; i < subscriptions.count(); ++i) {
		QStringList name;
		name.append(subscriptions[i]);

		bool found = false;
		for(int j = 0; j < ui.twSubscriptions->topLevelItemCount(); ++j) {
			if(ui.twSubscriptions->topLevelItem(j)->text(0) == subscriptions[i]) {
				found = true;
				break;
			}
		}

		if(!found) {
			qDebug()<<"add:" << subscriptions[i];
			QTreeWidgetItem* newItem = new QTreeWidgetItem(name);
			ui.twSubscriptions->addTopLevelItem(newItem);
			name.clear();
			name = subscriptions[i].split('/', QString::SkipEmptyParts);

			QTreeWidgetItem* topic = nullptr;
			for(int j = 0; j < ui.twTopics->topLevelItemCount(); ++j) {
				if(ui.twTopics->topLevelItem(j)->text(0) == name[0]) {
					qDebug()<<"found top level topic: "<<name[0]<<" "<<j;
					topic = ui.twTopics->topLevelItem(j);
					break;
				}
			}

			if(topic != nullptr) {
				qDebug()<<"restoring Children";
				restoreSubscriptionChildren(topic, newItem, name, 1);
			}
		}

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

bool LiveDataDock::checkTopicContains(const QString &superior, const QString &inferior) {
	if (superior == inferior)
		return true;
	else {
		if(superior.contains("/")) {
			QStringList superiorList = superior.split('/', QString::SkipEmptyParts);
			QStringList inferiorList = inferior.split('/', QString::SkipEmptyParts);

			bool ok = true;
			for(int i = 0; i < superiorList.size(); ++i) {
				if(superiorList.at(i) != inferiorList.at(i)) {
					if((superiorList.at(i) != "+") &&
							!(superiorList.at(i) == "#" && i == superiorList.size() - 1)) {
						qDebug() <<superiorList.at(i)<<"  "<<inferiorList.at(i);
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

void LiveDataDock::searchTreeItem(const QString& rootName) {
	m_editing = true;
	m_timer->start();

	qDebug()<<rootName;
	int topItemIdx = -1;
	for(int i = 0; i< ui.twTopics->topLevelItemCount(); ++i)
		if(ui.twTopics->topLevelItem(i)->text(0) == rootName) {
			topItemIdx = i;
			break;
		}

	if(topItemIdx >= 0) {
		qDebug() << "Scroll";
		ui.twTopics->scrollToItem(ui.twTopics->topLevelItem(topItemIdx), QAbstractItemView::ScrollHint::PositionAtTop);
	}
}

QString LiveDataDock::checkCommonLevel(const QString& first, const QString& second) {
	qDebug()<<first<<"  "<<second;
	QStringList firstList = first.split('/', QString::SkipEmptyParts);
	QStringList secondtList = second.split('/', QString::SkipEmptyParts);
	QString commonTopic = "";

	if(!firstList.isEmpty()) {
		if(firstList.size() == secondtList.size() && (first != second))	{
			int matchIndex = -1;
			for(int i = 0; i < firstList.size(); ++i) {
				if(firstList.at(i) != secondtList.at(i)) {
					matchIndex = i;
					break;
				}
			}
			bool differ = false;
			if(matchIndex > 0 && matchIndex < firstList.size() -1) {
				for(int j = matchIndex +1; j < firstList.size(); ++j) {
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
					if(i != matchIndex)
						commonTopic.append(firstList.at(i));
					else
						commonTopic.append("+");

					if(i != firstList.size() - 1)
						commonTopic.append("/");
				}
			}
		}
	}
	qDebug() << "Common topic: "<<commonTopic;
	return commonTopic;
}

void LiveDataDock::addSubscriptionChildren(QTreeWidgetItem * topic, QTreeWidgetItem * subscription) {
	if(topic->childCount() > 0) {
		for(int i = 0; i < topic->childCount(); ++i) {
			QTreeWidgetItem* temp = topic->child(i);
			QString name;
			if(topic->child(i)->childCount() > 0) {
				name.append(temp->text(0) + "/#");
				while(temp->parent() != nullptr) {
					temp = temp->parent();
					name.prepend(temp->text(0) + "/");
				}

			} else {
				name.append(temp->text(0));
				while(temp->parent() != nullptr) {
					temp = temp->parent();
					name.prepend(temp->text(0) + "/");
				}
			}
			QStringList nameList;
			nameList.append(name);
			QTreeWidgetItem* childItem = new QTreeWidgetItem(nameList);
			subscription->addChild(childItem);
			addSubscriptionChildren(topic->child(i), childItem);
		}
	}
}

void LiveDataDock::restoreSubscriptionChildren(QTreeWidgetItem * topic, QTreeWidgetItem * subscription, const QStringList& list, int level) {
	if(list[level] != "+" && list[level] != "#" && level < list.size() - 1) {
		for(int i = 0; i < topic->childCount(); ++i) {
			if(topic->child(i)->text(0) == list[level]) {
				restoreSubscriptionChildren(topic->child(i), subscription, list, level + 1);
				break;
			}
		}
	} else if (list[level] == "+") {
		for(int i = 0; i < topic->childCount(); ++i) {
			QString name;
			name.append(topic->child(i)->text(0));
			for(int j = level + 1; j < list.size(); ++j) {
				name.append("/" + list[j]);
			}
			QTreeWidgetItem* temp = topic->child(i);
			while(temp->parent() != nullptr) {
				temp = temp->parent();
				name.prepend(temp->text(0) + "/");
			}
			QStringList nameList;
			nameList.append(name);
			QTreeWidgetItem* newItem = new QTreeWidgetItem(nameList);
			subscription->addChild(newItem);
			restoreSubscriptionChildren(topic->child(i), newItem, list, level + 1);
		}
	} else if (list[level] == "#") {
		/*if(topic->parent() != nullptr) {
			QString name;
			name.append("#");
			name.prepend(topic->text(0) + "/");

			QTreeWidgetItem* temp = topic;
			while(temp->parent() != nullptr) {
				temp = temp->parent();
				name.prepend(temp->text(0) + "/");
			}

			QStringList nameList;
			nameList.append(name);
			QTreeWidgetItem* newItem = new QTreeWidgetItem(nameList);
			subscription->addChild(newItem);
			addSubscriptionChildren(topic, newItem);
		}*/
		addSubscriptionChildren(topic, subscription);

	}
}

int LiveDataDock::commonLevelIndex(const QString& first, const QString& second) {
	qDebug()<<first<<"  "<<second;
	QStringList firstList = first.split('/', QString::SkipEmptyParts);
	QStringList secondtList = second.split('/', QString::SkipEmptyParts);
	QString commonTopic = "";
	int matchIndex = -1;

	if(!firstList.isEmpty()) {
		if(firstList.size() == secondtList.size() && (first != second))	{
			for(int i = 0; i < firstList.size(); ++i) {
				if(firstList.at(i) != secondtList.at(i)) {
					matchIndex = i;
					break;
				}
			}
			bool differ = false;
			if(matchIndex > 0) {
				for(int j = matchIndex +1; j < firstList.size(); ++j) {
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
					if(i != matchIndex)
						commonTopic.append(firstList.at(i));
					else
						commonTopic.append("+");

					if(i != firstList.size() - 1)
						commonTopic.append("/");
				}
			}
		}
	}

	qDebug() << "Common topic: "<<commonTopic;
	if(!commonTopic.isEmpty())
		return matchIndex;
	else
		return -1;
}
#endif
