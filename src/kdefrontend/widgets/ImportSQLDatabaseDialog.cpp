/***************************************************************************
    File                 : ImportSQLDatabaseDialog.cpp
    Project              : LabPlot
    Description          : import file data dialog
    --------------------------------------------------------------------
    Copyright            : (C) 2016 by Ankit Wagadre (wagadre.ankit@gmail.com)

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

#include "ImportSQLDatabaseDialog.h"
#include "ImportSQLDatabaseWidget.h"
#include "kdefrontend/MainWin.h"

#include <QStatusBar>

/*!
    \class ImportSQLDatabaseDialog
    \brief Dialog for importing data from a file. Embeds \c ImportSQLDatabaseWidget and provides the standard buttons.

	\ingroup kdefrontend
 */

ImportSQLDatabaseDialog::ImportSQLDatabaseDialog(MainWin* parent, Project* project):
	KDialog(parent), m_mainWin(parent), importSQLDatabaseWidget(new ImportSQLDatabaseWidget(parent, project)) {

	QWidget* mainWidget = new QWidget(this);
	vLayout = new QVBoxLayout(mainWidget);
	vLayout->setSpacing(0);
	vLayout->setContentsMargins(0,0,0,0);

	vLayout->addWidget(importSQLDatabaseWidget);
	setMainWidget( mainWidget );

	setButtons( KDialog::Ok | KDialog::Cancel );

	m_statusBar = new QStatusBar();
	vLayout ->addWidget( m_statusBar );

	connect( importSQLDatabaseWidget, SIGNAL(statusChanged(QString)), m_statusBar, SLOT(showMessage(QString)) );
	setCaption(i18n("Import Data to Spreadsheet or Matrix"));
	setWindowIcon(KIcon("document-import-database"));
	setAttribute(Qt::WA_DeleteOnClose);
}

ImportSQLDatabaseDialog::~ImportSQLDatabaseDialog() {
}
