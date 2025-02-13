#include <QCoreApplication>
#include <QDebug>
#include <QThread>
#include <QTimer>
#include <QUdpSocket>

int main(int argc, char* argv[]) {
	QCoreApplication a(argc, argv);

	int udpNewDataUpdateTimeMs = PUBLISH_TIME_MS;
	int udpPort = UDP_PORT;
	QString hostname = QStringLiteral(HOSTNAME);

	// initialize the UDP socket
	QUdpSocket udpSocket(&a);
	if (!udpSocket.bind(QHostAddress(hostname), 56080)) {
		exit(-3);
		return -3;
	}
	QTimer timer(&a);
	QCoreApplication::connect(&timer, &QTimer::timeout, [&udpSocket, udpPort, hostname]() {
		udpSocket.writeDatagram("1,2", QHostAddress(hostname), udpPort);
	});
	timer.start(udpNewDataUpdateTimeMs);

	return a.exec();
}
