/***************************************************************************
    File                 : LineSymbolCurveDock.cpp
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2010 Alexander Semke (alexander.semke*web.de)
                           (replace * with @ in the email addresses)
    Description          : widget for LineSymbolCurve properties
                           
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
#include "core/AspectTreeModel.h"
#include "core/AbstractColumn.h"
#include "core/plugin/PluginManager.h"
#include "worksheet/StandardCurveSymbolFactory.h"
#include "widgets/TreeViewComboBox.h"
#include <QTextEdit>
#include <QCheckBox>
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
	
	chkVisible = new QCheckBox(ui.tabGeneral);
	gridLayout->addWidget(chkVisible, 2, 0, 1, 1);
	
	lXColumn= new QLabel(ui.tabGeneral);
	gridLayout->addWidget(lXColumn, 3, 0, 1, 1);
	
	cbXColumn = new TreeViewComboBox(ui.tabGeneral);
	gridLayout->addWidget(cbXColumn, 3, 1, 1, 1);
	
	lYColumn= new QLabel(ui.tabGeneral);
	gridLayout->addWidget(lYColumn, 4, 0, 1, 1);
	
	cbYColumn = new TreeViewComboBox(ui.tabGeneral);
	gridLayout->addWidget(cbYColumn, 4, 1, 1, 1);
	
	verticalSpacer = new QSpacerItem(24, 320, QSizePolicy::Minimum, QSizePolicy::Expanding);
	gridLayout->addItem(verticalSpacer, 5, 0, 1, 1);
	

	
	//Line
	ui.cbLineType->addItems(LineSymbolCurve::lineTypes());
	this->updatePenStyles(ui.cbLineStyle, Qt::black);
	this->updatePenStyles(ui.cbSymbolBorderStyle, Qt::black);
	this->fillSymbolStyles();
 	this->updateBrushStyles(ui.cbSymbolFillingStyle, Qt::black);


	//Slots
	
	//General
	connect( leName, SIGNAL(returnPressed()), this, SLOT(nameChanged()) );
	//TODO signal-slot for teComment
	connect( chkVisible, SIGNAL(stateChanged(int)), this, SLOT(visibilityChanged(int)) );
	connect( cbXColumn, SIGNAL(currentIndexChanged(int)), this, SLOT(xColumnChanged(int)) );
	connect( cbYColumn, SIGNAL(currentIndexChanged(int)), this, SLOT(yColumnChanged(int)) );
	
	//Lines
	connect( ui.cbLineType, SIGNAL(currentIndexChanged(int)), this, SLOT(lineTypeChanged(int)) );
	connect( ui.cbLineStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(lineStyleChanged(int)) );
	connect( ui.kcbLineColor, SIGNAL(changed (const QColor &)), this, SLOT(lineColorChanged(const QColor&)) );
	connect( ui.sbLineWidth, SIGNAL(valueChanged(int)), this, SLOT(lineWidthChanged(int)) );
	connect( ui.sbLineOpacity, SIGNAL(valueChanged(int)), this, SLOT(lineOpacityChanged(int)) );
	
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

void LineSymbolCurveDock::setModel(AspectTreeModel* model){
	QList<const char *>  list;
	list<<"Folder"<<"Spreadsheet"<<"FileDataSource"<<"Column";
	cbXColumn->setTopLevelClasses(list);
	cbYColumn->setTopLevelClasses(list);
	
 	list.clear();
	list<<"Column";
	model->setSelectableAspects(list);
	
	m_initializing=true;
  	cbXColumn->setModel(model);
	cbYColumn->setModel(model);
	
	m_aspectTreeModel=model;
	m_initializing=false;
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
  
  //General-tab
  chkVisible->setChecked(curve->isVisible());
  cbXColumn->setCurrentIndex( m_aspectTreeModel->modelIndexOfAspect(curve->xColumn()) );
  cbYColumn->setCurrentIndex( m_aspectTreeModel->modelIndexOfAspect(curve->yColumn()) );
  
  //Line-tab
  ui.cbLineType->setCurrentIndex( curve->lineType() );
  ui.cbLineStyle->setCurrentIndex( curve->linePen().style() );
  ui.kcbLineColor->setColor( curve->linePen().color() );
  ui.sbLineWidth->setValue( curve->linePen().width() );
  ui.sbLineOpacity->setValue( curve->lineOpacity()*100 );
  this->updatePenStyles(ui.cbLineStyle, curve->linePen().color() );
  
  //Symbol-tab
  ui.cbSymbolStyle->setCurrentIndex( ui.cbSymbolStyle->findText(curve->symbolTypeId()) );
  ui.sbSymbolSize->setValue( curve->symbolSize() );
  ui.sbSymbolRotation->setValue( curve->symbolRotationAngle() );
  ui.sbSymbolOpacity->setValue( curve->symbolsOpacity()*100 );
  ui.cbSymbolFillingStyle->setCurrentIndex( curve->symbolsBrush().style() );
  ui.kcbSymbolFillingColor->setColor( curve->symbolsBrush().color() );

  ui.cbSymbolBorderStyle->setCurrentIndex( curve->symbolsPen().style() );
  ui.kcbSymbolBorderColor->setColor( curve->symbolsPen().color() );
  ui.sbSymbolBorderWidth->setValue( curve->symbolsPen().width() );

  this->updatePenStyles(ui.cbSymbolBorderStyle, curve->symbolsPen().color() );
  this->updateBrushStyles(ui.cbSymbolFillingStyle, curve->symbolsBrush().color() );
  

  //TODO
  //Values
  //Area filling
  //Error bars


  m_curvesList=list;
  m_initializing=false;
}


void LineSymbolCurveDock::resizeEvent(QResizeEvent * event){
  Q_UNUSED(event);
  
/* TODO  
	this->updateSymbolBorderStyles();
// 	this->initAreaFillingStyles();
	this->updateSymbolFillingStyles();*/
}

/*!
	fills the ComboBox for the symbol style with all possible styles in the style factory.
*/
void LineSymbolCurveDock::fillSymbolStyles(){
	QPainter painter;
	int size=20; 	//TODO size of the icon depending on the actuall height of the combobox?
	QPixmap pm( size, size );
 	ui.cbSymbolStyle->setIconSize( QSize(size, size) );
	
  //TODO redesign the PluginManager and adjust this part here (load only symbol plugins and not everything!!!)
	foreach(QObject *plugin, PluginManager::plugins()) {
		CurveSymbolFactory *factory = qobject_cast<CurveSymbolFactory *>(plugin);
		if (factory){
		  symbolFactory=factory;
		  AbstractCurveSymbol* symbol;
		  foreach (const AbstractCurveSymbol* symbolPrototype, factory->prototypes()){
			if (symbolPrototype){
			  symbol= symbolPrototype->clone();
			  symbol->setSize(15);
			  
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
	fills the ComboBox \c combobox with the six possible Qt::PenStyles, the color \c color is used.
*/
void LineSymbolCurveDock::updatePenStyles(QComboBox* comboBox, const QColor& color){
	int index=comboBox->currentIndex();
	comboBox->clear();
	comboBox->addItem("none");

	QPainter pa;
	int offset=2;
	int w=comboBox->width()-2*offset;
	int h=10;
	QPixmap pm( w, h );
	comboBox->setIconSize( QSize(w,h) );
	
	//loop over six possible Qt-PenStyles, draw on the pixmap and insert it
	for (int i=1;i<6;i++) {
		pm.fill(Qt::transparent);
		pa.begin( &pm );
// 		pa.setRenderHint(QPainter::Antialiasing);
		pa.setPen( QPen( color, 1, (Qt::PenStyle)i ) );
		pa.drawLine( offset, h/2, w-offset, h/2);
		pa.end();
		comboBox->addItem( QIcon(pm), "" );
	}
	comboBox->setCurrentIndex(index);
}


/*!
	fills the ComboBox for the symbol filling patterns with the 14 possible Qt::BrushStyles.
*/
void LineSymbolCurveDock::updateBrushStyles(QComboBox* comboBox, const QColor& color){
  	int index=comboBox->currentIndex();
	comboBox->clear();
	comboBox->addItem( i18n("none") );

	QPainter pa;
	int offset=2;
	int w=comboBox->width() - 2*offset;
	qDebug()<<"cbwidth "<<w;
// 	int h=ui.cbSymbolFillingStyle->height() - 2*offset;
	int h=20 - 2*offset;
	QPixmap pm( w, h );
	comboBox->setIconSize( QSize(w,h) );

	QPen pen(Qt::SolidPattern, 1);
 	pa.setPen( pen );
	
	for (int i=1;i<15;i++) {
		pm.fill(Qt::transparent);
		pa.begin( &pm );
		pa.setRenderHint(QPainter::Antialiasing);
 		pa.setBrush( QBrush(color, (Qt::BrushStyle)i) );
		pa.drawRect( offset, offset, w - 2*offset, h - 2*offset);
		pa.end();
		comboBox->addItem( QIcon(pm), "" );
	}

	comboBox->setCurrentIndex(index);
}


//************************************************************
//****************** SLOTS ********************************
//************************************************************
void LineSymbolCurveDock::retranslateUi(){
	lName->setText(i18n("Name:"));
	lComment->setText(i18n("Comment:"));
	chkVisible->setText(i18n("Visible"));
	lXColumn->setText(i18n("x-data:"));
	lYColumn->setText(i18n("y-data:"));
	
// 	ui.cbSymbolStyle->setItemText(0, i18n("none"));
	ui.cbSymbolFillingStyle->setItemText(0, i18n("none"));
	ui.cbSymbolBorderStyle->setItemText(0, i18n("none"));
}

// "General"-tab
void LineSymbolCurveDock::nameChanged(){
    m_curvesList.first()->setName(leName->text());
}


void LineSymbolCurveDock::commentChanged(){
  
}

void LineSymbolCurveDock::xColumnChanged(int index){
  if (m_initializing)
	return;
  
  AbstractColumn* column= static_cast<AbstractColumn*>(cbXColumn->currentIndex().internalPointer());
  LineSymbolCurve* curve;
  foreach(curve, m_curvesList){
	curve->setXColumn(column);
  }
}

void LineSymbolCurveDock::yColumnChanged(int index){
  if (m_initializing)
  return;
  
  AbstractColumn* column= static_cast<AbstractColumn*>(cbXColumn->currentIndex().internalPointer());
  LineSymbolCurve* curve;
  foreach(curve, m_curvesList){
	curve->setYColumn(column);
  }
}

void LineSymbolCurveDock::visibilityChanged(int state){
  if (m_initializing)
	return;
  
  bool b;
  if (state==Qt::Checked)
	b=true;
  else
	b=false;

  LineSymbolCurve* curve;
  foreach(curve, m_curvesList){
	curve->setVisible(b);
  }
}

// "Line"-tab
void LineSymbolCurveDock::lineTypeChanged(int index){
  if (m_initializing)
	return;
  
  LineSymbolCurve::LineType lineType = LineSymbolCurve::LineType(index);
  LineSymbolCurve* curve;
  foreach(curve, m_curvesList){
	curve->setLineType(lineType);
  }
}


void LineSymbolCurveDock::lineStyleChanged(int index){
  Qt::PenStyle penStyle=Qt::PenStyle(index);
  
  if ( penStyle == Qt::NoPen ){
	ui.kcbLineColor->setEnabled(false);
	ui.sbLineWidth->setEnabled(false);
	ui.sbLineOpacity->setEnabled(false);
  }else{
	ui.kcbLineColor->setEnabled(true);
	ui.sbLineWidth->setEnabled(true);
	ui.sbLineOpacity->setEnabled(true);
  }
  
   if (m_initializing)
	return;
	
  LineSymbolCurve* curve;
  QPen pen;
  foreach(curve, m_curvesList){
	pen=curve->linePen();
	pen.setStyle(penStyle);
	curve->setLinePen(pen);
  }
}

void LineSymbolCurveDock::lineColorChanged(const QColor& color){
  if (m_initializing)
	return;

  LineSymbolCurve* curve;
  QPen pen;
  foreach(curve, m_curvesList){
	pen=curve->linePen();
	pen.setColor(color);
	curve->setLinePen(pen);
  }  

  this->updatePenStyles(ui.cbLineStyle, color);
}

void LineSymbolCurveDock::lineWidthChanged(int value){
  if (m_initializing)
	return;
  
  LineSymbolCurve* curve;
  QPen pen;
  foreach(curve, m_curvesList){
	pen=curve->linePen();
	pen.setWidth(value);
	curve->setLinePen(pen);
  }  
}

void LineSymbolCurveDock::lineOpacityChanged(int value){
  if (m_initializing)
	return;
		
  LineSymbolCurve* curve;
  qreal opacity = (float)value/100;
  foreach(curve, m_curvesList)
	curve->setLineOpacity(opacity);
	
}

//"Symbol"-tab
void LineSymbolCurveDock::symbolStyleChanged(int index){
  Q_UNUSED(index);
  QString currentSymbolTypeId = ui.cbSymbolStyle->currentText();
  bool fillingEnabled = symbolFactory->prototype(currentSymbolTypeId)->fillingEnabled();
  
  if (currentSymbolTypeId=="none"){
	ui.sbSymbolSize->setEnabled(false);
	ui.sbSymbolRotation->setEnabled(false);
	ui.sbSymbolOpacity->setEnabled(false);
	
	ui.lSymbolFilling->setEnabled(false);
	ui.lSymbolFillingColor->setEnabled(false);
	ui.kcbSymbolFillingColor->setEnabled(false);
	ui.lSymbolFillingStyle->setEnabled(false);
	ui.cbSymbolFillingStyle->setEnabled(false);
	
	ui.lSymbolBorder->setEnabled(false);
	ui.cbSymbolBorderStyle->setEnabled(false);
	ui.kcbSymbolBorderColor->setEnabled(false);
	ui.sbSymbolBorderWidth->setEnabled(false);
  }else{
	ui.sbSymbolSize->setEnabled(true);
	ui.sbSymbolRotation->setEnabled(true);
	ui.sbSymbolOpacity->setEnabled(true);
	
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
	
	ui.lSymbolBorder->setEnabled(true);
	ui.cbSymbolBorderStyle->setEnabled(true);
	ui.kcbSymbolBorderColor->setEnabled(true);
	ui.sbSymbolBorderWidth->setEnabled(true);
  }

  if (m_initializing)
	return;

  LineSymbolCurve* curve;
  foreach(curve, m_curvesList){
	curve->setSymbolTypeId(currentSymbolTypeId);
  }

}


void LineSymbolCurveDock::symbolSizeChanged(int value){
  if (m_initializing)
	return;
	
  LineSymbolCurve* curve;
  foreach(curve, m_curvesList)
	curve->setSymbolSize(value);
  
}

void LineSymbolCurveDock::symbolRotationChanged(int value){
  if (m_initializing)
	return;
	
  LineSymbolCurve* curve;
  foreach(curve, m_curvesList)
	curve->setSymbolRotationAngle(value);

}

void LineSymbolCurveDock::symbolOpacityChanged(int value){
  if (m_initializing)
	return;
		
  LineSymbolCurve* curve;
  qreal opacity = (float)value/100;
  foreach(curve, m_curvesList)
	curve->setSymbolsOpacity(opacity);
	
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

  this->updateBrushStyles(ui.cbSymbolFillingStyle, curve->symbolsBrush().color() );
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
  
  this->updatePenStyles(ui.cbSymbolBorderStyle, color);
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
}
