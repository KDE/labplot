/***************************************************************************
	File                 : MQTTWillSettingsWidget.h
	Project              : LabPlot
	Description          : widget for managing MQTT connection's will settings
	--------------------------------------------------------------------
	Copyright            : (C) 2018 by Ferencz Kovacs (kferike98@gmail.com)

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
#ifndef MQTTWILLSETTINGSWIDGET_H
#define MQTTWILLSETTINGSWIDGET_H

#include <QWidget>
#include "ui_mqttwillsettingswidget.h"

#ifdef HAVE_MQTT
#include "backend/datasources/MQTTClient.h"
#endif

class MQTTWillSettingsWidget: public QWidget {
#ifdef HAVE_MQTT
	Q_OBJECT

public:
	explicit MQTTWillSettingsWidget(QWidget*, const MQTTClient::MQTTWill&, const QVector<QString>&);

	MQTTClient::MQTTWill will() const;
	MQTTClient::WillStatisticsType statisticsType() const;

private:
	Ui::MQTTWillSettingsWidget ui;
	bool m_initialising;
	MQTTClient::MQTTWill m_will;

	MQTTClient::WillStatisticsType m_statisticsType;

signals:
	void applyClicked();

private slots:
	void enableWillSettings(int);
	void willMessageTypeChanged(int);
	void loadSettings(const MQTTClient::MQTTWill&, const QVector<QString>&);
	void willUpdateTypeChanged(int);

#endif	// HAVE_MQTT
};

#endif //MQTTWILLSETTINGSWIDGET_H

