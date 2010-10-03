/***************************************************************************
    File                 : LineSymbolCurveDock.cpp
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2010 by Alexander Semke
    Email (use @ for *)  : alexander.semke*web.de
    Description          : widget for curve properties
                           
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

#include "LineSymbolCurveDock.h"
#include "worksheet/LineSymbolCurve.h"
#include "core/plugin/PluginManager.h"
#include "worksheet/StandardCurveSymbolFactory.h"
#include "widgets/TreeViewComboBox.h"
#include <QTextEdit>
#include <QPainter>
#include <QBrush>
#include <QPen>
#include <QDebug>

/*!
  \class GuiObserver
  \brief  Provides a widget for editing the properties of the worksheets currently selected in the project explorer.

  \ingroup kdefrontend
*/

LineSymbolCurveDock::LineSymbolCurveDock(QWidget *parent): QWidget(parent){
	ui.setupUi(this);
	
	// Tab "General"
	gridLayout = new QGridLayout(ui.tabGeneral);
	gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
	lName = new QLabel(ui.tabGeneral);
	lName->setObjectName(QString::fromUtf8("lName"));
	QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	sizePolicy.setHorizontalStretch(0);
	sizePolicy.setVerticalStretch(0);
	sizePolicy.setHeightForWidth(lName->sizePolicy().hasHeightForWidth());
	lName->setSizePolicy(sizePolicy);

	gridLayout->addWidget(lName, 0, 0, 1, 1);

	leName = new QLineEdit(ui.tabGeneral);
	leName->setObjectName(QString::fromUtf8("leName"));

	gridLayout->addWidget(leName, 0, 1, 1, 1);

	lComment = new QLabel(ui.tabGeneral);
	lComment->setObjectName(QString::fromUtf8("lComment"));
	sizePolicy.setHeightForWidth(lComment->sizePolicy().hasHeightForWidth());
	lComment->setSizePolicy(sizePolicy);

	gridLayout->addWidget(lComment, 1, 0, 1, 1);

	teComment = new QTextEdit(ui.tabGeneral);
	teComment->setObjectName(QString::fromUtf8("teComment"));
	QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Fixed);
	sizePolicy1.setHorizontalStretch(0);
	sizePolicy1.setVerticalStretch(0);
	sizePolicy1.setHeightForWidth(teComment->sizePolicy().hasHeightForWidth());
	teComment->setSizePolicy(sizePolicy1);
	teComment->setMaximumSize(QSize(16777215, 50));

	gridLayout->addWidget(teComment, 1, 1, 1, 1);

	
	lXColumn= new QLabel(ui.tabGeneral);
	gridLayout->addWidget(lXColumn, 2, 0, 1, 1);
	
	cbXColumn = new TreeViewComboBox(ui.tabGeneral);
	gridLayout->addWidget(cbXColumn, 2, 1, 1, 1);
	
	lYColumn= new QLabel(ui.tabGeneral);
	gridLayout->addWidget(lYColumn, 3, 0, 1, 1);
	
	cbYColumn = new TreeViewComboBox(ui.tabGeneral);
	gridLayout->addWidget(cbYColumn, 3, 1, 1, 1);
	
	verticalSpacer = new QSpacerItem(24, 320, QSizePolicy::Minimum, QSizePolicy::Expanding);
	gridLayout->addItem(verticalSpacer, 4, 0, 1, 1);
	

	
	//Line
// 	QStringList stylelist;
// 	stylelist<<i18n("Lines")<<i18n("Steps")<<i18n("Boxes")<<i18n("Impulses")<<i18n("Y Boxes");
// 	ui.cbLineType->insertItems(-1, stylelist);
// 	ui.kcbSymbolBorderColor->setColor(Qt::black);
// 	this->initLineStyles();
// 	ui.cbSymbolBorderStyle->setCurrentIndex(0);

	this->updateSymbolStyles();
// 	this->updateSymbolFillingStyles();
// 	this->updateSymbolBorderStyles();


	//Slots
	// 	connect( ui.sbLineWidth, SIGNAL(valueChanged(int)), this, SLOT(initLineStyles()) );
	
	//Symbol
	connect( ui.cbSymbolStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(symbolStyleChanged(int)) );
	connect( ui.sbSymbolSize, SIGNAL(valueChanged(int)), this, SLOT(symbolSizeChanged(int)) );
	connect( ui.sbSymbolRotation, SIGNAL(valueChanged(int)), this, SLOT(symbolRotationChanged(int)) );
	connect( ui.sbSymbolOpacity, SIGNAL(valueChanged(int)), this, SLOT(symbolOpacityChanged(int)) );
	
	connect( ui.cbSymbolFillingStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(symbolFillingStyleChanged(int)) );
	connect( ui.kcbSymbolFillingColor, SIGNAL(changed (const QColor &)), this, SLOT(symbolFillingColorChanged(const QColor)) );

	connect( ui.cbSymbolBorderStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(symbolBorderStyleChanged(int)) );
	connect( ui.kcbSymbolBorderColor, SIGNAL(changed (const QColor &)), this, SLOT(symbolBorderColorChanged(const QColor&)) );
	connect( ui.sbSymbolBorderWidth, SIGNAL(valueChanged(int)), this, SLOT(symbolBorderWidthChanged(int)) );


	retranslateUi();
}

void LineSymbolCurveDock::setModel(QAbstractItemModel * model){
  	cbXColumn->setModel(model);
	cbYColumn->setModel(model);
}

void LineSymbolCurveDock::setCurves(QList<LineSymbolCurve*> list){
  m_initializing=true;
  
  LineSymbolCurve* curve=list.first();
  
  //if there are more then one curve in the list, disable the tab "general"
  if (list.size()==1){
	lName->setEnabled(true);
	leName->setEnabled(true);
	lComment->setEnabled(true);
	teComment->setEnabled(true);
	lXColumn->setEnabled(true);
	cbXColumn->setEnabled(true);
	lYColumn->setEnabled(true);
	cbYColumn->setEnabled(true);
	
	leName->setText(curve->name());
	teComment->setText(curve->comment());
  }else{
	lName->setEnabled(false);
	leName->setEnabled(false);
	lComment->setEnabled(false);
	teComment->setEnabled(false);
	lXColumn->setEnabled(false);
	cbXColumn->setEnabled(false);
	lYColumn->setEnabled(false);
	cbYColumn->setEnabled(false);	
	
	leName->setText("");
	teComment->setText("");
	cbXColumn->setCurrentIndex(QModelIndex());
	cbYColumn->setCurrentIndex(QModelIndex());
  }
  
  //show the properties of the first curve
  //TODO select the columns in the ComboBoxes
  
  //LineStyle
  //TODO

  //Symbol
  if (curve->symbolsVisible())
	ui.cbSymbolStyle->setCurrentIndex( ui.cbSymbolStyle->findText(curve->symbolTypeId()) );
  else
	ui.cbSymbolStyle->setCurrentIndex(0);

  ui.sbSymbolSize->setValue( curve->symbolSize() );
  ui.sbSymbolRotation->setValue( curve->symbolRotationAngle() );
  ui.sbSymbolOpacity->setValue( curve->symbolsOpacity()*100 );
  ui.cbSymbolFillingStyle->setCurrentIndex( curve->symbolsBrush().style() );
  ui.kcbSymbolFillingColor->setColor( curve->symbolsBrush().color() );

  ui.cbSymbolBorderStyle->setCurrentIndex( curve->symbolsPen().style() );
  ui.kcbSymbolBorderColor->setColor( curve->symbolsPen().color() );
  ui.sbSymbolBorderWidth->setValue( curve->symbolsPen().width() );
// qDebug()<<"pen width "<<curve->symbolsPen().width();
  this->updateSymbolBorderStyles();
  this->updateSymbolFillingStyles();
  
// 	//Area filling
//TODO


  m_curvesList=list;
  m_initializing=false;
}




void LineSymbolCurveDock::resizeEvent(QResizeEvent * event){
	this->updateSymbolBorderStyles();
// 	this->initAreaFillingStyles();
	this->updateSymbolFillingStyles();
}

/*!
	fills the ComboBox for the symbol style with all possible styles in the style factory.
*/
void LineSymbolCurveDock::updateSymbolStyles(){
	ui.cbSymbolStyle->addItem("none");
	
	QPainter painter;
	int offset=5;
	int size=20; 	//TODO size of the icon depending on the actuall height of the combobox?
	QPixmap pm( size, size );
 	ui.cbSymbolStyle->setIconSize( QSize(size, size) );
	
  //TODO redesign the PluginManager and adjust this part here (load only symbol plugins and not everything!!!)
	foreach(QObject *plugin, PluginManager::plugins()) {
		CurveSymbolFactory *factory = qobject_cast<CurveSymbolFactory *>(plugin);
		if (factory) {
			AbstractCurveSymbol* symbol;
			foreach (const AbstractCurveSymbol* symbolPrototype, factory->prototypes()){
			  if (symbolPrototype){
				symbol= symbolPrototype->clone();
				symbol->setSize(15);
// 				symbol->setColor(Qt::black);
				
				pm.fill(Qt::transparent);
				painter.begin( &pm );
				painter.setRenderHint(QPainter::Antialiasing);
				painter.translate(size/2,size/2);
				symbol->paint(&painter);
				painter.end();
				ui.cbSymbolStyle->addItem(QIcon(pm), symbol->id());
			  }
		  }
		}
	}
}



/*!
	fills the ComboBox for the symbol filling patterns with the 14 possible Qt::BrushStyles.
*/
void LineSymbolCurveDock::updateSymbolFillingStyles(){
	int index=ui.cbSymbolFillingStyle->currentIndex();
	ui.cbSymbolFillingStyle->clear();
	ui.cbSymbolFillingStyle->addItem("none");

	QPainter pa;
	int offset=2;
	int w=ui.cbSymbolFillingStyle->width() - 2*offset;
	qDebug()<<"cbwidth "<<w;
// 	int h=ui.cbSymbolFillingStyle->height() - 2*offset;
	int h=20 - 2*offset;
	QPixmap pm( w, h );
	ui.cbSymbolFillingStyle->setIconSize( QSize(w,h) );

	QPen pen(Qt::SolidPattern, 1);
 	pa.setPen( pen );
	QColor color = ui.kcbSymbolFillingColor->color();
	
	//loop over 14 possible Qt-BrushStyles
	for (int i=1;i<15;i++) {
		pm.fill(Qt::transparent);
		pa.begin( &pm );
		pa.setRenderHint(QPainter::Antialiasing);
 		pa.setBrush( QBrush(color, (Qt::BrushStyle)i) );
		pa.drawRect( offset, offset, w - 2*offset, h - 2*offset);
		pa.end();
		ui.cbSymbolFillingStyle->addItem( QIcon(pm), "" );
	}

	ui.cbSymbolFillingStyle->setCurrentIndex(index);
}


/*!
	fills the ComboBox for the symbol border styles with the six possible Qt::PenStyles.
*/
void LineSymbolCurveDock::updateSymbolBorderStyles(){
	int index=ui.cbSymbolBorderStyle->currentIndex();
	ui.cbSymbolBorderStyle->clear();
	ui.cbSymbolBorderStyle->addItem("none");

	QPainter pa;
	int offset=2;
	int w=ui.cbSymbolBorderStyle->width()-2*offset;
	int h=10;
	QPixmap pm( w, h );
	ui.cbSymbolBorderStyle->setIconSize( QSize(w,h) );
	
	QColor color = ui.kcbSymbolBorderColor->color();

	//loop over six possible Qt-PenStyles, draw on the pixmap and insert it
	for (int i=1;i<6;i++) {
		pm.fill(Qt::transparent);
		pa.begin( &pm );
// 		pa.setRenderHint(QPainter::Antialiasing);
		pa.setPen( QPen( color, 1, (Qt::PenStyle)i ) );
		pa.drawLine( offset, h/2, w-offset, h/2);
		pa.end();
		ui.cbSymbolBorderStyle->addItem( QIcon(pm), "" );
	}
	ui.cbSymbolBorderStyle->setCurrentIndex(index);
}


/*!
	fills the ComboBox for the area filling with the 14 possible Qt::BrushStyles.
*/
/*
void LineSymbolCurveDock::fillAreaFillingPatternBox() {
	int index=ui.cbFillBrushStyle->currentIndex();
	ui.cbFillBrushStyle->clear();

	QPainter pa;
	int offset=5;
	int w=ui.cbFillBrushStyle->width() - 2*offset;
	int h=ui.cbFillBrushStyle->height() - 2*offset;
	QPixmap pm( w, h );
	ui.cbFillBrushStyle->setIconSize( QSize(w,h) );

 	QColor penColor = ui.kcbAreaFillingColor->color();
	QPen pen(Qt::SolidPattern, 1);
 	pa.setPen( pen );

	//loop over 14 possible Qt::BrushStyles
	for (int i=1;i<15;i++) {
		pm.fill(Qt::transparent);
		pa.begin( &pm );
//  		pa.setRenderHint(QPainter::Antialiasing);
 		pa.setBrush( QBrush(penColor, (Qt::BrushStyle)i) );
		pa.drawRect( offset, offset, w-2*offset, h-2*offset);
		pa.end();
		ui.cbFillBrushStyle->addItem( QIcon(pm), "" );
	}

	ui.cbFillBrushStyle->setCurrentIndex(index);
}
*/


//************************************************************
//****************** SLOTS ********************************
//************************************************************
void LineSymbolCurveDock::retranslateUi(){
	lName->setText(i18n("Name:", 0));
	lComment->setText(i18n("Comment:", 0));
	lXColumn->setText(i18n("x-data:"));
	lYColumn->setText(i18n("y-data:"));
	
	ui.cbSymbolStyle->setItemText(0, i18n("none"));
	ui.cbSymbolFillingStyle->setItemText(0, i18n("none"));
	ui.cbSymbolBorderStyle->setItemText(0, i18n("none"));
}

/*!
	called if the symbol type was changed.
*/
void LineSymbolCurveDock::symbolStyleChanged(int index){
  bool fillingEnabled=true; //TODO make the option "filling enabled" available in the symbol.

	//enable/disable the symbol filling options in the GUI depending on the currently selected symbol.
	if (fillingEnabled){
	  ui.lSymbolFilling->setEnabled(true);
	  ui.lSymbolFillingColor->setEnabled(true);
	  ui.kcbSymbolFillingColor->setEnabled(true);
	  ui.lSymbolFillingStyle->setEnabled(true);
	  ui.cbSymbolFillingStyle->setEnabled(true);
	}else{
	  ui.lSymbolFilling->setEnabled(false);
	  ui.lSymbolFillingColor->setEnabled(false);
	  ui.kcbSymbolFillingColor->setEnabled(false);
	  ui.lSymbolFillingStyle->setEnabled(false);
	  ui.cbSymbolFillingStyle->setEnabled(false);
	}
	
	if (m_initializing)
	  return;
	
	QString currentSymbolTypeId=ui.cbSymbolStyle->currentText();
	LineSymbolCurve* curve;
	foreach(curve, m_curvesList)
	  curve->setSymbolTypeId(currentSymbolTypeId);
	
	m_initializing=false;
}


void LineSymbolCurveDock::symbolSizeChanged(int value){
	if (m_initializing)
	  return;
	  
  	LineSymbolCurve* curve;
	foreach(curve, m_curvesList)
	  curve->setSymbolSize(value);
	
	m_initializing=false;
}

void LineSymbolCurveDock::symbolRotationChanged(int value){
  if (m_initializing)
	return;
	
  LineSymbolCurve* curve;
  foreach(curve, m_curvesList)
	curve->setSymbolRotationAngle(value);

  m_initializing=false;
}

void LineSymbolCurveDock::symbolOpacityChanged(int value){
  if (m_initializing)
	return;
		
  LineSymbolCurve* curve;
  qreal opacity = (float)value/100;
  foreach(curve, m_curvesList)
	curve->setSymbolsOpacity(opacity);
	
  m_initializing=false;
}

void LineSymbolCurveDock::symbolFillingStyleChanged(int index){
  if (m_initializing)
	return;

  LineSymbolCurve* curve;
  QBrush brush;
  foreach(curve, m_curvesList){
	brush=curve->symbolsBrush();
	brush.setStyle(Qt::BrushStyle(index));
	curve->setSymbolsBrush(brush);
  }
	
  m_initializing=false;
}

void LineSymbolCurveDock::symbolFillingColorChanged(const QColor& color){
  if (m_initializing)
	return;
	
  LineSymbolCurve* curve;
  QBrush brush;
  foreach(curve, m_curvesList){
	brush=curve->symbolsBrush();
	brush.setColor(color);
	curve->setSymbolsBrush(brush);
  }

  this->updateSymbolFillingStyles();
  m_initializing=false;
}

void LineSymbolCurveDock::symbolBorderStyleChanged(int index){
	if (m_initializing)
	  return;
	  
    LineSymbolCurve* curve;
	QPen pen;
	foreach(curve, m_curvesList){
	  pen=curve->symbolsPen();
	  pen.setStyle(Qt::PenStyle(index));
	  curve->setSymbolsPen(pen);
	}
	
	m_initializing=false;
}

void LineSymbolCurveDock::symbolBorderColorChanged(const QColor& color){
  	if (m_initializing)
	  return;
	
    LineSymbolCurve* curve;
	QPen pen;
	foreach(curve, m_curvesList){
	  pen=curve->symbolsPen();
	  pen.setColor(color);
	  curve->setSymbolsPen(pen);
	}  
	
	this->updateSymbolBorderStyles();
	m_initializing=false;
}

void LineSymbolCurveDock::symbolBorderWidthChanged(int value){
  	if (m_initializing)
	  return;
	
    LineSymbolCurve* curve;
	QPen pen;
	foreach(curve, m_curvesList){
	  pen=curve->symbolsPen();
	  pen.setWidth(value);
	  curve->setSymbolsPen(pen);
	}  
	
	m_initializing=false;
}