#ifndef LABPLOT_TIME_H
#define LABPLOT_TIME_H

#include <QtGlobal>

namespace DateTime {
struct DateTime {
	qint64 year{0};
	qint64 month{0};
	qint64 day{0};
	qint64 hour{0};
	qint64 minute{0};
	qint64 second{0};
	qint64 millisecond{0};
};
qint64 milliseconds(qint64 hour, qint64 minute, qint64 second, qint64 millisecond);
qint64 createValue(qint64 year, qint64 month, qint64 day, qint64 hour, qint64 minute, qint64 second, qint64 millisecond);
DateTime dateTime(const qint64 value);
}

#endif // LABPLOT_TIME_H
