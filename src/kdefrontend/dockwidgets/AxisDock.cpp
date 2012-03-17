/***************************************************************************
    File                 : AxisDock.cc
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2011 Alexander Semke (alexander.semke*web.de)
    Copyright            : (C) 2012 Stefan Gerlach (stefan.gerlach*uni-konstanz.de)
    							(use @ for *)
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
#include "worksheet/plots/cartesian/Axis.h"
#include "kdefrontend/GuiTools.h"
#include "../TemplateHandler.h"
#include "worksheet/Worksheet.h"
#include "kdefrontend/widgets/LabelWidget.h"
#include <KMessageBox>
#include <QTimer>


 /*!
  \class AxisDock
  \brief Provides a widget for editing the properties of the axes currently selected in the project explorer.

  \ingroup kdefrontend
*/
 
AxisDock::AxisDock(QWidget* parent):QWidget(parent){
	ui.setupUi(this);

  //TODO
  ui.lPosition->hide();
  ui.cbPosition->hide();
  
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
	connect( ui.leComment, SIGNAL(returnPressed()), this, SLOT(commentChanged()) );
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
	connect( ui.sbLineWidth, SIGNAL(valueChanged(double)), this, SLOT(lineWidthChanged(double)) );
	connect( ui.sbLineOpacity, SIGNAL(valueChanged(int)), this, SLOT(lineOpacityChanged(int)) );	
	

	//"Title"-tab
	QHBoxLayout* hboxLayout = new QHBoxLayout(ui.tabTitle);
 	labelWidget=new LabelWidget(ui.tabTitle);
	hboxLayout->addWidget(labelWidget);
	connect( labelWidget, SIGNAL(dataChanged(bool)), this, SLOT(titleChanged()) );

	//"Major ticks"-tab
	connect( ui.cbMajorTicksDirection, SIGNAL(currentIndexChanged(int)), this, SLOT(majorTicksDirectionChanged(int)) );
	connect( ui.cbMajorTicksType, SIGNAL(currentIndexChanged(int)), this, SLOT(majorTicksTypeChanged(int)) );
	connect( ui.sbMajorTicksNumber, SIGNAL(valueChanged(int)), this, SLOT(majorTicksNumberChanged(int)) );
 	connect( ui.leMajorTicksIncrement, SIGNAL(returnPressed()), this, SLOT(majorTicksIncrementChanged()) );
	connect( ui.cbMajorTicksLineStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(majorTicksLineStyleChanged(int)) );
	connect( ui.kcbMajorTicksColor, SIGNAL(changed (const QColor &)), this, SLOT(majorTicksColorChanged(const QColor&)) );
	connect( ui.sbMajorTicksWidth, SIGNAL(valueChanged(double)), this, SLOT(majorTicksWidthChanged(double)) );
	connect( ui.sbMajorTicksLength, SIGNAL(valueChanged(double)), this, SLOT(majorTicksLengthChanged(double)) );
	connect( ui.sbMajorTicksOpacity, SIGNAL(valueChanged(int)), this, SLOT(majorTicksOpacityChanged(int)) );

	//"Minor ticks"-tab
	connect( ui.cbMinorTicksDirection, SIGNAL(currentIndexChanged(int)), this, SLOT(minorTicksDirectionChanged(int)) );
	connect( ui.cbMinorTicksType, SIGNAL(currentIndexChanged(int)), this, SLOT(minorTicksTypeChanged(int)) );
	connect( ui.sbMinorTicksNumber, SIGNAL(valueChanged(int)), this, SLOT(minorTicksNumberChanged(int)) );
 	connect( ui.leMinorTicksIncrement, SIGNAL(returnPressed()), this, SLOT(minorTicksIncrementChanged()) );
	connect( ui.cbMinorTicksLineStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(minorTicksLineStyleChanged(int)) );
	connect( ui.kcbMinorTicksColor, SIGNAL(changed (const QColor &)), this, SLOT(minorTicksColorChanged(const QColor&)) );
	connect( ui.sbMinorTicksWidth, SIGNAL(valueChanged(double)), this, SLOT(minorTicksWidthChanged(double)) );
	connect( ui.sbMinorTicksLength, SIGNAL(valueChanged(double)), this, SLOT(minorTicksLengthChanged(double)) );
	connect( ui.sbMinorTicksOpacity, SIGNAL(valueChanged(int)), this, SLOT(minorTicksOpacityChanged(int)) );
	
	//"Extra ticks"-tab
	
	//"Tick labels"-tab
	connect( ui.cbLabelsPosition, SIGNAL(currentIndexChanged(int)), this, SLOT(labelsPositionChanged(int)) );
	connect( ui.sbLabelsXOffset, SIGNAL(valueChanged(double)), this, SLOT(labelsXOffsetChanged(double)) );
	connect( ui.sbLabelsYOffset, SIGNAL(valueChanged(double)), this, SLOT(labelsYOffsetChanged(double)) );
	connect( ui.sbLabelsRotation, SIGNAL(valueChanged(int)), this, SLOT(labelsRotationChanged(int)) );
	connect( ui.kfrLabelsFont, SIGNAL(fontSelected(const QFont& )), this, SLOT(labelsFontChanged(const QFont&)) );
	connect( ui.kcbLabelsFontColor, SIGNAL(changed (const QColor &)), this, SLOT(labelsFontColorChanged(const QColor&)) );
	connect( ui.leLabelsPrefix, SIGNAL(returnPressed()), this, SLOT(labelsPrefixChanged()) );
	connect( ui.leLabelsSuffix, SIGNAL(returnPressed()), this, SLOT(labelsSuffixChanged()) );
	connect( ui.sbLabelsOpacity, SIGNAL(valueChanged(int)), this, SLOT(labelsOpacityChanged(int)) );
	
	/*

	connect( ui.sbLabelsPrecision, SIGNAL(valueChanged(int)), this, SLOT(slotDataChanged()) );
	connect( ui.cbLabelsFormat, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(labelFormatChanged(const QString&)) );
	connect( ui.leLabelsDateFormat, SIGNAL(textChanged(const QString&)), this, SLOT(slotDataChanged()) );

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

//TODO cbMajorTicksDirection is empty when the axes are set
	//QTimer::singleShot(0, this, SLOT(init()));

	TemplateHandler* templateHandler = new TemplateHandler(this, TemplateHandler::Axis);
	ui.verticalLayout->addWidget(templateHandler);
	connect( templateHandler, SIGNAL(loadConfigRequested(KConfig&)), this, SLOT(loadConfig(KConfig&)));
	connect( templateHandler, SIGNAL(saveConfigRequested(KConfig&)), this, SLOT(saveConfig(KConfig&)));

	init();
}

AxisDock::~AxisDock(){}

void AxisDock::init(){
	m_initializing=true;

	//Validators
	ui.lePositionOffset->setValidator( new QDoubleValidator(ui.lePositionOffset) );
	ui.leStart->setValidator( new QDoubleValidator(ui.leStart) );
	ui.leEnd->setValidator( new QDoubleValidator(ui.leEnd) );
	ui.leZeroOffset->setValidator( new QDoubleValidator(ui.leZeroOffset) );
	ui.leScalingFactor->setValidator( new QDoubleValidator(ui.leScalingFactor) );
	
	ui.leMajorTicksIncrement->setValidator( new QDoubleValidator(ui.leMajorTicksIncrement) );
	ui.leMinorTicksIncrement->setValidator( new QDoubleValidator(ui.leMinorTicksIncrement) );

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
	
	//TODO: Tick labels format

	//Grid
	//TODO: remove this later
	 ui.kcbMajorGridColor->setColor(Qt::black);
	 GuiTools::updatePenStyles(ui.cbMajorGridStyle, QColor(Qt::black));
	 ui.kcbMinorGridColor->setColor(Qt::black);
	 GuiTools::updatePenStyles(ui.cbMinorGridStyle, QColor(Qt::black));
	 
	m_initializing=false;
}


/*!
  sets the axes. The properties of the axes in the list \c list can be edited in this widget.
*/
void AxisDock::setAxes(QList<Axis*> list){
  	m_initializing=true;
  	m_axesList=list;
  	Axis* axis=list.first();
  
	labelWidget->setLabel(axis->title());

  //if there are more then one axis in the list, disable the tab "general"
  if (list.size()==1){
	ui.lName->setEnabled(true);
	ui.leName->setEnabled(true);
	ui.lComment->setEnabled(true);
	ui.leComment->setEnabled(true);
	ui.leName->setText(axis->name());
	ui.leComment->setText(axis->comment());
  }else{
	ui.lName->setEnabled(false);
	ui.leName->setEnabled(false);
	ui.lComment->setEnabled(false);
	ui.leComment->setEnabled(false);
	ui.leName->setText("");
	ui.leComment->setText("");	
  }

  	//show the properties of the first axis
	KConfig config("", KConfig::SimpleConfig);
	loadConfig(config);
  
  	m_initializing = false;
}

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
  
  m_axesList.first()->setComment(ui.leComment->text());
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
*/

void AxisDock::orientationChanged(int index){
   Axis::AxisOrientation orientation = (Axis::AxisOrientation)index;
  int oldIndex = ui.cbPosition->currentIndex();
  int oldLabelsIndex = ui.cbLabelsPosition->currentIndex();
  
  ui.cbPosition->clear();
  ui.cbLabelsPosition->clear();
  ui.cbLabelsPosition->addItem(i18n("no labels"));
  if (orientation == Axis::AxisHorizontal){
	ui.cbPosition->addItem( i18n("bottom") );
	ui.cbPosition->addItem( i18n("top") );
	ui.cbLabelsPosition->addItem( i18n("top") );
	ui.cbLabelsPosition->addItem( i18n("bottom") );	
  }else{//vertical
	ui.cbPosition->addItem( i18n("left") );
	ui.cbPosition->addItem( i18n("right") );
	ui.cbLabelsPosition->addItem( i18n("right") );
	ui.cbLabelsPosition->addItem( i18n("left") );
  }
  ui.cbPosition->addItem( i18n("custom") );	  
  ui.cbPosition->setCurrentIndex(oldIndex);
  ui.cbLabelsPosition->setCurrentIndex(oldLabelsIndex);
  
    if (m_initializing)
	  return;
	
  foreach(Axis* axis, m_axesList){
	axis->setOrientation(orientation);
  }
}

//TODO
void AxisDock::positionChanged(int index){
//   if (m_initializing)
// 	return;
// 
// 	double offset;
// 	if (index==1){//under or left
// 	  ofset=0;
// 	  ui.lPositionOffset->hide();
// 	  ui.lePositionOffset->hide();
// 	  
// 	Axis::AxisPosition position;
//   	if (index==2){ //custom position
// 		ui.lPositionOffset->show();
// 		ui.lePositionOffset->show();
// 		position = Axis::AxisCustom;
// 	}else{
// 		ui.lPositionOffset->hide();
// 		ui.lePositionOffset->hide();
// 		
// 		if ( m_axesList->first()->orientation() == Axis::AxisHorizontal)
// 		  postion = Axis::AxisPosition(index);
// 		else
// 		  postion = Axis::AxisPosition(index+2);
// 	}
// 
//   foreach(Axis* axis, m_axesList){
// 	axis->setPosition(position);
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

void AxisDock::zeroOffsetChanged(){
  if (m_initializing)
	return;
  
  double offset = ui.leZeroOffset->text().toDouble();
  foreach(Axis* axis, m_axesList){
	axis->setZeroOffset(offset);
  }
}

void AxisDock::scalingFactorChanged(){
  if (m_initializing)
	return;
  
  double scalingFactor = ui.leScalingFactor->text().toDouble();
  foreach(Axis* axis, m_axesList){
	axis->setScalingFactor(scalingFactor);
  }
}

// "Title"-tab
void AxisDock::titleChanged(){
  if (m_initializing)
	return;
  
	//TODO
}

// "Line"-tab
void AxisDock::lineStyleChanged(int index){
   if (m_initializing)
	return;

  Qt::PenStyle penStyle=Qt::PenStyle(index);

	bool b = (penStyle != Qt::NoPen);
	ui.lLineColor->setEnabled(b);
	ui.kcbLineColor->setEnabled(b);
	ui.lLineWidth->setEnabled(b);
	ui.sbLineWidth->setEnabled(b);
	ui.lLineOpacity->setEnabled(b);
	ui.sbLineOpacity->setEnabled(b);
  
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

  m_initializing=true;
  GuiTools::updatePenStyles(ui.cbLineStyle, color);
  m_initializing=false;
}

void AxisDock::lineWidthChanged(double  value){
  if (m_initializing)
	return;
  
  QPen pen;
  foreach(Axis* axis, m_axesList){
	pen=axis->linePen();
	pen.setWidthF(Worksheet::convertToSceneUnits(value, Worksheet::Point));
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
  
	bool b = (direction != Axis::noTicks);
	ui.lMajorTicksType->setEnabled(b);
	ui.cbMajorTicksType->setEnabled(b);
	ui.lMajorTicksType->setEnabled(b);  
	ui.cbMajorTicksType->setEnabled(b);  
	ui.lMajorTicksNumber->setEnabled(b);
	ui.sbMajorTicksNumber->setEnabled(b);
	ui.lMajorTicksIncrement->setEnabled(b);
	ui.leMajorTicksIncrement->setEnabled(b);
	ui.lMajorTicksLineStyle->setEnabled(b);
	ui.cbMajorTicksLineStyle->setEnabled(b);
	if (b){
		Qt::PenStyle penStyle=Qt::PenStyle(ui.cbMajorTicksLineStyle->currentIndex());
		b = (penStyle != Qt::NoPen);
	}
	ui.lMajorTicksColor->setEnabled(b);
	ui.kcbMajorTicksColor->setEnabled(b);
	ui.lMajorTicksWidth->setEnabled(b);
	ui.sbMajorTicksWidth->setEnabled(b);
	ui.lMajorTicksLength->setEnabled(b);
	ui.sbMajorTicksLength->setEnabled(b);
	ui.lMajorTicksOpacity->setEnabled(b);
	ui.sbMajorTicksOpacity->setEnabled(b);  
  
  foreach(Axis* axis, m_axesList)
	axis->setMajorTicksDirection(direction);
}

/*!
	called if the current style of the ticks (Number or Increment) is changed.
	Shows/hides the corresponding widgets.
*/
void AxisDock::majorTicksTypeChanged(int index){
  Axis::TicksType type = Axis::TicksType(index);
  if ( type == Axis::TicksTotalNumber){
	  ui.lMajorTicksNumber->show();
	  ui.sbMajorTicksNumber->show();
	  ui.lMajorTicksIncrement->hide();
	  ui.leMajorTicksIncrement->hide();	  
  }else{
	  ui.lMajorTicksNumber->hide();
	  ui.sbMajorTicksNumber->hide();
	  ui.lMajorTicksIncrement->show();
	  ui.leMajorTicksIncrement->show();	  
  }
  
  if (m_initializing)
	return;
		
  foreach(Axis* axis, m_axesList){
	axis->setMajorTicksType(type);
  }
}

void AxisDock::majorTicksNumberChanged(int value){
  if (m_initializing)
	return;

  foreach(Axis* axis, m_axesList){
	axis->setMajorTicksNumber(value);
  }
}

void AxisDock::majorTicksIncrementChanged(){
  if (m_initializing)
	return;

  double value = ui.leMajorTicksIncrement->text().toDouble();
  foreach(Axis* axis, m_axesList){
	axis->setMajorTicksIncrement(value);
  }
}

void AxisDock::majorTicksLineStyleChanged(int index){
	if (m_initializing)
	return;

	Qt::PenStyle penStyle=Qt::PenStyle(index);
	
	bool b=(penStyle != Qt::NoPen);
	ui.lMajorTicksColor->setEnabled(b);
	ui.kcbMajorTicksColor->setEnabled(b);
	ui.lMajorTicksWidth->setEnabled(b);
	ui.sbMajorTicksWidth->setEnabled(b);
	ui.lMajorTicksLength->setEnabled(b);
	ui.sbMajorTicksLength->setEnabled(b);
	ui.lMajorTicksOpacity->setEnabled(b);
	ui.sbMajorTicksOpacity->setEnabled(b);

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

  m_initializing=true;
  GuiTools::updatePenStyles(ui.cbMajorTicksLineStyle, color);
  m_initializing=false;
}

void AxisDock::majorTicksWidthChanged(double value){
  if (m_initializing)
	return;
  
  QPen pen;
  foreach(Axis* axis, m_axesList){
	pen=axis->majorTicksPen();
	pen.setWidthF( Worksheet::convertToSceneUnits(value, Worksheet::Point) );
	axis->setMajorTicksPen(pen);
  }  
}

void AxisDock::majorTicksLengthChanged(double value){
  if (m_initializing)
	return;
  
  foreach(Axis* axis, m_axesList){
	axis->setMajorTicksLength( Worksheet::convertToSceneUnits(value, Worksheet::Point) );
  }  
}

void AxisDock::majorTicksOpacityChanged(int value){
  if (m_initializing)
	return;
		
  qreal opacity = (float)value/100;
  foreach(Axis* axis, m_axesList){
	axis->setMajorTicksOpacity(opacity);
  }
}

//"Minor ticks" tab
void AxisDock::minorTicksDirectionChanged(int index){
	if (m_initializing)
	return;

	Axis::TicksDirection direction = Axis::TicksDirection(index);
	bool b = (direction != Axis::noTicks);
	ui.lMinorTicksType->setEnabled(b);
	ui.cbMinorTicksType->setEnabled(b);
	ui.lMinorTicksType->setEnabled(b);  
	ui.cbMinorTicksType->setEnabled(b);  
	ui.lMinorTicksNumber->setEnabled(b);
	ui.sbMinorTicksNumber->setEnabled(b);
	ui.lMinorTicksIncrement->setEnabled(b);
	ui.leMinorTicksIncrement->setEnabled(b);
	ui.lMinorTicksLineStyle->setEnabled(b);
	ui.cbMinorTicksLineStyle->setEnabled(b);
	if (b){
		Qt::PenStyle penStyle=Qt::PenStyle(ui.cbMinorTicksLineStyle->currentIndex());
		b = (penStyle != Qt::NoPen);
	}  	
	ui.lMinorTicksColor->setEnabled(b);
	ui.kcbMinorTicksColor->setEnabled(b);
	ui.lMinorTicksWidth->setEnabled(b);
	ui.sbMinorTicksWidth->setEnabled(b);
	ui.lMinorTicksLength->setEnabled(b);
	ui.sbMinorTicksLength->setEnabled(b);
	ui.lMinorTicksOpacity->setEnabled(b);
	ui.sbMinorTicksOpacity->setEnabled(b);  
  
  foreach(Axis* axis, m_axesList){
	axis->setMinorTicksDirection(direction);  
  }
}

void AxisDock::minorTicksTypeChanged(int index){
  Axis::TicksType type = Axis::TicksType(index);
  if ( type == Axis::TicksTotalNumber){
	ui.lMinorTicksNumber->show();
	ui.sbMinorTicksNumber->show();
	ui.lMinorTicksIncrement->hide();
	ui.leMinorTicksIncrement->hide();
  }else{
	  ui.lMinorTicksNumber->hide();
	  ui.sbMinorTicksNumber->hide();
	  ui.lMinorTicksIncrement->show();
	  ui.leMinorTicksIncrement->show();
  }
  
  if (m_initializing)
	return;
		
  foreach(Axis* axis, m_axesList){
	axis->setMinorTicksType(type);
  }
}

void AxisDock::minorTicksNumberChanged(int value){
  if (m_initializing)
	return;

  foreach(Axis* axis, m_axesList){
	axis->setMinorTicksNumber(value);
  }  
}

void AxisDock::minorTicksIncrementChanged(){
  if (m_initializing)
	return;

  double value = ui.leMinorTicksIncrement->text().toDouble();
  foreach(Axis* axis, m_axesList){
	axis->setMinorTicksIncrement(value);
  }  
}

void AxisDock::minorTicksLineStyleChanged(int index){
	if (m_initializing)
	return;

	Qt::PenStyle penStyle=Qt::PenStyle(index);
  
	bool b=(penStyle != Qt::NoPen);
	ui.lMinorTicksColor->setEnabled(b);
	ui.kcbMinorTicksColor->setEnabled(b);
	ui.lMinorTicksWidth->setEnabled(b);
	ui.sbMinorTicksWidth->setEnabled(b);
	ui.lMinorTicksLength->setEnabled(b);
	ui.sbMinorTicksLength->setEnabled(b);
	ui.lMinorTicksOpacity->setEnabled(b);
	ui.sbMinorTicksOpacity->setEnabled(b);
	
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

  m_initializing=true;
  GuiTools::updatePenStyles(ui.cbMinorTicksLineStyle, color);
  m_initializing=false;
}

void AxisDock::minorTicksWidthChanged(double value){
  if (m_initializing)
	return;
  
  QPen pen;
  foreach(Axis* axis, m_axesList){
	pen=axis->minorTicksPen();
	pen.setWidthF( Worksheet::convertToSceneUnits(value, Worksheet::Point) );
	axis->setMinorTicksPen(pen);
  }  
}

void AxisDock::minorTicksLengthChanged(double value){
  if (m_initializing)
	return;
  
  foreach(Axis* axis, m_axesList){
	axis->setMinorTicksLength( Worksheet::convertToSceneUnits(value, Worksheet::Point) );
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
void AxisDock::labelsPositionChanged(int index){
  if (m_initializing)
	return;
  
  Axis::LabelsPosition position = Axis::LabelsPosition(index);
  
	bool b = (position != Axis::NoLabels);
	ui.lLabelsOffset->setEnabled(b);
	ui.sbLabelsXOffset->setEnabled(b);
	ui.sbLabelsYOffset->setEnabled(b);
	ui.lLabelsRotation->setEnabled(b);
	ui.sbLabelsRotation->setEnabled(b);
	ui.lLabelsFont->setEnabled(b);
	ui.kfrLabelsFont->setEnabled(b);
	ui.lLabelsColor->setEnabled(b);
	ui.kcbLabelsFontColor->setEnabled(b);
	ui.lLabelsPrefix->setEnabled(b);
	ui.leLabelsPrefix->setEnabled(b);
	ui.lLabelsSuffix->setEnabled(b);
	ui.leLabelsSuffix->setEnabled(b);
	ui.lLabelsOpacity->setEnabled(b);
	ui.sbLabelsOpacity->setEnabled(b);  
	
	foreach(Axis* axis, m_axesList){
		axis->setLabelsPosition(position);
	}
}

void AxisDock::labelsXOffsetChanged(double value){
  if (m_initializing)
	return;
	
	double xvalue = Worksheet::convertToSceneUnits(value,Worksheet::Point);
	double yvalue = Worksheet::convertToSceneUnits(ui.sbLabelsYOffset->value(),Worksheet::Point);
	foreach(Axis* axis, m_axesList){
 		axis->setLabelsOffset(QPointF(xvalue,yvalue));
	}
}

void AxisDock::labelsYOffsetChanged(double value){
  if (m_initializing)
	return;
	
	double xvalue = Worksheet::convertToSceneUnits(ui.sbLabelsXOffset->value(),Worksheet::Point);
	double yvalue = Worksheet::convertToSceneUnits(value,Worksheet::Point);
	foreach(Axis* axis, m_axesList){
 		axis->setLabelsOffset(QPointF(xvalue,yvalue));
	}
}


void AxisDock::labelsRotationChanged(int value){
  if (m_initializing)
	return;
	
  foreach(Axis* axis, m_axesList){
	axis->setLabelsRotationAngle(value);
  }
}

void AxisDock::labelsPrefixChanged(){
  if (m_initializing)
	return;
		
  QString prefix = ui.leLabelsPrefix->text();
  foreach(Axis* axis, m_axesList){
	axis->setLabelsPrefix(prefix);
  }
}

void AxisDock::labelsSuffixChanged(){
  if (m_initializing)
	return;
		
  QString suffix = ui.leLabelsSuffix->text();
  foreach(Axis* axis, m_axesList){
	axis->setLabelsSuffix(suffix);
  }
}

void AxisDock::labelsFontChanged(const QFont& font){
  if (m_initializing)
	return;
	
  foreach(Axis* axis, m_axesList){
	axis->setLabelsFont(font);
  }
}

void AxisDock::labelsFontColorChanged(const QColor& color){
  if (m_initializing)
	return;
	
  foreach(Axis* axis, m_axesList){
	axis->setLabelsColor(color);
  }
}

void AxisDock::labelsOpacityChanged(int value){
  if (m_initializing)
	return;
		
  qreal opacity = (float)value/100;
  foreach(Axis* axis, m_axesList){
	axis->setLabelsOpacity(opacity);
  }  
}

/**************************************************/
/********* Settings *******************************/
/**************************************************/

void AxisDock::loadConfig(KConfig& config){
	KConfigGroup group = config.group( "Axis" );

  	Axis* axis=m_axesList.first();

	//General
  	ui.chkVisible->setChecked(group.readEntry("Visible", axis->isVisible()));
	// TODO:see save()
	ui.cbOrientation->setCurrentIndex( group.readEntry("Orientation", (int) axis->orientation()) );
	// TODO	ui.cbPosition->setCurrentIndex( axis->position() );
  	ui.lePositionOffset->setText( QString::number( group.readEntry("PositionOffset", axis->offset())) );
	ui.cbScale->setCurrentIndex( group.readEntry("Scale", (int) axis->scale()) );
  	ui.leStart->setText( QString::number( group.readEntry("Start", axis->start())) );
  	ui.leEnd->setText( QString::number( group.readEntry("End", axis->end())) );
  	ui.leZeroOffset->setText( QString::number( group.readEntry("ZeroOffset", axis->zeroOffset())) );
  	ui.leScalingFactor->setText( QString::number( group.readEntry("ScalingFactor", axis->scalingFactor())) );

	//Title
	labelWidget->loadConfig(group);	

	//Line
	ui.cbLineStyle->setCurrentIndex( group.readEntry("LineStyle", (int) axis->linePen().style()) );
	ui.kcbLineColor->setColor( group.readEntry("LineColor", axis->linePen().color()) );
	GuiTools::updatePenStyles(ui.cbLineStyle, group.readEntry("LineColor", axis->linePen().color()) );
	ui.sbLineWidth->setValue( Worksheet::convertFromSceneUnits(group.readEntry("LineWidth", axis->linePen().widthF()),Worksheet::Point) );
	ui.sbLineOpacity->setValue( group.readEntry("LineOpacity", axis->lineOpacity())*100 );

	//Major ticks
	ui.cbMajorTicksDirection->setCurrentIndex( group.readEntry("MajorTicksDirection", (int) axis->majorTicksDirection()) );
	ui.cbMajorTicksType->setCurrentIndex( group.readEntry("MajorTicksType", (int) axis->majorTicksType()) );
	this->majorTicksTypeChanged( group.readEntry("MajorTicksType", (int) axis->majorTicksType()) );
	ui.sbMajorTicksNumber->setValue( group.readEntry("MajorTicksNumber", axis->majorTicksNumber()) );
  	ui.leMajorTicksIncrement->setText( QString::number( group.readEntry("MajorTicksIncrement", axis->majorTicksIncrement())) );
	ui.cbMajorTicksLineStyle->setCurrentIndex( group.readEntry("MajorTicksLineStyle", (int) axis->majorTicksPen().style()) );
	ui.kcbMajorTicksColor->setColor( group.readEntry("MajorTicksColor", axis->majorTicksPen().color()) );
	ui.sbMajorTicksWidth->setValue( Worksheet::convertFromSceneUnits(group.readEntry("MajorTicksWidth", axis->majorTicksPen().widthF()),Worksheet::Point) );
	ui.sbMajorTicksLength->setValue( Worksheet::convertFromSceneUnits(group.readEntry("MajorTicksLength", axis->majorTicksLength()),Worksheet::Point) );
	ui.sbMajorTicksOpacity->setValue( group.readEntry("MajorTicksOpacity", axis->majorTicksOpacity())*100 );
	GuiTools::updatePenStyles(ui.cbMajorTicksLineStyle, group.readEntry("MajorTicksColor", axis->majorTicksPen().color()) );
	
	//Minor ticks
	ui.cbMinorTicksDirection->setCurrentIndex( group.readEntry("MinorTicksDirection", (int) axis->minorTicksDirection()) );
	ui.cbMinorTicksType->setCurrentIndex( group.readEntry("MinorTicksType", (int) axis->minorTicksType()) );
	this->minorTicksTypeChanged( group.readEntry("MinorTicksType", (int) axis->minorTicksType()) );
	ui.sbMinorTicksNumber->setValue( group.readEntry("MinorTicksNumber", axis->minorTicksNumber()) );
  	ui.leMinorTicksIncrement->setText( QString::number( group.readEntry("MinorTicksIncrement", axis->minorTicksIncrement())) );
	ui.cbMinorTicksLineStyle->setCurrentIndex( group.readEntry("MinorTicksLineStyle", (int) axis->minorTicksPen().style()) );
	ui.kcbMinorTicksColor->setColor( group.readEntry("MinorTicksColor", axis->minorTicksPen().color()) );
	ui.sbMinorTicksWidth->setValue( Worksheet::convertFromSceneUnits(group.readEntry("MinorTicksWidth", axis->minorTicksPen().widthF()),Worksheet::Point) );
	ui.sbMinorTicksLength->setValue( Worksheet::convertFromSceneUnits(group.readEntry("MinorTicksLength", axis->minorTicksLength()),Worksheet::Point) );
	ui.sbMinorTicksOpacity->setValue( group.readEntry("MinorTicksOpacity", axis->minorTicksOpacity())*100 );
	GuiTools::updatePenStyles(ui.cbMinorTicksLineStyle, group.readEntry("MinorTicksColor", axis->minorTicksPen().color()) );

	//Extra ticks
	//TODO

	// Tick label
	ui.cbLabelsPosition->setCurrentIndex( group.readEntry("LabelsPosition", (int) axis->labelsPosition()) );
	ui.sbLabelsXOffset->setValue( Worksheet::convertFromSceneUnits(group.readEntry("LabelsXOffset", axis->labelsOffset().x()),Worksheet::Point) );
	ui.sbLabelsYOffset->setValue( Worksheet::convertFromSceneUnits(group.readEntry("LabelsYOffset", axis->labelsOffset().y()),Worksheet::Point) );
	ui.sbLabelsRotation->setValue( group.readEntry("LabelsRotation", axis->labelsRotationAngle()) );
	ui.kfrLabelsFont->setFont( group.readEntry("LabelsFont", axis->labelsFont()) );
	ui.kcbLabelsFontColor->setColor( group.readEntry("LabelsFontColor", axis->labelsColor()) );
	ui.leLabelsPrefix->setText( group.readEntry("LabelsPrefix", axis->labelsPrefix()) );
	ui.leLabelsSuffix->setText( group.readEntry("LabelsSuffix", axis->labelsSuffix()) );
	ui.sbLabelsOpacity->setValue( group.readEntry("LabelsOpacity", axis->labelsOpacity())*100 );

	//Grid TODO
// 	ui.chbMajorGridVisible->setChecked( axis->hasMajorGrid() );
// 	ui.cbMajorGridStyle->setCurrentIndex( axis->majorGridStyle()-1 );
// 	ui.kcbMajorGridColor->setColor( axis->majorGridColor() );
// 	ui.sbMajorGridWidth->setValue( axis->majorGridWidth() );
// 	sbMajorGridOpacity->setValue( axis->majorGridOpacity() );
// 
// 	ui.chbMinorGrid->setChecked( axis->hasMinorGrid() );
// 	ui.cbMinorGridStyle->setCurrentIndex( axis->minorGridStyle()-1 );
// 	ui.kcbMinorGridColor->setColor( axis->minorGridColor() );
// 	ui.sbMinorGridWidth->setValue( axis->minorGridWidth() );
// 	sbMinroGridOpacity->setValue( axis->minorGridOpacity() );
}

void AxisDock::saveConfig(KConfig& config){
	KConfigGroup group = config.group( "Axis" );

	//General
	group.writeEntry("Visible", ui.chkVisible->isChecked());
	// TODO:not sure if this should be saved
	group.writeEntry("Orientation", ui.cbOrientation->currentIndex());
	//ui.cbPosition
	group.writeEntry("PositionOffset", ui.lePositionOffset->text());
	group.writeEntry("Scale", ui.cbScale->currentIndex());
	group.writeEntry("Start", ui.leStart->text());
	group.writeEntry("End", ui.leEnd->text());
	group.writeEntry("ZeroOffset", ui.leZeroOffset->text());
	group.writeEntry("ScalingFactor", ui.leScalingFactor->text());

	//Title
	labelWidget->saveConfig(group);	

	//Line
	group.writeEntry("LineStyle", ui.cbLineStyle->currentIndex());
	group.writeEntry("LineColor", ui.kcbLineColor->color());
	group.writeEntry("LineWidth", Worksheet::convertToSceneUnits(ui.sbLineWidth->value(), Worksheet::Point));
	group.writeEntry("LineOpacity", ui.sbLineOpacity->value()/100);

	//Major ticks
	group.writeEntry("MajorTicksDirection", ui.cbMajorTicksDirection->currentIndex());
	group.writeEntry("MajorTicksType", ui.cbMajorTicksType->currentIndex());
	group.writeEntry("MajorTicksNumber", ui.sbMajorTicksNumber->value());
	group.writeEntry("MajorTicksIncrement", ui.leMajorTicksIncrement->text());
	group.writeEntry("MajorTicksLineStyle", ui.cbMajorTicksLineStyle->currentIndex());
	group.writeEntry("MajorTicksColor", ui.kcbMajorTicksColor->color());
	group.writeEntry("MajorTicksWidth", Worksheet::convertToSceneUnits(ui.sbMajorTicksWidth->value(),Worksheet::Point));
	group.writeEntry("MajorTicksLength", Worksheet::convertToSceneUnits(ui.sbMajorTicksLength->value(),Worksheet::Point));
	group.writeEntry("MajorTicksOpacity", ui.sbMajorTicksOpacity->value()/100);

	//Minor ticks
	group.writeEntry("MinorTicksDirection", ui.cbMinorTicksDirection->currentIndex());
	group.writeEntry("MinorTicksType", ui.cbMinorTicksType->currentIndex());
	group.writeEntry("MinorTicksNumber", ui.sbMinorTicksNumber->value());
	group.writeEntry("MinorTicksIncrement", ui.leMinorTicksIncrement->text());
	group.writeEntry("MinorTicksLineStyle", ui.cbMinorTicksLineStyle->currentIndex());
	group.writeEntry("MinorTicksColor", ui.kcbMinorTicksColor->color());
	group.writeEntry("MinorTicksWidth", Worksheet::convertFromSceneUnits(ui.sbMinorTicksWidth->value(),Worksheet::Point));
	group.writeEntry("MinorTicksLength", Worksheet::convertFromSceneUnits(ui.sbMinorTicksLength->value(),Worksheet::Point));
	group.writeEntry("MinorTicksOpacity", ui.sbMinorTicksOpacity->value()/100);

	//Extra ticks
	// TODO

	// Tick label
	group.writeEntry("LabelsPosition", ui.cbLabelsPosition->currentIndex());
	group.writeEntry("LabelsXOffset", Worksheet::convertToSceneUnits(ui.sbLabelsXOffset->value(),Worksheet::Point));
	group.writeEntry("LabelsYOffset", Worksheet::convertToSceneUnits(ui.sbLabelsYOffset->value(),Worksheet::Point));
	group.writeEntry("LabelsRotation", ui.sbLabelsRotation->value());
	group.writeEntry("LabelsFont", ui.kfrLabelsFont->font());
	group.writeEntry("LabelsFontColor", ui.kcbLabelsFontColor->color());
	group.writeEntry("LabelsPrefix", ui.leLabelsPrefix->text());
	group.writeEntry("LabelsSuffix", ui.leLabelsSuffix->text());
	group.writeEntry("LabelsOpacity", ui.sbLabelsOpacity->value()/100);

	//Grid
	// TODO (see load())	

	config.sync();
}

