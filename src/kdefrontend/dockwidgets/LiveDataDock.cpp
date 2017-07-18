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

LiveDataDock::LiveDataDock(QWidget *parent) :
	QWidget(parent), m_paused(false) {
	ui.setupUi(this);

	connect(ui.bPausePlayReading, SIGNAL(clicked(bool)), this, SLOT(pauseContinueReading()));
	connect(ui.bUpdateNow, SIGNAL(clicked(bool)), this, SLOT(updateNow()));
	connect(ui.sbUpdateInterval, SIGNAL(valueChanged(int)), this, SLOT(updateIntervalChanged(int)));
	connect(ui.leKeepNValues, SIGNAL(textChanged(QString)), this, SLOT(keepNvaluesChanged(QString)));
	connect(ui.sbSampleRate, SIGNAL(valueChanged(int)), this, SLOT(sampleRateChanged(int)));
	connect(ui.cbUpdateType, SIGNAL(currentIndexChanged(int)), this, SLOT(updateTypeChanged(int)));
}

LiveDataDock::~LiveDataDock() {
}

/*!
 * \brief Sets the live data sources of this dock widget
 * \param sources
 */
void LiveDataDock::setLiveDataSources(const QList<FileDataSource *> &sources) {
	m_liveDataSources = sources;
	const FileDataSource* const fds = sources.at(0);
	ui.sbUpdateInterval->setValue(fds->updateInterval());
	ui.cbUpdateType->setCurrentIndex(static_cast<int>(fds->updateType()));

	if (fds->updateType() == FileDataSource::UpdateType::NewData) {
		ui.lUpdateInterval->hide();
		ui.sbUpdateInterval->hide();
	}

	if(!fds->keepLastValues()) {
		ui.leKeepNValues->hide();
		ui.lKeepNvalues->hide();
	} else {
		ui.leKeepNValues->setValidator(new QIntValidator(2, 100000));
		ui.leKeepNValues->setText(QString::number(fds->keepNvalues()));
	}

	if (fds->readingType() == FileDataSource::ReadingType::TillEnd) {
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
	FileDataSource::UpdateType type = static_cast<FileDataSource::UpdateType>(idx);

	if (type == FileDataSource::UpdateType::TimeInterval) {
		ui.lUpdateInterval->show();
		ui.sbUpdateInterval->show();

		for (auto* source: m_liveDataSources) {
			source->setUpdateType(type);
			source->setUpdateInterval(ui.sbUpdateInterval->value());
			source->setFileWatched(false);
		}
	} else if (type == FileDataSource::UpdateType::NewData) {
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
	FileDataSource::ReadingType type = static_cast<FileDataSource::ReadingType>(idx);

	if (type == FileDataSource::ReadingType::TillEnd) {
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
void LiveDataDock::keepNvaluesChanged(QString keepNvalues) {
	for (auto* source : m_liveDataSources)
		source->setKeepNvalues(keepNvalues.toInt());
}

/*!
 * \brief Stops the reading of the live data source
 */
void LiveDataDock::stopReading() {
	for (auto* source: m_liveDataSources)
		source->stopReading();
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
		ui.bPausePlayReading->setText("Continue reading");
	} else {
		continueReading();
		ui.bPausePlayReading->setText("Pause reading");
	}
}
