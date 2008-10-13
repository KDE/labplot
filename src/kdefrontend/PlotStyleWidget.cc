/***************************************************************************
    File                 : PlotStyleWidget.cc
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Alexander Semke
    Email (use @ for *)  : alexander.semke*web.de
    Description          : plot style widget
                           
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
#include "PlotStyleWidget.h"
#include "../elements/Style.h"
#include <KDebug>

PlotStyleWidget::PlotStyleWidget(QWidget* parent):QWidget(parent), PlotStyleWidgetInterface(){

	ui.setupUi(this);

	QStringList stylelist;
	stylelist<<i18n("Lines")<<i18n("Steps")<<i18n("Boxes")<<i18n("Impulses")<<i18n("Y Boxes");
	ui.cbLineType->insertItems(-1, stylelist);

	//TODO read out and set the default settings
	ui.kcbLineColor->setColor(Qt::black);
	this->fillLineStyleBox();
	ui.cbLineStyle->setCurrentIndex(0);

	ui.kcbAreaFillingColor->setColor(Qt::green);
	this->fillAreaFillingPatternBox();
	ui.cbFillBrushStyle->setCurrentIndex(0);

	ui.kcbSymbolColor->setColor(Qt::black);
	this->fillSymbolTypeBox();
	ui.cbSymbolType->setCurrentIndex(0);
	this->symbolTypeChanged(0);

	ui.kcbSymbolFillColor->setColor(Qt::black);
	this->fillSymbolFillingPatternBox();
	ui.cbSymbolFillBrushStyle->setCurrentIndex(0);
	this->fillSymbolFillingBox();
	ui.cbSymbolFillType->setCurrentIndex(0);

	//Slots
	connect( ui.cbSymbolType, SIGNAL(currentIndexChanged(int)), this, SLOT(symbolTypeChanged(int)) );
	connect( ui.kcbLineColor, SIGNAL(changed (const QColor &)), this, SLOT(fillLineStyleBox()) );
	connect( ui.sbLineWidth, SIGNAL(valueChanged(int)), this, SLOT(fillLineStyleBox()) );

	connect( ui.kcbAreaFillingColor, SIGNAL(changed (const QColor &)), this, SLOT(fillAreaFillingPatternBox()) );

	connect( ui.cbSymbolType, SIGNAL(currentIndexChanged(int)), this, SLOT(symbolTypeChanged(int)) );
	connect( ui.kcbSymbolColor, SIGNAL(changed (const QColor &)), this, SLOT(fillSymbolTypeBox()) );
	connect( ui.kcbSymbolFillColor, SIGNAL(changed (const QColor &)), this, SLOT(symbolFillingColorChanged()) );
	connect( ui.cbSymbolFillBrushStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(fillSymbolFillingBox()) );

	connect( ui.chbAutoBoxWidth, SIGNAL(stateChanged (int)), this, SLOT(boxWidthStateChanged(int)) );
}

PlotStyleWidget::~PlotStyleWidget(){}

/*!
	displays the properties of the Style \c style in the widget.
*/
void PlotStyleWidget::setStyle(const Style* style){
	//LineStyle
	ui.chbLineEnabled->setChecked( style->isLineEnabled() );
	ui.cbLineType->setCurrentIndex( style->type() );
 	ui.cbLineStyle->setCurrentIndex( style->lineStyle()-1 );
	ui.kcbLineColor->setColor( style->lineColor() );
	ui.sbLineWidth->setValue( style->lineWidth()  );

	//Area filling
	ui.chbAreaFillingEnabled->setChecked( style->isFilled()  );
	ui.cbFillBrushStyle->setCurrentIndex( style->fillBrushStyle()  );
	ui.kcbAreaFillingColor->setColor( style->fillColor()  );

	//Symbol
	Symbol* symbol=const_cast<Style*>(style)->symbol();
	ui.chbSymbolEnabled->setChecked( style->isSymbolEnabled() );
	ui.cbSymbolType->setCurrentIndex( symbol->type()-1 );
	ui.kcbSymbolColor->setColor( symbol->color()  );
	ui.sbSymbolSize->setValue( symbol->size()  );
	ui.cbSymbolFillType->setCurrentIndex( symbol->fillType() );
	ui.kcbSymbolFillColor->setColor( symbol->fillColor() );
	ui.cbSymbolFillBrushStyle->setCurrentIndex( symbol->fillBrushStyle()-1 );

	//Misc
	ui.chbAutoBoxWidth->setChecked( style->isAutoBoxWidth() );
	ui.sbBoxWidth->setValue( style->boxWidth() );
 	ui.chbSortPoints->setChecked( style->isPointsSorting() );
}

/*!
	save the widget data in the Style \c style.
*/
void PlotStyleWidget::saveStyle(Style* style) const{
	//Line style
	style->setLineEnabled( ui.chbLineEnabled->isChecked() );
 	style->setType( (Style::StyleType)ui.cbLineType->currentIndex() );
 	style->setLineStyle( Qt::PenStyle(ui.cbLineStyle->currentIndex()+1) );
	style->setLineColor( ui.kcbLineColor->color() );
	style->setLineWidth( ui.sbLineWidth->value() );

	//Area filling
	style->setFilled( ui.chbAreaFillingEnabled->isChecked() );
	style->setFillBrushStyle( (Qt::BrushStyle)ui.cbFillBrushStyle->currentIndex() );
	style->setFillColor( ui.kcbAreaFillingColor->color() );

	//Symbol
	style->setSymbolEnabled( ui.chbSymbolEnabled->isChecked() );
	style->symbol()->setType( Symbol::SymbolType(ui.cbSymbolType->currentIndex() +1) );
	style->symbol()->setColor( ui.kcbSymbolColor->color() );
	style->symbol()->setSize( ui.sbSymbolSize->value() );
	style->symbol()->setFillType( Symbol::SymbolFillType(ui.cbSymbolFillType->currentIndex()) );
	style->symbol()->setFillColor( ui.kcbSymbolFillColor->color() );
	style->symbol()->setFillBrushStyle( Qt::BrushStyle(ui.cbSymbolFillBrushStyle->currentIndex()+1) );

	//Misc
	style->setAutoBoxWidth( ui.chbAutoBoxWidth->isChecked() );
	style->setBoxWidth( ui.sbBoxWidth->value() );
	style->setPointsSorting( ui.chbSortPoints->isChecked() );
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
// 		pa.setRenderHint(QPainter::Antialiasing);
 		pa.setBrush( QBrush(penColor, (Qt::BrushStyle)i) );
		pa.drawRect( offset, offset, w-2*offset, h-2*offset);
		pa.end();
		ui.cbFillBrushStyle->addItem( QIcon(pm), "" );
	}

	ui.cbFillBrushStyle->setCurrentIndex(index);
}

/*!
	fills the ComboBox for the symbol type with all possible types defined in \c Symbol.h
	Called on start and if the symbol color is changed.
*/
void PlotStyleWidget::fillSymbolTypeBox(){
	int index = ui.cbSymbolType->currentIndex();
	ui.cbSymbolType->clear();

	QColor color=ui.kcbSymbolColor->color();
// 	QColor fillColor=ui.kcbSymbolFillColor->color();
// 	Qt::BrushStyle fillBrushStyle=Qt::BrushStyle(ui.cbSymbolFillBrushStyle->currentIndex());
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
		Symbol::draw(&pa, new QPoint(size/2, size/2), (Symbol::SymbolType)i, color, size-2*offset, Symbol::FNONE);
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
		int index = ui.cbSymbolFillType->currentIndex();
		ui.cbSymbolFillType->clear();

		QColor color=ui.kcbSymbolColor->color();
		Symbol::SymbolType symbolType=Symbol::SymbolType(ui.cbSymbolType->currentIndex() +1);
		QColor fillColor=ui.kcbSymbolFillColor->color();
		Qt::BrushStyle fillBrushStyle=Qt::BrushStyle(ui.cbSymbolFillBrushStyle->currentIndex()+1);
		QPainter pa;

		int offset=5;
		int size=20;
		QPixmap pm( size, size );

		int c=Symbol::fillingTypeCount();
		ui.cbSymbolFillType->addItem( i18n("no filling") );
		for (int i=1; i<c; i++) {
			pm.fill(Qt::transparent);
			pa.begin( &pm );
			pa.setRenderHint(QPainter::Antialiasing);

			//draw the symbol in the middle of the pixmap
			Symbol::draw(&pa, new QPoint(size/2, size/2), symbolType, color, size-2*offset, (Symbol::SymbolFillType)i, fillColor, fillBrushStyle);
			pa.end();

			ui.cbSymbolFillType->addItem(QIcon(pm), "");
		}
		ui.cbSymbolFillType->setCurrentIndex(index);
}

/*!
	fills the ComboBox for the symbol filling patterns with the 14 possible Qt::BrushStyles.
	Called on start and if the symbol filling color is changed.
*/
void PlotStyleWidget::fillSymbolFillingPatternBox() {
	int index=ui.cbSymbolFillBrushStyle->currentIndex();
	ui.cbSymbolFillBrushStyle->clear();

	QPainter pa;
	int offset=5;
	int w=ui.cbSymbolFillBrushStyle->width() - 2*offset;
	int h=ui.cbSymbolFillBrushStyle->height() - 2*offset;
	QPixmap pm( w, h );
	ui.cbSymbolFillBrushStyle->setIconSize( QSize(w,h) );

 	QColor penColor = ui.kcbSymbolFillColor->color();
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
		ui.cbSymbolFillBrushStyle->addItem( QIcon(pm), "" );
	}

	ui.cbSymbolFillBrushStyle->setCurrentIndex(index);
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
	Symbol::SymbolType t = Symbol::SymbolType(index+1);
	kDebug()<<"Symbol"<<t<<" selected"<<endl;

	if (t==Symbol::SCROSS || t==Symbol::SDOT || t==Symbol::SPLUS
		   || t==Symbol::SSTAR || t==Symbol::SMINUS || t==Symbol::SPIPE || t==Symbol::SSTAR2){
		ui.lSymbolFilling->setEnabled(false);
		ui.cbSymbolFillType->setEnabled(false);
		ui.kcbSymbolFillColor->setEnabled(false);
		ui.lSymbolFillingPattern->setEnabled(false);
		ui.cbSymbolFillBrushStyle->setEnabled(false);
	}else{
		ui.lSymbolFilling->setEnabled(true);
		ui.cbSymbolFillType->setEnabled(true);
		ui.kcbSymbolFillColor->setEnabled(true);
		ui.lSymbolFillingPattern->setEnabled(true);
		ui.cbSymbolFillBrushStyle->setEnabled(true);

		this->fillSymbolFillingBox();
	}
}


void PlotStyleWidget::boxWidthStateChanged(int state){
	if (state==Qt::Checked)
		ui.sbBoxWidth->setEnabled(false);
	else
		ui.sbBoxWidth->setEnabled(true);
}
