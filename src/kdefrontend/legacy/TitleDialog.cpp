/***************************************************************************
    File                 : TitleDialog.cc
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Stefan Gerlach, Alexander Semke
    Email (use @ for *)  : stefan.gerlach*uni-konstanz.de, alexander.semke*web.de
    Description          : title dialog
                           
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
#include "TitleDialog.h"
#include "LabelWidget.h"
#include "worksheet/Worksheet.h"
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
	//labelWidget->setLabel( worksheet->activePlot()->titleLabel() );
	enableButtonApply( false );
}

/*!
	called, if the user clicks the apply-button in the dialog.
	Triggers saving of the label properties in LabelWidget
	and repainting of the Title in the worksheet.
*/
void TitleDialog::apply() {
	//labelWidget->saveLabel(worksheet->activePlot()->titleLabel());
	//worksheet->repaint();//TODO triggers repaint for all plots in the worksheet. redesign.
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
