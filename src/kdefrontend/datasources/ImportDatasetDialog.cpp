/*
	File                 : ImportDatasetDialog.cpp
	Project              : LabPlot
	Description          : import dataset data dialog
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2019 Ferencz Koovacs (kferike98@gmail.com)
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#include "ImportDatasetDialog.h"
#include "ImportDatasetWidget.h"
#include "backend/datasources/DatasetHandler.h"
#include "backend/lib/macros.h"

#include <QDialogButtonBox>
#include <QElapsedTimer>
#include <QProgressBar>
#include <QPushButton>
#include <QStatusBar>
#include <QWindow>

#include <KConfigGroup>
#include <KSharedConfig>
#include <KWindowConfig>

/*!
	\class ImportDatasetDialog
	\brief Dialog for importing data from a dataset. Embeds \c ImportDatasetWidget and provides the standard buttons.

	\ingroup kdefrontend
 */
ImportDatasetDialog::ImportDatasetDialog(MainWin* parent) : ImportDialog(parent),
	m_importDatasetWidget(new ImportDatasetWidget(this)){

	vLayout->addWidget(m_importDatasetWidget);
	connect(m_importDatasetWidget, &ImportDatasetWidget::datasetSelected, this, &ImportDatasetDialog::checkOkButton);
	connect(m_importDatasetWidget, &ImportDatasetWidget::datasetDoubleClicked, [this]() {
		checkOkButton();
		if(okButton->isEnabled())
			accept();
	});

	//dialog buttons
	auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok |QDialogButtonBox::Cancel);
	okButton = buttonBox->button(QDialogButtonBox::Ok);
	okButton->setEnabled(false); //ok is only available if a valid container was selected
	vLayout->addWidget(buttonBox);

	connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

	setWindowTitle(i18nc("@title:window", "Import from Dataset Collection"));
	create();

	QApplication::processEvents(QEventLoop::AllEvents, 0);

	KConfigGroup conf(KSharedConfig::openConfig(), "ImportDatasetDialog");
	if (conf.exists()) {
		KWindowConfig::restoreWindowSize(windowHandle(), conf);
		resize(windowHandle()->size());
	} else
		resize(QSize(0, 0).expandedTo(minimumSize()));

	ImportDatasetDialog::checkOkButton();
}

ImportDatasetDialog::~ImportDatasetDialog() {
	KConfigGroup conf(KSharedConfig::openConfig(), "ImportDatasetDialog");
	KWindowConfig::saveWindowSize(windowHandle(), conf);
}

QString ImportDatasetDialog::selectedObject() const {
	return QString();
}

/*!
  triggers the import of a dataset's data
*/
void ImportDatasetDialog::importToDataset(DatasetHandler* datasetHandler, QStatusBar* statusBar) const {
	//show a progress bar in the status bar
	auto* progressBar = new QProgressBar();
	progressBar->setRange(0, 100);
	connect(datasetHandler, &DatasetHandler::downloadProgress, progressBar, &QProgressBar::setValue);

	statusBar->clearMessage();
	statusBar->addWidget(progressBar, 1);

	WAIT_CURSOR;
	QApplication::processEvents(QEventLoop::AllEvents, 100);

	QElapsedTimer timer;
	timer.start();

	m_importDatasetWidget->import(datasetHandler);

	statusBar->showMessage(i18n("Dataset imported in %1 seconds.", static_cast<float>(timer.elapsed())/1000));
	RESET_CURSOR;
	statusBar->removeWidget(progressBar);
}

/**
 * @brief Checks whether the OK button of the dialog can be pressed or not
 */
void ImportDatasetDialog::checkOkButton() {
	bool enable = (!m_importDatasetWidget->getSelectedDataset().isEmpty());
	okButton->setEnabled(enable);
}

void ImportDatasetDialog::importTo(QStatusBar* statusBar) const {
	Q_UNUSED(statusBar);
}
