#include "AxesDialog.h"
#include "AxesWidget.h"
#include "../Axis.h"
#include "../MainWin.h"
#include <KDebug>

AxesDialog::AxesDialog(MainWin *mw, const PlotType type) : KDialog(mw){
  	kDebug()<<"AxesDialog starting"<<endl;
	setCaption(i18n("Axes Settings"));

	axesWidget = new AxesWidget( this );
	axesWidget->setPlotType(type);
	this->setMainWidget( axesWidget );
	this->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply  | KDialog::Default );

	connect( this, SIGNAL( applyClicked() ), this, SLOT( apply() ) );
	connect( this, SIGNAL( okClicked() ), this, SLOT( ok() ) );
	connect( this, SIGNAL( defaultClicked() ), axesWidget, SLOT( restoreDefaults() ) );
	connect( this, SIGNAL( changed( bool ) ), this, SLOT( enableButtonApply( bool ) ) );

	this->enableButtonApply( false );
// 	this->resize(this->minimumSizeHint());
	resize( QSize(300,400) );
}

AxesDialog::~AxesDialog(){}

void AxesDialog::setAxesData(const QList<Axis> axes, const int axisNumber) const{
	axesWidget->setAxesList(axes, axisNumber);
}

void AxesDialog::apply(){
// 	axesWidget->saveAxesData();
}

/*!
	save and close
*/
void AxesDialog::save(){

}
