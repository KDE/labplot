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
	enum class Representation { Decimal, Scientific };

public:
	NumberSpinBox(QWidget* parent);

private:
	void keyPressEvent(QKeyEvent*) override;
	void setInvalid(bool invalid, const QString& tooltip);
	double value(bool* ok) const;
	void stepBy(int steps) override;
	QAbstractSpinBox::StepEnabled stepEnabled() const override;
	NumberSpinBox::Representation representation(const QString& value) const;

	void increaseValue();
	void decreaseValue();

Q_SIGNALS:
	void valueChanged(double value);

private:
	bool mInvalid{false};

	friend class WidgetsTest;
};

#endif // NUMBERSPINBOX_H
