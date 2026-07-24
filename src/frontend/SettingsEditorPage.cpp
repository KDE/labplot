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
#include <KTextEditor/ConfigPage>
#include <KTextEditor/Editor>

#include "SettingsEditorPage.h"

SettingsEditorPage::SettingsEditorPage(QWidget* parent) : SettingsPage(parent) {
    
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

	return {Settings::Type::ScriptEditor};
}

void SettingsEditorPage::restoreDefaults() {
	for (auto* page : m_editorPages)
		page->defaults();
}

void SettingsEditorPage::changed() {
	m_changed = true;
	Q_EMIT settingsChanged();
}
