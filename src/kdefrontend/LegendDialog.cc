/***************************************************************************
    File                 : LegendDialog.cc
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Alexander Semke
    Email (use @ for *)  : alexander.semke*web.de
    Description          : legend settings dialog

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
#include "LegendDialog.h"
#include "LegendWidget.h"
#include "../elements/Legend.h"
#include "Worksheet.h"
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
