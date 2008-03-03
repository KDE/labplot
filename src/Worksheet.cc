// LabPlot : Worksheet.cc

#include <KDebug>
#include <KLocale>
#include <KSharedConfig>
#include <KConfigGroup>
#include <QPrinter>
#include <QPrintDialog>
#include "Worksheet.h"
#include "Plot2DSimple.h"

Worksheet::Worksheet(MainWin *mw)
	:QWidget(), mw(mw)
{
	kdDebug()<<"Worksheet()"<<endl;
	type = WORKSHEET;
	setWindowTitle(i18n("Worksheet"));

	KConfigGroup conf(KSharedConfig::openConfig(),"Worksheet");
	api=0;

	// TODO : hardcoded :-(
	setMinimumSize(300,200);
	int w = conf.readEntry("Width",600);
	int h = conf.readEntry("Height",400);
	resize(w,h);
}

void Worksheet::paintEvent(QPaintEvent *) {
	kdDebug()<<"Worksheet::paintEvent()"<<endl;
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);	

	draw(&painter,width(),height());
}

void Worksheet::draw(QPainter *p, int w, int h) {
	kdDebug()<<"Worksheet::Draw() : w/h ="<<w<<h<<endl;
	// TEST
	p->setBrush(Qt::yellow);
	p->drawRect(0,0,w,h);
	p->drawLine(0,0,w,h);
	p->drawLine(0,h,w,0);

	for (int i=0;i<plotCount();i++) {
		kdDebug()<<"plot "<<i<<endl;
		plot[i]->draw(p,w,h);
	}
}

void Worksheet::addPlot(PlotType ptype) {
	kdDebug()<<"Worksheet::newPlot() : type ="<<ptype<<endl;
	Plot *newplot=0;
	switch(ptype) {
	case PLOT2D:
		newplot = new Plot2DSimple();break;
	default:
		break;
	}
	api = plotCount();
	kdDebug()<<"api ="<<api<<endl;
	plot += newplot;
}

void Worksheet::addSet(Set *s, PlotType ptype) {
	kdDebug()<<"Worksheet::addSet() plot type ="<<ptype<<endl;

	if(plotCount() == 0 || ptype != plot[api]->Type() )
		addPlot(ptype);
	plot[api]->addSet(s);

	plot[api]->resetRanges();

	// set actrange for new plots
	LRange *actrange = plot[api]->ActRanges();
	if (actrange[0].rMax()-actrange[0].rMin() == 0)
		plot[api]->setActRanges(plot[api]->Ranges());
	repaint();
}

void Worksheet::setupPrinter(QPrinter *pr, QString fn) {
	kdDebug()<<"Worksheet::setupPrinter()"<<endl;
	// TODO
//	KConfig *config = mw->Config();
//	config->setGroup( "KPrinter Settings" );

	pr->setCreator(QString("LabPlot ")+LVERSION);
//	pr->setOutputToFile( true );
	pr->setOutputFileName( fn );
//	pr->setPageSize((KPrinter::PageSize)config->readNumEntry("PageSize",9));
//	if (config->readNumEntry("Orientation",1))
		pr->setOrientation(QPrinter::Landscape);
//	else
//		pr->setOrientation(KPrinter::Portrait);

//	if (config->readBoolEntry("ColorMode",true))
		pr->setColorMode(QPrinter::Color);
//	else
//		pr->setColorMode(KPrinter::GrayScale);
}

void Worksheet::print(QString file) {
	kdDebug()<<"Worksheet::Print("<<file<<")"<<endl;

	QPrinter printer(QPrinter::ScreenResolution);
	setupPrinter(&printer,file);

	if (file.isEmpty()) {
		QPrintDialog *dialog = new QPrintDialog(&printer, this);
		dialog->setWindowTitle(tr("Print Document"));
		dialog->addEnabledOption(QAbstractPrintDialog::PrintSelection);
		if (dialog->exec() != QDialog::Accepted)
			return;
	}

	QPainter painter;
	painter.begin(&printer);
	draw(&painter,printer.width(),printer.height());
	painter.end();
}
