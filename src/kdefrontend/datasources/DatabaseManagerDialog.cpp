/***************************************************************************
    File                 : DatabaseManagerDialog.cc
    Project              : LabPlot
    Description          : dialog for managing database connections
    --------------------------------------------------------------------
    Copyright            : (C) 2016 Alexander Semke (alexander.semke@web.de)

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

/*!
	\class DatabaseManagerDialog
	\brief dialog for managing database connections

	\ingroup kdefrontend
 */

DatabaseManagerDialog::DatabaseManagerDialog(QWidget* parent) : KDialog(parent) {

	DatabaseManagerWidget* mainWidget = new DatabaseManagerWidget(this);
	setMainWidget(mainWidget);

	setWindowIcon(KIcon("network-server-database"));
	setWindowTitle(i18n("SQL Database Connections"));

	setButtons(KDialog::Ok);

	//restore saved settings
	KConfigGroup conf(KSharedConfig::openConfig(), "DatabaseManagerDialog");
	restoreDialogSize(conf);
}

DatabaseManagerDialog::~DatabaseManagerDialog() {
	//save current settings
	KConfigGroup conf(KSharedConfig::openConfig(), "DatabaseManagerDialog");
	saveDialogSize(conf);
}
