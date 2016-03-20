/***************************************************************************
    File                 : LabPlot.cpp
    Project              : LabPlot
    Description          : main function
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Stefan Gerlach (stefan.gerlach@uni-konstanz.de)
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
#include <KAboutData>
#include <KLocalizedString>
#include <QStandardPaths>
#include <QSplashScreen>
#include <KMessageBox>
#include <QFile>

#include "MainWin.h"
#include "backend/core/AbstractColumn.h"

int main (int argc, char *argv[]) {
	KAboutData aboutData( QStringLiteral("labplot2"), QString("labplot2"),
				LVERSION,
				i18n("LabPlot2 is a KDE-application for interactive graphing and analysis of scientific data."),
				KAboutLicense::GPL,
				i18n("(c) 2007-2016"),
				QString(),
				QStringLiteral("http://www.labplot.sourceforge.net"));

	aboutData.addAuthor(i18n("Stefan Gerlach"), i18n("developer"), "stefan.gerlach@uni-konstanz.de", 0);
	aboutData.addAuthor(i18n("Alexander Semke"), i18n("developer"), "alexander.semke@web.de", 0);
	aboutData.addAuthor(i18n("Andreas Kainz"), i18n("icon designer"), "kainz.a@gmail.com", 0);
	aboutData.addCredit(i18n("Yuri Chornoivan"), i18n("Help on many questions about the KDE-infrastructure and translation related topics"), "yurchor@ukr.net", 0);
	KAboutData::setApplicationData(aboutData);


	QApplication app(argc, argv);

	QCommandLineParser parser;
	QCommandLineOption nosplashOption("no-splash", i18n("disable splash screen"));
	parser.addOption(nosplashOption);

	parser.addPositionalArgument("+[file]", i18n( "open a project file"));
	aboutData.addCredit(i18n("Yuri Chornoivan"), i18n("Help on many questions about the KDE-infrastructure and translation related topics"), "yurchor@ukr.net", 0);

	aboutData.setupCommandLine(&parser);
	parser.process(app);
	aboutData.processCommandLine(&parser);
	const QStringList args = parser.positionalArguments();
	QString filename;
	if (args.count() > 0)
		filename = args[0];

	if(!filename.isEmpty() ) {
		if ( !QFile::exists(filename)) {
			if ( KMessageBox::warningContinueCancel( 0,
			        i18n( "Could not open file \'%1\'. Click \'Continue\' to proceed starting or \'Cancel\' to exit the application.", filename),
			        i18n("Failed to open")) == KMessageBox::Cancel) {
				exit(-1);  //"Cancel" clicked -> exit the application
			} else {
				filename=""; //Wrong file -> clear the file name and continue
			}
		} else if ( !(filename.contains(".lml", Qt::CaseInsensitive) || filename.contains(".lml.gz", Qt::CaseInsensitive)
				  || filename.contains(".lml.bz2", Qt::CaseInsensitive) ) ) {
			if ( KMessageBox::warningContinueCancel( 0,
			        i18n( "File \'%1\' doesn't contain any LabPlot data. Click \'Continue\' to proceed starting or \'Cancel\' to exit the application.", filename),
			        i18n("Failed to open")) == KMessageBox::Cancel) {
				exit(-1); //"Cancel" clicked -> exit the application
			} else {
				filename=""; //Wrong file -> clear the file name and continue
			}
		}
	}

	QSplashScreen *splash=0;
	if (!parser.isSet("no-splash")) {
		QString file = QStandardPaths::locate(QStandardPaths::DataLocation, "splash.png");
		splash = new QSplashScreen(QPixmap(file));
		splash->show();
	}

	// needed in order to have the signals triggered by SignallingUndoCommand
	//TODO: redesign/remove this
	qRegisterMetaType<const AbstractAspect*>("const AbstractAspect*");
	qRegisterMetaType<const AbstractColumn*>("const AbstractColumn*");

	MainWin* window = new MainWin(0, filename);
	window->show();
	if(splash)
		splash->finish(window);

	return app.exec();
}
