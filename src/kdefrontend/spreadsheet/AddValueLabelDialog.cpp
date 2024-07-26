/*
	File                 : AddValueLabelDialog.cpp
	Project              : LabPlot
	Description          : Dialog to add a new the value label
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2021-2022 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "AddValueLabelDialog.h"
#include "backend/core/Settings.h"
#include "backend/core/column/Column.h"
#include "backend/lib/macros.h"

#include <QComboBox>
#include <QDateTimeEdit>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QWindow>

#include <KLocalizedString>

#include <KWindowConfig>

/*!
	\class AddValueLabelDialog
	\brief Dialog to add a new the value label

	\ingroup kdefrontend
 */
AddValueLabelDialog::AddValueLabelDialog(QWidget* parent, const Column* column)
	: QDialog(parent) {
	setWindowTitle(i18nc("@title:window", "Add Value Label"));

	auto mode = column->columnMode();
	auto* layout = new QGridLayout(this);

	// value
	auto* l = new QLabel(i18n("Value:"));
	layout->addWidget(l, 0, 0);

	if (mode == AbstractColumn::ColumnMode::Double || mode == AbstractColumn::ColumnMode::Integer || mode == AbstractColumn::ColumnMode::BigInt) {
		leValue = new QLineEdit(this);
		leValue->setFocus();
		layout->addWidget(leValue, 0, 1);

		if (mode == AbstractColumn::ColumnMode::Double) {
			leValue->setLocale(QLocale());
			auto* validator = new QDoubleValidator(leValue);
			validator->setLocale(QLocale());
			leValue->setValidator(validator);
		} else if (mode == AbstractColumn::ColumnMode::Integer || mode == AbstractColumn::ColumnMode::BigInt)
			leValue->setValidator(new QIntValidator(leValue));
	} else if (mode == AbstractColumn::ColumnMode::Text) {
		cbValue = new QComboBox(this);

		// show all unique text values in the combobox
		QStringList items;
		QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
		const auto& frequencies = column->frequencies();
		auto i = frequencies.constBegin();
		while (i != frequencies.constEnd()) {
			items << i.key();
			++i;
		}

		cbValue->addItems(items);
		QApplication::restoreOverrideCursor();

		cbValue->setFocus();
		layout->addWidget(cbValue, 0, 1);

	} else {
		dateTimeEdit = new QDateTimeEdit(this);
		dateTimeEdit->setFocus();
		layout->addWidget(dateTimeEdit, 0, 1);
	}

	// label
	l = new QLabel(i18n("Label:"));
	layout->addWidget(l, 1, 0);

	leLabel = new QLineEdit(this);
	layout->addWidget(leLabel, 1, 1);

	auto* btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	connect(btnBox, &QDialogButtonBox::accepted, this, &AddValueLabelDialog::accept);
	connect(btnBox, &QDialogButtonBox::rejected, this, &AddValueLabelDialog::reject);
	layout->addWidget(btnBox, 2, 1);

	// restore saved settings if available
	KConfigGroup conf = Settings::group(QLatin1String("AddValueLabelDialog"));

	create(); // ensure there's a window created
	if (conf.exists()) {
		KWindowConfig::restoreWindowSize(windowHandle(), conf);
		resize(windowHandle()->size()); // workaround for QTBUG-40584
	} else
		resize(QSize(200, 0).expandedTo(minimumSize()));
}

AddValueLabelDialog::~AddValueLabelDialog() {
	// save the current settings
	KConfigGroup conf = Settings::group(QLatin1String("AddValueLabelDialog"));
	KWindowConfig::saveWindowSize(windowHandle(), conf);
}

void AddValueLabelDialog::setDateTimeFormat(const QString& format) {
	if (dateTimeEdit)
		dateTimeEdit->setDisplayFormat(format);
}

double AddValueLabelDialog::value() const {
	bool ok;
	double value = QLocale().toDouble(leValue->text(), &ok);

	return ok ? value : 0.0;
}

int AddValueLabelDialog::valueInt() const {
	bool ok;
	const int v = QLocale().toInt(leValue->text(), &ok);

	return ok ? v : 0;
}

qint64 AddValueLabelDialog::valueBigInt() const {
	bool ok;
	const qint64 v = QLocale().toLongLong(leValue->text(), &ok);

	return ok ? v : 0;
}

QString AddValueLabelDialog::valueText() const {
	return cbValue->currentText();
}

QDateTime AddValueLabelDialog::valueDateTime() const {
	return dateTimeEdit->dateTime();
}

QString AddValueLabelDialog::label() const {
	return leLabel->text();
}
