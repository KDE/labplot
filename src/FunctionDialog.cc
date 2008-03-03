//LabPlot : FunctionDialog.cc

#include <QTabWidget>
#include <KMessageBox>
#include <QMdiSubWindow>
#include <QProgressDialog>
#include "FunctionDialog.h"
#include "Point.h"
#include "Set2D.h"

#include "parser_struct.h"
extern "C" con constants[];
extern "C" init arith_fncts[];
#include "parser_extern.h"

FunctionDialog::FunctionDialog(MainWin *mw)
	: Dialog(mw)
{
	kdDebug()<<"FunctionDialog()"<<endl;
	setCaption(i18n("Add Function"));

	type = PLOT2D;

	setupGUI();

	showButton(KDialog::User2,false);
	QObject::connect(this,SIGNAL(applyClicked()),SLOT(Apply()));
	QObject::connect(this,SIGNAL(okClicked()),SLOT(Apply()));
	QObject::connect(this,SIGNAL(user1Clicked()),SLOT(saveSettings()));
	//QObject::connect(this,SIGNAL(user2Clicked()),SLOT(toggleOptions()));

	kdDebug()<<"FunctionDialog() DONE"<<endl;

/*	Plot *plot=0;
	if(p != 0)
	 	plot = p->getPlot(p->API());
	type = newtype;

	QString caption;
	if (type == P2D)
		caption=i18n("2D Function Dialog");
	else if (type == P3D)
		caption=i18n("3D Function Dialog");
	else if (type == PQWT3D)
		caption=i18n("3D QWT Function Dialog");
	else if (type == PSURFACE)
		caption=i18n("2D Surface Function Dialog");
	else if (type == PPOLAR)
		caption=i18n("2D Polar Function Dialog");
	caption += i18n(" : ")+QString(name);
	setCaption(caption);

	Style *style=0;
	Symbol *symbol=0;
	LRange rx, ry;
	GRAPHType st = GRAPH2D;
	int nrx = 0, nry = 0;

	if(type == PPOLAR) {
		if(symbol) symbol->setType(SCROSS);
	}

	kdDebug()<<"get values"<<endl;
	// default style and symbol for polar plots
	if (item == -1) {	// new graph
		graph = 0;
	}
	else {
		GraphList *gl = plot->getGraphList();
		graph = gl->getGraph(item);
		st = gl->getType(item);

		style = graph->getStyle();
		symbol = graph->getSymbol();
		kdDebug()<<"Struct : "<<st<<endl;

		if (st == GRAPH2D) {
			Graph2D *g = gl->getGraph2D(item);
			rx = g->Range(0);
			nrx = graph->Number();
		}
		else if (st == GRAPH3D) {
			Graph3D *g = gl->getGraph3D(item);
			rx = g->Range(0);
			ry = g->Range(1);
			nrx = g->NX();
			nry = g->NY();
		}
		else if (st == GRAPHM) {
			GraphM *g = gl->getGraphM(item);
			rx = g->Range(0);
			ry = g->Range(1);
			nrx = g->NX();
			nry = g->NY();
		}
	}

*/
}

void FunctionDialog::setupGUI() {
	kdDebug()<<"FunctionDialog::setupGUI()"<<endl;
	KConfigGroup conf(KSharedConfig::openConfig(),"Function");
	//TODO : config bug
	QString entry = QString("PlotType %1 ").arg(type);
//	kdDebug()<<"	default function :"<<conf.readEntry(entry+"Function",i18n("sin(x)"))<<endl;

	QTabWidget *tw = new QTabWidget;
	layout->addWidget(tw,0,0,1,2);

	QWidget *tab1 = new QWidget;
	tw->addTab(tab1,i18n("Options"));
	QGridLayout *tablayout = new QGridLayout(tab1);
	functionscb = new KComboBox;
	tablayout->addWidget(functionscb,0,1);
	int f=0;
	while(true){
		QString tmp = arith_fncts[f++].fname;
		if(tmp.isEmpty())
			break;
		functionscb->insertItem(f,tmp+"()");
	}
	connect(functionscb,SIGNAL(activated(QString)),SLOT(insertFunction(QString)));
	constantscb = new KComboBox;
	tablayout->addWidget(constantscb,0,2);
	f=0;
	while(true){
		QString tmp = constants[f++].name;
		if(tmp.isEmpty())
			break;
		constantscb->insertItem(f,tmp);
	}
	connect(constantscb,SIGNAL(activated(QString)),SLOT(insertConstant(QString)));

	tablayout->addWidget(new QLabel(i18n("Function : ")),1,0);
	functionle = new KLineEdit(conf.readEntry(entry+"Function",i18n("sin(x)")));
	tablayout->addWidget(functionle,1,1,1,2);
	tablayout->addWidget(new QLabel(i18n("Range :")),2,0);
	minle = new KLineEdit(conf.readEntry(entry+"XMin","0"));
	QDoubleValidator *validator = new QDoubleValidator(this);
	minle->setValidator(validator);
	tablayout->addWidget(minle,2,1);
	maxle = new KLineEdit(conf.readEntry(entry+"XMax","1"));
	validator = new QDoubleValidator(this);
	maxle->setValidator(validator);
	tablayout->addWidget(maxle,2,2);
	tablayout->addWidget(new QLabel(i18n("Number of Points :")),3,0);
	nxi = new KIntNumInput;
	nxi->setMinimum(1);
	nxi->setSliderEnabled(false);
	nxi->setValue(conf.readEntry(entry+"NX","100").toInt());
	tablayout->addWidget(nxi,3,2);

	// Label
	QWidget *tab2 = new QWidget;
	tw->addTab(tab2,i18n("Label"));
	// TODO : use label of set if set!=0
	labelWidget(tab2,new Label());
	xni->setEnabled(false);
	yni->setEnabled(false);

	// Style
	QWidget *tab3 = new QWidget;
	tw->addTab(tab3,i18n("Style"));
	tablayout = new QGridLayout(tab3);
	// TODO
	tablayout->addWidget(new QLabel(i18n("Not implemented yet!")),0,0);

	// destination
	QLabel *l = new QLabel(i18n("Add to "));
	l->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	layout->addWidget(l,1,0);
	sheetcb = new KComboBox;
	layout->addWidget(sheetcb,1,1);
	QList<QMdiSubWindow *> wlist = mw->getMdi()->subWindowList();
	for (int i=0; i<wlist.size(); i++)
		sheetcb->insertItem(i,wlist.at(i)->windowTitle());
	sheetcb->addItem(i18n("New Worksheet"));
	sheetcb->addItem(i18n("New Spreadsheet"));
	sheetcb->setCurrentIndex(mw->activeSheetIndex()<0?wlist.size():mw->activeSheetIndex());

/* OLD :
	// function
	QString fun("sin(x)");
	if (type == P3D || type == PSURFACE || type == PQWT3D)
		fun = QString("sin(x+y)");
	if (graph != 0) fun = graph->Name();
	else fun = config->readEntry(entry+"Function",fun);

	reread = 0;
	if (graph == 0 || (graph != 0 && graph->Source() == SFUNCTION)) {
		conlabel->show();
		concb->show();
		sellabel->show();
		selcb->show();
		reread = new QCheckBox(i18n("Recreate Function"),hb);
		reread->setChecked(true);
	}
	if (graph == 0 )	// don't show for new function
		reread->hide();

	KPushButton *setLabel = new KPushButton(i18n("Update Label"),hb);
	QObject::connect(setLabel,SIGNAL(clicked()),SLOT(updateLabel()));

	else if (type == P3D || type == PSURFACE || type == PQWT3D){
		if (graph == 0) {
			nrx = config->readNumEntry("NX",100);
			nry = config->readNumEntry("NY",100);
		}
		nx = new KIntNumInput(nrx,hb);
		nx->setMinValue(1);
		new QLabel(QString("NY = "),hb);
		ny = new KIntNumInput(nry,hb);
		ny->setMinValue(1);
	}

	if (graph != 0 && graph->Source() == SSPREADSHEET) {
		xmin->setEnabled(false);
		xmax->setEnabled(false);
		if(nx) nx->setEnabled(false);
		if(ny) ny->setEnabled(false);
	}

	Label* label = new Label(config->readEntry(entry+"Function",fun));
	if (graph != 0) {
		delete label;
		label = graph->getLabel();
	}
	rtw = new RichTextWidget((QWidget *)tab2,label,0);

	tw->addTab(tab1,i18n("Parameter"));
	tw->addTab(tab2,i18n("Label"));

	QVBox *styletab, *annotatetab, *errorbartab;
	if (type == PSURFACE) {
		styletab = surfaceStyle(tw,item==-1?true:false);
		tw->addTab(styletab,i18n("Style"));
	}
	else if (type != PQWT3D) {
		styletab = simpleStyle(tw, style, symbol);
		annotatetab = annotateValuesTab(tw,graph);
		errorbartab = errorbarTab(tw,symbol);
		tw->addTab(styletab,i18n("Style"));
		tw->addTab(annotatetab,i18n("Annotate Values"));
		tw->addTab(errorbartab,i18n("Errorbars"));
	}
*/
}

void FunctionDialog::saveSettings() {
	kdDebug()<<"FunctionDialog::saveSettings()"<<endl;
	KConfigGroup conf(KSharedConfig::openConfig(),"Function");
	QString entry = QString("PlotType %1 ").arg(type);

	conf.writeEntry(entry+"Function",functionle->text());
	conf.writeEntry(entry+"XMin",minle->text());
	conf.writeEntry(entry+"XMax",maxle->text());
/*	if(ymin != 0 && ymax != 0) {
		config->writeEntry(entry+"YMin",ymin->text().toDouble());
		config->writeEntry(entry+"YMax",ymax->text().toDouble());
	}
*/
	conf.writeEntry(entry+"NX",nxi->value());
	//if(ny != 0)
	//	config->writeEntry(entry+"NY",ny->value());
	// TODO	 : more stuff
	
	conf.config()->sync();
}

/*	// TODO : not used config->writeEntry(entry+"Reread",reread->isChecked());

	rtw->getLabel()->saveSettings(config,entry);

	if(type == PSURFACE)
		saveSurfaceStyle();

	if(type != PQWT3D && type != PSURFACE) {
		if(typecb)
			config->writeEntry(entry+"Annotate Type",typecb->currentItem());
		if(positioncb)
			config->writeEntry(entry+"Annotate Position",positioncb->currentItem());
		if(distance)
			config->writeEntry(entry+"Annotate Distance",distance->value());

		saveErrorbarSettings();
	}
*/

void FunctionDialog::insertConstant(QString c) {
	kdDebug()<<"FunctionDialog::insertConstant("<<c<<")"<<endl;
	functionle->insert(c);
	functionle->setFocus();
}

void FunctionDialog::insertFunction(QString f) {
	kdDebug()<<"FunctionDialog::insertFunction("<<f<<")"<<endl;
	functionle->insert(f);
	functionle->setFocus();
	functionle->setCursorPosition(functionle->cursorPosition()-1);
	// TODO : insert "x" ?
}

/*
// search for a usable plot; add worksheet if necessary
void FunctionDialog::findPlot() {
	kdDebug()<<"FunctionDialog::findPlot()"<<endl;
	int sitem = sheetcb->currentItem(), count = sheetcb->count();
	kdDebug()<<"	sheetcb->currentItem() = "<<sitem<<" of "<<count<<endl;

	if(sitem>=count-2)		// new sheet selected
		return;

	QWidgetList list = mw->getWorkspace()->windowList();
	p = (Worksheet *) list.at(sitem);
	if(p==0 || p->getWidgetType() != WWORKSHEET) {
		p = mw->activeWorksheet();
		return;
	}

	Plot *plot=0;
	if(p)
		plot = p->getPlot(p->API());

	if(plot && plot->Type() == PQWT3D || type == PQWT3D) {	// new worksheet if plot is qwt3d or type = qwt3d
		kdDebug()<<"	QWT Plot found! type = "<<type<<endl;
		if(plot && plot->getGraphList()->Number() > 0 ) {
			p = mw->newWorksheet();
			p->newPlot(type);
			plot = p->getPlot(p->API());
			sheetcb->setCurrentItem(count-2);		// set item to new worksheet
		}
		else
			p->newPlot(type);
	}
	else if(sitem < count-2 && plot && plot->Type() != type)		// new plot if choosen of other type.
		p->newPlot(type);					// not if already new sheet selected
}

int FunctionDialog::apply_clicked() {
	findPlot();

	// add function
	// TODO : use richtext label
//	QString label = labelle->text();
	int status=0;
	if (reread && reread->isChecked()) {
		kdDebug()<<"	reread is checked"<<endl;
		status = addFunction();
	}
	else {	// only edit function
		kdDebug()<<"	plot type = "<<type<<endl;
		if(type == PSURFACE) {
			kdDebug()<<"	surface plot"<<endl;
			if (p != 0) {
				kdDebug()<<"		p != 0 surface plot"<<endl;
				Plot2DSurface *plot = (Plot2DSurface *)p->getPlot(p->API());

				if (plot != 0) {
					kdDebug()<<"	reading settings"<<endl;
					plot->enableDensity(dcb->isChecked());
					plot->enableContour(ccb->isChecked());
					plot->setNumber(numberle->text().toInt());
					// OLD : plot->setPalette(pcb->currentItem());
					plot->setContourColor(contourcolor->color());
					plot->setColoredContour(coloredcb->isChecked());
					plot->setContourWidth(contourwidthle->text().toInt());
					plot->setMesh(meshcb->isChecked());
					plot->setRelative(relativecb->isChecked());
					plot->setBrush(dbrushcb->currentItem());
					plot->setThreshold(thresholdle->text().toDouble());
				}
			}
		}
		else {
			Style *style = new Style((StylesType)cb2->currentItem(),color->color(),filled->isChecked(),
				fcolor->color(),width->value(),pencb->currentItem(),brushcb->currentItem());
			style->setBoxWidth(boxwidth->value());
			style->setAutoBoxWidth(autobox->isChecked());
			style->setPointsSorting(sortpointscb->isChecked());
			Symbol *symbol = new Symbol((SType)symbolcb->currentItem(),scolor->color(),ssize->value(),
					(FType)symbolfillcb->currentItem(),sfcolor->color(),sbrushcb->currentItem());
			AnnotateValues av(typecb->currentItem(),positioncb->currentItem(),distance->value());
			Errorbar *errorbar = new Errorbar(ebarcolor->color(), ebarxsize->value(), ebarysize->value(),
				(Qt::PenStyle) ebarstylecb->currentItem(), ebarwidth->value(), (EType) ebarxtypecb->currentItem(),
				(EType) ebarytypecb->currentItem(), ebarbcolor->color(), ebarbwidth->value(),
				(Qt::PenStyle) ebarbstylecb->currentItem() );
			symbol->setErrorbar(errorbar);
			graph->setStyle(style);
			graph->setSymbol(symbol);
			graph->setAnnotateValues(av);
		}
		graph->setLabel(rtw->label());
	}

	if(l) l->updateList();
	if(p) {
		p->updatePixmap();
		// update item (graphlist changes)
		item = p->getPlot(p->API())->getGraphList()->Number()-1;
	}

	return status;
}
*/

void FunctionDialog::Apply() {
	kdDebug()<<"FunctionDialog::Apply()"<<endl;

	QString f = functionle->text().toLower();
	f.remove(QRegExp(".*="));		// remove any "xyz =" before expression
	kdDebug()<<"Parsing "<<f<<endl;

	int NX = nxi->value();
	Point *data = new Point[NX];
	double xmin = parse((char *) qPrintable(minle->text()));
	double xmax = parse((char *) qPrintable(maxle->text()));
	init_table();
	QProgressDialog progress( i18n("Creating data ..."), i18n("Cancel"), 0, NX);
	for(int i = 0;i < NX;i++) {
		if(i%10000 == 0) progress.setValue( i );

		double x = (xmax-xmin)*i/(double)(NX-1)+xmin;
		char var[]="x";
		assign_variable(var,x);
		double y = parse((char *) qPrintable(f));
		if(parse_errors()>0) {
			progress.cancel();
			KMessageBox::error(this, i18n("Syntax error!\nPlease check the given function."));
			delete_table();
			return;
		}
		if (progress.wasCanceled()) {
			kdDebug()<<"WARNING: Creating data canceled()"<<endl;
             		return;
		}
		data[i].setPoint(x,y);
	}

	// create Set
	Set2D *g = new Set2D(f,data,NX);
	setupLabel(g->getLabel());
//	Set2D *g = new Set2D(f,title,range,SFUNCTION,type,style,symbol,ptr,NX);
	// create Plot
	// TODO : other plot types
	mw->addSet(g,sheetcb->currentIndex(),PLOT2D);
}

/* OLD:
int FunctionDialog::addFunction() {
	int NY = 0;
	if (type == P3D || type == PSURFACE || type == PQWT3D)
		NY = ny->value();

	QProgressDialog progress( i18n("Creating function ..."), i18n("Cancel"), NX,this, "progress", true );
	progress.setMinimumDuration(2000);
	if (type == P2D || type == PPOLAR) {
		kdDebug()<<"	\"2d\" or \" polar \" selected"<<endl;
		Point *ptr = new Point[NX];

		double ymi=0, yma=1;
		double xmi = parse((char *) xmin->text().latin1());
		double xma = parse((char *) xmax->text().latin1());
		init_table();

		for(int i = 0;i < NX;i++) {
			if(i%100 == 0) progress.setProgress( i );
    			qApp->processEvents();

			double x = (xma-xmi)*i/(double)(NX-1)+xmi;
			char var[]="x";
			assign_variable(var,x);
//			kdDebug()<<"	parsing "<<fun.latin1()<<endl;
#ifdef HAVE_GSL
			gsl_set_error_handler_off();
#endif
			double y = parse((char *) fun.latin1());

			if(parse_errors()>0) {
				progress.cancel();
				KMessageBox::error(mw, i18n("Parse Error!\n Please check the given function."));
				delete_table();
				return 1;
			}

			bool nanvalue=false;
			if (!finite(x))	{ x=0; nanvalue=true; }
			if (!finite(y))	{ y=0; nanvalue=true; }

			if (i == 0) ymi=yma=y;
			y<ymi?ymi=y:0;
			y>yma?yma=y:0;

			ptr[i].setPoint(x,y);
			if(nanvalue)
				ptr[i].setMasked();
			if ( progress.wasCancelled() ) {
				delete_table();
				return 1;
			}
		}
		delete_table();

		if(p!=0 && graph != 0)
			p->getPlot(p->API())->getGraphList()->delGraph(item);

		if(yma-ymi == 0) {
			ymi -= 1;
			yma += 1;
		}

		if(type == PPOLAR) {
			xmi = 0;
			xma = 2*M_PI;
		}

		LRange range[2];
		range[0] = LRange(xmi,xma);
		range[1] = LRange(ymi,yma);
		Style *style = new Style((StylesType)cb2->currentItem(),color->color(),filled->isChecked(),
			fcolor->color(),width->value(),
			pencb->currentItem(),brushcb->currentItem());
		style->setBoxWidth(boxwidth->value());
		style->setAutoBoxWidth(autobox->isChecked());
		style->setPointsSorting(sortpointscb->isChecked());
		Symbol *symbol = new Symbol((SType)symbolcb->currentItem(),scolor->color(),ssize->value(),
			(FType)symbolfillcb->currentItem(),sfcolor->color(),sbrushcb->currentItem());

		kdDebug()<<"	range[0]="<<range[0].rMin()<<","<<range[0].rMax()<<endl;
		kdDebug()<<"	range[1]="<<range[1].rMin()<<","<<range[1].rMax()<<endl;

		Graph2D *g = new Graph2D(fun,title,range,SFUNCTION,type,style,symbol,ptr,NX);
		g->setLabel(label);
		kdDebug()<<"	label->isTeXLabel() : "<<label->isTeXLabel()<<endl;

		AnnotateValues av(typecb->currentItem(),positioncb->currentItem(),distance->value());
		g->setAnnotateValues(av);
		mw->addGraph2D(g,sheetcb->currentItem(),type);
	}
	else if (type == PSURFACE || type == PQWT3D) {
		kdDebug()<<"	\"surface\" or \" qwt 3d \" selected"<<endl;
		kdDebug()<<"	NX = "<<NX<<"/ NY = "<<NY<<endl;

		double *a = new double[NY*NX];

		double xmi=0, xma=1, ymi=0, yma=1, zmin=0, zmax=1;
		xmi = parse((char *) (xmin->text()).latin1());
		xma = parse((char *) (xmax->text()).latin1());
		ymi = parse((char *) (ymin->text()).latin1());
		yma = parse((char *) (ymax->text()).latin1());
		init_table();

		for (int i=0;i<NY;i++) {
			if(i%10==0)progress.setProgress( i );
    			qApp->processEvents();
			char var[]="y";
			assign_variable(var,(yma-ymi)*i/(double)(NY-1)+ymi);

			for (int j=0;j<NX;j++) {
				var[0]='x';
				assign_variable(var,(xma-xmi)*j/(double)(NX-1)+xmi);
#ifdef HAVE_GSL
			gsl_set_error_handler_off();
#endif
				double z = parse((char *) fun.latin1());

				if(parse_errors()>0) {
					progress.cancel();
					KMessageBox::error(mw, i18n("Parse Error!\n Please check the given function."));
					delete_table();
					return 1;
				}

				bool nanvalue=false;
				if (!finite(z)) { z=0; nanvalue=true; }

				//kdDebug()<<"z = "<<z<<" (i/j = "<<i<<'/'<<j<<")\n";

				if (i == 0 && j == 0) {
					zmin=z;
					zmax=z;
				}

				z<zmin?zmin=z:0;
				z>zmax?zmax=z:0;

				a[j+NX*i] = z;
				// TODO : mask if nanvalue
			}
			if ( progress.wasCancelled() ) {
				delete_table();
				return 1;
			}
		}
		delete_table();

		if(p==0) kdDebug()<<"	p==0"<<endl;
		if( p!=0 && type == PSURFACE) {
			kdDebug()<<"	p!=0 && type == PSURFACE"<<endl;
			Plot2DSurface *plot = (Plot2DSurface *)p->getPlot(p->API());

			// edit graph
			if(graph != 0)
				plot->getGraphList()->delGraph(item);

			if (plot != 0) {
				kdDebug()<<"	plot!=0"<<endl;
				kdDebug()<<"	reading settings"<<endl;
				plot->enableDensity(dcb->isChecked());
				plot->enableContour(ccb->isChecked());
				plot->setNumber(numberle->text().toInt());
				// OLD : plot->setPalette(pcb->currentItem());
				plot->setContourColor(contourcolor->color());
				plot->setColoredContour(coloredcb->isChecked());
				plot->setContourWidth(contourwidthle->text().toInt());
				plot->setMesh(meshcb->isChecked());
				plot->setRelative(relativecb->isChecked());
				plot->setBrush(dbrushcb->currentItem());
				plot->setThreshold(thresholdle->text().toDouble());
			}
		}
		else if (type == PQWT3D) {
			kdDebug() << "\"qwt 3d \" selected"<<endl;
			// TODO
			//PlotQWT3D *plot = (PlotQWT3D *)p->getPlot(p->API());
		}

		if(zmax-zmin == 0) {
			zmin -= 1;
			zmax += 1;
		}

		LRange range[3];
		// old : used 0..NX,0..NY
		// new : use range
		range[0] = LRange(xmi,xma);
		range[1] = LRange(ymi,yma);
		range[2] = LRange(zmin,zmax);

		kdDebug()<<"zmin : "<<zmin<<"/ zmax : "<<zmax<<endl;
		kdDebug()<<"range : "<<range[0].rMin()<<' '<<range[0].rMax()<<endl;
		kdDebug()<<"range : "<<range[1].rMin()<<' '<<range[1].rMax()<<endl;
		kdDebug()<<"range : "<<range[2].rMin()<<' '<<range[2].rMax()<<endl;
*/		/*for(int i=0;i<NX;i++) {
			for(int j=0;j<NY;j++) {
				kdDebug()<<" ("<<j+NY*i<<')'<<a[j+NY*i]<<endl;
			}
			kdDebug()<<endl;
		}*/

/*		Style *style = new Style(LINESTYLE);
		Symbol *symbol = new Symbol(SNONE);
		GraphM *g = new GraphM(fun,title,range,SFUNCTION,type,style,symbol,a,NX,NY);
		g->setLabel(label);
		mw->addGraphM(g,sheetcb->currentItem(),type);
	}
	else if (type == P3D) {
		kdDebug() << "\"3d\" selected"<<endl;

		Point3D *ptr = new Point3D[NX*NY];

		double xmi = xmin->text().toDouble(), xma = xmax->text().toDouble();
		double ymi=(ymin->text()).toDouble(), yma=(ymax->text()).toDouble();
		double zmin=0, zmax=1;
		init_table();

		for(int i = 0;i < NY;i++) {
			if(i%100==0)progress.setProgress( i );
    			qApp->processEvents();

			double y = (yma-ymi)*i/(double)(NY-1)+ymi;
			char var[]="y";
			assign_variable(var,y);

			for (int j = 0;j < NX;j++) {
				double x = (xma-xmi)*j/(double)(NX-1)+xmi;
				var[0]='x';
				assign_variable(var,x);
#ifdef HAVE_GSL
				gsl_set_error_handler_off();
#endif
				double z = parse((char *) fun.latin1());

				if(parse_errors()>0) {
					progress.cancel();
					KMessageBox::error(mw, i18n("Parse Error!\n Please check the given function."));
					delete_table();
					return 1;
				}

				bool nanvalue=false;
				if (!finite(z)) { z=0; nanvalue=true; }

				//kdDebug()<<"z = "<<z<<endl;

				if (i == 0 && j==0 ) zmin=zmax=z;
				z<zmin?zmin=z:0;
				z>zmax?zmax=z:0;

				ptr[i*NX+j].setPoint(x,y,z);
				if(nanvalue)
					ptr[i*NX+j].setMasked();
			}
			if ( progress.wasCancelled() ) {
				delete_table();
				return 1;
			}
		}
		delete_table();

		if(graph != 0)
			p->getPlot(p->API())->getGraphList()->delGraph(item);

		if(zmax-zmin == 0) {
			zmin -= 1;
			zmax += 1;
		}

		LRange range[3];
		range[0] = LRange(xmi,xma);
		range[1] = LRange(ymi,yma);
		range[2] = LRange(zmin,zmax);

		kdDebug()<<"Z RANGE = "<<zmin<<' '<<zmax<<endl;

		Style *style = new Style((StylesType)cb2->currentItem(),color->color(),filled->isChecked(),fcolor->color(),width->value(),
			pencb->currentItem(),brushcb->currentItem());
		style->setBoxWidth(boxwidth->value());
		style->setAutoBoxWidth(autobox->isChecked());
		style->setPointsSorting(sortpointscb->isChecked());
		Symbol *symbol = new Symbol((SType)symbolcb->currentItem(),scolor->color(),ssize->value(),
				(FType)symbolfillcb->currentItem(),sfcolor->color(),sbrushcb->currentItem());

		Graph3D *g = new Graph3D(fun,title,range,SFUNCTION,P3D,style,symbol,ptr,NX,NY);
		g->setLabel(label);
		AnnotateValues av(typecb->currentItem(),positioncb->currentItem(),distance->value());
		g->setAnnotateValues(av);
		mw->addGraph3D(g,sheetcb->currentItem());
	}

	// create new label
	Label* nlabel = new Label(*label);
	rtw->setLabel(nlabel);

	kdDebug()<<"FunctionDialog::apply_clicked() : DONE"<<endl;

	return 0;
}*/
