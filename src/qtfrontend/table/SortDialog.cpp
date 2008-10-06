/***************************************************************************
    File                 : SortDialog.cpp
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : Sorting options dialog

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
#include "SortDialog.h"

#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QGroupBox>
#include <QLayout>
#include <QApplication>

SortDialog::SortDialog( QWidget* parent, Qt::WFlags fl )
    : QDialog( parent, fl )
{
    setWindowIcon(qApp->windowIcon());
	setWindowTitle(tr("Sorting Options"));
	setSizeGripEnabled(true);

	QGroupBox *group_box1 = new QGroupBox();
	QGridLayout * top_layout = new QGridLayout(group_box1);
	QHBoxLayout * hl = new QHBoxLayout();
	hl->addStretch();

	top_layout->addWidget( new QLabel(tr("Sort columns")), 0, 0 );
	ui.box_type = new QComboBox();
	ui.box_type->addItem(tr("Separately"));
	ui.box_type->addItem(tr("Together"));
	top_layout->addWidget(ui.box_type, 0, 1 );
	ui.box_type->setCurrentIndex(Together);

	top_layout->addWidget( new QLabel( tr("Order")), 1, 0 );
	ui.box_order = new QComboBox();
    ui.box_order->addItem(tr("Ascending"));
	ui.box_order->addItem(tr("Descending"));
	top_layout->addWidget(ui.box_order, 1, 1 );

	top_layout->addWidget( new QLabel(tr("Leading column")), 2, 0 );
	ui.columns_list = new QComboBox();
	top_layout->addWidget(ui.columns_list, 2, 1);
	top_layout->setRowStretch(3, 1);

	ui.button_ok = new QPushButton(tr("&Sort"));
    ui.button_ok->setDefault( true );
	hl->addWidget(ui.button_ok);

    ui.button_cancel = new QPushButton(tr("&Cancel"));
	hl->addWidget(ui.button_cancel);

	QVBoxLayout * mainlayout = new QVBoxLayout(this);
    mainlayout->addWidget(group_box1);
	mainlayout->addLayout(hl);

    connect( ui.button_ok, SIGNAL( clicked() ), this, SLOT( accept() ) );
    connect( ui.button_cancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
    connect( ui.box_type, SIGNAL( currentIndexChanged(int) ), this, SLOT(changeType(int)));
}

void SortDialog::accept()
{
	Column* leading;
	if(ui.box_type->currentIndex() == Together) 
		leading = m_columns_list.at(ui.columns_list->currentIndex());
	emit sort(leading, m_columns_list, ui.box_order->currentIndex() == Ascending );
	//close();
}

void SortDialog::setColumnsList(QList<Column*> list)
{
	m_columns_list = list;

	for(int i=0; i<list.size(); i++)
		ui.columns_list->addItem( list.at(i)->name() );
	ui.columns_list->setCurrentIndex(0);
}

void SortDialog::changeType(int Type)
{
	if(Type == Together)
		ui.columns_list->setEnabled(true);
	else
		ui.columns_list->setEnabled(false);
}
