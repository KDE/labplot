#include "AxesDialog.h"
#include "AxesWidget.h"
#include "../Axis.h"
#include "../MainWin.h"


AxesDialog::AxesDialog(MainWin *mw, const PlotType type) : KDialog(mw){

// 	kdDebug()<<"AxesDialog starting"<<endl;
	setCaption(i18n("Axes Settings"));

	this->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply  | KDialog::Default );
	axesWidget = new AxesWidget( this );
	axesWidget->setPlotType(type);
	this->setMainWidget( axesWidget );

	connect( this, SIGNAL( applyClicked() ), this, SLOT( saveAxesData() ) );
	connect( this, SIGNAL( okClicked() ), this, SLOT( saveAxesData() ) );
	connect( this, SIGNAL( defaultClicked() ), axesWidget, SLOT( restoreDefaults() ) );
	connect( this, SIGNAL( changed( bool ) ), this, SLOT( enableButtonApply( bool ) ) );

	this->enableButtonApply( false );
// 	this->resize(this->minimumSizeHint());
		resize( QSize(300,400) );
}

AxesDialog::~AxesDialog(){}

void AxesDialog::setAxesData(const QList<Axis> axes, const int axisNumber) const{
	axesWidget->setAxesData(axes, axisNumber);
}

void AxesDialog::saveAxesData() const{
// 	axesWidget->saveAxesData();
}
