#ifndef SPARKLINERUNNABLE_H
#define SPARKLINERUNNABLE_H

#include <QObject>
#include <QRunnable>

#include <backend/core/column/Column.h>

class SparkLineRunnable : public QObject, public QRunnable {
	Q_OBJECT
public:
	SparkLineRunnable(Column* col)
		: QObject(nullptr)
		, col(col) {
	}

	void run() override;
	QPixmap getResultPixmap();

Q_SIGNALS:
	void taskFinished(const QPixmap& resultPixmap);

private:
	Column* col;
	QPixmap resultPixmap{QPixmap()};
};

#endif // SPARKLINERUNNABLE_H
