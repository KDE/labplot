// LabPlot : Worksheet.cc

#include <KDebug>
#include <KLocale>
#include <KSharedConfig>
#include <KConfigGroup>
#include <QPrinter>
#include <QPrintDialog>
#include <KInputDialog>
#include "Worksheet.h"
#include "MainWin.h"
#include "Plot2DSimple.h"

Worksheet::Worksheet(MainWin *m)
	:QWidget(), mw(m)
{
	kDebug()<<"Worksheet()"<<endl;
	type = WORKSHEET;

	//  set title
	int number=1;
	while(mw->getWorksheet(i18n("Worksheet %1").arg(number)) != 0)
		number++;
	setWindowTitle(i18n("Worksheet %1").arg(number));

	KConfigGroup conf(KSharedConfig::openConfig(),"Worksheet");
	// TODO : hardcoded :-(
	setMinimumSize(300,200);
	int w = conf.readEntry("Width",600);
	int h = conf.readEntry("Height",400);
	resize(w,h);

	api=0;
	kDebug()<<"Worksheet() DONE"<<endl;
}

Worksheet::~Worksheet() {
	mw->updateGUI();
}

void Worksheet::paintEvent(QPaintEvent *) {
	kDebug()<<"Worksheet::paintEvent()"<<endl;
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);	

	draw(&painter,width(),height());
}

QDomElement Worksheet::save(QDomDocument doc) {
	kDebug()<<endl;
	QDomElement wstag = doc.createElement( "Worksheet" );
	wstag.setAttribute("api",QString::number(api));
//	wstag.setAttribute("nr_plots",QString::number(nr_plots));

	QDomElement tag = doc.createElement( "Position" );
	tag.setAttribute("x",QString::number(parentWidget()->pos().x()));
	tag.setAttribute("y",QString::number(parentWidget()->pos().y()));
	wstag.appendChild( tag );
	tag = doc.createElement( "Size" );
	tag.setAttribute("width",QString::number(width()));
	tag.setAttribute("height",QString::number(height()));
	wstag.appendChild( tag );
	tag = doc.createElement( "Title" );
	wstag.appendChild( tag );
	QDomText t = doc.createTextNode( windowTitle() );
	tag.appendChild( t );
	// TODO
/*	tag = doc.createElement( "TitleEnabled" );
	wstag.appendChild( tag );
	t = doc.createTextNode( QString::number(title_enabled) );
	tag.appendChild( t );
	tag = doc.createElement( "Background" );
	wstag.appendChild( tag );
	t = doc.createTextNode( background.color().name() );
	tag.appendChild( t );
	tag = doc.createElement( "Brush" );
	wstag.appendChild( tag );
	t = doc.createTextNode( QString::number(background.style()) );
	tag.appendChild( t );
	tag = doc.createElement( "Timestamp" );
	wstag.appendChild( tag );
	QDomText t = doc.createTextNode( QString::number(timestamp.toTime_t()) );
	tag.appendChild( t );
	tag = doc.createElement( "TimestampEnabled" );
	wstag.appendChild( tag );
	t = doc.createTextNode( QString::number(timestamp_enabled) );
	tag.appendChild( t );
	tag = doc.createElement( "DrawObjectsFirst" );
	wstag.appendChild( tag );
	t = doc.createTextNode( QString::number(draw_objects_first) );
	tag.appendChild( t );
*/
	// drawing objects
	//TODO
/*	kDebug()<<"	saving drawing objects"<<endl;
	for(int i=0;i<NR_OBJECTS;i++) {
		// only save used objects
		if(! label[i]->Title().isEmpty()) {
			tag = label[i]->saveXML(doc);
			wstag.appendChild(tag);
		}
		Point start = line[i]->startPoint() ,end = line[i]->endPoint();
		if(fabs(start.X()-end.X())>1.0e-6 || fabs(start.Y()-end.Y())>1.0e-6) {
			tag = line[i]->saveXML(doc);
			wstag.appendChild(tag);
		}
		start = rect[i]->startPoint();
		end = rect[i]->endPoint();
		if(fabs(start.X()-end.X())>1.0e-6 || fabs(start.Y()-end.Y())>1.0e-6) {
			tag = rect[i]->saveXML(doc);
			wstag.appendChild(tag);
		}
		start = rect[i]->startPoint();
		end = rect[i]->endPoint();
		if(fabs(start.X()-end.X())>1.0e-6 || fabs(start.Y()-end.Y())>1.0e-6) {
			tag = ellipse[i]->saveXML(doc);
			wstag.appendChild(tag);
		}
		if(! image[i]->Name().isEmpty()) {
			tag = image[i]->saveXML(doc);
			wstag.appendChild(tag);
		}
	}
*/

	// all plots
	// TODO
/*	for (int i=0;i < plot.size();i++) {
		kDebug()<<"	saving Plot "<<i<<endl;
		tag = plot[i]->savePlotXML(doc);
		wstag.appendChild(tag);
	}
*/
	return wstag;
}

void Worksheet::open(QDomNode node) {
	kDebug()<<endl;
	int tmpnr_plots=0;
//	int nr_label=0, nr_line=0, nr_rect=0, nr_ellipse=0, nr_image=0;
	while(!node.isNull()) {
		QDomElement e = node.toElement();
		kDebug()<<"WS TAG = "<<e.tagName()<<endl;
		kDebug()<<"WS TEXT = "<<e.text()<<endl;

		if(e.tagName() == "Position") {
			int px = e.attribute("x").toInt();
			int py = e.attribute("y").toInt();
			parentWidget()->move(QPoint(px,py));
			if(px<0 || py<0) {	// fullscreen
				mw->getMdi()->cascadeSubWindows();
				showMaximized();
			}
		}
		else if(e.tagName() == "Size") {
			int X = e.attribute("width").toInt();
			int Y = e.attribute("height").toInt();
			resize(X,Y);
			kDebug()<<"Resizing to "<<X<<' '<<Y<<endl;
		}
		else if(e.tagName() == "Title")
			setTitle(e.text());
		// TODO
//		else if(e.tagName() == "TitleEnabled")
//			title_enabled = (bool) e.text().toInt();
//		else if(e.tagName() == "Background")
//			background.setColor(QColor(e.text()));
//		else if(e.tagName() == "Brush")
//			background.setStyle(Qt::BrushStyle(e.text().toInt()));
//		else if(e.tagName() == "Timestamp")
//			timestamp.setTime_t(e.text().toInt());
//		else if(e.tagName() == "TimestampEnabled")
//			timestamp_enabled = (bool) e.text().toInt();
//		else if(e.tagName() == "DrawObjectsFirst")
//			draw_objects_first = (bool) e.text().toInt();

/*		else if(e.tagName() == "Label")
			label[nr_label++]->openXML(e.firstChild());
		else if(e.tagName() == "Line")
			line[nr_line++]->openXML(e.firstChild());
		else if(e.tagName() == "Rect")
			rect[nr_rect++]->openXML(e.firstChild());
		else if(e.tagName() == "Ellipse")
			ellipse[nr_ellipse++]->openXML(e.firstChild());
		else if(e.tagName() == "Image")
			image[nr_image++]->openXML(e.firstChild());
*/
		else if(e.tagName() == "Plot") {
			// TODO
			kDebug()<<"	TAG Plot : type = "<<e.attribute("type").toInt()<<endl;
//			newPlot((PlotType) e.attribute("type").toInt());
			kDebug()<<"tmpnr_plots = "<<tmpnr_plots<<endl;
			kDebug()<<"	Calling openPlotXML()"<<endl;
//			plot[tmpnr_plots++]->openPlotXML(e.firstChild());
		}

		node = node.nextSibling();
	}
}

void Worksheet::setTitle(QString title) {
	kDebug()<<"title ="<<title<<endl;
	bool ok=true;
	if(title.isEmpty())
		title = KInputDialog::getText("LabPlot", i18n("Worksheet title : "), windowTitle(), &ok);

	if(ok && !title.isEmpty()) {
		setWindowTitle(title);
		mw->updateSheetList();
	}
}

void Worksheet::draw(QPainter *p, int w, int h) {
	kDebug()<<"Worksheet::Draw() : w/h ="<<w<<h<<endl;
	// TEST
	p->setBrush(Qt::yellow);
	p->drawRect(0,0,w,h);
	p->drawLine(0,0,w,h);
	p->drawLine(0,h,w,0);

	for (int i=0;i<plotCount();i++) {
		kDebug()<<"plot "<<i<<endl;
		plot[i]->draw(p,w,h);
	}
}

void Worksheet::addPlot(PlotType ptype) {
	kDebug()<<"Worksheet::newPlot() : type ="<<ptype<<endl;
	Plot *newplot=0;
	switch(ptype) {
	case PLOT2D:
		newplot = new Plot2DSimple();break;
	default:
		break;
	}
	api = plotCount();
	kDebug()<<"api ="<<api<<endl;
	plot += newplot;
}

void Worksheet::addSet(Set *s, PlotType ptype) {
	kDebug()<<"Worksheet::addSet() plot type ="<<ptype<<endl;

	if(plotCount() == 0 || ptype != plot[api]->Type() )
		addPlot(ptype);
	plot[api]->addSet(s);
	plot[api]->resetRanges();

	// set actrange for new plots
	Range *actrange = plot[api]->ActRanges();
	if (actrange[0].max()-actrange[0].min() == 0)
		plot[api]->setActRanges(plot[api]->Ranges());
	repaint();
}

void Worksheet::setupPrinter(QPrinter *pr, QString fn) {
	kDebug()<<"Worksheet::setupPrinter()"<<endl;
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
	kDebug()<<"Worksheet::Print("<<file<<")"<<endl;

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
