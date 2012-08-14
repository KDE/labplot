/***************************************************************************
    File                 : WorksheetDock.cpp
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2010 by Alexander Semke (alexander.semke*web.de)
    Copyright            : (C) 2012 by Stefan Gerlach (stefan.gerlach*uni-konstanz.de)
    							(use @ for *)
    Description          : widget for worksheet properties
                           
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

#include "WorksheetDock.h"
#include "worksheet/Worksheet.h"
#include "worksheet/plots/PlotArea.h"
#include "../TemplateHandler.h"
#include "../GuiTools.h"
#include <QTimer>
#include <QPrinter>
#include <QFileDialog>
#include <KUrlCompletion>

// a couple of standard sizes in mm, taken from qprinter.cpp
static const float qt_paperSizes[][2] = {
    {210, 297}, // A4
    {176, 250}, // B5
    {215.9f, 279.4f}, // Letter
    {215.9f, 355.6f}, // Legal
    {190.5f, 254}, // Executive
    {841, 1189}, // A0
    {594, 841}, // A1
    {420, 594}, // A2
    {297, 420}, // A3
    {148, 210}, // A5
    {105, 148}, // A6
    {74, 105}, // A7
    {52, 74}, // A8
    {37, 52}, // A8
    {1000, 1414}, // B0
    {707, 1000}, // B1
    {31, 44}, // B10
    {500, 707}, // B2
    {353, 500}, // B3
    {250, 353}, // B4
    {125, 176}, // B6
    {88, 125}, // B7
    {62, 88}, // B8
    {33, 62}, // B9
    {163, 229}, // C5E
    {105, 241}, // US Common
    {110, 220}, // DLE
    {210, 330}, // Folio
    {431.8f, 279.4f}, // Ledger
    {279.4f, 431.8f} // Tabloid
};

/*!
  \class WorksheetDock
  \brief  Provides a widget for editing the properties of the worksheets currently selected in the project explorer.

  \ingroup kdefrontend
*/

WorksheetDock::WorksheetDock(QWidget *parent): QWidget(parent){
	ui.setupUi(this);

	//Background-tab
	ui.cbBackgroundColorStyle->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
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

	//SLOTs
	
	//General
	connect( ui.leName, SIGNAL(returnPressed()), this, SLOT(nameChanged()) );
	connect( ui.leComment, SIGNAL(returnPressed()), this, SLOT(commentChanged()) );
	connect( ui.cbSize, SIGNAL(currentIndexChanged(int)), this, SLOT(sizeChanged(int)) );
	connect( ui.sbWidth, SIGNAL(valueChanged(double)), this, SLOT(sizeChanged()) );
	connect( ui.sbHeight, SIGNAL(valueChanged(double)), this, SLOT(sizeChanged()) );
	connect( ui.cbOrientation, SIGNAL(currentIndexChanged(int)), this, SLOT(orientationChanged(int)) );
	
	//Background
	connect( ui.cbBackgroundType, SIGNAL(currentIndexChanged(int)), this, SLOT(backgroundTypeChanged(int)) );
	connect( ui.cbBackgroundColorStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(backgroundColorStyleChanged(int)) );
	connect( ui.cbBackgroundImageStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(backgroundImageStyleChanged(int)) );
	connect( ui.cbBackgroundBrushStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(backgroundBrushStyleChanged(int)) );
	connect( ui.bOpen, SIGNAL(clicked(bool)), this, SLOT(selectFile()));
	connect( ui.kleBackgroundFileName, SIGNAL(returnPressed()), this, SLOT(fileNameChanged()) );
	connect( ui.kleBackgroundFileName, SIGNAL(clearButtonClicked()), this, SLOT(fileNameChanged()) );
	connect( ui.kcbBackgroundFirstColor, SIGNAL(changed (const QColor &)), this, SLOT(backgroundFirstColorChanged(const QColor&)) );
	connect( ui.kcbBackgroundSecondColor, SIGNAL(changed (const QColor &)), this, SLOT(backgroundSecondColorChanged(const QColor&)) );
	connect( ui.sbBackgroundOpacity, SIGNAL(valueChanged(int)), this, SLOT(opacityChanged(int)) );

	//Layout
	connect( ui.sbLayoutTopMargin, SIGNAL(valueChanged(double)), this, SLOT(layoutTopMarginChanged(double)) );
	connect( ui.sbLayoutBottomMargin, SIGNAL(valueChanged(double)), this, SLOT(layoutBottomMarginChanged(double)) );
	connect( ui.sbLayoutLeftMargin, SIGNAL(valueChanged(double)), this, SLOT(layoutLeftMarginChanged(double)) );
	connect( ui.sbLayoutRightMargin, SIGNAL(valueChanged(double)), this, SLOT(layoutRightMarginChanged(double)) );
	connect( ui.sbLayoutHorizontalSpacing, SIGNAL(valueChanged(double)), this, SLOT(layoutHorizontalSpacingChanged(double)) );
	connect( ui.sbLayoutVerticalSpacing, SIGNAL(valueChanged(double)), this, SLOT(layoutVerticalSpacingChanged(double)) );
	connect( ui.sbLayoutRowCount, SIGNAL(valueChanged(int)), this, SLOT(layoutRowCountChanged(int)) );
	connect( ui.sbLayoutColumnCount, SIGNAL(valueChanged(int)), this, SLOT(layoutColumnCountChanged(int)) );
	
	TemplateHandler* templateHandler = new TemplateHandler(this, TemplateHandler::Worksheet);
	ui.verticalLayout->addWidget(templateHandler, 0, 0);
	templateHandler->show();
	connect( templateHandler, SIGNAL(loadConfigRequested(KConfig&)), this, SLOT(loadConfig(KConfig&)));
	connect( templateHandler, SIGNAL(saveConfigRequested(KConfig&)), this, SLOT(saveConfig(KConfig&)));

	this->retranslateUi();
}

void WorksheetDock::setWorksheets(QList<Worksheet*> list){
	m_initializing = true;
	m_worksheetList = list;

	Worksheet* worksheet=list.first();
  
  //if there are more then one worksheets in the list, disable the name and comment field in the tab "general"
  if (list.size()==1){
	ui.lName->setEnabled(true);
	ui.leName->setEnabled(true);
	ui.lComment->setEnabled(true);
	ui.leComment->setEnabled(true);

	ui.leName->setText(worksheet->name());
	ui.leComment->setText(worksheet->comment());
  }else{
	ui.lName->setEnabled(false);
	ui.leName->setEnabled(false);
	ui.lComment->setEnabled(false);
	ui.leComment->setEnabled(false);

	ui.leName->setText("");
	ui.leComment->setText("");
  }
  
	//show the properties of the first worksheet
	KConfig config("", KConfig::SimpleConfig);
  	loadConfig(config);
  
	connect(worksheet, SIGNAL(layoutRowCountChanged(int)), this, SLOT(worksheetLayoutRowCountChanged(int)));
	m_initializing = false;
}

/*!
	Checks whether the size is one of the QPrinter::PaperSize and
	updates Size and Orientation checkbox when width/height changes.
*/
void WorksheetDock::updatePaperSize(){
	int i=0;
	
	//In UI we use cm, so we need to convert to mm first before we check with qt_paperSizes
	int w=(float)ui.sbWidth->value()*10;
	int h=(float)ui.sbHeight->value()*10;

	//check the portrait-orientation first
	while ( !(w==qt_paperSizes[i][0] && h==qt_paperSizes[i][1]) && i<30 ){
		i++;
	}
	
	if (i!=30){
		ui.cbOrientation->setCurrentIndex(0);  //a QPrinter::PaperSize  in portrait-orientation was found
	}else{
		//check for the landscape-orientation
		i=0;
		while ( !(w==qt_paperSizes[i][1] && h==qt_paperSizes[i][0]) && i<30 ){
			i++;
		}
		
		if (i!=30)
			ui.cbOrientation->setCurrentIndex(1); //a QPrinter::PaperSize  in landscape-orientation was found
	}
	
	//determine the position of the QPrinter::PaperSize in the combobox
	for (int index=0; index<31; index++){
		if (ui.cbSize->itemData(index).toInt() == i){
			ui.cbSize->setCurrentIndex(index);
			break;
		}
	}
}


//*************************************************************
//****** SLOTs for changes triggered in WorksheetDock *********
//*************************************************************
void WorksheetDock::retranslateUi(){
	m_initializing = true;
	
	//Geometry
	ui.cbOrientation->addItem(i18n("portrait"));
	ui.cbOrientation->addItem(i18n("landscape"));

    ui.cbSize->addItem(i18n("A0"), QPrinter::A0);
    ui.cbSize->addItem(i18n("A1"), QPrinter::A1);
    ui.cbSize->addItem(i18n("A2"), QPrinter::A2);
    ui.cbSize->addItem(i18n("A3"), QPrinter::A3);
    ui.cbSize->addItem(i18n("A4"), QPrinter::A4);
    ui.cbSize->addItem(i18n("A5"), QPrinter::A5);
    ui.cbSize->addItem(i18n("A6"), QPrinter::A6);
    ui.cbSize->addItem(i18n("A7"), QPrinter::A7);
    ui.cbSize->addItem(i18n("A8"), QPrinter::A8);
    ui.cbSize->addItem(i18n("A9"), QPrinter::A9);
    ui.cbSize->addItem(i18n("B0"), QPrinter::B0);
    ui.cbSize->addItem(i18n("B1"), QPrinter::B1);
    ui.cbSize->addItem(i18n("B2"), QPrinter::B2);
    ui.cbSize->addItem(i18n("B3"), QPrinter::B3);
    ui.cbSize->addItem(i18n("B4"), QPrinter::B4);
    ui.cbSize->addItem(i18n("B5"), QPrinter::B5);
    ui.cbSize->addItem(i18n("B6"), QPrinter::B6);
    ui.cbSize->addItem(i18n("B7"), QPrinter::B7);
    ui.cbSize->addItem(i18n("B8"), QPrinter::B8);
    ui.cbSize->addItem(i18n("B9"), QPrinter::B9);
    ui.cbSize->addItem(i18n("B10"), QPrinter::B10);
    ui.cbSize->addItem(i18n("C5E"), QPrinter::C5E);
    ui.cbSize->addItem(i18n("DLE"), QPrinter::DLE);
    ui.cbSize->addItem(i18n("Executive"), QPrinter::Executive);
    ui.cbSize->addItem(i18n("Folio"), QPrinter::Folio);
    ui.cbSize->addItem(i18n("Ledger"), QPrinter::Ledger);
    ui.cbSize->addItem(i18n("Legal"), QPrinter::Legal);
    ui.cbSize->addItem(i18n("Letter"), QPrinter::Letter);
    ui.cbSize->addItem(i18n("Tabloid"), QPrinter::Tabloid);
    ui.cbSize->addItem(i18n("US Common #10 Envelope"), QPrinter::Comm10E);
	ui.cbSize->addItem(i18n("Custom"), QPrinter::Custom);
	
	ui.sbWidth->setSuffix(i18n("cm"));
	ui.sbHeight->setSuffix(i18n("cm"));
	
	//Background
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
	GuiTools::updateBrushStyles(ui.cbBackgroundBrushStyle, Qt::SolidPattern);

	//layout
	ui.sbLayoutTopMargin->setSuffix(i18n("cm"));
	ui.sbLayoutBottomMargin->setSuffix(i18n("cm"));
	ui.sbLayoutLeftMargin->setSuffix(i18n("cm"));
	ui.sbLayoutRightMargin->setSuffix(i18n("cm"));
	ui.sbLayoutHorizontalSpacing->setSuffix(i18n("cm"));
	ui.sbLayoutVerticalSpacing->setSuffix(i18n("cm"));

	m_initializing = false;
}


// "General"-tab
void WorksheetDock::nameChanged(){
  if (m_initializing)
	return;
  
  m_worksheetList.first()->setName(ui.leName->text());
}

void WorksheetDock::commentChanged(){
  if (m_initializing)
	return;
  
  m_worksheetList.first()->setComment(ui.leComment->text());
}

void WorksheetDock::sizeChanged(int i){
	int index  = ui.cbSize->itemData(i).toInt();
    
	if (index==QPrinter::Custom){
		ui.sbWidth->setEnabled(true);
		ui.sbHeight->setEnabled(true);
		ui.lOrientation->hide();
		ui.cbOrientation->hide();
		return;
	}else{
		ui.sbWidth->setEnabled(false);
		ui.sbHeight->setEnabled(false);
		ui.lOrientation->show();
		ui.cbOrientation->show();
	}
	
	if (m_initializing)
		return;
  
	float w, h;
	if (ui.cbOrientation->currentIndex() == 0){
		w=qt_paperSizes[index][0];
		h=qt_paperSizes[index][1];
	}else{
		w=qt_paperSizes[index][1];
		h=qt_paperSizes[index][0];
	}
	
	m_initializing = true;
	//w and h from qt_paperSizes above are in mm, in UI we show everything in cm
	ui.sbWidth->setValue(w/10);
	ui.sbHeight->setValue(h/10);
	m_initializing=false;
	
	bool scaleContent = ui.chScaleContent->isChecked();
	w = Worksheet::convertToSceneUnits(w, Worksheet::Millimeter);
	h = Worksheet::convertToSceneUnits(h, Worksheet::Millimeter);
	foreach(Worksheet* worksheet, m_worksheetList){
		worksheet->setPageRect(QRect(0,0,w,h), scaleContent);
	}
}

void WorksheetDock::sizeChanged(){
  if (m_initializing)
	return;

  	int w = Worksheet::convertToSceneUnits(ui.sbWidth->value(), Worksheet::Centimeter);
	int h = Worksheet::convertToSceneUnits(ui.sbHeight->value(), Worksheet::Centimeter);
	bool scaleContent = ui.chScaleContent->isChecked();
	foreach(Worksheet* worksheet, m_worksheetList){
		worksheet->setPageRect(QRect(0,0,w,h), scaleContent);
	}
}

void WorksheetDock::orientationChanged(int index){
	Q_UNUSED(index);
  if (m_initializing)
	return;

  this->sizeChanged(ui.cbSize->currentIndex());
}

// "Background"-tab
void WorksheetDock::opacityChanged(int value){
  if (m_initializing)
	return;

	qreal opacity = (float)value/100;
	foreach(Worksheet* worksheet, m_worksheetList){
		worksheet->setBackgroundOpacity(opacity);
	}
}

void WorksheetDock::backgroundTypeChanged(int index){
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
		ui.lBackgroundFirstColor->hide();
		ui.kcbBackgroundFirstColor->hide();
		ui.lBackgroundSecondColor->hide();
		ui.kcbBackgroundSecondColor->hide();

		ui.lBackgroundColorStyle->hide();
		ui.cbBackgroundColorStyle->hide();
		ui.lBackgroundImageStyle->show();
		ui.cbBackgroundImageStyle->show();
		ui.lBackgroundBrushStyle->hide();
		ui.cbBackgroundBrushStyle->hide();
		ui.lBackgroundFileName->show();
		ui.kleBackgroundFileName->show();
		ui.bOpen->show();
	}else if(type == PlotArea::Pattern){
		ui.lBackgroundFirstColor->setText("Color");
		ui.lBackgroundFirstColor->show();
		ui.kcbBackgroundFirstColor->show();
		ui.lBackgroundSecondColor->hide();
		ui.kcbBackgroundSecondColor->hide();

		ui.lBackgroundColorStyle->hide();
		ui.cbBackgroundColorStyle->hide();
		ui.lBackgroundImageStyle->hide();
		ui.cbBackgroundImageStyle->hide();
		ui.lBackgroundBrushStyle->show();
		ui.cbBackgroundBrushStyle->show();
		ui.lBackgroundFileName->hide();
		ui.kleBackgroundFileName->hide();
		ui.bOpen->hide();
	}

	if (m_initializing)
		return;
   
	foreach(Worksheet* worksheet, m_worksheetList){
		worksheet->setBackgroundType(type);
  }
}

void WorksheetDock::backgroundColorStyleChanged(int index){
	PlotArea::BackgroundColorStyle style = (PlotArea::BackgroundColorStyle)index;

	if (style == PlotArea::SingleColor){
		ui.lBackgroundFirstColor->setText("Color");
		ui.lBackgroundSecondColor->hide();
		ui.kcbBackgroundSecondColor->hide();
	}else{
		ui.lBackgroundFirstColor->setText("First Color");
		ui.lBackgroundSecondColor->show();
		ui.kcbBackgroundSecondColor->show();
	}

	if (m_initializing)
		return;
   
	foreach(Worksheet* worksheet, m_worksheetList){
		worksheet->setBackgroundColorStyle(style);
  }  
}

void WorksheetDock::backgroundImageStyleChanged(int index){
	if (m_initializing)
		return;

	PlotArea::BackgroundImageStyle style = (PlotArea::BackgroundImageStyle)index;
	foreach(Worksheet* worksheet, m_worksheetList){
		worksheet->setBackgroundImageStyle(style);
	}  
}

void WorksheetDock::backgroundBrushStyleChanged(int index){
	if (m_initializing)
		return;

	Qt::BrushStyle style = (Qt::BrushStyle)index;
	foreach(Worksheet* worksheet, m_worksheetList){
		worksheet->setBackgroundBrushStyle(style);
	}  
}

void WorksheetDock::backgroundFirstColorChanged(const QColor& c){
  if (m_initializing)
	return;

	foreach(Worksheet* worksheet, m_worksheetList){
		worksheet->setBackgroundFirstColor(c);
	}
}


void WorksheetDock::backgroundSecondColorChanged(const QColor& c){
  if (m_initializing)
	return;

	foreach(Worksheet* worksheet, m_worksheetList){
		worksheet->setBackgroundFirstColor(c);
	}
}

//"Layout"-tab
void WorksheetDock::layoutTopMarginChanged(double margin){
	if (m_initializing)
		return;

	foreach(Worksheet* worksheet, m_worksheetList){
		worksheet->setLayoutTopMargin(Worksheet::convertToSceneUnits(margin, Worksheet::Centimeter));
	}
}

void WorksheetDock::layoutBottomMarginChanged(double margin){
	if (m_initializing)
		return;

	foreach(Worksheet* worksheet, m_worksheetList){
		worksheet->setLayoutBottomMargin(Worksheet::convertToSceneUnits(margin, Worksheet::Centimeter));
	}
}

void WorksheetDock::layoutLeftMarginChanged(double margin){
	if (m_initializing)
		return;

	foreach(Worksheet* worksheet, m_worksheetList){
		worksheet->setLayoutLeftMargin(Worksheet::convertToSceneUnits(margin, Worksheet::Centimeter));
	}
}

void WorksheetDock::layoutRightMarginChanged(double margin){
	if (m_initializing)
		return;

	foreach(Worksheet* worksheet, m_worksheetList){
		worksheet->setLayoutRightMargin(Worksheet::convertToSceneUnits(margin, Worksheet::Centimeter));
	}
}

void WorksheetDock::layoutHorizontalSpacingChanged(double spacing){
	if (m_initializing)
		return;

	foreach(Worksheet* worksheet, m_worksheetList){
		worksheet->setLayoutHorizontalSpacing(Worksheet::convertToSceneUnits(spacing, Worksheet::Centimeter));
	}
}


void WorksheetDock::layoutVerticalSpacingChanged(double spacing){
	if (m_initializing)
		return;

	foreach(Worksheet* worksheet, m_worksheetList){
		worksheet->setLayoutVerticalSpacing(Worksheet::convertToSceneUnits(spacing, Worksheet::Centimeter));
	}
}

void WorksheetDock::layoutRowCountChanged(int count){
	if (m_initializing)
		return;

	foreach(Worksheet* worksheet, m_worksheetList){
		worksheet->setLayoutRowCount(count);
	}
}

void WorksheetDock::layoutColumnCountChanged(int count){
	if (m_initializing)
		return;

	foreach(Worksheet* worksheet, m_worksheetList){
		worksheet->setLayoutColumnCount(count);
	}
}

/*!
	opens a file dialog and lets the user select the image file.
*/
void WorksheetDock::selectFile() {
    QString path=QFileDialog::getOpenFileName(this, i18n("Select the image file"));
    if (path=="")
        return;

    ui.kleBackgroundFileName->setText( path );

	foreach(Worksheet* worksheet, m_worksheetList){
		worksheet->setBackgroundFileName(path);
  }  
}

void WorksheetDock::fileNameChanged(){
	if (m_initializing)
		return;
	
	QString fileName = ui.kleBackgroundFileName->text();
	foreach(Worksheet* worksheet, m_worksheetList){
		worksheet->setBackgroundFileName(fileName);
  } 
}

//*************************************************************
//******** SLOTs for changes triggered in Worksheet ***********
//*************************************************************
void WorksheetDock::worksheetLayoutRowCountChanged(int value){
	m_initializing = true;
	ui.sbLayoutRowCount->setValue(value);
	m_initializing = false;
}


//*************************************************************
//******************** SETTINGS *******************************
//*************************************************************

void WorksheetDock::loadConfig(KConfig& config){
	KConfigGroup group = config.group( "Worksheet" );
	
	// Geometry
	ui.chScaleContent->setChecked(group.readEntry("ScaleContent", FALSE));
	Worksheet* worksheet=m_worksheetList.first();
	ui.sbWidth->setValue(Worksheet::convertFromSceneUnits(group.readEntry("Width",worksheet->pageRect().width()), Worksheet::Centimeter));
	ui.sbHeight->setValue(Worksheet::convertFromSceneUnits(group.readEntry("Height",worksheet->pageRect().height()), Worksheet::Centimeter));
	updatePaperSize();

	// Background-tab
	ui.cbBackgroundColorStyle->setCurrentIndex( group.readEntry("BackgroundColorStyle", (int) worksheet->backgroundColorStyle()) );
	ui.cbBackgroundImageStyle->setCurrentIndex( group.readEntry("BackgroundImageStyle", (int) worksheet->backgroundImageStyle()) );
	ui.cbBackgroundBrushStyle->setCurrentIndex( group.readEntry("BackgroundBrushStyle", (int) worksheet->backgroundBrushStyle()) );
	ui.kleBackgroundFileName->setText( group.readEntry("BackgroundFileName", worksheet->backgroundFileName()) );
	ui.kcbBackgroundFirstColor->setColor( group.readEntry("BackgroundFirstColor", worksheet->backgroundFirstColor()) );
	ui.kcbBackgroundSecondColor->setColor( group.readEntry("BackgroundSecondColor", worksheet->backgroundSecondColor()) );
	ui.sbBackgroundOpacity->setValue(group.readEntry("BackgroundOpacity", worksheet->backgroundOpacity())*100 );
	// this at last since others emmit backgroundColorStyleChanges
	// and enable SecondColor button, etc.!
	ui.cbBackgroundType->setCurrentIndex( group.readEntry("BackgroundType", (int) worksheet->backgroundType()) );
	
	// Layout
	ui.sbLayoutTopMargin->setValue(group.readEntry("LayoutTopMargin", 
												   Worksheet::convertFromSceneUnits(worksheet->layoutTopMargin(), Worksheet::Centimeter)) );
	ui.sbLayoutBottomMargin->setValue(group.readEntry("LayoutBottomMargin",
													   Worksheet::convertFromSceneUnits(worksheet->layoutBottomMargin(), Worksheet::Centimeter)) );
	ui.sbLayoutLeftMargin->setValue(group.readEntry("LayoutLeftMargin",
													 Worksheet::convertFromSceneUnits(worksheet->layoutLeftMargin(), Worksheet::Centimeter)) );
	ui.sbLayoutRightMargin->setValue(group.readEntry("LayoutRightMargin",
													  Worksheet::convertFromSceneUnits(worksheet->layoutRightMargin(), Worksheet::Centimeter)) );
	ui.sbLayoutHorizontalSpacing->setValue(group.readEntry("LayoutHorizontalSpacing",
														    Worksheet::convertFromSceneUnits(worksheet->layoutHorizontalSpacing(), Worksheet::Centimeter)) );
	ui.sbLayoutVerticalSpacing->setValue(group.readEntry("LayoutVerticalSpacing",
														  Worksheet::convertFromSceneUnits(worksheet->layoutVerticalSpacing(), Worksheet::Centimeter)) );
	
	ui.sbLayoutRowCount->setValue(group.readEntry("LayoutRowCount",worksheet->layoutRowCount()));
	ui.sbLayoutColumnCount->setValue(group.readEntry("LayoutColumnCount",worksheet->layoutColumnCount()));
}

void WorksheetDock::saveConfig(KConfig& config){
	KConfigGroup group = config.group( "Worksheet" );

	//General
	group.writeEntry("ScaleContent",ui.chScaleContent->isChecked());
	group.writeEntry("Width",Worksheet::convertToSceneUnits(ui.sbWidth->value(), Worksheet::Centimeter));
	group.writeEntry("Height",Worksheet::convertToSceneUnits(ui.sbHeight->value(), Worksheet::Centimeter));

	//Background
	group.writeEntry("BackgroundType",ui.cbBackgroundType->currentIndex());
	group.writeEntry("BackgroundColorStyle", ui.cbBackgroundColorStyle->currentIndex());
	group.writeEntry("BackgroundImageStyle", ui.cbBackgroundImageStyle->currentIndex());
	group.writeEntry("BackgroundBrushStyle", ui.cbBackgroundBrushStyle->currentIndex());
	group.writeEntry("BackgroundFileName", ui.kleBackgroundFileName->text());
	group.writeEntry("BackgroundFirstColor", ui.kcbBackgroundFirstColor->color());
	group.writeEntry("BackgroundSecondColor", ui.kcbBackgroundSecondColor->color());
	group.writeEntry("BackgroundOpacity", ui.sbBackgroundOpacity->value()/100.0);

	//Layout
	group.writeEntry("LayoutTopMargin",Worksheet::convertToSceneUnits(ui.sbLayoutTopMargin->value(), Worksheet::Centimeter));
	group.writeEntry("LayoutBottomMargin",Worksheet::convertToSceneUnits(ui.sbLayoutBottomMargin->value(), Worksheet::Centimeter));
	group.writeEntry("LayoutLeftMargin",Worksheet::convertToSceneUnits(ui.sbLayoutLeftMargin->value(), Worksheet::Centimeter));
	group.writeEntry("LayoutRightMargin",Worksheet::convertToSceneUnits(ui.sbLayoutRightMargin->value(), Worksheet::Centimeter));
	group.writeEntry("LayoutVerticalSpacing",Worksheet::convertToSceneUnits(ui.sbLayoutVerticalSpacing->value(), Worksheet::Centimeter));
	group.writeEntry("LayoutHorizontalSpacing",Worksheet::convertToSceneUnits(ui.sbLayoutHorizontalSpacing->value(), Worksheet::Centimeter));
	group.writeEntry("LayoutRowCount", ui.sbLayoutRowCount->value());
	group.writeEntry("LayoutColumnCount", ui.sbLayoutColumnCount->value());

	config.sync();
}
