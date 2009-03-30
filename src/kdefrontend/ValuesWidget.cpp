/***************************************************************************
    File                 : ValuesWidget.cc
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Alexander Semke
    Email (use @ for *)  : alexander.semke*web.de
    Description          : values widget class

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
#include "ValuesWidget.h"
#include "../elements/Label.h"
#include <KDebug>

ValuesWidget::ValuesWidget(QWidget *parent): QWidget(parent){

	ui.setupUi(this);

	//labels format
	 ui.cbLabelsFormat->addItem( i18n("automatic") );
	 ui.cbLabelsFormat->addItem( i18n("normal") );
	 ui.cbLabelsFormat->addItem( i18n("scientific") );
	 ui.cbLabelsFormat->addItem( i18n("power of 10") );
	 ui.cbLabelsFormat->addItem( i18n("power of 2") );
	 ui.cbLabelsFormat->addItem( i18n("power of e") );
	 ui.cbLabelsFormat->addItem( i18n("sqrt") );
	 ui.cbLabelsFormat->addItem( i18n("time") );
	 ui.cbLabelsFormat->addItem( i18n("date") );
	 ui.cbLabelsFormat->addItem( i18n("datetime") );
	 ui.cbLabelsFormat->addItem( i18n("degree") );

	//Slots
 	//General
	connect( ui.cbType, SIGNAL(currentIndexChanged(int)), this, SLOT(slotDataChanged()) );
	connect( ui.cbPlacement, SIGNAL(currentIndexChanged(int)), this, SLOT(slotDataChanged()) );
	connect( ui.sbRotation, SIGNAL(valueChanged(int)), this, SLOT(slotDataChanged()) );
	connect( ui.sbDistance, SIGNAL(valueChanged(int)), this, SLOT(slotDataChanged()) );

	//Format
	connect( ui.sbLabelsPrecision, SIGNAL(valueChanged(int)), this, SLOT(slotDataChanged()) );
	connect( ui.cbLabelsFormat, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(labelFormatChanged(const QString&)) );
	connect( ui.leLabelsDateFormat, SIGNAL(textChanged(const QString&)), this, SLOT(slotDataChanged()) );

	//Text
 	connect( ui.kcbTextColor, SIGNAL(changed(const QColor& )), this, SLOT(slotDataChanged()) );
 	connect( ui.kfrTextFont, SIGNAL(fontSelected(const QFont& )), this, SLOT(slotDataChanged()) );
}

ValuesWidget::~ValuesWidget() {}


//***********************************************************************************
//**********************************    SLOTS   **************************************
//***********************************************************************************
/*!
	called if the ticks format is changed in the corresponding ComboBox.
	Enables an additional text field for "Date-Format" if time/date/datetime selected,
	disables this field otherwise.
*/
void ValuesWidget::labelFormatChanged(const QString& text){
	if ( text != i18n("time") && text != i18n("date") && text != i18n("datetime") )
		ui.leLabelsDateFormat->setEnabled(false);
	else
		ui.leLabelsDateFormat->setEnabled(true);

	this->slotDataChanged();
}

void ValuesWidget::slotDataChanged(){
	emit dataChanged(true);
}
