/***************************************************************************
    File                 : SettingsDatasetsPage.cpp
    Project              : LabPlot
    Description          : settings page for Worksheet
    --------------------------------------------------------------------
    Copyright            : (C) 2008-2017 Alexander Semke (alexander.semke@web.de)

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

#include "SettingsDatasetsPage.h"

#include <QDir>
#include <QMessageBox>
#include <QStandardPaths>

#include <KLocalizedString>

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

			QString sizeStr;
			if (size > 1024*1024)
				sizeStr = QString::number(size/1024/1024) + QLatin1String("MB");
			if (size > 1024)
				sizeStr = QString::number(size/1024) + QLatin1String("kB");
			else
				sizeStr = QString::number(size) + QLatin1String("B");

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

			QFile file(dir.path() + QDir::separator() + fileName);
			file.remove();
		}

		ui.lFiles->setText(i18n("Files - 0"));
		ui.lSize->setText(i18n("Total size - 0B"));
		ui.bClearCache->setEnabled(false);
		QMessageBox::information(this, i18n("Datasets cache"), i18n("Downloaded files successfully deleted."));
	}
}
