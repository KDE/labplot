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
#include <Python.h>
#endif

#include <KConfigGroup>
#include <KMessageWidget>

#include <QDir>
#include <QDirIterator>
#include <QFileDialog>
#include <QFileInfo>
#include <QIcon>
#include <QProcess>
#include <QPushButton>
#include <QStandardPaths>
#include <QTimer>

/**
 * \brief Page for the 'General' settings of the Labplot settings dialog.
 */
SettingsScriptingPage::SettingsScriptingPage(QWidget* parent)
	: SettingsPage(parent) {
	ui.setupUi(this);

	// show information about the current python runtime
#ifdef HAVE_PYTHON_SCRIPTING
	ui.tbAddPythonEnvironment->setToolTip(i18n("Select the path to a python virtual environment."));

	ui.tbAddPythonEnvironment->setIcon(QIcon::fromTheme(QStringLiteral("document-open")));
	ui.lPythonVersion->setText(QString::fromUtf8(Py_GetVersion()).section(QLatin1Char(' '), 0, 0));
	if (Py_IsInitialized()) {
		ui.lPythonPath->setText(QString::fromWCharArray(Py_GetPrefix()));
	} else {
		// Python not yet initialized — hide path
		ui.lPythonPathTitle->setVisible(false);
		ui.lPythonPath->setVisible(false);
	}

	// populate available python environments
	ui.cbPythonEnvironment->addItem(i18n("System Default"), QString());
	discoverEnvironments();

	// load the current selected environment
	loadSettings();

	connect(ui.tbAddPythonEnvironment, &QPushButton::clicked, this, [&] {
		showErrorMessage(QString());
		const QString path = QFileDialog::getExistingDirectory(this, i18nc("@title:window", "Select the Virtual Environment"), QDir::homePath(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
		if (!path.isEmpty())
			this->addEnvironment(path, true, false, true);
	});
	connect(ui.cbPythonEnvironment, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsScriptingPage::changed);
#else
	ui.lPythonVersion->setText(QLatin1String("<font color=\"red\">") + i18n("missing") + QLatin1String("</font>"));
	ui.lPythonPathTitle->setVisible(false);
	ui.lPythonPath->setVisible(false);
	ui.cbPythonEnvironment->setVisible(false);
	ui.lPythonEnvironment->setVisible(false);
	ui.tbAddPythonEnvironment->setVisible(false);
#endif
}

QList<Settings::Type> SettingsScriptingPage::applySettings() {
	QList<Settings::Type> changes;
	if (!m_changed)
		return changes;

	QString executablePath = ui.cbPythonEnvironment->currentData().toString();
	QStringList executablePaths;
	for (int i = 0; i < ui.cbPythonEnvironment->count(); ++i) {
		const QString path = ui.cbPythonEnvironment->itemData(i).toString();
		if (!path.isEmpty() && !executablePaths.contains(path))
			executablePaths << path;
	}

	KConfigGroup group = Settings::group(QStringLiteral("Settings_Scripting"));
	group.writeEntry(QLatin1String("PythonExecutable"), executablePath); // save the python path for the currently selected environment
	group.writeEntry(QLatin1String("PythonExecutables"), executablePaths); // save the python paths for all discovered and user-provided environments
	return changes;
}

void SettingsScriptingPage::restoreDefaults() {
	ui.cbPythonEnvironment->setCurrentIndex(0); // "System Default"
}

void SettingsScriptingPage::loadSettings() {
	const KConfigGroup group = Settings::group(QStringLiteral("Settings_Scripting"));
	const QStringList savedPaths = group.readEntry(QLatin1String("PythonExecutables"), QStringList());
	const QString savedPath = group.readEntry(QLatin1String("PythonExecutable"), QString()); // load the currently selected environment

	// show all saved environments that were discovered and user-provided before
	for (const auto& path : savedPaths) {
		if (path.isEmpty())
			continue;

		bool exists = false;
		for (int i = 0; i < ui.cbPythonEnvironment->count(); ++i) {
			if (ui.cbPythonEnvironment->itemData(i).toString() == path) {
				exists = true;
				break;
			}
		}
		if (exists)
			continue;

#ifdef Q_OS_WIN
		QString executablePath = QStringLiteral("/Scripts/python.exe");
#else
		QString executablePath = QStringLiteral("/bin/python");
#endif
		if (path.endsWith(executablePath))
			addEnvironment(path.chopped(executablePath.length()), false, true);
		else if (path.endsWith(executablePath + QStringLiteral("/")))
			addEnvironment(path.chopped(executablePath.length() + 1), false, true);
	}

	// show the last selected environment if it is still available, otherwise show "System Default"
	if (!savedPath.isEmpty()) {
		for (int i = 0; i < ui.cbPythonEnvironment->count(); ++i) {
			if (ui.cbPythonEnvironment->itemData(i).toString() == savedPath) {
				ui.cbPythonEnvironment->setCurrentIndex(i);
				return;
			}
		}
#ifdef Q_OS_WIN
		QString executablePath = QStringLiteral("/Scripts/python.exe");
#else
		// On Unix, look specifically for lib/python<minor>/site-packages
		QString executablePath = QStringLiteral("/bin/python");
#endif
		if (savedPath.endsWith(executablePath))
			addEnvironment(savedPath.chopped(executablePath.length()), true, true);
		else if (savedPath.endsWith(executablePath + QStringLiteral("/")))
			addEnvironment(savedPath.chopped(executablePath.length() + 1), true, true);
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
	const QString pyVersion = QString::fromUtf8(Py_GetVersion()).section(QLatin1Char(' '), 0, 0);

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
 * Add a discovered environment to the combo box, using the directory name
 * as the display label and the site-packages path as item data.
 * Skips duplicates.
 */
void SettingsScriptingPage::addEnvironment(const QString& rawEnvPath, bool selectAfterAdd, bool blockSignalsDuringSelect, bool isUserSelected) {
	// Derive site-packages path — only accept environments matching the runtime Python minor version
	const QString envPath = QDir::cleanPath(QFileInfo(rawEnvPath).absoluteFilePath());
	QString executablePath;
#ifdef Q_OS_WIN
	executablePath = envPath + QStringLiteral("/Scripts/python.exe");
#else
	// On Unix, look specifically for lib/python<minor>/site-packages
	executablePath = envPath + QStringLiteral("/bin/python");
#endif
	QString cfgPath = envPath + QStringLiteral("/pyvenv.cfg");

	if (!QFile(executablePath).exists()) {
		if (isUserSelected)
			showErrorMessage(i18n("The selected virtual environment does not contain a python executable."));
		return;
	}

	if (!QFile(cfgPath).exists()) {
		if (isUserSelected)
			showErrorMessage(i18n("The selected virtual environment does not contain a pyvenv.cfg file."));
		return;
	}

	// Skip if already added
	for (int i = 0; i < ui.cbPythonEnvironment->count(); ++i) {
		if (ui.cbPythonEnvironment->itemData(i).toString() == executablePath) {
			if (selectAfterAdd)
				ui.cbPythonEnvironment->setCurrentIndex(i);
			return;
		}
	}

	// try to read the venv version from pyvenv.cfg file
	QFile cfgFile(QDir(envPath).absoluteFilePath(QStringLiteral("pyvenv.cfg")));
	if (cfgFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&cfgFile);
		while (!in.atEnd()) {
			const QString line = in.readLine().trimmed();
			// we found the version line in the pyvenv.cfg file
			if (line.startsWith(QStringLiteral("version"))) {
				const QString venvVersion = line.section(QLatin1Char('='), 1).trimmed();
				if ((venvVersion == m_pythonMinorVersion) || venvVersion.startsWith(m_pythonMinorVersion + QStringLiteral("."))) {
					ui.cbPythonEnvironment->addItem(envPath, executablePath);
					if (selectAfterAdd)
						ui.cbPythonEnvironment->setCurrentText(envPath);
				} else {
					if (isUserSelected)
						showErrorMessage(i18n("The selected virtual environment python version does not match the application python version."));
				}
				return;
			}
		}
	}

	// fallback to process method if we cannot read venv version from pyvenv.cfg file
	auto* process = new QProcess(this);
	connect(process, &QProcess::finished, this, [process, envPath, executablePath, this, selectAfterAdd, blockSignalsDuringSelect, isUserSelected](int exitCode, QProcess::ExitStatus exitStatus) {
		if (exitStatus == QProcess::NormalExit && exitCode == 0) {
			const QString version = QString::fromUtf8(process->readAllStandardOutput()).trimmed();
			if (version == m_pythonMinorVersion) {
				bool alreadyAdded = false;
				for (int i = 0; i < ui.cbPythonEnvironment->count(); ++i) {
					if (ui.cbPythonEnvironment->itemData(i).toString() == executablePath)
						alreadyAdded = true;
				}
				if (!alreadyAdded)
					ui.cbPythonEnvironment->addItem(envPath, executablePath);
				if (selectAfterAdd) {
					if (blockSignalsDuringSelect) {
						ui.cbPythonEnvironment->blockSignals(true);
						ui.cbPythonEnvironment->setCurrentText(envPath);
						ui.cbPythonEnvironment->blockSignals(false);
					} else {
						ui.cbPythonEnvironment->setCurrentText(envPath);
					}
				}
			} else {
				if (isUserSelected)
					showErrorMessage(i18n("The selected virtual environment python version does not match the application python version."));
			}
		}
		process->deleteLater();
	});

	connect(process, &QProcess::errorOccurred, this, [process](QProcess::ProcessError error) {
		Q_UNUSED(error);
		process->deleteLater();
	});

	process->start(executablePath, {QStringLiteral("-c"), QStringLiteral("import sys; print(f'{sys.version_info.major}.{sys.version_info.minor}')")});
}

void SettingsScriptingPage::showErrorMessage(const QString& message) {
	if (message.isEmpty()) {
		if (m_messageWidget && m_messageWidget->isVisible())
			m_messageWidget->close();
	} else {
		if (!m_messageWidget) {
			m_messageWidget = new KMessageWidget(this);
			m_messageWidget->setMessageType(KMessageWidget::Error);
			ui.gridLayout->addWidget(m_messageWidget, 6, 0, 1, 4);
		}
		m_messageWidget->setText(message);
		m_messageWidget->animatedShow();
		QTimer::singleShot(10000, [this] {
			if (m_messageWidget && m_messageWidget->isVisible())
				m_messageWidget->close();
		});
	}
}
