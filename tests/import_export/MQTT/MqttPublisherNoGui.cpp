#include <BrownianMotionMqttClient.h>
#include <QCoreApplication>
#include <QDebug>
#include <QThread>

int main(int argc, char* argv[]) {
	QCoreApplication a(argc, argv);

	int interval_ms = 300;

	BrownianMotionMqttClient client(nullptr, interval_ms, QStringLiteral(HOSTNAME), PORT, PATHS);
	client.connectToHost();
	const auto error = client.error();
	if (error != BrownianMotionMqttClient::ClientError::NoError) {
		qDebug() << error;
		exit(-1);
	}

	while (client.state() != QMqttClient::Connected) {
		QCoreApplication::processEvents(QEventLoop::AllEvents, 0);
	}

	const auto& status = client.subscribeBrownianTopic();
	if (!status.isEmpty()) {
		qDebug() << status;
		exit(-2);
	}

	while (1) {
		const auto& status = client.publishBrownianData();
		if (!status.isEmpty()) {
			qDebug() << status;
			exit(-3);
		}
		QThread::msleep(interval_ms);
		QCoreApplication::processEvents(QEventLoop::AllEvents, 0);
	}

	return a.exec();
}
