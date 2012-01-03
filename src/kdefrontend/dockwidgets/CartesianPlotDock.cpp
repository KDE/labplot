/***************************************************************************
    File                 : CartesianPlotDock.cpp
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2011 by Alexander Semke
    Email (use @ for *)  : alexander.semke*web.de
    Description          : widget for cartesian plot properties
                           
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

#include "CartesianPlotDock.h"
#include "worksheet/plots/cartesian/CartesianPlot.h"
#include "worksheet/plots/PlotArea.h"
#include "worksheet/Worksheet.h"
#include "kdefrontend/GuiTools.h"
#include <QTimer>
#include <KUrlCompletion>

/*!
  \class GuiObserver
  \brief  Provides a widget for editing the properties of the plot areas currently selected in the project explorer.

  \ingroup kdefrontend
*/

CartesianPlotDock::CartesianPlotDock(QWidget *parent): QWidget(parent){
	ui.setupUi(this);
	ui.kleBackgroundFileName->setClearButtonShown(true);

	ui.bOpen->setIcon( KIcon("document-open") );

	KUrlCompletion *comp = new KUrlCompletion();
    ui.kleBackgroundFileName->setCompletionObject(comp);

	//adjust layouts in the tabs
	QGridLayout* layout;
	for (int i=0; i<ui.tabWidget->count(); ++i){
		layout=static_cast<QGridLayout*>(ui.tabWidget->widget(i)->layout());
		if (!layout)
		continue;
		
		layout->setContentsMargins(2,2,2,2);
		layout->setHorizontalSpacing(2);
		layout->setVerticalSpacing(2);
  }
  
	//SIGNAL/SLOT

	//General
	connect( ui.leName, SIGNAL(returnPressed()), this, SLOT(nameChanged()) );
	connect( ui.leComment, SIGNAL(returnPressed()), this, SLOT(commentChanged()) );
	connect( ui.chkVisible, SIGNAL(stateChanged(int)), this, SLOT(visibilityChanged(int)) );
	
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
	connect( ui.sbBackgroundOpacity, SIGNAL(valueChanged(int)), this, SLOT(backgroundOpacityChanged(int)) );
	
	//Border 
	connect( ui.cbBorderStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(borderStyleChanged(int)) );
	connect( ui.kcbBorderColor, SIGNAL(changed (const QColor &)), this, SLOT(borderColorChanged(const QColor&)) );
	connect( ui.sbBorderWidth, SIGNAL(valueChanged(double)), this, SLOT(borderWidthChanged(double)) );
	connect( ui.sbBorderOpacity, SIGNAL(valueChanged(int)), this, SLOT(borderOpacityChanged(int)) );

	QTimer::singleShot(0, this, SLOT(init()));
}

void CartesianPlotDock::init(){
	this->retranslateUi();
}

void CartesianPlotDock::setPlots(QList<CartesianPlot*> list){
	m_initializing = true;
	m_plotList = list;

	CartesianPlot* plot=list.first();
  
  //if there are more then one curve in the list, disable the tab "general"
  if (list.size()==1){
	ui.lName->setEnabled(true);
	ui.leName->setEnabled(true);
	ui.lComment->setEnabled(true);
	ui.leComment->setEnabled(true);

	ui.leName->setText(plot->name());
	ui.leComment->setText(plot->comment());
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
	ui.chkVisible->setChecked(plot->isVisible());

	//Background-tab
	ui.cbBackgroundType->setCurrentIndex( plot->plotArea()->backgroundType() );
	ui.cbBackgroundColorStyle->setCurrentIndex( plot->plotArea()->backgroundColorStyle() );
	ui.cbBackgroundImageStyle->setCurrentIndex( plot->plotArea()->backgroundImageStyle() );
	ui.kleBackgroundFileName->setText( plot->plotArea()->backgroundFileName() );
	ui.kcbBackgroundFirstColor->setColor( plot->plotArea()->backgroundFirstColor() );
	ui.kcbBackgroundSecondColor->setColor( plot->plotArea()->backgroundSecondColor() );
	ui.sbBackgroundOpacity->setValue( plot->plotArea()->backgroundOpacity()*100 );

	//Border-tab
	ui.kcbBorderColor->setColor( plot->plotArea()->borderPen().color() );
	GuiTools::updatePenStyles(ui.cbBorderStyle, plot->plotArea()->borderPen().color());
	ui.cbBorderStyle->setCurrentIndex( plot->plotArea()->borderPen().style() );
	ui.sbBorderWidth->setValue( Worksheet::convertFromMillimeter(plot->plotArea()->borderPen().widthF(), Worksheet::Point) );
	ui.sbBorderOpacity->setValue( plot->plotArea()->borderOpacity()*100 );
	
	m_initializing = false;
}

//************************************************************
//****************** SLOTS ********************************
//************************************************************
void CartesianPlotDock::retranslateUi(){
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
void CartesianPlotDock::nameChanged(){
  if (m_initializing)
	return;
  
  m_plotList.first()->setName(ui.leName->text());
}

void CartesianPlotDock::commentChanged(){
  if (m_initializing)
	return;
  
  m_plotList.first()->setComment(ui.leComment->text());
}

void CartesianPlotDock::visibilityChanged(int state){
  if (m_initializing)
	return;
  
  bool b = (state==Qt::Checked);
  foreach(CartesianPlot* plot, m_plotList){
	plot->setVisible(b);
  }
}

// "Background"-tab
void CartesianPlotDock::backgroundTypeChanged(int index){
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
   
	foreach(CartesianPlot* plot, m_plotList){
		plot->plotArea()->setBackgroundType(type);
  }  
}

void CartesianPlotDock::backgroundColorStyleChanged(int index){
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
   
	foreach(CartesianPlot* plot, m_plotList){
		plot->plotArea()->setBackgroundColorStyle(style);
  }  
}

void CartesianPlotDock::backgroundImageStyleChanged(int index){
	if (m_initializing)
		return;

	PlotArea::BackgroundImageStyle style = (PlotArea::BackgroundImageStyle)index;
	foreach(CartesianPlot* plot, m_plotList){
		plot->plotArea()->setBackgroundImageStyle(style);
  }  
}

void CartesianPlotDock::backgroundFirstColorChanged(const QColor& c){
  if (m_initializing)
	return;

  foreach(CartesianPlot* plot, m_plotList){
	plot->plotArea()->setBackgroundFirstColor(c);
  }  
}


void CartesianPlotDock::backgroundSecondColorChanged(const QColor& c){
  if (m_initializing)
	return;

  foreach(CartesianPlot* plot, m_plotList){
	plot->plotArea()->setBackgroundSecondColor(c);
  }  
}

/*!
	opens a file dialog and lets the user select the image file.
*/
void CartesianPlotDock::selectFile() {
    QString path=QFileDialog::getOpenFileName(this, i18n("Select the image file"));
    if (path=="")
        return;

    ui.kleBackgroundFileName->setText( path );

	foreach(CartesianPlot* plot, m_plotList){
		plot->plotArea()->setBackgroundFileName(path);
  }  
}

void CartesianPlotDock::fileNameChanged(){
	if (m_initializing)
		return;
	
	QString fileName = ui.kleBackgroundFileName->text();
	foreach(CartesianPlot* plot, m_plotList){
		plot->plotArea()->setBackgroundFileName(fileName);
  } 
}

void CartesianPlotDock::backgroundOpacityChanged(int value){
  if (m_initializing)
	return;

  qreal opacity = (float)value/100;
  foreach(CartesianPlot* plot, m_plotList){
	plot->plotArea()->setBackgroundOpacity(opacity);
  }
}

// "Border"-tab
void CartesianPlotDock::borderStyleChanged(int index){
   if (m_initializing)
	return;

  Qt::PenStyle penStyle=Qt::PenStyle(index);
  QPen pen;
  foreach(CartesianPlot* plot, m_plotList){
	pen=plot->plotArea()->borderPen();
	pen.setStyle(penStyle);
	plot->plotArea()->setBorderPen(pen);
  }
}

void CartesianPlotDock::borderColorChanged(const QColor& color){
  if (m_initializing)
	return;

  QPen pen;
  foreach(CartesianPlot* plot, m_plotList){
	pen=plot->plotArea()->borderPen();
	pen.setColor(color);
	plot->plotArea()->setBorderPen(pen);
  }  

  GuiTools::updatePenStyles(ui.cbBorderStyle, color);
}

void CartesianPlotDock::borderWidthChanged(double value){
  if (m_initializing)
	return;
  
  QPen pen;
  foreach(CartesianPlot* plot, m_plotList){
	pen=plot->plotArea()->borderPen();
	pen.setWidthF( Worksheet::convertToMillimeter(value, Worksheet::Point) );
	plot->plotArea()->setBorderPen(pen);
  }  
}

void CartesianPlotDock::borderOpacityChanged(int value){
  if (m_initializing)
	return;

  qreal opacity = (float)value/100;
  foreach(CartesianPlot* plot, m_plotList){
	plot->plotArea()->setBorderOpacity(opacity);
  }
}
