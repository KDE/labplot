/***************************************************************************
File                 : LiveDataDock.cpp
Project              : LabPlot
Description          : Dock widget for live data properties
--------------------------------------------------------------------
Copyright            : (C) 2017 by Fabian Kristof (fkristofszabolcs@gmail.com)
Copyright            : (C) 2018-2019 Kovacs Ferencz (kferike98@gmail.com)
Copyright            : (C) 2018 by Stefan Gerlach (stefan.gerlach@uni.kn)
Copyright            : (C) 2017-2019 Alexander Semke (alexander.semke@web.de)
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
#include <QCompleter>
#include <QFile>
#include <QStandardItemModel>
#include <QTimer>
#include <QTreeWidgetItem>

#include <KLocalizedString>

#ifdef HAVE_MQTT
#include "kdefrontend/widgets/MQTTWillSettingsWidget.h"
#include "kdefrontend/datasources/MQTTSubscriptionWidget.h"
#include <QMessageBox>
#include <QWidgetAction>
#include <QMenu>
#endif

LiveDataDock::LiveDataDock(QWidget* parent) : BaseDock(parent)
#ifdef HAVE_MQTT
	,
	m_subscriptionWidget(new MQTTSubscriptionWidget(this))
#endif
{
	ui.setupUi(this);
	m_leName = ui.leName;
	//leComment = // not available

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
	connect(ui.bWillUpdateNow, &QPushButton::clicked, this, &LiveDataDock::willUpdateNow);
	connect(ui.bLWT, &QPushButton::clicked, this, &LiveDataDock::showWillSettings);
	connect(m_subscriptionWidget, &MQTTSubscriptionWidget::enableWill, this, &LiveDataDock::enableWill);

	ui.swSubscriptions->addWidget(m_subscriptionWidget);
	ui.swSubscriptions->setCurrentWidget(m_subscriptionWidget);

	ui.bLWT->setToolTip(i18n("Manage MQTT connection's will settings"));
	ui.bLWT->setIcon(ui.bLWT->style()->standardIcon(QStyle::SP_FileDialogDetailedView));

	QString info = i18n("Specify the 'Last Will and Testament' message (LWT). At least one topic has to be subscribed.");
	ui.lLWT->setToolTip(info);
	ui.bLWT->setToolTip(info);
	ui.bLWT->setEnabled(false);
	ui.bLWT->setIcon(ui.bLWT->style()->standardIcon(QStyle::SP_FileDialogDetailedView));
#endif
}

#ifdef HAVE_MQTT
LiveDataDock::~LiveDataDock() {
	for (auto & host : m_hosts)
		delete host.client;

	delete m_subscriptionWidget;
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

	if (client->updateType() == MQTTClient::UpdateType::NewData) {
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

	if (client->readingType() == MQTTClient::ReadingType::TillEnd) {
		ui.lSampleSize->hide();
		ui.sbSampleSize->hide();
	} else
		ui.sbSampleSize->setValue(client->sampleSize());

	// disable "whole file" option
	const QStandardItemModel* model = qobject_cast<const QStandardItemModel*>(ui.cbReadingType->model());
	QStandardItem* item = model->item(static_cast<int>(LiveDataSource::ReadingType::WholeFile));
	item->setFlags(item->flags() & ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled));
	if (ui.cbReadingType->currentIndex() == static_cast<int>(LiveDataSource::ReadingType::WholeFile))
		ui.cbReadingType->setCurrentIndex(static_cast<int>(LiveDataSource::ReadingType::TillEnd));

	m_mqttClient = client; // updates may be applied from now on

	//show MQTT connected options
	ui.lTopics->show();
	ui.swSubscriptions->setVisible(true);
	m_subscriptionWidget->setVisible(true);
	m_subscriptionWidget->makeVisible(true);
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

		connect(m_subscriptionWidget, &MQTTSubscriptionWidget::reparentTopic, client, &MQTTClient::reparentTopic);
		connect(m_subscriptionWidget, &MQTTSubscriptionWidget::addBeforeRemoveSubscription, client, &MQTTClient::addBeforeRemoveSubscription);
		connect(m_subscriptionWidget, &MQTTSubscriptionWidget::removeMQTTSubscription, client, &MQTTClient::removeMQTTSubscription);
		connect(m_subscriptionWidget, &MQTTSubscriptionWidget::makeSubscription, client, &MQTTClient::addMQTTSubscription);


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
		m_updateSubscriptionConn = connect(client, &MQTTClient::MQTTSubscribed, [this]() {
			emit updateSubscriptionTree(m_mqttClient->MQTTSubscriptions());
		});

		//Fill the subscription tree(useful if the MQTTClient was loaded)
		QVector<QString> topics = client->topicNames();
		for (const auto& topic : topics)
			addTopicToTree(topic);
		emit updateSubscriptionTree(m_mqttClient->MQTTSubscriptions());
	}

	//if the previous MQTTClient's host name was different from the current one we have to disconnect some slots
	//and clear the tree widgets
	else if (m_previousMQTTClient->clientHostName() != client->clientHostName()) {
		disconnect(m_updateSubscriptionConn);
		disconnect(m_previousHost->client, &QMqttClient::messageReceived, this, &LiveDataDock::mqttMessageReceived);
		connect(m_previousHost->client, &QMqttClient::messageReceived, this, &LiveDataDock::mqttMessageReceivedInBackground);

		disconnect(m_currentHost->client, &QMqttClient::messageReceived, this, &LiveDataDock::mqttMessageReceivedInBackground);

		disconnect(m_subscriptionWidget, &MQTTSubscriptionWidget::reparentTopic, m_previousMQTTClient, &MQTTClient::reparentTopic);
		disconnect(m_subscriptionWidget, &MQTTSubscriptionWidget::addBeforeRemoveSubscription, m_previousMQTTClient, &MQTTClient::addBeforeRemoveSubscription);
		disconnect(m_subscriptionWidget, &MQTTSubscriptionWidget::removeMQTTSubscription, m_previousMQTTClient, &MQTTClient::removeMQTTSubscription);
		disconnect(m_subscriptionWidget, &MQTTSubscriptionWidget::makeSubscription,  m_previousMQTTClient, &MQTTClient::addMQTTSubscription);

		m_previousHost->topicList = m_subscriptionWidget->getTopicList();
		m_subscriptionWidget->setTopicList(m_currentHost->topicList);

		emit MQTTClearTopics();
		//repopulating the tree widget with the already known topics of the client
		for (int i = 0; i < m_currentHost->addedTopics.size(); ++i)
			addTopicToTree(m_currentHost->addedTopics.at(i));

		//fill subscriptions tree widget
		emit updateSubscriptionTree(m_mqttClient->MQTTSubscriptions());

		m_updateSubscriptionConn = connect(client, &MQTTClient::MQTTSubscribed, [this]() {
			emit updateSubscriptionTree(m_mqttClient->MQTTSubscriptions());
		});
		connect(m_currentHost->client, &QMqttClient::messageReceived, this, &LiveDataDock::mqttMessageReceived);

		connect(m_subscriptionWidget, &MQTTSubscriptionWidget::reparentTopic, client, &MQTTClient::reparentTopic);
		connect(m_subscriptionWidget, &MQTTSubscriptionWidget::addBeforeRemoveSubscription, client, &MQTTClient::addBeforeRemoveSubscription);
		connect(m_subscriptionWidget, &MQTTSubscriptionWidget::removeMQTTSubscription, client, &MQTTClient::removeMQTTSubscription);
		connect(m_subscriptionWidget, &MQTTSubscriptionWidget::makeSubscription, client, &MQTTClient::addMQTTSubscription);
	}

	if (client->willUpdateType() == MQTTClient::WillUpdateType::OnClick && client->MQTTWillUse())
		ui.bWillUpdateNow->show();

	m_previousMQTTClient = oldclient;
}
#endif

/*!
 * \brief Sets the live data source of this dock widget
 * \param source
 */
void LiveDataDock::setLiveDataSource(LiveDataSource* const source) {
#ifdef HAVE_MQTT
	m_mqttClient = nullptr;
#endif
// 	if (m_liveDataSource == source)
// 		return;
	m_liveDataSource = nullptr; // prevent updates due to changes to input widgets

	ui.leName->setText(source->name());
	ui.leName->setStyleSheet("");
	ui.leName->setToolTip("");
	const LiveDataSource::SourceType sourceType = source->sourceType();
	const LiveDataSource::ReadingType readingType = source->readingType();
	const LiveDataSource::UpdateType updateType = source->updateType();
	const AbstractFileFilter::FileType fileType = source->fileType();
	ui.sbUpdateInterval->setValue(source->updateInterval());
	ui.cbUpdateType->setCurrentIndex(static_cast<int>(updateType));
	ui.cbReadingType->setCurrentIndex(static_cast<int>(readingType));

	switch (sourceType) {
	case LiveDataSource::SourceType::FileOrPipe:
		ui.leSourceInfo->setText(source->fileName());
		if (QFile::exists(source->fileName()))
			ui.leSourceInfo->setStyleSheet(QString());
		else
			ui.leSourceInfo->setStyleSheet("QLineEdit{background:red;}");
		break;
	case LiveDataSource::SourceType::NetworkTcpSocket:
	case LiveDataSource::SourceType::NetworkUdpSocket:
		ui.leSourceInfo->setText(QStringLiteral("%1:%2").arg(source->host()).arg(source->port()));
		break;
	case LiveDataSource::SourceType::LocalSocket:
		ui.leSourceInfo->setText(source->localSocketName());
		break;
	case LiveDataSource::SourceType::SerialPort:
		ui.leSourceInfo->setText(source->serialPortName());
		break;
	case LiveDataSource::SourceType::MQTT:
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
	QStandardItem* item = model->item(static_cast<int>(LiveDataSource::ReadingType::WholeFile));
	if (sourceType == LiveDataSource::SourceType::FileOrPipe) {
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		//for file types other than ASCII and binary we support re-reading the whole file only
		//select "read whole file" and deactivate the combobox
		if (fileType != AbstractFileFilter::Ascii && fileType != AbstractFileFilter::Binary) {
			ui.cbReadingType->setCurrentIndex(static_cast<int>(LiveDataSource::ReadingType::WholeFile));
			ui.cbReadingType->setEnabled(false);
		} else
			ui.cbReadingType->setEnabled(true);
	} else {
		if (ui.cbReadingType->currentIndex() == static_cast<int>(LiveDataSource::ReadingType::WholeFile))
			ui.cbReadingType->setCurrentIndex(static_cast<int>(LiveDataSource::ReadingType::TillEnd));
		item->setFlags(item->flags() & ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled));
	}

	if (((sourceType == LiveDataSource::SourceType::FileOrPipe || sourceType == LiveDataSource::SourceType::NetworkUdpSocket) &&
	        (readingType == LiveDataSource::ReadingType::ContinuousFixed || readingType == LiveDataSource::ReadingType::FromEnd)))
		ui.sbSampleSize->setValue(source->sampleSize());
	else {
		ui.lSampleSize->hide();
		ui.sbSampleSize->hide();
	}

	// disable "on new data"-option if not available
	model = qobject_cast<const QStandardItemModel*>(ui.cbUpdateType->model());
	item = model->item(static_cast<int>(LiveDataSource::UpdateType::NewData));
	if (sourceType == LiveDataSource::SourceType::NetworkTcpSocket || sourceType == LiveDataSource::SourceType::NetworkUdpSocket ||
	        sourceType == LiveDataSource::SourceType::SerialPort)
		item->setFlags(item->flags() & ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled));
	else
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

	ui.lTopics->hide();
	ui.bLWT->hide();
	ui.lLWT->hide();
	ui.bWillUpdateNow->hide();
	ui.swSubscriptions->hide();
#ifdef HAVE_MQTT
	m_subscriptionWidget->hide();
	m_subscriptionWidget->hide();
#endif
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
	if (m_liveDataSource) {
		if (!m_liveDataSource->setName(name, false)) {
			ui.leName->setStyleSheet("background:red;");
			ui.leName->setToolTip(i18n("Please choose another name, because this is already in use."));
			return;
		}
	}
#ifdef HAVE_MQTT
	else if (m_mqttClient) {
		if (!m_mqttClient->setName(name, false)) {
			ui.leName->setStyleSheet("background:red;");
			ui.leName->setToolTip(i18n("Please choose another name, because this is already in use."));
			return;
		}
	}
#endif
	ui.leName->setStyleSheet("");
	ui.leName->setToolTip("");
}

/*!
 * \brief LiveDataDock::updateTypeChanged
 * \param idx
 */
void LiveDataDock::updateTypeChanged(int idx) {
	if (m_liveDataSource)  {
		DEBUG("LiveDataDock::updateTypeChanged()");
		const auto updateType = static_cast<LiveDataSource::UpdateType>(idx);

		switch (updateType) {
		case LiveDataSource::UpdateType::TimeInterval: {
				ui.lUpdateInterval->show();
				ui.sbUpdateInterval->show();
				const auto s = m_liveDataSource->sourceType();
				const auto r = m_liveDataSource->readingType();
				const bool showSampleSize = ((s == LiveDataSource::SourceType::FileOrPipe || s == LiveDataSource::SourceType::NetworkUdpSocket) &&
				                             (r == LiveDataSource::ReadingType::ContinuousFixed || r == LiveDataSource::ReadingType::FromEnd));
				ui.lSampleSize->setVisible(showSampleSize);
				ui.sbSampleSize->setVisible(showSampleSize);

				m_liveDataSource->setUpdateType(updateType);
				m_liveDataSource->setUpdateInterval(ui.sbUpdateInterval->value());
				break;
			}
		case LiveDataSource::UpdateType::NewData:
			ui.lUpdateInterval->hide();
			ui.sbUpdateInterval->hide();
			ui.lSampleSize->hide();
			ui.sbSampleSize->hide();

			m_liveDataSource->setUpdateType(updateType);
		}
	}
#ifdef HAVE_MQTT
	else if (m_mqttClient) {
		DEBUG("LiveDataDock::updateTypeChanged()");
		const auto type = static_cast<MQTTClient::UpdateType>(idx);

		if (type == MQTTClient::UpdateType::TimeInterval) {
			ui.lUpdateInterval->show();
			ui.sbUpdateInterval->show();

			m_mqttClient->setUpdateType(type);
			m_mqttClient->setUpdateInterval(ui.sbUpdateInterval->value());
		} else if (type == MQTTClient::UpdateType::NewData) {
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
		const auto sourceType = m_liveDataSource->sourceType();
		const auto updateType = m_liveDataSource->updateType();

		if (sourceType == LiveDataSource::SourceType::NetworkTcpSocket || sourceType == LiveDataSource::SourceType::LocalSocket
			|| sourceType == LiveDataSource::SourceType::SerialPort
		        || type == LiveDataSource::ReadingType::TillEnd || type == LiveDataSource::ReadingType::WholeFile
		        || updateType == LiveDataSource::UpdateType::NewData) {
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

		if (type == MQTTClient::ReadingType::TillEnd) {
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
		if (m_mqttClient->willUpdateType() == MQTTClient::WillUpdateType::OnClick)
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
	if (useWillRetainMessages)
		m_mqttClient->setWillRetain(true);
	else
		m_mqttClient->setWillRetain(false);
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

<<<<<<< HEAD
	if (updateType == static_cast<int>(MQTTClient::WillUpdateType::TimePeriod)) {
		ui.bWillUpdateNow->hide();
		m_mqttClient->startWillTimer();
	} else if (updateType == static_cast<int>(MQTTClient::WillUpdateType::OnClick)) {
=======
	if (static_cast<MQTTClient::WillUpdateType>(updateType) == MQTTClient::WillUpdateType::TimePeriod) {
		ui.bWillUpdateNow->hide();
		m_mqttClient->startWillTimer();
	} else if (static_cast<MQTTClient::WillUpdateType>(updateType) == MQTTClient::WillUpdateType::OnClick) {
>>>>>>> 89e8ac3aa142df66580c3d747a1832c200e304dd
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
<<<<<<< HEAD
	if (willStatisticsType != MQTTClient::WillStatisticsType::NoStatistics) {
=======
	if (static_cast<int>(willStatisticsType) >= 0) {
>>>>>>> 89e8ac3aa142df66580c3d747a1832c200e304dd
		//if it's not already added and it's checked we add it
		if (!m_mqttClient->willStatistics().at(static_cast<int>(willStatisticsType)))
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
			for (int i = 0; i < m_subscriptionWidget->topicCount(); ++i) {
				if (m_subscriptionWidget->topLevelTopic(i)->text(0) == list.at(0)) {
					topItemIdx = i;
					break;
				}
			}
			//if not we simply add every level of the topic to the tree
			if ( topItemIdx < 0) {
				currentItem = new QTreeWidgetItem(name);
				m_subscriptionWidget->addTopic(currentItem);
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
				currentItem = m_subscriptionWidget->topLevelTopic(topItemIdx);
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
	} else {
		rootName = topicName;
		name.append(topicName);
		m_subscriptionWidget->addTopic(new QTreeWidgetItem(name));
	}

	//if a subscribed topic contains the new topic, we have to update twSubscriptions
	for (int i = 0; i < m_subscriptionWidget->subscriptionCount(); ++i) {
		QStringList subscriptionName = m_subscriptionWidget->topLevelSubscription(i)->text(0).split('/', QString::SkipEmptyParts);
		if (rootName == subscriptionName[0]) {
			emit updateSubscriptionTree(m_mqttClient->MQTTSubscriptions());
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
		emit MQTTClearTopics();
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
	for (int i = 0; i < m_subscriptionWidget->topicCount(); ++i) {
		if (m_subscriptionWidget->topLevelTopic(i)->text(0) == topicList[0]) {
			currentItem = m_subscriptionWidget->topLevelTopic(i);
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

	m_subscriptionWidget->testSubscribe(currentItem);
	return true;
}

/*!
 * \brief Used for testing the MQTT related features
 * \param topic
 */
bool LiveDataDock::testUnsubscribe(const QString& topic) {
	QTreeWidgetItem* currentItem = nullptr;
	for (int i = 0; i < m_subscriptionWidget->subscriptionCount(); ++i) {
		if (MQTTSubscriptionWidget::checkTopicContains(m_subscriptionWidget->topLevelSubscription(i)->text(0), topic)) {
			currentItem = m_subscriptionWidget->topLevelSubscription(i);
			break;
		}
	}

	if (currentItem) {
		do {
			if (topic == currentItem->text(0)) {
				m_subscriptionWidget->testUnsubscribe(currentItem);
				return true;
			} else {
				for (int i = 0; i < currentItem->childCount(); ++i) {
					qDebug()<<currentItem->child(i)->text(0)<<" "<<topic;
					if (MQTTSubscriptionWidget::checkTopicContains(currentItem->child(i)->text(0), topic)) {
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
		this->updateTypeChanged(static_cast<int>(willSettingsWidget.will().willUpdateType));
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

void LiveDataDock::enableWill(bool enable) {
	if(enable) {
		if(!ui.bLWT->isEnabled())
			ui.bLWT->setEnabled(enable);
	} else
		ui.bLWT->setEnabled(enable);
}
#endif
