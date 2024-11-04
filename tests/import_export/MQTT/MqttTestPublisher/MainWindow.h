/*
	File                 : MQTT_test.h
	Project              : LabPlot
	Description          : Tests for MQTT import.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2018 Kovacs Ferencz <kferike98@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <BrownianMotionMqttClient.h>
#include <QMainWindow>
#include <QTimer>
#include <QVector>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
	Q_OBJECT

public:
	explicit MainWindow(int interval = 1000, QWidget* parent = nullptr);
	~MainWindow();

public Q_SLOTS:
	void setClientPort(int p);

private Q_SLOTS:
	void on_buttonConnect_clicked();
	void on_buttonQuit_clicked();
	void brokerDisconnected();
	void on_buttonSubscribe_clicked();
	void publish();
	void intervalChanged(const QString&);
	void onConnect();

private:
	std::unique_ptr<BrownianMotionMqttClient> m_client;
	Ui::MainWindow* ui;
	bool m_run;
	QTimer* m_timer;
};

#endif // MAINWINDOW_H
