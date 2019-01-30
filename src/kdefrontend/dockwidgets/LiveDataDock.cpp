/***************************************************************************
File                 : LiveDataDock.cpp
Project              : LabPlot
Description          : Dock widget for live data properties
--------------------------------------------------------------------
Copyright            : (C) 2017 by Fabian Kristof (fkristofszabolcs@gmail.com)
Copyright            : (C) 2018 by Kovacs Ferencz (kferike98@gmail.com)
Copyright            : (C) 2018 by Stefan Gerlach (stefan.gerlach@uni.kn)
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
#include "kdefrontend/widgets/MQTTWillSettingsWidget.h"
#include "kdefrontend/datasources/MQTTHelpers.h"
#include <QMessageBox>
#include <QWidgetAction>
#include <QMenu>
#endif

LiveDataDock::LiveDataDock(QWidget* parent) : QWidget(parent)
#ifdef HAVE_MQTT
	,
	m_searchTimer(new QTimer(this))
#endif
{
	ui.setupUi(this);

	ui.bUpdateNow->setIcon(QIcon::fromTheme(QLatin1String("view-refresh")));

	connect(ui.leName, &QLineEdit::textChanged, this, &LiveDataDock::nameChanged);
	connect(ui.bPausePlayReading, &QPushButton::clicked, this, &LiveDataDock::pauseContinueReading);
	connect(ui.bUpdateNow, &QPushButton::clicked, this, &LiveDataDock::updateNow);
	connect(ui.sbUpdateInterval, static_cast<void (QSpinBox::*) (int)>(&QSpinBox::valueChanged), this, &LiveDataDock::updateIntervalChanged);

	connect(ui.sbKeepNValues, static_cast<void (QSpinBox::*) (int)>(&QSpinBox::valueChanged), this, &LiveDataDock::keepNValuesChanged);
	connect(ui.sbSampleSize, static_cast<void (QSpinBox::*) (int)>(&QSpinBox::valueChanged), this, &LiveDataDock::sampleSizeChanged);
	connect(ui.cbUpdateType, static_cast<void (QComboBox::*) (int)>(&QComboBox::currentIndexChanged), this, &LiveDataDock::updateTypeChanged);
	connect(ui.cbReadingType, static_cast<void (QComboBox::*) (int)>(&QComboBox::currentIndexChanged), this, &LiveDataDock::readingTypeChanged);

#ifdef HAVE_MQTT
	m_searchTimer->setInterval(10000);

	const int size = ui.leTopics->height();
	ui.lTopicSearch->setPixmap( QIcon::fromTheme(QLatin1String("go-next")).pixmap(size, size) );
	ui.lSubscriptionSearch->setPixmap( QIcon::fromTheme(QLatin1String("go-next")).pixmap(size, size) );

	connect(this, &LiveDataDock::newTopic, this, &LiveDataDock::setTopicCompleter);
	connect(m_searchTimer, &QTimer::timeout, this, &LiveDataDock::topicTimeout);
	connect(ui.bSubscribe, &QPushButton::clicked, this, &LiveDataDock::addSubscription);
	connect(ui.bUnsubscribe, &QPushButton::clicked, this, &LiveDataDock::removeSubscription);
	connect(ui.bWillUpdateNow, &QPushButton::clicked, this, &LiveDataDock::willUpdateNow);
	connect(ui.leTopics, &QLineEdit::textChanged, this, &LiveDataDock::scrollToTopicTreeItem);
	connect(ui.leSubscriptions, &QLineEdit::textChanged, this, &LiveDataDock::scrollToSubsriptionTreeItem);
	connect(ui.bLWT, &QPushButton::clicked, this, &LiveDataDock::showWillSettings);

	ui.bSubscribe->setIcon(ui.bSubscribe->style()->standardIcon(QStyle::SP_ArrowRight));
	ui.bSubscribe->setToolTip(i18n("Subscribe selected topics"));
	ui.bUnsubscribe->setIcon(ui.bUnsubscribe->style()->standardIcon(QStyle::SP_ArrowLeft));
	ui.bUnsubscribe->setToolTip(i18n("Unsubscribe selected topics"));
	ui.bLWT->setToolTip(i18n("Manage MQTT connection's will settings"));
	ui.bLWT->setIcon(ui.bLWT->style()->standardIcon(QStyle::SP_FileDialogDetailedView));

	QString info = i18n("Enter the name of the topic to navigate to it.");
	ui.lTopicSearch->setToolTip(info);
	ui.leTopics->setToolTip(info);
	ui.lSubscriptionSearch->setToolTip(info);
	ui.leSubscriptions->setToolTip(info);

	info = i18n("Specify the 'Last Will and Testament' message (LWT). At least one topic has to be subscribed.");
	ui.lLWT->setToolTip(info);
	ui.bLWT->setToolTip(info);
	ui.bLWT->setEnabled(false);
	ui.bLWT->setIcon(ui.bLWT->style()->standardIcon(QStyle::SP_FileDialogDetailedView));

	info = i18n("Set the Quality of Service (QoS) for the subscription to define the guarantee of the message delivery:"
	"<ul>"
	"<li>0 - deliver at most once</li>"
	"<li>1 - deliver at least once</li>"
	"<li>2 - deliver exactly once</li>"
	"</ul>");
	ui.cbQoS->setToolTip(info);
#endif
}

#ifdef HAVE_MQTT
LiveDataDock::~LiveDataDock() {
	delete m_searchTimer;
	for (auto & host : m_hosts)
		delete host.client;
}
#else
LiveDataDock::~LiveDataDock() = default;
#endif

#ifdef HAVE_MQTT
/*!
 * \brief Sets the MQTTClient of this dock widget
 * \param clients
 */
void LiveDataDock::setMQTTClient(MQTTClient* const client) {
	m_liveDataSource = nullptr; // prevent updates due to changes to input widgets
	if (m_mqttClient == client)
		return;
	auto oldclient = m_mqttClient;
	m_mqttClient = nullptr; // prevent updates due to changes to input widgets

	ui.leName->setText(client->name());
	const QPair<QString, quint16> id(client->clientHostName(), client->clientPort());
	ui.leSourceInfo->setText(QStringLiteral("%1:%2").arg(id.first).arg(id.second));

	ui.sbUpdateInterval->setValue(client->updateInterval());
	ui.cbUpdateType->setCurrentIndex(static_cast<int>(client->updateType()));
	ui.cbReadingType->setCurrentIndex(static_cast<int>(client->readingType()));

	if (client->updateType() == MQTTClient::NewData) {
		ui.lUpdateInterval->hide();
		ui.sbUpdateInterval->hide();
	}

	if (client->isPaused()) {
		ui.bPausePlayReading->setText(i18n("Continue reading"));
		ui.bPausePlayReading->setIcon(QIcon::fromTheme(QLatin1String("media-record")));
	} else {
		ui.bPausePlayReading->setText(i18n("Pause reading"));
		ui.bPausePlayReading->setIcon(QIcon::fromTheme(QLatin1String("media-playback-pause")));
	}

	ui.sbKeepNValues->setValue(client->keepNValues());
	ui.sbKeepNValues->setEnabled(true);

	if (client->readingType() == MQTTClient::TillEnd) {
		ui.lSampleSize->hide();
		ui.sbSampleSize->hide();
	} else
		ui.sbSampleSize->setValue(client->sampleSize());

	// disable "whole file" option
	const QStandardItemModel* model = qobject_cast<const QStandardItemModel*>(ui.cbReadingType->model());
	QStandardItem* item = model->item(LiveDataSource::WholeFile);
	item->setFlags(item->flags() & ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled));
	if (static_cast<LiveDataSource::ReadingType>(ui.cbReadingType->currentIndex()) == LiveDataSource::WholeFile)
		ui.cbReadingType->setCurrentIndex(LiveDataSource::TillEnd);

	m_mqttClient = client; // updates may be applied from now on

	//show MQTT connected options
	ui.lTopics->show();
	ui.gbManageSubscriptions->show();
	ui.bSubscribe->show();
	ui.bUnsubscribe->show();
	ui.twTopics->show();
	ui.leTopics->show();
	ui.lTopicSearch->show();
	ui.twSubscriptions->show();
	ui.cbQoS->show();
	ui.lLWT->show();
	ui.bLWT->show();

	m_previousHost = m_currentHost;
	//if there isn't a client with this hostname we instantiate a new one
	auto it = m_hosts.find(id);
	if (it == m_hosts.end()) {
		m_currentHost = &m_hosts[id];
		m_currentHost->count = 1;
		m_currentHost->client = new QMqttClient;

		connect(client, &MQTTClient::clientAboutToBeDeleted, this, &LiveDataDock::removeClient);

		connect(m_currentHost->client, &QMqttClient::connected, this, &LiveDataDock::onMQTTConnect);
		connect(m_currentHost->client, &QMqttClient::messageReceived, this, &LiveDataDock::mqttMessageReceived);

		m_currentHost->client->setHostname(id.first);
		m_currentHost->client->setPort(id.second);

		if (client->MQTTUseAuthentication()) {
			m_currentHost->client->setUsername(client->clientUserName());
			m_currentHost->client->setPassword(client->clientPassword());
		}

		if (client->MQTTUseID())
			m_currentHost->client->setClientId(client->clientID());

		m_currentHost->client->connectToHost();
	} else {
		m_currentHost = &it.value();
		++m_currentHost->count;
	}

	if (m_previousMQTTClient == nullptr) {
		connect(client, &MQTTClient::MQTTSubscribed, this, &LiveDataDock::fillSubscriptions);

		//Fill the subscription tree(useful if the MQTTClient was loaded)
		QVector<QString> topics = client->topicNames();
		for (const auto& topic : topics) {
			addTopicToTree(topic);
		}
		fillSubscriptions();
	}

	//if the previous MQTTClient's host name was different from the current one we have to disconnect some slots
	//and clear the tree widgets
	else if (m_previousMQTTClient->clientHostName() != client->clientHostName()) {
		disconnect(m_previousMQTTClient, &MQTTClient::MQTTSubscribed, this, &LiveDataDock::fillSubscriptions);
		disconnect(m_previousHost->client, &QMqttClient::messageReceived, this, &LiveDataDock::mqttMessageReceived);
		connect(m_previousHost->client, &QMqttClient::messageReceived, this, &LiveDataDock::mqttMessageReceivedInBackground);

		disconnect(m_currentHost->client, &QMqttClient::messageReceived, this, &LiveDataDock::mqttMessageReceivedInBackground);

		ui.twTopics->clear();
		//repopulating the tree widget with the already known topics of the client
		for (int i = 0; i < m_currentHost->addedTopics.size(); ++i) {
			addTopicToTree(m_currentHost->addedTopics.at(i));
		}

		//fill subscriptions tree widget
		ui.twSubscriptions->clear();
		fillSubscriptions();

		connect(client, &MQTTClient::MQTTSubscribed, this, &LiveDataDock::fillSubscriptions);
		connect(m_currentHost->client, &QMqttClient::messageReceived, this, &LiveDataDock::mqttMessageReceived);
	}

	if (client->willUpdateType() == MQTTClient::OnClick && client->MQTTWillUse())
		ui.bWillUpdateNow->show();

	m_previousMQTTClient = oldclient;
}
#endif

/*!
 * \brief Sets the live data source of this dock widget
 * \param sources
 */
void LiveDataDock::setLiveDataSource(LiveDataSource* const source) {
#ifdef HAVE_MQTT
	m_mqttClient = nullptr;
#endif
	if (m_liveDataSource == source)
		return;
	m_liveDataSource = nullptr; // prevent updates due to changes to input widgets

	ui.leName->setText(source->name());
	const LiveDataSource::SourceType sourceType = source->sourceType();
	const LiveDataSource::ReadingType readingType = source->readingType();
	const LiveDataSource::UpdateType updateType = source->updateType();
	const AbstractFileFilter::FileType fileType = source->fileType();
	ui.sbUpdateInterval->setValue(source->updateInterval());
	ui.cbUpdateType->setCurrentIndex(static_cast<int>(updateType));
	ui.cbReadingType->setCurrentIndex(static_cast<int>(readingType));

	switch (sourceType) {
	case LiveDataSource::FileOrPipe:
		ui.leSourceInfo->setText(source->fileName());
		break;
	case LiveDataSource::NetworkTcpSocket:
	case LiveDataSource::NetworkUdpSocket:
		ui.leSourceInfo->setText(QStringLiteral("%1:%2").arg(source->host()).arg(source->port()));
		break;
	case LiveDataSource::LocalSocket:
		ui.leSourceInfo->setText(source->localSocketName());
		break;
	case LiveDataSource::SerialPort:
		ui.leSourceInfo->setText(source->serialPortName());
		break;
	case LiveDataSource::MQTT:
		break;
	}

	if (updateType == LiveDataSource::UpdateType::NewData) {
		ui.lUpdateInterval->hide();
		ui.sbUpdateInterval->hide();
	}

	if (source->isPaused()) {
		ui.bPausePlayReading->setText(i18n("Continue Reading"));
		ui.bPausePlayReading->setIcon(QIcon::fromTheme(QLatin1String("media-record")));
	} else {
		ui.bPausePlayReading->setText(i18n("Pause Reading"));
		ui.bPausePlayReading->setIcon(QIcon::fromTheme(QLatin1String("media-playback-pause")));
	}

	ui.sbKeepNValues->setValue(source->keepNValues());

	// disable "whole file" when having no file (i.e. socket or port)
	auto* model = qobject_cast<const QStandardItemModel*>(ui.cbReadingType->model());
	QStandardItem* item = model->item(LiveDataSource::WholeFile);
	if (sourceType == LiveDataSource::SourceType::FileOrPipe) {
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		//for file types other than ASCII and binary we support re-reading the whole file only
		//select "read whole file" and deactivate the combobox
		if (fileType != AbstractFileFilter::Ascii && fileType != AbstractFileFilter::Binary) {
			ui.cbReadingType->setCurrentIndex(LiveDataSource::WholeFile);
			ui.cbReadingType->setEnabled(false);
		} else
			ui.cbReadingType->setEnabled(true);
	} else {
		if (static_cast<LiveDataSource::ReadingType>(ui.cbReadingType->currentIndex()) == LiveDataSource::WholeFile)
			ui.cbReadingType->setCurrentIndex(LiveDataSource::TillEnd);
		item->setFlags(item->flags() & ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled));
	}

	if (((sourceType == LiveDataSource::FileOrPipe || sourceType == LiveDataSource::NetworkUdpSocket) &&
	     (readingType == LiveDataSource::ContinuousFixed || readingType == LiveDataSource::FromEnd)))
		ui.sbSampleSize->setValue(source->sampleSize());
	else {
		ui.lSampleSize->hide();
		ui.sbSampleSize->hide();
	}

	// disable "on new data"-option if not available
	model = qobject_cast<const QStandardItemModel*>(ui.cbUpdateType->model());
	item = model->item(LiveDataSource::NewData);
	if (sourceType == LiveDataSource::NetworkTcpSocket || sourceType == LiveDataSource::NetworkUdpSocket ||
	    sourceType == LiveDataSource::SerialPort)
		item->setFlags(item->flags() & ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled));
	else
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

	ui.lTopics->hide();
	ui.bLWT->hide();
	ui.lLWT->hide();
	ui.bWillUpdateNow->hide();
	ui.bSubscribe->hide();
	ui.bUnsubscribe->hide();
	ui.twTopics->hide();
	ui.leTopics->hide();
	ui.lTopicSearch->hide();
	ui.twSubscriptions->hide();
	ui.gbManageSubscriptions->hide();

	m_liveDataSource = source; // updates may be applied from now on
}

/*!
 * \brief Modifies the sample size of the live data source or MQTTClient object
 * \param sampleSize
 */
void LiveDataDock::sampleSizeChanged(int sampleSize) {
	if (m_liveDataSource)
		m_liveDataSource->setSampleSize(sampleSize);
#ifdef HAVE_MQTT
	else if (m_mqttClient)
		m_mqttClient->setSampleSize(sampleSize);
#endif
}

/*!
 * \brief Updates the live data source now
 */
void LiveDataDock::updateNow() {
	if (m_liveDataSource)
		m_liveDataSource->updateNow();
#ifdef HAVE_MQTT
	else if (m_mqttClient)
		m_mqttClient->updateNow();
#endif
}

void LiveDataDock::nameChanged(const QString& name) {
	if (m_liveDataSource)
		m_liveDataSource->setName(name);
#ifdef HAVE_MQTT
	else if (m_mqttClient)
		m_mqttClient->setName(name);
#endif
}

/*!
 * \brief LiveDataDock::updateTypeChanged
 * \param idx
 */
void LiveDataDock::updateTypeChanged(int idx) {
	if (m_liveDataSource)  {
		DEBUG("LiveDataDock::updateTypeChanged()");
		const LiveDataSource::UpdateType updateType = static_cast<LiveDataSource::UpdateType>(idx);

		switch (updateType) {
		case LiveDataSource::TimeInterval: {
			ui.lUpdateInterval->show();
			ui.sbUpdateInterval->show();
			const LiveDataSource::SourceType s = m_liveDataSource->sourceType();
			const LiveDataSource::ReadingType r = m_liveDataSource->readingType();
			const bool showSampleSize = ((s == LiveDataSource::FileOrPipe || s == LiveDataSource::NetworkUdpSocket) &&
			                             (r == LiveDataSource::ContinuousFixed || r == LiveDataSource::FromEnd));
			ui.lSampleSize->setVisible(showSampleSize);
			ui.sbSampleSize->setVisible(showSampleSize);

			m_liveDataSource->setUpdateType(updateType);
			m_liveDataSource->setUpdateInterval(ui.sbUpdateInterval->value());
			m_liveDataSource->setFileWatched(false);
			break;
		}
		case LiveDataSource::NewData:
			ui.lUpdateInterval->hide();
			ui.sbUpdateInterval->hide();
			ui.lSampleSize->hide();
			ui.sbSampleSize->hide();

			m_liveDataSource->setFileWatched(true);
			m_liveDataSource->setUpdateType(updateType);
		}
	}
#ifdef HAVE_MQTT
	else if (m_mqttClient) {
		DEBUG("LiveDataDock::updateTypeChanged()");
		const MQTTClient::UpdateType type = static_cast<MQTTClient::UpdateType>(idx);

		if (type == MQTTClient::TimeInterval) {
			ui.lUpdateInterval->show();
			ui.sbUpdateInterval->show();

			m_mqttClient->setUpdateType(type);
			m_mqttClient->setUpdateInterval(ui.sbUpdateInterval->value());
		} else if (type == MQTTClient::NewData) {
			ui.lUpdateInterval->hide();
			ui.sbUpdateInterval->hide();

			m_mqttClient->setUpdateType(type);
		}
	}
#endif
}

/*!
 * \brief Handles the change of the reading type in the dock widget
 * \param idx
 */
void LiveDataDock::readingTypeChanged(int idx) {
	if (m_liveDataSource)  {
		const auto type = static_cast<LiveDataSource::ReadingType>(idx);
		const LiveDataSource::SourceType sourceType = m_liveDataSource->sourceType();
		const LiveDataSource::UpdateType updateType = m_liveDataSource->updateType();

		if (sourceType == LiveDataSource::NetworkTcpSocket || sourceType == LiveDataSource::LocalSocket || sourceType == LiveDataSource::SerialPort
				|| type == LiveDataSource::TillEnd || type == LiveDataSource::WholeFile
				|| updateType == LiveDataSource::NewData) {
			ui.lSampleSize->hide();
			ui.sbSampleSize->hide();
		} else {
			ui.lSampleSize->show();
			ui.sbSampleSize->show();
		}

		m_liveDataSource->setReadingType(type);
	}
#ifdef HAVE_MQTT
	else if (m_mqttClient) {
		MQTTClient::ReadingType type = static_cast<MQTTClient::ReadingType>(idx);

		if (type == MQTTClient::TillEnd) {
			ui.lSampleSize->hide();
			ui.sbSampleSize->hide();
		} else {
			ui.lSampleSize->show();
			ui.sbSampleSize->show();
		}

		m_mqttClient->setReadingType(type);
	}
#endif
}

/*!
 * \brief Modifies the update interval of the live data source
 * \param updateInterval
 */
void LiveDataDock::updateIntervalChanged(int updateInterval) {
	if (m_liveDataSource)
		m_liveDataSource->setUpdateInterval(updateInterval);
#ifdef HAVE_MQTT
	else if (m_mqttClient)
		m_mqttClient->setUpdateInterval(updateInterval);
#endif
}

/*!
 * \brief Modifies the number of samples to keep in each of the live data source
 * \param keepNValues
 */
void LiveDataDock::keepNValuesChanged(const int keepNValues) {
	if (m_liveDataSource)
		m_liveDataSource->setKeepNValues(keepNValues);
#ifdef HAVE_MQTT
	else if (m_mqttClient)
		m_mqttClient->setKeepNValues(keepNValues);
#endif
}

/*!
 * \brief Pauses the reading of the live data source
 */
void LiveDataDock::pauseReading() {
	if (m_liveDataSource)
		m_liveDataSource->pauseReading();
#ifdef HAVE_MQTT
	else if (m_mqttClient)
		m_mqttClient->pauseReading();
#endif
}

/*!
 * \brief Continues the reading of the live data source
 */
void LiveDataDock::continueReading() {
	if (m_liveDataSource)
		m_liveDataSource->continueReading();
#ifdef HAVE_MQTT
	else if (m_mqttClient)
		m_mqttClient->continueReading();
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
 *\brief called when use will message checkbox's state is changed in the will settings widget,
 * Sets the mqttUseWill according to state for the m_mqttClient
 *
 * \param state the state of the checbox
 */
void LiveDataDock::useWillMessage(bool use) {
	qDebug()<<"Use will message: " << use;

	if (use) {
		m_mqttClient->setMQTTWillUse(true);
		if (m_mqttClient->willUpdateType() == MQTTClient::OnClick)
			ui.bWillUpdateNow->show();

	} else {
		m_mqttClient->setMQTTWillUse(false);
		ui.bWillUpdateNow->hide();
	}
}

/*!
 *\brief called when will message's QoS is changed in the will settings widget
 * sets the will QoS level for the m_mqttClient
 *
 * \param QoS the QoS level of the will message
 */
void LiveDataDock::willQoSChanged(int QoS) {
	m_mqttClient->setWillQoS(QoS);
}

/*!
 *\brief called when will message's retain flag is changed in the will settings widget
 * sets the retain flag for the will message in in m_mqttClient
 *
 * \param state the state of the will retain chechbox
 */
void LiveDataDock::willRetainChanged(bool useWillRetainMessages) {
	if (useWillRetainMessages) {
		m_mqttClient->setWillRetain(true);
	} else {
		m_mqttClient->setWillRetain(false);
	}
}

/*!
 *\brief called when will topic combobox's current item is changed in the will settings widget
 * sets the will topic for the m_mqttClient
 *
 * \param topic the current text of cbWillTopic
 */
void LiveDataDock::willTopicChanged(const QString& topic) {
	if (m_mqttClient->willTopic() != topic)
		m_mqttClient->clearLastMessage();

	m_mqttClient->setWillTopic(topic);
}

/*!
 *\brief called when the selected will message type is changed in the will settings widget
 * sets the will message type for the m_mqttClient
 *
 * \param type the selected will message type
 */
void LiveDataDock::willMessageTypeChanged(MQTTClient::WillMessageType willMessageType) {
	m_mqttClient->setWillMessageType(willMessageType);
}

/*!
 *\brief called when the will own message is changed in the will settings widget
 * sets the will own message for the m_mqttClient
 *
 * \param message the will message given by the user
 */
void LiveDataDock::willOwnMessageChanged(const QString& message) {
	m_mqttClient->setWillOwnMessage(message);
}

/*!
 *\brief called when the selected update type for the will message is changed in the will settings widget
 * sets the will update type for the m_mqttClient
 *
 * \param type the selected will update type
 */
void LiveDataDock::willUpdateTypeChanged(int updateType) {
	m_mqttClient->setWillUpdateType(static_cast<MQTTClient::WillUpdateType>(updateType));

	if (static_cast<MQTTClient::WillUpdateType>(updateType) == MQTTClient::TimePeriod) {
		ui.bWillUpdateNow->hide();
		m_mqttClient->startWillTimer();
	} else if (static_cast<MQTTClient::WillUpdateType>(updateType) == MQTTClient::OnClick) {
		ui.bWillUpdateNow->show();

		//if update type is on click we stop the will timer
		m_mqttClient->stopWillTimer();
	}
}

/*!
 *\brief called when the will update now button is pressed
 * updates the will message of m_mqttClient
 */
void LiveDataDock::willUpdateNow() {
	m_mqttClient->updateWillMessage();
}

/*!
 *\brief called when the update interval for will message is changed in the will settings widget
 * sets the will update interval for the m_mqttClient, then starts the will timer for each one
 *
 * \param interval the new will update interval
 */
void LiveDataDock::willUpdateIntervalChanged(int interval) {
	m_mqttClient->setWillTimeInterval(interval);
	m_mqttClient->startWillTimer();
}

/*!
 *\brief called when the will statistics are changed in the will settings widget
 * adds or removes the statistic represented by the index from m_mqttClient
 */
void LiveDataDock::statisticsChanged(MQTTClient::WillStatisticsType willStatisticsType) {
	if (willStatisticsType >= 0) {
		//if it's not already added and it's checked we add it
		if (!m_mqttClient->willStatistics()[static_cast<int>(willStatisticsType)])
			m_mqttClient->addWillStatistics(willStatisticsType);
		else //otherwise remove it
			m_mqttClient->removeWillStatistics(willStatisticsType);
	}
}

/*!
 *\brief called when the client connects to the broker successfully, it subscribes to every topic (# wildcard)
 * in order to later list every available topic
 */
void LiveDataDock::onMQTTConnect() {
	if (!m_currentHost || !m_currentHost->client || !m_currentHost->client->subscribe(QMqttTopicFilter(QLatin1String("#")), 1))
		QMessageBox::critical(this, i18n("Couldn't subscribe"), i18n("Couldn't subscribe to all available topics. Something went wrong"));
}

/*!
 *\brief called when the client receives a message
 * if the message arrived from a new topic, the topic is put in twTopics
 */
void LiveDataDock::mqttMessageReceived(const QByteArray& message, const QMqttTopicName& topic) {
	Q_UNUSED(message)
	if (!m_currentHost->addedTopics.contains(topic.name())) {
		m_currentHost->addedTopics.push_back(topic.name());
		addTopicToTree(topic.name());
	}
}

/*!
 *\brief called when the subscribe button is pressed
 * subscribes to the topic represented by the current item of twTopics in every client from m_mqttClients
 */
void LiveDataDock::addSubscription() {
	QString name;
	QTreeWidgetItem* item = ui.twTopics->currentItem();
	if (item != nullptr) {
		QTreeWidgetItem* tempItem = item;

		//determine the topic name that the current item represents
		name.prepend(item->text(0));
		if (item->childCount() != 0)
			name.append("/#");
		while (tempItem->parent() != nullptr) {
			tempItem = tempItem->parent();
			name.prepend(tempItem->text(0) + '/');
		}

		//check if the subscription already exists
		const QList<QTreeWidgetItem *> topLevelList = ui.twSubscriptions->findItems(name, Qt::MatchExactly);
		if (topLevelList.isEmpty() || topLevelList.first()->parent() != nullptr) {
			qDebug() << "LiveDataDock: start to add new subscription: " << name;
			bool foundSuperior = false;

			for (int i = 0; i < ui.twSubscriptions->topLevelItemCount(); ++i) {
				//if the new subscriptions contains an already existing one, we remove the inferior one
				if (MQTTHelpers::checkTopicContains(name, ui.twSubscriptions->topLevelItem(i)->text(0))
						&& name != ui.twSubscriptions->topLevelItem(i)->text(0)) {
					ui.twSubscriptions->topLevelItem(i)->takeChildren();
					ui.twSubscriptions->takeTopLevelItem(i);
					--i;
					continue;
				}

				//if there is a subscription containing the new one we set foundSuperior true
				if (MQTTHelpers::checkTopicContains(ui.twSubscriptions->topLevelItem(i)->text(0), name)
						&& name != ui.twSubscriptions->topLevelItem(i)->text(0)) {
					foundSuperior = true;
					qDebug()<<"Can't add "<<name<<" because found superior: "<< ui.twSubscriptions->topLevelItem(i)->text(0);
					break;
				}
			}

			//if there wasn't a superior subscription we can subscribe to the new topic
			if (!foundSuperior) {
				QStringList toplevelName;
				toplevelName.push_back(name);
				QTreeWidgetItem* newTopLevelItem = new QTreeWidgetItem(toplevelName);
				ui.twSubscriptions->addTopLevelItem(newTopLevelItem);

				if (name.endsWith('#')) {
					//adding every topic that the subscription contains to twSubscriptions
					MQTTHelpers::addSubscriptionChildren(item, newTopLevelItem);
				}

				m_mqttClient->addMQTTSubscription(name, ui.cbQoS->currentIndex());

				if (name.endsWith('#')) {
					//if an already existing subscription contains a topic that the new subscription also contains
					//we decompose the already existing subscription
					//by unsubscribing from its topics, that are present in the new subscription as well
					const QStringList& nameList = name.split('/', QString::SkipEmptyParts);
					const QString root = nameList.first();
					QVector<QTreeWidgetItem*> children;
					for (int i = 0; i < ui.twSubscriptions->topLevelItemCount(); ++i) {
						if (ui.twSubscriptions->topLevelItem(i)->text(0).startsWith(root)
								&& name != ui.twSubscriptions->topLevelItem(i)->text(0)) {
							children.clear();

							//get the "leaf" children of the inspected subscription
							MQTTHelpers::findSubscriptionLeafChildren(children, ui.twSubscriptions->topLevelItem(i));
							for (int j = 0; j < children.size(); ++j) {
								if (MQTTHelpers::checkTopicContains(name, children[j]->text(0))) {
									//if the new subscription contains a topic, we unsubscribe from it
									QTreeWidgetItem* unsubscribeItem = children[j];
									while (unsubscribeItem->parent() != nullptr) {
										for (int i = 0; i < unsubscribeItem->parent()->childCount(); ++i) {

											if (unsubscribeItem->text(0) != unsubscribeItem->parent()->child(i)->text(0)) {
												//add topic as subscription
												m_mqttClient->addBeforeRemoveSubscription(unsubscribeItem->parent()->child(i)->text(0), ui.cbQoS->currentIndex());
												//also add it to twSubscriptions
												ui.twSubscriptions->addTopLevelItem(unsubscribeItem->parent()->takeChild(i));
												--i;
											} else {
												//before we remove the topic, we reparent it to the new subscription
												//so no data is lost
												m_mqttClient->reparentTopic(unsubscribeItem->text(0), name);
											}
										}
										unsubscribeItem = unsubscribeItem->parent();
									}

									qDebug()<<"Remove: "<<unsubscribeItem->text(0);
									m_mqttClient->removeMQTTSubscription(unsubscribeItem->text(0));

									ui.twSubscriptions->takeTopLevelItem(ui.twSubscriptions->indexOfTopLevelItem(unsubscribeItem));
								}
							}
						}
					}
				}

				manageCommonLevelSubscriptions();
				updateSubscriptionCompleter();

				if (!ui.bLWT->isEnabled())
					ui.bLWT->setEnabled(true);
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
 * unsubscribes from the topic represented by the current item of twSubscription in m_mqttClient
 */
void LiveDataDock::removeSubscription() {
	QTreeWidgetItem* unsubscribeItem = ui.twSubscriptions->currentItem();

	if (unsubscribeItem != nullptr) {
		qDebug() << "LiveDataDock: unsubscribe from " << unsubscribeItem->text(0);

		//if it is a top level item, meaning a topic that we really subscribed to(not one that belongs to a subscription)
		//we can simply unsubscribe from it
		if (unsubscribeItem->parent() == nullptr) {
			m_mqttClient->removeMQTTSubscription(unsubscribeItem->text(0));
			ui.twSubscriptions->takeTopLevelItem(ui.twSubscriptions->indexOfTopLevelItem(unsubscribeItem));
		}
		//otherwise we remove the selected item, but subscribe to every other topic, that was contained by
		//the selected item's parent subscription(top level item of twSubscriptions)
		else {
			while (unsubscribeItem->parent() != nullptr) {
				for (int i = 0; i < unsubscribeItem->parent()->childCount(); ++i) {
					if (unsubscribeItem->text(0) != unsubscribeItem->parent()->child(i)->text(0)) {
						//add topic as subscription
						m_mqttClient->addBeforeRemoveSubscription(unsubscribeItem->parent()->child(i)->text(0), ui.cbQoS->currentIndex());
						ui.twSubscriptions->addTopLevelItem(unsubscribeItem->parent()->takeChild(i));
						--i;
					}
				}
				unsubscribeItem = unsubscribeItem->parent();
			}

			//remove topic/subscription
			m_mqttClient->removeMQTTSubscription(unsubscribeItem->text(0));
			ui.twSubscriptions->takeTopLevelItem(ui.twSubscriptions->indexOfTopLevelItem(unsubscribeItem));

			//check if any common topics were subscribed, if possible merge them
			manageCommonLevelSubscriptions();
		}

		if (ui.twSubscriptions->topLevelItemCount() <= 0)
			ui.bLWT->setEnabled(false);

		updateSubscriptionCompleter();
	} else
		QMessageBox::warning(this, "Warning", "You didn't select any item from the Tree Widget");
}

/*!
 *\brief called when a new topic is added to the tree(twTopics)
 * appends the topic's root to the topicList if it isn't in the list already
 * then sets the completer for leTopics
 */
void LiveDataDock::setTopicCompleter(const QString& topicName) {
	if (!m_searching) {
		const QStringList& list = topicName.split('/', QString::SkipEmptyParts);
		QString topic;
		if (!list.isEmpty()) {
			topic = list.at(0);
		} else
			topic = topicName;

		if (!m_currentHost->topicList.contains(topic)) {
			m_currentHost->topicList.append(topic);
			m_topicCompleter = new QCompleter(m_currentHost->topicList, this);
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
	const QVector<QString>& subscriptions = m_mqttClient->MQTTSubscriptions();

	if (!subscriptions.isEmpty()) {
		for (const auto& subscription : subscriptions)
			subscriptionList << subscription;

		m_subscriptionCompleter = new QCompleter(subscriptionList, this);
		m_subscriptionCompleter->setCompletionMode(QCompleter::PopupCompletion);
		m_subscriptionCompleter->setCaseSensitivity(Qt::CaseSensitive);
		ui.leSubscriptions->setCompleter(m_subscriptionCompleter);
	} else {
		ui.leSubscriptions->setCompleter(nullptr);
	}
}

/*!
 *\brief called when 10 seconds passed since the last time the user searched for a certain root in twTopics
 * enables updating the completer for le
 */
void LiveDataDock::topicTimeout() {
	m_searching = false;
	m_searchTimer->stop();
}

/*!
 *\brief called when a new the host name of m_mqttClient changes
 * or when the MQTTClients initialize their subscriptions
 * Fills twSubscriptions with the subscriptions of the MQTTClient
 */
void LiveDataDock::fillSubscriptions() {
	ui.twSubscriptions->clear();

	QVector<QString> subscriptions = m_mqttClient->MQTTSubscriptions();
	for (int i = 0; i < subscriptions.count(); ++i) {
		QStringList name;
		name.append(subscriptions[i]);

		bool found = false;
		for (int j = 0; j < ui.twSubscriptions->topLevelItemCount(); ++j) {
			if (ui.twSubscriptions->topLevelItem(j)->text(0) == subscriptions[i]) {
				found = true;
				break;
			}
		}

		if (!found) {
			//Add the subscription to the tree widget
			QTreeWidgetItem* newItem = new QTreeWidgetItem(name);
			ui.twSubscriptions->addTopLevelItem(newItem);
			name.clear();
			name = subscriptions[i].split('/', QString::SkipEmptyParts);

			//find the corresponding "root" item in twTopics
			QTreeWidgetItem* topic = nullptr;
			for (int j = 0; j < ui.twTopics->topLevelItemCount(); ++j) {
				if (ui.twTopics->topLevelItem(j)->text(0) == name[0]) {
					topic = ui.twTopics->topLevelItem(j);
					break;
				}
			}

			//restore the children of the subscription
			if (topic != nullptr && topic->childCount() > 0)
				MQTTHelpers::restoreSubscriptionChildren(topic, newItem, name, 1);
		}
	}
	m_searching = false;
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

	int topItemIdx = -1;
	for (int i = 0; i < ui.twTopics->topLevelItemCount(); ++i)
		if (ui.twTopics->topLevelItem(i)->text(0) == rootName) {
			topItemIdx = i;
			break;
		}

	if (topItemIdx >= 0)
		ui.twTopics->scrollToItem(ui.twTopics->topLevelItem(topItemIdx), QAbstractItemView::ScrollHint::PositionAtTop);
}

/*!
 *\brief called when leSubscriptions' text is changed
 *		 if the rootName can be found in twSubscriptions, then we scroll it to the top of the tree widget
 *
 * \param rootName the current text of leSubscriptions
 */
void LiveDataDock::scrollToSubsriptionTreeItem(const QString& rootName) {
	int topItemIdx = -1;
	for (int i = 0; i < ui.twSubscriptions->topLevelItemCount(); ++i)
		if (ui.twSubscriptions->topLevelItem(i)->text(0) == rootName) {
			topItemIdx = i;
			break;
		}

	if (topItemIdx >= 0)
		ui.twSubscriptions->scrollToItem(ui.twSubscriptions->topLevelItem(topItemIdx), QAbstractItemView::ScrollHint::PositionAtTop);
}

/*!
 *\brief We search in twSubscriptions for topics that can be represented using + wildcards, then merge them.
 *		 We do this until there are no topics to merge
 */
void LiveDataDock::manageCommonLevelSubscriptions() {
	bool foundEqual = false;
	do {
		foundEqual = false;
		QMap<QString, QVector<QString>> equalTopicsMap;
		QVector<QString> equalTopics;

		//compare the subscriptions present in the TreeWidget
		for (int i = 0; i < ui.twSubscriptions->topLevelItemCount() - 1; ++i) {
			for (int j = i + 1; j < ui.twSubscriptions->topLevelItemCount(); ++j) {
				QString commonTopic = MQTTHelpers::checkCommonLevel(ui.twSubscriptions->topLevelItem(i)->text(0), ui.twSubscriptions->topLevelItem(j)->text(0));

				//if there is a common topic for the 2 compared topics, we add them to the map (using the common topic as key)
				if (!commonTopic.isEmpty()) {
					if (!equalTopicsMap[commonTopic].contains(ui.twSubscriptions->topLevelItem(i)->text(0))) {
						equalTopicsMap[commonTopic].push_back(ui.twSubscriptions->topLevelItem(i)->text(0));
					}

					if (!equalTopicsMap[commonTopic].contains(ui.twSubscriptions->topLevelItem(j)->text(0))) {
						equalTopicsMap[commonTopic].push_back(ui.twSubscriptions->topLevelItem(j)->text(0));
					}
				}
			}
		}

		if (!equalTopicsMap.isEmpty()) {
			qDebug()<<"Manage equal topics";

			QVector<QString> commonTopics;
			QMapIterator<QString, QVector<QString>> topics(equalTopicsMap);

			//check for every map entry, if the found topics can be merged or not
			while (topics.hasNext()) {
				topics.next();

				int level = MQTTHelpers::commonLevelIndex(topics.value().last(), topics.value().first());
				QStringList commonList = topics.value().first().split('/', QString::SkipEmptyParts);
				QTreeWidgetItem* currentItem = nullptr;

				//search the corresponding item to the common topics first level(root)
				for (int i = 0; i < ui.twTopics->topLevelItemCount(); ++i) {
					if (ui.twTopics->topLevelItem(i)->text(0) == commonList.first()) {
						currentItem = ui.twTopics->topLevelItem(i);
						break;
					}
				}

				if (!currentItem)
					break;

				//calculate the number of topics the new + wildcard could replace
				int childCount = MQTTHelpers::checkCommonChildCount(1, level, commonList, currentItem);
				if (childCount > 0) {
					//if the number of topics found and the calculated number of topics is equal, the topics can be merged
					if (topics.value().size() == childCount) {
						qDebug() << "Found common topic to manage: " << topics.key();
						foundEqual = true;
						commonTopics.push_back(topics.key());
					}
				}
			}

			if (foundEqual) {
				//if there are more common topics, the topics of which can be merged, we choose the one which has the lowest level new "+" wildcard
				int highestLevel = INT_MAX;
				int topicIdx = -1;
				for (int i = 0; i < commonTopics.size(); ++i) {
					int level = MQTTHelpers::commonLevelIndex(equalTopicsMap[commonTopics[i]].first(), commonTopics[i]);
					if (level < highestLevel) {
						topicIdx = i;
						highestLevel = level;
					}
				}
				qDebug() << "Start to manage: " << commonTopics[topicIdx];
				equalTopics.append(equalTopicsMap[commonTopics[topicIdx]]);

				//Add the common topic ("merging")
				QString commonTopic;
				commonTopic = MQTTHelpers::checkCommonLevel(equalTopics.first(), equalTopics.last());
				QStringList nameList;
				nameList.append(commonTopic);
				QTreeWidgetItem* newTopic = new QTreeWidgetItem(nameList);
				ui.twSubscriptions->addTopLevelItem(newTopic);

				//remove the "merged" topics
				for (int i = 0; i < equalTopics.size(); ++i) {
					for (int j = 0; j < ui.twSubscriptions->topLevelItemCount(); ++j) {
						if (ui.twSubscriptions->topLevelItem(j)->text(0) == equalTopics[i]) {
							newTopic->addChild(ui.twSubscriptions->takeTopLevelItem(j));
							break;
						}
					}
				}

				//remove any subscription that the new subscription contains
				for (int i = 0; i < ui.twSubscriptions->topLevelItemCount(); ++i) {
					if (MQTTHelpers::checkTopicContains(commonTopic, ui.twSubscriptions->topLevelItem(i)->text(0)) &&
							commonTopic != ui.twSubscriptions->topLevelItem(i)->text(0) ) {
						ui.twSubscriptions->topLevelItem(i)->takeChildren();
						ui.twSubscriptions->takeTopLevelItem(i);
						i--;
					}
				}

				//make the subscription on commonTopic in m_mqttClient
				m_mqttClient->addMQTTSubscription(commonTopic, ui.cbQoS->currentIndex());
			}
		}
	} while (foundEqual);
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
	if (topicName.contains(sep)) {
		QStringList list = topicName.split(sep, QString::SkipEmptyParts);

		if (!list.isEmpty()) {
			rootName = list.at(0);
			name.append(list.at(0));
			QTreeWidgetItem* currentItem;
			//check whether the first level of the topic can be found in twTopics
			int topItemIdx = -1;
			for (int i = 0; i < ui.twTopics->topLevelItemCount(); ++i) {
				if (ui.twTopics->topLevelItem(i)->text(0) == list.at(0)) {
					topItemIdx = i;
					break;
				}
			}
			//if not we simply add every level of the topic to the tree
			if ( topItemIdx < 0) {
				currentItem = new QTreeWidgetItem(name);
				ui.twTopics->addTopLevelItem(currentItem);
				for (int i = 1; i < list.size(); ++i) {
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
				for (; listIdx < list.size(); ++listIdx) {
					QTreeWidgetItem* childItem = nullptr;
					bool found = false;
					for (int j = 0; j < currentItem->childCount(); ++j) {
						childItem = currentItem->child(j);
						if (childItem->text(0) == list.at(listIdx)) {
							found = true;
							currentItem = childItem;
							break;
						}
					}
					if (!found) {
						//this is the level that isn't present in the tree
						break;
					}
				}

				//add every level to the tree starting with the first level that isn't part of the tree
				for (; listIdx < list.size(); ++listIdx) {
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
	for (int i = 0; i < ui.twSubscriptions->topLevelItemCount(); ++i) {
		QStringList subscriptionName = ui.twSubscriptions->topLevelItem(i)->text(0).split('/', QString::SkipEmptyParts);
		if (rootName == subscriptionName[0]) {
			fillSubscriptions();
			break;
		}
	}

	//signals that a newTopic was added, in order to fill the completer of leTopics
	//we have to pass the whole topic name, not just the root name, for testing purposes
	emit newTopic(topicName);
}

/*!
 *\brief called when a client receives a message, if the clients hostname isn't identic with the host name of MQTTClient
 * if the message arrived from a new topic, the topic is added to the host data
 */
void LiveDataDock::mqttMessageReceivedInBackground(const QByteArray& message, const QMqttTopicName& topic) {
	Q_UNUSED(message)
	if (!m_currentHost->addedTopics.contains(topic.name()))
		m_currentHost->addedTopics.push_back(topic.name());
}

/*!
 *\brief called when an MQTTClient is about to be deleted
 * removes every data connected to the MQTTClient, and disconnects the corresponding client from the host
 *
 * \param hostname the host name of the MQTTClient that will be deleted
 * \param name the host name of the MQTTClient that will be deleted
 */
void LiveDataDock::removeClient(const QString& hostname, quint16 port) {
	auto it = m_hosts.find(qMakePair(hostname, port));
	if (it == m_hosts.end())
		return;

	MQTTHost & host = it.value();

	if (host.count > 1) {
		--host.count;
		return;
	}

	host.client->disconnectFromHost();

	if (m_previousMQTTClient != nullptr && m_previousMQTTClient->clientHostName() == hostname) {
		disconnect(m_previousHost->client, &QMqttClient::messageReceived, this, &LiveDataDock::mqttMessageReceivedInBackground);
		m_previousMQTTClient = nullptr;
	}

	if (m_mqttClient->clientHostName() == hostname) {
		ui.twSubscriptions->clear();
		ui.twTopics->clear();
		m_mqttClient = nullptr;
	}

	delete host.client;
	m_hosts.erase(it);
}

/*!
 * \brief Used for testing the MQTT related features
 * \param topic
 */
bool LiveDataDock::testSubscribe(const QString& topic) {
	QStringList topicList = topic.split('/', QString::SkipEmptyParts);
	QTreeWidgetItem* currentItem = nullptr;
	for (int i = 0; i < ui.twTopics->topLevelItemCount(); ++i) {
		if (ui.twTopics->topLevelItem(i)->text(0) == topicList[0]) {
			currentItem = ui.twTopics->topLevelItem(i);
			break;
		}
	}

	if (currentItem) {
		for (int i = 1 ; i < topicList.size(); ++i) {
			if (topicList[i] == '#')
				break;

			for (int j = 0; j < currentItem->childCount(); ++j) {
				if (currentItem->child(j)->text(0) == topicList[i]) {
					currentItem = currentItem->child(j);
					break;
				} else if (j == currentItem->childCount() - 1)
					return false;

			}
		}
	} else
		return false;

	ui.twTopics->setCurrentItem(currentItem);
	addSubscription();
	return true;
}

/*!
 * \brief Used for testing the MQTT related features
 * \param topic
 */
bool LiveDataDock::testUnsubscribe(const QString& topic) {
	QTreeWidgetItem* currentItem = nullptr;
	for (int i = 0; i < ui.twSubscriptions->topLevelItemCount(); ++i) {
		if (MQTTHelpers::checkTopicContains(ui.twSubscriptions->topLevelItem(i)->text(0), topic)) {
			currentItem = ui.twSubscriptions->topLevelItem(i);
			break;
		}
	}

	if (currentItem) {
		do {
			if (topic == currentItem->text(0)) {
				ui.twSubscriptions->setCurrentItem(currentItem);
				removeSubscription();
				return true;
			} else {
				for (int i = 0; i < currentItem->childCount(); ++i) {
					qDebug()<<currentItem->child(i)->text(0)<<" "<<topic;
					if (MQTTHelpers::checkTopicContains(currentItem->child(i)->text(0), topic)) {
						currentItem = currentItem->child(i);
						break;
					} else if (i == currentItem->childCount() - 1)
						return false;
				}
			}
		} while (currentItem);
	} else
		return false;

	return false;
}

void LiveDataDock::showWillSettings() {
	QMenu menu;
	const QVector<QString>& topics = m_mqttClient->topicNames();
	MQTTWillSettingsWidget willSettingsWidget(&menu, m_mqttClient->willSettings(), topics);

	connect(&willSettingsWidget, &MQTTWillSettingsWidget::applyClicked, [this, &menu, &willSettingsWidget]() {
		this->useWillMessage(willSettingsWidget.will().enabled);
		this->willMessageTypeChanged(willSettingsWidget.will().willMessageType);
		this->updateTypeChanged(willSettingsWidget.will().willUpdateType);
		this->willRetainChanged(willSettingsWidget.will().willRetain);
		this->willUpdateIntervalChanged(willSettingsWidget.will().willTimeInterval);
		this->willOwnMessageChanged(willSettingsWidget.will().willOwnMessage);
		this->willTopicChanged(willSettingsWidget.will().willTopic);
		this->statisticsChanged(willSettingsWidget.statisticsType());

		menu.close();
	});

	QWidgetAction* widgetAction = new QWidgetAction(this);
	widgetAction->setDefaultWidget(&willSettingsWidget);
	menu.addAction(widgetAction);

	QPoint pos(ui.bLWT->sizeHint().width(), ui.bLWT->sizeHint().height());
	menu.exec(ui.bLWT->mapToGlobal(pos));
}
#endif
