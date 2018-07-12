/***************************************************************************
    File                 : DatabaseManagerDialog.cc
    Project              : LabPlot
    Description          : dialog for managing database connections
    --------------------------------------------------------------------
    Copyright            : (C) 2016-2017 Alexander Semke (alexander.semke@web.de)

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/

#include "DatabaseManagerDialog.h"
#include "DatabaseManagerWidget.h"

#include <KLocalizedString>
#include <KSharedConfig>
#include <KWindowConfig>

#include <QDialogButtonBox>
#include <QTimer>

/*!
	\class DatabaseManagerDialog
	\brief dialog for managing database connections

	\ingroup kdefrontend
*/
DatabaseManagerDialog::DatabaseManagerDialog(QWidget* parent, const QString& conn) : QDialog(parent),
	mainWidget(new DatabaseManagerWidget(this, conn)), m_changed(false) {

	setWindowIcon(QIcon::fromTheme("network-server-database"));
	setWindowTitle(i18nc("@title:window", "SQL Database Connections"));

	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->addWidget(mainWidget);
	layout->addWidget(buttonBox);

	connect(mainWidget, &DatabaseManagerWidget::changed, this, &DatabaseManagerDialog::changed);
	connect(buttonBox->button(QDialogButtonBox::Ok),&QPushButton::clicked, this, &DatabaseManagerDialog::save);
	connect(buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked, this, &DatabaseManagerDialog::close);
	connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

	QTimer::singleShot(0, this, &DatabaseManagerDialog::loadSettings);
}

void DatabaseManagerDialog::loadSettings() {
	//restore saved settings
	QApplication::processEvents(QEventLoop::AllEvents, 0);
	KConfigGroup conf(KSharedConfig::openConfig(), "DatabaseManagerDialog");
	KWindowConfig::restoreWindowSize(windowHandle(), conf);
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
