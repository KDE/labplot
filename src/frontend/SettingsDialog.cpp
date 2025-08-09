/*
	File                 : SettingsDialog.cpp
	Project              : LabPlot
	Description          : application settings dialog
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2008-2025 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "SettingsDialog.h"

#include "MainWin.h"
#include "SettingsDatasetsPage.h"
#include "SettingsGeneralPage.h"
// #include "SettingsWelcomePage.h"
#include "SettingsSpreadsheetPage.h"
#include "SettingsWorksheetPage.h"
#ifdef HAVE_SCRIPTING
#include "SettingsEditorPage.h"
#endif

#ifdef HAVE_CANTOR_LIBS
#include "SettingsNotebookPage.h"
#endif

#include <KConfigGroup>
#include <KMessageBox>

#include <KWindowConfig>

#ifdef HAVE_KUSERFEEDBACK
#include <KUserFeedback/FeedbackConfigWidget>
#endif

#include <QDialogButtonBox>
#include <QPushButton>
#include <QWindow>

/**
 * \brief Settings dialog for Labplot.
 *
 * Contains the pages for general settings and view settings.
 *
 */
SettingsDialog::SettingsDialog(QWidget* parent, const QLocale& locale)
	: KPageDialog(parent) {
	setFaceType(Tree);
	setWindowTitle(i18nc("@title:window", "Preferences"));
	setWindowIcon(QIcon::fromTheme(QLatin1String("preferences-other")));
	setAttribute(Qt::WA_DeleteOnClose);

	buttonBox()->addButton(QDialogButtonBox::Apply)->setEnabled(false);
	buttonBox()->addButton(QDialogButtonBox::RestoreDefaults);
	connect(buttonBox(), &QDialogButtonBox::clicked, this, &SettingsDialog::slotButtonClicked);

	m_generalPage = new SettingsGeneralPage(this, locale);
	KPageWidgetItem* generalFrame = addPage(m_generalPage, i18n("General"));
	generalFrame->setIcon(QIcon::fromTheme(QLatin1String("settings-configure")));
	connect(m_generalPage, &SettingsGeneralPage::settingsChanged, this, &SettingsDialog::changed);

	m_worksheetPage = new SettingsWorksheetPage(this);
	m_worksheetPageItem = addPage(m_worksheetPage, i18n("Worksheet"));
	m_worksheetPageItem->setIcon(QIcon::fromTheme(QLatin1String("labplot-worksheet")));
	connect(m_worksheetPage, &SettingsWorksheetPage::settingsChanged, this, &SettingsDialog::changed);

	m_spreadsheetPage = new SettingsSpreadsheetPage(this);
	m_spreadsheetPageItem = addPage(m_spreadsheetPage, i18n("Spreadsheet"));
	m_spreadsheetPageItem->setIcon(QIcon::fromTheme(QLatin1String("labplot-spreadsheet")));
	connect(m_spreadsheetPage, &SettingsSpreadsheetPage::settingsChanged, this, &SettingsDialog::changed);

#ifdef HAVE_CANTOR_LIBS
	m_notebookPage = new SettingsNotebookPage(this);
	m_notebookPageItem = addPage(m_notebookPage, i18n("Notebook"));
	m_notebookPage->addSubPages(m_notebookPageItem, this);
	m_notebookPageItem->setIcon(QIcon::fromTheme(QLatin1String("cantor")));
	connect(m_notebookPage, &SettingsNotebookPage::settingsChanged, this, &SettingsDialog::changed);
#endif

	m_datasetsPage = new SettingsDatasetsPage(this);
	m_datasetsPageItem = addPage(m_datasetsPage, i18n("Datasets"));
	m_datasetsPageItem->setIcon(QIcon::fromTheme(QLatin1String("database-index")));
	connect(m_datasetsPage, &SettingsDatasetsPage::settingsChanged, this, &SettingsDialog::changed);

	// 	m_welcomePage = new SettingsWelcomePage(this);
	// 	KPageWidgetItem* welcomeFrame = addPage(m_welcomePage, i18n("Welcome Screen"));
	// 	welcomeFrame->setIcon(QIcon::fromTheme(QLatin1String("database-index")));
	// 	connect(m_welcomePage, &SettingsWelcomePage::resetWelcomeScreen, this, &SettingsDialog::resetWelcomeScreen);

#ifdef HAVE_KUSERFEEDBACK
	auto* mainWin = static_cast<MainWin*>(parent);
	m_userFeedbackWidget = new KUserFeedback::FeedbackConfigWidget(this);
	m_userFeedbackWidget->setFeedbackProvider(&mainWin->userFeedbackProvider());
	connect(m_userFeedbackWidget, &KUserFeedback::FeedbackConfigWidget::configurationChanged, this, &SettingsDialog::changed);

	m_userFeedbackPageItem = addPage(m_userFeedbackWidget, i18n("User Feedback"));
	m_userFeedbackPageItem->setIcon(QIcon::fromTheme(QLatin1String("preferences-desktop-locale")));
#endif

#ifdef HAVE_SCRIPTING
	m_editorRootPage = new SettingsEditorPage(this);
	m_editorRootItem = addPage(m_editorRootPage, i18n("Text Editor"));
	m_editorRootPage->addSubPages(m_editorRootItem, this);
	m_editorRootItem->setIcon(QIcon::fromTheme(QLatin1String("accessories-text-editor")));
	connect(m_editorRootPage, &SettingsEditorPage::settingsChanged, this, &SettingsDialog::changed);
#endif

	// restore saved settings if available
	create(); // ensure there's a window created
	KConfigGroup conf = Settings::group(QStringLiteral("SettingsDialog"));
	if (conf.exists()) {
		KWindowConfig::restoreWindowSize(windowHandle(), conf);
		resize(windowHandle()->size()); // workaround for QTBUG-40584
	} else
		resize(QSize(0, 0).expandedTo(minimumSize()));
}

SettingsDialog::~SettingsDialog() {
	KConfigGroup dialogConfig = Settings::group(QStringLiteral("SettingsDialog"));
	KWindowConfig::saveWindowSize(windowHandle(), dialogConfig);
}

void SettingsDialog::navigateTo(Settings::Type type) {
	switch (type) {
	case Settings::Type::General:
	case Settings::Type::General_Number_Format:
	case Settings::Type::General_Units:
		// "general" is selected initially
		break;
	case Settings::Type::Worksheet:
		setCurrentPage(m_worksheetPageItem);
		break;
	case Settings::Type::Spreadsheet:
		setCurrentPage(m_spreadsheetPageItem);
		break;
#ifdef HAVE_CANTOR_LIBS
	case Settings::Type::Notebook:
		setCurrentPage(m_notebookPageItem);
		break;
#endif
	case Settings::Type::Datasets:
		setCurrentPage(m_datasetsPageItem);
		break;
#ifdef HAVE_KUSERFEEDBACK
	case Settings::Type::Feedback:
		setCurrentPage(m_userFeedbackPageItem);
		break;
#endif
#ifdef HAVE_SCRIPTING
	case Settings::Type::ScriptEditor:
		setCurrentPage(m_editorRootItem);
		break;
#endif
	}
}

void SettingsDialog::slotButtonClicked(QAbstractButton* button) {
	if ((button == buttonBox()->button(QDialogButtonBox::Ok)) || (button == buttonBox()->button(QDialogButtonBox::Apply))) {
		if (m_changed) {
			applySettings();
			setWindowTitle(i18nc("@title:window", "Preferences"));
			buttonBox()->button(QDialogButtonBox::Apply)->setEnabled(false);
		}
	} else if (button == buttonBox()->button(QDialogButtonBox::RestoreDefaults)) {
		const QString text(i18n("All settings will be reset to default values. Do you want to continue?"));
		if (KMessageBox::questionTwoActions(this, text, QString(), KStandardGuiItem::reset(), KStandardGuiItem::cancel()) == KMessageBox::PrimaryAction) {
			restoreDefaults();
			setWindowTitle(i18nc("@title:window", "Preferences"));
			buttonBox()->button(QDialogButtonBox::Apply)->setEnabled(false);
		}
	}
}

void SettingsDialog::changed() {
	m_changed = true;
	setWindowTitle(i18nc("@title:window", "Preferences    [Changed]"));
	buttonBox()->button(QDialogButtonBox::Apply)->setEnabled(true);
}

void SettingsDialog::applySettings() {
	m_changed = false;
	QList<Settings::Type> changes;
	changes << m_generalPage->applySettings();
	changes << m_worksheetPage->applySettings();
	changes << m_spreadsheetPage->applySettings();
#ifdef HAVE_CANTOR_LIBS
	changes << m_notebookPage->applySettings();
#endif
	changes << m_datasetsPage->applySettings();
#ifdef HAVE_SCRIPTING
	changes << m_editorRootPage->applySettings();
#endif

	Settings::sync();

#ifdef HAVE_KUSERFEEDBACK
	auto* mainWin = static_cast<MainWin*>(parent());
	mainWin->userFeedbackProvider().setTelemetryMode(m_userFeedbackWidget->telemetryMode());
	mainWin->userFeedbackProvider().setSurveyInterval(m_userFeedbackWidget->surveyInterval());
#endif

	if (!changes.isEmpty())
		Q_EMIT settingsChanged(changes);
}

void SettingsDialog::restoreDefaults() {
	m_changed = false;
	m_generalPage->restoreDefaults();
	m_worksheetPage->restoreDefaults();
	m_spreadsheetPage->restoreDefaults();
#ifdef HAVE_CANTOR_LIBS
	m_notebookPage->restoreDefaults();
#endif
	m_datasetsPage->restoreDefaults();
#ifdef HAVE_SCRIPTING
	m_editorRootPage->restoreDefaults();
#endif
}
