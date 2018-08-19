/***************************************************************************
	File                 : MQTTConnectionManagerDialog.h
	Project              : LabPlot
	Description          : dialog for managing MQTT connections
	--------------------------------------------------------------------
	Copyright            : (C) 2018 Ferencz Kovacs (kferike98@gmail.com)

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
#ifndef MQTTCONNECTIONMANAGERDIALOG_H
#define MQTTCONNECTIONMANAGERDIALOG_H

#include <QDialog>

class MQTTConnectionManagerWidget;
class QDialogButtonBox;

class MQTTConnectionManagerDialog : public QDialog {
#ifdef HAVE_MQTT
	Q_OBJECT

public:
	explicit MQTTConnectionManagerDialog(QWidget*, const QString&);
	~MQTTConnectionManagerDialog();

	QString connection() const;

private:
	MQTTConnectionManagerWidget* mainWidget;
	bool m_changed;
	QDialogButtonBox* m_buttonBox;

private slots:
	void changed();
	void save();
	void loadSettings();
#endif
};

#endif
