#include "Time.h"

namespace DateTime {
qint64 milliseconds(qint64 hour, qint64 minute, qint64 second, qint64 millisecond) {
	qint64 value = millisecond;
	value += (hour * 3600 + minute * 60 + second) * 1000;
	return value;
}
qint64 createValue(qint64 year, qint64 month, qint64 day, qint64 hour, qint64 minute, qint64 second, qint64 millisecond) {
	return millisecond + 1000 * (second + 60 * (minute + 60 * (hour + 24 * (day + 30 * (month + 12 * year)))));
}
DateTime dateTime(const qint64 value) {
	DateTime dt;
	qint64 divisor = qint64(12) * 30 * 24 * 60 * 60 * 1000;
	qint64 rest;
	dt.year = value / divisor;
	rest = value - dt.year * divisor;
	divisor = qint64(30) * 24 * 60 * 60 * 1000;
	dt.month = rest / divisor;
	rest = rest - dt.month * divisor;
	divisor = qint64(24) * 60 * 60 * 1000;
	dt.day = rest / divisor;
	rest = rest - dt.day * divisor;
	divisor = qint64(60) * 60 * 1000;
	dt.hour = rest / divisor;
	rest -= dt.hour * divisor;
	divisor = qint64(60) * 1000;
	dt.minute = rest / divisor;
	rest -= dt.minute * divisor;
	divisor = qint64(1000);
	dt.second = rest / divisor;
	rest -= dt.second * divisor;
	dt.millisecond = rest;

	return dt;
}
}
