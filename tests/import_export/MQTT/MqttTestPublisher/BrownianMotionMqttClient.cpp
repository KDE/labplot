#include "BrownianMotionMqttClient.h"
#include <QMqttTopicName>
#include <memory>
#include <random>

BrownianMotionMqttClient::BrownianMotionMqttClient(QObject* parent, int interval, const QString& hostname, int port, int yPaths)
	: QMqttClient(parent)
	, m_pathes(yPaths)
	, m_interval(interval) {
	setHostname(hostname);
	setPort(port);

	m_x.fill(0.0, m_pathes);

	m_generator = std::make_unique<std::default_random_engine>(m_seed);
	m_distribution = std::make_unique<std::normal_distribution<double>>(0.0, (std::pow(m_delta, 2.0) * m_dt));
}

void BrownianMotionMqttClient::setInterval(int interval) {
	m_interval = interval;
}

QString BrownianMotionMqttClient::subscribeBrownianTopic() {
	if (state() == ClientState::Connected) {
		QMqttTopicFilter filterX{QLatin1String("brownian/x")};

		QMqttSubscription* subscription;
		subscription = subscribe(filterX, m_qos);
		if (!subscription) {
			return QLatin1String("Could not subscribe. Is there a valid connection?");
		}
		m_qos = subscription->qos();
		m_brownian_xTopic = std::make_unique<QMqttTopicName>(subscription->topic().filter());

		for (int i = 0; i < m_pathes; i++) {
			QMqttTopicFilter filterY{QLatin1String("brownian/y") + QString::number(i)};
			subscription = subscribe(filterY, m_qos);
			if (!subscription) {
				return QLatin1String("Could not subscribe. Is there a valid connection?");
			}
			m_brownian_yTopics.push_back(std::move(std::make_unique<QMqttTopicName>(subscription->topic().filter())));
		}
	} else {
		return QLatin1String("Not connected");
	}
	return QLatin1String();
}

QString BrownianMotionMqttClient::publishBrownianData() {
	if (state() == ClientState::Connected && m_brownian_yTopics.size() != 0) {
		QString s;
		QVector<QString> brownianY;
		brownianY.fill(QString(), m_pathes);

		if (m_iterCount < m_itersTotal - m_iters - 1) {
			for (int i = 0; i < m_iters; i++) {
				if (!s.isEmpty())
					s.append(QLatin1String("\n"));

				s.append(QString::number(m_iterCount * m_dt));
				for (int j = 0; j < m_pathes; j++) {
					if (!brownianY[j].isEmpty())
						brownianY[j].append(QLatin1String("\n"));
					m_x[j] = m_x[j] + m_distribution->operator()(*m_generator);
					brownianY[j].append(QString::number(m_x[j]));
				}
				m_iterCount++;
			}
		} else {
			// Restart again
			m_iterCount = 0;
		}

		if (publish(*m_brownian_xTopic, s.toUtf8(), m_qos, false) == -1)
			return QLatin1String("Could not publish message");
		for (int i = 0; i < m_pathes; i++) {
			if (publish(*m_brownian_yTopics.at(i), brownianY[i].toUtf8(), m_qos, false) == -1)
				return QLatin1String("Could not publish message");
		}
	}
	return QLatin1String();
}
