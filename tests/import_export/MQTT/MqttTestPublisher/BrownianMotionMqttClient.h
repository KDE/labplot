#ifndef BROWNIANMOTIONMQTTCLIENT
#define BROWNIANMOTIONMQTTCLIENT

#include <QMqttClient>
#include <memory>
#include <random>

class BrownianMotionMqttClient : public QMqttClient {
public:
	BrownianMotionMqttClient(QObject* parent = nullptr,
							 int interval = 1000,
							 const QString& hostname = QStringLiteral("test.mosquitto.org"),
							 int port = 1883,
							 int yPaths = 20);
	~BrownianMotionMqttClient() = default;
	QString subscribeBrownianTopic();
	QString publishBrownianData();
	void setInterval(int);

private:
	std::unique_ptr<QMqttTopicName> m_brownian_xTopic;
	std::vector<std::unique_ptr<QMqttTopicName>> m_brownian_yTopics;

	quint8 m_qos{0};
	QVector<double> m_x;

	std::unique_ptr<std::default_random_engine> m_generator;
	std::unique_ptr<std::normal_distribution<double>> m_distribution;
	long m_seed{std::chrono::system_clock::now().time_since_epoch().count()};
	double m_delta{0.25};
	double m_dt{0.1};
	int m_pathes{1};
	int m_iters{300};
	int m_iterCount{0};
	int m_itersTotal{100000};
	int m_interval{1000};
};

#endif // BROWNIANMOTIONMQTTCLIENT
