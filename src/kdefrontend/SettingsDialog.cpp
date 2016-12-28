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

#include "MainWin.h"
#include "SettingsGeneralPage.h"
// #include "SettingsPrintingPage.h"
#include "SettingsWorksheetPage.h"

#include <kmessagebox.h>
#include <KConfigGroup>
#include <KSharedConfig>
#include <KWindowConfig>
#include <KLocalizedString>

/**
 * \brief Settings dialog for Labplot.
 *
 * Contains the pages for general settings and view settings.
 *
 */
SettingsDialog::SettingsDialog(QWidget* parent) : KPageDialog(parent), m_changed(false) {
	setFaceType(List);
	setWindowTitle(i18n("Preferences"));
	setWindowIcon(QIcon::fromTheme("preferences-other"));

	//setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );
	connect(this, &QDialog::accepted, this, &SettingsDialog::onOkButton);
	//connect(this, &KPageDialog::applyClicked, this, &SettingsDialog::onApplyButton);
	//TODO: enableButtonApply(false);
	setAttribute(Qt::WA_DeleteOnClose);

	generalPage = new SettingsGeneralPage(this);
	connect(generalPage, SIGNAL(settingsChanged()), this, SLOT(changed()));
	KPageWidgetItem* generalFrame = addPage(generalPage, i18n("General"));
	generalFrame->setIcon(QIcon::fromTheme("system-run"));

	//QPushButton* defaultbutton = dialogButtonBox->addButton(QDialogButtonBox::RestoreDefaults);
	//connect( defaultbutton, &QAbstractButton::clicked, this, &SettingsDialog::onRestoreDefaultsButton );

	// unused
	//QVBoxLayout* layout = new QVBoxLayout;
	//layout->addWidget( dialogButtonBox );
	//setLayout( layout );

//     printingPage = new SettingsPrintingPage(mainWindow, this);
//     KPageWidgetItem* printingFrame = addPage(printingPage, i18nc("@title:group", "Print"));
//     printingFrame->setIcon(KIcon("document-print"));

	const KConfigGroup dialogConfig = KSharedConfig::openConfig()->group("SettingsDialog");
	KWindowConfig::restoreWindowSize(windowHandle(), dialogConfig);
}

SettingsDialog::~SettingsDialog(){
	KConfigGroup dialogConfig = KSharedConfig::openConfig()->group("SettingsDialog");
	KWindowConfig::saveWindowSize(windowHandle(), dialogConfig);

	worksheetPage = new SettingsWorksheetPage(this);
	KPageWidgetItem* worksheetFrame = addPage(worksheetPage, i18n("Worksheet"));
	worksheetFrame->setIcon(QIcon::fromTheme(QLatin1String("labplot-worksheet")));
	connect(worksheetPage, SIGNAL(settingsChanged()), this, SLOT(changed()));

	KConfigGroup conf(KSharedConfig::openConfig(), "SettingsDialog");
	//restoreDialogSize(conf);
}

void SettingsDialog::onOkButton(){
	if (m_changed){
		applySettings();
		setWindowTitle(i18n("Preferences"));
//		applybutton->setEnabled(false);
	}
}

/*void SettingsDialog::onApplyButton(){
	qDebug()<<"SettingsDialog::onApplyButton()";
>>>>>>> frameworks
	if (m_changed){
		applySettings();
		setWindowTitle(i18n("Preferences"));
		applybutton->setEnabled(false);
	}
}*/

void SettingsDialog::onRestoreDefaultsButton(){
	const QString text(i18n("All settings will be reset to default values. Do you want to continue?"));
	if (KMessageBox::questionYesNo(this, text) == KMessageBox::Yes) {
		restoreDefaults();
		setWindowTitle(i18n("Preferences"));
//		applybutton->setEnabled(false);
	}
}

void SettingsDialog::changed() {
	m_changed = true;
	setWindowTitle(i18n("Preferences    [Changed]"));
//	applybutton->setEnabled(true);
}

void SettingsDialog::applySettings(){
	m_changed = false;
	generalPage->applySettings();
//     printingPage->applySettings();
	worksheetPage->applySettings();
	KSharedConfig::openConfig()->sync();
	emit settingsChanged();
}

void SettingsDialog::restoreDefaults(){
	m_changed = false;
	generalPage->restoreDefaults();
//    printingPage->restoreDefaults();
	worksheetPage->restoreDefaults();
}
