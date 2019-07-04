/***************************************************************************
    File                 : SettingsDialog.cpp
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008-2019 by Alexander Semke (alexander.semke@web.de)
    Description          : application settings dialog

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
#include "SettingsDialog.h"

#include "MainWin.h"
#include "SettingsGeneralPage.h"
#include "SettingsWorksheetPage.h"

#include <QPushButton>
#include <QDialogButtonBox>
#include <QWindow>

#include <KMessageBox>
#include <KConfigGroup>
#include <KSharedConfig>
#include <KWindowConfig>
#include <KI18n/KLocalizedString>

/**
 * \brief Settings dialog for Labplot.
 *
 * Contains the pages for general settings and view settings.
 *
 */
SettingsDialog::SettingsDialog(QWidget* parent) : KPageDialog(parent) {
	setFaceType(List);
	setWindowTitle(i18nc("@title:window", "Preferences"));
	setWindowIcon(QIcon::fromTheme("preferences-other"));
	setAttribute(Qt::WA_DeleteOnClose);

	buttonBox()->addButton(QDialogButtonBox::Apply)->setEnabled(false);
	buttonBox()->addButton(QDialogButtonBox::RestoreDefaults);
	connect(buttonBox(), &QDialogButtonBox::clicked, this, &SettingsDialog::slotButtonClicked);

	m_generalPage = new SettingsGeneralPage(this);
	KPageWidgetItem* generalFrame = addPage(m_generalPage, i18n("General"));
	generalFrame->setIcon(QIcon::fromTheme("system-run"));
	connect(m_generalPage, &SettingsGeneralPage::settingsChanged, this, &SettingsDialog::changed);
	connect(m_generalPage, &SettingsGeneralPage::resetWelcomeScreen, this, &SettingsDialog::resetWelcomeScreen);

	m_worksheetPage = new SettingsWorksheetPage(this);
	KPageWidgetItem* worksheetFrame = addPage(m_worksheetPage, i18n("Worksheet"));
	worksheetFrame->setIcon(QIcon::fromTheme(QLatin1String("labplot-worksheet")));
	connect(m_worksheetPage, &SettingsWorksheetPage::settingsChanged, this, &SettingsDialog::changed);

	//restore saved settings if available
	create(); // ensure there's a window created
	KConfigGroup conf(KSharedConfig::openConfig(), "SettingsDialog");
	if (conf.exists()) {
		KWindowConfig::restoreWindowSize(windowHandle(), conf);
		resize(windowHandle()->size()); // workaround for QTBUG-40584
	} else
		resize(QSize(0, 0).expandedTo(minimumSize()));
}

SettingsDialog::~SettingsDialog() {
	KConfigGroup dialogConfig = KSharedConfig::openConfig()->group("SettingsDialog");
	KWindowConfig::saveWindowSize(windowHandle(), dialogConfig);
}

void SettingsDialog::slotButtonClicked(QAbstractButton* button) {
	if ((button == buttonBox()->button(QDialogButtonBox::Ok)) || (button == buttonBox()->button(QDialogButtonBox::Apply))) {
		if (m_changed) {
			applySettings();
			setWindowTitle(i18nc("@title:window", "Preferences"));
			buttonBox()->button(QDialogButtonBox::Apply)->setEnabled(false);
		}
	} else if (button == buttonBox()->button(QDialogButtonBox::RestoreDefaults)) {
		const QString text(i18n("All settings will be reset to default values. Do you want to continue?"));
		if (KMessageBox::questionYesNo(this, text) == KMessageBox::Yes) {
			restoreDefaults();
			setWindowTitle(i18nc("@title:window", "Preferences"));
			buttonBox()->button(QDialogButtonBox::Apply)->setEnabled(false);
		}
	}
}

void SettingsDialog::changed() {
	m_changed = true;
	setWindowTitle(i18nc("@title:window", "Preferences    [Changed]"));
	buttonBox()->button(QDialogButtonBox::Apply)->setEnabled(true);
}

void SettingsDialog::applySettings() {
	m_changed = false;
	m_generalPage->applySettings();
	m_worksheetPage->applySettings();
	KSharedConfig::openConfig()->sync();
	emit settingsChanged();
}

void SettingsDialog::restoreDefaults() {
	m_changed = false;
	m_generalPage->restoreDefaults();
	m_worksheetPage->restoreDefaults();
}
