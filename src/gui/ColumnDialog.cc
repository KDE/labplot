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
#include "../column.h"
#include "../MainWin.h"

ColumnDialog::ColumnDialog(MainWin *parent, Spreadsheet *s) : KDialog(parent), s(s) {
	kDebug()<<"ColumnDialog()"<<endl;

	setupGUI();
}

void ColumnDialog::setupGUI() {
	kDebug()<<endl;
	QWidget *widget = new QWidget(this);
	ui.setupUi(widget);
	
	ui.leLabel->setText(s->columnName(s->currentColumn()));
	ui.cbFormat->insertItems(0,columnformatitems);
	ui.cbFormat->setCurrentItem(s->columnFormat(s->currentColumn()));
	ui.cbType->insertItems(0,columntypeitems);
	ui.cbType->setCurrentItem(s->columnType(s->currentColumn()));

	setMainWidget(widget);
	setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );
	setCaption(i18n("Column settings"));
	connect(this,SIGNAL(applyClicked()),SLOT(apply()));
	connect(this,SIGNAL(okClicked()),SLOT(apply()));
}

void ColumnDialog::apply() {
	int col = s->currentColumn();
	s->setColumnName(col,ui.leLabel->text());
	s->setColumnFormat(col,ui.cbFormat->currentText());
	s->setColumnType(col,ui.cbType->currentText());
}
