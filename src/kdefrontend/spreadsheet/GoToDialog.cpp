/*
    File                 : GoToDialog.cpp
    Project              : LabPlot
    Description          : Dialog to provide the cell coordinates to navigate to
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2020 Alexander Semke (alexander.semke@web.de)
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "GoToDialog.h"
#include "backend/core/column/Column.h"
#include "backend/lib/macros.h"
#include "backend/spreadsheet/Spreadsheet.h"

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
	\class GoToDialog
	\brief Dialog to provide the cell coordinates to navigate to

	\ingroup kdefrontend
 */
GoToDialog::GoToDialog(QWidget* parent) : QDialog(parent) {
	setWindowTitle(i18nc("@title:window", "Go to Cell"));

	auto* layout = new QGridLayout(this);

	//row
	auto* label = new QLabel(i18n("Row:"));
	layout->addWidget(label, 0, 0);

	leRow = new QLineEdit(this);
	leRow->setValidator(new QIntValidator(leRow));
	leRow->setText("1");
	layout->addWidget(leRow, 0, 1);

	//column
	label = new QLabel(i18n("Column:"));
	layout->addWidget(label, 1, 0);

	leColumn = new QLineEdit(this);
	leColumn->setValidator(new QIntValidator(leColumn));
	leColumn->setText("1");
	layout->addWidget(leColumn, 1, 1);

	auto* btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	connect(btnBox, &QDialogButtonBox::accepted, this, &GoToDialog::accept);
	connect(btnBox, &QDialogButtonBox::rejected, this, &GoToDialog::reject);
	layout->addWidget(btnBox, 2, 1);

	//restore saved settings if available
	KConfigGroup conf(KSharedConfig::openConfig(), QLatin1String("GoToDialog"));

	create(); // ensure there's a window created
	if (conf.exists()) {
		KWindowConfig::restoreWindowSize(windowHandle(), conf);
		resize(windowHandle()->size()); // workaround for QTBUG-40584
	} else
		resize(QSize(200, 0).expandedTo(minimumSize()));
}

GoToDialog::~GoToDialog() {
	//save the current settings
	KConfigGroup conf(KSharedConfig::openConfig(), QLatin1String("GoToDialog"));
	KWindowConfig::saveWindowSize(windowHandle(), conf);
}

int GoToDialog::row() {
	SET_NUMBER_LOCALE
	bool ok;
	int row = numberLocale.toInt(leRow->text(), &ok);

	return ok ? row : 0;
}


int GoToDialog::column() {
	SET_NUMBER_LOCALE
	bool ok;
	int col = numberLocale.toInt(leColumn->text(), &ok);
	return ok ? col : 0;
}
