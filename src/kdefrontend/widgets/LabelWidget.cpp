/***************************************************************************
    File                 : LabelWidget.cc
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 Alexander Semke (alexander.semke*web.de)
    Copyright            : (C) 2012 Stefan Gerlach (stefan.gerlach*uni-konstanz.de)
                                    (replace * with @ in the email addresses)
    Description          : label settings widget

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
#include "LabelWidget.h"
#include "../../backend/worksheet/Worksheet.h"
#include "../../backend/worksheet/TextLabel.h"
#include "../../tools/TexRenderer.h"
#include <QMenu>
#include <kdebug.h>

/*!
	\class LabelWidget
 	\brief Widget for editing the properties of a TextLabel object, mostly used in an an appropriate dock widget.
 	
 	In order the properties of the label to be shown, \c loadConfig() has to be called with the correspondig KConfigGroup
 	(settings for a label in *Plot, Axis etc. or for an independent label on the worksheet).

 	\ingroup kdefrontend
 */

// see legacy/LabelWidget.cpp
LabelWidget::LabelWidget(QWidget *parent): QWidget(parent){
	ui.setupUi(this);
	
	QGridLayout* layout =static_cast<QGridLayout*>(this->layout());
  	layout->setContentsMargins(2,2,2,2);
	layout->setHorizontalSpacing(2);
	layout->setVerticalSpacing(2);

	//TODO: Populate the symbol menu
	//QMenu* menu = new QMenu(this);
	//QFont symbol("Symbol", 12, QFont::Bold);
 	//menu->setFont(symbol);
	//for(int i=97;i<122;i++)
	//	menu->addAction( QChar(i) );
	//ui.tbSymbols->setMenu(menu);
	//ui.tbSymbols->setDefaultAction(menu->menuAction());

	//Icons
	ui.tbFontBold->setIcon( KIcon("format-text-bold") );
	ui.tbFontItalic->setIcon( KIcon("format-text-italic") );
	ui.tbFontUnderline->setIcon( KIcon("format-text-underline") );
	ui.tbFontSuperScript->setIcon( KIcon("format-text-superscript") );
	ui.tbFontSubScript->setIcon( KIcon("format-text-subscript") );
	ui.tbFontStrikeOut->setIcon( KIcon("format-text-strikethrough") );
	
	//SLOTS
	// text properties
	connect(ui.teLabel, SIGNAL(textChanged()), this, SLOT(textChanged()));
	connect(ui.teLabel, SIGNAL(currentCharFormatChanged(QTextCharFormat)), 
			this, SLOT(charFormatChanged(QTextCharFormat)));
	connect(ui.kcbTextColor, SIGNAL(changed(QColor)), this, SLOT(textColorChanged(QColor)));
	connect(ui.tbFontBold, SIGNAL(clicked(bool)), this, SLOT(fontBoldChanged(bool)));
	connect(ui.tbFontItalic, SIGNAL(clicked(bool)), this, SLOT(fontItalicChanged(bool)));
	connect(ui.tbFontUnderline, SIGNAL(clicked(bool)), this, SLOT(fontUnderlineChanged(bool)));
	connect(ui.tbFontStrikeOut, SIGNAL(clicked(bool)), this, SLOT(fontStrikeOutChanged(bool)));
	connect(ui.tbFontSuperScript, SIGNAL(clicked(bool)), this, SLOT(fontSuperScriptChanged(bool)));
	connect(ui.tbFontSubScript, SIGNAL(clicked(bool)), this, SLOT(fontSubScriptChanged(bool)));
	connect(ui.kfontRequester, SIGNAL(fontSelected(QFont)), this, SLOT(fontChanged(QFont)));
	
	// Geometry
	connect( ui.cbPositionX, SIGNAL(currentIndexChanged(int)), this, SLOT(positionXChanged(int)) );
	connect( ui.cbPositionY, SIGNAL(currentIndexChanged(int)), this, SLOT(positionYChanged(int)) );
	connect( ui.sbPositionX, SIGNAL(valueChanged(double)), this, SLOT(customPositionXChanged(double)) );
	connect( ui.sbPositionY, SIGNAL(valueChanged(double)), this, SLOT(customPositionYChanged(double)) );
	connect( ui.sbRotation, SIGNAL(valueChanged(int)), this, SLOT(rotationChanged(int)) );
	
	connect( ui.chbTex, SIGNAL(clicked(bool)), this, SLOT(texUsedChanged(bool)) );
	
	//TODO remove later
	ui.lOffset->hide();
	ui.sbOffset->hide();
}

LabelWidget::~LabelWidget() {}

/*!
	sets the label to be edited to \c label.
*/
void LabelWidget::setLabel(TextLabel *label){
	m_label = label;
	//TODO: set ui.teLabel text
	ui.teLabel->setText("test label");
	//TODO: ui.kcbTextColor->setColor(m_label->color());
}

//TODO
void LabelWidget::setLabels(QList<TextLabel*> labels){
	m_label = labels.first();
	KConfig config("", KConfig::SimpleConfig);
	KConfigGroup group = config.group( "TextLabel" );
  	loadConfig(group);
}

//**********************************************************
//******************** SLOTS *******************************
//**********************************************************
void LabelWidget::textChanged(){
	if (m_initializing)
		return;

	if(!m_label) {
		kWarning()<<"m_label not defined";
		return;
	}

	// TODO: use rich text
	m_label->setText(ui.teLabel->toPlainText());
}

void LabelWidget::charFormatChanged(QTextCharFormat format){
	if (m_initializing)
		return;

	// update button state
	if(format.fontWeight() == QFont::Bold)
		ui.tbFontBold->setChecked(true);
	else
		ui.tbFontBold->setChecked(false);
	ui.tbFontItalic->setChecked(format.fontItalic());
	ui.tbFontUnderline->setChecked(format.fontUnderline());
	if(format.verticalAlignment() == QTextCharFormat::AlignSuperScript)
		ui.tbFontSuperScript->setChecked(true);
	else
		ui.tbFontSuperScript->setChecked(false);
	if(format.verticalAlignment() == QTextCharFormat::AlignSubScript)
		ui.tbFontSubScript->setChecked(true);
	else
		ui.tbFontSubScript->setChecked(false);
	ui.tbFontStrikeOut->setChecked(format.fontStrikeOut());
	// multiple colors and fonts possible?
	//ui.kcbTextColor->setColor(format.foreground().color());
}

void LabelWidget::texUsedChanged(bool checked){
	if (m_initializing)
		return;

	//TODO activate again m_label->setTexUsed(checked);
}

void LabelWidget::textColorChanged(QColor color){
	if (m_initializing)
		return;

	ui.teLabel->setTextColor(color);
}

void LabelWidget::fontBoldChanged(bool checked){
	if (m_initializing)
		return;

	if(checked)
		ui.teLabel->setFontWeight(QFont::Bold);
	else
		ui.teLabel->setFontWeight(QFont::Normal);
}

void LabelWidget::fontItalicChanged(bool checked){
	if (m_initializing)
		return;

	ui.teLabel->setFontItalic(checked);
}

void LabelWidget::fontUnderlineChanged(bool checked){
	if (m_initializing)
		return;

	ui.teLabel->setFontUnderline(checked);
}

void LabelWidget::fontStrikeOutChanged(bool checked){
	if (m_initializing)
		return;

	QTextCharFormat format = ui.teLabel->currentCharFormat();
	format.setFontStrikeOut(checked);
	ui.teLabel->setCurrentCharFormat(format);
}

void LabelWidget::fontSuperScriptChanged(bool checked){
	if (m_initializing)
		return;

	QTextCharFormat format = ui.teLabel->currentCharFormat();
	if (checked)
		format.setVerticalAlignment(QTextCharFormat::AlignSuperScript);
	else 
		format.setVerticalAlignment(QTextCharFormat::AlignNormal);
	
	ui.teLabel->setCurrentCharFormat(format);
}

void LabelWidget::fontSubScriptChanged(bool checked){
	if (m_initializing)
		return;

	QTextCharFormat format = ui.teLabel->currentCharFormat();
	if (checked)
		format.setVerticalAlignment(QTextCharFormat::AlignSubScript);
	else 
		format.setVerticalAlignment(QTextCharFormat::AlignNormal);
	
	ui.teLabel->setCurrentCharFormat(format);
}

void LabelWidget::fontChanged(QFont font){
	if (m_initializing)
		return;

	ui.teLabel->setCurrentFont(font);
}

/*!
	called if the current position of the title is changed in the combobox.
	Enables/disables the lineedits for x- and y-coordinates if the "custom"-item is selected/deselected
*/
void LabelWidget::positionXChanged(int index){
	if (index == ui.cbPositionX->count()-1 ){
		ui.sbPositionX->setEnabled(true);
	}else{
		ui.sbPositionX->setEnabled(false);
	}
	
	if (m_initializing)
		return;
	
	m_label->setHorizontalPosition(TextLabel::HorizontalPosition(index));
}

void LabelWidget::positionYChanged(int index){
	if (index == ui.cbPositionY->count()-1 ){
		ui.sbPositionY->setEnabled(true);
	}else{
		ui.sbPositionY->setEnabled(false);
	}
	
	if (m_initializing)
		return;

	m_label->setVerticalPosition(TextLabel::VerticalPosition(index));
}

void LabelWidget::customPositionXChanged(double value){
	if (m_initializing)
		return;

	QPointF pos = m_label->position();
	pos.setY(Worksheet::convertToSceneUnits(value, Worksheet::Centimeter));
	m_label->setPosition(pos);
}

void LabelWidget::customPositionYChanged(double value){
	if (m_initializing)
		return;

	QPointF pos = m_label->position();
	pos.setY(Worksheet::convertToSceneUnits(value, Worksheet::Centimeter));
	m_label->setPosition(pos);
}

void LabelWidget::rotationChanged(int value){
	if (m_initializing)
		return;

	m_label->setRotationAngle(value);
}

//**********************************************************
//******************** SETTINGS ****************************
//**********************************************************
//TODO
void LabelWidget::loadConfig(KConfigGroup &group) {
	if(m_label == NULL)
		return;

	m_initializing = true;

	//Text
	ui.teLabel->setPlainText(group.readEntry("LabelText", m_label->text()));

	// Geometry
//	ui.cbPositionX->setCurrentIndex( group.readEntry("TitlePositionX", (int) m_label->position()) );
	ui.sbPositionX->setValue( Worksheet::convertFromSceneUnits(group.readEntry("TitlePositionXValue", m_label->position().x()),Worksheet::Centimeter) );
//	ui.cbPositionY->setCurrentIndex( group.readEntry("TitlePositionY", (int) m_label->positionY()) );
	ui.sbPositionY->setValue( Worksheet::convertFromSceneUnits(group.readEntry("TitlePositionYValue", m_label->position().y()),Worksheet::Centimeter) );
	//group.writeEntry("TitleOffset", ui.sbOffset->value());
	ui.cbHorizontalAlignment->setCurrentIndex( group.readEntry("TitleHorizontalAlignment", (int) m_label->horizontalAlignment()) );
	ui.cbVerticalAlignment->setCurrentIndex( group.readEntry("TitleVerticalAlignment", (int) m_label->verticalAlignment()) );
	ui.sbRotation->setValue( group.readEntry("TitleRotation", m_label->rotationAngle()) );
	
	m_initializing = false;
}

//TODO
void LabelWidget::saveConfig(KConfigGroup &group) {
	//Text
	group.writeEntry("LabelText", ui.teLabel->toPlainText());

	// Geometry
	group.writeEntry("TitlePositionX", ui.cbPositionX->currentIndex());
	group.writeEntry("TitlePositionXValue", Worksheet::convertToSceneUnits(ui.sbPositionX->value(),Worksheet::Centimeter) );
	group.writeEntry("TitlePositionY", ui.cbPositionY->currentIndex());
	group.writeEntry("TitlePositionYValue",  Worksheet::convertToSceneUnits(ui.sbPositionY->value(),Worksheet::Centimeter) );
	//group.writeEntry("TitleOffset", ui.sbOffset->value());
	group.writeEntry("TitleHorizontalAlignment", ui.cbHorizontalAlignment->currentIndex());
	group.writeEntry("TitleVerticalAlignment", ui.cbVerticalAlignment->currentIndex());
	group.writeEntry("TitleRotation", ui.sbRotation->value());
}
