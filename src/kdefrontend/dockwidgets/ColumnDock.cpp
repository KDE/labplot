/***************************************************************************
    File                 : ColumnDock.cpp
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Stefan Gerlach
    Email (use @ for *)  : stefan.gerlach*uni-konstanz.de
    Description          : widget for column properties
                           
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

#include "ColumnDock.h"
#include "core/column/Column.h"


 /*!
  \class SpreadsheetDock
  \brief Provides a widget for editing the properties of the spreadsheet columns currently selected in the project explorer.

  \ingroup kdefrontend
*/

ColumnDock::ColumnDock(QWidget *parent): QWidget(parent){
	ui.setupUi(this);

	connect(ui.cbType, SIGNAL(currentIndexChanged(int)), this, SLOT(columnTypeChanged(int)));
	
	// column type
	int i=0, j=0;
	do {
		QString item = SciDAVis::enumValueToString(j++, "ColumnMode");
		if(! item.isEmpty())
			ui.cbType->insertItem(i++,item);
	} while (i<5);
	
	// plot designation
	i=0, j=0;
	do{
		QString item = SciDAVis::enumValueToString(j++, "PlotDesignation");
		if(! item.isEmpty())
			ui.cbPlotDesignation->insertItem(i++,item);
	} while (i<6);
}

void ColumnDock::setColumns(QList<Column*> list){
  
}

// SLOTS 

void ColumnDock::columnTypeChanged(int index){
  ui.cbFormat->clear();
  
  switch (index){
	case 0:{
		  ui.cbFormat->addItem( QObject::tr( "Default" ) );
		  ui.cbFormat->addItem( QObject::tr( "Decimal: 1000" ) );
		  ui.cbFormat->addItem( QObject::tr( "Scientific: 1E3" ) );
		  
		  ui.cbFormat->setEditable( false );
	  }
	  //TODO borrow the code from scidavis
  }
		
	if (index==0){
	  ui.lPrecision->show();
	  ui.sbPrecision->show();
	}else{
	  ui.lPrecision->hide();
	  ui.sbPrecision->hide();
	}
	
	if (index==1){
	  ui.lFormat->hide();
	  ui.cbFormat->hide();
	}else{
	  ui.lFormat->show();
	  ui.cbFormat->show();
	}
}
