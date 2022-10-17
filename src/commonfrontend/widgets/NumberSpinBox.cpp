#include "NumberSpinBox.h"

#include "backend/lib/macros.h"

#include <QKeyEvent>
#include <QLineEdit>
#include <QString>
#include <QtMath>

NumberSpinBox::NumberSpinBox(QWidget* parent)
	: QAbstractSpinBox(parent) {
}

void NumberSpinBox::keyPressEvent(QKeyEvent* event) {
	lineEdit()->cursorPosition();

	if (mInvalid)
		QAbstractSpinBox::keyPressEvent(event);

	switch (event->key()) {
	case Qt::Key_Down:
		decreaseValue();
		break;
	case Qt::Key_Up:
		increaseValue();
		break;
	default:
		QAbstractSpinBox::keyPressEvent(event);
		break;
	}
	bool ok;
	double v = value(&ok);
	setInvalid(!ok, tr("Invalid number"));
	if (ok)
		emit valueChanged(v);
}

void NumberSpinBox::stepBy(int steps) {
	int cursorPos = lineEdit()->cursorPosition();
	int textLenght = lineEdit()->text().length();

	if (cursorPos == 0)
		return;

    QString v_str = lineEdit()->text().trimmed();

	bool ok;
	double v = v_str.toDouble(&ok);

	if (!ok)
		return;

    const auto comma = v_str.indexOf('.'); // TODO: locale??
    const auto exponentialIndex = v_str.indexOf('e', comma > 0 ? comma : 0, Qt::CaseInsensitive);

    if ((comma >= 0 && cursorPos -1 == comma) || (exponentialIndex > 0 && cursorPos -1 == exponentialIndex))
        return;

    bool before_comma = comma >= 0 && cursorPos - 1 < comma;
    bool before_exponent = exponentialIndex >= 0 && cursorPos -1 < exponentialIndex;

    double increase = 0;
    if (before_comma || (comma == -1 && before_exponent) || (comma == -1 && exponentialIndex == -1)) {
        int initial;
        if (comma >= 0)
            initial = comma;
        else if (exponentialIndex >= 0)
            initial = exponentialIndex;
        else
            initial = textLenght;
        increase = qPow(10, initial - cursorPos);
        v += steps * increase;
    } else if (comma >= 0 && (exponentialIndex == -1 || before_exponent)) {
        increase = qPow(10, - (cursorPos -1 - comma));
        v += steps * increase;
    } else {
        increase = cursorPos - 1 - textLenght;
        v *= steps * increase;
    }

    QString number = QString::number(v);

    lineEdit()->setText(number);
    lineEdit()->setCursorPosition(number.length() - (textLenght - cursorPos));
}

NumberSpinBox::Representation NumberSpinBox::representation(const QString& value) const {
	// Assumption: value is valid

	if (value.contains('e', Qt::CaseInsensitive))
		return Representation::Scientific;
	return Representation::Decimal;
}

void NumberSpinBox::increaseValue() {
	stepBy(1);
}

void NumberSpinBox::decreaseValue() {
	stepBy(-1);
}

QAbstractSpinBox::StepEnabled NumberSpinBox::stepEnabled() const {
	return QAbstractSpinBox::StepEnabledFlag::StepUpEnabled | QAbstractSpinBox::StepEnabledFlag::StepDownEnabled; // for testing
}

void NumberSpinBox::setInvalid(bool invalid, const QString& tooltip) {
	mInvalid = invalid;
	if (invalid) {
		SET_WARNING_PALETTE

		setToolTip(tooltip);
	} else {
		setPalette(qApp->palette());
		setToolTip("");
	}
}

double NumberSpinBox::value(bool* ok) const {
	const auto t = lineEdit()->text();
	const double v = t.toDouble(ok);
	return v;
}
