/*
	File                 : SettingsEditorPage.cpp
	Project              : LabPlot
	Description          : settings page for KTextEditor
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Israel Galadima <izzygaladima@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <KPageDialog>
#include <KPageWidgetItem>
#include <KLocalizedString>
#include <KTextEditor/ConfigPage>
#include <KTextEditor/Editor>

#include "SettingsEditorPage.h"
#include "backend/core/Settings.h"

#include <QCheckBox>
#include <QVBoxLayout>

SettingsEditorPage::SettingsEditorPage(QWidget* parent) : SettingsPage(parent) {
	auto* layout = new QVBoxLayout(this);
	m_autoShowOutputCheckBox = new QCheckBox(i18n("Automatically show output when a script produces output"), this);
	m_autoShowOutputCheckBox->setChecked(Settings::group(QStringLiteral("ScriptEditor")).readEntry(QStringLiteral("AutoShowOutput"), true));
	connect(m_autoShowOutputCheckBox, &QCheckBox::toggled, this, &SettingsEditorPage::changed);
	layout->addWidget(m_autoShowOutputCheckBox);
	layout->addStretch();
}

void SettingsEditorPage::addSubPages(KPageWidgetItem* editorRootFrame, KPageDialog* settingsDialog) {
	auto* instance = KTextEditor::Editor::instance();
	for (int i = 0; i < instance->configPages() - 1; ++i) {
		auto* page = instance->configPage(i, this);
		connect(page, &KTextEditor::ConfigPage::changed, this, &SettingsEditorPage::changed);
		m_editorPages.push_back(page);

		auto* item = settingsDialog->addSubPage(editorRootFrame, page, page->name());
		item->setHeader(page->fullName());
		item->setIcon(page->icon());
	}
}

QList<Settings::Type> SettingsEditorPage::applySettings() {
	if (!m_changed)
		return {};

	for (auto* page : m_editorPages)
		page->apply();
	Settings::group(QStringLiteral("ScriptEditor")).writeEntry(QStringLiteral("AutoShowOutput"), m_autoShowOutputCheckBox->isChecked());

	return {Settings::Type::ScriptEditor};
}

void SettingsEditorPage::restoreDefaults() {
	for (auto* page : m_editorPages)
		page->defaults();
	m_autoShowOutputCheckBox->setChecked(true);
}

void SettingsEditorPage::changed() {
	m_changed = true;
	Q_EMIT settingsChanged();
}
