#include "UTCDateTimeEdit.h"

UTCDateTimeEdit::UTCDateTimeEdit(QWidget* parent)
	: QDateTimeEdit(parent) {
	setMinimumDate(QDate(100, 1, 1));
	setTimeSpec(Qt::TimeSpec::UTC);
	connect(this, &QDateTimeEdit::dateTimeChanged, this, &UTCDateTimeEdit::dateTimeChanged);
}

void UTCDateTimeEdit::setMSecsSinceEpochUTC(qint64 value) {
	QDateTimeEdit::setDateTime(QDateTime::fromMSecsSinceEpoch(value, Qt::UTC));
}

void UTCDateTimeEdit::dateTimeChanged(const QDateTime& datetime) {
	Q_EMIT mSecsSinceEpochUTCChanged(datetime.toMSecsSinceEpoch());
}
