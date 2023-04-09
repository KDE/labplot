/*
	File                 : ImportSQLDatabaseDialog.cpp
	Project              : LabPlot
	Description          : import SQL dataase dialog
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2016 Ankit Wagadre <wagadre.ankit@gmail.com>
	SPDX-FileCopyrightText: 2016-2017 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ImportSQLDatabaseDialog.h"
#include "ImportSQLDatabaseWidget.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/core/Workbook.h"
#include "backend/lib/macros.h"
#include "backend/matrix/Matrix.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "commonfrontend/widgets/TreeViewComboBox.h"
#include "kdefrontend/MainWin.h"

#include <QDialogButtonBox>
#include <QElapsedTimer>
#include <QProgressBar>
#include <QStatusBar>
#include <QWindow>

#include <KLocalizedString>
#include <KSharedConfig>
#include <KWindowConfig>

/*!
	\class ImportSQLDatabaseDialog
	\brief Dialog for importing data from a SQL database. Embeds \c ImportSQLDatabaseWidget and provides the standard buttons.

	\ingroup kdefrontend
 */
ImportSQLDatabaseDialog::ImportSQLDatabaseDialog(MainWin* parent)
	: ImportDialog(parent)
	, importSQLDatabaseWidget(new ImportSQLDatabaseWidget(this)) {
	vLayout->addWidget(importSQLDatabaseWidget);

	setWindowTitle(i18nc("@title:window", "Import Data to Spreadsheet or Matrix"));
	setWindowIcon(QIcon::fromTheme(QStringLiteral("document-import-database")));
	setModel();

	// dialog buttons
	auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	okButton = buttonBox->button(QDialogButtonBox::Ok);
	okButton->setEnabled(false); // ok is only available if a valid container was selected
	vLayout->addWidget(buttonBox);

	// Signals/Slots
	connect(importSQLDatabaseWidget, &ImportSQLDatabaseWidget::stateChanged, this, &ImportSQLDatabaseDialog::checkOkButton);
	connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

	// restore saved settings if available
	create(); // ensure there's a window created
	KConfigGroup conf(KSharedConfig::openConfig(), "ImportSQLDatabaseDialog");
	if (conf.exists()) {
		KWindowConfig::restoreWindowSize(windowHandle(), conf);
		resize(windowHandle()->size()); // workaround for QTBUG-40584
	} else
		resize(QSize(0, 0).expandedTo(minimumSize()));
}

ImportSQLDatabaseDialog::~ImportSQLDatabaseDialog() {
	// save current settings
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

	const auto mode = AbstractFileFilter::ImportMode(cbPosition->currentIndex());

	// show a progress bar in the status bar
	auto* progressBar = new QProgressBar();
	progressBar->setMinimum(0);
	progressBar->setMaximum(100);
	connect(importSQLDatabaseWidget, &ImportSQLDatabaseWidget::completed, progressBar, &QProgressBar::setValue);

	statusBar->clearMessage();
	statusBar->addWidget(progressBar, 1);

	WAIT_CURSOR;
	QApplication::processEvents(QEventLoop::AllEvents, 100);

	QElapsedTimer timer;
	timer.start();
	if (aspect->inherits(AspectType::Matrix)) {
		auto* matrix = qobject_cast<Matrix*>(aspect);
		importSQLDatabaseWidget->read(matrix, mode);
	} else if (aspect->inherits(AspectType::Spreadsheet)) {
		auto* spreadsheet = qobject_cast<Spreadsheet*>(aspect);
		importSQLDatabaseWidget->read(spreadsheet, mode);
	} else if (aspect->inherits(AspectType::Workbook)) {
		// use active spreadsheet or matrix (only if numeric data is going to be imported) if present,
		// create a new spreadsheet in the selected workbook otherwise
		auto* workbook = qobject_cast<Workbook*>(aspect);
		Spreadsheet* spreadsheet = workbook->currentSpreadsheet();
		Matrix* matrix = workbook->currentMatrix();
		if (spreadsheet)
			importSQLDatabaseWidget->read(spreadsheet, mode);
		else if (matrix && importSQLDatabaseWidget->isNumericData())
			importSQLDatabaseWidget->read(matrix, mode);
		else {
			spreadsheet = new Spreadsheet(i18n("Spreadsheet"));
			workbook->addChild(spreadsheet);
			importSQLDatabaseWidget->read(spreadsheet, mode);
		}
	}
	statusBar->showMessage(i18n("Data imported in %1 seconds.", (float)timer.elapsed() / 1000));

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
		okButton->setEnabled(false);
		okButton->setToolTip(i18n("Select a data container where the data has to be imported into."));
		cbPosition->setEnabled(false);
		return;
	}

	// check whether a valid connection and an object to import were selected
	if (!importSQLDatabaseWidget->isValid()) {
		okButton->setEnabled(false);
		okButton->setToolTip(i18n("Select a valid database object (table or query result set) that has to be imported."));
		cbPosition->setEnabled(false);
		return;
	}

	// for matrix containers allow to import only numerical data
	if (dynamic_cast<const Matrix*>(aspect) && !importSQLDatabaseWidget->isNumericData()) {
		okButton->setEnabled(false);
		okButton->setToolTip(i18n("Cannot import into a matrix since the data contains non-numerical data."));
		cbPosition->setEnabled(false);
		return;
	}

	okButton->setEnabled(true);
	okButton->setToolTip(i18n("Close the dialog and import the data."));
	cbPosition->setEnabled(true);
}
