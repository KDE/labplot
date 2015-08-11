/***************************************************************************
    File                 : LabelWidget.cc
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008-2015 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2012-2014 Stefan Gerlach (stefan.gerlach@uni-konstanz.de)
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
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/Axis.h"
#include "tools/TeXRenderer.h"

#include <QWidgetAction>

#include <KCharSelect>
#include <KLocalizedString>
#include <QMenu>

/*!
	\class LabelWidget
	\brief Widget for editing the properties of a TextLabel object, mostly used in an an appropriate dock widget.

	In order the properties of the label to be shown, \c loadConfig() has to be called with the correspondig KConfigGroup
	(settings for a label in *Plot, Axis etc. or for an independent label on the worksheet).

	\ingroup kdefrontend
 */

// see legacy/LabelWidget.cpp
LabelWidget::LabelWidget(QWidget *parent): QWidget(parent), m_dateTimeMenu(new QMenu(this)) {
	ui.setupUi(this);

	m_dateTimeMenu->setSeparatorsCollapsible(false); //we don't want the first separator to be removed

	QGridLayout* layout =static_cast<QGridLayout*>(this->layout());
	layout->setContentsMargins(2,2,2,2);
	layout->setHorizontalSpacing(2);
	layout->setVerticalSpacing(2);
	ui.kcbFontColor->setColor(Qt::black); // default color

	//Icons
	ui.tbFontBold->setIcon( QIcon::fromTheme("format-text-bold") );
	ui.tbFontItalic->setIcon( QIcon::fromTheme("format-text-italic") );
	ui.tbFontUnderline->setIcon( QIcon::fromTheme("format-text-underline") );
	ui.tbFontStrikeOut->setIcon( QIcon::fromTheme("format-text-strikethrough") );
	ui.tbFontSuperScript->setIcon( QIcon::fromTheme("format-text-superscript") );
	ui.tbFontSubScript->setIcon( QIcon::fromTheme("format-text-subscript") );
	ui.tbSymbols->setIcon( QIcon::fromTheme("format-text-symbol") );
	ui.tbDateTime->setIcon( QIcon::fromTheme("chronometer") );
	ui.tbTexUsed->setIconSize(QSize(20, 20));
	ui.tbTexUsed->setIcon( QIcon::fromTheme("TeX_logo") );

	//Positioning and alignment
	ui.cbPositionX->addItem(i18n("left"));
	ui.cbPositionX->addItem(i18n("center"));
	ui.cbPositionX->addItem(i18n("right"));
	ui.cbPositionX->addItem(i18n("custom"));

	ui.cbPositionY->addItem(i18n("top"));
	ui.cbPositionY->addItem(i18n("center"));
	ui.cbPositionY->addItem(i18n("bottom"));
	ui.cbPositionY->addItem(i18n("custom"));

	ui.cbHorizontalAlignment->addItem(i18n("left"));
	ui.cbHorizontalAlignment->addItem(i18n("center"));
	ui.cbHorizontalAlignment->addItem(i18n("right"));

	ui.cbVerticalAlignment->addItem(i18n("top"));
	ui.cbVerticalAlignment->addItem(i18n("center"));
	ui.cbVerticalAlignment->addItem(i18n("bottom"));

	//SLOTS
	// text properties
	connect(ui.tbTexUsed, SIGNAL(clicked(bool)), this, SLOT(teXUsedChanged(bool)) );
	connect(ui.teLabel, SIGNAL(textChanged()), this, SLOT(textChanged()));
	connect(ui.teLabel, SIGNAL(currentCharFormatChanged(QTextCharFormat)),
			this, SLOT(charFormatChanged(QTextCharFormat)));
	connect(ui.kcbFontColor, SIGNAL(changed(QColor)), this, SLOT(fontColorChanged(QColor)));
	connect(ui.tbFontBold, SIGNAL(clicked(bool)), this, SLOT(fontBoldChanged(bool)));
	connect(ui.tbFontItalic, SIGNAL(clicked(bool)), this, SLOT(fontItalicChanged(bool)));
	connect(ui.tbFontUnderline, SIGNAL(clicked(bool)), this, SLOT(fontUnderlineChanged(bool)));
	connect(ui.tbFontStrikeOut, SIGNAL(clicked(bool)), this, SLOT(fontStrikeOutChanged(bool)));
	connect(ui.tbFontSuperScript, SIGNAL(clicked(bool)), this, SLOT(fontSuperScriptChanged(bool)));
	connect(ui.tbFontSubScript, SIGNAL(clicked(bool)), this, SLOT(fontSubScriptChanged(bool)));
	connect(ui.tbSymbols, SIGNAL(clicked(bool)), this, SLOT(charMenu()));
	connect(ui.tbDateTime, SIGNAL(clicked(bool)), this, SLOT(dateTimeMenu()));
	connect(m_dateTimeMenu, SIGNAL(triggered(QAction*)), this, SLOT(insertDateTime(QAction*)) );
	connect(ui.kfontRequester, SIGNAL(fontSelected(QFont)), this, SLOT(fontChanged(QFont)));
	connect(ui.sbFontSize, SIGNAL(valueChanged(int)), this, SLOT(fontSizeChanged(int)) );

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

	//TODO: https://bugreports.qt.io/browse/QTBUG-25420
	ui.tbFontUnderline->hide();
	ui.tbFontStrikeOut->hide();
}

void LabelWidget::setLabels(QList<TextLabel*> labels){
	m_labelsList = labels;
	m_label = labels.first();

	ui.lOffset->hide();
	ui.sbOffset->hide();

	this->load();
	initConnections();
}

void LabelWidget::setAxes(QList<Axis*> axes){
	m_labelsList.clear();
	foreach(Axis* axis, axes) {
		m_labelsList.append(axis->title());
		connect(axis, SIGNAL(titleOffsetChanged(float)), this, SLOT(labelOffsetChanged(float)) );
		connect(axis->title(), SIGNAL(rotationAngleChanged(float)), this, SLOT(labelRotationAngleChanged(float)) );
	}

	m_axesList = axes;
	m_label = m_labelsList.first();

	this->load();
	initConnections();
}

void LabelWidget::initConnections() {
	connect( m_label, SIGNAL(textWrapperChanged(TextLabel::TextWrapper)),
				this, SLOT(labelTextWrapperChanged(TextLabel::TextWrapper)) );
	connect( m_label, SIGNAL(teXFontSizeChanged(int)),
				this, SLOT(labelTeXFontSizeChanged(int)) );
	connect( m_label, SIGNAL(teXFontColorChanged(QColor)),
				this, SLOT(labelTeXFontColorChanged(QColor)) );
	connect( m_label, SIGNAL(positionChanged(TextLabel::PositionWrapper)),
				this, SLOT(labelPositionChanged(TextLabel::PositionWrapper)) );
	connect( m_label, SIGNAL(horizontalAlignmentChanged(TextLabel::HorizontalAlignment)),
				this, SLOT(labelHorizontalAlignmentChanged(TextLabel::HorizontalAlignment)) );
	connect( m_label, SIGNAL(verticalAlignmentChanged(TextLabel::VerticalAlignment)),
				this, SLOT(labelVerticalAlignmentChanged(TextLabel::VerticalAlignment)) );
	connect( m_label, SIGNAL(rotationAngleChanged(float)), this, SLOT(labelRotationAngleChanged(float)) );
	connect( m_label, SIGNAL(visibleChanged(bool)), this, SLOT(labelVisibleChanged(bool)) );
}

/*!
 * enables/disables the "fixed label"-mode, used when displaying
 * the properties of axis' title label.
 * In this mode, in the "geometry"-part only the offset (offset to the axis)
 * and the rotation of the label are available.
 */
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

/*!
 * enables/disables all geometry relevant widgets.
 * Used when displaying legend's title label.
 */
void LabelWidget::setNoGeometryMode(const bool b) {
	ui.lGeometry->setVisible(!b);
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
	ui.lOffset->setVisible(!b);
	ui.sbOffset->setVisible(!b);
	ui.lRotation->setVisible(!b);
	ui.sbRotation->setVisible(!b);
}

//**********************************************************
//****** SLOTs for changes triggered in LabelWidget ********
//**********************************************************

// text formating slots

void LabelWidget::textChanged(){
	if (m_initializing)
		return;

	if (ui.tbTexUsed->isChecked()) {
		QString text=ui.teLabel->toPlainText();
		TextLabel::TextWrapper wrapper(text, true);

		foreach(TextLabel* label, m_labelsList)
			label->setText(wrapper);
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

void LabelWidget::charFormatChanged(const QTextCharFormat& format){
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

	if(!ui.tbTexUsed->isChecked())
		ui.kcbFontColor->setColor(format.foreground().color());
	ui.kfontRequester->setFont(format.font());
}

void LabelWidget::teXUsedChanged(bool checked){
	//hide text editing elements if TeX-option is used
	ui.tbFontBold->setVisible(!checked);
	ui.tbFontItalic->setVisible(!checked);

	//TODO: https://bugreports.qt.io/browse/QTBUG-25420
// 	ui.tbFontUnderline->setVisible(!checked);
// 	ui.tbFontStrikeOut->setVisible(!checked);

	ui.tbFontSubScript->setVisible(!checked);
	ui.tbFontSuperScript->setVisible(!checked);
	ui.tbSymbols->setVisible(!checked);

	ui.lFont->setVisible(!checked);
	ui.kfontRequester->setVisible(!checked);
	ui.lFontSize->setVisible(checked);
	ui.sbFontSize->setVisible(checked);

	if (m_initializing)
		return;

	QString text = checked ? ui.teLabel->toPlainText() : ui.teLabel->toHtml();
	TextLabel::TextWrapper wrapper(text, checked);
	foreach(TextLabel* label, m_labelsList)
		label->setText(wrapper);
}

void LabelWidget::fontColorChanged(const QColor& color){
	if (m_initializing)
		return;

	ui.teLabel->setTextColor(color);
	foreach(TextLabel* label, m_labelsList)
		label->setTeXFontColor(color);
}

void LabelWidget::fontSizeChanged(int value){
	if (m_initializing)
		return;

	foreach(TextLabel* label, m_labelsList)
		label->setTeXFontSize(value);
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

void LabelWidget::fontChanged(const QFont& font){
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

void LabelWidget::dateTimeMenu() {
	m_dateTimeMenu->clear();

	QDate date = QDate::currentDate();
	m_dateTimeMenu->addSeparator()->setText(i18n("Date"));
	m_dateTimeMenu->addAction( date.toString(Qt::TextDate) );
	m_dateTimeMenu->addAction( date.toString(Qt::ISODate) );
	m_dateTimeMenu->addAction( date.toString(Qt::TextDate) );
	m_dateTimeMenu->addAction( date.toString(Qt::SystemLocaleShortDate) );
	m_dateTimeMenu->addAction( date.toString(Qt::SystemLocaleLongDate) );

	QDateTime time = QDateTime::currentDateTime();
	m_dateTimeMenu->addSeparator()->setText(i18n("Date and Time"));
	m_dateTimeMenu->addAction( time.toString(Qt::TextDate) );
	m_dateTimeMenu->addAction( time.toString(Qt::ISODate) );
	m_dateTimeMenu->addAction( time.toString(Qt::TextDate) );
	m_dateTimeMenu->addAction( time.toString(Qt::SystemLocaleShortDate) );
	m_dateTimeMenu->addAction( time.toString(Qt::SystemLocaleLongDate) );

	m_dateTimeMenu->exec( mapToGlobal(ui.tbDateTime->rect().bottomLeft()));
}

void LabelWidget::insertDateTime(QAction* action) {
	ui.teLabel->insertPlainText( action->text().remove('&') );
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

	TextLabel::PositionWrapper position = m_label->position();
	position.horizontalPosition = TextLabel::HorizontalPosition(index);
	foreach(TextLabel* label, m_labelsList)
		label->setPosition(position);
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

	TextLabel::PositionWrapper position = m_label->position();
	position.verticalPosition = TextLabel::VerticalPosition(index);
	foreach(TextLabel* label, m_labelsList)
		label->setPosition(position);
}

void LabelWidget::customPositionXChanged(double value){
	if (m_initializing)
		return;

	TextLabel::PositionWrapper position = m_label->position();
	position.point.setX(Worksheet::convertToSceneUnits(value, Worksheet::Centimeter));
	foreach(TextLabel* label, m_labelsList)
		label->setPosition(position);
}

void LabelWidget::customPositionYChanged(double value){
	if (m_initializing)
		return;

	TextLabel::PositionWrapper position = m_label->position();
	position.point.setY(Worksheet::convertToSceneUnits(value, Worksheet::Centimeter));
	foreach(TextLabel* label, m_labelsList)
		label->setPosition(position);
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
void LabelWidget::labelTextWrapperChanged(const TextLabel::TextWrapper& text){
	m_initializing = true;

	//save and restore the current cursor position after changing the text
	QTextCursor cursor = ui.teLabel->textCursor();
	int position = cursor.position();
	ui.teLabel->setText(text.text);
	cursor.movePosition(QTextCursor::Start);
	cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, position);
	ui.teLabel->setTextCursor(cursor);

	ui.tbTexUsed->setChecked(text.teXUsed);
	this->teXUsedChanged(text.teXUsed);
	m_initializing = false;
}

void LabelWidget::labelTeXFontSizeChanged(const int size){
	m_initializing = true;
	ui.sbFontSize->setValue(size);
	m_initializing = false;
}

void LabelWidget::labelTeXFontColorChanged(const QColor color){
	m_initializing = true;
	ui.kcbFontColor->setColor(color);
	m_initializing = false;
}

void LabelWidget::labelPositionChanged(const TextLabel::PositionWrapper& position){
	m_initializing = true;
	ui.sbPositionX->setValue( Worksheet::convertFromSceneUnits(position.point.x(), Worksheet::Centimeter) );
	ui.sbPositionY->setValue( Worksheet::convertFromSceneUnits(position.point.y(), Worksheet::Centimeter) );
	ui.cbPositionX->setCurrentIndex( position.horizontalPosition );
	ui.cbPositionY->setCurrentIndex( position.verticalPosition );
	m_initializing = false;
}

void LabelWidget::labelHorizontalAlignmentChanged(TextLabel::HorizontalAlignment index){
	m_initializing = true;
	ui.cbHorizontalAlignment->setCurrentIndex(index);
	m_initializing = false;
}

void LabelWidget::labelVerticalAlignmentChanged(TextLabel::VerticalAlignment index){
	m_initializing = true;
	ui.cbVerticalAlignment->setCurrentIndex(index);
	m_initializing = false;
}

void LabelWidget::labelOffsetChanged(float offset){
	m_initializing = true;
	ui.sbOffset->setValue(Worksheet::convertFromSceneUnits(offset, Worksheet::Point));
	m_initializing = false;
}

void LabelWidget::labelRotationAngleChanged(float angle){
	m_initializing = true;
	ui.sbRotation->setValue(angle);
	m_initializing = false;
}

void LabelWidget::labelVisibleChanged(bool on){
	m_initializing = true;
	ui.chbVisible->setChecked(on);
	m_initializing = false;
}

//**********************************************************
//******************** SETTINGS ****************************
//**********************************************************
void LabelWidget::load() {
	if(m_label == NULL)
		return;

	m_initializing = true;

	ui.chbVisible->setChecked( m_label->isVisible() );

	//Text
	ui.teLabel->setHtml( m_label->text().text );
	ui.teLabel->selectAll();
	ui.teLabel->setFocus();
	ui.tbTexUsed->setChecked( (bool) m_label->text().teXUsed );
	this->teXUsedChanged(m_label->text().teXUsed);
	ui.sbFontSize->setValue( m_label->teXFontSize() );
	if(m_label->text().teXUsed)
		ui.kcbFontColor->setColor( m_label->teXFontColor() );

	//Set text format
	ui.tbFontBold->setChecked(ui.teLabel->fontWeight()==QFont::Bold);
	ui.tbFontItalic->setChecked(ui.teLabel->fontItalic());
	ui.tbFontUnderline->setChecked(ui.teLabel->fontUnderline());
	QTextCharFormat format = ui.teLabel->currentCharFormat();
	ui.tbFontStrikeOut->setChecked(format.fontStrikeOut());
	ui.tbFontSuperScript->setChecked(format.verticalAlignment() == QTextCharFormat::AlignSuperScript);
	ui.tbFontSubScript->setChecked(format.verticalAlignment() == QTextCharFormat::AlignSubScript);
	ui.kfontRequester->setFont(format.font());

	// Geometry
	ui.cbPositionX->setCurrentIndex( (int) m_label->position().horizontalPosition );
	ui.sbPositionX->setValue( Worksheet::convertFromSceneUnits(m_label->position().point.x(),Worksheet::Centimeter) );
	ui.cbPositionY->setCurrentIndex( (int) m_label->position().verticalPosition );
	ui.sbPositionY->setValue( Worksheet::convertFromSceneUnits(m_label->position().point.y(),Worksheet::Centimeter) );

	if (m_axesList.size())
		ui.sbOffset->setValue( Worksheet::convertFromSceneUnits(m_axesList.first()->titleOffset(), Worksheet::Point) );

	ui.cbHorizontalAlignment->setCurrentIndex( (int) m_label->horizontalAlignment() );
	ui.cbVerticalAlignment->setCurrentIndex( (int) m_label->verticalAlignment() );
	ui.sbRotation->setValue( m_label->rotationAngle() );


	m_initializing = false;
}

void LabelWidget::loadConfig(KConfigGroup &group) {
	if(m_label == NULL)
		return;

	m_initializing = true;

	ui.chbVisible->setChecked( group.readEntry("Visible", m_label->isVisible()) );

	//Text
	ui.tbTexUsed->setChecked(group.readEntry("TeXUsed", (bool) m_label->text().teXUsed));
	ui.sbFontSize->setValue( group.readEntry("TeXFontSize", m_label->teXFontSize()) );
	if(m_label->text().teXUsed)
		ui.kcbFontColor->setColor(group.readEntry("TeXFontColor", m_label->teXFontColor()));

	//Set text format
	ui.tbFontBold->setChecked(ui.teLabel->fontWeight()==QFont::Bold);
	ui.tbFontItalic->setChecked(ui.teLabel->fontItalic());
	ui.tbFontUnderline->setChecked(ui.teLabel->fontUnderline());
	QTextCharFormat format = ui.teLabel->currentCharFormat();
	ui.tbFontStrikeOut->setChecked(format.fontStrikeOut());
	ui.tbFontSuperScript->setChecked(format.verticalAlignment() == QTextCharFormat::AlignSuperScript);
	ui.tbFontSubScript->setChecked(format.verticalAlignment() == QTextCharFormat::AlignSubScript);
	ui.kfontRequester->setFont(format.font());

	// Geometry
	ui.cbPositionX->setCurrentIndex( group.readEntry("PositionX", (int) m_label->position().horizontalPosition ) );
	ui.sbPositionX->setValue( Worksheet::convertFromSceneUnits(group.readEntry("PositionXValue", m_label->position().point.x()),Worksheet::Centimeter) );
	ui.cbPositionY->setCurrentIndex( group.readEntry("PositionY", (int) m_label->position().verticalPosition ) );
	ui.sbPositionY->setValue( Worksheet::convertFromSceneUnits(group.readEntry("PositionYValue", m_label->position().point.y()),Worksheet::Centimeter) );

	if (m_axesList.size())
		ui.sbOffset->setValue( Worksheet::convertFromSceneUnits(group.readEntry("Offset", m_axesList.first()->titleOffset()), Worksheet::Point) );

	ui.cbHorizontalAlignment->setCurrentIndex( group.readEntry("HorizontalAlignment", (int) m_label->horizontalAlignment()) );
	ui.cbVerticalAlignment->setCurrentIndex( group.readEntry("VerticalAlignment", (int) m_label->verticalAlignment()) );
	ui.sbRotation->setValue( group.readEntry("Rotation", m_label->rotationAngle()) );

	m_initializing = false;
}

void LabelWidget::saveConfig(KConfigGroup &group) {
	//Text
	group.writeEntry("TeXUsed", ui.tbTexUsed->isChecked());
	group.writeEntry("TeXFontColor", ui.kcbFontColor->color());
	group.writeEntry("TeXFontSize", ui.sbFontSize->value());

	// Geometry
	group.writeEntry("PositionX", ui.cbPositionX->currentIndex());
	group.writeEntry("PositionXValue", Worksheet::convertToSceneUnits(ui.sbPositionX->value(),Worksheet::Centimeter) );
	group.writeEntry("PositionY", ui.cbPositionY->currentIndex());
	group.writeEntry("PositionYValue",  Worksheet::convertToSceneUnits(ui.sbPositionY->value(),Worksheet::Centimeter) );

	if (m_axesList.size())
		group.writeEntry("Offset",  Worksheet::convertToSceneUnits(ui.sbOffset->value(), Worksheet::Point) );

	group.writeEntry("HorizontalAlignment", ui.cbHorizontalAlignment->currentIndex());
	group.writeEntry("VerticalAlignment", ui.cbVerticalAlignment->currentIndex());
	group.writeEntry("Rotation", ui.sbRotation->value());
}
