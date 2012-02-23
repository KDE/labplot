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
#include "worksheet/Worksheet.h"
#include "../../backend/worksheet/ScalableTextLabel.h"
#include <KDebug>
#include <QMenu>


/*!
	\class LabelWidget
 	\brief Widget for editing the properties of a Scalable Text Label object.

 	\ingroup kdefrontend
 */

// see legacy/LabelWidget.cpp
LabelWidget::LabelWidget(QWidget *parent): QWidget(parent){
	ui.setupUi(this);
	
	QGridLayout* layout =static_cast<QGridLayout*>(this->layout());
  	layout->setContentsMargins(2,2,2,2);
	layout->setHorizontalSpacing(2);
	layout->setVerticalSpacing(2);


		//Populate the menus //TODO
	QMenu* menu = new QMenu(this);
	QFont symbol("Symbol", 12, QFont::Bold);
 	menu->setFont(symbol);
	for(int i=97;i<122;i++)
		menu->addAction( QChar(i) );
	
	ui.tbSmallGreekLetters->setMenu(menu);
	ui.tbSmallGreekLetters->setDefaultAction(menu->menuAction());

	//Icons
	ui.tbFontBold->setIcon( KIcon("format-text-bold") );
	ui.tbFontItalic->setIcon( KIcon("format-text-italic") );
	ui.tbFontUnderline->setIcon( KIcon("format-text-underline") );
	ui.tbFontSuperscript->setIcon( KIcon("format-text-superscript") );
	ui.tbFontSubscript->setIcon( KIcon("format-text-subscript") );
	
	// Geometry
	connect( ui.cbPositionX, SIGNAL(currentIndexChanged(int)), this, SLOT(positionXChanged(int)) );
	connect( ui.cbPositionY, SIGNAL(currentIndexChanged(int)), this, SLOT(positionYChanged(int)) );
	connect( ui.sbRotation, SIGNAL(valueChanged(const QString&)), this, SLOT(slotDataChanged()) );
	
	//TODO remove later
	ui.cbPositionX->setCurrentIndex(0);
	ui.cbPositionY->setCurrentIndex(0);
	ui.lOffset->hide();
	ui.sbOffset->hide();	
}

LabelWidget::~LabelWidget() {}

void LabelWidget::setLabel(ScalableTextLabel *label) {
	m_label = label;

	//TODO: set ui elements
	// alignment
	ui.sbRotation->setValue( label->rotationAngle() );

	// background
	
	//text
}

//**********************************************************
//******************** SLOTS *******************************
//**********************************************************
//TODO

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
	emit dataChanged(true);
}

void LabelWidget::positionYChanged(int index){
	if (index == ui.cbPositionY->count()-1 ){
		ui.sbPositionY->setEnabled(true);
	}else{
		ui.sbPositionY->setEnabled(false);
	}
	emit dataChanged(true);
}

void LabelWidget::slotDataChanged(){
	emit dataChanged(true);
}

//**********************************************************
//******************** SETTINGS ****************************
//**********************************************************

void LabelWidget::loadConfig(KConfigGroup &group) {
	if(m_label == NULL)
		return;

	//Text
	//TODO

	// Geometry
	//TODO: saved in ScalableTextLabel ?
//	ui.cbPositionX->setCurrentIndex( group.readEntry("TitlePositionX", (int) m_label->position()) );
	ui.sbPositionX->setValue( Worksheet::convertFromSceneUnits(group.readEntry("TitlePositionXValue", m_label->position().x()),Worksheet::Centimeter) );
//	ui.cbPositionY->setCurrentIndex( group.readEntry("TitlePositionY", (int) m_label->positionY()) );
	ui.sbPositionY->setValue( Worksheet::convertFromSceneUnits(group.readEntry("TitlePositionYValue", m_label->position().y()),Worksheet::Centimeter) );
	//group.writeEntry("TitleOffset", ui.sbOffset->value());
	ui.cbHorizontalAlignment->setCurrentIndex( group.readEntry("TitleHorizontalAlignment", (int) m_label->horizontalAlignment()) );
	ui.cbVerticalAlignment->setCurrentIndex( group.readEntry("TitleVerticalAlignment", (int) m_label->verticalAlignment()) );
	ui.sbRotation->setValue( group.readEntry("TitleRotation", m_label->rotationAngle()) );
}

void LabelWidget::saveConfig(KConfigGroup &group) {
	//Text
	//TODO

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
