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
#include <QPluginLoader>
#include "core/Project.h"
#include "core/ProjectWindow.h"

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

	// show splash screen
	QSplashScreen splash(QPixmap(":/appsplash"));
	splash.show();
	QTimer *timer = new QTimer(&app);
	app.connect( timer, SIGNAL(timeout()), &splash, SLOT(close()) );
	app.connect( timer, SIGNAL(timeout()), timer, SLOT(stop()) );
	timer->start(5000); // autoclose after 5 seconds

	// module initialization
	Project::staticInit();
	foreach(QObject *plugin, QPluginLoader::staticInstances()) 
	{
		NeedsStaticInit * module = qobject_cast<NeedsStaticInit *>(plugin);
		if (module) 
			module->staticInit();
	}
	//

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
