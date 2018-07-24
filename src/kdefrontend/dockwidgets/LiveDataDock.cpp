/***************************************************************************
File                 : LiveDataDock.cpp
Project              : LabPlot
Description          : Dock widget for live data properties
--------------------------------------------------------------------
Copyright            : (C) 2017 by Fabian Kristof (fkristofszabolcs@gmail.com)
Copyright            : (C) 2018 by Kovacs Ferencz (kferike98@gmail.com)
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
#include <QCompleter>
#include <QString>
#include <QTimer>
#include <QTreeWidgetItem>
#include <QStandardItemModel>

#ifdef HAVE_MQTT
#include <QMessageBox>
#endif

LiveDataDock::LiveDataDock(QWidget* parent) :
	QWidget(parent),
	#ifdef HAVE_MQTT
	m_searching(true),
	m_previousMQTTClient(nullptr),
	m_searchTimer(new QTimer()),
	m_interpretMessage(true),
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
	m_searchTimer->setInterval(10000);

	connect(this, &LiveDataDock::newTopic, this, &LiveDataDock::setTopicCompleter);
	connect(m_searchTimer, &QTimer::timeout, this, &LiveDataDock::topicTimeout);
	connect(ui.bSubscribe, &QPushButton::clicked, this, &LiveDataDock::addSubscription);
	connect(ui.bUnsubscribe, &QPushButton::clicked, this, &LiveDataDock::removeSubscription);
	connect(ui.chbWill, &QCheckBox::stateChanged, this, &LiveDataDock::useWillMessage);
	connect(ui.cbWillQoS, static_cast<void (QComboBox::*) (int)>(&QComboBox::currentIndexChanged), this, &LiveDataDock::willQoSChanged);
	connect(ui.chbWillRetain, &QCheckBox::stateChanged, this, &LiveDataDock::willRetainChanged);
	connect(ui.cbWillTopic, &QComboBox::currentTextChanged, this, &LiveDataDock::willTopicChanged);
	connect(ui.cbWillMessageType, static_cast<void (QComboBox::*) (int)>(&QComboBox::currentIndexChanged), this, &LiveDataDock::willMessageTypeChanged);
	connect(ui.leWillOwnMessage, &QLineEdit::textChanged, this, &LiveDataDock::willOwnMessageChanged);
	connect(ui.cbWillUpdate, static_cast<void (QComboBox::*) (int)>(&QComboBox::currentIndexChanged), this, &LiveDataDock::willUpdateTypeChanged);
	connect(ui.bWillUpdateNow, &QPushButton::clicked, this, &LiveDataDock::willUpdateNow);
	connect(ui.leWillUpdateInterval, &QLineEdit::textChanged, this, &LiveDataDock::willUpdateIntervalChanged);
	connect(ui.lwWillStatistics, &QListWidget::itemChanged, this, &LiveDataDock::statisticsChanged);
	connect(ui.leTopics, &QLineEdit::textChanged, this, &LiveDataDock::scrollToTopicTreeItem);
	connect(ui.leSubscriptions, &QLineEdit::textChanged, this, &LiveDataDock::scrollToSubsriptionTreeItem);

	ui.bSubscribe->setIcon(ui.bSubscribe->style()->standardIcon(QStyle::SP_ArrowRight));
	ui.bUnsubscribe->setIcon(ui.bUnsubscribe->style()->standardIcon(QStyle::SP_BrowserStop));
#endif
}

LiveDataDock::~LiveDataDock() {
	delete m_searchTimer;
	QMapIterator<QString, QMqttClient*> clients(m_clients);
	while(clients.hasNext()) {
		clients.next();
		delete clients.value();
	}
}

#ifdef HAVE_MQTT
/*!
 * \brief Sets the MQTTClients of this dock widget
 * \param clients
 */
void LiveDataDock::setMQTTClients(const QList<MQTTClient *> &clients) {
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

	ui.sbKeepNValues->setValue(fds->keepNValues());

	if (fds->readingType() == MQTTClient::ReadingType::TillEnd) {
		ui.lSampleSize->hide();
		ui.sbSampleSize->hide();
	} else
		ui.sbSampleSize->setValue(fds->sampleSize());

	// disable "whole file" when having no file (i.e. socket or port)
	const QStandardItemModel* model = qobject_cast<const QStandardItemModel*>(ui.cbReadingType->model());
	QStandardItem* item = model->item(LiveDataSource::ReadingType::WholeFile);
	item->setFlags(item->flags() & ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled));

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

	//show MQTT connected options
	ui.gbManageSubscriptions->show();
	ui.bSubscribe->show();
	ui.bUnsubscribe->show();
	ui.twTopics->show();
	ui.leTopics->show();
	ui.lTopicSearch->show();
	ui.twSubscriptions->show();
	ui.lQoS->show();
	ui.cbQoS->show();
	ui.chbWill->show();

	//if there isn't a client with this hostname we instantiate a new one
	if(m_clients[fds->clientHostName()] == nullptr) {
		m_clients[fds->clientHostName()] = new QMqttClient();

		connect(fds, &MQTTClient::clientAboutToBeDeleted, this, &LiveDataDock::removeClient);

		connect(m_clients[fds->clientHostName()], &QMqttClient::connected, this, &LiveDataDock::onMQTTConnect);
		connect(m_clients[fds->clientHostName()], &QMqttClient::messageReceived, this, &LiveDataDock::mqttMessageReceived);

		m_clients[fds->clientHostName()]->setHostname(fds->clientHostName());
		m_clients[fds->clientHostName()]->setPort(fds->clientPort());

		if(fds->mqttUseAuthentication()) {
			m_clients[fds->clientHostName()]->setUsername(fds->clientUserName());
			m_clients[fds->clientHostName()]->setPassword(fds->clientPassword());
		}

		if(fds->mqttUseID()) {
			m_clients[fds->clientHostName()]->setClientId(fds->clientID());
		}

		m_clients[fds->clientHostName()]->connectToHost();
	}

	if(m_previousMQTTClient == nullptr) {
		connect(fds, &MQTTClient::mqttSubscribed, this, &LiveDataDock::fillSubscriptions);
		connect(fds, &MQTTClient::mqttTopicsChanged, this, &LiveDataDock::updateWillTopics);
	}
	//if the previous MQTTClient's host name was different from the current one we have to disconnect some slots
	//and clear the tree widgets
	else if(m_previousMQTTClient->clientHostName() != fds->clientHostName()) {
		qDebug()<<"At load host name not equal: "<<m_previousMQTTClient->clientHostName()<<" "<<fds->clientHostName();
		disconnect(m_previousMQTTClient, &MQTTClient::mqttSubscribed, this, &LiveDataDock::fillSubscriptions);
		disconnect(m_previousMQTTClient, &MQTTClient::mqttTopicsChanged, this, &LiveDataDock::updateWillTopics);
		disconnect(m_clients[m_previousMQTTClient->clientHostName()], &QMqttClient::messageReceived, this, &LiveDataDock::mqttMessageReceived);
		connect(m_clients[m_previousMQTTClient->clientHostName()], &QMqttClient::messageReceived, this, &LiveDataDock::mqttMessageReceivedInBackground);

		disconnect(m_clients[fds->clientHostName()], &QMqttClient::messageReceived, this, &LiveDataDock::mqttMessageReceivedInBackground);

		ui.twTopics->clear();
		//repopulating the tree widget with the already known topics of the client
		for(int i = 0; i < m_addedTopics[fds->clientHostName()].size(); ++i) {
			addTopicToTree(m_addedTopics[fds->clientHostName()].at(i));
		}

		//fill subscriptions tree widget
		ui.twSubscriptions->clear();
		fillSubscriptions();

		connect(fds, &MQTTClient::mqttSubscribed, this, &LiveDataDock::fillSubscriptions);
		connect(fds, &MQTTClient::mqttTopicsChanged, this, &LiveDataDock::updateWillTopics);
		connect(m_clients[fds->clientHostName()], &QMqttClient::messageReceived, this, &LiveDataDock::mqttMessageReceived);
	}

	//set will message connected options
	updateWillTopics();
	ui.leWillOwnMessage->setText(fds->willOwnMessage());
	ui.leWillUpdateInterval->setText(QString::number(fds->willTimeInterval()));
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
 * \brief Modifies the sample size of the live data sources or MQTTClient objects
 * \param sampleSize
 */
void LiveDataDock::sampleSizeChanged(int sampleSize) {
	if(!m_liveDataSources.isEmpty()) {
		for (auto* source : m_liveDataSources)
			source->setSampleSize(sampleSize);
	}
#ifdef HAVE_MQTT
	else if (!m_mqttClients.isEmpty()) {
		for (auto* client : m_mqttClients)
			client->setSampleSize(sampleSize);
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
 * \param keepNValues
 */
void LiveDataDock::keepNValuesChanged(const int keepNValues) {
	if(!m_liveDataSources.isEmpty())  {
		for (auto* source : m_liveDataSources)
			source->setKeepNValues(keepNValues);
	}
#ifdef HAVE_MQTT
	else if (!m_mqttClients.isEmpty()) {
		for (auto* client : m_mqttClients)
			client->setKeepNValues(keepNValues);
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

/*!
 *\brief called when use will message checkbox's state is changed,
 *if state is checked it shows the options regarding the will message
 * and also sets the mqttUseWill according to state for every client in m_mqttClients
 *
 * \param state the state of the checbox
 */
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

/*!
 *\brief called when will message's QoS is changed
 * sets the will QoS level for every client in m_mqttClients
 *
 * \param QoS the QoS level of the will message
 */
void LiveDataDock::willQoSChanged(int QoS) {
	for (auto* source: m_mqttClients)
		source->setWillQoS(QoS);
}

/*!
 *\brief called when will message's retain flag is changed
 * sets the retain flag for the will message in every client in m_mqttClients
 *
 * \param state the state of the will retain chechbox
 */
void LiveDataDock::willRetainChanged(int state) {
	if(state == Qt::Checked) {
		for (auto* source: m_mqttClients)
			source->setWillRetain(true);
	} else if (state == Qt::Unchecked) {
		for (auto* source: m_mqttClients)
			source->setWillRetain(false);
	}
}

/*!
 *\brief called when will topic combobox's current item is changed
 * sets the will topic for every client in m_mqttClients
 *
 * \param topic the current text of cbWillTopic
 */
void LiveDataDock::willTopicChanged(const QString& topic) {
	qDebug()<<"topic  changed" << topic;
	for (auto* source: m_mqttClients) {
		if(source->willTopic() != topic)
			source->clearLastMessage();
		source->setWillTopic(topic);
	}
}

/*!
 *\brief called when the selected will message type is changed,
 * shows the options for the selected message type, hides the irrelevant onesd
 * sets the will message type for every client in m_mqttClients
 *
 * \param type the selected will message type
 */
void LiveDataDock::willMessageTypeChanged(int type) {
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

/*!
 *\brief called when the will own message is changed
 * sets the will own message for every client in m_mqttClients
 *
 * \param message the will message given by the user
 */
void LiveDataDock::willOwnMessageChanged(const QString& message) {
	for (auto* source: m_mqttClients)
		source->setWillOwnMessage(message);
}

/*!
 *\brief called when the mqttTopicsChanged signal of a MQTTClient from m_mqttClients is emitted
 * updates the content of the cbWillTopic with every topic belonging to the MQTTClient
 */
void LiveDataDock::updateWillTopics() {
	ui.cbWillTopic->clear();
	const MQTTClient* const fds = m_mqttClients.at(0);
	QVector<QString> topics = fds->topicNames();

	if(!topics.isEmpty()) {
		for(int i = 0; i < topics.count(); i++) {
			qDebug()<<"Live Data Dock: updating will topics: "<<topics[i];
			ui.cbWillTopic->addItem(topics[i]);
		}
		if(!fds->willTopic().isEmpty())
			ui.cbWillTopic->setCurrentText(fds->willTopic());
	}
	else
		qDebug()<<"Topic Vector Empty";
}

/*!
 *\brief called when the selected update type for the will message is changed,
 * shows the options for the selected update type, hides the irrelevant ones
 * sets the will update type for every client in m_mqttClients
 *
 * \param type the selected will update type
 */
void LiveDataDock::willUpdateTypeChanged(int updateType) {
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

		//if update type is on click we stop the will timer
		for (auto* source: m_mqttClients)
			source->stopWillTimer();
	}

}

/*!
 *\brief called when the will update now button is pressed
 * updates the will message of every client in m_mqttClients
 */
void LiveDataDock::willUpdateNow() {	
	for (auto* source: m_mqttClients)
		source->updateWillMessage();
}

/*!
 *\brief called when the update interval for will message is changed,
 * sets the will update interval for every client in m_mqttClients, then starts the will timer for each one
 *
 * \param interval the new will update interval
 */
void LiveDataDock::willUpdateIntervalChanged(const QString& interval) {
	qDebug()<<"Update interval changed " <<interval;
	for (auto* source: m_mqttClients) {
		source->setWillTimeInterval(interval.toInt());
		source->startWillTimer();
	}
}

/*!
 *\brief called when the check state of a ListWidget item is changed
 * adds or removes the statistic represented by the item from every client in m_mqttClients
 *
 * \param item the ListWidgetItem, the check state of which was changed
 */
void LiveDataDock::statisticsChanged(QListWidgetItem *item) {
	//determine the index of the item
	int idx = -1;
	for(int i =  0; i < ui.lwWillStatistics->count(); i++)
		if(item->text() == ui.lwWillStatistics->item(i)->text()) {
			idx = i;
			break;
		}

	//if it's checked we add it
	if(item->checkState() == Qt::Checked) {
		if(idx >= 0) {
			for (auto* source: m_mqttClients)
				source->addWillStatistics(static_cast<MQTTClient::WillStatistics>(idx) );
		}
	}
	//otherwise remove it
	else {
		if(idx >= 0){
			for (auto* source: m_mqttClients)
				source->removeWillStatistics(static_cast<MQTTClient::WillStatistics>(idx) );
		}
	}
}

/*!
 *\brief called when the client connects to the broker succesfully, it subscribes to every topic (# wildcard)
 * in order to later list every available topic
 */
void LiveDataDock::onMQTTConnect() {
	QMqttTopicFilter globalFilter{"#"};
	QMqttSubscription * subscription = m_clients[m_mqttClients.first()->clientHostName()]->subscribe(globalFilter, 1);
	if(!subscription)
		qDebug()<<"Couldn't make global subscription in LiveDataDock";
}

/*!
 *\brief called when the client receives a message
 * if the message arrived from a new topic, the topic is put in twTopics
 */
void LiveDataDock::mqttMessageReceived(const QByteArray& message, const QMqttTopicName& topic) {
	if(!m_addedTopics[m_mqttClients.first()->clientHostName()].contains(topic.name())) {
		m_addedTopics[m_mqttClients.first()->clientHostName()].push_back(topic.name());
		addTopicToTree(topic.name());
	}
}

/*!
 *\brief called when the subscribe button is pressed
 * subscribes to the topic represented by the current item of twTopics in every client from m_mqttClients
 */
void LiveDataDock::addSubscription() {
	QString name;
	QTreeWidgetItem *item = ui.twTopics->currentItem();
	if(item != nullptr) {
		QTreeWidgetItem *tempItem = item;

		//determine the topic name that the current item represents
		name.prepend(item->text(0));
		if(item->childCount() != 0)
			name.append("/#");
		while(tempItem->parent() != nullptr) {
			tempItem = tempItem->parent();
			name.prepend(tempItem->text(0) + "/");
		}

		//check if the subscription already exists
		QList<QTreeWidgetItem *> topLevelList = ui.twSubscriptions->findItems(name, Qt::MatchExactly);
		if(topLevelList.isEmpty() || topLevelList.first()->parent() != nullptr) {

			qDebug() << name;
			bool foundSuperior = false;

			for(int i = 0; i < ui.twSubscriptions->topLevelItemCount(); ++i) {
				qDebug()<<i<<" "<<ui.twSubscriptions->topLevelItemCount();

				//if the new subscriptions contains an already existing one, we remove the inferior one
				if(checkTopicContains(name, ui.twSubscriptions->topLevelItem(i)->text(0))
						&& name != ui.twSubscriptions->topLevelItem(i)->text(0)) {
					qDebug()<<"1"<<name<<" "<< ui.twSubscriptions->topLevelItem(i)->text(0);
					ui.twSubscriptions->topLevelItem(i)->takeChildren();
					ui.twSubscriptions->takeTopLevelItem(i);
					i--;
					continue;
				}
				qDebug()<<"checked inferior";

				//if there is a subscription containing the new one we set foundSuperior true
				if(checkTopicContains(ui.twSubscriptions->topLevelItem(i)->text(0), name)
						&& name != ui.twSubscriptions->topLevelItem(i)->text(0)) {
					foundSuperior = true;
					qDebug()<<"2"<<name<<" "<< ui.twSubscriptions->topLevelItem(i)->text(0);
					break;
				}
				qDebug()<<"checked superior";
			}

			//if there wasn't a superior subscription we can subscribe to the new topic
			if(!foundSuperior) {
				qDebug()<<"Adding new topic";
				QStringList toplevelName;
				toplevelName.push_back(name);
				QTreeWidgetItem* newTopLevelItem = new QTreeWidgetItem(toplevelName);
				ui.twSubscriptions->addTopLevelItem(newTopLevelItem);

				if(name.endsWith("#")) {
					//adding every topic that the subscription contains to twSubscriptions
					addSubscriptionChildren(item, newTopLevelItem);
				}

				//subscribe in every MQTTClient
				for (auto* source: m_mqttClients) {
					source->addMQTTSubscription(name, ui.cbQoS->currentIndex());
				}

				if(name.endsWith("#")) {
					//if an already existing subscription contains a topic that the new subscription also contains
					//we decompose the already existing subscription
					//by unsubscribing from its topics, that are present in the new subscription as well
					QStringList nameList = name.split('/', QString::SkipEmptyParts);
					QString root = nameList.first();
					QVector<QTreeWidgetItem*> children;
					for(int i = 0; i < ui.twSubscriptions->topLevelItemCount(); ++i) {
						if(ui.twSubscriptions->topLevelItem(i)->text(0).startsWith(root)
								&& name != ui.twSubscriptions->topLevelItem(i)->text(0)) {
							children.clear();

							//get the "leaf" children of the inspected subscription
							findSubscriptionLeafChildren(children, ui.twSubscriptions->topLevelItem(i));
							for(int j = 0; j < children.size(); ++j) {
								if(checkTopicContains(name, children[j]->text(0))) {
									qDebug()<<children[j]->text(0);

									//if the new subscription contains a topic, we unsubscribe from it
									QTreeWidgetItem* unsubscribeItem = children[j];
									while(unsubscribeItem->parent() != nullptr) {
										for(int i = 0; i < unsubscribeItem->parent()->childCount(); ++i) {
											qDebug()<<i<<" "<<unsubscribeItem->parent()->childCount();

											if(unsubscribeItem->text(0) != unsubscribeItem->parent()->child(i)->text(0)) {
												//add topic as subscription to every client
												for (auto* source: m_mqttClients) {
													source->addBeforeRemoveSubscription(unsubscribeItem->parent()->child(i)->text(0), ui.cbQoS->currentIndex());
												}
												//also add it to twSubscripitons
												ui.twSubscriptions->addTopLevelItem(unsubscribeItem->parent()->takeChild(i));
												i--;
											} else {
												//before we remove the topic, we reparent it to the new subscription
												//so no data is lost
												for (auto* source: m_mqttClients) {
													source->reparentTopic(unsubscribeItem->text(0), name);
												}
											}
										}
										unsubscribeItem = unsubscribeItem->parent();
									}

									qDebug()<<"Remove: "<<unsubscribeItem->text(0);
									//Remove topic/subscription
									for (auto* source: m_mqttClients) {
										source->removeMQTTSubscription(unsubscribeItem->text(0));
									}
									ui.twSubscriptions->takeTopLevelItem(ui.twSubscriptions->indexOfTopLevelItem(unsubscribeItem));
								}
							}
						}
					}
				}

				manageCommonLevelSubscriptions();
				updateSubscriptionCompleter();
			} else {
				QMessageBox::warning(this, "Warning", "You already subscribed to a topic containing this one");
			}
		}
		else
			QMessageBox::warning(this, "Warning", "You already subscribed to this topic");
	}
	else
		QMessageBox::warning(this, "Warning", "You didn't select any item from the Tree Widget");
}

/*!
 *\brief called when the unsubscribe button is pressed
 * unsubscribes from the topic represented by the current item of twSubscription in every client from m_mqttClients
 */
void LiveDataDock::removeSubscription() {
	QTreeWidgetItem* unsubscribeItem = ui.twSubscriptions->currentItem();

	if(unsubscribeItem != nullptr) {
		//if it is a top level item, meaning a topic that we really subscribed to(not one that belongs to a subscription)
		//we can simply unsubscribe from it
		if(unsubscribeItem->parent() == nullptr) {
			for (auto* source: m_mqttClients) {
				source->removeMQTTSubscription(unsubscribeItem->text(0));
			}
			ui.twSubscriptions->takeTopLevelItem(ui.twSubscriptions->indexOfTopLevelItem(unsubscribeItem));
		}
		//otherwise we remove the selected item, but subscribe to every other topic, that was contained by
		//the selected item's parent subscription(top level item of twSubscripitons)
		else{
			while(unsubscribeItem->parent() != nullptr) {
				for(int i = 0; i < unsubscribeItem->parent()->childCount(); ++i) {
					if(unsubscribeItem->text(0) != unsubscribeItem->parent()->child(i)->text(0)) {
						//add topic as subscription to every client
						for (auto* source: m_mqttClients) {
							source->addBeforeRemoveSubscription(unsubscribeItem->parent()->child(i)->text(0), ui.cbQoS->currentIndex());
						}
						ui.twSubscriptions->addTopLevelItem(unsubscribeItem->parent()->takeChild(i));
						i--;
					}
				}
				unsubscribeItem = unsubscribeItem->parent();
			}

			//remove topic/subscription from every client
			for (auto* source: m_mqttClients) {
				source->removeMQTTSubscription(unsubscribeItem->text(0));
			}
			ui.twSubscriptions->takeTopLevelItem(ui.twSubscriptions->indexOfTopLevelItem(unsubscribeItem));

			//check if any common topics were subscribed, if possible merge them
			manageCommonLevelSubscriptions();
		}

		updateSubscriptionCompleter();
	} else
		QMessageBox::warning(this, "Warning", "You didn't select any item from the Tree Widget");
}

/*!
 *\brief called when a new topic is added to the tree(twTopics)
 * appends the topic's root to the topicList if it isn't in the list already
 * then sets the completer for leTopics
 */
void LiveDataDock::setTopicCompleter(const QString& topic) {
	if(!m_searching) {
		if(!m_topicList[m_mqttClients.first()->clientHostName()].contains(topic)) {
			m_topicList[m_mqttClients.first()->clientHostName()].append(topic);
			m_topicCompleter = new QCompleter(m_topicList[m_mqttClients.first()->clientHostName()], this);
			m_topicCompleter->setCompletionMode(QCompleter::PopupCompletion);
			m_topicCompleter->setCaseSensitivity(Qt::CaseSensitive);
			ui.leTopics->setCompleter(m_topicCompleter);
		}
	}
}

/*!
 *\brief Updates the completer for leSubscriptions
 */
void LiveDataDock::updateSubscriptionCompleter() {
	QStringList subscriptionList;
	QVector<QString> subscriptions = m_mqttClients.first()->mqttSubscriptions();

	if(!subscriptions.isEmpty()) {
		for(int i = 0; i < subscriptions.size(); ++i) {
			subscriptionList.append(subscriptions[i]);
		}

		m_subscriptionCompleter = new QCompleter(subscriptionList, this);
		m_subscriptionCompleter->setCompletionMode(QCompleter::PopupCompletion);
		m_subscriptionCompleter->setCaseSensitivity(Qt::CaseSensitive);
		ui.leSubscriptions->setCompleter(m_subscriptionCompleter);
	} else {
		ui.leSubscriptions->setCompleter(0);
	}
}

/*!
 *\brief called when 10 seconds passed since the last time the user searched for a certain root in twTopics
 * enables updating the completer for le
 */
void LiveDataDock::topicTimeout() {
	qDebug()<<"lejart ido";
	m_searching = false;
	m_searchTimer->stop();
}

/*!
 *\brief called when a new the host name of the MQTTClients from m _mqttClients changes
 * or when the MQTTClients initialize their subscriptions
 * Fills twSubscriptions with the subscriptions of the MQTTClient
 */
void LiveDataDock::fillSubscriptions() {
	const MQTTClient* const fds = m_mqttClients.at(0);

	ui.twSubscriptions->clear();

	QVector<QString> subscriptions = fds->mqttSubscriptions();
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
			//Add the subscription to the tree widget
			QTreeWidgetItem* newItem = new QTreeWidgetItem(name);
			ui.twSubscriptions->addTopLevelItem(newItem);
			name.clear();
			name = subscriptions[i].split('/', QString::SkipEmptyParts);

			//find the corresponding "root" item in twTopics
			QTreeWidgetItem* topic = nullptr;
			for(int j = 0; j < ui.twTopics->topLevelItemCount(); ++j) {
				if(ui.twTopics->topLevelItem(j)->text(0) == name[0]) {
					qDebug()<<"found top level topic: "<<name[0]<<" "<<j;
					topic = ui.twTopics->topLevelItem(j);
					break;
				}
			}

			//restore the children of the subscription
			if(topic != nullptr && topic->childCount() > 0) {
				qDebug()<<"restoring Children";

				restoreSubscriptionChildren(topic, newItem, name, 1);
			}
		}

	}
	m_searching = false;
}

/*!
 *\brief Checks if a topic contains another one
 *
 * \param superior the name of a topic
 * \param inferior the name of a topic
 * \return	true if superior is equal to or contains(if superior contains wildcards) inferior,
 *			false otherwise
 */
bool LiveDataDock::checkTopicContains(const QString &superior, const QString &inferior) {
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
 *\brief called when leTopics' text is changed
 *		 if the rootName can be found in twTopics, then we scroll it to the top of the tree widget
 *
 * \param rootName the current text of leTopics
 */
void LiveDataDock::scrollToTopicTreeItem(const QString& rootName) {
	m_searching = true;
	m_searchTimer->start();

	qDebug()<<rootName;
	int topItemIdx = -1;
	for(int i = 0; i< ui.twTopics->topLevelItemCount(); ++i)
		if(ui.twTopics->topLevelItem(i)->text(0) == rootName) {
			topItemIdx = i;
			break;
		}

	if(topItemIdx >= 0) {
		ui.twTopics->scrollToItem(ui.twTopics->topLevelItem(topItemIdx), QAbstractItemView::ScrollHint::PositionAtTop);
	}
}

/*!
 *\brief called when leSubscriptions' text is changed
 *		 if the rootName can be found in twSubscriptions, then we scroll it to the top of the tree widget
 *
 * \param rootName the current text of leSubscriptions
 */
void LiveDataDock::scrollToSubsriptionTreeItem(const QString& rootName) {
	m_searching = true;
	m_searchTimer->start();

	int topItemIdx = -1;
	for(int i = 0; i < ui.twSubscriptions->topLevelItemCount(); ++i)
		if(ui.twSubscriptions->topLevelItem(i)->text(0) == rootName) {
			topItemIdx = i;
			break;
		}

	if(topItemIdx >= 0) {
		ui.twSubscriptions->scrollToItem(ui.twSubscriptions->topLevelItem(topItemIdx), QAbstractItemView::ScrollHint::PositionAtTop);
	}
}

/*!
 *\brief Returns the "+" wildcard containing topic name, which includes the given topic names
 *
 * \param first the name of a topic
 * \param second the name of a topic
 * \return The name of the common topic, if it exists, otherwise ""
 */
QString LiveDataDock::checkCommonLevel(const QString& first, const QString& second) {
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
 *\brief Adds to a # wildcard containing topic, every topic present in twTopics that the former topic contains
 *
 * \param topic pointer to the TreeWidgetItem which was selected before subscribing
 * \param subscription pointer to the TreeWidgetItem which represents the new subscirption,
 *		  we add all of the children to this item
 */
void LiveDataDock::addSubscriptionChildren(QTreeWidgetItem * topic, QTreeWidgetItem * subscription) {
	//if the topic doesn't have any children we don't do anything
	if(topic->childCount() > 0) {
		for(int i = 0; i < topic->childCount(); ++i) {
			QTreeWidgetItem* temp = topic->child(i);
			QString name;
			//if it has children, then we add it as a # wildcrad containing topic
			if(topic->child(i)->childCount() > 0) {
				name.append(temp->text(0) + "/#");
				while(temp->parent() != nullptr) {
					temp = temp->parent();
					name.prepend(temp->text(0) + "/");
				}

			}
			//if not then we simply add the topic itself
			else {
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
			//we use the function recursively on the given item
			addSubscriptionChildren(topic->child(i), childItem);
		}
	}
}

/*!
 *\brief Restores the children of a top level item in twSubscriptions if it contains wildcards
 *
 * \param topic pointer to a top level item in twTopics wich represents the root of the subscription topic
 * \param subscription pointer to a top level item in twSubscriptions, this is the item whose children will be restored
 * \param list QStringList containing the levels of the subscription topic
 * \param level the level's number which is being investigated
 */
void LiveDataDock::restoreSubscriptionChildren(QTreeWidgetItem * topic, QTreeWidgetItem * subscription, const QStringList& list, int level) {
	if(list[level] != "+" && list[level] != "#" && level < list.size() - 1) {
		for(int i = 0; i < topic->childCount(); ++i) {
			//if the current level isn't + or # wildcard we recursively continue with the next level
			if(topic->child(i)->text(0) == list[level]) {
				restoreSubscriptionChildren(topic->child(i), subscription, list, level + 1);
				break;
			}
		}
	} else if (list[level] == "+") {
		for(int i = 0; i < topic->childCount(); ++i) {
			//determine the name of the topic, contained by the subscription
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

			//Add the topic as child of the subscription
			QStringList nameList;
			nameList.append(name);
			QTreeWidgetItem* newItem = new QTreeWidgetItem(nameList);
			subscription->addChild(newItem);
			//Continue adding children recursively to the new item
			restoreSubscriptionChildren(topic->child(i), newItem, list, level + 1);
		}
	} else if (list[level] == "#") {
		//add the children of the # wildcard containing subscription
		addSubscriptionChildren(topic, subscription);
	}
}

/*!
 *\brief Returns the index of level where the two topic names differ, if there is a common topic for them
 *
 * \param first the name of a topic
 * \param second the name of a topic
 * \return The index of the unequal level, if there is a common topic, otherwise -1
 */
int LiveDataDock::commonLevelIndex(const QString& first, const QString& second) {
	qDebug()<<first<<"  "<<second;
	QStringList firstList = first.split('/', QString::SkipEmptyParts);
	QStringList secondtList = second.split('/', QString::SkipEmptyParts);
	QString commonTopic = "";
	int differIndex = -1;

	if(!firstList.isEmpty()) {
		//the two topics have to be the same size and can't be identic
		if(firstList.size() == secondtList.size() && (first != second))	{

			//the index where they differ
			for(int i = 0; i < firstList.size(); ++i) {
				if(firstList.at(i) != secondtList.at(i)) {
					differIndex = i;
					break;
				}
			}

			//they can differ at only one level
			bool differ = false;
			if(differIndex > 0) {
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
					if(i != differIndex)
						commonTopic.append(firstList.at(i));
					else
						commonTopic.append("+");

					if(i != firstList.size() - 1)
						commonTopic.append("/");
				}
			}
		}
	}

	//if there is a common topic we return the differIndex
	if(!commonTopic.isEmpty())
		return differIndex;
	else
		return -1;
}

/*!
 *\brief Fills the children vector, with the root item's (twSubscriptions) leaf children (meaning no wildcard containing topics)
 *
 * \param children vector of TreeWidgetItem pointers
 * \param root pointer to a TreeWidgetItem of twSubscriptions
 */
void LiveDataDock::findSubscriptionLeafChildren(QVector<QTreeWidgetItem *>& children, QTreeWidgetItem* root) {
	if(root->childCount() == 0) {
		children.push_back(root);
	} else {
		for(int i = 0; i < root->childCount(); ++i) {
			findSubscriptionLeafChildren(children, root->child(i));
		}
	}
}

/*!
 *\brief Returns the amount of topics that the "+" wildcard will replace in the level position
 *
 * \param levelIdx the level currently being investigated
 * \param level the level where the new + wildcard will be placed
 * \param commonList the topic name split into levels
 * \param currentItem pointer to a TreeWidgetItem which represents the parent of the level
 *		  represented by levelIdx
 * \return returns the childCount, or -1 if some topics already represented by + wildcard have different
 *		   amount of children
 */
int LiveDataDock::checkCommonChildCount(int levelIdx, int level, QStringList& commonList, QTreeWidgetItem* currentItem) {
	//we recursively check the number of children, until we get to level-1
	if(levelIdx < level - 1) {
		if(commonList[levelIdx] != "+") {
			for(int j = 0; j < currentItem->childCount(); ++j) {
				if(currentItem->child(j)->text(0) == commonList[levelIdx]) {
					//if the level isn't represented by + wildcard we simply return the amount of children of the corresponding item, recursively
					return checkCommonChildCount(levelIdx + 1, level, commonList, currentItem->child(j));
				}
			}
		} else {
			int childCount = -1;
			bool ok = true;

			//otherwise we check if every + wildcard represented topic has the same number of children, recursively
			for(int j = 0; j < currentItem->childCount(); ++j) {
				int temp = checkCommonChildCount(levelIdx + 1, level, commonList, currentItem->child(j));

				if((j > 0) && (temp != childCount)) {
					ok = false;
					break;
				}
				childCount = temp;
			}

			//if yes we return this number, otherwise -1
			if(ok)
				return childCount;
			else
				return -1;
		}
	} else if (levelIdx == level - 1) {
		if(commonList[levelIdx] != "+") {
			for(int j = 0; j < currentItem->childCount(); ++j) {
				if(currentItem->child(j)->text(0) == commonList[levelIdx]) {
					//if the level isn't represented by + wildcard we simply return the amount of children of the corresponding item
					return currentItem->child(j)->childCount();
				}
			}
		} else {
			int childCount = -1;
			bool ok = true;

			//otherwise we check if every + wildcard represented topic has the same number of children
			for(int j = 0; j < currentItem->childCount(); ++j) {
				if((j > 0) && (currentItem->child(j)->childCount() != childCount)) {
					ok = false;
					break;
				}
				childCount = currentItem->child(j)->childCount();
			}

			//if yes we return this number, otherwise -1
			if(ok)
				return childCount;
			else
				return -1;
		}
	} else if (level == 1 && levelIdx == 1)
		return currentItem->childCount();

	return -1;
}

/*!
 *\brief We search in twSubscriptions for topics that can be represented using + wildcards, then merge them.
 *		 We do this until there are no topics to merge
 */
void LiveDataDock::manageCommonLevelSubscriptions() {
	bool foundEqual = false;
	do{
		foundEqual = false;
		QMap<QString, QVector<QString>> equalTopicsMap;
		QVector<QString> equalTopics;

		//compare the subscriptions present in the TreeWidget
		for(int i = 0; i < ui.twSubscriptions->topLevelItemCount() - 1; ++i) {
			for(int j = i + 1; j < ui.twSubscriptions->topLevelItemCount(); ++j) {
				qDebug()<<ui.twSubscriptions->topLevelItem(i)->text(0)<<"  "<<ui.twSubscriptions->topLevelItem(j)->text(0);
				QString commonTopic = checkCommonLevel(ui.twSubscriptions->topLevelItem(i)->text(0), ui.twSubscriptions->topLevelItem(j)->text(0));

				//if there is a common topic for the 2 compared topics, we add them to the map (using the common topic as key)
				if(!commonTopic.isEmpty()) {
					if(!equalTopicsMap[commonTopic].contains(ui.twSubscriptions->topLevelItem(i)->text(0))) {
						qDebug()<<commonTopic<<":  "<<ui.twSubscriptions->topLevelItem(i)->text(0);
						equalTopicsMap[commonTopic].push_back(ui.twSubscriptions->topLevelItem(i)->text(0));
					}

					if(!equalTopicsMap[commonTopic].contains(ui.twSubscriptions->topLevelItem(j)->text(0))) {
						qDebug()<<commonTopic<<":  "<<ui.twSubscriptions->topLevelItem(i)->text(0);
						equalTopicsMap[commonTopic].push_back(ui.twSubscriptions->topLevelItem(j)->text(0));
					}
				}
			}
		}

		if(!equalTopicsMap.isEmpty()) {
			qDebug()<<"Equal topics not empty";

			QVector<QString> commonTopics;
			QMapIterator<QString, QVector<QString>> topics(equalTopicsMap);

			//check for every map entry, if the found topics can be merged or not
			while(topics.hasNext()) {
				topics.next();
				qDebug()<<"Checking: " << topics.key();

				int level = commonLevelIndex(topics.value().last(), topics.value().first());
				QStringList commonList = topics.value().first().split('/', QString::SkipEmptyParts);
				QTreeWidgetItem* currentItem;

				//search the corresponding item to the common topics first level(root)
				for(int i = 0; i < ui.twTopics->topLevelItemCount(); ++i) {
					if(ui.twTopics->topLevelItem(i)->text(0) == commonList.first()) {
						currentItem = ui.twTopics->topLevelItem(i);
						break;
					}
				}

				//calculate the number of topics the new + wildcard could replace
				int childCount = checkCommonChildCount(1, level, commonList, currentItem);
				if(childCount > 0) {
					//if the number of topics found and the calculated number of topics is equal, the topics can be merged
					if(topics.value().size() == childCount) {
						foundEqual = true;
						commonTopics.push_back(topics.key());
						qDebug()<<topics.key()<<" equal is true";
					}
				}
			}

			if(foundEqual) {
				//if there are more common topics, the topics of which can be merged, we choose the one which has the lowest level new "+" wildcard
				int highestLevel = INT_MAX;
				int topicIdx = -1;
				for(int i = 0; i < commonTopics.size(); ++i) {
					int level = commonLevelIndex(equalTopicsMap[commonTopics[i]].first(), commonTopics[i]);
					if(level < highestLevel) {
						topicIdx = i;
						highestLevel = level;
					}
				}
				equalTopics.append(equalTopicsMap[commonTopics[topicIdx]]);

				qDebug()<<"Adding common topic";
				//Add the common topic ("merging")
				QString commonTopic;
				commonTopic = checkCommonLevel(equalTopics.first(), equalTopics.last());
				QStringList nameList;
				nameList.append(commonTopic);
				QTreeWidgetItem* newTopic = new QTreeWidgetItem(nameList);
				ui.twSubscriptions->addTopLevelItem(newTopic);

				//remove the "merged" topics
				for(int i = 0; i < equalTopics.size(); ++i) {
					for(int j = 0; j < ui.twSubscriptions->topLevelItemCount(); ++j){
						if(ui.twSubscriptions->topLevelItem(j)->text(0) == equalTopics[i]) {
							newTopic->addChild(ui.twSubscriptions->takeTopLevelItem(j));
							break;
						}
					}
				}

				//remove any subscription that the new subscription contains
				for(int i = 0; i < ui.twSubscriptions->topLevelItemCount(); ++i) {
					if(checkTopicContains(commonTopic, ui.twSubscriptions->topLevelItem(i)->text(0)) &&
							commonTopic != ui.twSubscriptions->topLevelItem(i)->text(0) ) {
						ui.twSubscriptions->topLevelItem(i)->takeChildren();
						ui.twSubscriptions->takeTopLevelItem(i);
						i--;
					}
				}

				//make the subscripiton on commonTopic in every MQTTClient from m_mqttClients
				for (auto* source: m_mqttClients) {
					source->addMQTTSubscription(commonTopic, ui.cbQoS->currentIndex());
				}
			}
		}
	} while(foundEqual);
}

/*!
 *\brief Adds topicName to twTopics
 *
 * \param topicName the name of the topic, which will be added to the tree widget
 */
void LiveDataDock::addTopicToTree(const QString &topicName) {
	QStringList name;
	QChar sep = '/';
	QString rootName;
	if(topicName.contains(sep)) {
		QStringList list = topicName.split(sep, QString::SkipEmptyParts);

		if(!list.isEmpty()) {
			rootName = list.at(0);
			name.append(list.at(0));
			QTreeWidgetItem* currentItem;
			//check whether the first level of the topic can be found in twTopics
			int topItemIdx = -1;
			for(int i = 0; i < ui.twTopics->topLevelItemCount(); ++i) {
				if(ui.twTopics->topLevelItem(i)->text(0) == list.at(0)) {
					topItemIdx = i;
					break;
				}
			}
			//if not we simply add every level of the topic to the tree
			if( topItemIdx < 0) {
				currentItem = new QTreeWidgetItem(name);
				ui.twTopics->addTopLevelItem(currentItem);
				for(int i = 1; i < list.size(); ++i) {
					name.clear();
					name.append(list.at(i));
					currentItem->addChild(new QTreeWidgetItem(name));
					currentItem = currentItem->child(0);
				}
			}
			//otherwise we search for the first level that isn't part of the tree,
			//then add every level of the topic to the tree from that certain level
			else {
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
					if(!found) {
						//this is the level that isn't present in the tree
						break;
					}
				}

				//add every level to the tree starting with the first level that isn't part of the tree
				for(; listIdx < list.size(); ++listIdx) {
					name.clear();
					name.append(list.at(listIdx));
					currentItem->addChild(new QTreeWidgetItem(name));
					currentItem = currentItem->child(currentItem->childCount() - 1);
				}
			}
		}
	}
	else {
		rootName = topicName;
		name.append(topicName);
		ui.twTopics->addTopLevelItem(new QTreeWidgetItem(name));
	}

	//if a subscribed topic contains the new topic, we have to update twSubscriptions
	for(int i = 0; i < ui.twSubscriptions->topLevelItemCount(); ++i) {
		QStringList subscriptionName = ui.twSubscriptions->topLevelItem(i)->text(0).split('/', QString::SkipEmptyParts);
		if (rootName == subscriptionName[0]) {
			qDebug()<<topicName;
			fillSubscriptions();
			break;
		}
	}

	//signals that a newTopic was added, in order to fill the completer of leTopics
	emit newTopic(rootName);
}

/*!
 *\brief called when a client receives a message, if the clients hostname isn't identic with the host name of MQTTClients
 * if the message arrived from a new topic, the topic is put in m_addedTopics
 */
void LiveDataDock::mqttMessageReceivedInBackground(const QByteArray& message, const QMqttTopicName& topic) {
	if(!m_addedTopics[m_mqttClients.first()->clientHostName()].contains(topic.name())) {
		m_addedTopics[m_mqttClients.first()->clientHostName()].push_back(topic.name());
	}
}

/*!
 *\brief called when an MQTTClient is about to be deleted
 * removes every data connected to the MQTTClient, and disconnects the corresponding client from m_clients
 *
 * \param name the host name of the MQTTClient that will be deleted
 */
void LiveDataDock::removeClient(const QString& name) {
	m_clients[name]->disconnectFromHost();

	m_addedTopics.remove(name);
	m_topicList.remove(name);

	if(m_previousMQTTClient != nullptr && m_previousMQTTClient->clientHostName() == name) {
		disconnect(m_clients[m_previousMQTTClient->clientHostName()], &QMqttClient::messageReceived, this, &LiveDataDock::mqttMessageReceivedInBackground);
		m_previousMQTTClient = nullptr;
	}

	if(m_mqttClients.first()->clientHostName() == name) {
		ui.twSubscriptions->clear();
		ui.twTopics->clear();
		m_mqttClients.clear();
	}

	delete m_clients[name];
	m_clients.remove(name);
}
#endif
