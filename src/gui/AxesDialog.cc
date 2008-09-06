#include "AxesDialog.h"
#include "AxesWidget.h"
#include "../elements/Axis.h"
#include "../Worksheet.h"
#include "../MainWin.h"
#include <KDebug>

AxesDialog::AxesDialog(MainWin *mw, const Plot::PlotType type) : KDialog(mw){
  	kDebug()<<"AxesDialog starting"<<endl;
	setCaption(i18n("Axes Settings"));

	axesWidget = new AxesWidget( this );
	this->setMainWidget( axesWidget );
	this->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply  | KDialog::Default );

	connect( this, SIGNAL( applyClicked() ), this, SLOT( apply() ) );
	connect( this, SIGNAL( okClicked() ), this, SLOT( save() ) );
	connect( this, SIGNAL( defaultClicked() ), axesWidget, SLOT( restoreDefaults() ) );
	connect( axesWidget, SIGNAL(dataChanged(bool)), SLOT(enableButtonApply(bool)) );

	this->enableButtonApply( false );
 	resize( QSize(300,400) );
}

AxesDialog::~AxesDialog(){}

void AxesDialog::setWorksheet(Worksheet* w){
	kDebug()<<"";
	worksheet=w;
 	axesWidget->setPlotType( worksheet->activePlot()->plotType() );
	axesWidget->setAxes( worksheet->activePlot()->axes() );
	this->enableButtonApply( false );
}

void AxesDialog::setAxes(QList<Axis>* axes, const int axisNumber){
  	axesWidget->setAxes(axes, axisNumber);
// 	list_axes=axes;
	this->enableButtonApply( false );
}

void AxesDialog::apply(){
 	axesWidget->saveAxes( worksheet->activePlot()->axes() );
	worksheet->repaint();//TODO triggers repaint for all plots in the worksheet. redesign.
	this->enableButtonApply( false );
	kDebug()<<"Changes applied."<<endl;
}

/*!
	save and close
*/
void AxesDialog::save(){
	this->apply();
	this->close();
	kDebug()<<"Changes saved."<<endl;
}