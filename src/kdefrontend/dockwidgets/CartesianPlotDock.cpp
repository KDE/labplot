/***************************************************************************
    File                 : CartesianPlotDock.cpp
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2011 by Alexander Semke (alexander.semke*web.de)
    Copyright            : (C) 2012-2013 by Stefan Gerlach (stefan.gerlach*uni-konstanz.de)
    							(use @ for *)
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
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/PlotArea.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "kdefrontend/widgets/LabelWidget.h"
#include "kdefrontend/GuiTools.h"
#include "kdefrontend/TemplateHandler.h"

#include <QTimer>
#include <KUrlCompletion>

/*!
  \class CartesianPlotDock
  \brief  Provides a widget for editing the properties of the cartesian plot currently selected in the project explorer.

  \ingroup kdefrontend
*/

CartesianPlotDock::CartesianPlotDock(QWidget *parent): QWidget(parent),
	m_initializing(false){

	ui.setupUi(this);
	
	//"Coordinate system"-tab
	ui.bAddXBreak->setIcon( KIcon("list-add") );
	ui.bRemoveXBreak->setIcon( KIcon("list-remove") );
	ui.cbXBreakNumber->addItem("1");

	ui.bAddYBreak->setIcon( KIcon("list-add") );
	ui.bRemoveYBreak->setIcon( KIcon("list-remove") );
	ui.cbYBreakNumber->addItem("1");

	//"Background"-tab
	ui.kleBackgroundFileName->setClearButtonShown(true);
	ui.bOpen->setIcon( KIcon("document-open") );

	KUrlCompletion *comp = new KUrlCompletion();
	ui.kleBackgroundFileName->setCompletionObject(comp);

	//"Title"-tab
	QHBoxLayout* hboxLayout = new QHBoxLayout(ui.tabTitle);
 	labelWidget=new LabelWidget(ui.tabTitle);
	hboxLayout->addWidget(labelWidget);

	//adjust layouts in the tabs
	for (int i=0; i<ui.tabWidget->count(); ++i){
		QGridLayout* layout = static_cast<QGridLayout*>(ui.tabWidget->widget(i)->layout());
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
	connect( ui.sbLeft, SIGNAL(valueChanged(double)), this, SLOT(geometryChanged()) );
	connect( ui.sbTop, SIGNAL(valueChanged(double)), this, SLOT(geometryChanged()) );
	connect( ui.sbWidth, SIGNAL(valueChanged(double)), this, SLOT(geometryChanged()) );
	connect( ui.sbHeight, SIGNAL(valueChanged(double)), this, SLOT(geometryChanged()) );
	
	connect( ui.chkAutoScaleX, SIGNAL(stateChanged(int)), this, SLOT(autoScaleXChanged(int)) );
	connect( ui.kleXMin, SIGNAL(returnPressed()), this, SLOT(xMinChanged()) );
	connect( ui.kleXMax, SIGNAL(returnPressed()), this, SLOT(xMaxChanged()) );
	connect( ui.cbXScaling, SIGNAL(currentIndexChanged(int)), this, SLOT(xScaleChanged(int)) );

	connect( ui.chkAutoScaleY, SIGNAL(stateChanged(int)), this, SLOT(autoScaleYChanged(int)) );
	connect( ui.kleYMin, SIGNAL(returnPressed()), this, SLOT(yMinChanged()) );
	connect( ui.kleYMax, SIGNAL(returnPressed()), this, SLOT(yMaxChanged()) );
	connect( ui.cbYScaling, SIGNAL(currentIndexChanged(int)), this, SLOT(yScaleChanged(int)) );

	//Scale breakings
	connect( ui.chkXBreak, SIGNAL(stateChanged(int)), this, SLOT(toggleXBreak(int)) );
	connect( ui.chkYBreak, SIGNAL(stateChanged(int)), this, SLOT(toggleYBreak(int)) );
	
	//Background
	connect( ui.cbBackgroundType, SIGNAL(currentIndexChanged(int)), this, SLOT(backgroundTypeChanged(int)) );
	connect( ui.cbBackgroundColorStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(backgroundColorStyleChanged(int)) );
	connect( ui.cbBackgroundImageStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(backgroundImageStyleChanged(int)) );
	connect( ui.cbBackgroundBrushStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(backgroundBrushStyleChanged(int)) );
	connect(ui.bOpen, SIGNAL(clicked(bool)), this, SLOT(selectFile()));
	connect( ui.kleBackgroundFileName, SIGNAL(returnPressed()), this, SLOT(fileNameChanged()) );
	connect( ui.kleBackgroundFileName, SIGNAL(clearButtonClicked()), this, SLOT(fileNameChanged()) );
	connect( ui.kcbBackgroundFirstColor, SIGNAL(changed (const QColor &)), this, SLOT(backgroundFirstColorChanged(const QColor&)) );
	connect( ui.kcbBackgroundSecondColor, SIGNAL(changed (const QColor &)), this, SLOT(backgroundSecondColorChanged(const QColor&)) );
	connect( ui.sbBackgroundOpacity, SIGNAL(valueChanged(int)), this, SLOT(backgroundOpacityChanged(int)) );
	
	//Border 
	connect( ui.cbBorderStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(borderStyleChanged(int)) );
	connect( ui.kcbBorderColor, SIGNAL(changed (const QColor &)), this, SLOT(borderColorChanged(const QColor&)) );
	connect( ui.sbBorderWidth, SIGNAL(valueChanged(double)), this, SLOT(borderWidthChanged(double)) );
	connect( ui.sbBorderOpacity, SIGNAL(valueChanged(int)), this, SLOT(borderOpacityChanged(int)) );

	//Padding
	connect( ui.sbPaddingHorizontal, SIGNAL(valueChanged(double)), this, SLOT(horizontalPaddingChanged(double)) );
	connect( ui.sbPaddingVertical, SIGNAL(valueChanged(double)), this, SLOT(verticalPaddingChanged(double)) );
	
	TemplateHandler* templateHandler = new TemplateHandler(this, TemplateHandler::CartesianPlot);
	ui.verticalLayout->addWidget(templateHandler);
	templateHandler->show();
	connect( templateHandler, SIGNAL(loadConfigRequested(KConfig&)), this, SLOT(loadConfig(KConfig&)));
	connect( templateHandler, SIGNAL(saveConfigRequested(KConfig&)), this, SLOT(saveConfig(KConfig&)));

	//TODO: activate the tab for scale breakings later again
	ui.tabWidget->removeTab(2);

	init();
}

void CartesianPlotDock::init(){
	this->retranslateUi();
	this->toggleXBreak(Qt::Unchecked);
	this->toggleYBreak(Qt::Unchecked);
}

void CartesianPlotDock::setPlots(QList<CartesianPlot*> list){
	m_initializing = true;
	m_plotList = list;

	m_plot=list.first();
  
	QList<TextLabel*> labels;
	foreach(CartesianPlot* plot, list)
		labels.append(plot->title());
	
	labelWidget->setLabels(labels);
	
  //if there is more then one curve in the list, disable the tab "general"
  if (list.size()==1){
	ui.lName->setEnabled(true);
	ui.leName->setEnabled(true);
	ui.lComment->setEnabled(true);
	ui.leComment->setEnabled(true);

	ui.leName->setText(m_plot->name());
	ui.leComment->setText(m_plot->comment());
  }else{
	ui.lName->setEnabled(false);
	ui.leName->setEnabled(false);
	ui.lComment->setEnabled(false);
	ui.leComment->setEnabled(false);

	ui.leName->setText("");
	ui.leComment->setText("");
  }

	//show the properties of the first curve
	KConfig config("", KConfig::SimpleConfig);
  	loadConfig(config);
	//update active widgets
	backgroundTypeChanged(ui.cbBackgroundType->currentIndex());

	//Deactivate the geometry related widgets, if the worksheet layout is active.
	//Currently, a plot can only be a child of the worksheet itself, so we only need to ask the parent aspect (=worksheet).
	//TODO redesign this, if the hierarchy will be changend in future (a plot is a child of a new object group/container or so)
	Worksheet* w = dynamic_cast<Worksheet*>(m_plot->parentAspect());
	if (w){
		bool b = (w->layout()==Worksheet::NoLayout);
		ui.sbTop->setEnabled(b);
		ui.sbLeft->setEnabled(b);
		ui.sbWidth->setEnabled(b);
		ui.sbHeight->setEnabled(b);
		connect(w, SIGNAL(layoutChanged(Worksheet::Layout)), this, SLOT(layoutChanged(Worksheet::Layout)));
	}

	//SIGNALs/SLOTs
	connect( m_plot, SIGNAL(aspectDescriptionChanged(const AbstractAspect*)), this, SLOT(plotDescriptionChanged(const AbstractAspect*)) );
	connect( m_plot, SIGNAL(rectChanged(QRectF&)), this, SLOT(plotRectChanged(QRectF&)) );
	connect( m_plot, SIGNAL(xMinChanged(float)), this, SLOT(plotXMinChanged(float)) );
	connect( m_plot, SIGNAL(xMaxChanged(float)), this, SLOT(plotXMaxChanged(float)) );
	connect( m_plot, SIGNAL(xScaleChanged(int)), this, SLOT(plotXScaleChanged(int)) );
	connect( m_plot, SIGNAL(yMinChanged(float)), this, SLOT(plotYMinChanged(float)) );
	connect( m_plot, SIGNAL(yMaxChanged(float)), this, SLOT(plotYMaxChanged(float)) );
	connect( m_plot, SIGNAL(yScaleChanged(int)), this, SLOT(plotYScaleChanged(int)) );
	connect( m_plot, SIGNAL(visibleChanged(bool)), this, SLOT(plotVisibleChanged(bool)) );
	//TODO: undo scale braking

	// Plot Area
	connect( m_plot->plotArea(), SIGNAL(backgroundTypeChanged(PlotArea::BackgroundType)), this, SLOT(plotBackgroundTypeChanged(PlotArea::BackgroundType)) );
	connect( m_plot->plotArea(), SIGNAL(backgroundColorStyleChanged(PlotArea::BackgroundColorStyle)), this, SLOT(plotBackgroundColorStyleChanged(PlotArea::BackgroundColorStyle)) );
	connect( m_plot->plotArea(), SIGNAL(backgroundImageStyleChanged(PlotArea::BackgroundImageStyle)), this, SLOT(plotBackgroundImageStyleChanged(PlotArea::BackgroundImageStyle)) );
	connect( m_plot->plotArea(), SIGNAL(backgroundBrushStyleChanged(Qt::BrushStyle)), this, SLOT(plotBackgroundBrushStyleChanged(Qt::BrushStyle)) );
	connect( m_plot->plotArea(), SIGNAL(backgroundFirstColorChanged(QColor&)), this, SLOT(plotBackgroundFirstColorChanged(QColor&)) );
	connect( m_plot->plotArea(), SIGNAL(backgroundSecondColorChanged(QColor&)), this, SLOT(plotBackgroundSecondColorChanged(QColor&)) );
	connect( m_plot->plotArea(), SIGNAL(backgroundFileNameChanged(QString&)), this, SLOT(plotBackgroundFileNameChanged(QString&)) );
	connect( m_plot->plotArea(), SIGNAL(backgroundOpacityChanged(float)), this, SLOT(plotBackgroundOpacityChanged(float)) );
	connect( m_plot->plotArea(), SIGNAL(borderPenChanged(QPen&)), this, SLOT(plotBorderPenChanged(QPen&)) );
	connect( m_plot->plotArea(), SIGNAL(borderOpacityChanged(float)), this, SLOT(plotBorderOpacityChanged(float)) );
	connect( m_plot, SIGNAL(horizontalPaddingChanged(float)), this, SLOT(plotHorizontalPaddingChanged(float)) );
	connect( m_plot, SIGNAL(verticalPaddingChanged(float)), this, SLOT(plotVerticalPaddingChanged(float)) );
	
	m_initializing = false;
}

void CartesianPlotDock::activateTitleTab(){
	ui.tabWidget->setCurrentWidget(ui.tabTitle);
}

//************************************************************
//**** SLOTs for changes triggered in CartesianPlotDock ******
//************************************************************
void CartesianPlotDock::retranslateUi(){
	m_initializing = true;

	ui.cbXScaling->addItem( i18n("linear") );
	ui.cbXScaling->addItem( i18n("log(x)") );
	ui.cbXScaling->addItem( i18n("log2(x)") );
	ui.cbXScaling->addItem( i18n("ln(x)") );

	ui.cbYScaling->addItem( i18n("linear") );
	ui.cbYScaling->addItem( i18n("log(y)") );
	ui.cbYScaling->addItem( i18n("log2(y)") );
	ui.cbYScaling->addItem( i18n("ln(y)") );

	ui.cbBackgroundType->addItem(i18n("color"));
	ui.cbBackgroundType->addItem(i18n("image"));
	ui.cbBackgroundType->addItem(i18n("pattern"));

	ui.cbBackgroundColorStyle->addItem(i18n("single color"));
	ui.cbBackgroundColorStyle->addItem(i18n("horizontal linear gradient"));
	ui.cbBackgroundColorStyle->addItem(i18n("vertical linear gradient"));
	ui.cbBackgroundColorStyle->addItem(i18n("diagonal linear gradient (start from top left)"));
	ui.cbBackgroundColorStyle->addItem(i18n("diagonal linear gradient (start from bottom left)"));
	ui.cbBackgroundColorStyle->addItem(i18n("radial gradient"));

	ui.cbBackgroundImageStyle->addItem(i18n("scaled and cropped"));
	ui.cbBackgroundImageStyle->addItem(i18n("scaled"));
	ui.cbBackgroundImageStyle->addItem(i18n("scaled, keep proportions"));
	ui.cbBackgroundImageStyle->addItem(i18n("centered"));
	ui.cbBackgroundImageStyle->addItem(i18n("tiled"));
	ui.cbBackgroundImageStyle->addItem(i18n("center tiled"));

	GuiTools::updatePenStyles(ui.cbBorderStyle, Qt::black);
	GuiTools::updateBrushStyles(ui.cbBackgroundBrushStyle, Qt::SolidPattern);

	m_initializing = false;
}

// "General"-tab
void CartesianPlotDock::nameChanged(){
  if (m_initializing)
	return;
  
  m_plot->setName(ui.leName->text());
}

void CartesianPlotDock::commentChanged(){
  if (m_initializing)
	return;
  
  m_plot->setComment(ui.leComment->text());
}

void CartesianPlotDock::visibilityChanged(int state){
  if (m_initializing)
	return;
  
  bool b = (state==Qt::Checked);
  foreach(CartesianPlot* plot, m_plotList){
	plot->setVisible(b);
  }
}

void CartesianPlotDock::geometryChanged(){
	if (m_initializing)
		return;

	float x = Worksheet::convertToSceneUnits(ui.sbLeft->value(), Worksheet::Centimeter);
	float y = Worksheet::convertToSceneUnits(ui.sbTop->value(), Worksheet::Centimeter);
	float w = Worksheet::convertToSceneUnits(ui.sbWidth->value(), Worksheet::Centimeter);
	float h = Worksheet::convertToSceneUnits(ui.sbHeight->value(), Worksheet::Centimeter);

	QRectF rect(x,y,w,h);
	m_plot->setRect(rect);
}

/*!
	Called when the layout in the worksheet gets changed.
	Enables/disables the geometry widgets if the layout was deactivated/activated.
	Shows the new geometry values of the first plot if the layout was activated.
 */
void CartesianPlotDock::layoutChanged(Worksheet::Layout layout){
	bool b = (layout == Worksheet::NoLayout);
	ui.sbTop->setEnabled(b);
	ui.sbLeft->setEnabled(b);
	ui.sbWidth->setEnabled(b);
	ui.sbHeight->setEnabled(b);
	if (!b){
		m_initializing = true;
		ui.sbLeft->setValue(Worksheet::convertFromSceneUnits(m_plot->rect().x(), Worksheet::Centimeter));
		ui.sbTop->setValue(Worksheet::convertFromSceneUnits(m_plot->rect().y(), Worksheet::Centimeter));
		ui.sbWidth->setValue(Worksheet::convertFromSceneUnits(m_plot->rect().width(), Worksheet::Centimeter));
		ui.sbHeight->setValue(Worksheet::convertFromSceneUnits(m_plot->rect().height(), Worksheet::Centimeter));		
		m_initializing = false;
	}
}


void CartesianPlotDock::autoScaleXChanged(int state){
	bool checked = (state==Qt::Checked);
	ui.kleXMin->setEnabled(!checked);
	ui.kleXMax->setEnabled(!checked);

	if (m_initializing)
		return;

	foreach(CartesianPlot* plot, m_plotList)
		plot->setAutoScaleX(checked);
}

void CartesianPlotDock::xMinChanged(){
	if (m_initializing)
		return;

	float value = ui.kleXMin->text().toDouble();
	foreach(CartesianPlot* plot, m_plotList)
		plot->setXMin(value);
}

void CartesianPlotDock::xMaxChanged(){
	if (m_initializing)
		return;

	float value = ui.kleXMax->text().toDouble();
	foreach(CartesianPlot* plot, m_plotList)
		plot->setXMax(value);
}

/*!
	called on scale changes (linear, log) for the x-axis
 */
void CartesianPlotDock::xScaleChanged(int scale){
  if (m_initializing)
	return;

  foreach(CartesianPlot* plot, m_plotList)
	plot->setXScale((CartesianPlot::Scale) scale);
}

void CartesianPlotDock::autoScaleYChanged(int state){
	bool checked = (state==Qt::Checked);
	ui.kleYMin->setEnabled(!checked);
	ui.kleYMax->setEnabled(!checked);

	if (m_initializing)
		return;

	foreach(CartesianPlot* plot, m_plotList)
		plot->setAutoScaleY(checked);
}

void CartesianPlotDock::yMinChanged(){
	if (m_initializing)
		return;

	float value = ui.kleYMin->text().toDouble();
	foreach(CartesianPlot* plot, m_plotList)
		plot->setYMin(value);
}

void CartesianPlotDock::yMaxChanged(){
	if (m_initializing)
		return;

	float value = ui.kleYMax->text().toDouble();
	foreach(CartesianPlot* plot, m_plotList)
		plot->setYMax(value);
}

/*!
	called on scale changes (linear, log) for the y-axis
 */
void CartesianPlotDock::yScaleChanged(int index){
  if (m_initializing)
	return;

  CartesianPlot::Scale scale = (CartesianPlot::Scale)index;
  foreach(CartesianPlot* plot, m_plotList)
	plot->setYScale(scale);
}

// "Scale Breakings"-tab
void CartesianPlotDock::toggleXBreak(int state){
	if (m_initializing)
		return;
  
	bool b = (state==Qt::Checked);
	ui.frameXBreakEdit->setVisible(b);
	ui.lXBreakStart->setVisible(b);
	ui.leXBreakStart->setVisible(b);
	ui.lXBreakEnd->setVisible(b);
	ui.leXBreakEnd->setVisible(b);
    ui.lXBreakPosition->setVisible(b);
	ui.sbXBreakPosition->setVisible(b);
    ui.lXBreakStyle->setVisible(b);
	ui.cbXBreakStyle->setVisible(b);
}

void CartesianPlotDock::toggleYBreak(int state){
	if (m_initializing)
		return;
  
	bool b = (state==Qt::Checked);
	ui.frameYBreakEdit->setVisible(b);
	ui.lYBreakStart->setVisible(b);
	ui.leYBreakStart->setVisible(b);
	ui.lYBreakEnd->setVisible(b);
	ui.leYBreakEnd->setVisible(b);
    ui.lYBreakPosition->setVisible(b);
	ui.sbYBreakPosition->setVisible(b);
    ui.lYBreakStyle->setVisible(b);
	ui.cbYBreakStyle->setVisible(b);
}

// "Plot area"-tab
void CartesianPlotDock::backgroundTypeChanged(int index){
	PlotArea::BackgroundType type = (PlotArea::BackgroundType)index;

	if (type == PlotArea::Color){
		ui.lBackgroundColorStyle->show();
		ui.cbBackgroundColorStyle->show();
		ui.lBackgroundImageStyle->hide();
		ui.cbBackgroundImageStyle->hide();
		ui.lBackgroundBrushStyle->hide();
		ui.cbBackgroundBrushStyle->hide();

		ui.lBackgroundFileName->hide();
		ui.kleBackgroundFileName->hide();
		ui.bOpen->hide();

		ui.lBackgroundFirstColor->show();
		ui.kcbBackgroundFirstColor->show();

		PlotArea::BackgroundColorStyle style = 
			(PlotArea::BackgroundColorStyle) ui.cbBackgroundColorStyle->currentIndex();
		if (style == PlotArea::SingleColor){
			ui.lBackgroundFirstColor->setText("Color");
			ui.lBackgroundSecondColor->hide();
			ui.kcbBackgroundSecondColor->hide();
		}else{
			ui.lBackgroundFirstColor->setText("First Color");
			ui.lBackgroundSecondColor->show();
			ui.kcbBackgroundSecondColor->show();
		}
	}else if(type == PlotArea::Image){
		ui.lBackgroundColorStyle->hide();
		ui.cbBackgroundColorStyle->hide();
		ui.lBackgroundImageStyle->show();
		ui.cbBackgroundImageStyle->show();
		ui.lBackgroundBrushStyle->hide();
		ui.cbBackgroundBrushStyle->hide();
		ui.lBackgroundFileName->show();
		ui.kleBackgroundFileName->show();
		ui.bOpen->show();

		ui.lBackgroundFirstColor->hide();
		ui.kcbBackgroundFirstColor->hide();
		ui.lBackgroundSecondColor->hide();
		ui.kcbBackgroundSecondColor->hide();
	}else if(type == PlotArea::Pattern) {
		ui.lBackgroundFirstColor->setText("Color");
		ui.lBackgroundColorStyle->hide();
		ui.cbBackgroundColorStyle->hide();
		ui.lBackgroundImageStyle->hide();
		ui.cbBackgroundImageStyle->hide();
		ui.lBackgroundBrushStyle->show();
		ui.cbBackgroundBrushStyle->show();
		ui.lBackgroundFileName->hide();
		ui.kleBackgroundFileName->hide();
		ui.bOpen->hide();

		ui.lBackgroundFirstColor->show();
		ui.kcbBackgroundFirstColor->show();
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
		ui.lBackgroundFirstColor->setText("Color");
		ui.lBackgroundSecondColor->hide();
		ui.kcbBackgroundSecondColor->hide();
		ui.lBackgroundBrushStyle->show();
		ui.cbBackgroundBrushStyle->show();
	}else{
		ui.lBackgroundFirstColor->setText("First Color");
		ui.lBackgroundSecondColor->show();
		ui.kcbBackgroundSecondColor->show();
		ui.lBackgroundBrushStyle->hide();
		ui.cbBackgroundBrushStyle->hide();
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

void CartesianPlotDock::backgroundBrushStyleChanged(int index){
	if (m_initializing)
		return;

	Qt::BrushStyle style = (Qt::BrushStyle)index;
	foreach(CartesianPlot* plot, m_plotList){
		plot->plotArea()->setBackgroundBrushStyle(style);
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
	KConfigGroup conf(KSharedConfig::openConfig(), "CartesianPlotDock");
	QString dir = conf.readEntry("LastImageDir", "");
    QString path = QFileDialog::getOpenFileName(this, i18n("Select the image file"), dir);
    if (path=="")
        return; //cancel was clicked in the file-dialog

	int pos = path.lastIndexOf(QDir::separator());
	if (pos!=-1) {
		QString newDir = path.left(pos);
		if (newDir!=dir)
			conf.writeEntry("LastImageDir", newDir);
	}
	
    ui.kleBackgroundFileName->setText( path );

	foreach(CartesianPlot* plot, m_plotList)
		plot->plotArea()->setBackgroundFileName(path);
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

	qreal opacity = (float)value/100.;
	foreach(CartesianPlot* plot, m_plotList)
		plot->plotArea()->setBackgroundOpacity(opacity);
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

  m_initializing=true;
  GuiTools::updatePenStyles(ui.cbBorderStyle, color);
  m_initializing=false;
}

void CartesianPlotDock::borderWidthChanged(double value){
  if (m_initializing)
	return;
  
  QPen pen;
  foreach(CartesianPlot* plot, m_plotList){
	pen=plot->plotArea()->borderPen();
	pen.setWidthF( Worksheet::convertToSceneUnits(value, Worksheet::Point) );
	plot->plotArea()->setBorderPen(pen);
  }  
}

void CartesianPlotDock::borderOpacityChanged(int value){
	if (m_initializing)
		return;

	qreal opacity = (float)value/100.;
	foreach(CartesianPlot* plot, m_plotList)
		plot->plotArea()->setBorderOpacity(opacity);
}

void CartesianPlotDock::horizontalPaddingChanged(double value){
  if (m_initializing)
	return;

  foreach(CartesianPlot* plot, m_plotList)
	plot->setHorizontalPadding(Worksheet::convertToSceneUnits(value, Worksheet::Centimeter));
}

void CartesianPlotDock::verticalPaddingChanged(double value){
  if (m_initializing)
	return;

  foreach(CartesianPlot* plot, m_plotList)
	plot->setVerticalPadding(Worksheet::convertToSceneUnits(value, Worksheet::Centimeter));
}

//*************************************************************
//****** SLOTs for changes triggered in CartesianPlot *********
//*************************************************************
void CartesianPlotDock::plotDescriptionChanged(const AbstractAspect* aspect) {
	if (m_plot != aspect)
		return;

	m_initializing = true;
	if (aspect->name() != ui.leName->text()) {
		ui.leName->setText(aspect->name());
	} else if (aspect->comment() != ui.leComment->text()) {
		ui.leComment->setText(aspect->comment());
	}
	m_initializing = false;
}

void CartesianPlotDock::plotRectChanged(QRectF& rect){
	m_initializing = true;
	ui.sbLeft->setValue(Worksheet::convertFromSceneUnits(rect.x(), Worksheet::Centimeter));
	ui.sbTop->setValue(Worksheet::convertFromSceneUnits(rect.y(), Worksheet::Centimeter));
	ui.sbWidth->setValue(Worksheet::convertFromSceneUnits(rect.width(), Worksheet::Centimeter));
	ui.sbHeight->setValue(Worksheet::convertFromSceneUnits(rect.height(), Worksheet::Centimeter));
	m_initializing = false;
}

void CartesianPlotDock::plotXMinChanged(float value){
	m_initializing = true;
	ui.kleXMin->setText( QString::number(value) );
	m_initializing = false;
}

void CartesianPlotDock::plotXMaxChanged(float value){
	m_initializing = true;
	ui.kleXMax->setText( QString::number(value) );
	m_initializing = false;
}

void CartesianPlotDock::plotXScaleChanged(int scale){
	m_initializing = true;
	ui.cbXScaling->setCurrentIndex( scale );
	m_initializing = false;
}

void CartesianPlotDock::plotYMinChanged(float value){
	m_initializing = true;
	ui.kleYMin->setText( QString::number(value) );
	m_initializing = false;
}

void CartesianPlotDock::plotYMaxChanged(float value){
	m_initializing = true;
	ui.kleYMax->setText( QString::number(value) );
	m_initializing = false;
}

void CartesianPlotDock::plotYScaleChanged(int scale){
	m_initializing = true;
	ui.cbYScaling->setCurrentIndex( scale );
	m_initializing = false;
}

void CartesianPlotDock::plotVisibleChanged(bool on){
	m_initializing = true;
	ui.chkVisible->setChecked(on);
	m_initializing = false;
}

void CartesianPlotDock::plotBackgroundTypeChanged(PlotArea::BackgroundType type){
	m_initializing = true;
	ui.cbBackgroundType->setCurrentIndex(type);
	m_initializing = false;
}

void CartesianPlotDock::plotBackgroundColorStyleChanged(PlotArea::BackgroundColorStyle style){
	m_initializing = true;
	ui.cbBackgroundColorStyle->setCurrentIndex(style);
	m_initializing = false;
}

void CartesianPlotDock::plotBackgroundImageStyleChanged(PlotArea::BackgroundImageStyle style){
	m_initializing = true;
	ui.cbBackgroundImageStyle->setCurrentIndex(style);
	m_initializing = false;
}

void CartesianPlotDock::plotBackgroundBrushStyleChanged(Qt::BrushStyle style){
	m_initializing = true;
	ui.cbBackgroundBrushStyle->setCurrentIndex(style);
	m_initializing = false;
}

void CartesianPlotDock::plotBackgroundFirstColorChanged(QColor& color){
	m_initializing = true;
	ui.kcbBackgroundFirstColor->setColor(color);
	m_initializing = false;
}

void CartesianPlotDock::plotBackgroundSecondColorChanged(QColor& color){
	m_initializing = true;
	ui.kcbBackgroundSecondColor->setColor(color);
	m_initializing = false;
}

void CartesianPlotDock::plotBackgroundFileNameChanged(QString& filename){
	m_initializing = true;
	ui.kleBackgroundFileName->setText(filename);
	m_initializing = false;
}

void CartesianPlotDock::plotBackgroundOpacityChanged(float opacity){
	m_initializing = true;
	ui.sbBackgroundOpacity->setValue( round(opacity*100.0) );
	m_initializing = false;
}

void CartesianPlotDock::plotBorderPenChanged(QPen& pen){
	m_initializing = true;
	if(ui.cbBorderStyle->currentIndex() != pen.style())
		ui.cbBorderStyle->setCurrentIndex(pen.style());
	if(ui.kcbBorderColor->color() != pen.color())
		ui.kcbBorderColor->setColor(pen.color());
	if(ui.sbBorderWidth->value() != pen.widthF())
		ui.sbBorderWidth->setValue(Worksheet::convertFromSceneUnits(pen.widthF(),Worksheet::Point));
	m_initializing = false;
}

void CartesianPlotDock::plotBorderOpacityChanged(float value){
	m_initializing = true;
	float v = (float)value*100.;
	ui.sbBorderOpacity->setValue(v);
	m_initializing = false;
}

void CartesianPlotDock::plotHorizontalPaddingChanged(float value){
	m_initializing = true;
	ui.sbPaddingHorizontal->setValue(Worksheet::convertFromSceneUnits(value, Worksheet::Centimeter));
	m_initializing = false;
}

void CartesianPlotDock::plotVerticalPaddingChanged(float value){
	m_initializing = true;
	ui.sbPaddingVertical->setValue(Worksheet::convertFromSceneUnits(value, Worksheet::Centimeter));
	m_initializing = false;
}

//*************************************************************
//******************** SETTINGS *******************************
//*************************************************************

void CartesianPlotDock::loadConfig(KConfig& config){
	KConfigGroup group = config.group( "CartesianPlot" );

	//General-tab
	ui.chkVisible->setChecked( group.readEntry("Visible", m_plot->isVisible()) );
	ui.sbLeft->setValue(Worksheet::convertFromSceneUnits(group.readEntry("Left", m_plot->rect().x()), Worksheet::Centimeter));
	ui.sbTop->setValue(Worksheet::convertFromSceneUnits(group.readEntry("Top", m_plot->rect().y()), Worksheet::Centimeter));
	ui.sbWidth->setValue(Worksheet::convertFromSceneUnits(group.readEntry("Width", m_plot->rect().width()), Worksheet::Centimeter));
	ui.sbHeight->setValue(Worksheet::convertFromSceneUnits(group.readEntry("Height", m_plot->rect().height()), Worksheet::Centimeter));

	ui.chkAutoScaleX->setChecked(group.readEntry("AutoScaleX", m_plot->autoScaleX()));
  	ui.kleXMin->setText( QString::number( group.readEntry("xMin", m_plot->xMin())) );
	ui.kleXMax->setText( QString::number( group.readEntry("xMax", m_plot->xMax())) );
	ui.cbXScaling->setCurrentIndex( group.readEntry("xScale", (int) m_plot->xScale()) );

	ui.chkAutoScaleY->setChecked(group.readEntry("AutoScaleY", m_plot->autoScaleY()));
  	ui.kleYMin->setText( QString::number( group.readEntry("yMin", m_plot->yMin())) );
	ui.kleYMax->setText( QString::number( group.readEntry("yMax", m_plot->yMax())) );
	ui.cbYScaling->setCurrentIndex( group.readEntry("yScale", (int) m_plot->yScale()) );

	//Title
	labelWidget->loadConfig(group);

	//Scale breakings
	//TODO
	
	//Background-tab
	ui.cbBackgroundType->setCurrentIndex( group.readEntry("BackgroundType", (int) m_plot->plotArea()->backgroundType()) );
	ui.cbBackgroundColorStyle->setCurrentIndex( group.readEntry("BackgroundColorStyle", (int) m_plot->plotArea()->backgroundColorStyle()) );
	ui.cbBackgroundImageStyle->setCurrentIndex( group.readEntry("BackgroundImageStyle", (int) m_plot->plotArea()->backgroundImageStyle()) );
	ui.cbBackgroundBrushStyle->setCurrentIndex( group.readEntry("BackgroundBrushStyle", (int) m_plot->plotArea()->backgroundBrushStyle()) );
	ui.kleBackgroundFileName->setText( group.readEntry("BackgroundFileName", m_plot->plotArea()->backgroundFileName()) );
	ui.kcbBackgroundFirstColor->setColor( group.readEntry("BackgroundFirstColor", m_plot->plotArea()->backgroundFirstColor()) );
	ui.kcbBackgroundSecondColor->setColor( group.readEntry("BackgroundSecondColor", m_plot->plotArea()->backgroundSecondColor()) );
	ui.sbBackgroundOpacity->setValue( round(group.readEntry("BackgroundOpacity", m_plot->plotArea()->backgroundOpacity())*100.0) );
	ui.sbPaddingHorizontal->setValue(Worksheet::convertFromSceneUnits(group.readEntry("HorizontalPadding", m_plot->horizontalPadding()), Worksheet::Centimeter));
	ui.sbPaddingVertical->setValue(Worksheet::convertFromSceneUnits(group.readEntry("VerticalPadding", m_plot->verticalPadding()), Worksheet::Centimeter));

	//Border-tab
	ui.kcbBorderColor->setColor( group.readEntry("BorderColor", m_plot->plotArea()->borderPen().color()) );
	GuiTools::updatePenStyles(ui.cbBorderStyle, group.readEntry("BorderColor", m_plot->plotArea()->borderPen().color()));
	ui.cbBorderStyle->setCurrentIndex( group.readEntry("BorderStyle", (int) m_plot->plotArea()->borderPen().style()) );
	ui.sbBorderWidth->setValue( Worksheet::convertFromSceneUnits(group.readEntry("BorderWidth", m_plot->plotArea()->borderPen().widthF()), Worksheet::Point) );
	ui.sbBorderOpacity->setValue( group.readEntry("BorderOpacity", m_plot->plotArea()->borderOpacity())*100 );
}

void CartesianPlotDock::saveConfig(KConfig& config){
	KConfigGroup group = config.group( "CartesianPlot" );

	//General-tab
	group.writeEntry("Visible", ui.chkVisible->isChecked());
	group.writeEntry("Left", Worksheet::convertToSceneUnits(ui.sbLeft->value(), Worksheet::Centimeter));
	group.writeEntry("Top", Worksheet::convertToSceneUnits(ui.sbTop->value(), Worksheet::Centimeter));
	group.writeEntry("Width", Worksheet::convertToSceneUnits(ui.sbWidth->value(), Worksheet::Centimeter));
	group.writeEntry("Height", Worksheet::convertToSceneUnits(ui.sbHeight->value(), Worksheet::Centimeter));

	group.writeEntry("AutoScaleX", ui.chkAutoScaleX->isChecked());
	group.writeEntry("xMin", ui.kleXMin->text());
	group.writeEntry("xMax", ui.kleXMax->text());
	group.writeEntry("xScale", ui.cbXScaling->currentIndex());
	
	group.writeEntry("AutoScaleY", ui.chkAutoScaleY->isChecked());
	group.writeEntry("yMin", ui.kleYMin->text());
	group.writeEntry("yMax", ui.kleYMax->text());
	group.writeEntry("yScale", ui.cbYScaling->currentIndex());
	
	//Title
	labelWidget->saveConfig(group);

	//Scale breakings
	//TODO

	//Background
	group.writeEntry("BackgroundType", ui.cbBackgroundType->currentIndex());
	group.writeEntry("BackgroundColorStyle", ui.cbBackgroundColorStyle->currentIndex());
	group.writeEntry("BackgroundImageStyle", ui.cbBackgroundImageStyle->currentIndex());
	group.writeEntry("BackgroundBrushStyle", ui.cbBackgroundBrushStyle->currentIndex());
	group.writeEntry("BackgroundFileName", ui.kleBackgroundFileName->text());
	group.writeEntry("BackgroundFirstColor", ui.kcbBackgroundFirstColor->color());
	group.writeEntry("BackgroundSecondColor", ui.kcbBackgroundSecondColor->color());
	group.writeEntry("BackgroundOpacity", ui.sbBackgroundOpacity->value()/100.0);
	group.writeEntry("HorizontalPadding", Worksheet::convertToSceneUnits(ui.sbPaddingHorizontal->value(), Worksheet::Centimeter));
	group.writeEntry("VerticalPadding", Worksheet::convertToSceneUnits(ui.sbPaddingVertical->value(), Worksheet::Centimeter));

	//Border
	group.writeEntry("BorderStyle", ui.cbBorderStyle->currentIndex());
	group.writeEntry("BorderColor", ui.kcbBorderColor->color());
	group.writeEntry("BorderWidth", Worksheet::convertToSceneUnits(ui.sbBorderWidth->value(), Worksheet::Point));
	group.writeEntry("BorderOpacity", ui.sbBorderOpacity->value()/100.0);

	config.sync();
}
