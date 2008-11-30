/***************************************************************************
    File                 : AxesDialog.cc
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Stefan Gerlach, Alexander Semke
    Email (use @ for *)  : stefan.gerlach*uni-konstanz.de, alexander.semke*web.de
    Description          : axes settings dialog

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
#include "AxesDialog.h"
#include "AxesWidget.h"
#include "../elements/Axis.h"
#include "Worksheet.h"
#include "pixmaps/pixmap.h"
#include <KDebug>

AxesDialog::AxesDialog(QWidget* parent) : KDialog(parent){
	this->setCaption(i18n("Axes Settings"));
	this->setWindowIcon(KIcon(QIcon(axes_xpm)));

	axesWidget = new AxesWidget( this );
	this->setMainWidget( axesWidget );

	this->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply  | KDialog::Default );
	this->enableButtonApply( false );
 	resize( QSize(300,400) );

  	connect( this, SIGNAL( applyClicked() ), this, SLOT( apply() ) );
 	connect( this, SIGNAL( okClicked() ), this, SLOT( save() ) );
	connect( this, SIGNAL( defaultClicked() ), axesWidget, SLOT( restoreDefaults() ) );
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
	worksheet->repaint();//TODO triggers repaint for all plots and for all objects in the worksheet. redesign.
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
