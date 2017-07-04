/***************************************************************************
    File                 : ImportSQLDatabaseDialog.cpp
    Project              : LabPlot
    Description          : import file data dialog
    --------------------------------------------------------------------
    Copyright            : (C) 2016 by Ankit Wagadre (wagadre.ankit@gmail.com)
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

#include "ImportSQLDatabaseDialog.h"
#include "ImportSQLDatabaseWidget.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/lib/macros.h"
#include "kdefrontend/MainWin.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/matrix/Matrix.h"
#include "backend/core/Workbook.h"
#include "commonfrontend/widgets/TreeViewComboBox.h"

#include <QProgressBar>
#include <QStatusBar>

#include <KLocale>
#include <KWindowConfig>

/*!
    \class ImportSQLDatabaseDialog
    \brief Dialog for importing data from a SQL database. Embeds \c ImportSQLDatabaseWidget and provides the standard buttons.

	\ingroup kdefrontend
 */
ImportSQLDatabaseDialog::ImportSQLDatabaseDialog(MainWin* parent) : ImportDialog(parent),
	importSQLDatabaseWidget(new ImportSQLDatabaseWidget(this)) {

	vLayout->addWidget(importSQLDatabaseWidget);

	setButtons( KDialog::Ok | KDialog::Cancel );
	setCaption(i18n("Import Data to Spreadsheet or Matrix"));
	setWindowIcon(QIcon::fromTheme("document-import-database"));

	setModel();

	connect( importSQLDatabaseWidget, SIGNAL(stateChanged()), this, SLOT(importWidgetStateChanged()) );

	//restore saved settings
	KConfigGroup conf(KSharedConfig::openConfig(), "ImportSQLDatabaseDialog");
	KWindowConfig::restoreWindowSize(windowHandle(), conf);
}

ImportSQLDatabaseDialog::~ImportSQLDatabaseDialog() {
	//save current settings
	KConfigGroup conf(KSharedConfig::openConfig(), "ImportSQLDatabaseDialog");
	KWindowConfig::saveWindowSize(windowHandle(), conf);
}

void ImportSQLDatabaseDialog::importTo(QStatusBar* statusBar) const {
	DEBUG("ImportSQLDatabaseDialog::import()");
	AbstractAspect* aspect = static_cast<AbstractAspect*>(cbAddTo->currentModelIndex().internalPointer());
	if (!aspect) {
		DEBUG("ERROR: No aspect available!");
		return;
	}

	AbstractFileFilter::ImportMode mode = AbstractFileFilter::ImportMode(cbPosition->currentIndex());

	//show a progress bar in the status bar
	QProgressBar* progressBar = new QProgressBar();
	progressBar->setMinimum(0);
	progressBar->setMaximum(100);
	connect(importSQLDatabaseWidget, SIGNAL(completed(int)), progressBar, SLOT(setValue(int)));

	statusBar->clearMessage();
	statusBar->addWidget(progressBar, 1);

	WAIT_CURSOR;
	QApplication::processEvents(QEventLoop::AllEvents, 100);

	QTime timer;
	timer.start();
	if (aspect->inherits("Matrix")) {
		Matrix* matrix = qobject_cast<Matrix*>(aspect);
		importSQLDatabaseWidget->read(matrix, mode);
	} else if (aspect->inherits("Spreadsheet")) {
		Spreadsheet* spreadsheet = qobject_cast<Spreadsheet*>(aspect);
		importSQLDatabaseWidget->read(spreadsheet, mode);
	} else if (aspect->inherits("Workbook")) {
		// use active spreadsheet or matrix (only if numeric data is going to be improted) if present,
		// create a new spreadsheet in the selected workbook otherwise
		Workbook* workbook = qobject_cast<Workbook*>(aspect);
		Spreadsheet* spreadsheet = workbook->currentSpreadsheet();
		Matrix* matrix = workbook->currentMatrix();
		if (spreadsheet)
			importSQLDatabaseWidget->read(spreadsheet, mode);
		else if (matrix && importSQLDatabaseWidget->isNumericData())
			importSQLDatabaseWidget->read(matrix, mode);
		else {
			spreadsheet = new Spreadsheet(0, i18n("Spreadsheet"));
			workbook->addChild(spreadsheet);
			importSQLDatabaseWidget->read(spreadsheet, mode);
		}
	}
	statusBar->showMessage( i18n("Data imported in %1 seconds.", (float)timer.elapsed()/1000) );

	RESET_CURSOR;
	statusBar->removeWidget(progressBar);
}

QString ImportSQLDatabaseDialog::selectedObject() const {
	return importSQLDatabaseWidget->selectedTable();
}

void ImportSQLDatabaseDialog::checkOkButton() {
	DEBUG("ImportSQLDatabaseDialog::checkOkButton()");

	AbstractAspect* aspect = static_cast<AbstractAspect*>(cbAddTo->currentModelIndex().internalPointer());
	if (!aspect) {
		enableButtonOk(false);
		cbPosition->setEnabled(false);
		return;
	}

	//check whether a valid connection and an object to import were selected
	if (!importSQLDatabaseWidget->isValid()) {
		enableButtonOk(false);
		cbPosition->setEnabled(false);
		return;
	}

	//for matrix containers allow to import only numerical data
	if (dynamic_cast<const Matrix*>(aspect) && !importSQLDatabaseWidget->isNumericData()) {
		enableButtonOk(false);
		cbPosition->setEnabled(false);
		return;
	}

	enableButtonOk(true);
	cbPosition->setEnabled(true);
}

void ImportSQLDatabaseDialog::importWidgetStateChanged() {
	checkOkButton();
}
