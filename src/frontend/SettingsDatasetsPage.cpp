/*
	File                 : SettingsDatasetsPage.cpp
	Project              : LabPlot
	Description          : settings page for Datasets
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2019-2025 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "SettingsDatasetsPage.h"
#include "backend/core/Settings.h"
#include "backend/lib/macros.h"
#include "frontend/GuiTools.h"
#include "frontend/SettingsPage.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <QDir>
#include <QDirIterator>
#include <QMessageBox>
#include <QStandardPaths>

/**
 * \brief Page for the 'General' settings of the Labplot settings dialog.
 */
SettingsDatasetsPage::SettingsDatasetsPage(QWidget* parent)
	: SettingsPage(parent) {
	ui.setupUi(this);

	ui.bClearCache->setIcon(QIcon::fromTheme(QLatin1String("edit-clear")));
	ui.bClearCache->setToolTip(i18n("Clear downloaded files"));
	ui.bClearCache->setEnabled(false);

	ui.lKaggleUrl->setText(QStringLiteral("(<a href=\"https://www.kaggle.com/docs/api\">") + i18n("How to Use Kaggle") + QStringLiteral("</a>)"));
	ui.lKaggleUrl->setTextFormat(Qt::RichText);
	ui.lKaggleUrl->setTextInteractionFlags(Qt::TextBrowserInteraction);
	ui.lKaggleUrl->setOpenExternalLinks(true);

	loadSettings();

	connect(ui.bClearCache, &QPushButton::clicked, this, &SettingsDatasetsPage::clearCache);
	connect(ui.leKagglePath, &QLineEdit::textChanged, [&] {
		m_changed = true;
		const QString& kagglePath = ui.leKagglePath->text();
		bool invalid = (!kagglePath.isEmpty() && !QFile::exists(kagglePath));
		GuiTools::highlight(ui.leKagglePath, invalid);
		Q_EMIT settingsChanged();
	});
}

QList<Settings::Type> SettingsDatasetsPage::applySettings() {
	QList<Settings::Type> changes;
	if (!m_changed)
		return changes;

	KConfigGroup group = Settings::group(QStringLiteral("Settings_Datasets"));
	group.writeEntry(QLatin1String("KaggleCLIPath"), ui.leKagglePath->text());

	changes << Settings::Type::Datasets;
	return changes;
}

void SettingsDatasetsPage::restoreDefaults() {
	ui.leKagglePath->clear();
}

void SettingsDatasetsPage::loadSettings() {
	auto group = Settings::group(QStringLiteral("Settings_Datasets"));
	ui.leKagglePath->setText(group.readEntry(QLatin1String("KaggleCLIPath"), QString()));

	if (ui.leKagglePath->text().isEmpty()) {
		QString kagglePath = QStandardPaths::findExecutable(QStringLiteral("kaggle"));
		if (!kagglePath.isEmpty()) {
			ui.leKagglePath->setText(kagglePath);
			group.writeEntry(QLatin1String("KaggleCLIPath"), ui.leKagglePath->text());
		}
	}

	QDir dir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QLatin1String("/datasets_local/"));
	if (dir.exists()) {
		int count = 0;
		int size = 0;
		QDirIterator dirIterator(dir.path(), QDir::Files, QDirIterator::Subdirectories);
		while (dirIterator.hasNext()) {
			dirIterator.next();
			size += dirIterator.fileInfo().size();
			count++;
		}
		ui.lFiles->setText(i18n("Files - %1", count));

		if (count > 0) {
			ui.bClearCache->setEnabled(true);
		}

		const auto numberLocale = QLocale();
		QString sizeStr;
		if (size > 1024 * 1024)
			sizeStr = numberLocale.toString(size / 1024 / 1024) + QLatin1String("MB");
		else if (size > 1024)
			sizeStr = numberLocale.toString(size / 1024) + QLatin1String("kB");
		else
			sizeStr = numberLocale.toString(size) + QLatin1String("B");

		ui.lSize->setText(i18n("Total size - %1", sizeStr));
	}
}

void SettingsDatasetsPage::clearCache() {
	QDir dir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QLatin1String("/datasets_local/"));
	if (dir.exists()) {
		dir.removeRecursively();
		dir.mkpath(dir.path());

		ui.lFiles->setText(i18n("Files - 0"));
		ui.lSize->setText(i18n("Total size - 0B"));
		ui.bClearCache->setEnabled(false);
		QMessageBox::information(this, i18n("Datasets cache"), i18n("Downloaded files successfully deleted."));
	}
}
