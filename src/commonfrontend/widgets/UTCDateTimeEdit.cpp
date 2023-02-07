#include "UTCDateTimeEdit.h"

UTCDateTimeEdit::UTCDateTimeEdit(QWidget* parent)
	: QDateTimeEdit(parent) {
	connect(this, &QDateTimeEdit::dateTimeChanged, this, &UTCDateTimeEdit::dateTimeChanged);
}

void UTCDateTimeEdit::setMSecsSinceEpochUTC(qint64 value) {
	QDateTimeEdit::setDateTime(QDateTime::fromMSecsSinceEpoch(value, Qt::UTC));
}

void UTCDateTimeEdit::dateTimeChanged(QDateTime dt) {
	dt.setTimeSpec(Qt::TimeSpec::UTC);
	Q_EMIT mSecsSinceEpochUTCChanged(dt.toMSecsSinceEpoch());
}

void UTCDateTimeEdit::setDateTime(const QDateTime&) {
	// not used function
	// just to make it private
}
