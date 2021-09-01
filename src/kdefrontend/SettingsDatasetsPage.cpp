/*
    File                 : SettingsDatasetsPage.cpp
    Project              : LabPlot
    Description          : settings page for Datasets
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2019 Alexander Semke (alexander.semke@web.de)

*/

/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#include "SettingsDatasetsPage.h"
#include "backend/lib/macros.h"

#include <KLocalizedString>

#include <QDir>
#include <QMessageBox>
#include <QStandardPaths>

/**
 * \brief Page for the 'General' settings of the Labplot settings dialog.
 */
SettingsDatasetsPage::SettingsDatasetsPage(QWidget* parent) : SettingsPage(parent) {
	ui.setupUi(this);

	ui.bClearCache->setIcon(QIcon::fromTheme(QLatin1String("edit-clear")));
	ui.bClearCache->setToolTip(i18n("Clear downloaded files"));
	ui.bClearCache->setEnabled(false);

	connect(ui.bClearCache, &QPushButton::clicked, this, &SettingsDatasetsPage::clearCache);

	loadSettings();
}

void SettingsDatasetsPage::applySettings() {

}

void SettingsDatasetsPage::restoreDefaults() {
}

void SettingsDatasetsPage::loadSettings() {
	QDir dir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QLatin1String("/datasets_local/"));
	if (dir.exists()) {
		int count = dir.count() - 2; //subtract 2 for . and ..
		ui.lFiles->setText(i18n("Files - %1", count));

		if (count > 0) {
			ui.bClearCache->setEnabled(true);

			//calculate the size
			int size = 0;
			for (auto file : dir.entryList()) {
				if (file == QLatin1Char('.') || file == QLatin1String(".."))
					continue;

				size += QFileInfo(dir, file).size();
			}

			SET_NUMBER_LOCALE
			QString sizeStr;
			if (size > 1024*1024)
				sizeStr = numberLocale.toString(size/1024/1024) + QLatin1String("MB");
			if (size > 1024)
				sizeStr = numberLocale.toString(size/1024) + QLatin1String("kB");
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
		for (auto fileName : dir.entryList()) {
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
