/***************************************************************************
    File                 : main.cpp
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : SciDAVis main function

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
#include <QAction>
#include <QSplashScreen>
#include <QTimer>
#include "core/Project.h"
#include "core/ProjectWindow.h"
#include "core/column/Column.h"
#include "core/plugin/PluginManager.h"
#include <stdio.h>

// The following stuff is for the doxygen title page
/*!  \mainpage SciDAVis - Scientific Data Analysis and Visualization - API documentation

  \section description Program description:
	SciDAVis is a user-friendly data analysis and visualization program primarily aimed at high-quality plotting of scientific data.
	It strives to combine an intuitive, easy-to-use graphical user interface with powerful features such as Python scriptability.

	 The SciDAVis web page can be found at<br>
	 <a href="http://scidavis.sourceforge.net">http://scidavis.sourceforge.net</a><br>
	 
	All suggestions and contributions are most welcome!<br>
	If you want to contribute code, please read the notes on \ref style "coding style" first.
	There is also a section with some notes about \ref future "future plans".
	<br>

  \section libs SciDAVis uses the following libraries:
  <a href="http://www.trolltech.com/products/qt/index.html">Qt</a>,
  <a href="http://qwt.sourceforge.net/index.html">Qwt</a>,
  <a href="http://qwtplot3d.sourceforge.net/">QwtPlot3D</a>,
  <a href="http://sources.redhat.com/gsl/">GSL</a>,
  <a href="http://muparser.sourceforge.net/">muParser</a>,
  <a href="http://www.zlib.net/">zlib</a>,
  and <a href="http://sourceforge.net/projects/liborigin/">liborigin</a>.
*/

int main( int argc, char ** argv )
{
    QApplication app( argc, argv );

	QStringList args = app.arguments();
	args.removeFirst(); // remove application name

	bool noGui = false;
	bool noSplash = false;
	while (args.count() > 0) {
		if (("--help" == args[0]) || ("-h" == args[0])) {
			args.removeFirst();
			printf("%s%s\n", SciDAVis::versionString().toAscii().data(), SciDAVis::extraVersion().toAscii().data());
			printf("Released %s\n", SciDAVis::releaseDateString().toAscii().data());
			printf("License: GPLv2 or later\n");
			printf("Supported command line arguments:\n");
			printf("--about | -a: show about dialog\n");
			printf("--help | -h: show version info and command line arguments\n");
			printf("--enable-plugin FILENAME | -ep FILENAME: enable and load the plugin FILENAME\n");
			printf("--disable-plugin FILENAME | -dp FILENAME: disable the plugin FILENAME\n");
			printf("--no-gui: don't start the graphical user interface, only parse\n the command line arguments and quit\n");
			printf("--no-splash: disable the splash sceen\n");
		} else if (("--about" == args[0]) || ("-a" == args[0])) {
			args.removeFirst();
			SciDAVis::about();
		} else if ("--no-gui" == args[0]) {
			args.removeFirst();
			noGui = true;
		} else if ("--no-splash" == args[0]) {
			args.removeFirst();
			noSplash = true;
		} else if (("--enable-plugin" == args[0]) || ("-ep" == args[0])) {
			args.removeFirst();
			if (args.count() > 0) {
	 			PluginManager::enablePlugin(args[0]);
				args.removeFirst();
			}
		} else if (("--disable-plugin" == args[0]) || ("-dp" == args[0])) {
			args.removeFirst();
			if (args.count() > 0) {
	 			PluginManager::disablePlugin(args[0]);
				args.removeFirst();
			}
		}
	}

#ifdef QT_DEBUG
	PluginManager::printAll();
#endif

	if (!noSplash) {
		// show splash screen
		QSplashScreen splash(QPixmap(":/appsplash"));
		splash.show();
		QTimer *timer = new QTimer(&app);
		app.connect( timer, SIGNAL(timeout()), &splash, SLOT(close()) );
		app.connect( timer, SIGNAL(timeout()), timer, SLOT(stop()) );
		timer->start(5000); // autoclose after 5 seconds
	}

	// module initialization
	AbstractAspect::staticInit();
	AbstractColumn::staticInit();
	Project::staticInit();
	Column::staticInit();
	foreach(QObject *plugin, PluginManager::plugins()) 
	{
		NeedsStaticInit * module = qobject_cast<NeedsStaticInit *>(plugin);
		if (module) 
			module->staticInit();
	}
	if (noGui)
		return 0;

	// create initial empty project
	Project* p = new Project();
	p->view()->showMaximized();

	// TODO: who deletes projects that get closed?


#if 0
	QStringList args = app.arguments();
	args.removeFirst(); // remove application name

	if( (args.count() == 1) && (args[0] == "-m" || args[0] == "--manual") )
		ApplicationWindow::showStandAloneHelp();
	else if ( (args.count() == 1) && (args[0] == "-a" || args[0] == "--about") ) {
		ApplicationWindow::about();
		exit(0);
	} else {
		QSplashScreen *splash = new QSplashScreen(QPixmap(":/appsplash"));
		splash->show();
		QTimer *timer = new QTimer(&app);
		app.connect( timer, SIGNAL(timeout()), splash, SLOT(close()) );
		app.connect( timer, SIGNAL(timeout()), timer, SLOT(stop()) );
		timer->start(5000);
		ApplicationWindow *mw = new ApplicationWindow();
		mw->applyUserSettings();
		mw->newTable();
		mw->showMaximized();
		mw->savedProject();
		if (mw->autoSearchUpdates){
			mw->autoSearchUpdatesRequest = true;
			mw->searchForUpdates();
		}
		mw->parseCommandLineArguments(args);
	}
#endif

	app.connect( &app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()) );
	return app.exec();
}
