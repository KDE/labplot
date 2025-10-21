/*
	File                 : ImportDatasetDialog.cpp
	Project              : LabPlot
	Description          : import dataset data dialog
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2019 Ferencz Koovacs <kferike98@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ImportDatasetDialog.h"
#include "ImportDatasetWidget.h"
#include "backend/core/Settings.h"
#include "backend/datasources/DatasetHandler.h"
#include "backend/lib/macros.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "frontend/MainWin.h"

#include <QDialogButtonBox>
#include <QElapsedTimer>
#include <QProgressBar>
#include <QPushButton>
#include <QStatusBar>
#include <QWindow>

#include <KConfigGroup>

#include <KWindowConfig>

/*!
	\class ImportDatasetDialog
	\brief Dialog for importing data from a dataset. Embeds \c ImportDatasetWidget and provides the standard buttons.

	\ingroup frontend
 */
ImportDatasetDialog::ImportDatasetDialog(MainWin* parent)
	: ImportDialog(parent)
	, m_importDatasetWidget(new ImportDatasetWidget(this)) {
	vLayout->addWidget(m_importDatasetWidget);
	connect(m_importDatasetWidget, &ImportDatasetWidget::datasetSelected, this, &ImportDatasetDialog::checkOkButton);
	connect(m_importDatasetWidget, &ImportDatasetWidget::datasetDoubleClicked, [this]() {
		checkOkButton();
		if (okButton->isEnabled())
			accept();
	});

	// dialog buttons
	auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	okButton = buttonBox->button(QDialogButtonBox::Ok);
	okButton->setEnabled(false); // ok is only available if a valid container was selected
	vLayout->addWidget(buttonBox);

	connect(buttonBox, &QDialogButtonBox::accepted, this, &ImportDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

	setWindowTitle(i18nc("@title:window", "Import from Dataset Collection"));
	create();

	QApplication::processEvents(QEventLoop::AllEvents, 0);

	KConfigGroup conf = Settings::group(QStringLiteral("ImportDatasetDialog"));
	if (conf.exists()) {
		KWindowConfig::restoreWindowSize(windowHandle(), conf);
		resize(windowHandle()->size());
	} else
		resize(QSize(0, 0).expandedTo(minimumSize()));

	ImportDatasetDialog::checkOkButton();
}

ImportDatasetDialog::~ImportDatasetDialog() {
	KConfigGroup conf = Settings::group(QStringLiteral("ImportDatasetDialog"));
	KWindowConfig::saveWindowSize(windowHandle(), conf);
}

QString ImportDatasetDialog::selectedObject() const {
	// TODO?
	return {};
}

/**
 * @brief Checks whether the OK button of the dialog can be pressed or not
 */
void ImportDatasetDialog::checkOkButton() {
	bool enable = (!m_importDatasetWidget->getSelectedDataset().isEmpty());
	okButton->setEnabled(enable);
}

bool ImportDatasetDialog::importTo(QStatusBar* statusBar) const {
	auto* progressBar = new QProgressBar();
	progressBar->setRange(0, 100);

	auto* spreadsheet = new Spreadsheet(i18n("Dataset%1", 1));
	auto* datasetHandler = new DatasetHandler(spreadsheet);

	int duration = 5000;
	QTimer timer;
	timer.setSingleShot(true);
	QEventLoop loop;
	connect(&timer, &QTimer::timeout, [&] {
		disconnect(datasetHandler, &DatasetHandler::downloadCompleted, nullptr, nullptr);
		disconnect(datasetHandler, &DatasetHandler::error, nullptr, nullptr);
		const_cast<ImportDatasetDialog*>(this)->showErrorMessage(i18n("Failed to connect within %1 seconds", static_cast<float>(duration) / 1000));
		loop.exit(static_cast<int>(Status::FAILURE));
	});
	connect(datasetHandler, &DatasetHandler::error, [&](const QString& message) {
		const_cast<ImportDatasetDialog*>(this)->showErrorMessage(message);
		loop.exit(static_cast<int>(Status::FAILURE));
	});
	connect(datasetHandler, &DatasetHandler::downloadCompleted, [&] {
		m_mainWin->addAspectToProject(spreadsheet);
		loop.exit(static_cast<int>(Status::SUCCESS));
	});
	connect(datasetHandler, &DatasetHandler::downloadProgress, progressBar, &QProgressBar::setValue);

	statusBar->clearMessage();
	statusBar->addWidget(progressBar, 1);

	WAIT_CURSOR;
	QApplication::processEvents(QEventLoop::AllEvents, 100);

	timer.start(duration);
	m_importDatasetWidget->import(datasetHandler);
	int status = loop.exec();

	bool success = status == static_cast<int>(Status::SUCCESS);
	if (success) {
		statusBar->showMessage(i18n("Dataset imported in %1 seconds.", static_cast<float>(duration - timer.remainingTime()) / 1000));
		timer.stop();
	} else
		delete spreadsheet;

	delete datasetHandler;

	RESET_CURSOR;
	statusBar->removeWidget(progressBar);
	return success;
}
