/***************************************************************************
    File                 : LabPlot.cpp
    Project              : LabPlot
    Description          : main function
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Stefan Gerlach (stefan.gerlach@uni.kn)
    Copyright            : (C) 2008-2016 Alexander Semke (alexander.semke@web.de)

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/
#include <QApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QFile>
#include <QSplashScreen>
#include <QStandardPaths>
#include <QModelIndex>
#ifdef _WIN32
#include <windows.h>
#endif

#include <KAboutData>
#include <KColorSchemeManager>
#include <KConfigGroup>
#include <KCrash>
#include <KLocalizedString>
#include <KMessageBox>
#include <KSharedConfig>

#include "MainWin.h"
#include "backend/core/AbstractColumn.h"
#include "backend/lib/macros.h"

int main (int argc, char *argv[]) {
	QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
	QApplication app(argc, argv);
	KLocalizedString::setApplicationDomain("labplot2");
	KCrash::initialize();

	KAboutData aboutData( QStringLiteral("labplot2"), QString("LabPlot"),
		LVERSION, i18n("LabPlot2 is a KDE-application for interactive graphing and analysis of scientific data."),
		KAboutLicense::GPL,i18n("(c) 2007-2019"), QString(), QStringLiteral("https://labplot.kde.org"));
	aboutData.addAuthor(i18n("Stefan Gerlach"), i18nc("@info:credit", "Developer"), "stefan.gerlach@uni.kn", nullptr);
	aboutData.addAuthor(i18n("Alexander Semke"), i18nc("@info:credit", "Developer"), "alexander.semke@web.de", nullptr);
	aboutData.addAuthor(i18n("Fábián Kristóf-Szabolcs"), i18nc("@info:credit", "Developer"), "f-kristof@hotmail.com", nullptr);
	aboutData.addAuthor(i18n("Martin Marmsoler"), i18nc("@info:credit", "Developer"), "martin.marmsoler@gmail.com", nullptr);
	aboutData.addAuthor(i18n("Andreas Kainz"), i18nc("@info:credit", "Icon designer"), "kainz.a@gmail.com", nullptr);
	aboutData.addCredit(i18n("Yuri Chornoivan"), i18nc("@info:credit", "Help on many questions about the KDE-infrastructure and translation related topics"), "yurchor@ukr.net", nullptr);
	aboutData.addCredit(i18n("Garvit Khatri"), i18nc("@info:credit", "Porting LabPlot2 to KF5 and Integration with Cantor"), "garvitdelhi@gmail.com", nullptr);
	aboutData.addCredit(i18n("Christoph Roick"), i18nc("@info:credit", "Support import of ROOT (CERN) TH1 histograms"), "chrisito@gmx.de", nullptr);
	aboutData.setOrganizationDomain(QByteArray("kde.org"));
	aboutData.setDesktopFileName(QStringLiteral("org.kde.labplot2"));
	KAboutData::setApplicationData(aboutData);

	QCommandLineParser parser;
	parser.addHelpOption();
	parser.addVersionOption();

	QCommandLineOption nosplashOption("no-splash", i18n("disable splash screen"));
	parser.addOption(nosplashOption);

	QCommandLineOption presenterOption("presenter", i18n("start in the presenter mode"));
	parser.addOption(presenterOption);

	parser.addPositionalArgument("+[file]", i18n( "open a project file"));

	aboutData.setupCommandLine(&parser);
	parser.process(app);
	aboutData.processCommandLine(&parser);

	const QStringList args = parser.positionalArguments();
	QString filename;
	if (args.count() > 0)
		filename = args[0];

	if (!filename.isEmpty() ) {
		//determine the absolute file path in order to properly save it in MainWin in "Recent Files"
		QDir dir;
		filename = dir.absoluteFilePath(filename);

		if ( !QFile::exists(filename)) {
			if ( KMessageBox::warningContinueCancel( nullptr,
					i18n( "Could not open file \'%1\'. Click \'Continue\' to proceed starting or \'Cancel\' to exit the application.", filename),
					i18n("Failed to Open")) == KMessageBox::Cancel) {
				exit(-1);  //"Cancel" clicked -> exit the application
			} else {
				filename.clear(); //Wrong file -> clear the file name and continue
			}
		}
	}

	QSplashScreen* splash = nullptr;
	if (!parser.isSet(nosplashOption)) {
		const QString& file = QStandardPaths::locate(QStandardPaths::DataLocation, "splash.png");
		splash = new QSplashScreen(QPixmap(file));
		splash->show();
	}

	// debugging paths
	QStringList appdatapaths = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);
	QDEBUG("AppDataLocation paths = " << appdatapaths);
	QDEBUG("Icon theme search paths = " << QIcon::themeSearchPaths());
	QDEBUG("Library search paths = " << QCoreApplication::libraryPaths());

	// needed in order to have the signals triggered by SignallingUndoCommand
	//TODO: redesign/remove this
	qRegisterMetaType<const AbstractAspect*>("const AbstractAspect*");
	qRegisterMetaType<const AbstractColumn*>("const AbstractColumn*");

#ifdef _WIN32
	// enable debugging on console
	if (AttachConsole(ATTACH_PARENT_PROCESS)) {
		freopen("CONOUT$", "w", stdout);
		freopen("CONOUT$", "w", stderr);
	}
#endif

	KConfigGroup generalGlobalsGroup = KSharedConfig::openConfig(QLatin1String("kdeglobals"))->group("General");
	QString defaultSchemeName = generalGlobalsGroup.readEntry("ColorScheme", QStringLiteral("Breeze"));
	KConfigGroup group = KSharedConfig::openConfig()->group(QLatin1String("Settings_General"));
	QString schemeName = group.readEntry("ColorScheme", defaultSchemeName);
	KColorSchemeManager manager;
	manager.activateScheme(manager.indexForScheme(schemeName));

	MainWin* window = new MainWin(nullptr, filename);
	window->show();

	if (splash) {
		splash->finish(window);
		delete splash;
	}

	if (parser.isSet(presenterOption))
		window->showPresenter();

	return app.exec();
}
