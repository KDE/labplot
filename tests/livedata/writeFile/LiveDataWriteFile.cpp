#include <QFile>
#include <QString>
#include <QThread>
#include <cmath>

int main() {
	QFile f(QStringLiteral(EXPORT_FILE));
	const QString header = QStringLiteral("x,cos,sin\n");

	double x = 0;
	while (1) {
		QString content;
		if (!f.open(QIODevice::WriteOnly))
			return -1;

		for (int i = 0; i < FILE_NUMBER_LINES; i++) {
			// generating lines
			const auto cos = std::cos(x);
			const auto sin = std::sin(x);
			x += 0.01;

			content += QStringLiteral("%1,%2,%3\n").arg(x, 0, 'f').arg(cos).arg(sin);
		}

		f.write(header.toLatin1() + content.toLatin1());
		f.flush();
		f.close();

		QThread::msleep(SLEEP_TIME_MS);
	}
}
