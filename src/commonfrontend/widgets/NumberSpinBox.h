/*
	File                 : NumberSpinBox.h
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
		bool groupSeparators;
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

	enum class Errors {
		NoError,
		NoNumber,
		Invalid,
		Min, // value smaller than min
		Max, // value larger than max
	};

	Q_PROPERTY(bool feedback READ feedback WRITE setFeedback NOTIFY feedbackChanged)

public:
	explicit NumberSpinBox(QWidget* parent = nullptr);
	explicit NumberSpinBox(double initValue, QWidget* parent = nullptr);
	explicit NumberSpinBox(double initValue, bool feedback, QWidget* parent = nullptr);
	QString errorToString(Errors);
	bool setValue(double);
	void setFeedback(bool enable);
	bool feedback();
	void setStrongFocus(bool);
	double value();
	void setClearButtonEnabled(bool);
	double minimum() const;
	void setMinimum(double min);
	double maximum() const;
	void setMaximum(double max);

Q_SIGNALS:
	void valueChanged(double);
	void feedbackChanged(bool);

private:
	void init(double initValue, bool feedback);
	void keyPressEvent(QKeyEvent*) override;
	void wheelEvent(QWheelEvent* event) override;
	void setInvalid(Errors e);
	void setInvalid(const QString& str);
	bool properties(const QString& value, NumberProperties& p) const;
	QString createStringNumber(double integerFraction, int exponent, const NumberProperties&) const;
	void stepBy(int steps) override;
	Errors step(int steps);
	QString strip(const QString& t) const;
	virtual QString textFromValue(double value) const override;
	virtual double valueFromText(const QString&) const override;
	QAbstractSpinBox::StepEnabled stepEnabled() const override;
	virtual QValidator::State validate(QString& input, int& pos) const override;
	Errors validate(QString& input, double& value, QString& valueStr) const;
	void setText(const QString&);

	void increaseValue();
	void decreaseValue();
	void valueChanged();

private:
	QString m_valueStr;
	// See https://invent.kde.org/education/labplot/-/merge_requests/167
	// for explanation of the feature
	bool m_feedback{true}; // defines if the spinbox expects a feedback
	bool m_waitFeedback{false};

	bool m_strongFocus{true};

	// The value stored in QAbstractSpinBox is rounded to
	// decimals and this is not desired
	double m_value{0};
	double m_maximum{std::numeric_limits<double>::max()};
	double m_minimum{std::numeric_limits<double>::lowest()};

	friend class SpinBoxTest;
};

#endif // NUMBERSPINBOX_H
