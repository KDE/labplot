/***************************************************************************
    File                 : ImportSQLDatabaseDialog.cpp
    Project              : LabPlot
    Description          : import file data dialog
    --------------------------------------------------------------------
    Copyright            : (C) 2016 by Ankit Wagadre (wagadre.ankit@gmail.com)
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

#include "ImportSQLDatabaseDialog.h"
#include "ImportSQLDatabaseWidget.h"
#include "backend/core/AspectTreeModel.h"
#include "kdefrontend/MainWin.h"

#include <QStatusBar>

/*!
    \class ImportSQLDatabaseDialog
    \brief Dialog for importing data from a SQL database. Embeds \c ImportSQLDatabaseWidget and provides the standard buttons.

	\ingroup kdefrontend
 */
ImportSQLDatabaseDialog::ImportSQLDatabaseDialog(MainWin* parent, AbstractAspect* currentAspect): ImportDialog(parent),
	importSQLDatabaseWidget(new ImportSQLDatabaseWidget(this)) {

	vLayout->addWidget(importSQLDatabaseWidget);

	setButtons( KDialog::Ok | KDialog::Cancel );

	setModel(parent->model(), currentAspect);

// 	connect( importSQLDatabaseWidget, SIGNAL(statusChanged(QString)), m_statusBar, SLOT(showMessage(QString)) );
	setCaption(i18n("Import Data to Spreadsheet or Matrix"));
	setWindowIcon(KIcon("document-import-database"));

	//restore saved settings
	KConfigGroup conf(KSharedConfig::openConfig(), "ImportSQLDatabaseDialog");
	restoreDialogSize(conf);
}

ImportSQLDatabaseDialog::~ImportSQLDatabaseDialog() {
	//save current settings
	KConfigGroup conf(KSharedConfig::openConfig(), "ImportSQLDatabaseDialog");
	saveDialogSize(conf);
}

void ImportSQLDatabaseDialog::import(QStatusBar*) const {
	
}

QString ImportSQLDatabaseDialog::selectedObject() const {
	return QString();
}

void ImportSQLDatabaseDialog::checkOkButton() {
	
}
