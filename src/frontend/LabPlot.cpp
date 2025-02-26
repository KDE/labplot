/*
	File                 : LabPlot.cpp
	Project              : LabPlot
	Description          : main function
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2008-2025 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-FileCopyrightText: 2008-2016 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "MainWin.h"
#include "backend/core/AbstractColumn.h"
#include "backend/core/Settings.h"
#include "backend/lib/macros.h"
#include "frontend/AboutDialog.h"

#include <KAboutData>
#include <KAboutComponent>
#include <KColorSchemeManager>
#include <KConfigGroup>
#include <KCrash>
#include <KIconTheme>
#include <KLocalizedString>
#include <KMessageBox>
#include <kcoreaddons_version.h>

#define HAVE_STYLE_MANAGER __has_include(<KStyleManager>)
#if HAVE_STYLE_MANAGER
#include <KStyleManager>
#endif

#include <QApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QFile>
#include <QModelIndex>
#include <QSplashScreen>
#include <QSysInfo>

#ifdef _WIN32
#include <windows.h>
#endif

int main(int argc, char* argv[]) {
#if defined(Q_OS_UNIX) && !defined(Q_OS_DARWIN)
	// the qads library has issues on wayland so we force qt to use x11 instead
	// see https://invent.kde.org/education/labplot/-/issues/1067
	QByteArray xcbQtQpaEnvVar("xcb");
	qputenv("QT_QPA_PLATFORM", xcbQtQpaEnvVar);
#endif

	// trigger initialisation of proper icon theme
	KIconTheme::initTheme();

	QApplication app(argc, argv);

#if HAVE_STYLE_MANAGER
	//trigger initialisation of proper application style
	KStyleManager::initStyle();
#else
	// For Windows and macOS: use Breeze if available
	// Of all tested styles that works the best for us
#if defined(Q_OS_MACOS) || defined(Q_OS_WIN)
	QApplication::setStyle(QStringLiteral("breeze"));
#endif
#endif

	KLocalizedString::setApplicationDomain("labplot");

	KAboutData aboutData(QStringLiteral("labplot"),
						 QStringLiteral("LabPlot"),
						 QLatin1String(LVERSION),
						 i18n("LabPlot is a FREE, open-source and cross-platform Data Visualization and Analysis software accessible to everyone."),
						 KAboutLicense::GPL,
						 i18n("(c) 2007-%1 LabPlot Team", QLatin1String(YEAR)),
						 AboutDialog::systemInfo() + AboutDialog::links());
	aboutData.addAuthor(i18n("Stefan Gerlach"), i18nc("@info:credit", "Developer"), QStringLiteral("stefan.gerlach@uni.kn"), QString());
	aboutData.addAuthor(i18n("Alexander Semke"), i18nc("@info:credit", "Developer"), QStringLiteral("alexander.semke@web.de"), QString());
	aboutData.addAuthor(i18n("Fábián Kristóf-Szabolcs"), i18nc("@info:credit", "Developer"), QStringLiteral("f-kristof@hotmail.com"), QString());
	aboutData.addAuthor(i18n("Martin Marmsoler"), i18nc("@info:credit", "Developer"), QStringLiteral("martin.marmsoler@gmail.com"), QString());
	aboutData.addAuthor(i18n("Dariusz Laska"),
						i18nc("@info:credit", "Conceptual work, documentation, example projects"),
						QStringLiteral("dariuszlaska@gmail.com"),
						QString());
	aboutData.addAuthor(i18n("Andreas Kainz"), i18nc("@info:credit", "Icon designer"), QStringLiteral("kainz.a@gmail.com"), QString());
	// no text (link to bugs.kde.org) before authors list
	aboutData.setCustomAuthorText(QString(), QString());

	aboutData.addCredit(i18n("Yuri Chornoivan"),
						i18nc("@info:credit", "Help on many questions about the KDE-infrastructure and translation related topics"),
						QStringLiteral("yurchor@ukr.net"),
						QString());
	aboutData.addCredit(i18n("Garvit Khatri"),
						i18nc("@info:credit", "Porting LabPlot to KF5 and Integration with Cantor"),
						QStringLiteral("garvitdelhi@gmail.com"),
						QString());
	aboutData.addCredit(i18n("Christoph Roick"),
						i18nc("@info:credit", "Support import of ROOT (CERN) TH1 histograms"),
						QStringLiteral("chrisito@gmx.de"),
						QString());
	aboutData.setOrganizationDomain(QByteArray("kde.org"));
	aboutData.setDesktopFileName(QStringLiteral("org.kde.labplot"));
	aboutData.setProgramLogo(QIcon::fromTheme(QStringLiteral("labplot")));

	const auto& group = Settings::settingsGeneral();
	enableInfoTrace(group.readEntry<bool>(QLatin1String("InfoTrace"), false));
	enableDebugTrace(group.readEntry<bool>(QLatin1String("DebugTrace"), false));
	enablePerfTrace(group.readEntry<bool>(QLatin1String("PerfTrace"), false));

#ifdef _WIN32
	// enable debugging on console
	if (AttachConsole(ATTACH_PARENT_PROCESS)) {
		freopen("CONOUT$", "w", stdout);
		freopen("CONOUT$", "w", stderr);
	}
#endif
	INFO("INFO messages enabled")
	DEBUG("DEBUG debugging enabled")
	QDEBUG("QDEBUG debugging enabled")

	// components
	for (auto c: AboutDialog::components())
		aboutData.addComponent(c.at(0), c.at(1), c.at(2), c.at(3));

	// no translators set (too many to mention)
	KAboutData::setApplicationData(aboutData);

	KCrash::initialize();

	QCommandLineParser parser;

	QCommandLineOption nosplashOption(QStringLiteral("no-splash"), i18n("Disable splash screen"));
	parser.addOption(nosplashOption);

	QCommandLineOption presenterOption(QStringLiteral("presenter"), i18n("Start in the presenter mode"));
	parser.addOption(presenterOption);

	parser.addPositionalArgument(QStringLiteral("+[file]"), i18n("Open a project file."));

	aboutData.setupCommandLine(&parser);
	parser.process(app);
	aboutData.processCommandLine(&parser);

	const auto args = parser.positionalArguments();
	QString fileName;
	if (args.count() > 0)
		fileName = args[0];

	if (!fileName.isEmpty()) {
		// determine the absolute file path in order to properly save it in MainWin in "Recent Files"
		QDir dir;
		fileName = dir.absoluteFilePath(fileName);

		if (!QFile::exists(fileName)) {
			if (KMessageBox::warningContinueCancel(
					nullptr,
					i18n(R"(Could not open file '%1'. Click 'Continue' to proceed starting or 'Cancel' to exit the application.)", fileName),
					i18n("Failed to Open"))
				== KMessageBox::Cancel) {
				exit(-1); //"Cancel" clicked -> exit the application
			} else {
				fileName.clear(); // Wrong file -> clear the file name and continue
			}
		}
	}

	QSplashScreen* splash = nullptr;
	if (!parser.isSet(nosplashOption)) {
		const QString& file = QStandardPaths::locate(QStandardPaths::AppDataLocation, QStringLiteral("splash.png"));
		splash = new QSplashScreen(QPixmap(file));
		splash->show();
	}

	// needed in order to have the signals triggered by SignallingUndoCommand
	// TODO: redesign/remove this
	qRegisterMetaType<const AbstractAspect*>("const AbstractAspect*");
	qRegisterMetaType<const AbstractColumn*>("const AbstractColumn*");

	INFO("Current path: " << STDSTRING(QDir::currentPath()))
	const QString applicationPath = QCoreApplication::applicationDirPath();
	INFO("Application dir: " << STDSTRING(applicationPath))

#ifdef _WIN32
	// append application path to PATH to find Cantor backends
	QString path = qEnvironmentVariable("PATH");
	INFO("Old PATH = " << STDSTRING(path))
	path.append(QLatin1String(";") + applicationPath);
	qputenv("PATH", qPrintable(path));
	INFO("New PATH = " << STDSTRING(qEnvironmentVariable("PATH")))
#endif

#if !defined(NDEBUG) || defined(Q_OS_WIN) || defined(Q_OS_MACOS)
	// debugging paths
	const auto& appdatapaths = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);
	INFO("AppDataLocation paths:")
	for (const auto& path : appdatapaths)
		WARN("	" << STDSTRING(path))
	INFO("Icon theme search paths:")
	for (const auto& path : QIcon::themeSearchPaths())
		WARN("	" << STDSTRING(path))
	INFO("Library search paths:")
	for (const auto& path : QCoreApplication::libraryPaths())
		WARN("	" << STDSTRING(path))
#endif

	QString schemeName = group.readEntry("ColorScheme");
	KColorSchemeManager manager;
	manager.activateScheme(manager.indexForScheme(schemeName));

#if defined(Q_OS_MACOS) || defined(Q_OS_WIN)
	QApplication::setStyle(QStringLiteral("breeze"));
#endif

	auto* window = new MainWin(nullptr, fileName);
	window->show();

	if (splash) {
		splash->finish(window);
		delete splash;
	}

	if (parser.isSet(presenterOption))
		window->showPresenter();

	return app.exec();
}
