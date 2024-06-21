/*
	File                 : LabPlot.cpp
	Project              : LabPlot
	Description          : main function
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2008 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-FileCopyrightText: 2008-2016 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "MainWin.h"
#include "backend/core/AbstractColumn.h"
#include "backend/core/Settings.h"
#include "backend/lib/macros.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"

#include <KAboutData>
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
#include <QSettings>
#include <QSplashScreen>
#include <QStandardPaths>
#include <QSysInfo>

#ifdef _WIN32
#include <windows.h>
#endif

/*
 * collect all system info to show in About dialog
 */
const QString getSystemInfo() {
	// build type
#ifdef NDEBUG
	const QString buildType(i18n("Release build ") + QLatin1String(GIT_COMMIT));
#else
	const QString buildType(i18n("Debug build ") + QLatin1String(GIT_COMMIT));
#endif
	QLocale locale;
	const QString numberSystemInfo{QStringLiteral("(") + i18n("Decimal point ") + QLatin1Char('\'') + QString(locale.decimalPoint()) + QLatin1String("\', ")
								   + i18n("Group separator ") + QLatin1Char('\'') + QString(locale.groupSeparator()) + QLatin1Char('\'')};

	const QString numberLocaleInfo{QStringLiteral(" ") + i18n("Decimal point ") + QLatin1Char('\'') + QString(QLocale().decimalPoint()) + QLatin1String("\', ")
								   + i18n("Group separator ") + QLatin1Char('\'') + QString(QLocale().groupSeparator()) + QLatin1String("\', ")
								   + i18n("Exponential ") + QLatin1Char('\'') + QString(QLocale().exponential()) + QLatin1String("\', ") + i18n("Zero digit ")
								   + QLatin1Char('\'') + QString(QLocale().zeroDigit()) + QLatin1String("\', ") + i18n("Percent ") + QLatin1Char('\'')
								   + QString(QLocale().percent()) + QLatin1String("\', ") + i18n("Positive/Negative sign ") + QLatin1Char('\'')
								   + QString(QLocale().positiveSign()) + QLatin1Char('\'') + QLatin1Char('/') + QLatin1Char('\'')
								   + QString(QLocale().negativeSign()) + QLatin1Char('\'')};

	// get language set in 'switch language'
	const QString configPath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
	QSettings languageoverride(configPath + QStringLiteral("/klanguageoverridesrc"), QSettings::IniFormat);
	languageoverride.beginGroup(QStringLiteral("Language"));
	QString usedLocale = languageoverride.value(qAppName(), QString()).toString(); // something like "en_US"
	if (!usedLocale.isEmpty())
		locale = QLocale(usedLocale);
	QString usedLanguage = QLocale::languageToString(locale.language()) + QStringLiteral(",") + QLocale::countryToString(locale.country());

	return buildType + QLatin1Char('\n')
#ifndef REPRODUCIBLE_BUILD
		+ QStringLiteral("%1, %2").arg(QLatin1String(__DATE__), QLatin1String(__TIME__)) + QLatin1Char('\n')
#endif
		+ i18n("System: ") + QSysInfo::prettyProductName() + QLatin1Char('\n') + i18n("Locale: ") + usedLanguage + QLatin1Char(' ') + numberSystemInfo
		+ QLatin1Char('\n') + i18n("Number settings:") + numberLocaleInfo + QLatin1String(" (") + i18n("Updated on restart") + QLatin1Char(')')
		+ QLatin1Char('\n') + i18n("Architecture: ") + QSysInfo::buildAbi() + QLatin1Char('\n') + i18n("Kernel: ") + QSysInfo::kernelType() + QLatin1Char(' ')
		+ QSysInfo::kernelVersion() + QLatin1Char('\n') + i18n("C++ Compiler: ") + QLatin1String(CXX_COMPILER) + QLatin1Char('\n')
		+ i18n("C++ Compiler Flags: ") + QLatin1String(CXX_COMPILER_FLAGS);
}

int main(int argc, char* argv[]) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
	QApplication app(argc, argv);
	KLocalizedString::setApplicationDomain("labplot2");
	KCrash::initialize();

	MainWin::updateLocale();

	QString systemInfo{getSystemInfo()};

	KAboutData aboutData(QStringLiteral("labplot2"),
						 QStringLiteral("LabPlot"),
						 QLatin1String(LVERSION),
						 i18n("LabPlot is a FREE, open-source and cross-platform Data Visualization and Analysis software accessible to everyone."),
						 KAboutLicense::GPL,
						 i18n("(c) 2007-2024"),
						 systemInfo,
						 QStringLiteral("https://labplot.kde.org"));
	aboutData.addAuthor(i18n("Stefan Gerlach"), i18nc("@info:credit", "Developer"), QStringLiteral("stefan.gerlach@uni.kn"), QString());
	aboutData.addAuthor(i18n("Alexander Semke"), i18nc("@info:credit", "Developer"), QStringLiteral("alexander.semke@web.de"), QString());
	aboutData.addAuthor(i18n("Fábián Kristóf-Szabolcs"), i18nc("@info:credit", "Developer"), QStringLiteral("f-kristof@hotmail.com"), QString());
	aboutData.addAuthor(i18n("Martin Marmsoler"), i18nc("@info:credit", "Developer"), QStringLiteral("martin.marmsoler@gmail.com"), QString());
	aboutData.addAuthor(i18n("Dariusz Laska"),
						i18nc("@info:credit", "Conceptual work, documentation, example projects"),
						QStringLiteral("dariuszlaska@gmail.com"),
						QString());
	aboutData.addAuthor(i18n("Andreas Kainz"), i18nc("@info:credit", "Icon designer"), QStringLiteral("kainz.a@gmail.com"), QString());
	aboutData.addCredit(i18n("Yuri Chornoivan"),
						i18nc("@info:credit", "Help on many questions about the KDE-infrastructure and translation related topics"),
						QStringLiteral("yurchor@ukr.net"),
						QString());
	aboutData.addCredit(i18n("Garvit Khatri"),
						i18nc("@info:credit", "Porting LabPlot2 to KF5 and Integration with Cantor"),
						QStringLiteral("garvitdelhi@gmail.com"),
						QString());
	aboutData.addCredit(i18n("Christoph Roick"),
						i18nc("@info:credit", "Support import of ROOT (CERN) TH1 histograms"),
						QStringLiteral("chrisito@gmx.de"),
						QString());
	aboutData.setOrganizationDomain(QByteArray("kde.org"));
	aboutData.setDesktopFileName(QStringLiteral("org.kde.labplot2"));
	KAboutData::setApplicationData(aboutData);

	const auto& group = Settings::settingsGeneral();
	enableDebugTrace(group.readEntry<bool>(QLatin1String("DebugTrace"), false));
	enablePerfTrace(group.readEntry<bool>(QLatin1String("PerfTrace"), false));
	XYCurve::setOptimizationLimit(Settings::readXYCurveOptimizationLimit());
	XYCurve::setDrawPathLimit(Settings::readXYCurveDrawPathLimit());

	// TODO: add library information (GSL version, etc.) in about dialog

	QCommandLineParser parser;

	QCommandLineOption nosplashOption(QStringLiteral("no-splash"), i18n("Disable splash screen"));
	parser.addOption(nosplashOption);

	QCommandLineOption presenterOption(QStringLiteral("presenter"), i18n("Start in the presenter mode"));
	parser.addOption(presenterOption);

	parser.addPositionalArgument(QStringLiteral("+[file]"), i18n("Open a project file."));

	aboutData.setupCommandLine(&parser);
	parser.process(app);
	aboutData.processCommandLine(&parser);

	const QStringList args = parser.positionalArguments();
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

#if KCOREADDONS_VERSION >= QT_VERSION_CHECK(5, 67, 0) // KColorSchemeManager has a system default option
	QString schemeName = group.readEntry("ColorScheme");
#else
	KConfigGroup generalGlobalsGroup = KSharedConfig::openConfig(QLatin1String("kdeglobals"))->group("General");
	QString defaultSchemeName = generalGlobalsGroup.readEntry("ColorScheme", QStringLiteral("Breeze"));
	QString schemeName = group.readEntry("ColorScheme", defaultSchemeName);
#endif
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
