#include "LegendWidget.h"
#include "../elements/Legend.h"

LegendWidget::LegendWidget(QWidget* parent):QWidget(parent){
	ui.setupUi(this);

	//Validators
	ui.lePositionX->setValidator( new QDoubleValidator(ui.lePositionX) );
	ui.lePositionY->setValidator( new QDoubleValidator(ui.lePositionY) );

	//Slots
	connect( ui.rbFilling0, SIGNAL(toggled(bool)), this, SLOT( fillingChanged(bool)) );
	connect( ui.rbFilling1, SIGNAL(toggled(bool)), this, SLOT( fillingChanged(bool)) );
}

void LegendWidget::setLegend(Legend* l){
	legend=l;
}

void LegendWidget::fillingChanged(bool){
	if (ui.rbFilling0->isChecked())
		ui.kcbFillingColor->setEnabled(false);
	else
		ui.kcbFillingColor->setEnabled(true);
}

void LegendWidget::save(){
	//TODO
}

LegendWidget::~LegendWidget(){
}
