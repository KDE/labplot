/*
    File                 : RescaleDialog.h
    Project              : LabPlot
    Description          : Dialog to provide the rescale interval
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2020 Alexander Semke <alexander.semke@web.de>


    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "RescaleDialog.h"
#include "backend/core/column/Column.h"
#include "backend/lib/macros.h"

#include <QDoubleValidator>
#include <QPushButton>
#include <QWindow>

#include <KWindowConfig>
#include <KSharedConfig>

/*!
	\class RescaleDialog
	\brief Dialog to provide the rescale interval for the select columns in the spreadsheet.

	\ingroup kdefrontend
 */
RescaleDialog::RescaleDialog(QWidget* parent) : QDialog(parent) {
	setWindowIcon(QIcon::fromTheme("view-sort-ascending"));
	setWindowTitle(i18nc("@title:window", "Rescale Interval"));
	setSizeGripEnabled(true);

	ui.setupUi(this);

	ui.buttonBox->button(QDialogButtonBox::Ok)->setText(i18n("Rescale"));
	ui.lInterval->setText(i18n("Interval [a, b]:"));
	ui.lInfo->setText(i18n("More than one column selected. The same interval will be applied to <i>all</i> columns."));

	ui.leMin->setValidator(new QDoubleValidator(ui.leMin));
	ui.leMax->setValidator(new QDoubleValidator(ui.leMax));

	connect(ui.buttonBox, &QDialogButtonBox::rejected, this, &RescaleDialog::reject);
	connect(ui.buttonBox, &QDialogButtonBox::accepted, this, &RescaleDialog::accept);
	connect(ui.leMin, &QLineEdit::textChanged, this, &RescaleDialog::validateOkButton);
	connect(ui.leMax, &QLineEdit::textChanged, this, &RescaleDialog::validateOkButton);

	//restore saved settings if available
	create(); // ensure there's a window created
	KConfigGroup conf(KSharedConfig::openConfig(), QLatin1String("RescaleDialog"));
	if (conf.exists()) {
		KWindowConfig::restoreWindowSize(windowHandle(), conf);
		resize(windowHandle()->size()); // workaround for QTBUG-40584
	} else
		resize(QSize(300, 0).expandedTo(minimumSize()));

	double min = conf.readEntry(QLatin1String("Min"), 0.0);
	double max = conf.readEntry(QLatin1String("Max"), 1.0);
	SET_NUMBER_LOCALE
	ui.leMin->setText(numberLocale.toString(min));
	ui.leMax->setText(numberLocale.toString(max));

	validateOkButton();
}

RescaleDialog::~RescaleDialog() {
	//save the current settings
	KConfigGroup conf(KSharedConfig::openConfig(), QLatin1String("RescaleDialog"));
	KWindowConfig::saveWindowSize(windowHandle(), conf);

	// general settings
	SET_NUMBER_LOCALE
	conf.writeEntry(QLatin1String("Min"), numberLocale.toDouble(ui.leMin->text()));
	conf.writeEntry(QLatin1String("Max"), numberLocale.toDouble(ui.leMax->text()));
}

void RescaleDialog::setColumns(const QVector<Column*>& columns) {
	ui.lInfo->setVisible(columns.size() > 1);
}

double RescaleDialog::min() const {
	SET_NUMBER_LOCALE
	return numberLocale.toDouble(ui.leMin->text());
}

double RescaleDialog::max() const {
	SET_NUMBER_LOCALE
	return numberLocale.toDouble(ui.leMax->text());
}

void RescaleDialog::validateOkButton() {
	const QString& min = ui.leMin->text().simplified();
	const QString& max = ui.leMax->text().simplified();
	bool valid = !min.isEmpty() && !max.isEmpty() && min != max;
	ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(valid);
}
