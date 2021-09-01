/*
    File                 : AddValueLabelDialog.cpp
    Project              : LabPlot
    Description          : Dialog to add a new the value label
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2021 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "AddValueLabelDialog.h"
#include "backend/lib/macros.h"

#include <QDateTimeEdit>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QWindow>

#include <KLocalizedString>
#include <KSharedConfig>
#include <KWindowConfig>

/*!
	\class AddValueLabelDialog
	\brief Dialog to add a new the value label

	\ingroup kdefrontend
 */
AddValueLabelDialog::AddValueLabelDialog(QWidget* parent, AbstractColumn::ColumnMode mode) : QDialog(parent) {
	setWindowTitle(i18nc("@title:window", "Add Value Label"));

	auto* layout = new QGridLayout(this);

	//value
	auto* label = new QLabel(i18n("Value:"));
	layout->addWidget(label, 0, 0);

	if (mode == AbstractColumn::ColumnMode::Numeric
		|| mode == AbstractColumn::ColumnMode::Integer
		|| mode == AbstractColumn::ColumnMode::BigInt) {

		leValue = new QLineEdit(this);
		leValue->setFocus();
		layout->addWidget(leValue, 0, 1);

		if (mode == AbstractColumn::ColumnMode::Numeric) {
			SET_NUMBER_LOCALE
			leValue->setLocale(numberLocale);
			leValue->setValidator(new QDoubleValidator(leValue));
		} else if (mode == AbstractColumn::ColumnMode::Integer
			|| mode == AbstractColumn::ColumnMode::BigInt)
			leValue->setValidator(new QIntValidator(leValue));
	} else {
		dateTimeEdit = new QDateTimeEdit(this);
		dateTimeEdit->setFocus();
		layout->addWidget(dateTimeEdit, 0, 1);
	}

	//label
	label = new QLabel(i18n("Label:"));
	layout->addWidget(label, 1, 0);

	leLabel = new QLineEdit(this);
	layout->addWidget(leLabel, 1, 1);

	auto* btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	connect(btnBox, &QDialogButtonBox::accepted, this, &AddValueLabelDialog::accept);
	connect(btnBox, &QDialogButtonBox::rejected, this, &AddValueLabelDialog::reject);
	layout->addWidget(btnBox, 2, 1);

	//restore saved settings if available
	KConfigGroup conf(KSharedConfig::openConfig(), QLatin1String("AddValueLabelDialog"));

	create(); // ensure there's a window created
	if (conf.exists()) {
		KWindowConfig::restoreWindowSize(windowHandle(), conf);
		resize(windowHandle()->size()); // workaround for QTBUG-40584
	} else
		resize(QSize(200, 0).expandedTo(minimumSize()));
}

AddValueLabelDialog::~AddValueLabelDialog() {
	//save the current settings
	KConfigGroup conf(KSharedConfig::openConfig(), QLatin1String("AddValueLabelDialog"));
	KWindowConfig::saveWindowSize(windowHandle(), conf);
}

void AddValueLabelDialog::setDateTimeFormat(const QString& format) {
	if (dateTimeEdit)
		dateTimeEdit->setDisplayFormat(format);
}

double AddValueLabelDialog::value() const {
	SET_NUMBER_LOCALE
	bool ok;
	double value = numberLocale.toDouble(leValue->text(), &ok);

	return ok ? value : 0.0;
}

int AddValueLabelDialog::valueInt() const {
	SET_NUMBER_LOCALE
	bool ok;
	int value = numberLocale.toInt(leValue->text(), &ok);

	return ok ? value : 0;
}

qint64 AddValueLabelDialog::valueBigInt() const {
	SET_NUMBER_LOCALE
	bool ok;
	qint64 value = numberLocale.toLongLong(leValue->text(), &ok);

	return ok ? value : 0;
}

QString AddValueLabelDialog::valueText() const {
	return leValue->text();
}

QDateTime AddValueLabelDialog::valueDateTime() const {
	return dateTimeEdit->dateTime();
}

QString AddValueLabelDialog::label() const {
	return leLabel->text();
}
