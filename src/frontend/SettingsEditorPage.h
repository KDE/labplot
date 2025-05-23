/*
	File                 : SettingsEditorPage.h
	Project              : LabPlot
	Description          : settings page for KTextEditor
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Israel Galadima <izzygaladima@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SETTINGEDITORPAGE_H
#define SETTINGEDITORPAGE_H

#include <KTextEditor/ConfigPage>

#include "SettingsPage.h"

class KPageWidgetItem;
class KPageDialog;

class SettingsEditorPage : public SettingsPage {
	Q_OBJECT

public:
	explicit SettingsEditorPage(QWidget*);

	QList<Settings::Type> applySettings() override;
	void restoreDefaults() override;

    void addSubPages(KPageWidgetItem*, KPageDialog*);

private:
	bool m_changed{false};
    QVector<KTextEditor::ConfigPage*> m_editorPages;

private Q_SLOTS:
	void changed();

Q_SIGNALS:
	void settingsChanged();
};

#endif
