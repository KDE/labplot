#ifndef UTCDATETIMEEDIT_H
#define UTCDATETIMEEDIT_H

#include <QDateTimeEdit>

class UTCDateTimeEdit : public QDateTimeEdit {
	Q_OBJECT
public:
	UTCDateTimeEdit(QWidget* parent = nullptr);
	void setMSecsSinceEpochUTC(qint64);

Q_SIGNALS:
	void mSecsSinceEpochUTCChanged(qint64);

private:
	// so it is not visible from outside
	void setDateTime(const QDateTime&) {
	}
	// For testing purpose only
	QLineEdit* lineEdit() const {
		return QDateTimeEdit::lineEdit();
	}
private Q_SLOTS:
	void dateTimeChanged(const QDateTime&);

	friend class WorksheetElementTest;
};

#endif // UTCDATETIMEEDIT_H
