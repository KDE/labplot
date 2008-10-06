/***************************************************************************
    File                 : FunctionPlotWidget.cc
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Stefan Gerlach, Alexander Semke
    Email (use @ for *)  : stefan.gerlach*uni-konstanz.de, alexander.semke*web.de
    Description          : widget for creating plot function
                           
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
#include "FunctionPlotWidget.h"
#include "LabelWidget.h"
#include "PlotStyleWidget.h"
#include "PlotSurfaceStyleWidget.h"
#include "../elements/Set.h"
#include "../elements/Point3D.h"

#include <QMdiSubWindow>
#include <QProgressDialog>
#include <KDebug>
#include <KMessageBox>
#include <cmath>

#ifdef HAVE_GSL
#include <gsl/gsl_errno.h>
#include <gsl/gsl_math.h>
#endif

#include "../parser/parser_struct.h"
extern "C" con constants[];
extern "C" init arith_fncts[];
#include "../parser/parser_extern.h"

FunctionPlotWidget::FunctionPlotWidget(QWidget* parent):QWidget(parent){

	ui.setupUi(this);
	plotType=Plot::Plot::PLOT2D;

	//Validators
	ui.leAxis1Start->setValidator( new QDoubleValidator(ui.leAxis1Start) );
	ui.leAxis1End->setValidator( new QDoubleValidator(ui.leAxis1End) );
	ui.leAxis2Start->setValidator( new QDoubleValidator(ui.leAxis2Start) );
	ui.leAxis2End->setValidator( new QDoubleValidator(ui.leAxis2End) );

	//"Title"-tab ->create a LabelWidget
    QHBoxLayout* hboxLayout = new QHBoxLayout(ui.tabTitle);
	labelWidget=new LabelWidget(ui.tabTitle);
    hboxLayout->addWidget(labelWidget);

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
}

FunctionPlotWidget::~FunctionPlotWidget(){
}


/*!
	sets the current plot type  in this widget.
	Depending on the current type adjusts the appearance of the widget.
*/
void FunctionPlotWidget::setPlotType(const Plot::PlotType& type){
	//"Plotstyle"-tab
    QHBoxLayout* hboxLayout = new QHBoxLayout(ui.tabPlotStyle);
	if (type==Plot::PLOTSURFACE)
		plotStyleWidget=new PlotSurfaceStyleWidget(ui.tabPlotStyle);
	else
		plotStyleWidget=new PlotStyleWidget(ui.tabPlotStyle);

	hboxLayout->addWidget( dynamic_cast<QWidget*>(plotStyleWidget) );
	//TODO what about  3D-QWT-functions

	if (type == Plot::Plot::PLOT2D || type == Plot::PLOTPOLAR){
		ui.leFunction->setText( "sin(x)" );
		ui.leAxis1Start->setText( "0" );
		ui.leAxis1End->setText( "1" );
		ui.frameAxis2->hide();
	}else if (type == Plot::PLOT3D || type == Plot::PLOTSURFACE || type == Plot::PLOTQWT3D){
		ui.leFunction->setText( "sin(x+y)" );
		ui.leAxis1Start->setText( "0" );
		ui.leAxis1End->setText( "1" );
		ui.leAxis2Start->setText( "0" );
		ui.leAxis2End->setText( "1" );
		ui.frameAxis2->show();
	}

	plotType=type;
}


/*!
	set the set object \c set to be displayed/edited in this widget
*/
void FunctionPlotWidget::setSet(Set* set){
	ui.leFunction->setText( set->functionName() );

	ui.leAxis1Start->setText( QString::number(set->list_ranges.at(0).min()) );
	ui.leAxis1End->setText( QString::number(set->list_ranges.at(0).max()) );
	ui.sbAxis1Number->setValue( set->list_numbers.at(0) );

	if (plotType == Plot::PLOT3D || plotType == Plot::PLOTSURFACE || plotType == Plot::PLOTQWT3D){
		ui.leAxis2Start->setText( QString::number(set->list_ranges.at(2).min()) );
		ui.leAxis2End->setText( QString::number(set->list_ranges.at(2).max()) );
		ui.sbAxis2Number->setValue( set->list_numbers.at(1) );
	}

	labelWidget->setLabel(set->label());
	plotStyleWidget->setStyle(set->style());


	//TODO "Tick labels"


	//TODO old stuff
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

/*!

*/
void FunctionPlotWidget::saveSet(Set* set){//TODO add const
	set->setFunctionName( ui.leFunction->text() );

	set->list_ranges[0].setMin( ui.leAxis1Start->text().toFloat() );
	set->list_ranges[0].setMax( ui.leAxis1End->text().toFloat() );
	set->list_numbers[0] = ui.sbAxis1Number->value();

	if (plotType == Plot::PLOT3D || plotType == Plot::PLOTSURFACE || plotType == Plot::PLOTQWT3D){
		set->list_ranges[1].setMin( ui.leAxis2Start->text().toFloat() );
		set->list_ranges[1].setMax( ui.leAxis2End->text().toFloat() );
		set->list_numbers[1] = ui.sbAxis2Number->value();
	}

	labelWidget->saveLabel(set->label());
	plotStyleWidget->saveStyle(set->style());

	//TODO "Tick labels"

	//call this only if the function was changed.
 	this->createSetData(set);
	kDebug()<<"Set saved"<<endl;
}

int FunctionPlotWidget::createSetData(Set* set) {
	set->list_data.clear(); //clear the old stuff. a new data set is going to be created now.
	int NX=set->list_numbers[0];

	QProgressDialog progress( i18n("Creating function ..."), i18n("Cancel"), 0, NX, this );
	progress.setMinimumDuration(2000);
	progress.setWindowModality(Qt::WindowModal);
	bool nanvalue;
	double x, y;
	if (plotType == Plot::PLOT2D || plotType == Plot::PLOTPOLAR){
		kDebug()<<"	\"2d\" or \" polar \" selected"<<endl;
		double xmin = parse( QString::number(set->list_ranges[0].min()).toLatin1().data() );
		double xmax = parse( QString::number(set->list_ranges[0].max()).toLatin1().data() );
		double ymin=0, ymax=1;

		kDebug()<<"xmi="<<xmin<<"	xma="<<xmax<<endl;
		kDebug()<<"NX="<<NX<<endl;
		kDebug()<<"	parsing "<<set->functionName()<<endl;

		init_table();
		for(int i = 0;i < NX;i++) {
			if(i%100 == 0) progress.setValue( i );
    			qApp->processEvents();

			x = (xmax-xmin)*i/(double)(NX-1)+xmin;
			char var[]="x";
			assign_variable(var,x);
#ifdef HAVE_GSL
			gsl_set_error_handler_off();
#endif
   			y = parse( (char *) set->functionName().toLatin1().data() );

			if(parse_errors()>0) {
				progress.cancel();
				KMessageBox::error(this, i18n("Parse Error!\n Please check the given function."));
				delete_table();
				return 1;
			}

			nanvalue=false;
			if (!finite(x))	{ x=0; nanvalue=true; }
			if (!finite(y))	{ y=0; nanvalue=true; }

			if (i == 0) ymin=ymax=y;
			y<ymin?ymin=y:0;
			y>ymax?ymax=y:0;

			Point point(x, y);
			if(nanvalue)
				point.setMasked();

			if ( progress.wasCanceled() ) {
				delete_table();
				return 1;
			}

			set->list_data<<point;
// 			kDebug()<<"Point created:   x="<<point.x()<<",	 y="<<point.y()<<endl;
		}
		delete_table();

		if(ymax-ymin == 0) {
			ymin -= 1;
			ymax += 1;
		}

		if(plotType == Plot::PLOTPOLAR) {
			xmin = 0;
			xmax = 2*M_PI;
		}

		set->list_ranges.clear();
 		set->list_ranges<<Range(xmin, xmax);
 		set->list_ranges<< Range(ymin, ymax);
		kDebug()<<"	\"2D\" or \" polar \" data set created"<<endl;
	}else if( plotType == Plot::PLOT3D || plotType == Plot::PLOTSURFACE || plotType == Plot::PLOTQWT3D ){
 		int NY = set->list_numbers[1];

		kDebug()<<"	\"surface\" or \" qwt 3d \" selected"<<endl;
		kDebug()<<"	NX = "<<NX<<"/ NY = "<<NY<<endl;
		kDebug()<<"	parsing "<<set->functionName()<<endl;

		double xmin = parse( QString::number(set->list_ranges[0].min()).toLatin1().data() );
		double xmax = parse( QString::number(set->list_ranges[0].max()).toLatin1().data() );
		double ymin = parse( QString::number(set->list_ranges[1].min()).toLatin1().data() );
		double ymax = parse( QString::number(set->list_ranges[1].max()).toLatin1().data() );
		double zmin=0,  zmax=1;

		init_table();
		double z;
		for (int i=0;i<NY;i++) {
			if(i%10==0)
				progress.setValue( i );

    		qApp->processEvents();
			y = (ymax-ymin)*i/(double)(NY-1)+ymin;
			char var[]="y";
			assign_variable(var, y);

			for (int j=0;j<NX;j++) {
				x = (xmax-xmin)*j/(double)(NX-1)+xmin;
 				var[0]='x';
				assign_variable(var, x);
#ifdef HAVE_GSL
			gsl_set_error_handler_off();
#endif
				z = parse( (char *) set->functionName().toLatin1().data() );
				if(parse_errors()>0) {
					progress.cancel();
					KMessageBox::error(this, i18n("Parse Error!\n Please check the given function."));
					delete_table();
					return 1;
				}

				nanvalue=false;
				if (!finite(z)){
					z=0;
					nanvalue=true;
				}

				if (i == 0 && j == 0) {
					zmin=z;
					zmax=z;
				}

				z<zmin?zmin=z:0;
				z>zmax?zmax=z:0;

				Point3D point(x, y, z);
				if(nanvalue)
					point.setMasked();

				if ( progress.wasCanceled() ) {
					delete_table();
					return 1;
				}
 				set->list_data<<point;
				kDebug()<<"Point created:   x="<<x<<",	 y="<<y<<",	 z="<<z<<endl;
			}
		}
		delete_table();

		if(zmax-zmin == 0) {
			zmin -= 1;
			zmax += 1;
		}

		set->list_ranges.clear();
 		set->list_ranges<<Range(xmin,xmax);
 		set->list_ranges<< Range(ymin,ymax);
		set->list_ranges<< Range(zmin,zmax);
		kDebug()<<"	\"3D\"  data set created"<<endl;
	}

	return 0;
}

//**********************************************************
//****************** SLOTS ********************************
//**********************************************************
/*!
 	Shows the data for the axis \c number.
	Is called when the user changes the current axis in the corresponding ComboBox
*/
void FunctionPlotWidget::insert(const QString s){
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
	mw->addSet(g,sheetcb->currentIndex(),Plot::PLOT2D);
}
*/

// OLD:
/*
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
*/
	/*for(int i=0;i<NX;i++) {
			for(int j=0;j<NY;j++) {
				kDebug()<<" ("<<j+NY*i<<')'<<a[j+NY*i]<<endl;
			}
			kDebug()<<endl;
		}*/
/*
	Style *style = new Style(LINESTYLE);
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
}
*/
