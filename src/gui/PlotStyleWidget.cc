#include "PlotStyleWidget.h"

PlotStyleWidget::PlotStyleWidget(QWidget* parent):QWidget(parent){

	ui.setupUi(this);

	QStringList stylelist;
	stylelist<<i18n("Lines")<<i18n("NoCurve")<<i18n("Steps")<<i18n("Boxes")<<i18n("Impulses")<<i18n("Y Boxes");
	ui.cbLineType->insertItems(-1, stylelist);

// 	this->fillFilliingPatterns();

	//Slots
	/*
	connect( ui.cbAxes, SIGNAL(stateChanged(int)), this, SLOT(currentAxisChanged(int)) );

	//"Title"-tab
	//create a labelwidget
    QHBoxLayout* hboxLayout = new QHBoxLayout(ui.tabTitle);
	LabelWidget* labelWidget=new LabelWidget(ui.tabTitle);
    hboxLayout->addWidget(labelWidget);

	//"Ticks"-tab
	connect( ui.cbTicksStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(ticksStyleChanged(int)) );
	connect( ui.bTicksColour, SIGNAL(clicked()), this, SLOT( ticksColourClicked()) );

	//"Tick labels"-tab
	connect( ui.cbLabelsFormat, SIGNAL(currentIndexChanged(int)), this, SLOT(labelFormatChanged(int)) );
	connect( ui.bLabelsFont, SIGNAL(clicked()), this, SLOT( labelFontClicked()) );
	connect( ui.bLabelsColour, SIGNAL(clicked()), this, SLOT( labelColourClicked()) );

	//"Grid"-tab
	connect( ui.sbMajorGridWidth, SIGNAL(valueChanged(int)), this, SLOT(createMajorGridStyles()) );
	//TODO colour
	connect( ui.sbMinorGridWidth, SIGNAL(valueChanged(int)), this, SLOT(createMinorGridStyles()) );
	//TODO colour
	*/
}

PlotStyleWidget::~PlotStyleWidget(){
}

/*
void Dialog::fillBrushBox(KComboBox *cb, SType t, QColor c,FType f, QColor sc) {
	int item = cb->currentItem();
	cb->clear();
	// Qt : number of different brush styles
	int BRUSHNR=15;
	for (int i=0;i<BRUSHNR;i++) {
		QPainter pa;
		QPixmap pm( 30, 30 );
		pm.fill(Qt::white);
		pa.begin( &pm );

		Symbol symbol((SType)t,c,10,f,sc,i);
		symbol.draw(&pa,15,15);
		pa.end();

		cb->insertItem(pm);
	}
	cb->setCurrentItem(item);
}

//! fill the symbol combo box with all symbols
void Dialog::fillSymbolBox(QColor c, FType f, QColor sc, int b) {
	int item = symbolcb->currentItem();
	symbolcb->clear();
	for (int i=0;i<SYMBOLNR;i++) {
		QPainter pa;
		QPixmap pm( 30, 30 );
		pm.fill(Qt::white);
		pa.begin( &pm );

		Symbol symbol((SType)i,c,10,f,sc,b);
		symbol.draw(&pa,15,15);
		pa.end();

		symbolcb->insertItem(pm);
	}
	symbolcb->setCurrentItem(item);
}

//! fill the symbol combo box with all symbols
void Dialog::fillSymbolFillBox(SType t, QColor c, QColor sc, int b) {
	int item = symbolfillcb->currentItem();
	symbolfillcb->clear();
	for (int i=0;i<SYMBOLFILLNR;i++) {
		QPainter pa;
		QPixmap pm( 30, 30 );
		pm.fill(Qt::white);
		pa.begin( &pm );

		Symbol symbol(t,c,10,(FType)i,sc,b);
		symbol.draw(&pa,15,15);
		pa.end();

		symbolfillcb->insertItem(pm);
	}
	symbolfillcb->setCurrentItem(item);
}


//! used from dialogs for simple plots
QVBox* Dialog::simpleStyle(QTabWidget *tw, Style *style, Symbol *symbol) {


	cb2->setCurrentItem(style==0?config->readNumEntry("Graph Style",0):style->Type());
	sortpointscb = new QCheckBox(i18n("sort points"),hb);
	sortpointscb->setChecked(style==0?config->readBoolEntry("Sort Points",true):style->PointsSortingEnabled());

	hb = new QHBox(stylegb);
	new QLabel(i18n(" Box width : "),hb);
	boxwidth = new KIntNumInput(style==0?config->readNumEntry("Box Width",10):style->BoxWidth(),hb);
	boxwidth->setRange(1,1000,1,false);
	autobox = new QCheckBox(i18n("auto box width"),hb);
	autobox->setChecked(style==0?config->readBoolEntry("Auto Box Width",false):style->AutoBoxWidth());

	hb = new QHBox(stylegb);
	new QLabel(i18n("   Color : "),hb);
	color = new KColorButton(style==0?config->readColorEntry("Style Color",&Qt::blue):style->Color(),hb);
	QObject::connect(color,SIGNAL(changed(const QColor &)),this,SLOT(styleChanged()));
	hb = new QHBox(stylegb);
	new QLabel(i18n(" Line Width : "),hb);
	width = new KIntNumInput(style==0?config->readNumEntry("Style Width",1):style->Width(),hb);
	width->setMinValue(0);
	new QLabel(i18n(" Style : "),hb);
	pencb = new KComboBox(hb);
	pencb->clear();
	for (int i=0;i<6;i++) {
		QPainter pa;
		QPixmap pm( 50, 30 );
		pm.fill(Qt::white);
		pa.begin( &pm );

		pa.setPen((PenStyle)i);
		pa.drawLine(5,15,45,15);
		pa.end();

		pencb->insertItem(pm);
	}
	pencb->setCurrentItem(style==0?config->readNumEntry("Pen Style",1):style->PenStyle());

	hb = new QHBox(stylegb);
	filled = new QCheckBox(i18n("Filled "),hb);
	filled->setChecked(style==0?config->readBoolEntry("Filled",false):style->isFilled());
	fcolor = new KColorButton(style==0?config->readColorEntry("Fill Color",&Qt::green):style->FillColor(),hb);
	QObject::connect(fcolor,SIGNAL(changed(const QColor &)),this,SLOT(styleChanged()));

	hb = new QHBox(stylegb);
	new QLabel(i18n("    Brush : "),hb);
	brushcb = new KComboBox(hb);
	fillBrushBox(brushcb,SRECT,Qt::blue,FFULL,fcolor->color());
	brushcb->setCurrentItem(style==0?config->readNumEntry("Brush",0):style->Brush());

	// read Symbol values from config or from symbol if defined
	SType stype = (SType) (symbol==0?config->readNumEntry("Symbol Type",SNONE):symbol->Type());
	QColor sycolor = (symbol==0?config->readColorEntry("Symbol Color",&Qt::blue):symbol->Color());
	FType sfill = (FType) (symbol==0?config->readNumEntry("Symbol Fill",FNONE):symbol->Fill());
	QColor syfillcolor = (symbol==0?config->readColorEntry("Symbol Fill Color",&Qt::red):symbol->FillColor());
	int sysize = (symbol==0?config->readNumEntry("Symbol Size",5):symbol->Size());
	int sbrush = (symbol==0?config->readNumEntry("Symbol Brush",1):symbol->Brush());

	QGroupBox *symbolgb = new QGroupBox(1,QGroupBox::Horizontal,i18n("Symbol"),styletab);
	hb = new QHBox(symbolgb);
	new QLabel(i18n(" Type : "),hb);
	symbolcb = new KComboBox(hb);
	fillSymbolBox(sycolor,sfill,syfillcolor,sbrush);
	symbolcb->setCurrentItem(stype);
	QObject::connect(symbolcb,SIGNAL(activated(int)),this,SLOT(symbolChanged()));
	scolor = new KColorButton(sycolor,hb);
	QObject::connect(scolor,SIGNAL(changed(const QColor &)),this,SLOT(symbolChanged()));

	hb = new QHBox(symbolgb);
	new QLabel(i18n("    Size : "),hb);
	ssize = new KIntNumInput(sysize,hb);
	ssize->setRange(1,30);

	hb = new QHBox(symbolgb);
	new QLabel(i18n("    Fill : "),hb);
	symbolfillcb = new KComboBox(hb);
	fillSymbolFillBox(stype,sycolor,syfillcolor,sbrush);
	symbolfillcb->setCurrentItem(sfill);
	QObject::connect(symbolfillcb,SIGNAL(activated(int)),this,SLOT(symbolChanged()));

	// needed ???
	ssize->setValue(sysize);
	sfcolor = new KColorButton(syfillcolor,hb);
	QObject::connect(sfcolor,SIGNAL(changed(const QColor &)),this,SLOT(symbolChanged()));

	hb = new QHBox(symbolgb);
	new QLabel(i18n("    Brush : "),hb);
	sbrushcb = new KComboBox(hb);
	fillBrushBox(sbrushcb,stype,sycolor,sfill,syfillcolor);
	sbrushcb->setCurrentItem(sbrush);
	QObject::connect(sbrushcb,SIGNAL(activated(int)),this,SLOT(symbolChanged()));

	return styletab;
}
*/
//**********************************************************
//****************** SLOTS ********************************
//**********************************************************

