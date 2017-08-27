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

#include <QTimer>
#include <KLocale>
#include <KSharedConfig>
#include <KWindowConfig>

/*!
	\class DatabaseManagerDialog
	\brief dialog for managing database connections

	\ingroup kdefrontend
*/
DatabaseManagerDialog::DatabaseManagerDialog(QWidget* parent, const QString& conn) : KDialog(parent),
	mainWidget(new DatabaseManagerWidget(this, conn)), m_changed(false) {

	setMainWidget(mainWidget);

	setWindowIcon(QIcon::fromTheme("network-server-database"));
	setWindowTitle(i18n("SQL Database Connections"));

	setButtons(KDialog::Ok | KDialog::Cancel);

	connect(mainWidget, SIGNAL(changed()), this, SLOT(changed()));
	connect(this, SIGNAL(okClicked()), this, SLOT(save()));

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
	setWindowTitle(i18n("SQL Database Connections  [Changed]"));
	m_changed = true;
}

void DatabaseManagerDialog::save() {
	//ok-button was clicked, save the connections if they were changed
	if (m_changed)
		mainWidget->saveConnections();
}
