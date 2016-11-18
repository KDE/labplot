/***************************************************************************
    File                 : SettingsGeneralPage.cpp
    Project              : LabPlot
    Description          : general settings page
    --------------------------------------------------------------------
    Copyright            : (C) 2008-2016 Alexander Semke (alexander.semke@web.de)

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

#include "SettingsGeneralPage.h"
#include "MainWin.h"
#include "tools/TeXRenderer.h"

#include <KDialog>
#include <KLocale>
#include <kfiledialog.h>

/**
 * \brief Page for the 'General' settings of the Labplot settings dialog.
 */
SettingsGeneralPage::SettingsGeneralPage(QWidget* parent) : SettingsPage(parent),
	m_changed(false) {

	ui.setupUi(this);
	retranslateUi();

	ui.lLatexWarning->setPixmap( KIcon("state-warning").pixmap(QSize(48,48)) );

	//add available TeX typesetting engines
	if (TeXRenderer::executableExists(QLatin1String("lualatex")))
		ui.cbTexEngine->addItem(QLatin1String("LuaLaTeX"), QLatin1String("lualatex"));

	if (TeXRenderer::executableExists(QLatin1String("xelatex")))
		ui.cbTexEngine->addItem(QLatin1String("XeLaTex"), QLatin1String("xelatex"));

	if (TeXRenderer::executableExists(QLatin1String("pdflatex")))
		ui.cbTexEngine->addItem(QLatin1String("pdfLaTeX"), QLatin1String("pdflatex"));

	if (TeXRenderer::executableExists(QLatin1String("latex")))
		ui.cbTexEngine->addItem(QLatin1String("LaTeX"), QLatin1String("latex"));

	connect(ui.cbLoadOnStart, SIGNAL(currentIndexChanged(int)), this, SLOT(changed()) );
	connect(ui.cbInterface, SIGNAL(currentIndexChanged(int)), this, SLOT(interfaceChanged(int)) );
	connect(ui.cbMdiVisibility, SIGNAL(currentIndexChanged(int)), this, SLOT(changed()) );
	connect(ui.cbTabPosition, SIGNAL(currentIndexChanged(int)), this, SLOT(changed()) );
	connect(ui.chkAutoSave, SIGNAL(stateChanged(int)), this, SLOT(changed()) );
	connect(ui.sbAutoSaveInterval, SIGNAL(valueChanged(int)), this, SLOT(changed()) );
	connect(ui.chkDoubleBuffering, SIGNAL(stateChanged(int)), this, SLOT(changed()) );
	connect(ui.cbTexEngine, SIGNAL(currentIndexChanged(int)), this, SLOT(changed()) );
	connect(ui.cbTexEngine, SIGNAL(currentIndexChanged(int)), this, SLOT(checkTeX(int)) );

	loadSettings();
	interfaceChanged(ui.cbInterface->currentIndex());
}

void SettingsGeneralPage::applySettings(){
	KConfigGroup group = KGlobal::config()->group( "General" );
	group.writeEntry("LoadOnStart", ui.cbLoadOnStart->currentIndex());
	group.writeEntry("ViewMode", ui.cbInterface->currentIndex());
	group.writeEntry("TabPosition", ui.cbTabPosition->currentIndex());
	group.writeEntry("MdiWindowVisibility", ui.cbMdiVisibility->currentIndex());
	group.writeEntry("AutoSave", ui.chkAutoSave->isChecked());
	group.writeEntry("AutoSaveInterval", ui.sbAutoSaveInterval->value());
	group.writeEntry("DoubleBuffering", ui.chkDoubleBuffering->isChecked());
	group.writeEntry("LaTeXEngine", ui.cbTexEngine->itemData(ui.cbTexEngine->currentIndex()));
}

void SettingsGeneralPage::restoreDefaults(){
    loadSettings();
}

void SettingsGeneralPage::loadSettings(){
	const KConfigGroup group = KGlobal::config()->group( "General" );
	ui.cbLoadOnStart->setCurrentIndex(group.readEntry("LoadOnStart", 0));
	ui.cbInterface->setCurrentIndex(group.readEntry("ViewMode", 0));
	ui.cbTabPosition->setCurrentIndex(group.readEntry("TabPosition", 0));
	ui.cbMdiVisibility->setCurrentIndex(group.readEntry("MdiWindowVisibility", 0));
	ui.chkAutoSave->setChecked(group.readEntry<bool>("AutoSave", 0));
	ui.sbAutoSaveInterval->setValue(group.readEntry("AutoSaveInterval", 0));
	ui.chkDoubleBuffering->setChecked(group.readEntry<bool>("DoubleBuffering", 1));

	QString engine = group.readEntry("LaTeXEngine", "");
	int index = -1;
	if (engine.isEmpty())
		index = ui.cbTexEngine->findData("xelatex");
	else
		index = ui.cbTexEngine->findData(engine);

	ui.cbTexEngine->setCurrentIndex(index);
}

void SettingsGeneralPage::retranslateUi() {
	ui.cbLoadOnStart->clear();
	ui.cbLoadOnStart->addItem(i18n("Do nothing"));
	ui.cbLoadOnStart->addItem(i18n("Create new empty project"));
	ui.cbLoadOnStart->addItem(i18n("Create new project with worksheet"));
	ui.cbLoadOnStart->addItem(i18n("Load last used project"));
	
	ui.cbInterface->clear();
	ui.cbInterface->addItem(i18n("Sub-window view"));
	ui.cbInterface->addItem(i18n("Tabbed view"));
	
	ui.cbMdiVisibility->clear();
	ui.cbMdiVisibility->addItem(i18n("Show windows of the current folder only"));
	ui.cbMdiVisibility->addItem(i18n("Show windows of the current folder and its subfolders only"));
	ui.cbMdiVisibility->addItem(i18n("Show all windows"));

	ui.cbTabPosition->clear();
	ui.cbTabPosition->addItem(i18n("Top"));
	ui.cbTabPosition->addItem(i18n("Bottom"));
	ui.cbTabPosition->addItem(i18n("Left"));
	ui.cbTabPosition->addItem(i18n("Right"));
}

void SettingsGeneralPage::changed() {
	m_changed = true;
	emit settingsChanged();
}

void SettingsGeneralPage::interfaceChanged(int index) {
	bool tabbedView = (index==1);
	ui.lTabPosition->setVisible(tabbedView);
	ui.cbTabPosition->setVisible(tabbedView);
	ui.lMdiVisibility->setVisible(!tabbedView);
	ui.cbMdiVisibility->setVisible(!tabbedView);
	changed();
}

/*!
 checks whether all tools required for latex typesetting are available. shows a warning if not.
 \sa TeXRenderer::active()
 */
void SettingsGeneralPage::checkTeX(int engineIndex) {
	if (engineIndex == -1){
		ui.lLatexWarning->show();
		ui.lLatexWarning->setToolTip(i18n("No LaTeX installation found or selected. LaTeX typesetting not possible."));
		return;
	}

	//engine found, check the precense of other required tools (s.a. TeXRenderer.cpp):
	//to convert the generated PDF/PS files to PNG we need 'convert' from the ImageMagic package
	if (!TeXRenderer::executableExists(QLatin1String("convert"))) {
		ui.lLatexWarning->show();
		ui.lLatexWarning->setToolTip(i18n("No 'convert' found. LaTeX typesetting not possible."));
		return;
	}

	QString engine = ui.cbTexEngine->itemData(engineIndex).toString();
	if (engine=="latex") {
		//to convert the generated PS files to DVI we need 'dvips'
		if (!TeXRenderer::executableExists(QLatin1String("dvips"))) {
			ui.lLatexWarning->show();
			ui.lLatexWarning->setToolTip(i18n("No 'dvips' found. LaTeX typesetting not possible."));
			return;
		}
	}

	ui.lLatexWarning->hide();
}
