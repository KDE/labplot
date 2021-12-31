/*
    File                 : SettingsNotebookPage.cpp
    Project              : LabPlot
    Description          : Settings page for Notebook
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2021 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "SettingsNotebookPage.h"

#include <KLocalizedString>
#include <KSharedConfig>
#include <KConfigGroup>

/**
 * \brief Page for Notebook settings of the Labplot settings dialog.
 */
SettingsNotebookPage::SettingsNotebookPage(QWidget* parent) : SettingsPage(parent) {
	ui.setupUi(this);

	ui.chkSyntaxHighlighting->setToolTip(i18n("Enable syntax highlighting"));
	ui.chkSyntaxCompletion->setToolTip(i18n("Enable syntax completion"));
	ui.chkLineNumbers->setToolTip(i18n("Show line numbers"));
	ui.chkLatexTypesetting->setToolTip(i18n("Use LaTeX typesetting for the results of calculations, if supported by the backend system."));
	ui.chkAnimations->setToolTip(i18n("Animate transitions"));

	ui.chkReevaluateEntries->setToolTip(i18n("Automatically re-evaluate all entries below the current one."));
	ui.chkAskConfirmation->setToolTip(i18n("Ask for confirmation when restarting the backend system."));

	connect(ui.chkSyntaxHighlighting, &QCheckBox::toggled, this, &SettingsNotebookPage::changed);
	connect(ui.chkSyntaxCompletion, &QCheckBox::toggled, this, &SettingsNotebookPage::changed);
	connect(ui.chkLineNumbers, &QCheckBox::toggled, this, &SettingsNotebookPage::changed);
	connect(ui.chkLatexTypesetting, &QCheckBox::toggled, this, &SettingsNotebookPage::changed);
	connect(ui.chkAnimations, &QCheckBox::toggled, this, &SettingsNotebookPage::changed);
	connect(ui.chkReevaluateEntries, &QCheckBox::toggled, this, &SettingsNotebookPage::changed);
	connect(ui.chkAskConfirmation, &QCheckBox::toggled, this, &SettingsNotebookPage::changed);

	loadSettings();
}

void SettingsNotebookPage::applySettings() {
	if (!m_changed)
		return;

	KConfigGroup group = KSharedConfig::openConfig()->group(QLatin1String("Settings_Notebook"));

	//Appearance
	group.writeEntry(QLatin1String("SyntaxHighlighting"), ui.chkSyntaxHighlighting->isChecked());
	group.writeEntry(QLatin1String("SyntaxCompletion"), ui.chkSyntaxCompletion->isChecked());
	group.writeEntry(QLatin1String("LineNumbers"), ui.chkLineNumbers->isChecked());
	group.writeEntry(QLatin1String("LatexTypesetting"), ui.chkLatexTypesetting->isChecked());
	group.writeEntry(QLatin1String("Animations"), ui.chkAnimations->isChecked());

	//Evaluation
	group.writeEntry(QLatin1String("ReevaluateEntries"), ui.chkReevaluateEntries->isChecked());
	group.writeEntry(QLatin1String("AskConfirmation"), ui.chkAskConfirmation->isChecked());
}

void SettingsNotebookPage::restoreDefaults() {
	//Appearance
	ui.chkSyntaxHighlighting->setChecked(true);
	ui.chkSyntaxCompletion->setChecked(true);
	ui.chkLineNumbers->setChecked(false);
	ui.chkLatexTypesetting->setChecked(true);
	ui.chkAnimations->setChecked(true);

	//Evaluation
	ui.chkReevaluateEntries->setChecked(false);
	ui.chkAskConfirmation->setChecked(true);
}

void SettingsNotebookPage::loadSettings() {
	const KConfigGroup group = KSharedConfig::openConfig()->group(QLatin1String("Settings_Notebook"));

	//Appearance
	ui.chkSyntaxHighlighting->setChecked(group.readEntry(QLatin1String("SyntaxHighlighting"), true));
	ui.chkSyntaxCompletion->setChecked(group.readEntry(QLatin1String("SyntaxCompletion"), true));
	ui.chkLineNumbers->setChecked(group.readEntry(QLatin1String("LineNumbers"), false));
	ui.chkLatexTypesetting->setChecked(group.readEntry(QLatin1String("LatexTypesetting"), true));
	ui.chkAnimations->setChecked(group.readEntry(QLatin1String("Animations"), true));

	//Evaluation
	ui.chkReevaluateEntries->setChecked(group.readEntry(QLatin1String("ReevaluateEntries"), false));
	ui.chkAskConfirmation->setChecked(group.readEntry(QLatin1String("AskConfirmation"), true));
}

void SettingsNotebookPage::changed() {
	m_changed = true;
	Q_EMIT settingsChanged();
}
