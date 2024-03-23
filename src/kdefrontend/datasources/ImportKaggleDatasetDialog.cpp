/*
	File                 : ImportKaggleDatasetDialog.cpp
	Project              : LabPlot
	Description          : import kaggle dataset dialog
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Israel Galadima <izzygaladima@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "ImportKaggleDatasetDialog.h"
#include "ImportKaggleDatasetWidget.h"
#include "backend/core/Settings.h"
#include "backend/lib/macros.h"

#include <QDialogButtonBox>
#include <QElapsedTimer>
#include <QProgressBar>
#include <QPushButton>
#include <QStatusBar>
#include <QWindow>

#include <KConfigGroup>

#include <KWindowConfig>

/*!
	\class ImportKaggleDatasetDialog
	\brief Dialog for importing dataset from kaggle.com. Embeds \c ImportKaggleDatasetWidget and provides the standard buttons.

	\ingroup kdefrontend
 */
ImportKaggleDatasetDialog::ImportKaggleDatasetDialog(MainWin* parent)
	: ImportDialog(parent)
	, m_mainWin(parent)
	, m_importKaggleDatasetWidget(new ImportKaggleDatasetWidget(this)) {
	auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	okButton = buttonBox->button(QDialogButtonBox::Ok);
	okButton->setEnabled(false);

	connect(m_importKaggleDatasetWidget, &ImportKaggleDatasetWidget::toggleOkBtn, [&](bool enabled) {
		okButton->setEnabled(enabled);
	});
	connect(m_importKaggleDatasetWidget, &ImportKaggleDatasetWidget::showError, this, &ImportKaggleDatasetDialog::showErrorMessage);
	connect(m_importKaggleDatasetWidget, &ImportKaggleDatasetWidget::clearError, [&] {
		DEBUG(Q_FUNC_INFO << " clearError");
		showErrorMessage(QString());
	});
	connect(buttonBox, &QDialogButtonBox::accepted, this, &ImportDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

	vLayout->addWidget(m_importKaggleDatasetWidget);
	vLayout->addWidget(buttonBox);

	setWindowTitle(i18nc("@title:window", "Import from kaggle.com"));
	create();

	QApplication::processEvents(QEventLoop::AllEvents, 0);

	KConfigGroup conf = Settings::group(QStringLiteral("ImportKaggleDatasetDialog"));
	if (conf.exists()) {
		KWindowConfig::restoreWindowSize(windowHandle(), conf);
		resize(windowHandle()->size());
	} else
		resize(QSize(0, 0).expandedTo(minimumSize()));
}

ImportKaggleDatasetDialog::~ImportKaggleDatasetDialog() {
	KConfigGroup conf = Settings::group(QStringLiteral("ImportKaggleDatasetDialog"));
	KWindowConfig::saveWindowSize(windowHandle(), conf);
}

void ImportKaggleDatasetDialog::checkOkButton() {
}

QString ImportKaggleDatasetDialog::selectedObject() const {
	return {};
}

bool ImportKaggleDatasetDialog::importTo(QStatusBar*) const {
	auto* spreadsheet = new Spreadsheet(i18n("Dataset%1", 1));
	m_importKaggleDatasetWidget->importToSpreadsheet(spreadsheet);
	m_mainWin->addAspectToProject(spreadsheet);
	return true;
}
