/*
	File                 : NumberSpinBox.g
	Project              : LabPlot
	Description          : widget for setting numbers with a spinbox
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef NUMBERSPINBOX_H
#define NUMBERSPINBOX_H

#include <QAbstractSpinBox>

class NumberSpinBox : public QAbstractSpinBox {
	Q_OBJECT

public:
	struct NumberProperties {
		QChar integerSign;
		int integer;
		int intergerDigits;

		bool fraction; // 5. is a valid number, so just setting fractionDigits to 0 is not correct
		int fractionPos;
		int fractionDigits;

		QChar exponentLetter;
		int exponentPos;
		QChar exponentSign;
		int exponent;
		int exponentDigits;

		double value;
	};

public:
	NumberSpinBox(QWidget* parent);

private:
	void keyPressEvent(QKeyEvent*) override;
	void setInvalid(bool invalid, const QString& tooltip);
	double value(bool* ok = nullptr) const;
	static bool properties(const QString& value, NumberProperties& p);
	static QString createStringNumber(double integerFraction, int exponent, const NumberProperties& p);
	void stepBy(int steps) override;
	bool step(int steps);
	QAbstractSpinBox::StepEnabled stepEnabled() const override;

	bool increaseValue();
	bool decreaseValue();

Q_SIGNALS:
	void valueChanged(double value);

private:
	bool mInvalid{false};

	friend class WidgetsTest;
};

#endif // NUMBERSPINBOX_H
