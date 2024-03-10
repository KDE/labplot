/*
	File                 : ImportKaggleDialog.cpp
	Project              : LabPlot
	Description          : import kaggle dataset dialog
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Israel Galadima <izzygaladima@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "ImportKaggleDialog.h"
#include "ImportKaggleWidget.h"
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
	\class ImportKaggleDialog
	\brief Dialog for importing dataset from kaggle.com. Embeds \c ImportKaggleWidget and provides the standard buttons.

	\ingroup kdefrontend
 */
ImportKaggleDialog::ImportKaggleDialog(MainWin* parent)
	: ImportDialog(parent)
	, m_mainWin(parent)
	, m_importKaggleWidget(new ImportKaggleWidget(this)) {
	auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	okButton = buttonBox->button(QDialogButtonBox::Ok);
	okButton->setEnabled(false);

	connect(m_importKaggleWidget, &ImportKaggleWidget::toggleOkBtn, [&](bool enabled) {
		okButton->setEnabled(enabled);
	});
	connect(m_importKaggleWidget, &ImportKaggleWidget::showError, this, &ImportKaggleDialog::showErrorMessage);
	connect(m_importKaggleWidget, &ImportKaggleWidget::clearError, [&] {
		DEBUG(Q_FUNC_INFO << " clearError");
		showErrorMessage(QString());
	});
	connect(buttonBox, &QDialogButtonBox::accepted, this, &ImportDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

	vLayout->addWidget(m_importKaggleWidget);
	vLayout->addWidget(buttonBox);

	setWindowTitle(i18nc("@title:window", "Import from kaggle.com"));
	create();

	QApplication::processEvents(QEventLoop::AllEvents, 0);

	KConfigGroup conf = Settings::group(QStringLiteral("ImportKaggleDialog"));
	if (conf.exists()) {
		KWindowConfig::restoreWindowSize(windowHandle(), conf);
		resize(windowHandle()->size());
	} else
		resize(QSize(0, 0).expandedTo(minimumSize()));
}

ImportKaggleDialog::~ImportKaggleDialog() {
	KConfigGroup conf = Settings::group(QStringLiteral("ImportKaggleDialog"));
	KWindowConfig::saveWindowSize(windowHandle(), conf);
}

void ImportKaggleDialog::checkOkButton() {
}

QString ImportKaggleDialog::selectedObject() const {
	return {};
}

bool ImportKaggleDialog::importTo(QStatusBar*) const {
	auto* spreadsheet = new Spreadsheet(i18n("Dataset%1", 1));
	m_importKaggleWidget->importToSpreadsheet(spreadsheet);
	m_mainWin->addAspectToProject(spreadsheet);
	return true;
}
