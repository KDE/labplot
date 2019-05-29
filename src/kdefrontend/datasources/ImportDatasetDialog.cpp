/***************************************************************************
	File                 : ImportDatasetDialog.cpp
	Project              : LabPlot
	Description          : import dataset data dialog
	--------------------------------------------------------------------
	Copyright            : (C) 2019 Ferencz Koovacs (kferike98@gmail.com)

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

#include "ImportDatasetDialog.h"
#include "ImportDatasetWidget.h"
#include "backend/datasources/DatasetHandler.h"
#include "KConfigGroup"
#include "KSharedConfig"
#include "KWindowConfig"
#include "QWindow"
#include "QProgressBar"
#include "QDialogButtonBox"
#include "QStatusBar"
#include "QDebug"

ImportDatasetDialog::ImportDatasetDialog(MainWin* parent, const QString& fileName) : ImportDialog(parent),
	m_importDatasetWidget(new ImportDatasetWidget(this)){

	qDebug("add dataset widget");
	vLayout->addWidget(m_importDatasetWidget);
	qDebug("Add completed");
	connect(m_importDatasetWidget, &ImportDatasetWidget::datasetSelected, this, &ImportDatasetDialog::checkOkButton);

	//dialog buttons
	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok |QDialogButtonBox::Cancel);
	okButton = buttonBox->button(QDialogButtonBox::Ok);
	okButton->setEnabled(false); //ok is only available if a valid container was selected
	vLayout->addWidget(buttonBox);

	connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

	setWindowTitle(i18nc("@title:window", "Add new Dataset"));
	setWindowIcon(QIcon::fromTheme("document-import-database"));

	create();

	QApplication::processEvents(QEventLoop::AllEvents, 0);

	KConfigGroup conf(KSharedConfig::openConfig(), "ImportDatasetDialog");
	if (conf.exists()) {
		KWindowConfig::restoreWindowSize(windowHandle(), conf);
		resize(windowHandle()->size());
	} else
		resize(QSize(0, 0).expandedTo(minimumSize()));

	checkOkButton();
}

ImportDatasetDialog::~ImportDatasetDialog() {
	KConfigGroup conf(KSharedConfig::openConfig(), "ImportDatasetDialog");
	KWindowConfig::saveWindowSize(windowHandle(), conf);
}

QString ImportDatasetDialog::selectedObject() const {
	return QString();
	//return m_importDatasetWidget->selectedObject();
}

void ImportDatasetDialog::importToDataset(DatasetHandler* datasetHandler, QStatusBar* statusBar) const {
	m_importDatasetWidget->loadDatasetToProcess(datasetHandler);
}

void ImportDatasetDialog::checkOkButton() {
	bool enable = (!m_importDatasetWidget->getSelectedDataset().isEmpty());
	okButton->setEnabled(enable);
}

void ImportDatasetDialog::importTo(QStatusBar* statusBar) const {
	auto filter = m_importDatasetWidget->currentFileFilter();
	//show a progress bar in the status bar
	auto* progressBar = new QProgressBar();
	progressBar->setRange(0, 100);
	connect(filter, &AbstractFileFilter::completed, progressBar, &QProgressBar::setValue);

	statusBar->clearMessage();
	statusBar->addWidget(progressBar, 1);

	WAIT_CURSOR;
	QApplication::processEvents(QEventLoop::AllEvents, 100);

	QTime timer;
	timer.start();

	statusBar->showMessage(i18n("Dataset imported in %1 seconds.", (float)timer.elapsed()/1000));
	RESET_CURSOR;
	statusBar->removeWidget(progressBar);
}

