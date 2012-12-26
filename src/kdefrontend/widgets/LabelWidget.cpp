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
#include "../../backend/worksheet/plots/cartesian/Axis.h"
#include "../../tools/TeXRenderer.h"
#include <QMenu>
#include <QWidgetAction>
#include <QTimer>
#include <kdebug.h>
#include <kcharselect.h>

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
	ui.kcbTextColor->setColor(Qt::black); // default color

	//Icons
	ui.tbFontBold->setIcon( KIcon("format-text-bold") );
	ui.tbFontItalic->setIcon( KIcon("format-text-italic") );
	ui.tbFontUnderline->setIcon( KIcon("format-text-underline") );
	ui.tbFontStrikeOut->setIcon( KIcon("format-text-strikethrough") );
	ui.tbFontSuperScript->setIcon( KIcon("format-text-superscript") );
	ui.tbFontSubScript->setIcon( KIcon("format-text-subscript") );
	ui.tbSymbols->setIcon( KIcon("applications-education-mathematics") );
	
	//SLOTS
	// text properties
	connect(ui.chbTeX, SIGNAL(clicked(bool)), this, SLOT(teXUsedChanged(bool)) );
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
	connect(ui.tbSymbols, SIGNAL(clicked(bool)), this, SLOT(charMenu()));
	connect(ui.kfontRequester, SIGNAL(fontSelected(QFont)), this, SLOT(fontChanged(QFont)));
	
	// geometry
	connect( ui.cbPositionX, SIGNAL(currentIndexChanged(int)), this, SLOT(positionXChanged(int)) );
	connect( ui.cbPositionY, SIGNAL(currentIndexChanged(int)), this, SLOT(positionYChanged(int)) );
	connect( ui.sbPositionX, SIGNAL(valueChanged(double)), this, SLOT(customPositionXChanged(double)) );
	connect( ui.sbPositionY, SIGNAL(valueChanged(double)), this, SLOT(customPositionYChanged(double)) );
	connect( ui.cbHorizontalAlignment, SIGNAL(currentIndexChanged(int)), this, SLOT(horizontalAlignmentChanged(int)) );
	connect( ui.cbVerticalAlignment, SIGNAL(currentIndexChanged(int)), this, SLOT(verticalAlignmentChanged(int)) );
	connect( ui.sbRotation, SIGNAL(valueChanged(int)), this, SLOT(rotationChanged(int)) );
	connect( ui.sbOffset, SIGNAL(valueChanged(double)), this, SLOT(offsetChanged(double)) );

	connect( ui.chbVisible, SIGNAL(clicked(bool)), this, SLOT(visibilityChanged(bool)) );
}

LabelWidget::~LabelWidget() {}

void LabelWidget::setLabels(QList<TextLabel*> labels){
	m_labelsList = labels;
	
	//TODO: labplot craches sporadicaly when a new TextLabel is created upon a plot was already created.
	//The list is (sporadicaly) empty. This shouldn't happen!
	if (labels.isEmpty())
		return;
	
	m_label = labels.first();

	KConfig config("", KConfig::SimpleConfig);
	KConfigGroup group = config.group( "TextLabel" );
  	loadConfig(group);

	m_initializing = true;
	ui.chbVisible->setChecked( m_label->isVisible() );
	ui.teLabel->setText(m_label->text().text);
	m_initializing = false;

	connect( m_label, SIGNAL(positionChanged(QPointF&)), this, SLOT(labelPostionChanged(QPointF&)) );
	this->setFixedLabelMode(false);
}

void LabelWidget::setAxes(QList<Axis*> axes){
	m_labelsList.clear();
	foreach(Axis* axis, axes)
		m_labelsList.append(axis->title());
	
	m_axesList = axes;
	m_label = m_labelsList.first();

	KConfig config("", KConfig::SimpleConfig);
	KConfigGroup group = config.group( "TextLabel" );
  	loadConfig(group);

	m_initializing = true;
	ui.chbVisible->setChecked( m_label->isVisible() );
	ui.teLabel->setText(m_label->text().text);
	//TODO save the offset value in KConfigGroup? (available only for the axis labels)
	ui.sbOffset->setValue( Worksheet::convertFromSceneUnits(axes.first()->titleOffset(), Worksheet::Point) );
	m_initializing = false;

	this->setFixedLabelMode(true);
}

void LabelWidget::setFixedLabelMode(const bool b){
	ui.lPositionX->setVisible(!b);
	ui.cbPositionX->setVisible(!b);
	ui.sbPositionX->setVisible(!b);
	ui.lPositionY->setVisible(!b);
	ui.cbPositionY->setVisible(!b);
	ui.sbPositionY->setVisible(!b);
	ui.lHorizontalAlignment->setVisible(!b);
	ui.cbHorizontalAlignment->setVisible(!b);
	ui.lVerticalAlignment->setVisible(!b);
	ui.cbVerticalAlignment->setVisible(!b);	
	ui.lOffset->setVisible(b);
	ui.sbOffset->setVisible(b);
}

//**********************************************************
//****** SLOTs for changes triggered in LabelWidget*********
//**********************************************************

// text formating slots

void LabelWidget::textChanged(){
	if (m_initializing)
		return;

	if (ui.chbTeX->isChecked()) {
		QString text=ui.teLabel->toPlainText();
		TextLabel::TextWrapper wrapper(text, true);

		// TODO: this uses format of current selection only
		QTextCharFormat format = ui.teLabel->currentCharFormat();

		foreach(TextLabel* label, m_labelsList){
			label->setTeXFontSize(format.fontPointSize());
			label->setText(wrapper);
		}
	}else{
		//save an empty string instead of a html-string with empty body, if no text available in QTextEdit
		QString text;
		if (ui.teLabel->toPlainText() == "")
			text = "";
		else
			text = ui.teLabel->toHtml();

		TextLabel::TextWrapper wrapper(text, false);
		foreach(TextLabel* label, m_labelsList)
			label->setText(wrapper);
	}
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

	if(!ui.chbTeX->isChecked())
		ui.kcbTextColor->setColor(format.foreground().color());
	ui.kfontRequester->setFont(format.font());
}

void LabelWidget::teXUsedChanged(bool checked){
	if (m_initializing)
		return;

	//disable text editing elements if Tex-option is used
	ui.tbFontBold->setEnabled(!checked);
	ui.tbFontItalic->setEnabled(!checked);
	ui.tbFontUnderline->setEnabled(!checked);
	ui.tbFontSuperScript->setEnabled(!checked);
	ui.tbFontSubScript->setEnabled(!checked);
	ui.tbFontStrikeOut->setEnabled(!checked);
	ui.tbSymbols->setEnabled(!checked);

	QString text = checked ? ui.teLabel->toPlainText() : ui.teLabel->toHtml();
	TextLabel::TextWrapper wrapper(text, checked);
	foreach(TextLabel* label, m_labelsList)
		label->setText(wrapper);
}

void LabelWidget::textColorChanged(QColor color){
	if (m_initializing)
		return;

	ui.teLabel->setTextColor(color);
	foreach(TextLabel* label, m_labelsList)
		label->setTeXFontColor(color);
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

	// underline and strike-out not included
	ui.teLabel->setFontFamily(font.family());
	ui.teLabel->setFontPointSize(font.pointSize());
	ui.teLabel->setFontItalic(font.italic());
	ui.teLabel->setFontWeight(font.weight());
}

void LabelWidget::charMenu(){
	QMenu menu;
	KCharSelect selection(this,0,KCharSelect::SearchLine | KCharSelect::CharacterTable | KCharSelect::BlockCombos | KCharSelect::HistoryButtons);
	selection.setCurrentFont(ui.teLabel->currentFont());
	connect(&selection, SIGNAL(charSelected(QChar)), this, SLOT(insertChar(QChar)));
	connect(&selection, SIGNAL(charSelected(QChar)), &menu, SLOT(close()));

	QWidgetAction *widgetAction = new QWidgetAction(this);
	widgetAction->setDefaultWidget(&selection);
	menu.addAction(widgetAction);
	
	QPoint pos(-menu.sizeHint().width()+ui.tbSymbols->width(),-menu.sizeHint().height());
	menu.exec(ui.tbSymbols->mapToGlobal(pos));
}

void LabelWidget::insertChar(QChar c) {
	ui.teLabel->insertPlainText(QString(c));
}

// geometry slots

/*!
	called when label's current horizontal position relative to its parent (left, center, right, custom ) is changed.
*/
void LabelWidget::positionXChanged(int index){
	//Enable/disable the spinbox for the x- oordinates if the "custom position"-item is selected/deselected
	if (index == ui.cbPositionX->count()-1 ){
		ui.sbPositionX->setEnabled(true);
	}else{
		ui.sbPositionX->setEnabled(false);
	}
	
	if (m_initializing)
		return;
	
	foreach(TextLabel* label, m_labelsList)
		label->setHorizontalPosition(TextLabel::HorizontalPosition(index));
	
	//show the new y-postion if changed (calculated in TextLabel, if not custom position was specified)
	if (index != ui.cbPositionX->count()-1 ){
		m_initializing = true;
		ui.sbPositionX->setValue( Worksheet::convertFromSceneUnits(m_label->position().x(), Worksheet::Centimeter) );
		m_initializing = false;
	}
}

/*!
	called when label's current horizontal position relative to its parent (top, center, bottom, custom ) is changed.
*/
void LabelWidget::positionYChanged(int index){
	//Enable/disable the spinbox for the y- oordinates if the "custom position"-item is selected/deselected
	if (index == ui.cbPositionY->count()-1 ){
		ui.sbPositionY->setEnabled(true);
	}else{
		ui.sbPositionY->setEnabled(false);
	}
	
	if (m_initializing)
		return;

	foreach(TextLabel* label, m_labelsList)
		label->setVerticalPosition(TextLabel::VerticalPosition(index));

	//show the new y-postion if changed (calculated in TextLabel, if not custom position was specified)
	if (index != ui.cbPositionX->count()-1 ){
		m_initializing = true;
		ui.sbPositionY->setValue( Worksheet::convertFromSceneUnits(m_label->position().y(), Worksheet::Centimeter) );
		m_initializing = false;
	}
}

void LabelWidget::customPositionXChanged(double value){
	if (m_initializing)
		return;

	QPointF pos = m_label->position();
	pos.setX(Worksheet::convertToSceneUnits(value, Worksheet::Centimeter));
	foreach(TextLabel* label, m_labelsList)
		label->setPosition(pos);
}

void LabelWidget::customPositionYChanged(double value){
	if (m_initializing)
		return;

	QPointF pos = m_label->position();
	pos.setY(Worksheet::convertToSceneUnits(value, Worksheet::Centimeter));
	foreach(TextLabel* label, m_labelsList)
		label->setPosition(pos);
}

void LabelWidget::horizontalAlignmentChanged(int index){
	if (m_initializing)
		return;

	foreach(TextLabel* label, m_labelsList)
		label->setHorizontalAlignment(TextLabel::HorizontalAlignment(index));
}

void LabelWidget::verticalAlignmentChanged(int index){
	if (m_initializing)
		return;

	foreach(TextLabel* label, m_labelsList)
		label->setVerticalAlignment(TextLabel::VerticalAlignment(index));
}

void LabelWidget::rotationChanged(int value){
	if (m_initializing)
		return;

	foreach(TextLabel* label, m_labelsList)
		label->setRotationAngle(value);
}

void LabelWidget::offsetChanged(double value){
	if (m_initializing)
		return;

	foreach(Axis* axis, m_axesList)
		axis->setTitleOffset( Worksheet::convertToSceneUnits(value, Worksheet::Point) );
}

void LabelWidget::visibilityChanged(bool state){
	if (m_initializing)
		return;

	foreach(TextLabel* label, m_labelsList)
		label->setVisible(state);
}

//*********************************************************
//****** SLOTs for changes triggered in TextLabel *********
//*********************************************************
void LabelWidget::labelPostionChanged(QPointF& point){
	m_initializing = true;
	ui.sbPositionX->setValue( Worksheet::convertFromSceneUnits(point.x(), Worksheet::Centimeter) );
	ui.sbPositionY->setValue( Worksheet::convertFromSceneUnits(point.y(), Worksheet::Centimeter) );
	ui.cbPositionX->setCurrentIndex( TextLabel::hPositionCustom );
	ui.cbPositionY->setCurrentIndex( TextLabel::vPositionCustom );
	m_initializing = false;
}

//**********************************************************
//******************** SETTINGS ****************************
//**********************************************************
void LabelWidget::loadConfig(KConfigGroup &group) {
	if(m_label == NULL)
		return;

	m_initializing = true;
	
	//Text
	//TODO font, color etc.
	ui.chbTeX->setChecked(group.readEntry("TeXUsed", (bool) m_label->text().teXUsed));
	if(m_label->text().teXUsed)
		ui.kcbTextColor->setColor(group.readEntry("TeXFontColor", m_label->teXFontColor()));

	// Geometry
	ui.cbPositionX->setCurrentIndex( group.readEntry("TitlePositionX", (int) m_label->horizontalPosition()) );
	ui.sbPositionX->setValue( Worksheet::convertFromSceneUnits(group.readEntry("TitlePositionXValue", m_label->position().x()),Worksheet::Centimeter) );
	ui.cbPositionY->setCurrentIndex( group.readEntry("TitlePositionY", (int) m_label->verticalPosition()) );
	ui.sbPositionY->setValue( Worksheet::convertFromSceneUnits(group.readEntry("TitlePositionYValue", m_label->position().y()),Worksheet::Centimeter) );
	//TODO group.writeEntry("TitleOffset", ui.sbOffset->value());
	ui.cbHorizontalAlignment->setCurrentIndex( group.readEntry("TitleHorizontalAlignment", (int) m_label->horizontalAlignment()) );
	ui.cbVerticalAlignment->setCurrentIndex( group.readEntry("TitleVerticalAlignment", (int) m_label->verticalAlignment()) );
	ui.sbRotation->setValue( group.readEntry("TitleRotation", m_label->rotationAngle()) );
	
	m_initializing = false;
}

void LabelWidget::saveConfig(KConfigGroup &group) {
	//Text
	//TODO font, color etc.
	group.writeEntry("TeXUsed", ui.chbTeX->isChecked());
	group.writeEntry("TeXFontColor", ui.kcbTextColor->color());

	// Geometry
	group.writeEntry("TitlePositionX", ui.cbPositionX->currentIndex());
	group.writeEntry("TitlePositionXValue", Worksheet::convertToSceneUnits(ui.sbPositionX->value(),Worksheet::Centimeter) );
	group.writeEntry("TitlePositionY", ui.cbPositionY->currentIndex());
	group.writeEntry("TitlePositionYValue",  Worksheet::convertToSceneUnits(ui.sbPositionY->value(),Worksheet::Centimeter) );
	//TODO group.writeEntry("TitleOffset", ui.sbOffset->value());
	group.writeEntry("TitleHorizontalAlignment", ui.cbHorizontalAlignment->currentIndex());
	group.writeEntry("TitleVerticalAlignment", ui.cbVerticalAlignment->currentIndex());
	group.writeEntry("TitleRotation", ui.sbRotation->value());
}
