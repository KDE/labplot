/*
	File                 : MQTT_test.cpp
	Project              : LabPlot
	Description          : Tests for MQTT import.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2018 Kovacs Ferencz <kferike98@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QDateTime>
#include <QMessageBox>
#include <QMqttClient>
#include <QTimer>

#include <chrono>
#include <iostream>
#include <random>

MainWindow::MainWindow(int interval, QWidget* parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
	, m_run(false) {
	ui->setupUi(this);

	m_timer = new QTimer(this);
	m_timer->setInterval(interval);

	m_client = std::make_unique<BrownianMotionMqttClient>(this, interval);
	ui->lineEditHost->setText(m_client->hostname());

	connect(m_client.get(), &QMqttClient::disconnected, this, &MainWindow::brokerDisconnected);
	connect(m_timer, &QTimer::timeout, this, &MainWindow::publish);
	connect(ui->lePublishInterval, &QLineEdit::textChanged, this, &MainWindow::intervalChanged);
	connect(ui->bPublish, &QPushButton::clicked, this, &MainWindow::publish);
	connect(m_client.get(), &QMqttClient::connected, this, &MainWindow::onConnect);

	connect(m_client.get(), &QMqttClient::messageReceived, this, [this](const QByteArray& message, const QMqttTopicName& topic) {
		const QString content = QDateTime::currentDateTime().toString() + QLatin1String(" Received Topic: %1").arg(topic.name())
			+ QLatin1String(" Message:\n%1\n").arg(QString::fromUtf8(message));
		ui->editLog->insertPlainText(content);
	});

	connect(ui->lineEditHost, &QLineEdit::textChanged, m_client.get(), &QMqttClient::setHostname);
	connect(ui->spinBoxPort, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::setClientPort);
}

MainWindow::~MainWindow() {
	delete ui;
	qApp->quit();
}

void MainWindow::on_buttonConnect_clicked() {
	if (m_client->state() == QMqttClient::Disconnected) {
		ui->lineEditHost->setEnabled(false);
		ui->spinBoxPort->setEnabled(false);
		ui->buttonConnect->setText(tr("Disconnect"));
		m_client->connectToHost();
	} else {
		ui->lineEditHost->setEnabled(true);
		ui->spinBoxPort->setEnabled(true);
		ui->buttonConnect->setText(tr("Connect"));
		m_client->disconnectFromHost();
	}
}

void MainWindow::on_buttonQuit_clicked() {
	QApplication::quit();
}

void MainWindow::brokerDisconnected() {
	ui->lineEditHost->setEnabled(true);
	ui->spinBoxPort->setEnabled(true);
	ui->buttonConnect->setText(tr("Connect"));
}

void MainWindow::setClientPort(int p) {
	m_client->setPort(p);
}

void MainWindow::on_buttonSubscribe_clicked() {
	const auto& status = m_client->subscribeBrownianTopic();
	if (!status.isEmpty())
		QMessageBox::critical(this, QLatin1String("Error"), status);
	else
		m_timer->start();
}

void MainWindow::publish() {
	m_client->publishBrownianData();
}

void MainWindow::intervalChanged(const QString& interval) {
	int newInterval = interval.toInt();
	m_timer->start(newInterval);
	m_client->setInterval(newInterval);
}

void MainWindow::onConnect() {
	const QString content = QDateTime::currentDateTime().toString() + QLatin1String("  Successfully connected to broker\n");
	ui->editLog->insertPlainText(content);
}
