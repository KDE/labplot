/***************************************************************************
    File                 : ColumnDialog.cc
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Stefan Gerlach
    Email (use @ for *)  : stefan.gerlach*uni-konstanz.de
    Description          : dialog for column properties
                           
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
#include "ColumnDialog.h"
#include "SpreadsheetView.h"

ColumnDialog::ColumnDialog(QWidget *parent, SpreadsheetView *s) : KDialog(parent), s(s) {
	kDebug()<<endl;

	setupGUI();
}

void ColumnDialog::setupGUI() {
// 	kDebug()<<endl;
// 	QWidget *widget = new QWidget(this);
// 	ui.setupUi(widget);
// 	
// 	ui.leLabel->setText(s->columnName(s->currentColumn()));
// 
// 	// column format selection
// 	unsigned int i=0, j=0;
// //	kDebug()<<"size : "<<sizeof(SciDAVis::ColumnMode)<<endl;
// 	do {
// 		QString item = SciDAVis::enumValueToString(j++, "ColumnMode");
// //		kDebug()<<i<<item<<endl;
// 		if(! item.isEmpty())
// 			ui.cbFormat->insertItem(i++,item);
// 	} while (i<5);
// 	// sizeof(SciDAVis::ColumnMode) does not work	
// 	//kDebug()<<s->columnFormat(s->currentColumn())<<endl;
// 	ui.cbFormat->setCurrentIndex((int)s->columnFormat(s->currentColumn()));
// 
// 	// column type selection
// 	i=0, j=0;
// //	kDebug()<<"size : "<<sizeof(SciDAVis::PlotDesignation)<<endl;
// 	do {
// 		QString item = SciDAVis::enumValueToString(j++, "PlotDesignation");
// //		kDebug()<<i<<item<<endl;
// 		if(! item.isEmpty())
// 			ui.cbType->insertItem(i++,item);
// 	} while (i<6);
// 	
// 	//kDebug()<<s->columnType(s->currentColumn())<<endl;
// 	ui.cbType->setCurrentIndex((int)(s->columnType(s->currentColumn())));
// 
// 	setMainWidget(widget);
// 	setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );
// 	setCaption(i18n("Column settings"));
// 	connect(this,SIGNAL(applyClicked()),SLOT(apply()));
// 	connect(this,SIGNAL(okClicked()),SLOT(apply()));
}

void ColumnDialog::apply() {
// 	int col = s->currentColumn();
// 	s->setColumnName(col,ui.leLabel->text());
// 	// item-enum mismatch :
// 	s->setColumnFormat(col,(SciDAVis::ColumnMode) SciDAVis::enumStringToValue(
// 		ui.cbFormat->currentText(),"ColumnMode"));
// 	s->setColumnType(col,(SciDAVis::PlotDesignation) ui.cbType->currentIndex());
// 	//TODO : repaint header ?
}
