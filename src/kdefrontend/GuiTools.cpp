/***************************************************************************
    File                 : GuiTools.cpp
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2011 Alexander Semke (alexander.semke*web.de)
                           (replace * with @ in the email addresses)
    Description          :  constains several static functions which are used on frequently throughout the kde frontend.
                           
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


#include "GuiTools.h"
#include <KLocale>
// #include <KLocalizedString>
/*!
	fills the ComboBox \c combobox with the six possible Qt::PenStyles, the color \c color is used.
*/
void GuiTools::updatePenStyles(QComboBox* comboBox, const QColor& color){
	int index=comboBox->currentIndex();
	comboBox->clear();

	QPainter pa;
	int offset=2;
	int w=50;
	int h=10;
	QPixmap pm( w, h );
	comboBox->setIconSize( QSize(w,h) );
	
	//loop over six possible Qt-PenStyles, draw on the pixmap and insert it
	static QString list[6] = { i18n("no line"), i18n("solid line"), i18n("dash line"),
							   i18n("dot line"), i18n("dash-dot line"), i18n("dash-dot-dot line") };
	for (int i=0;i<6;i++){
		pm.fill(Qt::transparent);
		pa.begin( &pm );
		pa.setPen( QPen( color, 1, (Qt::PenStyle)i ) );
		pa.drawLine( offset, h/2, w-offset, h/2);
		pa.end();
 		comboBox->addItem( QIcon(pm), list[i] );
	}
	comboBox->setCurrentIndex(index);
}

/*!
	fills the QMenu \c menu with the six possible Qt::PenStyles, the color \c color is used.
	QActions are created with \c actionGroup as the parent, if not available.
	If already available, onle the color in the QAction's icons is updated.
*/
void GuiTools::updatePenStyles(QMenu* menu, QActionGroup* actionGroup, const QColor& color){
	QPainter pa;
	int offset=2;
	int w=50;
	int h=10;
	QPixmap pm( w, h );
	
	//loop over six possible Qt-PenStyles, draw on the pixmap and insert it
	static QString list[6] = { i18n("no line"), i18n("solid line"), i18n("dash line"),
							   i18n("dot line"), i18n("dash-dot line"), i18n("dash-dot-dot line") };

	QAction* action;
	if (actionGroup->actions().size() == 0){
		//TODO setting of the icon size doesn't work here
		menu->setStyleSheet( "QMenu::icon { width:50px; height:10px; }" );
		
		for (int i=0;i<6;i++){
			pm.fill(Qt::transparent);
			pa.begin( &pm );
			pa.setPen( QPen( color, 1, (Qt::PenStyle)i ) );
			pa.drawLine( offset, h/2, w-offset, h/2);
			pa.end();
			action = new QAction( QIcon(pm), list[i], actionGroup );
			action->setCheckable(true);
			menu->addAction( action );
			
		}
	}else{
		for (int i=0;i<6;i++){
			pm.fill(Qt::transparent);
			pa.begin( &pm );
			pa.setPen( QPen( color, 1, (Qt::PenStyle)i ) );
			pa.drawLine( offset, h/2, w-offset, h/2);
			pa.end();
			action = actionGroup->actions()[i];
			action->setIcon( QIcon(pm) );
		}
	}
}

/*!
	fills the ComboBox for the symbol filling patterns with the 14 possible Qt::BrushStyles.
*/
void GuiTools::updateBrushStyles(QComboBox* comboBox, const QColor& color){
  	int index=comboBox->currentIndex();
	comboBox->clear();

	QPainter pa;
	int offset=2;
	int w=50;
	int h=20;
	QPixmap pm( w, h );
	comboBox->setIconSize( QSize(w,h) );

	QPen pen(Qt::SolidPattern, 1);
 	pa.setPen( pen );
	
	static QString list[15] = { i18n("none"), i18n("uniform"), i18n("extremely dense"),
								i18n("very dense"), i18n("somewhat dense"), i18n("half dense"),
								i18n("somewhat sparse"), i18n("very sparse"), i18n("extremely sparse"),
								i18n("horiz. lines"), i18n("vert. lines"), i18n("crossing lines"), 
								i18n("backward diag. lines"), i18n("forward diag. lines"), i18n("crossing diag. lines") };
	for (int i=0;i<15;i++) {
		pm.fill(Qt::transparent);
		pa.begin( &pm );
		pa.setRenderHint(QPainter::Antialiasing);
 		pa.setBrush( QBrush(color, (Qt::BrushStyle)i) );
		pa.drawRect( offset, offset, w - 2*offset, h - 2*offset);
		pa.end();
		comboBox->addItem( QIcon(pm), list[i] );
	}

	comboBox->setCurrentIndex(index);
}

//TODO
void GuiTools::fillColorMenu(QMenu* menu, QActionGroup* actionGroup){
	
}