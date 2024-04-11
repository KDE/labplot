/*
	File                 : SettingsDatasetsPage.cpp
	Project              : LabPlot
	Description          : settings page for Datasets
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2019 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "SettingsDatasetsPage.h"
#include "backend/core/Settings.h"
#include "backend/lib/macros.h"
#include "kdefrontend/GuiTools.h"
#include "kdefrontend/SettingsPage.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <QDir>
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

void SettingsDatasetsPage::applySettings() {
	DEBUG(Q_FUNC_INFO)
	if (!m_changed) {
		return;
	}

	KConfigGroup group = Settings::group(QStringLiteral("Settings_Datasets"));
	group.writeEntry(QLatin1String("KaggleCLIPath"), ui.leKagglePath->text());
}

void SettingsDatasetsPage::restoreDefaults() {
	ui.leKagglePath->clear();
}

void SettingsDatasetsPage::loadSettings() {
	const auto group = Settings::group(QStringLiteral("Settings_Datasets"));
	ui.leKagglePath->setText(group.readEntry(QLatin1String("KaggleCLIPath"), QString()));

	QDir dir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QLatin1String("/datasets_local/"));
	if (dir.exists()) {
		int count = dir.count() - 2; // subtract 2 for . and ..
		ui.lFiles->setText(i18n("Files - %1", count));

		if (count > 0) {
			ui.bClearCache->setEnabled(true);

			// calculate the size
			int size = 0;
			for (auto& file : dir.entryList()) {
				if (file == QLatin1Char('.') || file == QLatin1String(".."))
					continue;

				size += QFileInfo(dir, file).size();
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
		} else
			ui.lSize->setText(i18n("Total size - 0B"));
	}
}

void SettingsDatasetsPage::clearCache() {
	QDir dir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QLatin1String("/datasets_local/"));
	if (dir.exists()) {
		for (auto& fileName : dir.entryList()) {
			if (fileName == QLatin1Char('.') || fileName == QLatin1String(".."))
				continue;

			QFile file(dir.path() + QLatin1String("/") + fileName);
			file.remove();
		}

		ui.lFiles->setText(i18n("Files - 0"));
		ui.lSize->setText(i18n("Total size - 0B"));
		ui.bClearCache->setEnabled(false);
		QMessageBox::information(this, i18n("Datasets cache"), i18n("Downloaded files successfully deleted."));
	}
}
