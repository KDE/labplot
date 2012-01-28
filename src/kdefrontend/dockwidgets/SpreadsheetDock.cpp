/***************************************************************************
    File                 : SpreadsheetDock.cpp
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2010 by Alexander Semke (alexander.semke*web.de)
    Copyright            : (C) 2012 by Stefan Gerlach (stefan.gerlach*uni-konstanz.de)
    							(use @ for *)
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

#include <QFileDialog>

 /*!
  \class SpreadsheetDock
  \brief Provides a widget for editing the properties of the spreadsheets currently selected in the project explorer.

  \ingroup kdefrontend
*/
 
SpreadsheetDock::SpreadsheetDock(QWidget *parent): QWidget(parent){
	ui.setupUi(this);
	m_initializing = false;
	
	ui.tbLoad->setIcon(KIcon("document-open"));
	ui.tbSave->setIcon(KIcon("document-save"));
	ui.tbSaveDefault->setIcon(KIcon("document-save-as"));
	ui.tbCopy->setIcon(KIcon("edit-copy"));
	ui.tbPaste->setIcon(KIcon("edit-paste"));

	connect(ui.leName, SIGNAL(returnPressed()), this, SLOT(nameChanged()));
	connect(ui.leComment, SIGNAL(returnPressed()), this, SLOT(commentChanged()));
	connect(ui.sbColumnCount, SIGNAL(valueChanged(int)), this, SLOT(columnCountChanged(int)));
	connect(ui.sbRowCount, SIGNAL(valueChanged(int)), this, SLOT(rowCountChanged(int)));
	connect(ui.cbShowComments, SIGNAL(stateChanged(int)), this, SLOT(commentsShownChanged(int)));

	connect( ui.tbLoad, SIGNAL(clicked()), this, SLOT(loadSettings()));
	connect( ui.tbSave, SIGNAL(clicked()), this, SLOT(saveSettings()));
	connect( ui.tbSaveDefault, SIGNAL(clicked()), this, SLOT(saveDefaults()));
}

/*!
  
*/
void SpreadsheetDock::setSpreadsheets(QList<Spreadsheet*> list){
  Spreadsheet* spreadsheet;
  spreadsheet=list.first();

  Q_ASSERT(spreadsheet);
  
  m_initializing = true;
  
  if (list.size()==1){
	ui.leName->setEnabled(true);
	ui.leComment->setEnabled(true);
	
  ui.leName->setText(spreadsheet->name());
  ui.leComment->setText(spreadsheet->comment());
  }else{
	//disable the fields "Name" and "Comment" if there are >1 Spreadsheets
	ui.leName->setEnabled(false);
	ui.leComment->setEnabled(false);
	
	ui.leName->setText("");
	ui.leComment->setText("");
  }
  
  //show the properties of the first Spreadsheet in the list, if there are >1 spreadsheets
  ui.sbColumnCount->setValue(spreadsheet->columnCount());
  ui.sbRowCount->setValue(spreadsheet->rowCount());
  SpreadsheetView* view= qobject_cast<SpreadsheetView*>(spreadsheet->view());
  ui.cbShowComments->setChecked(view->areCommentsShown());
  
  m_spreadsheets=list;
  m_initializing = false;
}

// ###### SLOTS  ##############

void SpreadsheetDock::nameChanged(){
  if (m_initializing)
	return;
  
  m_spreadsheets.first()->setName(ui.leName->text());
}


void SpreadsheetDock::commentChanged(){
  if (m_initializing)
	return;
  
  m_spreadsheets.first()->setComment(ui.leComment->text());
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

void SpreadsheetDock::loadSettings(){
    	QString filename=QFileDialog::getOpenFileName(this, i18n("Select the file to load settings"),
			"LabPlotrc", i18n("KDE resource files (*rc)"));
    	if (filename=="")
        	return;

	KConfig config(filename, KConfig::SimpleConfig);
	KConfigGroup group = config.group( "Spreadsheet" );

  	ui.sbColumnCount->setValue(group.readEntry("ColumnCount", 2));
  	ui.sbRowCount->setValue(group.readEntry("RowCount", 100));
  	ui.cbShowComments->setChecked(group.readEntry("ShowComments", FALSE));
}

void SpreadsheetDock::saveSettings(){
    	QString filename=QFileDialog::getSaveFileName(this, i18n("Select the file to save settings"),
			"LabPlotrc", i18n("KDE resource files (*rc)"));
    	if (filename=="")
        	return;

	KConfig config(filename, KConfig::SimpleConfig );
	save(config);
	config.sync();
}

void SpreadsheetDock::saveDefaults(){
	KConfig config;
	save(config);
	config.sync();
}

void SpreadsheetDock::save(const KConfig& config){
	KConfigGroup group = config.group( "Spreadsheet" );

	group.writeEntry("ColumnCount", ui.sbColumnCount->value());
	group.writeEntry("RowCount", ui.sbRowCount->value());
	group.writeEntry("ShowComments",ui.cbShowComments->isChecked());
}

