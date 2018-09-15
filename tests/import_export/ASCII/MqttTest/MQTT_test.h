/***************************************************************************
File                 : MQTT_test.h
Project              : LabPlot
Description          : Tests for MQTT import.
--------------------------------------------------------------------
--------------------------------------------------------------------
Copyright	: (C) 2018 Kovacs Ferencz (kferike98@gmail.com)

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

#ifndef MQTT_TEST_H
#define MQTT_TEST_H

#include <QMainWindow>
#include <QMqttClient>
#include <QMqttTopicName>
#include <QTimer>
#include <QVector>
#include <random>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void setClientPort(int p);

private slots:
    void on_buttonConnect_clicked();
    void on_buttonQuit_clicked();
    void brokerDisconnected();
	void on_buttonSubscribe_clicked();
	void onTimeout();
	void intervalChanged(const QString&);
	void onConnect();

private:
    Ui::MainWindow *ui;
    QMqttClient *m_client;
	bool m_run;
	QMqttTopicName* m_brownianX;
	quint8 m_qos;
	QTimer* m_timer;
	unsigned m_seed;
	std::default_random_engine* m_generator;
	std::normal_distribution<double>* m_distribution;
	double m_delta;
	double m_dt;
	int m_pathes;
	int m_iters;
	int m_iterCount;
	int m_itersTotal;
	int m_interval;

	QVector<QMqttTopicName*> m_brownianTopics;
	QVector<double> m_x;
};

#endif // MQTT_TEST_H
