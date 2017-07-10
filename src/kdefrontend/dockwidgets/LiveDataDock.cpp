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

}

LiveDataDock::~LiveDataDock() {
}

/*!
 * \brief Sets the live data sources of this dock widget
 * \param sources
 */
void LiveDataDock::setLiveDataSources(const QList<FileDataSource *> &sources) {
    m_liveDataSources = sources;
    ui.sbSampleRate->setValue(m_liveDataSources.at(0)->sampleRate());
    ui.sbUpdateFrequency->setValue(m_liveDataSources.at(0)->updateFrequency());
    ui.cbUpdateType->setCurrentIndex(static_cast<int>(m_liveDataSources.at(0)->updateType()));
}

/*!
 * \brief Modifies the sample rate of the live data sources
 * \param sampleRate
 */
void LiveDataDock::sampleRateChanged(int sampleRate) {
    for (auto* source : m_liveDataSources) {
        source->setSampleRate(sampleRate);
    }
}

/*!
 * \brief Updates the live data sources now
 */
void LiveDataDock::updateNow() {
    for (auto* source : m_liveDataSources) {
        source->updateNow();
    }
}

/*!
 * \brief LiveDataDock::updateTypeChanged
 * \param idx
 */
void LiveDataDock::updateTypeChanged(int idx) {
    FileDataSource::UpdateType type = static_cast<FileDataSource::UpdateType>(idx);

    if (type == FileDataSource::UpdateType::TimeInterval) {
        ui.lUpdateFrequency->show();
        ui.sbUpdateFrequency->show();

        for (auto* source: m_liveDataSources) {
            source->setUpdateType(type);
            source->setUpdateFrequency(ui.sbUpdateFrequency->value());
        }

    } else if (type == FileDataSource::UpdateType::NewData) {
        ui.lUpdateFrequency->hide();
        ui.sbUpdateFrequency->hide();

        for (auto* source: m_liveDataSources) {
            source->setUpdateType(type);
        }
    }
}

/*!
 * \brief Modifies the update frequency of the live data sources
 * \param updateFrequency
 */
void LiveDataDock::updateFrequencyChanged(int updateFrequency) {
    for (auto* source : m_liveDataSources) {
        source->setUpdateFrequency(updateFrequency);
    }
}

/*!
 * \brief Modifies the number of samples to keep in each of the live data sources
 * \param keepNvalues
 */
void LiveDataDock::keepNvaluesChanged(int keepNvalues) {
    for (auto* source : m_liveDataSources) {
        source->setKeepNvalues(keepNvalues);
    }
}

/*!
 * \brief Stops the reading of the live data source
 */
void LiveDataDock::stopReading() {
    for (auto* source: m_liveDataSources) {
        source->stopReading();
    }
}

/*!
 * \brief Pauses the reading of the live data source
 */
void LiveDataDock::pauseReading() {
    for (auto* source: m_liveDataSources) {
        source->pauseReading();
    }
}

/*!
 * \brief Continues the reading of the live data source
 */
void LiveDataDock::continueReading() {
    for (auto* source: m_liveDataSources) {
        source->continueReading();
    }
}

/*!
 * \brief Handles the pausing/continuing of reading of the live data source
 */
void LiveDataDock::pauseContinueReading() {
    m_paused = !m_paused;

    if (m_paused) {
        pauseReading();
    } else {
        continueReading();
    }
}
