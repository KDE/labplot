#include "LegendWidget.h"
#include "../elements/Legend.h"
#include <KDebug>

LegendWidget::LegendWidget(QWidget* parent):QWidget(parent){
	kDebug()<<""<<endl;
	ui.setupUi(this);
	initializing=false;

	//Validators
	ui.lePositionX->setValidator( new QDoubleValidator(ui.lePositionX) );
	ui.lePositionY->setValidator( new QDoubleValidator(ui.lePositionY) );

	//Slots
	connect( ui.chbLegend, SIGNAL(stateChanged(int)), this, SLOT(slotDataChanged()) );
	connect( ui.lePositionX, SIGNAL(textChanged(const QString&)), this, SLOT(slotDataChanged()) );
	connect( ui.lePositionY, SIGNAL(textChanged(const QString&)), this, SLOT(slotDataChanged()) );

	connect( ui.rbFilling0, SIGNAL(toggled(bool)), this, SLOT( fillingChanged(bool)) );
	connect( ui.rbFilling1, SIGNAL(toggled(bool)), this, SLOT( fillingChanged(bool)) );
	connect( ui.chbBox, SIGNAL(stateChanged(int)), this, SLOT(slotDataChanged()) );
	connect( ui.chbShadow, SIGNAL(stateChanged(int)), this, SLOT(slotDataChanged()) );

	connect( ui.kfrTextFont, SIGNAL(fontSelected(const QFont& )), this, SLOT(slotDataChanged()) );
	connect( ui.kcbTextColor, SIGNAL(changed(const QColor& )), this, SLOT(slotDataChanged()) );
	kDebug()<<"initializaton done."<<endl;
}


LegendWidget::~LegendWidget(){}

/*!
	show the properties of the Legend-object \c l in the UI.
*/
void LegendWidget::setLegend(const Legend* l){
	kDebug()<<""<<endl;
	initializing=true;
	ui.chbLegend->setChecked( l->isEnabled() );
	ui.lePositionX->setText( QString().setNum( l->position().x() ) );
	ui.lePositionY->setText( QString().setNum( l->position().y() ) );
	ui.cbOrientation->setCurrentIndex( l->orientation() );

	ui.kcbFillingColor->setColor( l->fillingColor() );
	if ( l->hasFilling() )
		ui.rbFilling1->setChecked(true);
	else
		ui.rbFilling0->setChecked(true);


	ui.chbBox->setChecked( l->hasBox() );
	ui.chbShadow->setChecked( l->hasShadow() );

	ui.kfrTextFont->setFont( l->textFont() );
	ui.kcbTextColor->setColor( l->textColor() );
	initializing=false;
	kDebug()<<"legend shown."<<endl;
}

/*!
	saves the content of the UI to the legend-object \c l.
*/
void LegendWidget::saveLegend(Legend* l) const{
	kDebug()<<""<<endl;
	l->setEnabled( ui.chbLegend->isChecked()  );
	l->setPosition( QPoint(ui.lePositionX->text().toDouble(), ui.lePositionX->text().toDouble()) );
	l->setOrientation( (Qt::Orientation)ui.cbOrientation->currentIndex() );

	l->setFillingColor( ui.kcbFillingColor->color( ) );
	if ( ui.rbFilling1->isChecked() )
		l->enableFilling(true);
	else
		l->enableFilling(false);


	l->enableBox( ui.chbBox->isChecked() );
	l->enableShadow( ui.chbShadow->isChecked() );

	ui.kfrTextFont->setFont( l->textFont() );
	ui.kcbTextColor->setColor( l->textColor() );
	kDebug()<<"legend saved."<<endl;
}


//**********************************************************
//****************** SLOTS *********************************
//**********************************************************
void LegendWidget::slotDataChanged(){
	if (initializing)
		return;

	emit dataChanged(true);
}

void LegendWidget::fillingChanged(bool){
	if (ui.rbFilling0->isChecked())
		ui.kcbFillingColor->setEnabled(false);
	else
		ui.kcbFillingColor->setEnabled(true);

	this->slotDataChanged();
}