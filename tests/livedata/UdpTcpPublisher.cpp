#include <QCoreApplication>
#include <QDebug>
#include <QThread>
#include <QUdpSocket>
#include <QTimer>

int main(int argc, char* argv[]) {
	QCoreApplication a(argc, argv);

	int udpNewDataUpdateTimeMs = PUBLISH_TIME_MS;
	int port = PORT;
	QString hostname = QStringLiteral(HOSTNAME);

	// initialize the UDP socket
	QUdpSocket udpSocket(&a);
	if (!udpSocket.bind(QHostAddress(hostname), port)) {
		exit(-3);
		return -3;
	}
	QTimer timer(&a);
	QCoreApplication::connect(&timer, &QTimer::timeout, [&udpSocket, port, hostname]() {
		udpSocket.writeDatagram("1,2", QHostAddress(hostname), port);
	});
	timer.start(udpNewDataUpdateTimeMs);

	return a.exec();
}
