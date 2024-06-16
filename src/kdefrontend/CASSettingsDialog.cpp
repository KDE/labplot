/*
	File                 : CASSettingsDialog.cpp
	Project              : LabPlot
	Description          : CAS settings dialog
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "CASSettingsDialog.h"
#include "backend/core/Settings.h"

#include <KConfigDialog>
#include <KConfigGroup>
#include <KConfigSkeleton>
#include <KCoreConfigSkeleton>
#include <KLocalizedString>
#include <KWindowConfig>

#include <QWindow>

#ifdef HAVE_CANTOR_LIBS
#include <cantor/backend.h>
#endif

/**
 * \brief Settings dialog for CAS Backends.
 *
 * Contains the pages for settings of the CAS backends.
 *
 */
CASSettingsDialog::CASSettingsDialog(QWidget* parent)
	: KConfigDialog(parent, QLatin1String("Settings"), new KCoreConfigSkeleton()) {
	setWindowTitle(i18nc("@title:window", "CAS Preferences"));
	setWindowIcon(QIcon::fromTheme(QLatin1String("preferences-other")));
	setAttribute(Qt::WA_DeleteOnClose);

#ifdef HAVE_CANTOR_LIBS
	for (auto* backend : Cantor::Backend::availableBackends())
		if (backend->config()) // It has something to configure, so add it to the dialog
			this->addPage(backend->settingsWidget(this), backend->config(), backend->name(), backend->icon());
#endif

	// restore saved settings if available
	create(); // ensure there's a window created
	KConfigGroup conf = Settings::group(QStringLiteral("CASSettingsDialog"));
	if (conf.exists()) {
		KWindowConfig::restoreWindowSize(windowHandle(), conf);
		resize(windowHandle()->size()); // workaround for QTBUG-40584
	} else
		resize(QSize(0, 0).expandedTo(minimumSize()));
}

CASSettingsDialog::~CASSettingsDialog() {
	KConfigGroup dialogConfig = Settings::group(QStringLiteral("CASSettingsDialog"));
	KWindowConfig::saveWindowSize(windowHandle(), dialogConfig);
}
