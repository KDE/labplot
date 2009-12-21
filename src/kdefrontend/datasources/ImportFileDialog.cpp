/***************************************************************************
    File                 : ImportDialog.cc
    Project              : LabPlot
    Description          : import file data dialog
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Stefan Gerlach
    Email (use @ for *)  : stefan.gerlach*uni-konstanz.de, alexander.semke*web.de

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

#include "ImportFileDialog.h"
#include "ImportFileWidget.h"
#include "backend/datasources/FileDataSource.h"

 /*!
	\class ImportFileDialog
	\brief Dialog for importing data from a file. Embeddes \c ImportFileWidget and provides the standard buttons.

	\ingroup kdefrontend
 */
 
ImportFileDialog::ImportFileDialog(QWidget* parent) : KDialog(parent) {
	importFileWidget = new ImportFileWidget( this );
	setMainWidget( importFileWidget );

    setButtons( KDialog::Ok | KDialog::User1 | KDialog::Cancel );
	setButtonText(KDialog::User1,i18n("Show Options"));

    //connect(this,SIGNAL(applyClicked()),SLOT(apply()));
// 	connect(this,SIGNAL(okClicked()),SLOT(apply()));
// 	connect(this,SIGNAL(user1Clicked()),importWidget,SLOT(save()));
	connect(this,SIGNAL(user1Clicked()), this, SLOT(toggleOptions()));
	
	setCaption(i18n("Import Data"));
	setWindowIcon(KIcon("document-import-database"));
	resize( QSize(500,0).expandedTo(minimumSize()) );
}

void ImportFileDialog::saveSettings(FileDataSource* source) const{
	importFileWidget->saveSettings(source);
}

void ImportFileDialog::toggleOptions(){
	if (importFileWidget->toggleOptions()){
		setButtonText(KDialog::User1,i18n("Hide Options"));
	}else{
		setButtonText(KDialog::User1,i18n("Show Options"));
	}

	//resize the dialog
	layout()->activate();
	resize( QSize(this->width(),0).expandedTo(minimumSize()) );
}
