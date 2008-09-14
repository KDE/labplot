#include "AxesDialog.h"
#include "AxesWidget.h"
#include "../elements/Axis.h"
#include "../Worksheet.h"
#include <KDebug>

AxesDialog::AxesDialog(QWidget* parent) : KDialog(parent){
	this->setCaption(i18n("Axes Settings"));
	this->setWindowIcon(KIcon(QIcon("axes-xpm")));

	axesWidget = new AxesWidget( this );
	this->setMainWidget( axesWidget );

	this->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply  | KDialog::Default );
	this->enableButtonApply( false );
 	resize( QSize(300,400) );

  	connect( this, SIGNAL( applyClicked() ), this, SLOT( apply() ) );
 	connect( this, SIGNAL( okClicked() ), this, SLOT( save() ) );
	connect( axesWidget, SIGNAL(dataChanged(bool)), SLOT(enableButtonApply(bool)) );

	kDebug()<<"Initialization done."<<endl;
}

AxesDialog::~AxesDialog(){}

void AxesDialog::setWorksheet(Worksheet* w){
	kDebug()<<"";
	worksheet=w;
 	axesWidget->setPlotType( worksheet->activePlot()->plotType() );
	axesWidget->setAxes( worksheet->activePlot()->axes() );
	enableButtonApply( false );
}

void AxesDialog::setAxes(QList<Axis>* axes, const int axisNumber){
  	axesWidget->setAxes(axes, axisNumber);
	enableButtonApply( false );
}

/*!
	called, if the user clicks the apply-button in the dialog.
	Triggers saving of the axes properties in AxisWidget
	and repainting of the axes in the worksheet.
*/
void AxesDialog::apply(){
	axesWidget->saveAxes( worksheet->activePlot()->axes() );
 	worksheet->repaint();//TODO triggers repaint for all plots in the worksheet. redesign.
	enableButtonApply( false );
	kDebug()<<"Changes applied."<<endl;
}


/*!
	called, if the user clicks the Ok-button in the dialog.
	Saves the changes, if needed, and closes the dialog.
*/
void AxesDialog::save(){
	if ( this->isButtonEnabled(KDialog::Apply) )
		this->apply();

	this->close();
	kDebug()<<"Changes saved."<<endl;
}
