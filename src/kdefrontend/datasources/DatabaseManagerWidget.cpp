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

#include <QFileDialog>
#include <QTimer>
#include <QSqlDatabase>

/*!
   \class DatabaseManagerWidget
   \brief widget for managing database connections, embedded in \c DatabaseManagerDialog.

   \ingroup kdefrontend
*/

DatabaseManagerWidget::DatabaseManagerWidget(QWidget* parent) : QWidget(parent) {
	ui.setupUi(this);

	ui.tbAdd->setIcon(KIcon("list-add"));
	ui.tbDelete->setIcon(KIcon("list-remove"));
	ui.bOpen->setIcon(KIcon("document-open"));

	ui.tbAdd->setToolTip(i18n("Add new database connection"));
	ui.tbDelete->setToolTip(i18n("Delete selected database connection"));
	ui.bOpen->setToolTip(i18n("Open database file"));
	ui.bTestConnection->setToolTip(i18n("Test selected database connection"));

	//add the list of supported SQL drivers
	foreach (const QString& driver, QSqlDatabase::drivers())
		ui.cbDriver->addItem(driver);

	//SIGNALs/SLOTs
	connect( ui.bOpen, SIGNAL(clicked()), this, SLOT (selectFile()) );
	connect(ui.cbDriver, SIGNAL(currentIndexChanged(int)), SLOT(driverChanged()) );

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
	// save available database connections
	KConfigGroup conf(KSharedConfig::openConfig(), "DatabaseManager");
	//TODO
}

void DatabaseManagerWidget::driverChanged() {
	//determine whether it's a file database (like SQLite) and hide non-relevant fields (like host name, etc.)
	bool fileDB = ui.cbDriver->currentText() == QLatin1String("QSQLITE");
	ui.lHost->setVisible(!fileDB);
	ui.leHost->setVisible(!fileDB);
	ui.lPort->setVisible(!fileDB);
	ui.sbPort->setVisible(!fileDB);
	ui.bOpen->setVisible(fileDB);
	ui.gbAuthentication->setVisible(!fileDB);
}

void DatabaseManagerWidget::selectFile() {
	KConfigGroup conf(KSharedConfig::openConfig(), "DatabaseManagerWidget");
	QString dir = conf.readEntry("LastDir", "");
	QString path = QFileDialog::getOpenFileName(this, i18n("Select the database file"), dir);
	if (path.isEmpty())
		return; //cancel was clicked in the file-dialog

	int pos = path.lastIndexOf(QDir::separator());
	if (pos != -1) {
		QString newDir = path.left(pos);
		if (newDir != dir)
			conf.writeEntry("LastDir", newDir);
	}

	ui.kleDatabase->setText(path);
}
