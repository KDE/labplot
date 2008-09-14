#include "LegendDialog.h"
#include "LegendWidget.h"
#include "../elements/Legend.h"
#include "../Worksheet.h"
#include <KDebug>

LegendDialog::LegendDialog(QWidget* parent) : KDialog(parent){
	this->setCaption(i18n("Legend Settings"));
	this->setWindowIcon(KIcon("format-list-unordered"));

	legendWidget = new LegendWidget( this );
	this->setMainWidget( legendWidget );

	this->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );
	this->enableButtonApply( false );
	resize( QSize(100,200) );

	connect( this, SIGNAL( applyClicked() ), this, SLOT( apply() ) );
 	connect( this, SIGNAL( okClicked() ), this, SLOT( save() ) );
	connect( legendWidget, SIGNAL(dataChanged(bool)), SLOT(enableButtonApply(bool)) );

	kDebug()<<"Initialization done."<<endl;
}


void LegendDialog::setWorksheet(Worksheet* w){
	worksheet=w;
 	legendWidget->setLegend( worksheet->activePlot()->legend() );
}

/*!
	called, if the user clicks the apply-button in the dialog.
	Triggers saving of the legend properties in LegendWidget
	and repainting of the legend in the worksheet.
*/
void LegendDialog::apply(){
 	legendWidget->saveLegend( worksheet->activePlot()->legend() );
	worksheet->repaint();//TODO triggers repaint for all plots and for all objects in the worksheet. redesign.
	this->enableButtonApply( false );
	kDebug()<<"Changes applied."<<endl;
}


/*!
	called, if the user clicks the Ok-button in the dialog.
	Saves the changes, if needed, and closes the dialog.
*/
void LegendDialog::save(){
	if ( this->isButtonEnabled(KDialog::Apply) )
		this->apply();

	this->close();
	kDebug()<<"Changes saved."<<endl;
}
