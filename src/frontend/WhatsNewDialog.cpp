/*
	File                 : WhatsNewDialog.cpp
	Project              : LabPlot
	Description          : Dialog showing what's new in the current release
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2025 LabPlot developers
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "WhatsNewDialog.h"
#include "backend/core/Settings.h"

#include <KLocalizedString>
#include <KWindowConfig>

#include <QDialogButtonBox>
#include <QFile>
#include <QPushButton>
#include <QStandardPaths>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <QWindow>

/*!
	\class WhatsNewDialog
	\brief Shows the changelog for the current release on first launch after an upgrade.

	The dialog reads a versioned HTML file from the application data directory
	(e.g. whats-new/whats-new-2.12.80.html). If the file is not found it falls
	back to a link pointing to the online changelog.

	\ingroup frontend
 */
WhatsNewDialog::WhatsNewDialog(QWidget* parent)
	: QDialog(parent) {
	setWindowTitle(i18n("What's New in LabPlot %1", QLatin1String(LVERSION)));
	setAttribute(Qt::WA_DeleteOnClose);

	auto* browser = new QTextBrowser(this);
	browser->setOpenExternalLinks(true);
	browser->setMinimumWidth(500);

	// Try to load the versioned HTML file from the installed data directory
	const QString fileName = QStringLiteral("whats-new/whats-new-") + QLatin1String(LVERSION) + QStringLiteral(".html");
	const QString path = QStandardPaths::locate(QStandardPaths::AppDataLocation, fileName);
	if (!path.isEmpty()) {
		QFile file(path);
		if (file.open(QIODevice::ReadOnly | QIODevice::Text))
			browser->setHtml(QString::fromUtf8(file.readAll()));
	} else {
		// Fallback: link to the online changelog
		const QString url = QStringLiteral("https://labplot.org/changelog/#") + QLatin1String(LVERSION);
		browser->setHtml(QStringLiteral("<p>")
						 + i18n("See the full changelog for LabPlot %1 on our <a href=\"%2\">website</a>.", QLatin1String(LVERSION), url)
						 + QStringLiteral("</p>"));
	}

	auto* btnBox = new QDialogButtonBox(QDialogButtonBox::Close);
	connect(btnBox->button(QDialogButtonBox::Close), &QPushButton::clicked, this, &WhatsNewDialog::accept);

	auto* layout = new QVBoxLayout(this);
	layout->addWidget(browser);
	layout->addWidget(btnBox);

	// restore saved window size
	create(); // ensure there's a window created
	const KConfigGroup conf = Settings::group(QStringLiteral("WhatsNewDialog"));
	if (conf.exists()) {
		KWindowConfig::restoreWindowSize(windowHandle(), conf);
		resize(windowHandle()->size()); // workaround for QTBUG-40584
	} else
		resize(QSize(600, 450).expandedTo(minimumSize()));
}

WhatsNewDialog::~WhatsNewDialog() {
	KConfigGroup conf = Settings::group(QStringLiteral("WhatsNewDialog"));
	KWindowConfig::saveWindowSize(windowHandle(), conf);
}
