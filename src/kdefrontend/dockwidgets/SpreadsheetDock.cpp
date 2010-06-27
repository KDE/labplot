/***************************************************************************
    File                 : SpreadsheetDock.cpp
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2010 by Alexander Semke (alexander.semke*web.de)
									 (replace * with @ in the email addresses)
    Description          : widget for spreadsheet properties
                           
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

#include "SpreadsheetDock.h"
#include "commonfrontend/spreadsheet/SpreadsheetView.h"
#include "spreadsheet/Spreadsheet.h"

 /*!
  \class SpreadsheetDock
  \brief Provides a widget for editing the properties of the spreadsheets currently selected in the project explorer.

  \ingroup kdefrontend
*/
 
SpreadsheetDock::SpreadsheetDock(QWidget *parent): QWidget(parent){
	ui.setupUi(this);
	
	connect(ui.leName, SIGNAL(returnPressed()), this, SLOT(nameChanged()));
  //connect(ui.leName, SIGNAL(lostFocus()), this, SLOT(nameChanged()));
  //TODO add slot for the ui.teComment
	
	connect(ui.sbColumnCount, SIGNAL(valueChanged(int)), this, SLOT(columnCountChanged(int)));
	connect(ui.sbRowCount, SIGNAL(valueChanged(int)), this, SLOT(columnRowChanged(int)));
	
	connect(ui.cbShowComments, SIGNAL(stateChanged(int)), this, SLOT(commentsShownChanged(int)));
}

/*!
  
*/
void SpreadsheetDock::setSpreadsheets(QList<Spreadsheet*> list){
  Spreadsheet* spreadsheet;
  spreadsheet=list.first();
//   qDebug()<<"size"<<list.size();

  Q_ASSERT(spreadsheet);
  
  if (list.size()==1){
	ui.leName->setEnabled(true);
	ui.teComment->setEnabled(true);
	
  ui.leName->setText(spreadsheet->name());
  ui.teComment->setText(spreadsheet->comment());
  }else{
	//disable the fields "Name" and "Comment" if there are >1 Spreadsheets
	ui.leName->setEnabled(false);
	ui.teComment->setEnabled(false);
	
	ui.leName->setText("");
	ui.teComment->setText("");
  }
  
  //show the properties of the first Spreadsheet in the list, if there are >1 spreadsheets
  ui.sbColumnCount->setValue(spreadsheet->columnCount());
  ui.sbRowCount->setValue(spreadsheet->rowCount());
  SpreadsheetView* view= qobject_cast<SpreadsheetView*>(spreadsheet->view());
  ui.cbShowComments->setChecked(view->areCommentsShown());
  
  m_spreadsheets=list;
}

// ###### SLOTS  ##############

//TODO
void SpreadsheetDock::nameChanged(){
  
}


void SpreadsheetDock::commentChanged(){
  
}

void SpreadsheetDock::rowCountChanged(int c){
  
}

void SpreadsheetDock::columnCountChanged(int c){
  
}

/*!
  switches on/off  the comment header in the views of the selected spreadsheets.
*/
void SpreadsheetDock::commentsShownChanged(int state){
  Spreadsheet* spreadsheet;
  foreach(spreadsheet, m_spreadsheets)
	qobject_cast<SpreadsheetView*>(spreadsheet->view())->showComments(state);
}
