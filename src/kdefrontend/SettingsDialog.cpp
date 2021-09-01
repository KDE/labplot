/*
    File                 : SettingsDialog.cpp
    Project              : LabPlot
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2008-2020 Alexander Semke <alexander.semke@web.de>
    Description          : application settings dialog
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "SettingsDialog.h"

#include "MainWin.h"
#include "SettingsGeneralPage.h"
#include "SettingsDatasetsPage.h"
// #include "SettingsWelcomePage.h"
#include "SettingsSpreadsheetPage.h"
#include "SettingsWorksheetPage.h"

#include <QPushButton>
#include <QDialogButtonBox>
#include <QWindow>

#include <KMessageBox>
#include <KConfigGroup>
#include <KSharedConfig>
#include <KWindowConfig>
#include <KI18n/KLocalizedString>

#ifdef HAVE_KUSERFEEDBACK
#include <KUserFeedback/FeedbackConfigWidget>
#endif

/**
 * \brief Settings dialog for Labplot.
 *
 * Contains the pages for general settings and view settings.
 *
 */
SettingsDialog::SettingsDialog(QWidget* parent) : KPageDialog(parent) {
	setFaceType(List);
	setWindowTitle(i18nc("@title:window", "Preferences"));
	setWindowIcon(QIcon::fromTheme("preferences-other"));
	setAttribute(Qt::WA_DeleteOnClose);

	buttonBox()->addButton(QDialogButtonBox::Apply)->setEnabled(false);
	buttonBox()->addButton(QDialogButtonBox::RestoreDefaults);
	connect(buttonBox(), &QDialogButtonBox::clicked, this, &SettingsDialog::slotButtonClicked);

	m_generalPage = new SettingsGeneralPage(this);
	KPageWidgetItem* generalFrame = addPage(m_generalPage, i18n("General"));
	generalFrame->setIcon(QIcon::fromTheme("system-run"));
	connect(m_generalPage, &SettingsGeneralPage::settingsChanged, this, &SettingsDialog::changed);

	m_worksheetPage = new SettingsWorksheetPage(this);
	KPageWidgetItem* worksheetFrame = addPage(m_worksheetPage, i18n("Worksheet"));
	worksheetFrame->setIcon(QIcon::fromTheme(QLatin1String("labplot-worksheet")));
	connect(m_worksheetPage, &SettingsWorksheetPage::settingsChanged, this, &SettingsDialog::changed);

	m_spreadsheetPage = new SettingsSpreadsheetPage(this);
	KPageWidgetItem* spreadsheetFrame = addPage(m_spreadsheetPage, i18n("Spreadsheet"));
	spreadsheetFrame->setIcon(QIcon::fromTheme(QLatin1String("labplot-spreadsheet")));
	connect(m_spreadsheetPage, &SettingsSpreadsheetPage::settingsChanged, this, &SettingsDialog::changed);

	m_datasetsPage = new SettingsDatasetsPage(this);
	KPageWidgetItem* datasetsFrame = addPage(m_datasetsPage, i18n("Datasets"));
	datasetsFrame->setIcon(QIcon::fromTheme(QLatin1String("database-index")));

// 	m_welcomePage = new SettingsWelcomePage(this);
// 	KPageWidgetItem* welcomeFrame = addPage(m_welcomePage, i18n("Welcome Screen"));
// 	welcomeFrame->setIcon(QIcon::fromTheme(QLatin1String("database-index")));
// 	connect(m_welcomePage, &SettingsWelcomePage::resetWelcomeScreen, this, &SettingsDialog::resetWelcomeScreen);

#ifdef HAVE_KUSERFEEDBACK
	auto* mainWin = static_cast<MainWin*>(parent);
	m_userFeedbackWidget = new KUserFeedback::FeedbackConfigWidget(this);
	m_userFeedbackWidget->setFeedbackProvider(&mainWin->userFeedbackProvider());
	connect(m_userFeedbackWidget, &KUserFeedback::FeedbackConfigWidget::configurationChanged, this, &SettingsDialog::changed);

	KPageWidgetItem* userFeedBackFrame = addPage(m_userFeedbackWidget, i18n("User Feedback"));
	userFeedBackFrame->setIcon(QIcon::fromTheme(QLatin1String("preferences-desktop-locale")));
#endif

	//restore saved settings if available
	create(); // ensure there's a window created
	KConfigGroup conf(KSharedConfig::openConfig(), "SettingsDialog");
	if (conf.exists()) {
		KWindowConfig::restoreWindowSize(windowHandle(), conf);
		resize(windowHandle()->size()); // workaround for QTBUG-40584
	} else
		resize(QSize(0, 0).expandedTo(minimumSize()));
}

SettingsDialog::~SettingsDialog() {
	KConfigGroup dialogConfig = KSharedConfig::openConfig()->group("SettingsDialog");
	KWindowConfig::saveWindowSize(windowHandle(), dialogConfig);
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
		if (KMessageBox::questionYesNo(this, text) == KMessageBox::Yes) {
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
	m_generalPage->applySettings();
	m_worksheetPage->applySettings();
	m_spreadsheetPage->applySettings();
	KSharedConfig::openConfig()->sync();

#ifdef HAVE_KUSERFEEDBACK
	auto* mainWin = static_cast<MainWin*>(parent());
	mainWin->userFeedbackProvider().setTelemetryMode(m_userFeedbackWidget->telemetryMode());
	mainWin->userFeedbackProvider().setSurveyInterval(m_userFeedbackWidget->surveyInterval());
#endif

	emit settingsChanged();
}

void SettingsDialog::restoreDefaults() {
	m_changed = false;
	m_generalPage->restoreDefaults();
	m_worksheetPage->restoreDefaults();
	m_spreadsheetPage->restoreDefaults();
}
