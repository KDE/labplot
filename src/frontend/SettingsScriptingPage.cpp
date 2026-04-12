/*
	File                 : SettingsScriptingPage.cpp
	Project              : LabPlot
	Description          : settings page for Scripting
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2026 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "SettingsScriptingPage.h"
#include "backend/core/Settings.h"

#ifdef HAVE_PYTHON_SCRIPTING
#include "backend/script/python/PythonScriptingInfo.h"
#endif

#include <KConfigGroup>

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QStandardPaths>

/**
 * \brief Page for the 'General' settings of the Labplot settings dialog.
 */
SettingsScriptingPage::SettingsScriptingPage(QWidget* parent)
	: SettingsPage(parent) {
	ui.setupUi(this);

	// show information about the current python runtime
#ifdef HAVE_PYTHON_SCRIPTING
	if (PythonScriptingInfo::isInitialized()) {
		ui.lPythonVersion->setText(PythonScriptingInfo::version());
		ui.lPythonPath->setText(PythonScriptingInfo::prefix());
	} else {
		// Python not yet initialized — show build-time version, hide path
		ui.lPythonVersion->setText(QStringLiteral(PYTHON3_VERSION_STRING));
		ui.lPythonPathTitle->setVisible(false);
		ui.lPythonPath->setVisible(false);
	}
#endif

	// populate available python environments
	ui.cbPythonEnvironment->addItem(i18n("System Default"));
	discoverEnvironments();

	connect(ui.cbPythonEnvironment, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsScriptingPage::changed);

	// load the current selected environment
	loadSettings();
}

QList<Settings::Type> SettingsScriptingPage::applySettings() {
	QList<Settings::Type> changes;
	if (!m_changed)
		return changes;

	KConfigGroup group = Settings::group(QStringLiteral("Settings_Scripting"));
	group.writeEntry(QLatin1String("PythonEnvironment"), ui.cbPythonEnvironment->currentData().toString());

	return changes;
}

void SettingsScriptingPage::restoreDefaults() {
	ui.cbPythonEnvironment->setCurrentIndex(0); // "System Default"
}

void SettingsScriptingPage::loadSettings() {
	const KConfigGroup group = Settings::group(QStringLiteral("Settings_Scripting"));
	const QString savedPath = group.readEntry(QLatin1String("PythonEnvironment"), QString());

	if (!savedPath.isEmpty()) {
		for (int i = 0; i < ui.cbPythonEnvironment->count(); ++i) {
			if (ui.cbPythonEnvironment->itemData(i).toString() == savedPath) {
				ui.cbPythonEnvironment->setCurrentIndex(i);
				return;
			}
		}
	}
}

void SettingsScriptingPage::changed() {
	m_changed = true;
	Q_EMIT settingsChanged();
}

/*!
 * Scan standard locations for virtual environments and conda environments
 * and add them to the environment combo box.
 */
void SettingsScriptingPage::discoverEnvironments() {
	// determine the runtime Python minor version for environment compatibility filtering
#ifdef HAVE_PYTHON_SCRIPTING
	const QString pyVersion = PythonScriptingInfo::isInitialized()
		? PythonScriptingInfo::version()
		: QStringLiteral(PYTHON3_VERSION_STRING);
#else
	const QString pyVersion = QStringLiteral(PYTHON3_VERSION_STRING);
#endif
	m_pythonMinorVersion = pyVersion.section(QLatin1Char('.'), 0, 1); // e.g. "3.11"

	const QString home = QDir::homePath();
	QStringList searchDirs;

	// --- Virtual environments ---
	// Common virtualenv/venv wrapper locations
	searchDirs << home + QStringLiteral("/.virtualenvs"); // virtualenvwrapper default
	searchDirs << home + QStringLiteral("/.venv"); // single-project convention
	searchDirs << home + QStringLiteral("/.local/share/virtualenvs"); // pipenv default

	// Poetry: platform-specific cache locations
#if defined(Q_OS_MACOS)
	searchDirs << home + QStringLiteral("/Library/Caches/pypoetry/virtualenvs");
#elif defined(Q_OS_WIN)
	const QString appData = QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation);
	searchDirs << appData + QStringLiteral("/pypoetry/virtualenvs");
#else // Linux
	searchDirs << home + QStringLiteral("/.cache/pypoetry/virtualenvs");
#endif

	for (const auto& dir : searchDirs)
		scanForVenvs(dir);

	// --- Conda environments ---
	// Read ~/.conda/environments.txt (conda's own registry, cross-platform)
	const QString condaEnvsFile = home + QStringLiteral("/.conda/environments.txt");
	if (QFileInfo::exists(condaEnvsFile)) {
		QFile file(condaEnvsFile);
		if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
			while (!file.atEnd()) {
				const QString line = QString::fromUtf8(file.readLine()).trimmed();
				if (!line.isEmpty() && QDir(line).exists())
					addEnvironment(line);
			}
		}
	}

	// Also scan standard conda env directories
	QStringList condaDirs;
	condaDirs << home + QStringLiteral("/miniconda3/envs");
	condaDirs << home + QStringLiteral("/anaconda3/envs");
	condaDirs << home + QStringLiteral("/miniforge3/envs");
	condaDirs << home + QStringLiteral("/mambaforge/envs");
#ifdef Q_OS_WIN
	condaDirs << home + QStringLiteral("/Miniconda3/envs");
	condaDirs << home + QStringLiteral("/Anaconda3/envs");
#endif
	for (const auto& dir : condaDirs)
		scanForCondaEnvs(dir);
}

/*!
 * Scan a directory for Python virtual environments (identified by pyvenv.cfg).
 */
void SettingsScriptingPage::scanForVenvs(const QString& dir) {
	if (!QDir(dir).exists())
		return;

	// Check if dir itself is a venv
	if (QFileInfo::exists(dir + QStringLiteral("/pyvenv.cfg"))) {
		addEnvironment(dir);
		return;
	}

	// Check immediate subdirectories
	const QDir parent(dir);
	const auto entries = parent.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
	for (const auto& entry : entries) {
		const QString path = dir + QLatin1Char('/') + entry;
		if (QFileInfo::exists(path + QStringLiteral("/pyvenv.cfg")))
			addEnvironment(path);
	}
}

/*!
 * Scan a conda envs directory for environments (identified by conda-meta/).
 */
void SettingsScriptingPage::scanForCondaEnvs(const QString& dir) {
	if (!QDir(dir).exists())
		return;

	const QDir parent(dir);
	const auto entries = parent.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
	for (const auto& entry : entries) {
		const QString path = dir + QLatin1Char('/') + entry;
		if (QDir(path + QStringLiteral("/conda-meta")).exists())
			addEnvironment(path);
	}
}

/*!
 * Add a discovered environment to the combo box, using the directory name
 * as the display label and the site-packages path as item data.
 * Skips duplicates.
 */
void SettingsScriptingPage::addEnvironment(const QString& envPath) {
	// Derive site-packages path — only accept environments matching the runtime Python minor version
	QString sitePackages;
#ifdef Q_OS_WIN
	// On Windows, Lib/site-packages doesn't encode the version, so check pyvenv.cfg
	const QString cfgPath = envPath + QStringLiteral("/pyvenv.cfg");
	if (QFileInfo::exists(cfgPath)) {
		QFile cfg(cfgPath);
		if (cfg.open(QIODevice::ReadOnly | QIODevice::Text)) {
			while (!cfg.atEnd()) {
				const QString line = QString::fromUtf8(cfg.readLine()).trimmed();
				if (line.startsWith(QLatin1String("version"))) {
					const QString ver = line.section(QLatin1Char('='), 1).trimmed();
					const QString minor = ver.section(QLatin1Char('.'), 0, 1);
					if (minor != m_pythonMinorVersion)
						return; // incompatible Python version
					break;
				}
			}
		}
	}
	sitePackages = envPath + QStringLiteral("/Lib/site-packages");
#else
	// On Unix, look specifically for lib/python<minor>/site-packages
	sitePackages = envPath + QStringLiteral("/lib/") + m_pythonMinorVersion + QStringLiteral("/site-packages");
#endif

	if (sitePackages.isEmpty() || !QDir(sitePackages).exists())
		return;

	// Skip if already added
	for (int i = 0; i < ui.cbPythonEnvironment->count(); ++i) {
		if (ui.cbPythonEnvironment->itemData(i).toString() == sitePackages)
			return;
	}

	const QString name = QFileInfo(envPath).fileName();
	ui.cbPythonEnvironment->addItem(name, sitePackages);
}
