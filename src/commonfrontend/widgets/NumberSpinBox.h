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

#include "backend/lib/Common.h"

#include <QDoubleSpinBox>

class EquationHighlighter;
class QCompleter;

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

	enum class Errors {
		NoErrorNumeric,
		NoErrorExpression,
		NoNumber,
		Invalid,
		Min, // value smaller than min
		Max, // value larger than max
	};

	Q_PROPERTY(bool feedback READ feedback WRITE setFeedback NOTIFY feedbackChanged)

public:
	NumberSpinBox(QWidget* parent = nullptr);
	NumberSpinBox(double initValue, QWidget* parent = nullptr);
	NumberSpinBox(const QString& initExpression, QWidget* parent = nullptr);
	NumberSpinBox(const QString& initExpression, bool feedback, QWidget* parent = nullptr);
	QString errorToString(Errors);
	// bool setExpression(const QString&, bool skipValidation = false);
	QString expression();
	// TODO: rename to scale() to indicate that it will be executed only once
	void setScaling(double);
	bool setValue(double);
	bool setValue(const Common::ExpressionValue&);
	void setFeedback(bool enable);
	bool feedback();
	void setStrongFocus(bool);
	void setClearButtonEnabled(bool);
	double minimum() const;
	void setMinimum(double min);
	double maximum() const;
	void setMaximum(double max);
	const Common::ExpressionValue& value() const;
	template<typename T>
	T numericValue() const {
		return m_value.value<T>();
	}

Q_SIGNALS:
	void valueChanged(const Common::ExpressionValue&);
	void feedbackChanged(bool);

private:
	void init(const QString& initExpression, bool feedback);
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
	Errors validate(QString& input, Common::ExpressionValue& value) const;
	void setText(const QString&);

	bool error(Errors e) const;

	void increaseValue();
	void decreaseValue();
	void valueChanged();

	bool validateExpression(const QString& text, bool force) const;
	void insertCompletion(const QString& completion);
	QString convertToInternalRep(const QString&) const;
	QString convertFromInternalRep(const QString&) const;

private:
	// See https://invent.kde.org/education/labplot/-/merge_requests/167
	// for explanation of the feature
	bool m_feedback{true}; // defines if the spinbox expects a feedback
	bool m_waitFeedback{false};
	bool m_expression{false}; // If the text is an expression or a pure number

	bool m_strongFocus{true};

	Common::ExpressionValue m_value{Common::ExpressionValue(0.0)};

	double m_maximum{std::numeric_limits<double>::max()};
	double m_minimum{std::numeric_limits<double>::lowest()};
	// Everything related to expression handling
	EquationHighlighter* m_highlighter{nullptr};
	QCompleter* m_completer{nullptr};

	friend class WidgetsTest;
};

#endif // NUMBERSPINBOX_H
