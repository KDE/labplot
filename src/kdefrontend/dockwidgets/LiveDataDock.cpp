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

LiveDataDock::LiveDataDock(QWidget* parent) :
	QWidget(parent), m_paused(false) {
	ui.setupUi(this);

	ui.bUpdateNow->setIcon(QIcon::fromTheme(QLatin1String("view-refresh")));

	connect(ui.bPausePlayReading, &QPushButton::clicked, this, &LiveDataDock::pauseContinueReading);
	connect(ui.bUpdateNow, &QPushButton::clicked, this, &LiveDataDock::updateNow);
	connect(ui.sbUpdateInterval, static_cast<void (QSpinBox::*) (int)>(&QSpinBox::valueChanged), this, &LiveDataDock::updateIntervalChanged);

	connect(ui.sbKeepNValues, static_cast<void (QSpinBox::*) (int)>(&QSpinBox::valueChanged), this, &LiveDataDock::keepNValuesChanged);
	connect(ui.sbSampleSize, static_cast<void (QSpinBox::*) (int)>(&QSpinBox::valueChanged), this, &LiveDataDock::sampleSizeChanged);
	connect(ui.cbUpdateType, static_cast<void (QComboBox::*) (int)>(&QComboBox::currentIndexChanged), this, &LiveDataDock::updateTypeChanged);
	connect(ui.cbReadingType, static_cast<void (QComboBox::*) (int)>(&QComboBox::currentIndexChanged), this, &LiveDataDock::readingTypeChanged);

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

	if (fds->sourceType() == LiveDataSource::SourceType::NetworkTcpSocket || fds->sourceType() == LiveDataSource::SourceType::LocalSocket
			|| fds->readingType() == LiveDataSource::ReadingType::TillEnd || fds->readingType() == LiveDataSource::ReadingType::WholeFile) {
		ui.lSampleSize->hide();
		ui.sbSampleSize->hide();
	} else
		ui.sbSampleSize->setValue(fds->sampleSize());
}

/*!
 * \brief Modifies the sample rate of the live data sources
 * \param sampleRate
 */
void LiveDataDock::sampleSizeChanged(int sampleSize) {
	for (auto* source : m_liveDataSources)
		source->setSampleSize(sampleSize);
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

/*!
 * \brief Handles the change of the reading type in the dock widget
 * \param idx
 */
void LiveDataDock::readingTypeChanged(int idx) {
	LiveDataSource::ReadingType type = static_cast<LiveDataSource::ReadingType>(idx);
	const LiveDataSource* const fds = m_liveDataSources.at(0);

	if (fds->sourceType() == LiveDataSource::SourceType::NetworkTcpSocket || fds->sourceType() == LiveDataSource::SourceType::LocalSocket
		|| type == LiveDataSource::ReadingType::TillEnd || type == LiveDataSource::ReadingType::WholeFile) {
		ui.lSampleSize->hide();
		ui.sbSampleSize->hide();
	} else {
		ui.lSampleSize->show();
		ui.sbSampleSize->show();
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
void LiveDataDock::keepNValuesChanged(const int keepNValues) {
	for (auto* source : m_liveDataSources)
		source->setKeepNValues(keepNValues);
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
		ui.bPausePlayReading->setText(i18n("Continue Reading"));
		ui.bPausePlayReading->setIcon(QIcon::fromTheme(QLatin1String("media-record")));
	} else {
		continueReading();
		ui.bPausePlayReading->setText(i18n("Pause Reading"));
		ui.bPausePlayReading->setIcon(QIcon::fromTheme(QLatin1String("media-playback-pause")));
	}
}
