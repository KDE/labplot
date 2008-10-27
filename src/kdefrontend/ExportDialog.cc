/***************************************************************************
    File                 : ExportDialog.cc
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Stefan Gerlach
    Email (use @ for *)  : stefan.gerlach*uni-konstanz.de
    Description          : dialog for exporting data
                           
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
#include "ExportDialog.h"
#include "MainWin.h"

ExportDialog::ExportDialog(Spreadsheet *parent) : KDialog(parent) {
	kDebug()<<endl;
	setCaption(i18n("Export Data"));

	setupGUI();
}

void ExportDialog::setupGUI() {
	kDebug()<<endl;
	exportWidget = new ExportWidget( this );
	setMainWidget( exportWidget );

	setButtons( KDialog::Ok | KDialog::User1 | KDialog::User2 | KDialog::Cancel | KDialog::Apply );
        setButtonText(KDialog::User1,i18n("Save"));
        setButtonText(KDialog::User2,i18n("Show Options"));

	connect(this,SIGNAL(applyClicked()),SLOT(apply()));
	connect(this,SIGNAL(okClicked()),SLOT(apply()));
	connect(this,SIGNAL(user1Clicked()),exportWidget,SLOT(save()));
	connect(this,SIGNAL(user2Clicked()),exportWidget,SLOT(toggleOptions()));

	resize( QSize(500,200) );
}
