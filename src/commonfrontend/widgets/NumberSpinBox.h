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

#include <QDoubleSpinBox>

class NumberSpinBox : public QDoubleSpinBox {
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
	};

public:
	NumberSpinBox(double initValue = 0, QWidget* parent = nullptr);

private:
	void keyPressEvent(QKeyEvent*) override;
	void setInvalid(bool invalid, const QString& tooltip);
	bool properties(const QString& value, NumberProperties& p) const;
	QString createStringNumber(double integerFraction, int exponent, const NumberProperties&) const;
	void stepBy(int steps) override;
	bool step(int steps);
	QString strip(const QString& t) const;
	virtual QString textFromValue(double value) const override;
	virtual double valueFromText(const QString&) const override;
	QAbstractSpinBox::StepEnabled stepEnabled() const override;
	virtual QValidator::State validate(QString& input, int& pos) const override;
	QValidator::State validate(QString& input, double& value, QString& valueStr) const;
	void setText(const QString&);
	void setValue(double);

	bool increaseValue();
	bool decreaseValue();

Q_SIGNALS:
	void valueChanged(double value);

private:
	QString mValueStr;

	friend class WidgetsTest;
};

#endif // NUMBERSPINBOX_H
