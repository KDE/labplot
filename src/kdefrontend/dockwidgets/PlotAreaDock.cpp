/***************************************************************************
    File                 : PlotAreaDock.cpp
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2011 by Alexander Semke
    Email (use @ for *)  : alexander.semke*web.de
    Description          : widget for plot area properties
                           
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

#include "PlotAreaDock.h"
#include "worksheet/PlotArea.h"
#include "kdefrontend/GuiTools.h"
#include <QTimer>
#include <KUrlCompletion>

/*!
  \class GuiObserver
  \brief  Provides a widget for editing the properties of the plot areas currently selected in the project explorer.

  \ingroup kdefrontend
*/

PlotAreaDock::PlotAreaDock(QWidget *parent): QWidget(parent){
	ui.setupUi(this);

	ui.kleBackgroundFileName->setClearButtonShown(true);

	ui.bOpen->setIcon( KIcon("document-open") );

	KUrlCompletion *comp = new KUrlCompletion();
    ui.kleBackgroundFileName->setCompletionObject(comp);

	//General
	connect( ui.leName, SIGNAL(returnPressed()), this, SLOT(nameChanged()) );
	connect( ui.leComment, SIGNAL(returnPressed()), this, SLOT(commentChanged()) );	
	connect( ui.sbOpacity, SIGNAL(valueChanged(int)), this, SLOT(opacityChanged(int)) );
	
	//Background
	connect( ui.cbBackgroundType, SIGNAL(currentIndexChanged(int)), this, SLOT(backgroundTypeChanged(int)) );
	connect( ui.cbBackgroundColorStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(backgroundColorStyleChanged(int)) );
	connect( ui.cbBackgroundImageStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(backgroundImageStyleChanged(int)) );
	connect(ui.bOpen, SIGNAL(clicked(bool)), this, SLOT(selectFile()));
	connect( ui.kleBackgroundFileName, SIGNAL(returnPressed()), this, SLOT(fileNameChanged()) );
	connect( ui.kleBackgroundFileName, SIGNAL(clearButtonClicked()), this, SLOT(fileNameChanged()) );
// 	connect( ui.kleBackgroundFileName, SIGNAL(textChanged (const QString&)), SLOT(fileNameChanged(const QString&)) );
	connect( ui.kcbBackgroundFirstColor, SIGNAL(changed (const QColor &)), this, SLOT(backgroundFirstColorChanged(const QColor&)) );
	connect( ui.kcbBackgroundSecondColor, SIGNAL(changed (const QColor &)), this, SLOT(backgroundSecondColorChanged(const QColor&)) );
	
	//Border 
	connect( ui.cbBorderStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(borderStyleChanged(int)) );
	connect( ui.kcbBorderColor, SIGNAL(changed (const QColor &)), this, SLOT(borderColorChanged(const QColor&)) );
	connect( ui.sbBorderWidth, SIGNAL(valueChanged(int)), this, SLOT(borderWidthChanged(int)) );

	QTimer::singleShot(0, this, SLOT(init()));
}

void PlotAreaDock::init(){
	this->retranslateUi();
}

void PlotAreaDock::setPlotAreas(QList<PlotArea*> list){
	m_initializing = true;
	m_plotAreaList = list;

	PlotArea* area=list.first();
  
  //if there are more then one curve in the list, disable the tab "general"
  if (list.size()==1){
	ui.lName->setEnabled(true);
	ui.leName->setEnabled(true);
	ui.lComment->setEnabled(true);
	ui.leComment->setEnabled(true);

	ui.leName->setText(area->name());
	ui.leComment->setText(area->comment());
  }else{
	ui.lName->setEnabled(false);
	ui.leName->setEnabled(false);
	ui.lComment->setEnabled(false);
	ui.leComment->setEnabled(false);

	ui.leName->setText("");
	ui.leComment->setText("");
  }
  
	//show the properties of the first curve
  
	//General-tab
	ui.sbOpacity->setValue( area->opacity()*100 );

	//Background-tab
	ui.cbBackgroundType->setCurrentIndex( area->backgroundType() );
	ui.cbBackgroundColorStyle->setCurrentIndex( area->backgroundColorStyle() );
	ui.cbBackgroundImageStyle->setCurrentIndex( area->backgroundImageStyle() );
	ui.kleBackgroundFileName->setText( area->backgroundFileName() );
	ui.kcbBackgroundFirstColor->setColor( area->backgroundFirstColor() );
	ui.kcbBackgroundSecondColor->setColor( area->backgroundSecondColor() );

	//Border-tab
	ui.kcbBorderColor->setColor( area->borderPen().color() );
	GuiTools::updatePenStyles(ui.cbBorderStyle, area->borderPen().color());
	ui.cbBorderStyle->setCurrentIndex( area->borderPen().style() );
	ui.sbBorderWidth->setValue( area->borderPen().width() );

	m_initializing = false;
}

//************************************************************
//****************** SLOTS ********************************
//************************************************************
void PlotAreaDock::retranslateUi(){
	m_initializing = true;
	
	ui.cbBackgroundType->addItem(i18n("color"));
	ui.cbBackgroundType->addItem(i18n("image"));

	ui.cbBackgroundColorStyle->addItem(i18n("single color"));
	ui.cbBackgroundColorStyle->addItem(i18n("horizontal linear gradient"));
	ui.cbBackgroundColorStyle->addItem(i18n("vertical linear gradient"));
	ui.cbBackgroundColorStyle->addItem(i18n("diagonal linear gradient (start from top left)"));
	ui.cbBackgroundColorStyle->addItem(i18n("diagonal linear gradient (start from bottom left)"));
	ui.cbBackgroundColorStyle->addItem(i18n("radial gradient"));
// 		//TODO ui.cbBackgroundColorStyle->addItem(i18n("custom gradient"));

	ui.cbBackgroundImageStyle->addItem(i18n("scaled and cropped"));
	ui.cbBackgroundImageStyle->addItem(i18n("scaled"));
	ui.cbBackgroundImageStyle->addItem(i18n("scaled, keep proportions"));
	ui.cbBackgroundImageStyle->addItem(i18n("centered"));
	ui.cbBackgroundImageStyle->addItem(i18n("tiled"));
	ui.cbBackgroundImageStyle->addItem(i18n("center tiled"));

	GuiTools::updatePenStyles(ui.cbBorderStyle, Qt::black);
	m_initializing = false;
}


// "General"-tab
void PlotAreaDock::nameChanged(){
  if (m_initializing)
	return;
  
  m_plotAreaList.first()->setName(ui.leName->text());
}

void PlotAreaDock::commentChanged(){
  if (m_initializing)
	return;
  
  m_plotAreaList.first()->setComment(ui.leComment->text());
}

void PlotAreaDock::opacityChanged(int value){
  if (m_initializing)
	return;

  qreal opacity = (float)value/100;
  foreach(PlotArea* area, m_plotAreaList){
	area->setOpacity(opacity);
  }
}

// "Background"-tab
void PlotAreaDock::backgroundTypeChanged(int index){
	PlotArea::BackgroundType type = (PlotArea::BackgroundType)index;

	if (type == PlotArea::Color){
		ui.lBackgroundColorStyle->show();
		ui.cbBackgroundColorStyle->show();
		ui.lBackgroundImageStyle->hide();
		ui.cbBackgroundImageStyle->hide();
		
		ui.lBackgroundFileName->hide();
		ui.kleBackgroundFileName->hide();
		ui.bOpen->hide();

		ui.lBackgroundFirstColor->show();
		ui.kcbBackgroundFirstColor->show();

		PlotArea::BackgroundColorStyle style = (PlotArea::BackgroundColorStyle)index;
		if (style == PlotArea::SingleColor){
			ui.lBackgroundSecondColor->hide();
			ui.kcbBackgroundSecondColor->hide();
		}else{
			ui.lBackgroundSecondColor->show();
			ui.kcbBackgroundSecondColor->show();
		}
	}else if(type == PlotArea::Image){
		ui.lBackgroundColorStyle->hide();
		ui.cbBackgroundColorStyle->hide();
		ui.lBackgroundImageStyle->show();
		ui.cbBackgroundImageStyle->show();
		ui.lBackgroundFileName->show();
		ui.kleBackgroundFileName->show();
		ui.bOpen->show();

		ui.lBackgroundFirstColor->hide();
		ui.kcbBackgroundFirstColor->hide();
		ui.lBackgroundSecondColor->hide();
		ui.kcbBackgroundSecondColor->hide();
	}

	if (m_initializing)
		return;
   
	foreach(PlotArea* area, m_plotAreaList){
		area->setBackgroundType(type);
  }  
}

void PlotAreaDock::backgroundColorStyleChanged(int index){
	PlotArea::BackgroundColorStyle style = (PlotArea::BackgroundColorStyle)index;

	if (style == PlotArea::SingleColor){
		ui.lBackgroundSecondColor->hide();
		ui.kcbBackgroundSecondColor->hide();
	}else{
		ui.lBackgroundSecondColor->show();
		ui.kcbBackgroundSecondColor->show();
	}

	if (m_initializing)
		return;
   
	foreach(PlotArea* area, m_plotAreaList){
		area->setBackgroundColorStyle(style);
  }  
}

void PlotAreaDock::backgroundImageStyleChanged(int index){
	if (m_initializing)
		return;

	PlotArea::BackgroundImageStyle style = (PlotArea::BackgroundImageStyle)index;
	foreach(PlotArea* area, m_plotAreaList){
		area->setBackgroundImageStyle(style);
  }  
}

void PlotAreaDock::backgroundFirstColorChanged(const QColor& c){
  if (m_initializing)
	return;

  foreach(PlotArea* area, m_plotAreaList){
	area->setBackgroundFirstColor(c);
  }  
}


void PlotAreaDock::backgroundSecondColorChanged(const QColor& c){
  if (m_initializing)
	return;

  foreach(PlotArea* area, m_plotAreaList){
	area->setBackgroundSecondColor(c);
  }  
}

/*!
	opens a file dialog and lets the user select the image file.
*/
void PlotAreaDock::selectFile() {
    QString path=QFileDialog::getOpenFileName(this, i18n("Select the image file"));
    if (path=="")
        return;

    ui.kleBackgroundFileName->setText( path );

	foreach(PlotArea* area, m_plotAreaList){
		area->setBackgroundFileName(path);
  }  
}

void PlotAreaDock::fileNameChanged(){
	if (m_initializing)
		return;
	
	QString fileName = ui.kleBackgroundFileName->text();
	foreach(PlotArea* area, m_plotAreaList){
		area->setBackgroundFileName(fileName);
  } 
}

// "Border"-tab
void PlotAreaDock::borderStyleChanged(int index){
   if (m_initializing)
	return;

  Qt::PenStyle penStyle=Qt::PenStyle(index);
  QPen pen;
  foreach(PlotArea* area, m_plotAreaList){
	pen=area->borderPen();
	pen.setStyle(penStyle);
	area->setBorderPen(pen);
  }
}

void PlotAreaDock::borderColorChanged(const QColor& color){
  if (m_initializing)
	return;

  QPen pen;
  foreach(PlotArea* area, m_plotAreaList){
	pen=area->borderPen();
	pen.setColor(color);
	area->setBorderPen(pen);
  }  

  GuiTools::updatePenStyles(ui.cbBorderStyle, color);
}

void PlotAreaDock::borderWidthChanged(int value){
  if (m_initializing)
	return;
  
  QPen pen;
  foreach(PlotArea* area, m_plotAreaList){
	pen=area->borderPen();
	pen.setWidth(value);
	area->setBorderPen(pen);
  }  
}
