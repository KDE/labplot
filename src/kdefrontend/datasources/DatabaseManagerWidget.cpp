/***************************************************************************
File                 : DatabaseManagerWidget.cpp
Project              : LabPlot
Description          : widget for managing database connections
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

#include "DatabaseManagerWidget.h"

#include <QTimer>

/*!
   \class DatabaseManagerWidget
   \brief widget for managing database connections, embedded in \c DatabaseManagerDialog.

   \ingroup kdefrontend
*/

DatabaseManagerWidget::DatabaseManagerWidget(QWidget* parent) : QWidget(parent) {
	ui.setupUi(this);

	ui.tbAdd->setIcon(KIcon("list-add"));
	ui.tbDelete->setIcon(KIcon("list-remove"));

	ui.tbAdd->setToolTip(i18n("Add new database connection"));
	ui.tbDelete->setToolTip(i18n("Delete selected database connection"));
	ui.pbTestConnection->setToolTip(i18n("Test selected database connection"));

	QTimer::singleShot( 100, this, SLOT(loadSettings()) );
}

/*!
	read and show all available database connections
 */
void DatabaseManagerWidget::loadSettings() {
	KConfigGroup conf(KSharedConfig::openConfig(),"DatabaseManager");
	//TODO
}

DatabaseManagerWidget::~DatabaseManagerWidget() {
	// save current settings
	KConfigGroup conf(KSharedConfig::openConfig(), "DatabaseManager");
	//TODO
}
