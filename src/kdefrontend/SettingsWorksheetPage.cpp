/***************************************************************************
    File                 : SettingsWorksheetPage.cpp
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

#include "SettingsWorksheetPage.h"
#include "tools/TeXRenderer.h"
#include "kdefrontend/widgets/ThemesComboBox.h"

#include <KLocalizedString>
#include <KSharedConfig>
#include <KConfigGroup>

/**
 * \brief Page for the 'General' settings of the Labplot settings dialog.
 */
SettingsWorksheetPage::SettingsWorksheetPage(QWidget* parent) : SettingsPage(parent),
	m_changed(false) {

	ui.setupUi(this);

	m_cbThemes = new ThemesComboBox();
	ui.gridLayout->addWidget(m_cbThemes, 1, 4, 1, 1);

	const int size = ui.cbTexEngine->height();
	ui.lLatexWarning->setPixmap( QIcon::fromTheme(QLatin1String("state-warning")).pixmap(size, size) );

	//add available TeX typesetting engines
	if (TeXRenderer::executableExists(QLatin1String("lualatex")))
		ui.cbTexEngine->addItem(QLatin1String("LuaLaTeX"), QLatin1String("lualatex"));

	if (TeXRenderer::executableExists(QLatin1String("xelatex")))
		ui.cbTexEngine->addItem(QLatin1String("XeLaTex"), QLatin1String("xelatex"));

	if (TeXRenderer::executableExists(QLatin1String("pdflatex")))
		ui.cbTexEngine->addItem(QLatin1String("pdfLaTeX"), QLatin1String("pdflatex"));

	if (TeXRenderer::executableExists(QLatin1String("latex")))
		ui.cbTexEngine->addItem(QLatin1String("LaTeX"), QLatin1String("latex"));

	connect(m_cbThemes, SIGNAL(currentThemeChanged(QString)), this, SLOT(changed()) );
	connect(ui.chkPresenterModeInteractive, SIGNAL(stateChanged(int)), this, SLOT(changed()) );
	connect(ui.chkDoubleBuffering, SIGNAL(stateChanged(int)), this, SLOT(changed()) );
	connect(ui.cbTexEngine, SIGNAL(currentIndexChanged(int)), this, SLOT(changed()) );
	connect(ui.cbTexEngine, SIGNAL(currentIndexChanged(int)), this, SLOT(checkTeX(int)) );

	loadSettings();
}

void SettingsWorksheetPage::applySettings() {
	KConfigGroup group = KSharedConfig::openConfig()->group(QLatin1String("Settings_Worksheet"));
	group.writeEntry(QLatin1String("Theme"), m_cbThemes->currentText());
	group.writeEntry(QLatin1String("PresenterModeInteractive"), ui.chkPresenterModeInteractive->isChecked());
	group.writeEntry(QLatin1String("DoubleBuffering"), ui.chkDoubleBuffering->isChecked());
	group.writeEntry(QLatin1String("LaTeXEngine"), ui.cbTexEngine->itemData(ui.cbTexEngine->currentIndex()));
}

void SettingsWorksheetPage::restoreDefaults() {
	loadSettings();
}

void SettingsWorksheetPage::loadSettings() {
	const KConfigGroup group = KSharedConfig::openConfig()->group(QLatin1String("Settings_Worksheet"));
	m_cbThemes->setItemText(0, group.readEntry(QLatin1String("Theme"), ""));
	ui.chkPresenterModeInteractive->setChecked(group.readEntry(QLatin1String("PresenterModeInteractive"), false));
	ui.chkDoubleBuffering->setChecked(group.readEntry(QLatin1String("DoubleBuffering"), true));

	QString engine = group.readEntry(QLatin1String("LaTeXEngine"), "");
	int index = -1;
	if (engine.isEmpty()) {
		//empty string was found in the settings (either the settings never saved or no tex engine was available during the last save)
		//->check whether the latex environment was installed in the meantime
		index = ui.cbTexEngine->findData(QLatin1String("xelatex"));
		if (index == -1) {
			index = ui.cbTexEngine->findData(QLatin1String("lualatex"));
			if (index == -1) {
				index = ui.cbTexEngine->findData(QLatin1String("pdflatex"));
				if (index == -1)
					index = ui.cbTexEngine->findData(QLatin1String("latex"));
			}
		}

		if (index != -1) {
			//one of the tex engines was found -> automatically save it in the settings without any user action
			KConfigGroup group = KSharedConfig::openConfig()->group(QLatin1String("Settings_Worksheet"));
			group.writeEntry(QLatin1String("LaTeXEngine"), ui.cbTexEngine->itemData(index));
		}
	}
	else
		index = ui.cbTexEngine->findData(engine);

	ui.cbTexEngine->setCurrentIndex(index);
	checkTeX(index);
}

void SettingsWorksheetPage::changed() {
	m_changed = true;
	emit settingsChanged();
}

/*!
 checks whether all tools required for latex typesetting are available. shows a warning if not.
 \sa TeXRenderer::active()
 */
void SettingsWorksheetPage::checkTeX(int engineIndex) {
	if (engineIndex == -1) {
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

#if defined(_WIN64)
	if (!TeXRenderer::executableExists(QLatin1String("gswin64c"))) {
		ui.lLatexWarning->show();
		ui.lLatexWarning->setToolTip(i18n("No Ghostscript found. LaTeX typesetting not possible."));
		return;
	}
#elif defined(HAVE_WINDOWS)
	if (!TeXRenderer::executableExists(QLatin1String("gswin32c"))) {
		ui.lLatexWarning->show();
		ui.lLatexWarning->setToolTip(i18n("No Ghostscript found. LaTeX typesetting not possible."));
		return;
	}
#endif

	ui.lLatexWarning->hide();
}
