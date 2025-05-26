/*
	File                 : SettingsEditorPage.cpp
	Project              : LabPlot
	Description          : settings page for KTextEditor
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Israel Galadima <izzygaladima@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/
#include <QIcon>
#include <KLocalizedString>
#include <KPageDialog>
#include <KPageWidgetItem>
#include <KTextEditor/Editor>

#include "SettingsEditorPage.h"

SettingsEditorPage::SettingsEditorPage(QWidget* parent) : SettingsPage(parent) {
    
}

void SettingsEditorPage::addSubPages(KPageWidgetItem* editorRootFrame, KPageDialog* settingsDialog) {
    for (int i = 0; i < KTextEditor::Editor::instance()->configPages() - 1; ++i) {
        KTextEditor::ConfigPage* page = KTextEditor::Editor::instance()->configPage(i, this);
        connect(page, &KTextEditor::ConfigPage::changed, this, &SettingsEditorPage::changed);
        m_editorPages.push_back(page);
        KPageWidgetItem* item = settingsDialog->addSubPage(editorRootFrame, page, page->name());
        item->setHeader(page->fullName());
        item->setIcon(page->icon());
    }
}

QList<Settings::Type> SettingsEditorPage::applySettings() {
    QList<Settings::Type> changes;
    if (!m_changed)
		return changes;

    for (KTextEditor::ConfigPage* page : m_editorPages) {
        page->apply();
    }

    changes << Settings::Type::ScriptEditor;

    return changes;
}

void SettingsEditorPage::restoreDefaults() {
    for (KTextEditor::ConfigPage* page : m_editorPages) {
        page->defaults();
    }
}

void SettingsEditorPage::changed() {
	m_changed = true;
	Q_EMIT settingsChanged();
}