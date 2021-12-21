/*
    File                 : MQTTConnectionManagerDialog.h
    Project              : LabPlot
    Description          : dialog for managing MQTT connections
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2018 Ferencz Kovacs <kferike98@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MQTTCONNECTIONMANAGERDIALOG_H
#define MQTTCONNECTIONMANAGERDIALOG_H

#include <QDialog>

class MQTTConnectionManagerWidget;
class QDialogButtonBox;

class MQTTConnectionManagerDialog : public QDialog {
	Q_OBJECT

public:
	explicit MQTTConnectionManagerDialog(QWidget*, const QString&, bool);
	~MQTTConnectionManagerDialog() override;

	QString connection() const;
	bool initialConnectionChanged() const;

private:
	MQTTConnectionManagerWidget* mainWidget;
	QDialogButtonBox* m_buttonBox;
	bool m_changed{false};
	bool m_initialConnectionChanged;
	QString m_initialConnection;

private Q_SLOTS:
	void changed();
	void save();
};

#endif	// MQTTCONNECTIONMANAGERDIALOG_H
