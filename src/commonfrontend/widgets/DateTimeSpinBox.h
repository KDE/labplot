/*
	File                 : DateTimeSpinBox.h
	Project              : LabPlot
	Description          : widget for setting datetimes with a spinbox
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2019 Martin Marmsoler (martin.marmsoler@gmail.com)
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef DATETIMESPINBOX_H
#define DATETIMESPINBOX_H

#include <QAbstractSpinBox>

class QRegularExpressionValidator;

// Assumption: Month has always 30 days
class DateTimeSpinBox : public QAbstractSpinBox {

	Q_OBJECT;
private:
	enum Type {
		year,
		month,
		day,
		hour,
		minute,
		second,
		millisecond
	};

public:
	explicit DateTimeSpinBox(QWidget* parent);
	void keyPressEvent(QKeyEvent*) override;
	void stepBy(int steps) override;
	QAbstractSpinBox::StepEnabled stepEnabled() const override;
	bool increaseValue(Type type, int step);
	bool changeValue(qint64& thisType, Type nextTypeType, int step);
	Type determineType(int cursorPos) const;
	void writeValue();
	void setValue(qint64 increment);
	qint64 value();
	void getValue();
	void setCursorPosition(Type type);
	bool valid();

private:
	QRegularExpressionValidator *m_regularExpressionValidator;
	qint64 m_year{0}, m_month{0}, m_day{0}, m_hour{0}, m_minute{0}, m_second{0}, m_millisecond{0};

signals:
	void valueChanged();
};

#endif // DATETIMESPINBOX_H
