/*
    File                 : DatabaseManagerDialog.cc
    Project              : LabPlot
    Description          : dialog for managing database connections
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016-2019 Alexander Semke (alexander.semke@web.de)
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#include "DatabaseManagerDialog.h"
#include "DatabaseManagerWidget.h"

#include <KLocalizedString>
#include <KSharedConfig>
#include <KWindowConfig>

#include <QDialogButtonBox>
#include <QWindow>

/*!
	\class DatabaseManagerDialog
	\brief dialog for managing database connections

	\ingroup kdefrontend
*/
DatabaseManagerDialog::DatabaseManagerDialog(QWidget* parent, const QString& conn) : QDialog(parent),
	mainWidget(new DatabaseManagerWidget(this, conn)) {

	setWindowIcon(QIcon::fromTheme("network-server-database"));
	setWindowTitle(i18nc("@title:window", "SQL Database Connections"));

	auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	auto* layout = new QVBoxLayout(this);
	layout->addWidget(mainWidget);
	layout->addWidget(buttonBox);

	connect(mainWidget, &DatabaseManagerWidget::changed, this, &DatabaseManagerDialog::changed);
	connect(buttonBox->button(QDialogButtonBox::Ok),&QPushButton::clicked, this, &DatabaseManagerDialog::save);
	connect(buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked, this, &DatabaseManagerDialog::close);
	connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

	//restore saved settings if available
	create(); // ensure there's a window created
	KConfigGroup conf(KSharedConfig::openConfig(), "DatabaseManagerDialog");
	if (conf.exists()) {
		KWindowConfig::restoreWindowSize(windowHandle(), conf);
		resize(windowHandle()->size()); // workaround for QTBUG-40584
	} else
		resize(QSize(0, 0).expandedTo(minimumSize()));
}

QString DatabaseManagerDialog::connection() const {
	return mainWidget->connection();
}

DatabaseManagerDialog::~DatabaseManagerDialog() {
	//save current settings
	KConfigGroup conf(KSharedConfig::openConfig(), "DatabaseManagerDialog");
	KWindowConfig::saveWindowSize(windowHandle(), conf);
}

void DatabaseManagerDialog::changed() {
	setWindowTitle(i18nc("@title:window", "SQL Database Connections  [Changed]"));
	m_changed = true;
}

void DatabaseManagerDialog::save() {
	//ok-button was clicked, save the connections if they were changed
	if (m_changed)
		mainWidget->saveConnections();
}
