/***************************************************************************
    File                 : ColumnDock.cpp
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2011 by Alexander Semke (alexander.semke*web.de)
    Copyright            : (C) 2013 by Stefan Gerlach (stefan.gerlach*uni.kn)
                                                        (use @ for *)
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

#include "backend/core/AbstractFilter.h"
#include "backend/core/datatypes/SimpleCopyThroughFilter.h"
#include "backend/core/datatypes/Double2StringFilter.h"
#include "backend/core/datatypes/String2DoubleFilter.h"
#include "backend/core/datatypes/DateTime2StringFilter.h"
#include "backend/core/datatypes/String2DateTimeFilter.h"

#include <QDebug>

/*!
  \class SpreadsheetDock
  \brief Provides a widget for editing the properties of the spreadsheet columns currently selected in the project explorer.

  \ingroup kdefrontend
*/

ColumnDock::ColumnDock(QWidget *parent): QWidget(parent){
	ui.setupUi(this);
	
	dateStrings<<"yyyy-MM-dd";
	dateStrings<<"yyyy/MM/dd";
	dateStrings<<"dd/MM/yyyy"; 
	dateStrings<<"dd/MM/yy";
	dateStrings<<"dd.MM.yyyy";
	dateStrings<<"dd.MM.yy";
	dateStrings<<"MM/yyyy";
	dateStrings<<"dd.MM."; 
	dateStrings<<"yyyyMMdd";

	timeStrings<<"hh";
	timeStrings<<"hh ap";
	timeStrings<<"hh:mm";
	timeStrings<<"hh:mm ap";
	timeStrings<<"hh:mm:ss";
	timeStrings<<"hh:mm:ss.zzz";
	timeStrings<<"hh:mm:ss:zzz";
	timeStrings<<"mm:ss.zzz";
	timeStrings<<"hhmmss";
	
	connect(ui.leName, SIGNAL(returnPressed()), this, SLOT(nameChanged()));
	connect(ui.leComment, SIGNAL(returnPressed()), this, SLOT(commentChanged()));
	connect(ui.cbType, SIGNAL(currentIndexChanged(int)), this, SLOT(typeChanged(int)));
	connect(ui.cbFormat, SIGNAL(currentIndexChanged(int)), this, SLOT(formatChanged(int)));
	connect(ui.sbPrecision, SIGNAL(valueChanged(int)), this, SLOT(precisionChanged(int)) );
	connect(ui.cbPlotDesignation, SIGNAL(currentIndexChanged(int)), this, SLOT(plotDesignationChanged(int)));
	
	retranslateUi();
}

void ColumnDock::setColumns(QList<Column*> list){
	m_initializing=true;
	m_columnsList = list;

	m_column=list.first();

	if (list.size()==1){
		ui.lName->setEnabled(true);
		ui.leName->setEnabled(true);
		ui.lComment->setEnabled(true);
		ui.leComment->setEnabled(true);

		ui.leName->setText(m_column->name());
		ui.leComment->setText(m_column->comment());
	}else{
		ui.lName->setEnabled(false);
		ui.leName->setEnabled(false);
		ui.lComment->setEnabled(false);
		ui.leComment->setEnabled(false);

		ui.leName->setText("");
		ui.leComment->setText("");
	}

	//show the properties of the first column
	AbstractColumn::ColumnMode columnMode = m_column->columnMode();
	ui.cbType->setCurrentIndex(ui.cbType->findData((int)columnMode));
	this->updateFormatWidgets(columnMode);

	switch(columnMode) {
		case AbstractColumn::Numeric:{
			Double2StringFilter* filter = static_cast<Double2StringFilter*>(m_column->outputFilter());
			ui.cbFormat->setCurrentIndex(ui.cbFormat->findData(filter->numericFormat()));
			//qDebug()<<"set columns, numeric format"<<filter->numericFormat();
			ui.sbPrecision->setValue(filter->numDigits());
			break;
		}
		case AbstractColumn::Month:
		case AbstractColumn::Day:
		case AbstractColumn::DateTime: {
			DateTime2StringFilter* filter = static_cast<DateTime2StringFilter*>(m_column->outputFilter());
			ui.cbFormat->setCurrentIndex(ui.cbFormat->findData(filter->format()));
			break;
		}
		default:
			break;
	}

	ui.cbPlotDesignation->setCurrentIndex( int(m_column->plotDesignation()) );

	// slots 
	connect(m_column, SIGNAL(aspectDescriptionChanged(const AbstractAspect*)),this, SLOT(columnDescriptionChanged(const AbstractAspect*)));
	connect(m_column->outputFilter(), SIGNAL(formatChanged()),this, SLOT(columnFormatChanged()));
	connect(m_column->outputFilter(), SIGNAL(digitsChanged()),this, SLOT(columnPrecisionChanged()));
	connect(m_column, SIGNAL(plotDesignationChanged(const AbstractColumn*)),this, SLOT(columnPlotDesignationChanged(const AbstractColumn *)));

	m_initializing=false;
}

/*!
  depending on the currently selected column type (column mode) updates the widgets for the column format, 
  shows/hides the allowed widgets, fills the corresponding combobox with the possible entries.
  Called when the type (column mode) is changed.
*/
void ColumnDock::updateFormatWidgets(const AbstractColumn::ColumnMode columnMode){
  ui.cbFormat->clear();

  switch (columnMode){
	case AbstractColumn::Numeric:
	  ui.cbFormat->addItem(i18n("Decimal"), QVariant('f'));
	  ui.cbFormat->addItem(i18n("Scientific (e)"), QVariant('e'));
	  ui.cbFormat->addItem(i18n("Scientific (E)"), QVariant('E'));
	  ui.cbFormat->addItem(i18n("Automatic (g)"), QVariant('g'));
	  ui.cbFormat->addItem(i18n("Automatic (G)"), QVariant('G'));
	  break;
	case AbstractColumn::Text:
	  break;
	case AbstractColumn::Month:
	  ui.cbFormat->addItem(i18n("Number without leading zero"), QVariant("M"));
	  ui.cbFormat->addItem(i18n("Number with leading zero"), QVariant("MM"));
	  ui.cbFormat->addItem(i18n("Abbreviated month name"), QVariant("MMM"));
	  ui.cbFormat->addItem(i18n("Full month name"), QVariant("MMMM"));
	  break;
	case AbstractColumn::Day:
	  ui.cbFormat->addItem(i18n("Number without leading zero"), QVariant("d"));
	  ui.cbFormat->addItem(i18n("Number with leading zero"), QVariant("dd"));
	  ui.cbFormat->addItem(i18n("Abbreviated day name"), QVariant("ddd"));
	  ui.cbFormat->addItem(i18n("Full day name"), QVariant("dddd"));
	  break;
	case AbstractColumn::DateTime:{
	  foreach(QString s, dateStrings)
		ui.cbFormat->addItem(s, QVariant(s));
	  
	  foreach(QString s, timeStrings)
		ui.cbFormat->addItem(s, QVariant(s));
	  
	  foreach(QString s1, dateStrings){
		foreach(QString s2, timeStrings)
		  ui.cbFormat->addItem(s1 + " " + s2, QVariant(s1 + " " + s2));
	  }
	  
	  break;
	}
	default:
		break;
  }
  
  if (columnMode == AbstractColumn::Numeric){
	ui.lPrecision->show();
	ui.sbPrecision->show();
  }else{
	ui.lPrecision->hide();
	ui.sbPrecision->hide();
  }
  
  if (columnMode == AbstractColumn::Text){
	ui.lFormat->hide();
	ui.cbFormat->hide();
  }else{
	ui.lFormat->show();
	ui.cbFormat->show();
	ui.cbFormat->setCurrentIndex(0);
  }
  
  if (columnMode == AbstractColumn::DateTime){
	ui.cbFormat->setEditable( true );
  }else{
	ui.cbFormat->setEditable( false );
  }
}

// SLOTS 
void ColumnDock::retranslateUi(){
	m_initializing = true;
	
  	ui.cbType->clear();
	ui.cbType->addItem(i18n("Numeric"), QVariant(int(AbstractColumn::Numeric)));
	ui.cbType->addItem(i18n("Text"), QVariant(int(AbstractColumn::Text)));
	ui.cbType->addItem(i18n("Month names"), QVariant(int(AbstractColumn::Month)));
	ui.cbType->addItem(i18n("Day names"), QVariant(int(AbstractColumn::Day)));
	ui.cbType->addItem(i18n("Date and time"), QVariant(int(AbstractColumn::DateTime)));
	
	ui.cbPlotDesignation->clear();
	ui.cbPlotDesignation->addItem(i18n("none"));
	ui.cbPlotDesignation->addItem(i18n("X"));
	ui.cbPlotDesignation->addItem(i18n("Y"));
	ui.cbPlotDesignation->addItem(i18n("Z"));
	ui.cbPlotDesignation->addItem(i18n("X-error"));
	ui.cbPlotDesignation->addItem(i18n("Y-error"));
	
	m_initializing = false;
}


void ColumnDock::nameChanged(){
  if (m_initializing)
	return;
  
  m_columnsList.first()->setName(ui.leName->text());
}


void ColumnDock::commentChanged(){
  if (m_initializing)
	return;
  
  m_columnsList.first()->setComment(ui.leComment->text());
}

/*!
  called when the type (column mode - numeric, text etc.) of the column was changed.
*/ 
void ColumnDock::typeChanged(int index){
   if (m_initializing)
	return;


  AbstractColumn::ColumnMode columnMode = (AbstractColumn::ColumnMode)ui.cbType->itemData( index ).toInt();
  
  m_initializing = true;
  this->updateFormatWidgets(columnMode);
  m_initializing = false;

  int format_index = ui.cbFormat->currentIndex();

  switch(columnMode) {
	  case AbstractColumn::Numeric:{
		int digits = ui.sbPrecision->value();
		Double2StringFilter * filter;
		foreach(Column* col, m_columnsList) {
		  col->beginMacro(i18n("%1: change column type").arg(col->name()));
		  col->setColumnMode(columnMode);
		  filter = static_cast<Double2StringFilter*>(col->outputFilter());
		  filter->setNumericFormat(ui.cbFormat->itemData(format_index).toChar().toLatin1());
		  filter->setNumDigits(digits);
		  col->endMacro();
		}
		break;
	  }
	  case AbstractColumn::Text:
		  foreach(Column* col, m_columnsList){
			  col->setColumnMode(columnMode);
		  }
		  break;
	  case AbstractColumn::Month:
	  case AbstractColumn::Day:
	  case AbstractColumn::DateTime:{
		QString format;
		DateTime2StringFilter * filter;
		foreach(Column* col, m_columnsList) {
		  col->beginMacro(i18n("%1: change column type").arg(col->name()));
		  format = ui.cbFormat->currentText();
		  col->setColumnMode(columnMode);
		  filter = static_cast<DateTime2StringFilter*>(col->outputFilter());
		  filter->setFormat(format);
		  col->endMacro();
		}
		break;
	  }
  }
}

/*!
  called when the format for the current type (column mode) was changed.
*/
void ColumnDock::formatChanged(int index){
  if (m_initializing)
	return;

  AbstractColumn::ColumnMode mode = (AbstractColumn::ColumnMode)ui.cbType->itemData( ui.cbType->currentIndex() ).toInt();
  int format_index = index;

  switch(mode) {
	  case AbstractColumn::Numeric:{
		Double2StringFilter * filter;
		foreach(Column* col, m_columnsList) {
		  filter = static_cast<Double2StringFilter*>(col->outputFilter());
		  filter->setNumericFormat(ui.cbFormat->itemData(format_index).toChar().toLatin1());
		}
		break;
	  }
	  case AbstractColumn::Text:
		  break;
	  case AbstractColumn::Month:
	  case AbstractColumn::Day:
	  case AbstractColumn::DateTime:{
		DateTime2StringFilter * filter;
		QString format = ui.cbFormat->itemData( ui.cbFormat->currentIndex() ).toString();
		foreach(Column* col, m_columnsList){
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
  
  AbstractColumn::PlotDesignation pd=AbstractColumn::PlotDesignation(index);
  foreach(Column* col, m_columnsList){
	col->setPlotDesignation(pd);
  }  
}


//*************************************************************
//******** SLOTs for changes triggered in Column ***********
//*************************************************************
void ColumnDock::columnDescriptionChanged(const AbstractAspect* aspect) {
        if (m_column != aspect)
                return;

        m_initializing = true;
        if (aspect->name() != ui.leName->text()) {
                ui.leName->setText(aspect->name());
        } else if (aspect->comment() != ui.leComment->text()) {
                ui.leComment->setText(aspect->comment());
        }
        m_initializing = false;
}

void ColumnDock::columnFormatChanged() {
        m_initializing = true;
	AbstractColumn::ColumnMode columnMode = m_column->columnMode();
	switch(columnMode) {
                case AbstractColumn::Numeric:{
                        Double2StringFilter* filter = static_cast<Double2StringFilter*>(m_column->outputFilter());
                        ui.cbFormat->setCurrentIndex(ui.cbFormat->findData(filter->numericFormat()));
                        break;
                }
                case AbstractColumn::Month:
                case AbstractColumn::Day:
                case AbstractColumn::DateTime: {
                        DateTime2StringFilter* filter = static_cast<DateTime2StringFilter*>(m_column->outputFilter());
                        ui.cbFormat->setCurrentIndex(ui.cbFormat->findData(filter->format()));
                        break;
                }
                default:
                        break;
        }
        m_initializing = false;
}

void ColumnDock::columnPrecisionChanged() {
        m_initializing = true;
	Double2StringFilter * filter = static_cast<Double2StringFilter*>(m_column->outputFilter());
	ui.sbPrecision->setValue(filter->numDigits());
        m_initializing = false;
}

void ColumnDock::columnPlotDesignationChanged(const AbstractColumn* col) {
	m_initializing = true;
	ui.cbPlotDesignation->setCurrentIndex( int(col->plotDesignation()) );
	m_initializing = false;
}
