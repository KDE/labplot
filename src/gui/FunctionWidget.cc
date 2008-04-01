#include "FunctionWidget.h"
#include "LabelWidget.h"
#include "PlotStyleWidget.h"

#include "../parser_struct.h"
extern "C" con constants[];
extern "C" init arith_fncts[];
#include "../parser_extern.h"

FunctionWidget::FunctionWidget(QWidget* parent):QWidget(parent){

	ui.setupUi(this);
	plotType=PLOT2D;

	//Validators
	ui.leAxis1Start->setValidator( new QDoubleValidator(ui.leAxis1Start) );
	ui.leAxis1End->setValidator( new QDoubleValidator(ui.leAxis1End) );
	ui.leAxis2Start->setValidator( new QDoubleValidator(ui.leAxis2Start) );
	ui.leAxis2End->setValidator( new QDoubleValidator(ui.leAxis2End) );

	//"Title"-tab
	//create a labelwidget
    QHBoxLayout* hboxLayout = new QHBoxLayout(ui.tabTitle);
	labelWidget=new LabelWidget(ui.tabTitle);
    hboxLayout->addWidget(labelWidget);

	//"Plotstyle"-tab
    hboxLayout = new QHBoxLayout(ui.tabPlotStyle);
	plotStyleWidget=new PlotStyleWidget(ui.tabPlotStyle);
    hboxLayout->addWidget(plotStyleWidget);


	ui.bClear->setIcon( KIcon("edit-clear-locationbar-rtl") );

	int f=0;
	QString tmp;
	while(true){
		tmp = arith_fncts[f++].fname;
		if(tmp.isEmpty())
			break;
		ui.cbFunctions->addItem( tmp+"()" );
	}

	f=0;
	while(true){
		QString tmp = constants[f++].name;
		if(tmp.isEmpty())
			break;
		ui.cbConstants->addItem( tmp );
	}


	//SLOTS
	connect( ui.bClear, SIGNAL(clicked()), ui.leFunction, SLOT(clear()) );
 	connect( ui.cbFunctions,SIGNAL(activated(const QString&)),SLOT(insert(const QString&)) );
	connect( ui.cbConstants,SIGNAL(activated(const QString&)),SLOT(insert(const QString&)) );
	/*
	//"Ticks"-tab
	connect( ui.cbTicksStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(ticksStyleChanged(int)) );
	connect( ui.bTicksColour, SIGNAL(clicked()), this, SLOT( ticksColourClicked()) );

	//"Tick labels"-tab
	connect( ui.cbLabelsFormat, SIGNAL(currentIndexChanged(int)), this, SLOT(labelFormatChanged(int)) );
	connect( ui.bLabelsFont, SIGNAL(clicked()), this, SLOT( labelFontClicked()) );
	connect( ui.bLabelsColour, SIGNAL(clicked()), this, SLOT( labelColourClicked()) );

	//"Grid"-tab
	connect( ui.sbMajorGridWidth, SIGNAL(valueChanged(int)), this, SLOT(createMajorGridStyles()) );
	//TODO colour
	connect( ui.sbMinorGridWidth, SIGNAL(valueChanged(int)), this, SLOT(createMinorGridStyles()) );
	//TODO colour
	*/


		/*
	Style *style=0;
	Symbol *symbol=0;
	LRange rx, ry;
	GRAPHType st = GRAPH2D;
	int nrx = 0, nry = 0;

	if(type == PPOLAR) {
		if(symbol) symbol->setType(SCROSS);
	}

	kDebug()<<"get values"<<endl;
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
		kDebug()<<"Struct : "<<st<<endl;

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

FunctionWidget::~FunctionWidget(){
}


/*!
	sets the current plot type for the axes to be edited in this widget.
	Depending on the current type adjusts the appearance of the widget.
*/
void FunctionWidget::setPlotType(const PlotType& type){
	//TODO


	if (type == PLOT2D)
		ui.leFunction->setText( QString("sin(x)") );
	else if (type == PLOT3D || type == PLOTSURFACE || type == PLOTQWT3D)
		ui.leFunction->setText( QString("sin(x+y)") );

	plotType=type;
}

/*!
	sets the pointer \c axes to list containing all the axes. \c axisNumber is the number of the axis to be edited.<br>
*/
void FunctionWidget::setFunction(const Function& func){

}

/*!

*/
void FunctionWidget::saveFunction(Function* func) const{
	//TODO

}


//**********************************************************
//****************** SLOTS ********************************
//**********************************************************
/*!
 	Shows the data for the axis \c number.
	Is called when the user changes the current axis in the corepsonding ComboBox
*/
void FunctionWidget::insert(const QString s){
	ui.leFunction->insert(s);
	ui.leFunction->setFocus();
}



/*
void FunctionDialog::setupGUI() {
	kDebug()<<"FunctionDialog::setupGUI()"<<endl;
	KConfigGroup conf(KSharedConfig::openConfig(),"Function");
	//TODO : config bug
	QString entry = QString("PlotType %1 ").arg(type);
//	kDebug()<<"	default function :"<<conf.readEntry(entry+"Function",i18n("sin(x)"))<<endl;

//den Rest entfernt
*/

/* OLD setupGUI:
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

/*
void FunctionDialog::saveSettings() {
	kDebug()<<"FunctionDialog::saveSettings()"<<endl;
	KConfigGroup conf(KSharedConfig::openConfig(),"Function");
	QString entry = QString("PlotType %1 ").arg(type);

	conf.writeEntry(entry+"Function",functionle->text());
	conf.writeEntry(entry+"XMin",minle->text());
	conf.writeEntry(entry+"XMax",maxle->text());
// 	if(ymin != 0 && ymax != 0) {
// 		config->writeEntry(entry+"YMin",ymin->text().toDouble());
// 		config->writeEntry(entry+"YMax",ymax->text().toDouble());
// 	}

	conf.writeEntry(entry+"NX",nxi->value());
	//if(ny != 0)
	//	config->writeEntry(entry+"NY",ny->value());
	// TODO	 : more stuff

	conf.config()->sync();
}
*/

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

/*
void FunctionDialog::insertConstant(QString c) {
	kDebug()<<"FunctionDialog::insertConstant("<<c<<")"<<endl;
	functionle->insert(c);
	functionle->setFocus();
}

void FunctionDialog::insertFunction(QString f) {
	kDebug()<<"FunctionDialog::insertFunction("<<f<<")"<<endl;
	functionle->insert(f);
	functionle->setFocus();
	functionle->setCursorPosition(functionle->cursorPosition()-1);
	// TODO : insert "x" ?
}
*/

/*
// search for a usable plot; add worksheet if necessary
void FunctionDialog::findPlot() {
	kDebug()<<"FunctionDialog::findPlot()"<<endl;
	int sitem = sheetcb->currentItem(), count = sheetcb->count();
	kDebug()<<"	sheetcb->currentItem() = "<<sitem<<" of "<<count<<endl;

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
		kDebug()<<"	QWT Plot found! type = "<<type<<endl;
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
		kDebug()<<"	reread is checked"<<endl;
		status = addFunction();
	}
	else {	// only edit function
		kDebug()<<"	plot type = "<<type<<endl;
		if(type == PSURFACE) {
			kDebug()<<"	surface plot"<<endl;
			if (p != 0) {
				kDebug()<<"		p != 0 surface plot"<<endl;
				Plot2DSurface *plot = (Plot2DSurface *)p->getPlot(p->API());

				if (plot != 0) {
					kDebug()<<"	reading settings"<<endl;
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

/*
void FunctionDialog::Apply() {
	kDebug()<<"FunctionDialog::Apply()"<<endl;

	QString f = functionle->text().toLower();
	f.remove(QRegExp(".*="));		// remove any "xyz =" before expression
	kDebug()<<"Parsing "<<f<<endl;

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
			kDebug()<<"WARNING: Creating data canceled()"<<endl;
             		return;
		}
		data[i].setPoint(x,y);
	}

	// create Set
	Set2D *g = new Set2D(f,data,NX);
	setupLabel(g->getLabel());
//	Set2D *g = new Set2D(f,title,range,SFUNCTION,type,style,symbol,ptr,NX);
	// create plot, TODO : other plot types
	mw->addSet(g,sheetcb->currentIndex(),PLOT2D);
}
*/

/* OLD:
int FunctionDialog::addFunction() {
	int NY = 0;
	if (type == P3D || type == PSURFACE || type == PQWT3D)
		NY = ny->value();

	QProgressDialog progress( i18n("Creating function ..."), i18n("Cancel"), NX,this, "progress", true );
	progress.setMinimumDuration(2000);
	if (type == P2D || type == PPOLAR) {
		kDebug()<<"	\"2d\" or \" polar \" selected"<<endl;
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
//			kDebug()<<"	parsing "<<fun.latin1()<<endl;
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

		kDebug()<<"	range[0]="<<range[0].rMin()<<","<<range[0].rMax()<<endl;
		kDebug()<<"	range[1]="<<range[1].rMin()<<","<<range[1].rMax()<<endl;

		Graph2D *g = new Graph2D(fun,title,range,SFUNCTION,type,style,symbol,ptr,NX);
		g->setLabel(label);
		kDebug()<<"	label->isTeXLabel() : "<<label->isTeXLabel()<<endl;

		AnnotateValues av(typecb->currentItem(),positioncb->currentItem(),distance->value());
		g->setAnnotateValues(av);
		mw->addGraph2D(g,sheetcb->currentItem(),type);
	}
	else if (type == PSURFACE || type == PQWT3D) {
		kDebug()<<"	\"surface\" or \" qwt 3d \" selected"<<endl;
		kDebug()<<"	NX = "<<NX<<"/ NY = "<<NY<<endl;

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

				//kDebug()<<"z = "<<z<<" (i/j = "<<i<<'/'<<j<<")\n";

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

		if(p==0) kDebug()<<"	p==0"<<endl;
		if( p!=0 && type == PSURFACE) {
			kDebug()<<"	p!=0 && type == PSURFACE"<<endl;
			Plot2DSurface *plot = (Plot2DSurface *)p->getPlot(p->API());

			// edit graph
			if(graph != 0)
				plot->getGraphList()->delGraph(item);

			if (plot != 0) {
				kDebug()<<"	plot!=0"<<endl;
				kDebug()<<"	reading settings"<<endl;
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
			kDebug() << "\"qwt 3d \" selected"<<endl;
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

		kDebug()<<"zmin : "<<zmin<<"/ zmax : "<<zmax<<endl;
		kDebug()<<"range : "<<range[0].rMin()<<' '<<range[0].rMax()<<endl;
		kDebug()<<"range : "<<range[1].rMin()<<' '<<range[1].rMax()<<endl;
		kDebug()<<"range : "<<range[2].rMin()<<' '<<range[2].rMax()<<endl;
*/		/*for(int i=0;i<NX;i++) {
			for(int j=0;j<NY;j++) {
				kDebug()<<" ("<<j+NY*i<<')'<<a[j+NY*i]<<endl;
			}
			kDebug()<<endl;
		}*/

/*		Style *style = new Style(LINESTYLE);
		Symbol *symbol = new Symbol(SNONE);
		GraphM *g = new GraphM(fun,title,range,SFUNCTION,type,style,symbol,a,NX,NY);
		g->setLabel(label);
		mw->addGraphM(g,sheetcb->currentItem(),type);
	}
	else if (type == P3D) {
		kDebug() << "\"3d\" selected"<<endl;

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

				//kDebug()<<"z = "<<z<<endl;

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

		kDebug()<<"Z RANGE = "<<zmin<<' '<<zmax<<endl;

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

	kDebug()<<"FunctionDialog::apply_clicked() : DONE"<<endl;

	return 0;
}*/

