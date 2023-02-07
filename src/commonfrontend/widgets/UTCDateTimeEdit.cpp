#include "UTCDateTimeEdit.h"

UTCDateTimeEdit::UTCDateTimeEdit(QWidget* parent)
	: QDateTimeEdit(parent) {
	connect(this, &QDateTimeEdit::dateTimeChanged, this, &UTCDateTimeEdit::dateTimeChanged);
}

void UTCDateTimeEdit::setMSecsSinceEpochUTC(qint64 value) {
	QDateTimeEdit::setDateTime(QDateTime::fromMSecsSinceEpoch(value, Qt::UTC));
}

void UTCDateTimeEdit::dateTimeChanged(const QDateTime& datetime) {
	QDateTime dt = datetime;
	dt.setTimeSpec(Qt::TimeSpec::UTC);
	Q_EMIT mSecsSinceEpochUTCChanged(dt.toMSecsSinceEpoch());
}
