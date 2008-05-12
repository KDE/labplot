#include "PlotStyleWidget.h"
#include "../elements/Style.h"
#include "../elements/Symbol.h"
#include <KDebug>

PlotStyleWidget::PlotStyleWidget(QWidget* parent):QWidget(parent), PlotStyleWidgetInterface(){

	ui.setupUi(this);

	QStringList stylelist;
	stylelist<<i18n("Lines")<<i18n("NoCurve")<<i18n("Steps")<<i18n("Boxes")<<i18n("Impulses")<<i18n("Y Boxes");
	ui.cbLineType->insertItems(-1, stylelist);

	//TODO read out and set the default settings
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
	ui.cbLineType->setCurrentIndex(style->type());
	//TODO
}

void PlotStyleWidget::saveStyle(Style* style) const{
 	style->setType( (StyleType)ui.cbLineType->currentIndex() );
// 	style->setLineStyle(
	//TODO
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
