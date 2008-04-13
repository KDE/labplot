#include "PlotStyleWidget.h"
#include "../elements/Style.h"
#include "../elements/Symbol.h"
#include <KDebug>

PlotStyleWidget::PlotStyleWidget(QWidget* parent):QWidget(parent){

	ui.setupUi(this);

	QStringList stylelist;
	stylelist<<i18n("Lines")<<i18n("NoCurve")<<i18n("Steps")<<i18n("Boxes")<<i18n("Impulses")<<i18n("Y Boxes");
	ui.cbLineType->insertItems(-1, stylelist);

	ui.kcbLineColor->setColor(Qt::black);
	this->fillLineStyleBox();
	ui.cbLineStyle->setCurrentIndex(0);

	ui.kcbAreaFillingColor->setColor(Qt::green);
	this->fillAreaFillingPatternBox();
	ui.cbAreaFillingPattern->setCurrentIndex(0);

	ui.kcbSymbolColor->setColor(Qt::black);
	this->fillSymbolTypeBox();
	ui.cbSymbolType->setCurrentIndex(0);
	this->symbolTypeChanged(0);

	ui.kcbSymbolFillingColor->setColor(Qt::black);
	this->fillSymbolFillingPatternBox();
	ui.cbSymbolFillingPattern->setCurrentIndex(0);
	this->fillSymbolFillingBox();
	ui.cbSymbolFilling->setCurrentIndex(0);

	//Slots
	connect( ui.kcbLineColor, SIGNAL(changed (const QColor &)), this, SLOT(fillLineStyleBox()) );
	connect( ui.sbLineWidth, SIGNAL(valueChanged(int)), this, SLOT(fillLineStyleBox()) );

	connect( ui.kcbAreaFillingColor, SIGNAL(changed (const QColor &)), this, SLOT(fillAreaFillingPatternBox()) );

	connect( ui.cbSymbolType, SIGNAL(currentIndexChanged(int)), this, SLOT(symbolTypeChanged(int)) );
	connect( ui.kcbSymbolColor, SIGNAL(changed (const QColor &)), this, SLOT(fillSymbolTypeBox()) );
	connect( ui.kcbSymbolFillingColor, SIGNAL(changed (const QColor &)), this, SLOT(symbolFillingColorChanged()) );
	connect( ui.cbSymbolFillingPattern, SIGNAL(currentIndexChanged(int)), this, SLOT(fillSymbolFillingBox()) );

	connect( ui.chbBoxWidth, SIGNAL(stateChanged (int)), this, SLOT(boxWidthStateChanged(int)) );
}

PlotStyleWidget::~PlotStyleWidget(){}

void PlotStyleWidget::setStyle(const Style* style){

}

void PlotStyleWidget::saveStyle(Style* style) const{

}

/*!
	fills the ComboBox for the line styles with the six possible Qt::PenStyles.
	Called on start and if the line color or the line width are changed.
*/
void PlotStyleWidget::fillLineStyleBox(){
	int index=ui.cbLineStyle->currentIndex();
	ui.cbLineStyle->clear();

	QPainter pa;
	int offset=5;
	int w=ui.cbLineStyle->width()-2*offset;
	int h=10;
	QPixmap pm( w, h );
	ui.cbLineStyle->setIconSize( QSize(w,h) );
 	QColor penColor = ui.kcbLineColor->color();
	int penWidth = ui.sbLineWidth->value();

	//loop over six possible PenStyles, draw on the pixmap and insert it
	for (int i=1;i<6;i++) {
		pm.fill(Qt::transparent);
		pa.begin( &pm );
// 		pa.setRenderHint(QPainter::Antialiasing);
		pa.setPen( QPen( penColor, penWidth, (Qt::PenStyle)i ) );
		pa.drawLine( offset, h/2, w-offset, h/2);
		pa.end();
		ui.cbLineStyle->addItem( QIcon(pm), "" );
	}
	ui.cbLineStyle->setCurrentIndex(index);

	kDebug()<<"line styles updated"<<endl;
}


/*!
	fills the ComboBox for the area filling with the 14 possible Qt::BrushStyles.
	Called on start and if the color of the area filling is changed.
*/
void PlotStyleWidget::fillAreaFillingPatternBox() {
	int index=ui.cbAreaFillingPattern->currentIndex();
	ui.cbAreaFillingPattern->clear();

	QPainter pa;
	int offset=5;
	int w=ui.cbAreaFillingPattern->width() - 2*offset;
	int h=ui.cbAreaFillingPattern->height() - 2*offset;
	QPixmap pm( w, h );
	ui.cbAreaFillingPattern->setIconSize( QSize(w,h) );

 	QColor penColor = ui.kcbAreaFillingColor->color();
	QPen pen(Qt::SolidPattern, 1);
 	pa.setPen( pen );

	//loop over 14 possible Qt::BrushStyles
	for (int i=1;i<15;i++) {
		pm.fill(Qt::transparent);
		pa.begin( &pm );
// 		pa.setRenderHint(QPainter::Antialiasing);
 		pa.setBrush( QBrush(penColor, (Qt::BrushStyle)i) );
		pa.drawRect( offset, offset, w-2*offset, h-2*offset);
		pa.end();
		ui.cbAreaFillingPattern->addItem( QIcon(pm), "" );
	}

	ui.cbAreaFillingPattern->setCurrentIndex(index);
}

/*!
	fills the ComboBox for the symbol type with all possible types defined in \c Symbol.h
	Called on start and if the symbol color is changed.
*/
void PlotStyleWidget::fillSymbolTypeBox(){
	int index = ui.cbSymbolType->currentIndex();
	ui.cbSymbolType->clear();

	QColor color=ui.kcbSymbolColor->color();
// 	QColor fillColor=ui.kcbSymbolFillingColor->color();
// 	Qt::BrushStyle fillBrushStyle=Qt::BrushStyle(ui.cbSymbolFillingPattern->currentIndex());
	QPainter pa;

	int offset=5;
	//TODO size of the icon depending on the actuall height of the combobox?
	int size=20;
	QPixmap pm( size, size );
// 	ui.cbSymbolType->setIconSize( QSize(size, size) );

	int c=Symbol::styleCount();
	for (int i=1; i<c; i++) {
		pm.fill(Qt::transparent);
		pa.begin( &pm );
		pa.setRenderHint(QPainter::Antialiasing);

		//draw the symbol in the middle of the pixmap
// 		Symbol::draw(&pa, QPoint(size/2, size/2), (Symbol::SType)i, color, size, Symbol::FNONE, fillColor, fillBrushStyle);
		Symbol::draw(&pa, QPoint(size/2, size/2), (Symbol::SType)i, color, size-2*offset, Symbol::FNONE);
		pa.end();

		ui.cbSymbolType->addItem(QIcon(pm), "");
	}
	ui.cbSymbolType->setCurrentIndex(index);
}


/*!
	fills the ComboBox for the filling types (no filling, full filling etc.)
	Called on start and if the symbol filling color or the symbol type are changed.
*/
void PlotStyleWidget::fillSymbolFillingBox(){
		int index = ui.cbSymbolFilling->currentIndex();
		ui.cbSymbolFilling->clear();

		QColor color=ui.kcbSymbolColor->color();
		Symbol::SType symbolType=Symbol::SType(ui.cbSymbolType->currentIndex() +1);
		QColor fillColor=ui.kcbSymbolFillingColor->color();
		Qt::BrushStyle fillBrushStyle=Qt::BrushStyle(ui.cbSymbolFillingPattern->currentIndex()+1);
		QPainter pa;

		int offset=5;
		int size=20;
		QPixmap pm( size, size );

		int c=Symbol::fillingTypeCount();
		ui.cbSymbolFilling->addItem( i18n("no filling") );
		for (int i=1; i<c; i++) {
			pm.fill(Qt::transparent);
			pa.begin( &pm );
			pa.setRenderHint(QPainter::Antialiasing);

			//draw the symbol in the middle of the pixmap
			Symbol::draw(&pa, QPoint(size/2, size/2), symbolType, color, size-2*offset, (Symbol::FType)i, fillColor, fillBrushStyle);
			pa.end();

			ui.cbSymbolFilling->addItem(QIcon(pm), "");
		}
		ui.cbSymbolFilling->setCurrentIndex(index);
}

/*!
	fills the ComboBox for the symbol filling patterns with the 14 possible Qt::BrushStyles.
	Called on start and if the symbol filling color is changed.
*/
void PlotStyleWidget::fillSymbolFillingPatternBox() {
	int index=ui.cbSymbolFillingPattern->currentIndex();
	ui.cbSymbolFillingPattern->clear();

	QPainter pa;
	int offset=5;
	int w=ui.cbSymbolFillingPattern->width() - 2*offset;
	int h=ui.cbSymbolFillingPattern->height() - 2*offset;
	QPixmap pm( w, h );
	ui.cbSymbolFillingPattern->setIconSize( QSize(w,h) );

 	QColor penColor = ui.kcbSymbolFillingColor->color();
	QPen pen(Qt::SolidPattern, 1);
 	pa.setPen( pen );

	//loop over 14 possible Qt-BrushStyles
	for (int i=1;i<15;i++) {
		pm.fill(Qt::transparent);
		pa.begin( &pm );
// 		pa.setRenderHint(QPainter::Antialiasing);
 		pa.setBrush( QBrush(penColor, (Qt::BrushStyle)i) );
		pa.drawRect( offset, offset, w-2*offset, h-2*offset);
		pa.end();
		ui.cbSymbolFillingPattern->addItem( QIcon(pm), "" );
	}

	ui.cbSymbolFillingPattern->setCurrentIndex(index);
}



//**********************************************************
//****************** SLOTS ********************************
//**********************************************************
/*!
	called if the color for the symbol filling was changed.
	Triggers the update of the ComboBox for the symbol fillings and filling patterns.
*/
void PlotStyleWidget::symbolFillingColorChanged(){
	this->fillSymbolFillingBox();
	this->fillSymbolFillingPatternBox();
}

/*!
	called if the symbol type was changed.
	Depending on the selected symbol type, enables/disables the widgets for the symboll filling.
*/
void PlotStyleWidget::symbolTypeChanged(int index){
	Symbol::SType t = Symbol::SType(index+1);
	kDebug()<<"Symbol"<<t<<" selected"<<endl;

	if (t==Symbol::SCROSS || t==Symbol::SDOT || t==Symbol::SPLUS
		   || t==Symbol::SSTAR || t==Symbol::SMINUS || t==Symbol::SPIPE || t==Symbol::SSTAR2){
		ui.lSymbolFilling->setEnabled(false);
		ui.cbSymbolFilling->setEnabled(false);
		ui.kcbSymbolFillingColor->setEnabled(false);
		ui.lSymbolFillingPattern->setEnabled(false);
		ui.cbSymbolFillingPattern->setEnabled(false);
	}else{
		ui.lSymbolFilling->setEnabled(true);
		ui.cbSymbolFilling->setEnabled(true);
		ui.kcbSymbolFillingColor->setEnabled(true);
		ui.lSymbolFillingPattern->setEnabled(true);
		ui.cbSymbolFillingPattern->setEnabled(true);

		this->fillSymbolFillingBox();
	}
}


void PlotStyleWidget::boxWidthStateChanged(int state){
	if (state==Qt::Checked)
		ui.sbBoxWidth->setEnabled(false);
	else
		ui.sbBoxWidth->setEnabled(true);
}


//TODO old code. Check whether everything, was taken over and remove the old stuff.
/*
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
