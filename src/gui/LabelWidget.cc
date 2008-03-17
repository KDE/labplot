#include "LabelWidget.h"
#include "../Label.h"

#include <kfontrequester.h>
#include <kcolorbutton.h>

LabelWidget::LabelWidget(QWidget *parent): QWidget(parent){

	ui.setupUi(this);

	//******************************
	//Manage to use kde4-widgets in designer and move the following stuff to the ui-file
	fontRequester = new KFontRequester(ui.gbText);
	qobject_cast<QGridLayout*>(ui.gbText->layout())->addWidget(fontRequester, 0, 2, 1, 2);

	colorButton= new KColorButton(ui.gbText);
	qobject_cast<QGridLayout*>(ui.gbText->layout())->addWidget(colorButton, 1, 2, 1, 2);

	//TODO colorbutton for the filling color.
	//*************************

	//Populate the comboboxes
	QFont symbol("Symbol", 12, QFont::Bold);
 	ui.cbSmallGreekLetters->setFont(symbol);
	for(int i=97;i<122;i++)
		ui.cbSmallGreekLetters->addItem(QChar(i));

	ui.cbBigGreekLetters->setFont(symbol);
	for(int i=68;i<90;i++)
		ui.cbBigGreekLetters->addItem(QChar(i));

	ui.cbSymbolLetters->setFont(symbol);
 	ui.cbSymbolLetters->addItem(QChar(34));
	ui.cbSymbolLetters->addItem(QChar(36));
	ui.cbSymbolLetters->addItem(QChar(39));
	ui.cbSymbolLetters->addItem(QChar(64));
	for(int i=160;i<255;i++)
		ui.cbSymbolLetters->addItem(QChar(i));

	//Validators
	ui.lePositionX->setValidator( new QDoubleValidator(ui.lePositionX) );
	ui.lePositionY->setValidator( new QDoubleValidator(ui.lePositionY) );
	ui.leRotation->setValidator( new QDoubleValidator(ui.leRotation) );

	//Slots
	connect( ui.cbPosition, SIGNAL(currentIndexChanged(int)), this, SLOT(positionChanged(int)) );
	connect( ui.rbFilling0, SIGNAL(toggled(bool)), this, SLOT( fillingChanged(bool)) );
	connect( ui.rbFilling1, SIGNAL(toggled(bool)), this, SLOT( fillingChanged(bool)) );

	//TODO These signals doesn't work at the moment.
// 	connect( colorButton, SIGNAL(changed(const QColor& c)), ui.teLabel, SLOT(setTextColor(const QColor& c)) );
// 	connect( fontRequester, SIGNAL(fontSelected(const QFont& f)), ui.teLabel, SLOT(setCurrentFont(const QFont& f)) );
	connect( colorButton, SIGNAL(changed(const QColor& c)), this, SLOT(fontColorChanged(const QColor& c)) );
	connect( fontRequester, SIGNAL(fontSelected(const QFont& f)), this, SLOT(fontChanged(const QFont& f)) );

	connect( ui.chbUseTex, SIGNAL(stateChanged(int)), this, SLOT(useTexChanged(int)) );
	connect( ui.bFontBold, SIGNAL(toggled(bool)), this, SLOT( fontBoldToggled(bool)) );
	connect( ui.bFontItalic, SIGNAL(toggled(bool)), this, SLOT( fontItalicToggled(bool)) );
	connect( ui.bFontUnderline, SIGNAL(toggled(bool)), this, SLOT( fontUnderlineToggled(bool)) );
	connect( ui.bFontSuperscript, SIGNAL(toggled(bool)), this, SLOT( fontSuperscriptToggled(bool)) );
	connect( ui.bFontSubscript, SIGNAL(toggled(bool)), this, SLOT( fontSubscriptToggled(bool)) );

	connect( ui.cbSmallGreekLetters, SIGNAL(activated(const QString &)), this, SLOT(insertSymbol(const QString &)));
	connect( ui.cbBigGreekLetters, SIGNAL(activated(const QString &)), this, SLOT(insertSymbol(const QString &)));
	connect( ui.cbSymbolLetters, SIGNAL(activated(const QString &)), this, SLOT(insertSymbol(const QString &)));
}

LabelWidget::~LabelWidget() {}

void LabelWidget::setLabel(const Label* l) {
	//kdDebug()<<"RTW::update()"<<endl;
	//TODO add some "asserts"!!!
	/*
	//Alignment
	ui.cbPosition->setCurrentIndex( l->position() );
	ui.lePositionX->setText( QString::number(l->x()) );
	ui.lePositionY->setText( QString::number(l->y()) );
	ui.leRotation->setText( QString::number(l->rotation()) );

	//Background
	if ( l->isTransparent() )
		ui.rbFilling0->setChecked(true);
	else
		ui.rbFilling0->setChecked(true);

	//backgroundColorButton->setColor( l->backgroundColor() );
	ui.chbBox->setChecked( l->hasBox() );
	ui.chbShadow->setChecked( l->hasShadow() );

	//Text
	fontRequester->setFont( l->font() );
	ui.teLabel->setFont( l->font() );
	colorButton->setColor( l->fontColor() );
	ui.chbUseTex->setChecked( l->isTeXLabel() );
	*/
}

void LabelWidget::setLabelRotationEnabled(const bool b){
	ui.lRotation->setEnabled(b);
	ui.leRotation->setEnabled(b);
	ui.lDegree->setEnabled(b);
}

//TODO
void LabelWidget::saveLabel(Label*) const{
	/*
	l->setTeXLabel(texcb->isChecked());
	if(l->isTeXLabel())
		te->setTextFormat(Qt::PlainText);
	*/
//	kdDebug()<<"LabelWidget::apply()"<<endl;
//	kdDebug()<<"	RICHTEXT = "<<te->text()<<endl;

	// doesn't work :-(
/*	const QChar *utf8text = te->text().unicode();

	te->setTextFormat(Qt::PlainText);
	kdDebug()<<"	PLAINTEXT = "<<te->text().utf8()<<endl;
	kdDebug()<<"	PLAINTEXT = "<<te->text().local8Bit()<<endl;
	kdDebug()<<"	PLAINTEXT = "<<te->text().unicode()<<endl;
	kdDebug()<<"	UTF8 TEXT = "<<utf8text<<endl;
*/

	//****
	/*
	l->setTitle(te->text());
	l->setPosition(xle->text().toDouble(),yle->text().toDouble());
	l->setBoxed(boxedcb->isChecked());
	l->setRotation(rotle->text().toDouble());
	l->setTransparent(transcb->isChecked());
	l->setBackgroundColor(bgcolor->color());
	*/
}


//***********************************************************************************
//**********************************    SLOTS   **************************************
//***********************************************************************************
/*!
	called if the current position of the title is changed in the combobox.
	Enables/disables the lineedits for x- and y-coordinates if the "custom"-item is selected/deselected
*/
void LabelWidget::positionChanged(int index){
	if (index == ui.cbPosition->count()-1 ){
		ui.lePositionX->setEnabled(true);
		ui.lePositionY->setEnabled(true);
	}else{
		ui.lePositionX->setEnabled(false);
		ui.lePositionY->setEnabled(false);
	}
}

/*!

*/
void LabelWidget::fillingChanged(bool){
	if (ui.rbFilling0->isChecked())
		ui.bFillingColor->setEnabled(false);
	else
		ui.bFillingColor->setEnabled(true);
}

void LabelWidget::fillingColorClicked(){

}

void LabelWidget::fontChanged(const QFont& font){
	ui.teLabel->setCurrentFont(font);
	ui.teLabel->setFocus();
}

void LabelWidget::fontColorChanged(const QColor& color){
	ui.teLabel->setTextColor(color);
	ui.teLabel->setFocus();
}

void LabelWidget::useTexChanged(int state){
	QFont font;
	if (state==Qt::Checked){	//use Tex-syntax
		//remove bold-, italic- etc. properties in texteditwidget
		font.setBold(false);
		font.setItalic(false);
		font.setUnderline(false);

		ui.frameFontOptions->setEnabled(false);
	}else{
		font=fontRequester->font();
		font.setBold(ui.bFontBold->isChecked());
		font.setItalic(ui.bFontItalic->isChecked());
		font.setUnderline(ui.bFontUnderline->isChecked());

		ui.frameFontOptions->setEnabled(true);
	}
	ui.teLabel->setCurrentFont(font);
}

void LabelWidget::fontBoldToggled(bool checked){
	QFont font=ui.teLabel->font();
	font.setBold(checked);
	ui.teLabel->setCurrentFont(font);
	ui.teLabel->setFocus();
}


void LabelWidget::fontItalicToggled(bool checked){
	if (checked)
		ui.teLabel->setFontItalic(true);
	else
		ui.teLabel->setFontItalic(false);

	ui.teLabel->setFocus();
}

void LabelWidget::fontUnderlineToggled(bool checked){
	if (checked)
		ui.teLabel->setFontUnderline(true);
	else
		ui.teLabel->setFontUnderline(false);

	ui.teLabel->setFocus();
}

void LabelWidget::fontSuperscriptToggled(bool checked){
	QTextCharFormat format;
	if (checked)
		format.setVerticalAlignment(QTextCharFormat::AlignSuperScript);
	else
		format.setVerticalAlignment(QTextCharFormat::AlignNormal);

	ui.teLabel->mergeCurrentCharFormat(format);
	ui.teLabel->setFocus();
}


void LabelWidget::fontSubscriptToggled(bool checked){
	QTextCharFormat format;
	if (checked)
		format.setVerticalAlignment(QTextCharFormat::AlignSubScript);
	else
		format.setVerticalAlignment(QTextCharFormat::AlignNormal);

	ui.teLabel->mergeCurrentCharFormat(format);
	ui.teLabel->setFocus();
}

void LabelWidget::insertSymbol(const QString& string) {
	QString fontFamily=ui.teLabel->fontFamily();
	ui.teLabel->setFontFamily("Symbol");
	ui.teLabel->insertPlainText(string);
	ui.teLabel->setFocus();
	ui.teLabel->setFontFamily(fontFamily);
	ui.teLabel->setFocus();
}
