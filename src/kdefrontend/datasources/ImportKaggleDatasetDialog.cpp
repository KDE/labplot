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

#include <KConfigGroup>
#include <KWindowConfig>
#include <QDialogButtonBox>
#include <QElapsedTimer>
#include <QProgressBar>
#include <QPushButton>
#include <QStatusBar>
#include <QWindow>

/*!
	\class ImportKaggleDatasetDialog
	\brief Dialog for importing dataset from kaggle.com. Embeds \c ImportKaggleDatasetWidget and provides the standard buttons.

	\ingroup kdefrontend
 */
ImportKaggleDatasetDialog::ImportKaggleDatasetDialog(MainWin* parent)
	: ImportDialog(parent)
	, m_mainWin(parent)
	, m_importKaggleDatasetWidget(new ImportKaggleDatasetWidget(this)) {
	auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Reset);
	okButton = buttonBox->button(QDialogButtonBox::Ok);
	okButton->setEnabled(false);
	m_optionsButton = buttonBox->button(QDialogButtonBox::Reset);
	m_optionsButton->setText(i18n("Toggle Options"));

	connect(m_importKaggleDatasetWidget, &ImportKaggleDatasetWidget::toggleOkBtn, [&](bool enabled) {
		okButton->setEnabled(enabled);
	});
	connect(m_importKaggleDatasetWidget, &ImportKaggleDatasetWidget::toggleOptionsBtn, [&](bool enabled) {
		m_optionsButton->setEnabled(enabled);
	});
	connect(m_importKaggleDatasetWidget, &ImportKaggleDatasetWidget::showError, this, &ImportKaggleDatasetDialog::showErrorMessage);
	connect(m_importKaggleDatasetWidget, &ImportKaggleDatasetWidget::clearError, [&] {
		DEBUG(Q_FUNC_INFO << " clearError");
		showErrorMessage(QString());
	});
	connect(m_optionsButton, &QPushButton::clicked, m_importKaggleDatasetWidget, &ImportKaggleDatasetWidget::toggleOptionsVisibility);
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

	m_optionsButton->setEnabled(false);
	okButton->setEnabled(false);
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

bool ImportKaggleDatasetDialog::checkKaggle() {
	auto group = Settings::group(QStringLiteral("Settings_Datasets"));
	QString kagglePath = group.readEntry(QLatin1String("KaggleCLIPath"), QString());
	if (kagglePath.isEmpty()) {
		kagglePath = QStandardPaths::findExecutable(QStringLiteral("kaggle"));
		if (kagglePath.isEmpty())
			return false;
		else
			group.writeEntry(QLatin1String("KaggleCLIPath"), kagglePath);
	}

	QProcess kaggleCli;
	kaggleCli.setProgram(kagglePath);
	kaggleCli.setArguments({QStringLiteral("--version")});
	kaggleCli.start();

	if (!kaggleCli.waitForFinished(1500))
		return false;

	QRegularExpression re(QStringLiteral(R"(^Kaggle API \d+\.\d+\.\d+$)"));
	kaggleCli.setReadChannel(QProcess::StandardOutput);

	while (kaggleCli.canReadLine()) {
		QString text = QLatin1String(kaggleCli.readLine()).trimmed();
		QRegularExpressionMatch match = re.match(text);
		if (match.hasMatch())
			return true;
	}

	return false;
}