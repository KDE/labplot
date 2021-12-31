/*
    File                 : SettingsDialog.h
    Project              : LabPlot
    Description          : application settings dialog
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2008-2021 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <KPageDialog>

class QAbstractButton;
class SettingsGeneralPage;
class SettingsSpreadsheetPage;
class SettingsWorksheetPage;
class SettingsNotebookPage;
// class SettingsWelcomePage;
class SettingsDatasetsPage;

#ifdef HAVE_KUSERFEEDBACK
namespace KUserFeedback {
	class FeedbackConfigWidget;
}
#endif

class SettingsDialog : public KPageDialog {
	Q_OBJECT

public:
	explicit SettingsDialog(QWidget*);
	~SettingsDialog() override;

private Q_SLOTS:
	void changed();
	void slotButtonClicked(QAbstractButton*);

private:
	bool m_changed{false};
	SettingsGeneralPage* m_generalPage;
	SettingsWorksheetPage* m_worksheetPage;
	SettingsSpreadsheetPage* m_spreadsheetPage;
#ifdef HAVE_CANTOR_LIBS
	SettingsNotebookPage* m_notebookPage;
#endif
// 	SettingsWelcomePage* m_welcomePage;
	SettingsDatasetsPage* m_datasetsPage;

#ifdef HAVE_KUSERFEEDBACK
	KUserFeedback::FeedbackConfigWidget* m_userFeedbackWidget;
#endif

	void applySettings();
	void restoreDefaults();

Q_SIGNALS:
	void settingsChanged();
	void resetWelcomeScreen();
};

#endif
