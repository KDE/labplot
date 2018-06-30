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
			if (ui.cbReadingType->itemText(i) == i18n("Read Whole File")) { // FIXME never ever compare to UI strings!
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
		ui.bPausePlayReading->setText(i18n("Continue Reading"));
		ui.bPausePlayReading->setIcon(QIcon::fromTheme(QLatin1String("media-record")));
	} else {
		continueReading();
		ui.bPausePlayReading->setText(i18n("Pause Reading"));
		ui.bPausePlayReading->setIcon(QIcon::fromTheme(QLatin1String("media-playback-pause")));
	}
}
