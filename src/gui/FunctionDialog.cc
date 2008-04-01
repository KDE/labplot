#include "FunctionDialog.h"
#include "FunctionWidget.h"
#include "../MainWin.h"
#include "../Point.h"
#include "../Set2D.h"
#include <KDebug>

FunctionDialog::FunctionDialog(MainWin *mw, const PlotType& type)
	: KDialog(mw){

	kDebug()<<"FunctionDialog()"<<endl;

	functionWidget = new FunctionWidget( this );
	functionWidget->setPlotType(type);
	this->setMainWidget( functionWidget );

	this->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );

// 	Plot *plot=0;
// 	if(p != 0)
// 	 	plot = p->getPlot(p->API());
// 	type = newtype;

	QString caption;
	if (type == PLOT2D)
		caption=i18n("Add new 2D function");
	else if (type == PLOT3D)
		caption=i18n("Add new 3D function");
	else if (type == PLOTQWT3D)
		caption=i18n("Add new QWT function");
	else if (type == PLOTSURFACE)
		caption=i18n("Add new 2D surface function");
	else if (type == PLOTPOLAR)
		caption=i18n("2D Polar Function Dialog");

	//TODO caption += i18n(" : ")+QString(name);
	setCaption(caption);

	//SLOTs
	connect( this, SIGNAL( applyClicked() ), this, SLOT( apply() ) );
	connect( this, SIGNAL( okClicked() ), this, SLOT( save() ) );
	connect( this, SIGNAL( changed( bool ) ), this, SLOT( enableButtonApply( bool ) ) );

	this->enableButtonApply( false );
	resize( QSize(300,400) );
	kDebug()<<"FunctionDialog() DONE"<<endl;

}

void FunctionDialog::setFunction(Function* func){

}

void FunctionDialog::save() const{

}


void FunctionDialog::apply() const{

}
