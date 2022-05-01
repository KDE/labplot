/*
    File                 : MQTT_test.cpp
    Project              : LabPlot
    Description          : Tests for MQTT import.
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2018 Kovacs Ferencz <kferike98@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "MQTT_test.h"
#include "ui_MQTT_test.h"

#include <QDateTime>
#include <QMqttClient>
#include <QMessageBox>
#include <QtMath>
#include <QTimer>

#include <iostream>
#include <chrono>
#include <random>

MainWindow::MainWindow(QWidget *parent) :
		QMainWindow(parent),
		ui(new Ui::MainWindow),
		m_run (false),
		m_pathes (1),
		m_itersTotal (100000),
		m_iters (300),
		m_iterCount (0),
		m_delta (0.25),
		m_dt (0.1),
		m_interval (1000),
		m_seed (std::chrono::system_clock::now().time_since_epoch().count()) {
	ui->setupUi(this);

	m_timer = new QTimer(this);
	m_timer->setInterval(m_interval);

	m_generator = new std::default_random_engine (m_seed);
	m_distribution = new std::normal_distribution<double> (0.0, (qPow(m_delta, 2.0) * m_dt));

	m_x.fill(0.0, m_pathes);

	m_client = new QMqttClient(this);
	m_client->setHostname(ui->lineEditHost->text());
	m_client->setPort(ui->spinBoxPort->value());

	connect(m_client, &QMqttClient::disconnected, this, &MainWindow::brokerDisconnected);
	connect(m_timer, &QTimer::timeout, this, &MainWindow::onTimeout);
	connect(ui->lePublishInterval, &QLineEdit::textChanged, this, &MainWindow::intervalChanged);
	connect(ui->bPublish, &QPushButton::clicked, this, &MainWindow::onTimeout);
	connect(m_client, &QMqttClient::connected, this, &MainWindow::onConnect);

	connect(m_client, &QMqttClient::messageReceived, this, [this](const QByteArray &message, const QMqttTopicName &topic) {
		const QString content = QDateTime::currentDateTime().toString()
				+ QLatin1String(" Received Topic: ")
				+ topic.name()
				+ QLatin1String(" Message:\n")
				+ message
				+ QLatin1Char('\n');
		ui->editLog->insertPlainText(content);
	});

	connect(ui->lineEditHost, &QLineEdit::textChanged, m_client, &QMqttClient::setHostname);
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
	if(m_client->state() == QMqttClient::ClientState::Connected) {
		QMqttTopicFilter filterX{"brownian/x"};

		QMqttSubscription* subscription;
		subscription = m_client->subscribe(filterX , ui->spinQoS->text().toUInt());
		if (!subscription) {
			QMessageBox::critical(this, QLatin1String("Error"), QLatin1String("Could not subscribe. Is there a valid connection?"));
			return;
		}
		m_qos = subscription->qos();
		m_brownianX = new QMqttTopicName(subscription->topic().filter()) ;

		for (int i = 0; i < m_pathes; i++) {
			QMqttTopicFilter filterY{"brownian/y"+QString::number(i)};
			subscription = m_client->subscribe(filterY , ui->spinQoS->text().toUInt());
			if (!subscription) {
				QMessageBox::critical(this, QLatin1String("Error"), QLatin1String("Could not subscribe. Is there a valid connection?"));
				return;
			}
			m_brownianTopics.push_back(new QMqttTopicName(subscription->topic().filter())) ;
		}
		m_timer->start();
	}
}

void MainWindow::onTimeout() {
	if(m_client->state() == QMqttClient::ClientState::Connected &&
			!m_brownianTopics.isEmpty()) {
		QString s;
		QVector<QString> brownianY;
		brownianY.fill(QString(), m_pathes);

		if (m_iterCount < m_itersTotal - m_iters - 1)
			for (int i = 0; i < m_iters; i++) {
				if (!s.isEmpty())
					s.append("\n");

				s.append(QString::number(m_iterCount * m_dt));
				for (int j = 0; j < m_pathes; j++) {
					if(!brownianY[j].isEmpty())
						brownianY[j].append("\n");
					m_x[j] = m_x[j] + m_distribution->operator() (*m_generator);
					brownianY[j].append(QString::number(m_x[j]));
				}
				m_iterCount++;
			}

		if (m_client->publish(*m_brownianX,
							  s.toUtf8(),
							  m_qos,
							  false) == -1)
			QMessageBox::critical(this, QLatin1String("Error"), QLatin1String("Could not publish message"));
		for (int i = 0; i < m_pathes; i++) {
			if (m_client->publish(*m_brownianTopics[i],
								  brownianY[i].toUtf8(),
								  m_qos,
								  false) == -1)
				QMessageBox::critical(this, QLatin1String("Error"), QLatin1String("Could not publish message"));
		}
	}
}

void MainWindow::intervalChanged(const QString& interval) {
	int newInterval = interval.toInt();
	m_timer->start(newInterval);
	m_interval = newInterval;
}

void MainWindow::onConnect() {
	const QString content = QDateTime::currentDateTime().toString() +"  Successfully connected to broker";
	ui->editLog->insertPlainText(content);
}
