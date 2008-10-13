/***************************************************************************
    File                 : ImportDialog.cc
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Stefan Gerlach
    Email (use @ for *)  : stefan.gerlach*uni-konstanz.de
    Description          : import data dialog
                           
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

#include <KDebug>
#include "ImportDialog.h"
#include "../MainWin.h"

ImportDialog::ImportDialog(MainWin *parent) : KDialog(parent), mainWin(parent) {
	kDebug()<<endl;
	setCaption(i18n("Import Data"));

	setupGUI();
}

void ImportDialog::setupGUI() {
	kDebug()<<endl;
	importWidget = new ImportWidget( this );
	setMainWidget( importWidget );

	setButtons( KDialog::Ok | KDialog::User1 | KDialog::User2 | KDialog::Cancel | KDialog::Apply );
        setButtonText(KDialog::User1,i18n("Save"));
        setButtonText(KDialog::User2,i18n("Show Options"));

	connect(this,SIGNAL(applyClicked()),SLOT(apply()));
	connect(this,SIGNAL(okClicked()),SLOT(apply()));
	connect(this,SIGNAL(user1Clicked()),importWidget,SLOT(save()));
	connect(this,SIGNAL(user2Clicked()),importWidget,SLOT(toggleOptions()));

	resize( QSize(500,200) );
}
