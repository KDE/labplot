/*
File                 : FITSHeaderEditAddUnitDialog.cpp
Project              : LabPlot
Description          : Widget for adding or modifying FITS header keyword units
--------------------------------------------------------------------
SPDX-FileCopyrightText: 2016-2017 Fabian Kristof (fkristofszabolcs@gmail.com)
*/

/***************************************************************************
*                                                                         *
*  SPDX-License-Identifier: GPL-2.0-or-later
*                                                                         *
***************************************************************************/
#include "FITSHeaderEditAddUnitDialog.h"
#include "backend/datasources/filters/FITSFilter.h"

#include <QDialogButtonBox>
#include <QCompleter>
#include <QPushButton>
#include <QWindow>
#include <QVBoxLayout>

#include <KSharedConfig>
#include <KWindowConfig>

FITSHeaderEditAddUnitDialog::FITSHeaderEditAddUnitDialog(const QString& unit, QWidget* parent) : QDialog(parent) {
	ui.setupUi(this);
	auto* btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	ui.gridLayout->addWidget(btnBox, 1, 0, 1, 2);
	m_okButton = btnBox->button(QDialogButtonBox::Ok);
	m_okButton->setText(i18n("&Add"));

	setWindowTitle(i18nc("@title:window", "Add New Unit"));
	setWindowIcon(QIcon::fromTheme("document-new"));
	m_okButton->setEnabled(false);

	QCompleter* keyCompleter = new QCompleter(FITSFilter::units(), this);
	ui.leUnit->setCompleter(keyCompleter);
	ui.leUnit->setPlaceholderText(i18n("Enter unit name here"));

	connect(ui.leUnit, &QLineEdit::textChanged, this, &FITSHeaderEditAddUnitDialog::unitChanged);
	connect(btnBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked, this, &FITSHeaderEditAddUnitDialog::close);

	connect(btnBox, &QDialogButtonBox::accepted, this, &FITSHeaderEditAddUnitDialog::accept);
	connect(btnBox, &QDialogButtonBox::rejected, this, &FITSHeaderEditAddUnitDialog::reject);

	ui.leUnit->setText(unit);

	//restore saved settings if available
	create(); // ensure there's a window created
	KConfigGroup conf(KSharedConfig::openConfig(), "FITSHeaderEditAddUnitDialog");
	if (conf.exists()) {
		KWindowConfig::restoreWindowSize(windowHandle(), conf);
		resize(windowHandle()->size()); // workaround for QTBUG-40584
	} else
		resize(QSize(300, 0).expandedTo(minimumSize()));
}

FITSHeaderEditAddUnitDialog::~FITSHeaderEditAddUnitDialog() {
	KConfigGroup conf(KSharedConfig::openConfig(), "FITSHeaderEditAddUnitDialog");
	KWindowConfig::saveWindowSize(windowHandle(), conf);
}

QString FITSHeaderEditAddUnitDialog::unit() const {
	QString unit = ui.leUnit->text();
	if (unit.contains(QLatin1Char('(')))
		unit = unit.left(unit.indexOf(QLatin1Char('('))-1);

	return unit;
}

void FITSHeaderEditAddUnitDialog::unitChanged() {
	m_okButton->setEnabled(!ui.leUnit->text().isEmpty());
}
