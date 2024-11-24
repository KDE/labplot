/*
	File                 : LabPlot.cpp
	Project              : LabPlot
	Description          : main function
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2008-2024 Stefan Gerlach <stefan.gerlach@uni.kn>
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
#include <KLocalizedString>
#include <KMessageBox>
#include <kcoreaddons_version.h>

#include <QApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QFile>
#include <QModelIndex>
#include <QSplashScreen>
#include <QSysInfo>

//#ifdef _WIN32
//#include <windows.h>
//#endif

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);
	KLocalizedString::setApplicationDomain("labplot");
	MainWin::updateLocale();

	QString systemInfo{AboutDialog::systemInfo()};
	QString links = i18n("Visit website:") + QLatin1Char(' ') + QStringLiteral("<a href=\"%1\">%1</a>").arg(QStringLiteral("labplot.kde.org")) + QLatin1Char('\n')
		// Release notes: LINK ?
		+ i18n("Mastodon:") + QLatin1Char(' ') + QStringLiteral("<a href=\"%1\">%1</a>").arg(QStringLiteral("floss.social/@LabPlot")) + QLatin1Char('\n')
		+ i18n("Watch video tutorials:") + QLatin1Char(' ') + QStringLiteral("<a href=\"%1\">%1</a>").arg(QStringLiteral("tube.kockatoo.org/c/labplot")) + QLatin1Char('\n')
		+ i18n("Please report bugs to:") + QLatin1Char(' ') + QStringLiteral("<a href=\"%1\">%1</a>").arg(QStringLiteral("bugs.kde.org"));
	KAboutData aboutData(QStringLiteral("labplot"),
						 QStringLiteral("LabPlot"),
						 QLatin1String(LVERSION),
						 i18n("LabPlot is a FREE, open-source and cross-platform Data Visualization and Analysis software accessible to everyone."),
						 KAboutLicense::GPL,
						 i18n("(c) 2007-2024 LabPlot authors"),
						 systemInfo + QLatin1Char('\n') + links);
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

	// components
	for (auto c: AboutDialog::components())
		aboutData.addComponent(c.at(0), c.at(1), c.at(2), c.at(3));

	// no translators set (too many to mention)
	KAboutData::setApplicationData(aboutData);

	KCrash::initialize();

	const auto& group = Settings::settingsGeneral();
	enableInfoTrace(group.readEntry<bool>(QLatin1String("InfoTrace"), false));
	enableDebugTrace(group.readEntry<bool>(QLatin1String("DebugTrace"), false));
	enablePerfTrace(group.readEntry<bool>(QLatin1String("PerfTrace"), false));

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
	QString filename;
	if (args.count() > 0)
		filename = args[0];

	if (!filename.isEmpty()) {
		// determine the absolute file path in order to properly save it in MainWin in "Recent Files"
		QDir dir;
		filename = dir.absoluteFilePath(filename);

		if (!QFile::exists(filename)) {
			if (KMessageBox::warningContinueCancel(
					nullptr,
					i18n(R"(Could not open file '%1'. Click 'Continue' to proceed starting or 'Cancel' to exit the application.)", filename),
					i18n("Failed to Open"))
				== KMessageBox::Cancel) {
				exit(-1); //"Cancel" clicked -> exit the application
			} else {
				filename.clear(); // Wrong file -> clear the file name and continue
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

#ifdef _WIN32
	// enable debugging on console
	if (AttachConsole(ATTACH_PARENT_PROCESS)) {
		freopen("CONOUT$", "w", stdout);
		freopen("CONOUT$", "w", stderr);
	}
#endif
	DEBUG("DEBUG debugging enabled")
	QDEBUG("QDEBUG debugging enabled")

	DEBUG("Current path: " << STDSTRING(QDir::currentPath()))
	const QString applicationPath = QCoreApplication::applicationDirPath();
	DEBUG("Application dir: " << STDSTRING(applicationPath))

#ifdef _WIN32
	// append application path to PATH to find Cantor backends
	QString path = qEnvironmentVariable("PATH");
	DEBUG("OLD PATH = " << STDSTRING(path))
	path.append(QLatin1String(";") + applicationPath);
	qputenv("PATH", qPrintable(path));
	DEBUG("NEW PATH = " << STDSTRING(qEnvironmentVariable("PATH")))
#endif

#if !defined(NDEBUG) || defined(Q_OS_WIN) || defined(Q_OS_MACOS)
	// debugging paths
	const auto& appdatapaths = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);
	WARN("AppDataLocation paths:")
	for (const auto& path : appdatapaths)
		WARN("	" << STDSTRING(path))
	WARN("Icon theme search paths:")
	for (const auto& path : QIcon::themeSearchPaths())
		WARN("	" << STDSTRING(path))
	WARN("Library search paths:")
	for (const auto& path : QCoreApplication::libraryPaths())
		WARN("	" << STDSTRING(path))
#endif

	QString schemeName = group.readEntry("ColorScheme");
	KColorSchemeManager manager;
	manager.activateScheme(manager.indexForScheme(schemeName));

#if defined(Q_OS_MACOS) || defined(Q_OS_WIN)
	QApplication::setStyle(QStringLiteral("breeze"));
#endif

	auto* window = new MainWin(nullptr, filename);
	window->show();

	if (splash) {
		splash->finish(window);
		delete splash;
	}

	if (parser.isSet(presenterOption))
		window->showPresenter();

	return app.exec();
}
