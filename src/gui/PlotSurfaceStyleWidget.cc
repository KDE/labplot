#include "ColorMapPreview.h"
#include "PlotSurfaceStyleWidget.h"
#include "../elements/Symbol.h"
#include "../elements/Style.h"

#include <KFileDialog>
#include <KDebug>

PlotSurfaceStyleWidget::PlotSurfaceStyleWidget(QWidget* parent):QWidget(parent), PlotStyleWidgetInterface(){

	ui.setupUi(this);

	//Slots
	connect( ui.bColorMap, SIGNAL(clicked()), this, SLOT(selectColormap()) );
}

PlotSurfaceStyleWidget::~PlotSurfaceStyleWidget(){}

void PlotSurfaceStyleWidget::setStyle(const Style* style){
	ui.kcbAreaFillingColor->setColor(style->fillColor());
	//TODO
}

void PlotSurfaceStyleWidget::saveStyle(Style* style) const{
	style->setFillColor(ui.kcbAreaFillingColor->color());
	//TODO
}

//**********************************************************
//****************** SLOTS ********************************
//**********************************************************
void PlotSurfaceStyleWidget::selectColorMap(){
// 	KUrl url=KUrl::fromPath(locate("data","LabPlot/colormaps/");
	KUrl url=KUrl::fromPath("../../1.6.0/examples/colormaps"); //TODO !!!
	QString filter="Colormap files (*.map; *.MAP)";
/*
	KFileDialog* dialog = new KFileDialog(url, filter, this);
	ColorMapPreview* colormap= new ColorMapPreview( this );
	dialog->setPreviewWidget(colormap);

	if (dialog->exec()){
		//get the pixmal and show on the button
	}*/


	//OLD
// 	connect(dialog, SIGNAL(fileHighlighted(const QString&)), this, SLOT(adaptDataColors(const QString&)));
// 	datacolor->show();
}


//TODO old code. Make sure everything was taken over and remove the old stuff.
/*
//! used from dialogs for surface plots
QVBox* Dialog::surfaceStyle(QTabWidget *tw, bool fresh) {
	kdDebug()<<"Dialog::surfaceStyle(): fresh="<<fresh<<endl;
	QVBox *styletab = new QVBox(tw);
	KConfig *config = mw->Config();
	config->setGroup( "Plot Surface Style" );

	//TODO : bug?
	Plot *tmpplot=0;
	if(p) {
		tmpplot = p->getPlot(p->API());
		kdDebug()<<"	Getting plot "<<p->API()<<" from worksheet"<<endl;
	}
	Plot2DSurface *plot=0;
	if (tmpplot && tmpplot->Type() == PSURFACE)
		plot = (Plot2DSurface *)tmpplot;
	else {
		kdDebug()<<"Creating new Surface Plot"<<endl;
		if(p) plot = new Plot2DSurface(p);
	}
	if(plot) kdDebug()<<"	gl->Number()="<<plot->getGraphList()->Number()<<endl;

	QHBox *hb = new QHBox(styletab);
	dcb = new QCheckBox(i18n("density"),hb);
	dcb->setChecked(fresh?config->readBoolEntry("Density Enabled",true):plot->densityEnabled());
	ccb = new QCheckBox(i18n("contour"),hb);
	ccb->setChecked(fresh?config->readBoolEntry("Contour Enabled",true):plot->contourEnabled());

	hb = new QHBox(styletab);
	new QLabel(i18n("number of level : "),hb);
	numberle = new KLineEdit(QString::number(fresh?config->readNumEntry("Contour Level",10):
		plot->Number()),hb);
	numberle->setValidator(new QIntValidator(numberle));
	numberle->setMaximumWidth(100);
	hb = new QHBox(styletab);
	new QLabel(i18n("contour line color : "),hb);
	contourcolor = new KColorButton(fresh?config->readColorEntry("Contour Color",&Qt::black):
		plot->ContourColor(),hb);
	new QLabel(i18n("contour line width : "),hb);
	contourwidthle = new KLineEdit(QString::number(fresh?config->readNumEntry("Contour Width",1):
		plot->ContourWidth()),hb);

	hb = new QHBox(styletab);
	new QLabel(i18n("Brush : "),hb);
	dbrushcb = new KComboBox(hb);
	fillBrushBox(dbrushcb,SRECT,Qt::blue,FFULL,Qt::red);
	dbrushcb->setCurrentItem(fresh?config->readNumEntry("Density Brush",1):plot->Brush());

	hb = new QHBox(styletab);
	coloredcb = new QCheckBox(i18n("colored contour lines"),hb);
	coloredcb->setChecked(fresh?config->readBoolEntry("Colored Contour",false):plot->ColoredContour());
	meshcb = new QCheckBox(i18n("show mesh"),hb);
	meshcb->setChecked(fresh?config->readBoolEntry("Show Mesh",false):plot->Mesh());
	relativecb = new QCheckBox(i18n("relative color scale"),hb);
	relativecb->setChecked(fresh?config->readBoolEntry("Relative Colorscale",true):plot->Relative());

	hb = new QHBox(styletab);
	new QLabel(i18n(" Threshold : "),hb);
	thresholdle = new KLineEdit(QString::number(fresh?config->readDoubleNumEntry("Threshold",-INF):
		plot->Threshold()),hb);
	thresholdle->setValidator(new QDoubleValidator(thresholdle));
	thresholdle->setMaximumWidth(100);

	hb = new QHBox(styletab);
	new QLabel(i18n(" Color Palette : "),hb);
	//palabel->setAlignment(Qt::AlignHCenter);

	// use QWT colorscale
	KPushButton *colormap = new KPushButton(i18n("Colormap"),hb);
	QObject::connect(colormap,SIGNAL(clicked()),SLOT(selectColormap()));

	return styletab;
}

*/
