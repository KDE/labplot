/***************************************************************************
    File                 : ColumnDock.cpp
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2011 Alexander Semke
    Email (use @ for *)  : alexander.semke*web.de
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

#include "core/AbstractFilter.h"
#include "core/datatypes/SimpleCopyThroughFilter.h"
#include "core/datatypes/Double2StringFilter.h"
#include "core/datatypes/String2DoubleFilter.h"
#include "core/datatypes/DateTime2StringFilter.h"
#include "core/datatypes/String2DateTimeFilter.h"

#include <QDebug>

/*!
  \class SpreadsheetDock
  \brief Provides a widget for editing the properties of the spreadsheet columns currently selected in the project explorer.

  \ingroup kdefrontend
*/

ColumnDock::ColumnDock(QWidget *parent): QWidget(parent){
	ui.setupUi(this);
	
	connect(ui.cbType, SIGNAL(currentIndexChanged(int)), this, SLOT(typeChanged(int)));
	connect(ui.cbFormat, SIGNAL(currentIndexChanged(int)), this, SLOT(formatChanged(int)));
	connect( ui.sbPrecision, SIGNAL(valueChanged(int)), this, SLOT(precisionChanged(int)) );
	connect(ui.cbPlotDesignation, SIGNAL(currentIndexChanged(int)), this, SLOT(plotDesignationChanged(int)));
	
	retranslateUi();
}

void ColumnDock::setColumns(QList<Column*> list){
  m_initializing=true;
  m_columnsList = list;
  
  Column* column=list.first();
  
  if (list.size()==1){
	ui.lName->setEnabled(true);
	ui.leName->setEnabled(true);
	ui.lComment->setEnabled(true);
	ui.teComment->setEnabled(true);
	
	ui.leName->setText(column->name());
	ui.teComment->setText(column->comment());
  }else{
	ui.lName->setEnabled(false);
	ui.leName->setEnabled(false);
	ui.lComment->setEnabled(false);
	ui.teComment->setEnabled(false);
	
	ui.leName->setText("");
	ui.teComment->setText("");
  }
  
  //show the properties of the first column
  ui.cbType->setCurrentIndex(ui.cbType->findData((int)column->columnMode()));
  this->updateFormat();

  switch(column->columnMode()) {
	  case SciDAVis::Numeric:{
			  Double2StringFilter * filter = static_cast<Double2StringFilter*>(column->outputFilter());
			  ui.cbFormat->setCurrentIndex(ui.cbFormat->findData(filter->numericFormat()));
			  ui.sbPrecision->setValue(filter->numDigits());
			  break;
		  }
	  case SciDAVis::Month:
	  case SciDAVis::Day:
	  case SciDAVis::DateTime: {
			  DateTime2StringFilter * filter = static_cast<DateTime2StringFilter*>(column->outputFilter());
			  ui.cbFormat->setCurrentIndex(ui.cbFormat->findData(filter->format()));
			  break;
		  }
	  default:
		  break;
  }
  
  ui.cbPlotDesignation->setCurrentIndex( int(column->plotDesignation()) );
  
  m_initializing=false;
}

void ColumnDock::updateFormat(){
  ui.cbFormat->clear();
  SciDAVis::ColumnMode columnMode = SciDAVis::ColumnMode( ui.cbType->itemData(ui.cbType->currentIndex()).toInt() );

  switch (columnMode){
	case SciDAVis::Numeric:
	  ui.cbFormat->addItem(tr("Decimal"), QVariant('f'));
	  ui.cbFormat->addItem(tr("Scientific (e)"), QVariant('e'));
	  ui.cbFormat->addItem(tr("Scientific (E)"), QVariant('E'));
	  ui.cbFormat->addItem(tr("Automatic (e)"), QVariant('g'));
	  ui.cbFormat->addItem(tr("Automatic (E)"), QVariant('G'));
	  break;
	case SciDAVis::Text:
	  ui.cbFormat->addItem(tr("Text"), QVariant());
	  break;
	case SciDAVis::Month:
	  ui.cbFormat->addItem(tr("Number without leading zero"), QVariant("M"));
	  ui.cbFormat->addItem(tr("Number with leading zero"), QVariant("MM"));
	  ui.cbFormat->addItem(tr("Abbreviated month name"), QVariant("MMM"));
	  ui.cbFormat->addItem(tr("Full month name"), QVariant("MMMM"));
	  break;
	case SciDAVis::Day:
	  ui.cbFormat->addItem(tr("Number without leading zero"), QVariant("d"));
	  ui.cbFormat->addItem(tr("Number with leading zero"), QVariant("dd"));
	  ui.cbFormat->addItem(tr("Abbreviated day name"), QVariant("ddd"));
	  ui.cbFormat->addItem(tr("Full day name"), QVariant("dddd"));
	  break;
	case SciDAVis::DateTime:{
	  const char * date_strings[] = {
		  "yyyy-MM-dd", 	
		  "yyyy/MM/dd", 
		  "dd/MM/yyyy", 
		  "dd/MM/yy", 
		  "dd.MM.yyyy", 	
		  "dd.MM.yy",
		  "MM/yyyy",
		  "dd.MM.", 
		  "yyyyMMdd",
		  0
	  };

	  const char * time_strings[] = {
		  "hh",
		  "hh ap",
		  "hh:mm",
		  "hh:mm ap",
		  "hh:mm:ss",
		  "hh:mm:ss.zzz",
		  "hh:mm:ss:zzz",
		  "mm:ss.zzz",
		  "hhmmss",
		  0
	  };
	  
	  int j,i;
	  for (i=0; date_strings[i] != 0; i++)
		  ui.cbFormat->addItem(QString(date_strings[i]), QVariant(date_strings[i]));
	  for (j=0; time_strings[j] != 0; j++)
		  ui.cbFormat->addItem(QString(time_strings[j]), QVariant(time_strings[j]));
	  for (i=0; date_strings[i] != 0; i++)
		  for (j=0; time_strings[j] != 0; j++)
			  ui.cbFormat->addItem(QString("%1 %2").arg(date_strings[i]).arg(time_strings[j]), 
				  QVariant(QString(date_strings[i]) + " " + QString(time_strings[j])));	
	  break;
	}
	default:
		break;
  }
	

  if (columnMode == SciDAVis::Numeric){
	ui.lPrecision->show();
	ui.sbPrecision->show();
  }else{
	ui.lPrecision->hide();
	ui.sbPrecision->hide();
  }
  
  if (columnMode == SciDAVis::Text){
	ui.lFormat->hide();
	ui.cbFormat->hide();
  }else{
	ui.lFormat->show();
	ui.cbFormat->show();
  }
  
  if (columnMode == SciDAVis::DateTime){
	ui.cbFormat->setEditable( true );
  }else{
	ui.cbFormat->setEditable( false );
  }
}

// SLOTS 
void ColumnDock::retranslateUi(){
  	ui.cbType->clear();
	ui.cbType->addItem(i18n("Numeric"), QVariant(int(SciDAVis::Numeric)));
	ui.cbType->addItem(i18n("Text"), QVariant(int(SciDAVis::Text)));
	ui.cbType->addItem(i18n("Month names"), QVariant(int(SciDAVis::Month)));
	ui.cbType->addItem(i18n("Day names"), QVariant(int(SciDAVis::Day)));
	ui.cbType->addItem(i18n("Date and time"), QVariant(int(SciDAVis::DateTime)));
	
	ui.cbPlotDesignation->clear();
	ui.cbPlotDesignation->addItem(i18n("none"));
	ui.cbPlotDesignation->addItem(i18n("X"));
	ui.cbPlotDesignation->addItem(i18n("Y"));
	ui.cbPlotDesignation->addItem(i18n("Z"));
	ui.cbPlotDesignation->addItem(i18n("X-error"));
	ui.cbPlotDesignation->addItem(i18n("Y-error"));
}


void ColumnDock::nameChanged(){
  if (m_initializing)
	return;
  
  m_columnsList.first()->setName(ui.leName->text());
}


void ColumnDock::commentChanged(){
  if (m_initializing)
	return;
  
  m_columnsList.first()->setComment(ui.teComment->toPlainText());
}

void ColumnDock::typeChanged(int index){
   if (m_initializing)
	return;

   this->updateFormat();

  SciDAVis::ColumnMode mode = (SciDAVis::ColumnMode)ui.cbType->itemData( index ).toInt();
  int format_index = ui.cbFormat->currentIndex();

  switch(mode) {
	  case SciDAVis::Numeric:{
		int digits = ui.sbPrecision->value();
		Double2StringFilter * filter;
		foreach(Column* col, m_columnsList) {
		  col->beginMacro(QObject::tr("%1: change column type").arg(col->name()));
		  col->setColumnMode(mode);
		  filter = static_cast<Double2StringFilter*>(col->outputFilter());
		  filter->setNumericFormat(ui.cbFormat->itemData(format_index).toChar().toLatin1());
		  filter->setNumDigits(digits);
		  col->endMacro();
		}
		break;
	  }
	  case SciDAVis::Text:
		  foreach(Column* col, m_columnsList){
			  col->setColumnMode(mode);
		  }
		  break;
	  case SciDAVis::Month:
	  case SciDAVis::Day:
	  case SciDAVis::DateTime:{
		QString format;
		DateTime2StringFilter * filter;
		foreach(Column* col, m_columnsList) {
		  col->beginMacro(QObject::tr("%1: change column type").arg(col->name()));
		  format = ui.cbFormat->currentText();
		  col->setColumnMode(mode);
		  filter = static_cast<DateTime2StringFilter*>(col->outputFilter());
		  filter->setFormat(format);
		  col->endMacro();
		}
		break;
	  }
  }
}


void ColumnDock::formatChanged(int index){
  if (m_initializing)
	return;

  SciDAVis::ColumnMode mode = (SciDAVis::ColumnMode)ui.cbType->itemData( ui.cbType->currentIndex() ).toInt();
  int format_index = index;

  switch(mode) {
	  case SciDAVis::Numeric:{
		Double2StringFilter * filter;
		foreach(Column* col, m_columnsList) {
		  filter = static_cast<Double2StringFilter*>(col->outputFilter());
		  filter->setNumericFormat(ui.cbFormat->itemData(format_index).toChar().toLatin1());
		}
		break;
	  }
	  case SciDAVis::Text:
		  break;
	  case SciDAVis::Month:
	  case SciDAVis::Day:
	  case SciDAVis::DateTime:{
		DateTime2StringFilter * filter;
		QString format;
		foreach(Column* col, m_columnsList) {
		  format = ui.cbFormat->currentText();
		  filter = static_cast<DateTime2StringFilter*>(col->outputFilter());
		  filter->setFormat(format);
		}
		break;
	  }
  }
}


void ColumnDock::precisionChanged(int digits){
  if (m_initializing)
	return;
  
  foreach(Column* col, m_columnsList){
	  Double2StringFilter * filter = static_cast<Double2StringFilter*>(col->outputFilter());
	  filter->setNumDigits(digits);
  }  
}


void ColumnDock::plotDesignationChanged(int index){
  if (m_initializing)
	return;
  
  SciDAVis::PlotDesignation pd=SciDAVis::PlotDesignation(index);
  foreach(Column* col, m_columnsList){
	col->setPlotDesignation(pd);
  }  
}