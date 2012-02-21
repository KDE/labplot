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
#include "backend/worksheet/ScalableTextLabel.h"
#include <KDebug>

/*!
	\class LabelWidget
 	\brief Widget for editing the properties of a Scalable Text Label object.

 	\ingroup kdefrontend
 */

// see legacy/LabelWidget.cpp
LabelWidget::LabelWidget(QWidget *parent): QWidget(parent){
	ui.setupUi(this);
	
	connect( ui.leRotation, SIGNAL(textChanged(const QString&)), this, SLOT(slotDataChanged()) );
}

LabelWidget::~LabelWidget() {}

void LabelWidget::setLabel(ScalableTextLabel *label) {
	m_label = label;

	//TODO: set ui elements
	// alignment
	ui.leRotation->setText( QString::number(label->rotationAngle()) );

	// background
	
	//text

}

//**********************************************************
//****************** SLOTS *******************************
//**********************************************************

//TODO

void LabelWidget::slotDataChanged(){
	emit dataChanged(true);
}
