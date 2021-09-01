/*
    File                 : SettingsWorksheetPage.cpp
    Project              : LabPlot
    Description          : settings page for Worksheet
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2008-2017 Alexander Semke (alexander.semke@web.de)

*/

/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
SettingsWorksheetPage::SettingsWorksheetPage(QWidget* parent) : SettingsPage(parent) {
	ui.setupUi(this);

	m_cbThemes = new ThemesComboBox();
	ui.gridLayout->addWidget(m_cbThemes, 1, 4, 1, 1);
	QString info = i18n("Default theme for newly created worksheets and worksheet objects");
	ui.lTheme->setToolTip(info);
	m_cbThemes->setToolTip(info);

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

	connect(m_cbThemes, &ThemesComboBox::currentThemeChanged, this, &SettingsWorksheetPage::changed);
	connect(ui.chkPresenterModeInteractive, &QCheckBox::stateChanged, this, &SettingsWorksheetPage::changed);
	connect(ui.chkDoubleBuffering, &QCheckBox::stateChanged, this, &SettingsWorksheetPage::changed);
	connect(ui.cbTexEngine, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsWorksheetPage::changed);
	connect(ui.cbTexEngine, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsWorksheetPage::checkTeX);

	loadSettings();
}

void SettingsWorksheetPage::applySettings() {
	if (!m_changed)
		return;

	KConfigGroup group = KSharedConfig::openConfig()->group(QLatin1String("Settings_Worksheet"));
	if (m_cbThemes->currentText() == i18n("Default"))
		group.writeEntry(QLatin1String("Theme"), QString());
	else
		group.writeEntry(QLatin1String("Theme"), m_cbThemes->currentText());
	group.writeEntry(QLatin1String("PresenterModeInteractive"), ui.chkPresenterModeInteractive->isChecked());
	group.writeEntry(QLatin1String("DoubleBuffering"), ui.chkDoubleBuffering->isChecked());
	group.writeEntry(QLatin1String("LaTeXEngine"), ui.cbTexEngine->itemData(ui.cbTexEngine->currentIndex()));
}

void SettingsWorksheetPage::restoreDefaults() {
	m_cbThemes->setItemText(0, i18n("Default")); //default theme
	ui.chkPresenterModeInteractive->setChecked(false);
	ui.chkDoubleBuffering->setChecked(true);

	int index = ui.cbTexEngine->findData(QLatin1String("xelatex"));
	if (index == -1) {
		index = ui.cbTexEngine->findData(QLatin1String("lualatex"));
		if (index == -1) {
			index = ui.cbTexEngine->findData(QLatin1String("pdflatex"));
			if (index == -1)
				index = ui.cbTexEngine->findData(QLatin1String("latex"));
		}
	}
	ui.cbTexEngine->setCurrentIndex(index);
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

	//engine found, check the presence of other required tools (s.a. TeXRenderer.cpp):
	//to convert the generated PDF/PS files to PNG we need 'convert' from the ImageMagic package
	if (!TeXRenderer::executableExists(QLatin1String("convert"))) {
		ui.lLatexWarning->show();
		ui.lLatexWarning->setToolTip(i18n("No 'convert' found. LaTeX typesetting not possible."));
		return;
	}

	QString engine = ui.cbTexEngine->itemData(engineIndex).toString();
	if (engine == "latex") {
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
