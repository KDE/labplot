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
#include <KLocale>

LiveDataDock::LiveDataDock(QWidget* parent) :
	QWidget(parent), m_paused(false) {
	ui.setupUi(this);

	ui.bUpdateNow->setIcon(QIcon::fromTheme(QLatin1String("view-refresh")));

	connect(ui.bPausePlayReading, &QPushButton::clicked, this, &LiveDataDock::pauseContinueReading);
	connect(ui.bUpdateNow, &QPushButton::clicked, this, &LiveDataDock::updateNow);
	connect(ui.sbUpdateInterval, static_cast<void (QSpinBox::*) (int)>(&QSpinBox::valueChanged), this, &LiveDataDock::updateIntervalChanged);

	connect(ui.leKeepNValues, &QLineEdit::textChanged, this, &LiveDataDock::keepNvaluesChanged);
	connect(ui.sbSampleRate, static_cast<void (QSpinBox::*) (int)>(&QSpinBox::valueChanged), this, &LiveDataDock::sampleRateChanged);
	connect(ui.cbUpdateType, static_cast<void (QComboBox::*) (int)>(&QComboBox::currentIndexChanged), this, &LiveDataDock::updateTypeChanged);
	connect(ui.cbReadingType, static_cast<void (QComboBox::*) (int)>(&QComboBox::currentIndexChanged), this, &LiveDataDock::readingTypeChanged);

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
}

LiveDataDock::~LiveDataDock() {
}

/*!
 * \brief Sets the live data sources of this dock widget
 * \param sources
 */
void LiveDataDock::setLiveDataSources(const QList<LiveDataSource*>& sources) {
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
        if (itemIdx != -1) {
            ui.cbReadingType->removeItem(itemIdx);
        }
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


	if(fds->sourceType() == LiveDataSource::SourceType::Mqtt) {
		ui.chbWill->show();
		connect(fds, &LiveDataSource::mqttSubscribed, this, &LiveDataDock::updateTopics);		

		if(fds->mqttWillUse()) {
			ui.chbWillRetain->show();
			ui.cbWillQoS->show();
			ui.cbWillMessageType->show();
			ui.cbWillTopic->show();
			ui.cbWillUpdate->show();
			ui.lWillMessageType->show();
			ui.lWillQos->hide();
			ui.lWillTopic->show();
			ui.lWillUpdate->show();

			if (ui.cbWillMessageType->currentIndex() == (int)LiveDataSource::WillMessageType::OwnMessage) {
				ui.leWillOwnMessage->show();
				ui.lWillOwnMessage->show();
			}
			else if(ui.cbWillMessageType->currentIndex() == (int)LiveDataSource::WillMessageType::Statistics){
				ui.lWillStatistics->show();
				ui.lwWillStatistics->show();
			}


			if(ui.cbWillUpdate->currentIndex() == 0) {
				ui.leWillUpdateInterval->show();
				ui.lWillUpdateInterval->show();
			}
			else if (ui.cbWillUpdate->currentIndex() == 1)
				ui.bWillUpdateNow->show();
		}
	}
}

/*!
 * \brief Modifies the sample rate of the live data sources
 * \param sampleRate
 */
void LiveDataDock::sampleRateChanged(int sampleRate) {
	for (auto* source : m_liveDataSources)
		source->setSampleRate(sampleRate);
}

/*!
 * \brief Updates the live data sources now
 */
void LiveDataDock::updateNow() {
	for (auto* source : m_liveDataSources)
		source->updateNow();
}

/*!
 * \brief LiveDataDock::updateTypeChanged
 * \param idx
 */
void LiveDataDock::updateTypeChanged(int idx) {
	LiveDataSource::UpdateType type = static_cast<LiveDataSource::UpdateType>(idx);

	if (type == LiveDataSource::UpdateType::TimeInterval) {
		ui.lUpdateInterval->show();
		ui.sbUpdateInterval->show();

		for (auto* source: m_liveDataSources) {
			source->setUpdateType(type);
			source->setUpdateInterval(ui.sbUpdateInterval->value());
			source->setFileWatched(false);
		}
	} else if (type == LiveDataSource::UpdateType::NewData) {
		ui.lUpdateInterval->hide();
		ui.sbUpdateInterval->hide();

		for (auto* source: m_liveDataSources) {
			source->setFileWatched(true);
			source->setUpdateType(type);
		}
	}
}

/*!
 * \brief Handles the change of the reading type in the dock widget
 * \param idx
 */
void LiveDataDock::readingTypeChanged(int idx) {
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

/*!
 * \brief Modifies the update interval of the live data sources
 * \param updateInterval
 */
void LiveDataDock::updateIntervalChanged(int updateInterval) {
	for (auto* source : m_liveDataSources)
		source->setUpdateInterval(updateInterval);
}

/*!
 * \brief Modifies the number of samples to keep in each of the live data sources
 * \param keepNvalues
 */
void LiveDataDock::keepNvaluesChanged(const QString& keepNvalues) {
	for (auto* source : m_liveDataSources)
		source->setKeepNvalues(keepNvalues.toInt());
}

/*!
 * \brief Pauses the reading of the live data source
 */
void LiveDataDock::pauseReading() {
	for (auto* source: m_liveDataSources)
		source->pauseReading();
}

/*!
 * \brief Continues the reading of the live data source
 */
void LiveDataDock::continueReading() {
	for (auto* source: m_liveDataSources)
		source->continueReading();
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

void LiveDataDock::useWillMessage(int state) {
	if(state == Qt::Checked) {
		for (auto* source: m_liveDataSources)
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

		if (ui.cbWillMessageType->currentIndex() == (int)LiveDataSource::WillMessageType::OwnMessage) {
			ui.leWillOwnMessage->show();
			ui.lWillOwnMessage->show();
		}
		else if(ui.cbWillMessageType->currentIndex() == (int)LiveDataSource::WillMessageType::Statistics){
			ui.lWillStatistics->show();
			ui.lwWillStatistics->show();
		}


		if(ui.cbWillUpdate->currentIndex() == 0) {
			ui.leWillUpdateInterval->show();
			ui.lWillUpdateInterval->show();
		}
		else if (ui.cbWillUpdate->currentIndex() == 1)
			ui.bWillUpdateNow->show();

		for (auto* source: m_liveDataSources) {
			source->setWillRetain(ui.chbWillRetain->isChecked());
			source->setWillQoS(ui.cbWillQoS->currentIndex());

			source->setWillMessageType(static_cast<LiveDataSource::WillMessageType> (ui.cbWillMessageType->currentIndex()) );
			if(static_cast<LiveDataSource::WillMessageType> (ui.cbWillMessageType->currentIndex())
					== LiveDataSource::WillMessageType::OwnMessage)
				source->setWillOwnMessage(ui.leWillOwnMessage->text());
			if(ui.cbWillTopic->count() > 0) {
				source->setWillTopic(ui.cbWillTopic->currentText());
				source->clearLastMessage();
			}

			source->setWillUpdateType(static_cast<LiveDataSource::WillUpdateType>(ui.cbWillUpdate->currentIndex()));
			source->setWillTimeInterval(ui.leWillUpdateInterval->text().toInt());
		}
	}
	else if (state == Qt::Unchecked) {
		for (auto* source: m_liveDataSources)
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
	for (auto* source: m_liveDataSources)
		source->setWillQoS(QoS);
}

void LiveDataDock::willRetainChanged(int state) {
	if(state == Qt::Checked) {
		for (auto* source: m_liveDataSources)
			source->setWillRetain(true);
	}
	else if (state == Qt::Unchecked) {
		for (auto* source: m_liveDataSources)
			source->setWillRetain(false);
	}
}

void LiveDataDock::willTopicChanged(const QString& topic) {
	for (auto* source: m_liveDataSources) {
		if(source->willTopic() != topic)
			source->clearLastMessage();
		source->setWillTopic(topic);		
	}
}

void LiveDataDock::willMessageTypeChanged(int type) {
	for (auto* source: m_liveDataSources)
		source->setWillMessageType(static_cast<LiveDataSource::WillMessageType> (type));
	if(static_cast<LiveDataSource::WillMessageType> (type) == LiveDataSource::WillMessageType::OwnMessage) {
		ui.leWillOwnMessage->show();
		ui.lWillOwnMessage->show();
		ui.lWillStatistics->hide();
		ui.lwWillStatistics->hide();
	}
	else if(static_cast<LiveDataSource::WillMessageType> (type) == LiveDataSource::WillMessageType::LastMessage) {
		ui.leWillOwnMessage->hide();
		ui.lWillOwnMessage->hide();
		ui.lWillStatistics->hide();
		ui.lwWillStatistics->hide();
	}
	else if(static_cast<LiveDataSource::WillMessageType> (type) == LiveDataSource::WillMessageType::Statistics) {
		ui.lWillStatistics->show();
		ui.lwWillStatistics->show();
		ui.leWillOwnMessage->hide();
		ui.lWillOwnMessage->hide();
	}
}

void LiveDataDock::willOwnMessageChanged(const QString& message) {
	for (auto* source: m_liveDataSources)
		source->setWillOwnMessage(message);
}

void LiveDataDock::updateTopics() {
	const LiveDataSource* const fds = m_liveDataSources.at(0);
	QVector<QString> topics = fds->topicVector();

	if(!topics.isEmpty()) {
		for(int i = 0; i < topics.count(); i++) {
			ui.cbWillTopic->addItem(topics[i]);
		}
		for (auto* source: m_liveDataSources) {
			source->setWillTopic(ui.cbWillTopic->currentText());
			source->clearLastMessage();
		}
	}
	else
		qDebug()<<"Topic Vector Empty";
}

void LiveDataDock::willUpdateChanged(int updateType) {
	for (auto* source: m_liveDataSources)
		source->setWillUpdateType(static_cast<LiveDataSource::WillUpdateType>(updateType));

	if(static_cast<LiveDataSource::WillUpdateType>(updateType) == LiveDataSource::WillUpdateType::TimePeriod) {
		ui.bWillUpdateNow->hide();
		ui.leWillUpdateInterval->show();
		ui.lWillUpdateInterval->show();

		for (auto* source: m_liveDataSources)
			source->setWillTimeInterval(ui.leWillUpdateInterval->text().toInt());
	}
	else if (static_cast<LiveDataSource::WillUpdateType>(updateType) == LiveDataSource::WillUpdateType::OnClick) {
		ui.bWillUpdateNow->show();
		ui.leWillUpdateInterval->hide();
		ui.lWillUpdateInterval->hide();
	}

}

void LiveDataDock::willUpdateNow() {
	for (auto* source: m_liveDataSources)
		source->setWillForMqtt();
}

void LiveDataDock::willUpdateIntervalChanged(const QString& interval) {
	for (auto* source: m_liveDataSources)
		source->setWillTimeInterval(interval.toInt());
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
			for (auto* source: m_liveDataSources)
				source->addWillStatistics(static_cast<LiveDataSource::WillStatistics>(idx) );
		}
	}
	else {
		if(idx >= 0){
			for (auto* source: m_liveDataSources)
				source->removeWillStatistics(static_cast<LiveDataSource::WillStatistics>(idx) );
		}
	}
}
