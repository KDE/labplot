#include <KApplication>
#include <KAboutData>
#include <KCmdLineArgs>
#include <KStandardDirs>
#include <KSplashScreen>
#include <KDebug>
 
#include "MainWin.h"

int main (int argc, char *argv[]) {
	KAboutData aboutData( "LabPlot", "LabPlot",
			ki18n("LabPlot"), LVERSION,
			ki18n("An application for plotting and analysis of 2d and 3d functions and data."),
			KAboutData::License_GPL,
			ki18n("Copyright (c) 2008 Stefan Gerlach") );
	aboutData.addAuthor(ki18n("Stefan Gerlach"), ki18n("developer"), "stefan.gerlach@uni-konstanz.de", 0);
	aboutData.addAuthor(ki18n("Alexander Semke"), ki18n("developer"), "", 0);

	KCmdLineArgs::init( argc, argv, &aboutData );
	KCmdLineOptions options;
	options.add("no-splash",ki18n("do not show the splash screen"));
	options.add("+[file]",ki18n("open a project file"));
	KCmdLineArgs::addCmdLineOptions( options );
 
	KApplication app;
	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
	QString filename;
	if (args->count() > 0)
		filename = args->arg(0);

	KSplashScreen *splash=0;
	if (args->isSet("-splash")) {
		QString file = KStandardDirs::locate("appdata", "labplot.png");
		QPixmap pixmap(file);
		splash= new KSplashScreen(pixmap);
		splash->show();
	}

#ifdef HAVE_GSL
	kDebug()<<"HAVE_GSL defined"<<endl;
#endif
#ifdef GSL_FOUND
	kDebug()<<"GSL_FOUND defined"<<endl;
#endif
#ifdef GSL_VERSION
	kDebug()<<"GSL_VERSION defined"<<endl;
#endif
// TODO:
//#if GSL_VERSION > 1.8
//	kDebug()<<"GSL_VERSION > 1.8"<<endl;
//#endif

	MainWin* window = new MainWin();
	window->show();
//	sleep(1);		// to see splash screen
	if(splash)
		splash->finish(window);
	return app.exec();
}
