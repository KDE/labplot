/***************************************************************************
    File                 : SettingsDialog.cpp
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008-2016 by Alexander Semke (alexander.semke@web.de)
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
#include "SettingsGeneralPage.h"
#include "SettingsWorksheetPage.h"

#include <KMessageBox>

/**
 * \brief Settings dialog for Labplot.
 *
 * Contains the pages for general settings and view settings.
 *
 */
SettingsDialog::SettingsDialog(QWidget* parent) : KPageDialog(parent), m_changed(false) {
	const QSize minSize = minimumSize();
	setMinimumSize(QSize(512, minSize.height()));

	setFaceType(List);
	setCaption(i18n("Preferences"));
	setWindowIcon(KIcon("preferences-other"));
	setButtons(KDialog::Ok | KDialog::Apply | KDialog::Cancel | KDialog::Default);
	setDefaultButton(KDialog::Ok);
	enableButton(KDialog::Apply, false);
    setAttribute(Qt::WA_DeleteOnClose);

	generalPage = new SettingsGeneralPage(this);
	KPageWidgetItem* generalFrame = addPage(generalPage, i18n("General"));
	generalFrame->setIcon(KIcon("system-run"));
	connect(generalPage, SIGNAL(settingsChanged()), this, SLOT(changed()));

	worksheetPage = new SettingsWorksheetPage(this);
	KPageWidgetItem* worksheetFrame = addPage(worksheetPage, i18n("Worksheet"));
	worksheetFrame->setIcon(KIcon(QLatin1String("labplot-worksheet")));
	connect(worksheetPage, SIGNAL(settingsChanged()), this, SLOT(changed()));

	KConfigGroup conf(KSharedConfig::openConfig(), "SettingsDialog");
	restoreDialogSize(conf);
}

SettingsDialog::~SettingsDialog() {
	KConfigGroup conf(KSharedConfig::openConfig(), "SettingsDialog");
	saveDialogSize(conf);
}

void SettingsDialog::slotButtonClicked(int button) {
	if ((button == KDialog::Ok) || (button == KDialog::Apply)) {
		if (m_changed){
			applySettings();
			setCaption(i18n("Preferences"));
			enableButton(KDialog::Apply, false);
		}
	} else if (button == KDialog::Default) {
		const QString text(i18n("All settings will be reset to default values. Do you want to continue?"));
		if (KMessageBox::questionYesNo(this, text) == KMessageBox::Yes) {
			restoreDefaults();
			setCaption(i18n("Preferences"));
			enableButton(KDialog::Apply, false);
		}
	}

	KPageDialog::slotButtonClicked(button);
}

void SettingsDialog::changed() {
	m_changed = true;
	setCaption(i18n("Preferences    [Changed]"));
	enableButton(KDialog::Apply, true);
}

void SettingsDialog::applySettings() {
	m_changed = false;
	generalPage->applySettings();
	KGlobal::config()->sync();	
	emit settingsChanged();
}

void SettingsDialog::restoreDefaults(){
	m_changed = false;
	generalPage->restoreDefaults();
}
