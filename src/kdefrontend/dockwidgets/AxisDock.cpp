/***************************************************************************
    File                 : AxisDock.cc
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 20011 by Alexander Semke
    Email (use @ for *)  : alexander.semke*web.de
    Description          : axes widget class

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
#include "AxisDock.h"
#include "worksheet/Axis.h"
#include "kdefrontend/LabelWidget.h"
#include "kdefrontend/GuiTools.h"
#include <KDebug>
#include <KMessageBox>
#include <QPainter>
#include <QTimer>


 /*!
  \class AxisDock
  \brief Provides a widget for editing the properties of the axes currently selected in the project explorer.

  \ingroup kdefrontend
*/
 
AxisDock::AxisDock(QWidget* parent):QWidget(parent){

  ui.setupUi(this);

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

	//**********************************  Slots **********************************************

	//"General"-tab
	connect( ui.leName, SIGNAL(returnPressed()), this, SLOT(nameChanged()) );
	//TODO signal-slot for teComment
	connect( ui.chkVisible, SIGNAL(stateChanged(int)), this, SLOT(visibilityChanged(int)) );
	
	connect( ui.cbOrientation, SIGNAL(currentIndexChanged(int)), this, SLOT(orientationChanged(int)) );
	connect( ui.cbPosition, SIGNAL(currentIndexChanged(int)), this, SLOT(positionChanged(int)) );
	connect( ui.lePositionOffset, SIGNAL(returnPressed()), this, SLOT(positionOffsetChanged()) );
	connect( ui.cbScale, SIGNAL(currentIndexChanged(int)), this, SLOT(scaleChanged(int)) );
	
	connect( ui.leStart, SIGNAL(returnPressed()), this, SLOT(startChanged()) );
	connect( ui.leEnd, SIGNAL(returnPressed()), this, SLOT(endChanged()) );
	connect( ui.leZeroOffset, SIGNAL(returnPressed()), this, SLOT(zeroOffsetChanged()) );
	connect( ui.leScalingFactor, SIGNAL(returnPressed()), this, SLOT(scalingFactorChanged()) );
	
	//"Line"-tab
	connect( ui.cbLineStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(lineStyleChanged(int)) );
	connect( ui.kcbLineColor, SIGNAL(changed (const QColor &)), this, SLOT(lineColorChanged(const QColor&)) );
	connect( ui.sbLineWidth, SIGNAL(valueChanged(int)), this, SLOT(lineWidthChanged(int)) );
	connect( ui.sbLineOpacity, SIGNAL(valueChanged(int)), this, SLOT(lineOpacityChanged(int)) );	
	

	//"Title"-tab
//	connect( labelWidget, SIGNAL(dataChanged(bool)), this, SLOT(slotDataChanged()) );

	//"Major ticks"-tab
	connect( ui.cbMajorTicksDirection, SIGNAL(currentIndexChanged(int)), this, SLOT(majorTicksDirectionChanged(int)) );
	connect( ui.cbMajorTicksType, SIGNAL(currentIndexChanged(int)), this, SLOT(majorTicksTypeChanged(int)) );
 	connect( ui.leMajorTicksNumber, SIGNAL(returnPressed()), this, SLOT(majorTicksNumberChanged()) );
	connect( ui.cbMajorTicksLineStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(majorTicksLineStyleChanged(int)) );
	connect( ui.kcbMajorTicksColor, SIGNAL(changed (const QColor &)), this, SLOT(majorTicksColorChanged(const QColor&)) );
	connect( ui.sbMajorTicksWidth, SIGNAL(valueChanged(int)), this, SLOT(majorTicksWidthChanged(int)) );
	connect( ui.sbMajorTicksLength, SIGNAL(valueChanged(int)), this, SLOT(majorTicksLengthChanged(int)) );
	connect( ui.sbMajorTicksOpacity, SIGNAL(valueChanged(int)), this, SLOT(majorTicksOpacityChanged(int)) );

	//"Minor ticks"-tab
	connect( ui.cbMinorTicksDirection, SIGNAL(currentIndexChanged(int)), this, SLOT(minorTicksDirectionChanged(int)) );
	connect( ui.cbMinorTicksType, SIGNAL(currentIndexChanged(int)), this, SLOT(minorTicksTypeChanged(int)) );
 	connect( ui.leMinorTicksNumber, SIGNAL(returnPressed()), this, SLOT(minorTicksNumberChanged()) );
	connect( ui.cbMinorTicksLineStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(minorTicksLineStyleChanged(int)) );
	connect( ui.kcbMinorTicksColor, SIGNAL(changed (const QColor &)), this, SLOT(minorTicksColorChanged(const QColor&)) );
	connect( ui.sbMinorTicksWidth, SIGNAL(valueChanged(int)), this, SLOT(minorTicksWidthChanged(int)) );
	connect( ui.sbMinorTicksLength, SIGNAL(valueChanged(int)), this, SLOT(minorTicksLengthChanged(int)) );
	connect( ui.sbMinorTicksOpacity, SIGNAL(valueChanged(int)), this, SLOT(minorTicksOpacityChanged(int)) );
	
	//"Extra ticks"-tab
	
/*
	//"Tick labels"-tab
	connect( ui.chbLabels, SIGNAL(stateChanged(int)), this, SLOT(slotDataChanged()) );
	connect( ui.sbLabelsPosition, SIGNAL(valueChanged(int)), this, SLOT(slotDataChanged()) );
	connect( ui.leLabelsRotation, SIGNAL(textChanged(const QString&)), this, SLOT(slotDataChanged()) );

	connect( ui.sbLabelsPrecision, SIGNAL(valueChanged(int)), this, SLOT(slotDataChanged()) );
	connect( ui.cbLabelsFormat, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(labelFormatChanged(const QString&)) );
	connect( ui.leLabelsDateFormat, SIGNAL(textChanged(const QString&)), this, SLOT(slotDataChanged()) );

	connect( ui.kfrLabelsFont, SIGNAL(fontSelected(const QFont& )), this, SLOT(slotDataChanged()) );
	connect( ui.kcbLabelsColor, SIGNAL(changed(const QColor& )), this, SLOT(slotDataChanged()) );
	connect( ui.leLabelsPrefix, SIGNAL(textChanged(const QString&)), this, SLOT(slotDataChanged()) );
	connect( ui.leLabelsSuffix, SIGNAL(textChanged(const QString&)), this, SLOT(slotDataChanged()) );

	//"Grid"-tab
	connect( ui.chbMajorGrid, SIGNAL(stateChanged(int)), this, SLOT(slotDataChanged()) );
	connect( ui.cbMajorGridStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(slotDataChanged()) );
	connect( ui.kcbMajorGridColor, SIGNAL(changed (const QColor &)), this, SLOT(createMajorGridStyles()) );
	connect( ui.sbMajorGridWidth, SIGNAL(valueChanged(int)), this, SLOT(createMajorGridStyles()) );

	connect( ui.chbMinorGrid, SIGNAL(stateChanged(int)), this, SLOT(slotDataChanged()) );
	connect( ui.cbMinorGridStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(slotDataChanged()) );
	connect( ui.kcbMinorGridColor, SIGNAL(changed (const QColor &)), this, SLOT(createMinorGridStyles()) );
	connect( ui.sbMinorGridWidth, SIGNAL(valueChanged(int)), this, SLOT(createMinorGridStyles()) );
	*/
	QTimer::singleShot(0, this, SLOT(init()));
}

AxisDock::~AxisDock(){}

void AxisDock::init(){
	m_initializing=true;

// 	//create a LabelWidget in the "Title"-tab
//     QHBoxLayout* hboxLayout = new QHBoxLayout(ui.tabTitle);
// 	labelWidget=new LabelWidget(ui.tabTitle);
//     hboxLayout->addWidget(labelWidget);

	//Validators
	ui.lePositionOffset->setValidator( new QDoubleValidator(ui.lePositionOffset) );
	ui.leStart->setValidator( new QDoubleValidator(ui.leStart) );
	ui.leEnd->setValidator( new QDoubleValidator(ui.leEnd) );
	ui.leZeroOffset->setValidator( new QDoubleValidator(ui.leZeroOffset) );
	ui.leScalingFactor->setValidator( new QDoubleValidator(ui.leScalingFactor) );
	ui.leMajorTicksNumber->setValidator( new QDoubleValidator(ui.leMajorTicksNumber) );
// 	ui.leMajorTicksLength->setValidator( new QDoubleValidator(ui.leMajorTicksLength) );
	ui.leMinorTicksNumber->setValidator( new QDoubleValidator(ui.leMinorTicksNumber) );
// 	ui.leMinorTicksLength->setValidator( new QDoubleValidator(ui.leMinorTicksLength) );
// 	ui.leLabelsRotation->setValidator( new QIntValidator(ui.leLabelsRotation) );

	//TODO move this stuff to retranslateUI()
	ui.cbScale->addItem( i18n("linear") );
	ui.cbScale->addItem( i18n("log(x)") );
	ui.cbScale->addItem( i18n("log2(x)") );
	ui.cbScale->addItem( i18n("ln(x)") );
	ui.cbScale->addItem( i18n("sqrt(x)") );
	ui.cbScale->addItem( i18n("x^2") );

	ui.cbOrientation->addItem( i18n("horizontal") );
	ui.cbOrientation->addItem( i18n("vertical") );
	
	ui.cbMajorTicksDirection->addItem( i18n("none") );
	ui.cbMajorTicksDirection->addItem( i18n("in") );
	ui.cbMajorTicksDirection->addItem( i18n("out") );
	ui.cbMajorTicksDirection->addItem( i18n("in and out") );

	ui.cbMinorTicksDirection->addItem( i18n("none") );
	ui.cbMinorTicksDirection->addItem( i18n("in") );
	ui.cbMinorTicksDirection->addItem( i18n("out") );
	ui.cbMinorTicksDirection->addItem( i18n("in and out") );
	
	//Tick labels format
// 	 ui.cbLabelsFormat->addItem( i18n("automatic") );
// 	 ui.cbLabelsFormat->addItem( i18n("normal") );
// 	 ui.cbLabelsFormat->addItem( i18n("scientific") );
// 	 ui.cbLabelsFormat->addItem( i18n("power of 10") );
// 	 ui.cbLabelsFormat->addItem( i18n("power of 2") );
// 	 ui.cbLabelsFormat->addItem( i18n("power of e") );
// 	 ui.cbLabelsFormat->addItem( i18n("sqrt") );
// 	 ui.cbLabelsFormat->addItem( i18n("time") );
// 	 ui.cbLabelsFormat->addItem( i18n("date") );
// 	 ui.cbLabelsFormat->addItem( i18n("datetime") );
// 	 ui.cbLabelsFormat->addItem( i18n("degree") );

	//Grid
// 	 ui.kcbMajorGridColor->setColor(Qt::black);
// 	 this->createMajorGridStyles();
// 	 ui.kcbMinorGridColor->setColor(Qt::black);
// 	 this->createMinorGridStyles();
  m_initializing=false;
}


/*!
  sets the axiss. The properties of the axiss in the list \c list can be edited in this widget.
*/
void AxisDock::setAxes(QList<Axis*> list){
  m_initializing=true;
  m_axesList=list;
  Axis* axis=list.first();
  
  //if there are more then one axis in the list, disable the tab "general"
  if (list.size()==1){
	ui.lName->setEnabled(true);
	ui.leName->setEnabled(true);
	ui.lComment->setEnabled(true);
	ui.teComment->setEnabled(true);
	ui.leName->setText(axis->name());
	ui.teComment->setText(axis->comment());
  }else{
	ui.lName->setEnabled(false);
	ui.leName->setEnabled(false);
	ui.lComment->setEnabled(false);
	ui.teComment->setEnabled(false);
	ui.leName->setText("");
	ui.teComment->setText("");	
  }

  //show the properties of the first axis
  
 	//"General"-tab
  ui.chkVisible->setChecked(axis->isVisible());
	//orientation
// 	ui.cbPosition->setCurrentIndex( axis->position() );
	ui.cbScale->setCurrentIndex( axis->scale() );
	ui.leStart->setText( QString::number(axis->start()) );
	ui.leEnd->setText( QString::number(axis->end()) );
// 	ui.leZeroOffset->setText( QString::number(axis->offset()) );
// 	ui.leScalingFactor->setText( QString::number(axis->scaleFactor()) );

 
// 	//*******************   "Title"-tab  *************************************
//  	labelWidget->setLabel( const_cast<Axis*>(axis)->label() );

  //Line-tab
  ui.cbLineStyle->setCurrentIndex( axis->linePen().style() );
  ui.kcbLineColor->setColor( axis->linePen().color() );
  ui.sbLineWidth->setValue( axis->linePen().width() );
  ui.sbLineOpacity->setValue( axis->lineOpacity()*100 );
  GuiTools::updatePenStyles(ui.cbLineStyle, axis->linePen().color() );
  
	//"Major ticks"-tab
 	ui.cbMajorTicksDirection->setCurrentIndex( axis->majorTicksDirection() );
	ui.cbMajorTicksType->setCurrentIndex( axis->majorTicksType() );
	ui.leMajorTicksNumber->setText(QString::number(axis->majorTicksNumber()));
	ui.cbMajorTicksLineStyle->setCurrentIndex( axis->majorTicksPen().style() );
	ui.kcbMajorTicksColor->setColor( axis->majorTicksPen().color() );
	ui.sbMajorTicksWidth->setValue( axis->majorTicksPen().width() );
	ui.sbMajorTicksLength->setValue( axis->majorTicksLength() );
	ui.sbMajorTicksOpacity->setValue( axis->majorTicksOpacity()*100 );
	GuiTools::updatePenStyles(ui.cbMajorTicksLineStyle, axis->majorTicksPen().color() );

	//"Minor ticks"-tab
 	ui.cbMinorTicksDirection->setCurrentIndex( axis->minorTicksDirection() );
	ui.cbMinorTicksType->setCurrentIndex( axis->minorTicksType() );
	ui.leMinorTicksNumber->setText(QString::number(axis->minorTicksNumber()));
	ui.cbMinorTicksLineStyle->setCurrentIndex( axis->minorTicksPen().style() );
	ui.kcbMinorTicksColor->setColor( axis->minorTicksPen().color() );
	ui.sbMinorTicksWidth->setValue( axis->minorTicksPen().width() );
	ui.sbMinorTicksLength->setValue( axis->minorTicksLength() );
	ui.sbMinorTicksOpacity->setValue( axis->minorTicksOpacity()*100 );
	GuiTools::updatePenStyles(ui.cbMinorTicksLineStyle, axis->minorTicksPen().color() );
	
	//"Extra ticks"-tab
	
// 
// 	//*******************   "Tick labels"-tab  ************************************
// 	ui.chbLabels->setChecked( axis->hasLabels() );
// 	ui.sbLabelsPosition->setValue( axis->labelsPosition() );
// 	ui.leLabelsRotation->setText( QString::number(axis->labelsRotation()) );
// 
// 	ui.sbLabelsPrecision->setValue( axis->labelsPrecision() );
// 	ui.cbLabelsFormat->setCurrentIndex( axis->labelsFormat() );
// 	ui.leLabelsDateFormat->setText( axis->labelsDateFormat() );
// 
// 	ui.kfrLabelsFont->setFont( axis->labelsFont() );
// 	ui.kcbLabelsColor->setColor( axis->labelsColor() );
// 	ui.leLabelsPrefix->setText( axis->labelsPrefix() );
// 	ui.leLabelsSuffix->setText( axis->labelsSuffix() );
// 
// 
// 	//*******************   "Grid"-tab  ************************************
// 	ui.chbMajorGrid->setChecked( axis->hasMajorGrid() );
// 	ui.cbMajorGridStyle->setCurrentIndex( axis->majorGridStyle()-1 );
// 	ui.kcbMajorGridColor->setColor( axis->majorGridColor() );
// 	ui.sbMajorGridWidth->setValue( axis->majorGridWidth() );
// 
// 	ui.chbMinorGrid->setChecked( axis->hasMinorGrid() );
// 	ui.cbMinorGridStyle->setCurrentIndex( axis->minorGridStyle()-1 );
// 	ui.kcbMinorGridColor->setColor( axis->minorGridColor() );
// 	ui.sbMinorGridWidth->setValue( axis->minorGridWidth() );

  m_initializing = false;
}




/*!
	restores/shows default axis properties in the UI.
*/
/*
void AxisDock::restoreDefaults(){
	if (KMessageBox::warningYesNo(this,
																i18n("All settings will be reset to the default values. Do you want to continue?"),
																i18n("Restore default axes values")) ==  KMessageBox::Yes){

		//TODO
		KMessageBox::sorry(this,  i18n("Not yet implemented. Sorry."), i18n("Restore default axes values") );
	}
}
*/

//**********************************************************
//****************** SLOTS *******************************
//**********************************************************


//"General"-tab
void AxisDock::nameChanged(){
  if (m_initializing)
	return;
  
  m_axesList.first()->setName(ui.leName->text());
}


void AxisDock::commentChanged(){
  if (m_initializing)
	return;
  
  m_axesList.first()->setComment(ui.teComment->toPlainText());
}

void AxisDock::visibilityChanged(int state){
  if (m_initializing)
	return;
  
  bool b;
  if (state==Qt::Checked)
	b=true;
  else
	b=false;

  foreach(Axis* axis, m_axesList){
	axis->setVisible(b);
  }
}

/*!
	called if the orientation (horizontal or vertical) of the current axis is changed.
	Updates the number of minor ticks.
*/

void AxisDock::orientationChanged(int index){
  if (m_initializing)
	return;

  int oldIndex = ui.cbPosition->currentIndex();
  ui.cbPosition->clear();
  if (index==0){//horizontal
	ui.cbPosition->addItem( i18n("bottom") );
	ui.cbPosition->addItem( i18n("top") );
  }else{//vertical
	ui.cbPosition->addItem( i18n("left") );
	ui.cbPosition->addItem( i18n("right") );
  }
  ui.cbPosition->addItem( i18n("custom") );	  
  ui.cbPosition->setCurrentIndex(oldIndex);
  
//   Axis::AxisScaling scaling = (Axis::AxisScaling)index;
//   foreach(Axis* axis, m_axesList){
// 	axis->setScaling(scaling);
//   }
}

void AxisDock::positionChanged(int index){
  if (m_initializing)
	return;

  	if (index==3){
		ui.lPositionOffset->show();
		ui.lePositionOffset->show();
	}else{
		ui.lPositionOffset->hide();
		ui.lePositionOffset->hide();
	}

//   Axis::AxisScaling scaling = (Axis::AxisScaling)index;
//   foreach(Axis* axis, m_axesList){
// 	axis->setScaling(scaling);
//   }
}

void AxisDock::positionOffsetChanged(){
  if (m_initializing)
	return;
  
  double offset = ui.lePositionOffset->text().toDouble();
  foreach(Axis* axis, m_axesList){
	axis->setOffset(offset);
  }
}

void AxisDock::scaleChanged(int index){
  if (m_initializing)
	return;

  Axis::AxisScale scale = (Axis::AxisScale)index;
  foreach(Axis* axis, m_axesList){
	axis->setScale(scale);
  }
}

void AxisDock::startChanged(){
  if (m_initializing)
	return;
  
  double value = ui.leStart->text().toDouble();

  //check first, whether the value for the lower limit is valid for the log- and square root scaling. If not, set the default values.
  Axis::AxisScale scale = Axis::AxisScale(ui.cbScale->currentIndex());
  if (scale==Axis::ScaleLog10|| scale==Axis::ScaleLog2|| scale==Axis::ScaleLn){
	  if(value <= 0){
		  KMessageBox::sorry(this,
										  i18n("The axes lower limit has a non-positive value. Default minimal value will be used."),
										  i18n("Wrong lower limit value") );
		  ui.leStart->setText( "0.01" );
		  value=0.01;
	  }
  }else if (scale==Axis::ScaleSqrt){
	  if(value < 0){
		  KMessageBox::sorry(this,
										  i18n("The axes lower limit has a negative value. Default minimal value will be used."),
										  i18n("Wrong lower limit value") );
		  ui.leStart->setText( "0" );
		  value=0;
	  }
  }

  foreach(Axis* axis, m_axesList){
	axis->setStart(value);
  }
}

void AxisDock::endChanged(){
  if (m_initializing)
	return;

  double value = ui.leEnd->text().toDouble();
  foreach(Axis* axis, m_axesList){
	axis->setEnd(value);
  }
}


// "Line"-tab
void AxisDock::lineStyleChanged(int index){
   if (m_initializing)
	return;

  Qt::PenStyle penStyle=Qt::PenStyle(index);
  QPen pen;
  foreach(Axis* axis, m_axesList){
	pen=axis->linePen();
	pen.setStyle(penStyle);
	axis->setLinePen(pen);
  }
}

void AxisDock::lineColorChanged(const QColor& color){
  if (m_initializing)
	return;

  QPen pen;
  foreach(Axis* axis, m_axesList){
	pen=axis->linePen();
	pen.setColor(color);
	axis->setLinePen(pen);
  }  

  GuiTools::updatePenStyles(ui.cbLineStyle, color);
}

void AxisDock::lineWidthChanged(int value){
  if (m_initializing)
	return;
  
  QPen pen;
  foreach(Axis* axis, m_axesList){
	pen=axis->linePen();
	pen.setWidth(value);
	axis->setLinePen(pen);
  }  
}

void AxisDock::lineOpacityChanged(int value){
  if (m_initializing)
	return;
		
  qreal opacity = (float)value/100;
  foreach(Axis* axis, m_axesList)
	axis->setLineOpacity(opacity);
}

//"Major ticks" tab
void AxisDock::majorTicksDirectionChanged(int index){
  if (m_initializing)
	return;

  Axis::TicksDirection direction = Axis::TicksDirection(index);
  foreach(Axis* axis, m_axesList)
	axis->setMajorTicksDirection(direction);
}

/*!
	called if the current style of the ticks (Number or Increment) is changed.
	Adjusts the names of the corresponding labels in UI.
*/
void AxisDock::majorTicksTypeChanged(int index){
  Axis::TicksType type = Axis::TicksType(index);
  if ( type == Axis::TicksTotalNumber){
	  ui.lMajorTicksNumber->setText( i18n("Number") );
	  ui.leMajorTicksNumber->setText( QString::number(m_axesList.first()->majorTicksNumber()) );
  }else{
	  ui.lMajorTicksNumber->setText( i18n("Increment") );
	  ui.leMajorTicksNumber->setText( QString::number(m_axesList.first()->majorTicksIncrement()) );
  }
  
  if (m_initializing)
	return;
		
  foreach(Axis* axis, m_axesList){
	axis->setMajorTicksType(type);
  }
}

void AxisDock::majorTicksNumberChanged(){
  if (m_initializing)
	return;

  double value = ui.leMajorTicksNumber->text().toDouble();
  foreach(Axis* axis, m_axesList){
	axis->setMajorTicksNumber(value);
  }
}

void AxisDock::majorTicksLineStyleChanged(int index){
   if (m_initializing)
	return;

  Qt::PenStyle penStyle=Qt::PenStyle(index);
  QPen pen;
  foreach(Axis* axis, m_axesList){
	pen=axis->majorTicksPen();
	pen.setStyle(penStyle);
	axis->setMajorTicksPen(pen);
  }
}

void AxisDock::majorTicksColorChanged(const QColor& color){
  if (m_initializing)
	return;

  QPen pen;
  foreach(Axis* axis, m_axesList){
	pen=axis->majorTicksPen();
	pen.setColor(color);
	axis->setMajorTicksPen(pen);
  }  

  GuiTools::updatePenStyles(ui.cbMajorTicksLineStyle, color);
}

void AxisDock::majorTicksWidthChanged(int value){
  if (m_initializing)
	return;
  
  QPen pen;
  foreach(Axis* axis, m_axesList){
	pen=axis->majorTicksPen();
	pen.setWidth(value);
	axis->setMajorTicksPen(pen);
  }  
}

void AxisDock::majorTicksLengthChanged(int value){
  if (m_initializing)
	return;
  
  foreach(Axis* axis, m_axesList){
	axis->setMajorTicksLength(value);
  }  
}

void AxisDock::majorTicksOpacityChanged(int value){
  if (m_initializing)
	return;
		
  qreal opacity = (float)value/100;
  foreach(Axis* axis, m_axesList)
	axis->setMajorTicksOpacity(opacity);
	
}

//"Minor ticks" tab
void AxisDock::minorTicksDirectionChanged(int index){
  if (m_initializing)
	return;

  Axis::TicksDirection direction = Axis::TicksDirection(index);
  foreach(Axis* axis, m_axesList)
	axis->setMinorTicksDirection(direction);  
}

void AxisDock::minorTicksTypeChanged(int index){
  Axis::TicksType type = Axis::TicksType(index);
  if ( type == Axis::TicksTotalNumber){
	  ui.lMinorTicksNumber->setText( i18n("Number") );
	  ui.leMinorTicksNumber->setText( QString::number(m_axesList.first()->minorTicksNumber()) );
  }else{
	  ui.lMinorTicksNumber->setText( i18n("Increment") );
	  ui.leMinorTicksNumber->setText( QString::number(m_axesList.first()->minorTicksIncrement()) );
  }
  
  if (m_initializing)
	return;
		
  foreach(Axis* axis, m_axesList){
	axis->setMinorTicksType(type);
  }
}

void AxisDock::minorTicksNumberChanged(){
  if (m_initializing)
	return;

  double value = ui.leMajorTicksNumber->text().toDouble();
  foreach(Axis* axis, m_axesList){
	axis->setMinorTicksNumber(value);
  }  
}

void AxisDock::minorTicksLineStyleChanged(int index){
   if (m_initializing)
	return;

  Qt::PenStyle penStyle=Qt::PenStyle(index);
  QPen pen;
  foreach(Axis* axis, m_axesList){
	pen=axis->minorTicksPen();
	pen.setStyle(penStyle);
	axis->setMinorTicksPen(pen);
  }
}

void AxisDock::minorTicksColorChanged(const QColor& color){
  if (m_initializing)
	return;

  QPen pen;
  foreach(Axis* axis, m_axesList){
	pen=axis->minorTicksPen();
	pen.setColor(color);
	axis->setMinorTicksPen(pen);
  }  

  GuiTools::updatePenStyles(ui.cbMinorTicksLineStyle, color);
}

void AxisDock::minorTicksWidthChanged(int value){
  if (m_initializing)
	return;
  
  QPen pen;
  foreach(Axis* axis, m_axesList){
	pen=axis->minorTicksPen();
	pen.setWidth(value);
	axis->setMinorTicksPen(pen);
  }  
}

void AxisDock::minorTicksLengthChanged(int value){
  if (m_initializing)
	return;
  
  foreach(Axis* axis, m_axesList){
	axis->setMinorTicksLength(value);
  }  
}

void AxisDock::minorTicksOpacityChanged(int value){
  if (m_initializing)
	return;
		
  qreal opacity = (float)value/100;
  foreach(Axis* axis, m_axesList){
	axis->setMinorTicksOpacity(opacity);
  }
}

//"Tick labels"-tab
