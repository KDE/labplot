#include "TitleDialog.h"
#include "LabelWidget.h"
#include "../Worksheet.h"
#include <KDebug>

TitleDialog::TitleDialog(QWidget* parent): KDialog(parent){
	this->setCaption(i18n("Title Dialog"));
	this->setWindowIcon( KIcon("draw-text") );

	labelWidget = new LabelWidget(this);
	this->setMainWidget( labelWidget );
	this->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply);
	this->enableButtonApply( false );
	resize( QSize(300,400) );

	connect( labelWidget, SIGNAL(dataChanged(bool)), SLOT(enableButtonApply(bool)) );
	connect( this, SIGNAL( applyClicked() ), this, SLOT( apply() ) );
	connect( this, SIGNAL( okClicked() ), this, SLOT( save() ) );

	kDebug()<<"Initialization done."<<endl;
}

void TitleDialog::setWorksheet(Worksheet* w){
	worksheet=w;
	labelWidget->setLabel( worksheet->activePlot()->titleLabel() );
	enableButtonApply( false );
}

/*!
	called, if the user clicks the apply-button in the dialog.
	Triggers saving of the label properties in LabelWidget
	and repainting of the Title in the worksheet.
*/
void TitleDialog::apply() {
	labelWidget->saveLabel(worksheet->activePlot()->titleLabel());
	worksheet->repaint();//TODO triggers repaint for all plots in the worksheet. redesign.
	this->enableButtonApply( false );
	kDebug()<<"Changes applied."<<endl;
}


/*!
	called, if the user clicks the Ok-button in the dialog.
	Saves the changes, if needed, and closes the dialog.
*/
void TitleDialog::save(){
	if ( this->isButtonEnabled(KDialog::Apply) )
		this->apply();

	this->close();
	kDebug()<<"Changes saved."<<endl;
}
