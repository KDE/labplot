/***************************************************************************
    File                 : LegendWidget.cc
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Alexander Semke
    Email (use @ for *)  : alexander.semke*web.de
    Description          : legend settings widget

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/
#include "LegendWidget.h"
#include "../elements/Legend.h"
#include <KDebug>

LegendWidget::LegendWidget(QWidget* parent):QWidget(parent){
	ui.setupUi(this);
	initializing=false;

	//Validators
	ui.lePositionX->setValidator( new QDoubleValidator(ui.lePositionX) );
	ui.lePositionY->setValidator( new QDoubleValidator(ui.lePositionY) );

	//Slots
	connect( ui.chbLegend, SIGNAL(stateChanged(int)), this, SLOT(slotDataChanged()) );
	connect( ui.lePositionX, SIGNAL(textChanged(const QString&)), this, SLOT(slotDataChanged()) );
	connect( ui.lePositionY, SIGNAL(textChanged(const QString&)), this, SLOT(slotDataChanged()) );
	connect( ui.cbOrientation, SIGNAL(currentIndexChanged(int)), this, SLOT(slotDataChanged()) );

	connect( ui.rbFilling0, SIGNAL(toggled(bool)), this, SLOT( fillingChanged(bool)) );
	connect( ui.rbFilling1, SIGNAL(toggled(bool)), this, SLOT( fillingChanged(bool)) );
	connect( ui.kcbFillingColor, SIGNAL(changed(const QColor& )), this, SLOT(slotDataChanged()) );
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
	ui.cbOrientation->setCurrentIndex( l->orientation()-1 ); //-1 because of Qt::Horizontal=0x1 and Qt::Vertical=0x2

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
	l->setEnabled( ui.chbLegend->isChecked()  );
	l->setPosition( QPointF(ui.lePositionX->text().toDouble(), ui.lePositionY->text().toDouble()) );
	l->setOrientation( Qt::Orientation(ui.cbOrientation->currentIndex()+1) );//+1 because of Qt::Horizontal=0x1 and Qt::Vertical=0x2

	l->setFillingColor( ui.kcbFillingColor->color( ) );
	if ( ui.rbFilling1->isChecked() )
		l->enableFilling(true);
	else
		l->enableFilling(false);

	l->enableBox( ui.chbBox->isChecked() );
	l->enableShadow( ui.chbShadow->isChecked() );

	l->setTextFont( ui.kfrTextFont->font()  );
	l->setTextColor( ui.kcbTextColor->color() );
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

/*!
	called, if the filling type (transparent or not) was changed.
	Enables/disables the button for choosing the filling color.
*/
void LegendWidget::fillingChanged(bool){
	if (ui.rbFilling0->isChecked())
		ui.kcbFillingColor->setEnabled(false);
	else
		ui.kcbFillingColor->setEnabled(true);

	this->slotDataChanged();
}
