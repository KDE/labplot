/*
	File                 : SettingsDialog.h
	Project              : LabPlot
	Description          : application settings dialog
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2008-2025 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include "backend/core/Settings.h"
#include <QLocale>
#include <KPageDialog>

class QAbstractButton;
class SettingsGeneralPage;
class SettingsSpreadsheetPage;
class SettingsWorksheetPage;
class SettingsNotebookPage;
// class SettingsWelcomePage;
class SettingsDatasetsPage;
#ifdef HAVE_SCRIPTING
class SettingsEditorPage;
#endif

#ifdef HAVE_KUSERFEEDBACK
namespace KUserFeedback {
class FeedbackConfigWidget;
}
#endif

class SettingsDialog : public KPageDialog {
	Q_OBJECT

public:
	explicit SettingsDialog(QWidget*, const QLocale&);
	void navigateTo(Settings::Type);
	~SettingsDialog() override;

private Q_SLOTS:
	void changed();
	void slotButtonClicked(QAbstractButton*);

private:
	bool m_changed{false};

	SettingsGeneralPage* m_generalPage{nullptr};
	SettingsWorksheetPage* m_worksheetPage{nullptr};
	KPageWidgetItem* m_worksheetPageItem{nullptr};
	SettingsSpreadsheetPage* m_spreadsheetPage{nullptr};
	KPageWidgetItem* m_spreadsheetPageItem{nullptr};
#ifdef HAVE_CANTOR_LIBS
	SettingsNotebookPage* m_notebookPage{nullptr};
	KPageWidgetItem* m_notebookPageItem{nullptr};
#endif
	SettingsDatasetsPage* m_datasetsPage{nullptr};
#ifdef HAVE_SCRIPTING
	SettingsEditorPage* m_editorRootPage;
#endif
	KPageWidgetItem* m_datasetsPageItem{nullptr};

#ifdef HAVE_KUSERFEEDBACK
	KUserFeedback::FeedbackConfigWidget* m_userFeedbackWidget{nullptr};
	KPageWidgetItem* m_userFeedbackPageItem{nullptr};
#endif

	void applySettings();
	void restoreDefaults();

Q_SIGNALS:
	void settingsChanged(QList<Settings::Type>);
	void resetWelcomeScreen();
};

#endif
